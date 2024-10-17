// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText:  2022 Istvan Pasztor
// 
// https://github.com/pasztorpisti/hid-report-parser/tree/master (with private VPX modifications)
// 
// Note: This is a customized version for VPX's use.  There
// should be no need for any ongoing maintenance work to keep our
// forked version up to date with the base project, because the
// base project is no longer being developed or maintained; it's
// marked as "archived", so it's presumably frozen permanently
// at the version we used as our starting point.
//
// The VPX-specific modifications are mostly to add support for
// some additional HID elements not supported in the original
// library but used in the Open Pinball Device report descriptor,
// particularly string usage indices.

#include "stdafx.h"
#include "hid_report_parser.h"

namespace hidrp {


	static const char* STR_ERROR[] = {
		"ERR_SUCCESS",                              //  0
		"ERR_UNSPECIFIED",                          // -1
		"ERR_INCOMPLETE_ITEM",                      // -2
		"ERR_INVALID_ITEM_TYPE",                    // -3
		"ERR_UNCLOSED_COLLECTION",                  // -4
		"ERR_PUSH_WITHOUT_POP",                     // -5
		"ERR_INVALID_COLLECTION_TYPE",              // -6
		"ERR_NO_COLLECTION_TO_CLOSE",               // -7
		"ERR_LONELY_USAGE_MIN_MAX",                 // -8
		"ERR_BAD_REPORT_ID_ASSIGNMENT",             // -9
		"ERR_LOGICAL_MIN_IS_GREATER_THAN_MAX",      // -10
		"ERR_INVALID_USAGE_PAGE",                   // -11
		"ERR_REPORT_SIZE_TOO_LARGE",                // -12
		"ERR_REPORT_COUNT_TOO_LARGE",               // -13
		"ERR_INVALID_REPORT_ID",                    // -14
		"ERR_PUSH_STACK_OVERFLOW",                  // -15
		"ERR_NOTHING_TO_POP",                       // -16
		"ERR_TOO_MANY_USAGES",                      // -17
		"ERR_EXTENDED_USAGE_MIN_MAX_PAGE_MISMATCH", // -18
		"ERR_INVALID_USAGE_MIN_MAX_RANGE",          // -19
		"ERR_INVALID_PARAMETERS",                   // -20
		"ERR_COULD_NOT_MAP_ANY_USAGES",             // -21
		"ERR_UNINITIALISED_PARSER",                 // -22
		"ERR_NOTHING_CHANGED",                      // -23
		"ERR_INVALID_REPORT_SIZE",                  // -24
		"ERR_UNDEFINED_USAGE_PAGE",                 // -25
	};
	static_assert(26 == sizeof(STR_ERROR)/sizeof(STR_ERROR[0]), "wrong array size");


	const char* str_error(int error_code, const char* default_str) {
		if (error_code > 0 || error_code < -25)
			return default_str;
		return STR_ERROR[-error_code];
	}


namespace {


	// ITEM_LONG is a special value that can be used without a mask.
	static constexpr uint8_t ITEM_LONG = 0b11111110;

	static constexpr uint8_t ITEM_TAG_MASK  = 0b11110000;
	static constexpr uint8_t ITEM_TYPE_MASK = 0b00001100;
	static constexpr uint8_t ITEM_SIZE_MASK = 0b00000011;
	static constexpr uint8_t ITEM_TAG_AND_TYPE_MASK = ITEM_TAG_MASK | ITEM_TYPE_MASK;

	// To be used with the ITEM_TYPE_MASK:
	static constexpr uint8_t ITEM_TYPE_MAIN   = 0b00000000;
	static constexpr uint8_t ITEM_TYPE_GLOBAL = 0b00000100;
	static constexpr uint8_t ITEM_TYPE_LOCAL  = 0b00001000;

	// Main items: hid1_11.pdf section 6.2.2.4:
	// To be used with the ITEM_TAG_AND_TYPE_MASK:
	static constexpr uint8_t ITEM_INPUT          = 0b10000000;
	static constexpr uint8_t ITEM_OUTPUT         = 0b10010000;
	static constexpr uint8_t ITEM_FEATURE        = 0b10110000;
	static constexpr uint8_t ITEM_COLLECTION     = 0b10100000;
	static constexpr uint8_t ITEM_END_COLLECTION = 0b11000000;

	// Global items: hid1_11.pdf section 6.2.2.7:
	// To be used with the ITEM_TAG_AND_TYPE_MASK:
	static constexpr uint8_t ITEM_USAGE_PAGE    = 0b00000100;
	static constexpr uint8_t ITEM_LOGICAL_MIN   = 0b00010100;
	static constexpr uint8_t ITEM_LOGICAL_MAX   = 0b00100100;
	static constexpr uint8_t ITEM_PHYSICAL_MIN  = 0b00110100;
	static constexpr uint8_t ITEM_PHYSICAL_MAX  = 0b01000100;
	static constexpr uint8_t ITEM_UNIT_EXPONENT = 0b01010100;
	static constexpr uint8_t ITEM_UNIT          = 0b01100100;
	static constexpr uint8_t ITEM_REPORT_SIZE   = 0b01110100;
	static constexpr uint8_t ITEM_REPORT_ID     = 0b10000100;
	static constexpr uint8_t ITEM_REPORT_COUNT  = 0b10010100;
	static constexpr uint8_t ITEM_PUSH          = 0b10100100;
	static constexpr uint8_t ITEM_POP           = 0b10110100;

	// Local items: hid1_11.pdf section 6.2.2.8:
	// To be used with the ITEM_TAG_AND_TYPE_MASK:
	static constexpr uint8_t ITEM_USAGE            = 0b00001000;
	static constexpr uint8_t ITEM_USAGE_MIN        = 0b00011000;
	static constexpr uint8_t ITEM_USAGE_MAX        = 0b00101000;
	static constexpr uint8_t ITEM_DESIGNATOR_INDEX = 0b00111000;
	static constexpr uint8_t ITEM_DESIGNATOR_MIN   = 0b01001000;
	static constexpr uint8_t ITEM_DESIGNATOR_MAX   = 0b01011000;
	static constexpr uint8_t ITEM_STRING_INDEX     = 0b01111000;
	static constexpr uint8_t ITEM_STRING_MIN       = 0b10001000;
	static constexpr uint8_t ITEM_STRING_MAX       = 0b10011000;
	static constexpr uint8_t ITEM_DELIMITER        = 0b10101000;


	// Fails if data_size isn't one of the expected values (0, 1, 2, 4).
	bool usage_data(const uint8_t* p, uint8_t data_size, uint16_t& usage, uint16_t& usage_page) {
		usage_page = 0;
		usage = 0;
		switch (data_size) {
		case 4: usage_page = (uint16_t)p[2] | ((uint16_t)p[3] << 8);
		case 2: usage |= (uint16_t)p[1] << 8;
		case 1: usage |= p[0];
		case 0: return true;
		default: return false;
		}
	}

	// Fails if the unsigned value is greater than 0xFF.
	// Fails if data_size isn't one of the expected values (0, 1, 2, 4).
	bool uint8_data(const uint8_t* p, uint8_t data_size, uint8_t& data) {
		switch (data_size) {
		case 4:
			if (p[2] || p[3])
				return false;
		case 2:
			if (p[1])
				return false;
		case 1:
			data = p[0];
			return true;
		case 0:
			data = 0;
			return true;
		default:
			return false;
		}
	}

	// Fails if allow_overflow=false and the unsigned value is greater than 0xFFFF.
	// Fails if data_size isn't one of the expected values (0, 1, 2, 4).
	bool uint16_data(const uint8_t* p, uint8_t data_size, uint16_t& data, bool allow_overflow = false) {
		switch (data_size) {
		case 4:
			if (!allow_overflow && (p[2] | p[3]))
				return false;
		case 2:
			data = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
			return true;
		case 1:
			data = p[0];
			return true;
		case 0:
			data = 0;
			return true;
		default:
			return false;
		}
	}

	// Fails if data_size isn't one of the expected values (0, 1, 2, 4).
	bool uint32_data(const uint8_t* p, uint8_t data_size, uint32_t& data) {
		data = 0;
		switch (data_size) {
		case 4: data |= ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
		case 2: data |= (uint32_t)p[1] << 8;
		case 1: data |= p[0];
		case 0: return true;
		default: return false;
		}
	}

	// Fails if data_size isn't one of the expected values (0, 1, 2, 4).
	bool int32_data(const uint8_t* p, uint8_t data_size, int32_t& data) {
		switch (data_size) {
		case 4:
			data = (int32_t)((int32_t)p[0] | ((int32_t)p[1] << 8) | ((int32_t)p[2] << 16) | ((int32_t)p[3] << 24));
			return true;
		case 2:
			data = (int16_t)((int16_t)p[0] | ((int16_t)p[1] << 8));
			return true;
		case 1:
			data = (int8_t)p[0];
			return true;
		case 0:
			data = 0;
			return true;
		default:
			return false;
		}
	}


} // namespace


	int DescriptorParser::Parse(const void* descriptor, size_t descriptor_size, EventHandler* handler) {
		Reset();

		auto p = (const uint8_t*)descriptor;
		auto q = p + descriptor_size;

		int res;

		while (p < q) {
			uint8_t b = *p++;
			auto bytes_left = q - p;

			if (b == ITEM_LONG) { // long item, skipping
				if (bytes_left < 1)
					return ERR_INCOMPLETE_ITEM;
				p += 2 + (size_t)*p;
				continue;
			}

			uint8_t data_size = b & ITEM_SIZE_MASK;
			if (data_size == 3)
				data_size = 4;
			if (bytes_left < data_size)
				return ERR_INCOMPLETE_ITEM;

			uint8_t item = b & ITEM_TAG_AND_TYPE_MASK;

			switch (b & ITEM_TYPE_MASK) {
			case ITEM_TYPE_MAIN:
				if ((res = AssertMinMaxItemsAreMatched()) != ERR_SUCCESS)
					return res;
				if ((res = ParseMainItems(item, p, data_size, handler)) != ERR_SUCCESS)
					return res;
				_locals.Reset();
				break;

			case ITEM_TYPE_GLOBAL:
				if ((res = ParseGlobalItems(item, p, data_size)) != ERR_SUCCESS)
					return res;
				break;

			case ITEM_TYPE_LOCAL:
				if ((res = ParseLocalItems(item, p, data_size)) != ERR_SUCCESS)
					return res;
				break;

			default:
				return ERR_INVALID_ITEM_TYPE;
			}

			p += data_size;
		}

		if (p > q)
			return ERR_INCOMPLETE_ITEM;
		if ((res = AssertMinMaxItemsAreMatched()) != ERR_SUCCESS)
			return res;
		if (_collection_depth != 0)
			return ERR_UNCLOSED_COLLECTION;
		if (_globals_stack_size != 0)
			return ERR_PUSH_WITHOUT_POP;

		return ERR_SUCCESS;
	}


	int DescriptorParser::AssertMinMaxItemsAreMatched() {
		if (_locals.flags & (Locals::FLAG_USAGE_MIN | Locals::FLAG_USAGE_MAX)) {
#if HRP_IGNORE_LONELY_USAGE_MIN_OR_MAX
			_locals.num_usage_ranges--;
			_locals.flags &= ~(Locals::FLAG_USAGE_MIN | Locals::FLAG_USAGE_MAX);
#else
			return ERR_LONELY_USAGE_MIN_MAX;
#endif
		}
		return 0;
	}


	int DescriptorParser::ParseMainItems(uint8_t item, const uint8_t* p_data, uint8_t data_size, EventHandler* handler) {
		switch (item) {
		case ITEM_COLLECTION:
		{
			uint8_t collection_type;
			if (!uint8_data(p_data, data_size, collection_type))
				return ERR_INVALID_COLLECTION_TYPE;
			_collection_depth++;
			// The HID specification allows other collection parameters too
			// but parsers seem to ignore them. HID specification:
			//     String and Physical indices, as well as
			//     delimiters may be associated with collections.
			return handler->BeginCollection(collection_type, _globals.usage_page, _locals.FirstUsage(), _collection_depth);
		}

		case ITEM_END_COLLECTION:
			if (_collection_depth == 0)
				return ERR_NO_COLLECTION_TO_CLOSE;
			handler->EndCollection(_collection_depth);
			_collection_depth--;
			return 0;

		case ITEM_INPUT:
			return AddField(ReportType::input, p_data, data_size, handler);

		case ITEM_OUTPUT:
			return AddField(ReportType::output, p_data, data_size, handler);

		case ITEM_FEATURE:
			return AddField(ReportType::feature, p_data, data_size, handler);

		default:
			return 0;
		}
	}

	int DescriptorParser::AddField(ReportType rt, const uint8_t* p_data, uint8_t data_size, EventHandler* handler) {
		FieldParams fp;
		fp.bit_size = _globals.report_size * _globals.report_count;

		// HID specification:
		//   If an item has no controls (Report Count = 0), the Local item
		//   tags apply to the Main item (usually a collection item).
		//
		// I have never seen a real-life example to this REPORT_COUNT=0 trick
		// so our implementation silently ignores this zero sized "field".
		// The only items that some descriptor parsers sometimes seem to apply
		// to collections is a USAGE_PAGE + USAGE pair. That is easier to change
		// by closing the current collection and opening a new one in the HID
		// descriptor than messing with a REPORT_COUNT=0 hack.
		if (fp.bit_size == 0)
			return 0;

		if (_first_field_processed) {
			if (_first_field_has_report_id != (_globals.report_id != 0))
				return ERR_BAD_REPORT_ID_ASSIGNMENT;
		}
		else {
			_first_field_has_report_id = _globals.report_id != 0;
			_first_field_processed = true;
		}

		if (!_locals.num_usage_ranges)
			return handler->Padding(rt, _globals.report_id, fp.bit_size);

		if ((_globals.logical_min < 0 && _globals.logical_max < _globals.logical_min) ||
			(_globals.logical_min >= 0 && (uint32_t)_globals.logical_max < (uint32_t)_globals.logical_min))
			return ERR_LOGICAL_MIN_IS_GREATER_THAN_MAX;

		if (!uint16_data(p_data, data_size, fp.flags, true))
			return ERR_UNSPECIFIED;

		// In case of an extended USAGE or USAGE_MIN/MAX the usage_page field is
		// set immediately.
		// A normal USAGE or USAGE_MIN/MAX item leaves the usage_page zero and
		// the parser sets it to the value of _globals.usage_page when a main
		// item is encountered as per specification:
		//
		//   hid1_11.pdf section 6.2.2.8:
		//   When the parser encounters a main item it concatenates the last declared
		//   Usage Page with a Usage to form a complete usage value. Extended usages can
		//   be used to override the currently defined Usage Page for individual usages.
		UsageRange* q = _locals.usage_ranges + _locals.num_usage_ranges;
		for (UsageRange* p = _locals.usage_ranges; p < q; ++p) {
			if (p->usage_page == 0) {
				if (_globals.usage_page == 0)
					return ERR_UNDEFINED_USAGE_PAGE;
				p->usage_page = _globals.usage_page;
			}
		}

		fp.report_type = rt;
		fp.globals = &_globals;
		fp.usage_ranges = _locals.usage_ranges;
		fp.num_usage_ranges = _locals.num_usage_ranges;
		fp.string_ranges = _locals.string_ranges;
		fp.num_string_ranges = _locals.num_string_ranges;

		int res = handler->Field(fp);
		if (res)
			return res;

		return 0;
	}

	int DescriptorParser::ParseGlobalItems(uint8_t item, const uint8_t* p_data, uint8_t data_size) {
		switch (item) {
		case ITEM_USAGE_PAGE:
			if (!uint16_data(p_data, data_size, _globals.usage_page))
				return ERR_INVALID_USAGE_PAGE;
			break;

		case ITEM_LOGICAL_MIN:
			if (!int32_data(p_data, data_size, _globals.logical_min))
				return ERR_UNSPECIFIED;
			break;

		case ITEM_LOGICAL_MAX:
			// This is a workaround for a common mistake: LOGICAL_MAX
			// values that were incorrectly defined as negative numbers
			// because the developers forgot about sign extension.
			// Example: LOGICAL_MIN=0 and LOGICAL_MAX=0xff. If this LOGICAL_MAX
			// value is defined as 1-byte long item data (instead of 2 or 4)
			// then it gets sign extended to (int32_t)0xffffffff which is -1.
			// The correct descriptor would define this value as a 2-byte
			// 0x00ff to avoid sign extension. Our workaround detects this
			// mistake by checking if the LOGICAL_MAX is less than LOGICAL_MIN.
			//
			// This works only when LOGICAL_MIN is defined before LOGICAL_MAX
			// but that always seemed to be the case in real-life HID descriptors.
			if (_globals.logical_min >= 0) {
				if (!uint32_data(p_data, data_size, (uint32_t&)_globals.logical_max))
					return ERR_UNSPECIFIED;
			}
			else {
				if (!int32_data(p_data, data_size, _globals.logical_max))
					return ERR_UNSPECIFIED;
				if (_globals.logical_max < _globals.logical_min) {
					if (!uint32_data(p_data, data_size, (uint32_t&)_globals.logical_max))
						return ERR_UNSPECIFIED;
				}
			}
			break;

#if HRP_ENABLE_PHYISICAL_UNITS
		case ITEM_PHYSICAL_MIN:
			if (!int32_data(p_data, data_size, _globals.physical_min))
				return ERR_UNSPECIFIED;
			break;

		case ITEM_PHYSICAL_MAX:
			if (_globals.physical_min >= 0) {
				if (!uint32_data(p_data, data_size, (uint32_t&)_globals.physical_max))
					return ERR_UNSPECIFIED;
			}
			else {
				if (!int32_data(p_data, data_size, _globals.physical_max))
					return ERR_UNSPECIFIED;
				if (_globals.physical_max < _globals.physical_min) {
					if (!uint32_data(p_data, data_size, (uint32_t&)_globals.physical_max))
						return ERR_UNSPECIFIED;
				}
			}
			break;

		case ITEM_UNIT_EXPONENT:
			if (!uint32_data(p_data, data_size, _globals.unit_exponent))
				return ERR_UNSPECIFIED;
			break;

		case ITEM_UNIT:
			if (!uint32_data(p_data, data_size, _globals.unit))
				return ERR_UNSPECIFIED;
			break;
#endif
		case ITEM_REPORT_SIZE:
			if (!uint32_data(p_data, data_size, _globals.report_size))
				return ERR_UNSPECIFIED;
			if (_globals.report_size > HRP_MAX_REPORT_SIZE)
				return ERR_REPORT_SIZE_TOO_LARGE;
			break;

		case ITEM_REPORT_ID:
			if (!uint8_data(p_data, data_size, _globals.report_id))
				return ERR_INVALID_REPORT_ID;
			if (_globals.report_id == 0)
				return ERR_INVALID_REPORT_ID;
			break;

		case ITEM_REPORT_COUNT:
			if (!uint32_data(p_data, data_size, _globals.report_count))
				return ERR_UNSPECIFIED;
			if (_globals.report_count > HRP_MAX_REPORT_COUNT)
				return ERR_REPORT_COUNT_TOO_LARGE;
			break;

		case ITEM_PUSH:
			if (_globals_stack_size >= HRP_MAX_PUSH_POP_STACK_SIZE)
				return ERR_PUSH_STACK_OVERFLOW;
			_global_stack[_globals_stack_size] = _globals;
			_globals_stack_size++;
			break;

		case ITEM_POP:
			if (_globals_stack_size == 0)
				return ERR_NOTHING_TO_POP;
			_globals_stack_size--;
			_globals = _global_stack[_globals_stack_size];
			break;
		}

		return 0;
	}

	int DescriptorParser::ParseLocalItems(uint8_t item, const uint8_t* p_data, uint8_t data_size) {
		uint16_t usage, usage_page;
        uint8_t stringIndex;

		switch (item) {
		case ITEM_USAGE:
			if (!usage_data(p_data, data_size, usage, usage_page))
				return ERR_UNSPECIFIED;
			if (!_locals.AddUsageRange(usage, usage, usage_page))
				return ERR_TOO_MANY_USAGES;
			break;

		case ITEM_USAGE_MIN:
			if (!usage_data(p_data, data_size, usage, usage_page))
				return ERR_UNSPECIFIED;

			switch (_locals.flags & (Locals::FLAG_USAGE_MIN | Locals::FLAG_USAGE_MAX)) {
			case Locals::FLAG_USAGE_MIN:
#if HRP_IGNORE_LONELY_USAGE_MIN_OR_MAX
			{

				// Overwriting the previous USAGE_MIN that wasn't closed with a USAGE_MAX.
				UsageRange& r = _locals.usage_ranges[_locals.num_usage_ranges - 1];
				r.usage_min = usage;
				r.usage_page = usage_page;
#else
				return ERR_LONELY_USAGE_MIN_MAX;
#endif
				break;
			}
			case Locals::FLAG_USAGE_MAX:
			{
				UsageRange& r = _locals.usage_ranges[_locals.num_usage_ranges - 1];
				if (r.usage_page != usage_page)
					return ERR_EXTENDED_USAGE_MIN_MAX_PAGE_MISMATCH;
				if (usage > r.usage_max)
					return ERR_INVALID_USAGE_MIN_MAX_RANGE;
				r.usage_min = usage;
				_locals.flags &= ~Locals::FLAG_USAGE_MAX;
				break;
			}
			case 0:
				_locals.AddUsageRange(usage, usage, usage_page);
				_locals.flags |= Locals::FLAG_USAGE_MIN;
				break;
			}
			break;

		case ITEM_USAGE_MAX:
			if (!usage_data(p_data, data_size, usage, usage_page))
				return ERR_UNSPECIFIED;

			switch (_locals.flags & (Locals::FLAG_USAGE_MIN | Locals::FLAG_USAGE_MAX)) {
			case Locals::FLAG_USAGE_MAX:
			{
#if HRP_IGNORE_LONELY_USAGE_MIN_OR_MAX
				// Overwriting the previous USAGE_MAX that wasn't closed with a USAGE_MIN.
				UsageRange& r = _locals.usage_ranges[_locals.num_usage_ranges - 1];
				r.usage_max = usage;
				r.usage_page = usage_page;
#else
				return ERR_LONELY_USAGE_MIN_MAX;
#endif
				break;
			}
			case Locals::FLAG_USAGE_MIN:
			{
				UsageRange& r = _locals.usage_ranges[_locals.num_usage_ranges - 1];
				if (r.usage_page != usage_page)
					return ERR_EXTENDED_USAGE_MIN_MAX_PAGE_MISMATCH;
				if (usage < r.usage_min)
					return ERR_INVALID_USAGE_MIN_MAX_RANGE;
				r.usage_max = usage;
				_locals.flags &= ~Locals::FLAG_USAGE_MIN;
				break;
			}
			case 0:
				_locals.AddUsageRange(usage, usage, usage_page);
				_locals.flags |= Locals::FLAG_USAGE_MAX;
				break;
			}
			break;

		case ITEM_STRING_INDEX:
            if (!uint8_data(p_data, data_size, stringIndex))
                return ERR_UNSPECIFIED;
            if (!_locals.AddStringRange(stringIndex, stringIndex))
                return ERR_TOO_MANY_STRING_USAGES;
            break;


         /*
			// unsupported items:
			case ITEM_DESIGNATOR_INDEX:
			case ITEM_DESIGNATOR_MIN:
			case ITEM_DESIGNATOR_MAX:
			case ITEM_STRING_MIN:
			case ITEM_STRING_MAX:
			case ITEM_DELIMITER:
				return 0;
*/
		}

		return 0;
	}


	bool DescriptorParser::Locals::AddUsageRange(uint16_t usage_min, uint16_t usage_max, uint16_t usage_page) {
		// If this range is a continuation of the previously added range
		// then extend the previous range instead of adding a new one.
		if (num_usage_ranges > 0) {
			UsageRange& r = usage_ranges[num_usage_ranges - 1];
			if (r.usage_page == usage_page && r.usage_max + 1 == usage_min) {
				r.usage_max = usage_max;
				return true;
			}
		}

		if (num_usage_ranges >= HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM)
			return false;
		UsageRange& r = usage_ranges[num_usage_ranges++];
		r.usage_min = usage_min;
		r.usage_max = usage_max;
		r.usage_page = usage_page;
		return true;
	}

	bool DescriptorParser::Locals::AddStringRange(uint8_t index_min, uint8_t index_max)
    {
       if (num_string_ranges > 0)
       {
          StringRange& r = string_ranges[num_string_ranges - 1];
          if (r.string_max + 1 == index_min)
          {
             r.string_max = index_max;
             return true;
          }
       }
 
       if (num_string_ranges >= HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM)
          return false;
 
       StringRange& r = string_ranges[num_string_ranges++];
       r.string_min = index_min;
       r.string_max = index_max;
       return true;
    }

	int SelectiveInputReportParser::Init(Collection* input_fields, const void* descriptor, size_t descriptor_size) {
		Reset();
		if (!input_fields || !descriptor || !descriptor_size)
			return ERR_INVALID_PARAMETERS;

		DescriptorMapper m(&_mapping, input_fields);
		int res = m.MapFields(descriptor, descriptor_size);
		if (res) {
			Reset();
			return res;
		}

		if (_mapping.empty())
			return ERR_COULD_NOT_MAP_ANY_USAGES;

		_have_report_ids = _mapping.find(0) == _mapping.end();
		return 0;
	}

	int SelectiveInputReportParser::Parse(const void* report, size_t report_size) {
		if (!report || !report_size)
			return ERR_INVALID_PARAMETERS;
		if (_mapping.empty())
			return ERR_UNINITIALISED_PARSER;

		const uint8_t* r = (const uint8_t*)report;

		uint8_t report_id = 0;
		if (_have_report_ids) {
			report_id = *r++;
			report_size -= 1;
		}

		auto it = _mapping.find(report_id);
		if (it == _mapping.end())
			return ERR_NOTHING_CHANGED;
		ReportMapper& m = it->second;

		if (report_size * 8 != m.bit_size)
			return ERR_INVALID_REPORT_SIZE;

		if (_have_report_ids) {
			for (auto& it : _mapping) {
				if (it.first == report_id)
					continue;
				// Resetting those relative fields that don't belong to this
				// report_id because they won't be updated by this report and
				// and "no change" means zero value in case of a relative field.
				for (ReportFieldMapping& fm : it.second.fields) {
					if (fm.relative)
						ReportFieldMapping::ResetFields(fm);
				}
			}
		}

		for (ReportFieldMapping& fm : m.fields) {
			int res = (fm.variable
				? ReportFieldMapping::ParseVarFields
				: ReportFieldMapping::ParseArrayFields)(fm, r);
			if (res)
				return res;
		}

		return 0;
	}

	void SelectiveInputReportParser::ReportFieldMapping::ResetFields(const ReportFieldMapping& m) {
		for (auto const& it : m.mappings.int32_values) {
			for (const UsageIndexRange& r : it.second)
				memset(&it.first[r.val_min], 0, sizeof(int32_t)*r.length);
		}

		for (auto const& it : m.mappings.bool_values) {
			for (const UsageIndexRange& r : it.second) {
				if (r.length <= 2) {
					for (size_t i = r.val_min, e = r.val_min + r.length; i < e; ++i)
						it.first[i >> 3] &= ~(1 << (i & 7));
				}
				else {
					size_t idx = r.val_min >> 3;
					size_t bits_left = r.length;

					uint8_t first_byte_shift = (uint8_t)(r.val_min & 7);
					if (first_byte_shift) {
						uint8_t bits_in_first_byte = (uint8_t)_hrp_min((size_t)(8 - first_byte_shift), bits_left);
						bits_left -= bits_in_first_byte;
						it.first[idx++] &= ~((((uint8_t)1 << bits_in_first_byte) - 1) << first_byte_shift);
					}

					size_t whole_bytes = bits_left >> 3;
					if (whole_bytes)
						memset(&it.first[idx], 0, whole_bytes);

					uint8_t bits_in_last_byte = bits_left & 7;
					if (bits_in_last_byte)
						it.first[idx+whole_bytes] &= ~(((uint8_t)1 << bits_in_last_byte) - 1);
				}
			}
		}
	}

	static bool IsOutOfRange(int32_t v, int32_t logical_min, int32_t logical_max) {
		return v < logical_min || v > logical_max;
	}

	static bool IsOutOfRange(uint32_t v, int32_t logical_min, int32_t logical_max) {
		return v < (uint32_t)logical_min || v >(uint32_t)logical_max;
	}

	// signed int32_t value
	static void SetInt32Value(int32_t& dest, int32_t v, int32_t logical_min, int32_t logical_max, bool relative) {
		// From the HID specification:
		//   If the host or the device receives an out-of-range value then
		//   the current value for the respective control will not be modified.
		if (IsOutOfRange(v, logical_min, logical_max)) {
			if (relative)
				dest = 0;
		}
		else {
			dest = v;
		}
	}

	// unsigned uint32_t value
	static void SetInt32Value(int32_t& dest, uint32_t v, int32_t logical_min, int32_t logical_max, bool relative) {
		// From the HID specification:
		//   If the host or the device receives an out-of-range value then
		//   the current value for the respective control will not be modified.
		if (IsOutOfRange(v, logical_min, logical_max)) {
			if (relative)
				dest = 0;
		}
		else {
			dest = (int32_t)v;
		}
	}

	int SelectiveInputReportParser::ReportFieldMapping::ParseVarFields(const ReportFieldMapping& m, const uint8_t* report) {

		// IInt32Target

		if (m.byte_aligned) {
			// integer fields are often byte-aligned in HID descriptors
			size_t offset = m.bit_offset >> 3;
			size_t size = m.report_size >> 3;
			if (m.signed_) {
				for (auto const& it : m.mappings.int32_values) {
					for (const UsageIndexRange& r : it.second) {
						for (size_t i=r.val_min,e=r.val_min+r.length,k=offset+r.desc_min*size; i<e; ++i,k+=size) {
							int32_t v;
							switch (size) {
							case 1: v = (int8_t)report[k]; break;
							case 2: v = (int16_t)((int16_t)report[k] | ((int16_t)report[k+1] << 8)); break;
							case 3: v = (int32_t)((int16_t)report[k] | ((int16_t)report[k+1] << 8) | ((int32_t)(int8_t)report[k+2] << 16)); break;
							default: v = (int32_t)((int32_t)report[k] | ((int32_t)report[k+1] << 8) | ((int32_t)report[k+2] << 16) | ((int32_t)report[k+3] << 24)); break;
							}
							SetInt32Value(it.first[i], v, m.logical_min, m.logical_max, m.relative);
						}
					}
				}
			}
			else { // !m.signed_
				for (auto const& it : m.mappings.int32_values) {
					for (const UsageIndexRange& r : it.second) {
						for (size_t i=r.val_min,e=r.val_min+r.length,k=offset+r.desc_min*size; i<e; ++i,k+=size) {
							uint32_t v;
							switch (size) {
							case 1: v = report[k]; break;
							case 2: v = report[k] | ((uint16_t)report[k+1] << 8); break;
							case 3: v = report[k] | ((uint16_t)report[k+1] << 8) | ((uint32_t)report[k+2] << 16); break;
							default: v = report[k] | ((uint16_t)report[k+1] << 8) | ((uint32_t)report[k+2] << 16) | ((uint32_t)report[k+3] << 24); break;
							}
							SetInt32Value(it.first[i], v, m.logical_min, m.logical_max, m.relative);
						}
					}
				}
			}
		}
		else { // !m.byte_aligned

			size_t size = m.report_size;
			size_t limited_size = _hrp_min(size, size_t(32));

			// https://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend
			int32_t mask = m.signed_ ? (uint32_t)1 << (limited_size - 1) : 0;

			for (auto const& it : m.mappings.int32_values) {
				for (const UsageIndexRange& r : it.second) {
					for (size_t i=r.val_min,e=r.val_min+r.length,k=m.bit_offset+r.desc_min*size; i<e; ++i,k+=size) {
						uint8_t shift = (uint8_t)(k & 7);
						size_t idx = k >> 3;

						// n is the number of bytes in the report array that contain our report_size-bits long integer
						// Example: A 10-bit integer may span 2 or 3 bytes. It spans 3 bytes only if the first bit
						//          starts at the most significant bit of the first byte (in which case shift==7).
						// An unaligned 32 bit integer spans 5 bytes but the HID specification clearly states that
						// the maximum span is 4 bytes so 32 bit values can satisfy that only by being byte-aligned.
						uint8_t n = (uint8_t)((shift + limited_size + 7) >> 3);

						uint32_t v = 0;
						switch (n) {
						case 5: v |= (uint32_t)report[idx+4] << (32 - shift);
						case 4: v |= (uint32_t)report[idx+3] << (24 - shift);
						case 3: v |= (uint32_t)report[idx+2] << (16 - shift);
						case 2: v |= (uint16_t)report[idx+1] << (8 - shift);
						case 1: v |= report[idx] >> shift;
						}

						// zero'ing the bits above position 'limited_size'
						v &= ((uint32_t)1 << limited_size) - 1;

						if (m.signed_) {
							// sign-extending the limited_size-bits wide integer
							int32_t sv = (int32_t)((v ^ mask) - mask);
							SetInt32Value(it.first[i], sv, m.logical_min, m.logical_max, m.relative);
						}
						else {
							SetInt32Value(it.first[i], v, m.logical_min, m.logical_max, m.relative);
						}
					}
				}
			}
		}

		// IBoolTarget

		if (m.report_size == 1) {
			// This is a bitfield that can be very long: up to HRP_MAX_REPORT_COUNT bits.
			// A typical example to this is an NKRO gaming keyboard sending 100+
			// keys but many other gaming- and simulation-related devices send
			// the state of similarly high number of buttons in large bitfields.

			// As a 1-bit integer the value of 1 can be interpreted as either
			// 1 or -1 so logical_max can be anything but zero. In practice
			// logical_min and logical_max will be 0 and 1 respectively 99.99%
			// of the time. Anything else is likely to be a pathological case.
			if (m.logical_min <= 0 && m.logical_max != 0) {
				// bit_copy_t must be an unsigned integral type that is at least 16 bits wide.
				typedef size_t bit_copy_t;  // C99 standard: The bit width of size_t is not less than 16.
				static_assert(sizeof(bit_copy_t)>=2 && sizeof(bit_copy_t)<=8, "the width of bit_copy_t must be between 16 and 64 bits");
				static_assert((bit_copy_t)-1 > (bit_copy_t)0, "bit_copy_t must be unsigned");
				// MAX_BLOCK_SIZE is the maximum number of bits we shift and move
				// from the source to the destination in a bit_copy_t instance.
				static constexpr size_t MAX_BLOCK_SIZE = sizeof(bit_copy_t) * 8 - 8;

				for (auto const& it : m.mappings.bool_values) {
					uint8_t* bits = it.first;
					for (const UsageIndexRange& r : it.second) {
						size_t bits_remaining = r.length;
						for (size_t i=r.val_min,k=m.bit_offset+r.desc_min; bits_remaining; i+=MAX_BLOCK_SIZE,k+=MAX_BLOCK_SIZE) {
							size_t block_size = _hrp_min(bits_remaining, MAX_BLOCK_SIZE);
							bits_remaining -= block_size;

							int8_t shift_src = (int8_t)(k & 7);
							int8_t shift_dest = (int8_t)(i & 7);
							// positive shift -> left shift
							// negative shift -> right shift (by -shift number of bits)
							int8_t shift = shift_dest - shift_src;

							bit_copy_t mask = (((bit_copy_t)1 << block_size) - 1) << shift_dest;
							bit_copy_t v = 0;
							{
								// source
								size_t idx = k >> 3;
								uint8_t n = (uint8_t)((shift_src + block_size + 7) >> 3);

								if (shift == 0) {
									switch (n) {
									case 8: v |= (bit_copy_t)report[idx+7] << 56;
									case 7: v |= (bit_copy_t)report[idx+6] << 48;
									case 6: v |= (bit_copy_t)report[idx+5] << 40;
									case 5: v |= (bit_copy_t)report[idx+4] << 32;
									case 4: v |= (bit_copy_t)report[idx+3] << 24;
									case 3: v |= (bit_copy_t)report[idx+2] << 16;
									case 2: v |= (bit_copy_t)report[idx+1] << 8;
									case 1: v |= (bit_copy_t)report[idx];
									}
								}
								else {
									switch (n) {
									case 8: v |= (bit_copy_t)report[idx+7] << (56 + shift);
									case 7: v |= (bit_copy_t)report[idx+6] << (48 + shift);
									case 6: v |= (bit_copy_t)report[idx+5] << (40 + shift);
									case 5: v |= (bit_copy_t)report[idx+4] << (32 + shift);
									case 4: v |= (bit_copy_t)report[idx+3] << (24 + shift);
									case 3: v |= (bit_copy_t)report[idx+2] << (16 + shift);
									case 2: v |= (bit_copy_t)report[idx+1] << (8 + shift);
									case 1: v |= (shift<0) ? ((bit_copy_t)report[idx] >> -shift) : ((bit_copy_t)report[idx] << shift);
									}
								}

								v &= mask;
							}

							{
								// destination
								size_t idx = i >> 3;
								uint8_t n = (uint8_t)((shift_dest + block_size + 7) >> 3);
								mask = ~mask;

								switch (n) {
								case 8: bits[idx+7] = (sizeof(bit_copy_t)!=8 && n>8) ? (uint8_t)(v >> 56) : ((bits[idx+7] & (uint8_t)(mask >> 56)) | (uint8_t)(v >> 56));
								case 7: bits[idx+6] = (sizeof(bit_copy_t)!=7 && n>7) ? (uint8_t)(v >> 48) : ((bits[idx+6] & (uint8_t)(mask >> 48)) | (uint8_t)(v >> 48));
								case 6: bits[idx+5] = (sizeof(bit_copy_t)!=6 && n>6) ? (uint8_t)(v >> 40) : ((bits[idx+5] & (uint8_t)(mask >> 40)) | (uint8_t)(v >> 40));
								case 5: bits[idx+4] = (sizeof(bit_copy_t)!=5 && n>5) ? (uint8_t)(v >> 32) : ((bits[idx+4] & (uint8_t)(mask >> 32)) | (uint8_t)(v >> 32));
								case 4: bits[idx+3] = (sizeof(bit_copy_t)!=4 && n>4) ? (uint8_t)(v >> 24) : ((bits[idx+3] & (uint8_t)(mask >> 24)) | (uint8_t)(v >> 24));
								case 3: bits[idx+2] = (sizeof(bit_copy_t)!=3 && n>3) ? (uint8_t)(v >> 16) : ((bits[idx+2] & (uint8_t)(mask >> 16)) | (uint8_t)(v >> 16));
								case 2: bits[idx+1] = (sizeof(bit_copy_t)!=2 && n>2) ? (uint8_t)(v >> 8)  : ((bits[idx+1] & (uint8_t)(mask >> 8))  | (uint8_t)(v >> 8));
								case 1: bits[idx] = (bits[idx] & (uint8_t)mask) | (uint8_t)v;
								}
							}
						}
					}
				}
			}
		}
		else { // m.report_size > 1

			// This branch maps integer fields of the reports onto bool fields
			// of the application. No one should do this in practice.

			size_t size = m.report_size;
			size_t limited_size = _hrp_min(size, size_t(32));

			int32_t mask = m.signed_ ? (uint32_t)1 << (limited_size - 1) : 0;

			for (auto const& it : m.mappings.bool_values) {
				for (const UsageIndexRange& r : it.second) {
					for (size_t i=r.val_min,e=r.val_min+r.length,k=m.bit_offset+r.desc_min*size; i<e; ++i,k+=size) {
						uint8_t shift = (uint8_t)(k & 7);
						size_t idx = k >> 3;

						uint8_t n = (uint8_t)((shift + limited_size + 7) >> 3);
						uint32_t v = 0;
						switch (n) {
						case 5: v |= (uint32_t)report[idx+4] << (32 - shift);
						case 4: v |= (uint32_t)report[idx+3] << (24 - shift);
						case 3: v |= (uint32_t)report[idx+2] << (16 - shift);
						case 2: v |= (uint16_t)report[idx+1] << (8 - shift);
						case 1: v |= report[idx] >> shift;
						}

						// zero'ing the bits above position 'limited_size'
						v &= ((uint32_t)1 << limited_size) - 1;

						bool out_of_range;
						if (m.signed_) {
							// sign-extending the limited_size-bits wide integer
							int32_t sv = (int32_t)((v ^ mask) - mask);
							out_of_range = IsOutOfRange(sv, m.logical_min, m.logical_max);
						}
						else {
							out_of_range = IsOutOfRange(v, m.logical_min, m.logical_max);
						}

						if (!out_of_range) {
							if (v)
								it.first[i >> 3] |= 1 << (i & 7);
							else
								it.first[i >> 3] &= ~(1 << (i & 7));
						}
					}
				}
			}
		}

		return 0;
	}

	int SelectiveInputReportParser::ReportFieldMapping::ParseArrayFields(const ReportFieldMapping& m, const uint8_t* report) {
		// Zeroing out the bitfields and the rest of the function will set only
		// those bits that are referenced by the integer values found in the array.
		ResetFields(m);

		if (m.byte_aligned) {
			// integer fields are often byte-aligned in HID descriptors
			size_t offset = m.bit_offset >> 3;
			size_t size = m.report_size >> 3;

			for (size_t i=offset,e=offset+m.report_count*size; i<e; i+=size) {
				uint32_t v;
				switch (size) {
				case 1: v = report[i]; break;
				case 2: v = report[i] | ((uint16_t)report[i+1] << 8); break;
				case 3: v = report[i] | ((uint16_t)report[i+1] << 8) | ((uint32_t)report[i+2] << 16); break;
				default: v = report[i] | ((uint16_t)report[i+1] << 8) | ((uint32_t)report[i+2] << 16) | ((uint32_t)report[i+3] << 24); break;
				}

				ProcessArrayItem(m, v);
			}
		}
		else { // !m.byte_aligned
			size_t size = m.report_size;
			size_t limited_size = _hrp_min(size, size_t(32));

			for (size_t i=m.bit_offset,e=m.bit_offset+m.report_count*size; i<e; i+=size) {
				uint8_t shift = (uint8_t)(i & 7);
				size_t idx = i >> 3;

				// n is the number of bytes in the report array that contain our report_size-bits long integer
				// Example: A 10-bit integer may span 2 or 3 bytes. It spans 3 bytes only if the first bit
				//          starts at the most significant bit of the first byte (in which case shift==7).
				// An unaligned 32 bit integer spans 5 bytes but the HID specification clearly states that
				// the maximum span is 4 bytes so 32 bit values can satisfy that only by being byte-aligned.
				uint8_t n = (uint8_t)((shift + limited_size + 7) >> 3);

				uint32_t v = 0;
				switch (n) {
				case 5: v |= (uint32_t)report[idx+4] << (32 - shift);
				case 4: v |= (uint32_t)report[idx+3] << (24 - shift);
				case 3: v |= (uint32_t)report[idx+2] << (16 - shift);
				case 2: v |= (uint16_t)report[idx+1] << (8 - shift);
				case 1: v |= report[idx] >> shift;
				}

				// zero'ing the bits above position 'limited_size'
				v &= ((uint32_t)1 << limited_size) - 1;

				ProcessArrayItem(m, v);
			}
		}
		return 0;
	}

	void SelectiveInputReportParser::ReportFieldMapping::ProcessArrayItem(const ReportFieldMapping& m, uint32_t item) {
		if (IsOutOfRange(item, m.logical_min, m.logical_max))
			return;
		item -= m.logical_min;
		if (item == 0 && m.first_usage_is_zero)
			return;

		// I don't know why anyone would use IInt32Target to store bool values
		// but we provide the implementation for the sake of completeness...
		for (auto const& it : m.mappings.int32_values) {
			for (const UsageIndexRange& r : it.second) {
				if (item >= r.desc_min && item < r.desc_min + r.length) {
					it.first[r.val_min + item - r.desc_min] = 1;
					return;
				}
			}
		}

		for (auto const& it : m.mappings.bool_values) {
			for (const UsageIndexRange& r : it.second) {
				if (item >= r.desc_min && item < r.desc_min + r.length) {
					size_t idx = r.val_min + item - r.desc_min;
					it.first[idx >> 3] |= (uint8_t)(1 << (idx & 7));
					return;
				}
			}
		}
	}

	int SelectiveInputReportParser::DescriptorMapper::MapFields(const void* descriptor, size_t descriptor_size) {
		ResizeVectors(_root);

		DescriptorParser dp;
		int res = dp.Parse(descriptor, descriptor_size, this);
		if (res)
			return res;

		for (auto it=_mapping->begin(),eit=_mapping->end(); it!=eit;) {
			// _mapping may contain empty entries because it is used not only to
			// store mappings but also to track the bit positions inside the
			// report structs while parsing the descriptor.
			if (it->second.fields.empty()) {
				_mapping->erase(it++);
				continue;
			}

			// rounding up the bit_size to byte boundary
			it->second.bit_size = (it->second.bit_size + 7) & ~7;
			++it;
		}

		return 0;
	}

	void SelectiveInputReportParser::DescriptorMapper::ResizeVectors(Collection* c) {
		for (Int32Fields* f : c->int32s) {
			size_t n = CountUsages(f->usages.data(), f->usages.size());
			f->mapped.assign(n, false);
			f->properties.assign(n, {});
			if (f->target)
				f->target->Reset(n);
		}
		for (BoolFields* f : c->bools) {
			size_t n = CountUsages(f->usages.data(), f->usages.size());
			f->mapped.assign(n, false);
			if (f->target)
				f->target->Reset(n);
		}
		for (Collection* child : c->collections)
			ResizeVectors(child);
	}

	int SelectiveInputReportParser::DescriptorMapper::Field(const DescriptorParser::FieldParams& fp) {
		if (fp.report_type != ReportType::input)
			return 0;

		DescFieldMappings dfm;

		if (_matched.empty()) {
			// If the root collection doesn't have type and usages defined
			// then its fields can match even those descriptor fields that
			// are defined outside of descriptor collections. Descriptor
			// fields like that aren't allowed by the HID specification and
			// real-world USB devices are unlikely to define them but you
			// can never know...
			// The HID driver in Linux can parse and accept them.
			if (!_root->type && _root->usages.empty()) {
				int32_t res = FindFieldUsagesInCollection(_root, fp, dfm);
				if (res < 0)
					return (int)res;
			}
		}
		else {
			std::vector<bool> matched_usage_indexes;
			if (fp.flags & FLAG_FIELD_VARIABLE)
				matched_usage_indexes.resize(fp.globals->report_count);
			else
				matched_usage_indexes.resize(_hrp_min((uint32_t)CountUsages(fp.usage_ranges, fp.num_usage_ranges), (uint32_t)HRP_MAX_REPORT_COUNT));

			uint32_t num_usage_indexes_left = (uint32_t)matched_usage_indexes.size();

			for (auto it = _matched.rbegin(), eit = _matched.rend(); it != eit; ++it) {
				int32_t res = FindFieldUsagesInCollection(*it, fp, dfm, &matched_usage_indexes);
				if (res < 0)
					return (int)res;
				assert((uint32_t)res <= num_usage_indexes_left);
				num_usage_indexes_left -= (uint32_t)res;
				if (num_usage_indexes_left == 0)
					break;
			}
		}

		ReportMapper& rm = (*_mapping)[fp.globals->report_id];

		if (!dfm.int32_values.empty() || !dfm.bool_values.empty()) {
			(*_mapping)[fp.globals->report_id].fields.push_back({
				std::move(dfm),
				rm.bit_size,
				fp.globals->report_size,
				fp.globals->report_count,
				fp.globals->logical_min,
				fp.globals->logical_max,
				(fp.flags & FLAG_FIELD_VARIABLE) != 0,
				(fp.flags & FLAG_FIELD_RELATIVE) != 0,
				fp.globals->logical_min < 0,
				fp.usage_ranges[0].usage_min == 0,
				(rm.bit_size & 7) == 0 && (fp.globals->report_size & 7) == 0,
			});
		}

		rm.bit_size += fp.bit_size;
		return 0;
	}

	int SelectiveInputReportParser::DescriptorMapper::Padding(ReportType rt, uint8_t report_id, uint32_t bit_size) {
		if (rt != ReportType::input)
			return 0;
		ReportMapper& rm = (*_mapping)[report_id];
		rm.bit_size += bit_size;
		return 0;
	}

	int SelectiveInputReportParser::DescriptorMapper::BeginCollection(uint8_t collection_type, uint16_t usage_page, uint16_t usage, uint32_t depth) {
		_prev_matched_size.push_back(_matched.size());
		if (_matched.empty()) {
			RecursiveAdd(_root, collection_type, usage_page, usage);
			return 0;
		}
		for (auto it = _matched.rbegin(), eit = _matched.rend(); it != eit; ++it)
			RecursiveAddChildren(*it, collection_type, usage_page, usage);
		return 0;
	}

	int SelectiveInputReportParser::DescriptorMapper::EndCollection(uint32_t depth) {
		size_t prev_size = _prev_matched_size.back();
		_prev_matched_size.pop_back();
		for (size_t i = prev_size, e = _matched.size(); i < e; ++i)
			_matched_set.erase(_matched[i]);
		_matched.resize(prev_size);
		return 0;
	}

	void SelectiveInputReportParser::DescriptorMapper::RecursiveAdd(Collection* c, uint8_t collection_type, uint16_t usage_page, uint16_t usage) {
		if (_matched_set.find(c) != _matched_set.end())
			return;
		if (!CollectionMatch(c, collection_type, usage_page, usage))
			return;
		_matched.push_back(c);
		_matched_set.insert(c);
		RecursiveAddChildren(c, collection_type, usage_page, usage);
	}

	void SelectiveInputReportParser::DescriptorMapper::RecursiveAddChildren(Collection* c, uint8_t collection_type, uint16_t usage_page, uint16_t usage) {
		for (Collection* c : c->collections)
			RecursiveAdd(c, collection_type, usage_page, usage);
	}

	bool SelectiveInputReportParser::DescriptorMapper::CollectionMatch(Collection* c, uint8_t collection_type, uint16_t usage_page, uint16_t usage) {
		if (c->type && c->type != collection_type)
			return false;
		if (!c->usages.empty() && !UsagesMatch(c->usages, usage_page, usage))
			return false;
		return true;
	}

	bool SelectiveInputReportParser::DescriptorMapper::UsagesMatch(const std::vector<UsageRange>& usages, uint16_t usage_page, uint16_t usage) {
		for (const UsageRange& ur : usages) {
			if (UsageMatch(ur, usage_page, usage))
				return true;
		}
		return false;
	}

	bool SelectiveInputReportParser::DescriptorMapper::UsageMatch(const UsageRange& ur, uint16_t usage_page, uint16_t usage) {
		if (ur.usage_page != usage_page)
			return false;
		uint16_t usage_max = ur.usage_max < ur.usage_min ? ur.usage_min : ur.usage_max;
		return usage >= ur.usage_min && usage <= usage_max;
	}

	size_t SelectiveInputReportParser::DescriptorMapper::CountUsages(const UsageRange* usage_ranges, size_t num_usage_ranges) {
		size_t num_usages = 0;
		const UsageRange* ur_end = usage_ranges + num_usage_ranges;
		for (const UsageRange* ur = usage_ranges; ur < ur_end; ++ur) {
			uint16_t usage_max = ur->usage_max < ur->usage_min ? ur->usage_min : ur->usage_max;
			num_usages += (size_t)usage_max - (size_t)ur->usage_min + 1;
		}
		return num_usages;
	}

	void SelectiveInputReportParser::DescriptorMapper::AddFieldIndexes(usage_to_field_index& m, size_t field_index, const std::vector<UsageRange>& usages) {
		size_t index = 0;
		for (const UsageRange& ur : usages) {
			uint32_t page32 = (uint32_t)ur.usage_page << 16;
			uint16_t usage_max = ur.usage_max < ur.usage_min ? ur.usage_min : ur.usage_max;
			for (uint32_t u = ur.usage_min; u <= usage_max; ++u, ++index)
				m.insert(usage_to_field_index::value_type(page32 | u, { field_index, index }));
		}
	}

	SelectiveInputReportParser::DescriptorMapper::FieldIndexes& SelectiveInputReportParser::DescriptorMapper::GetFieldIndexes(Collection* c) {
		auto it = _collection_field_indexes.find(c);
		if (it != _collection_field_indexes.end())
			return it->second;

		FieldIndexes fi;
		for (size_t i = 0, e = c->int32s.size(); i < e; ++i)
			AddFieldIndexes(fi.int32_indexes, i, c->int32s[i]->usages);
		for (size_t i = 0, e = c->bools.size(); i < e; ++i)
			AddFieldIndexes(fi.bool_indexes, i, c->bools[i]->usages);

		return _collection_field_indexes[c] = std::move(fi);
	}

	bool SelectiveInputReportParser::DescriptorMapper::TestInt32FieldFlags(Collection* c, const FieldIndex& fi, uint16_t flags) {
		Int32Fields* f = c->int32s[fi.field_index];
		return (flags & f->mask) == f->flags;
	}

	bool SelectiveInputReportParser::DescriptorMapper::TestBoolFieldFlags(Collection* c, const FieldIndex& fi, uint16_t flags) {
		BoolFields* f = c->bools[fi.field_index];
		return (flags & f->mask) == f->flags;
	}

	class UsageRangesIterator {
	public:
		UsageRangesIterator(UsageRange* usage_ranges, uint16_t num_usage_ranges) {
			assert(num_usage_ranges > 0);
			_p = usage_ranges;
			_last = usage_ranges + num_usage_ranges - 1;
			_page32 = (uint32_t)_p->usage_page << 16;
			_u = _p->usage_min;
			_processed_all = false;
		}

		void Next() {
			if (_u < _p->usage_max) {
				_u++;
				return;
			}
			if (_p == _last) {
				_processed_all = true;
				return;
			}
			_p++;
			_page32 = (uint32_t)_p->usage_page << 16;
			_u = _p->usage_min;
		}

		uint32_t Usage32() const { return _page32 | (uint32_t)_u; }
		bool ProcessedAll() const { return _processed_all; }

	private:
		uint32_t _page32;
		uint16_t _u;
		UsageRange* _p;
		UsageRange* _last;
		bool _processed_all;
	};

	// Returns the number of found/mapped usage indexes or a negative error code.
	int32_t SelectiveInputReportParser::DescriptorMapper::FindFieldUsagesInCollection(
		Collection* c, const DescriptorParser::FieldParams& fp,
		DescFieldMappings& dfm, std::vector<bool>* matched_usage_indexes) {
		FieldIndexes& fi = GetFieldIndexes(c);

		// In case of an array field we have to iterate through all declared usages.
		//
		// In case of a var field we have report_count number of usage indexes
		// despite the fact that the number of declared usages can be lower or
		// higher than report_count. If we have more usages (which is a soft
		// error in the descriptor) then we process only the first report_count
		// number of usages. If we have less than report_count number of usages
		// then the last declared usage is used for the remaining usage indexes
		// as per HID specification.

		// number of usage indexes matched
		int32_t found = 0;

		UsageRangesIterator ur(fp.usage_ranges, fp.num_usage_ranges);

		// index is the usage_index of the descriptor field
		for (size_t index=0; index<HRP_MAX_REPORT_COUNT; index++,ur.Next()) {
			if ((fp.flags & FLAG_FIELD_VARIABLE)) {
				if (index >= fp.globals->report_count)
					break;
			}
			else if (ur.ProcessedAll()) {
				// processed all declared usages for the array field
				break;
			}

			if (matched_usage_indexes && (*matched_usage_indexes)[index])
				continue;

			uint32_t usage32 = ur.Usage32();

			auto it = fi.int32_indexes.find(usage32);
			if (it != fi.int32_indexes.end() && TestInt32FieldFlags(c, it->second, fp.flags)) {
				//HRP_DEBUGF("%08x %d->i%d.%d %x\n", usage32, (int)index, (int)it->second.field_index, (int)it->second.usage_index, (int)(int64_t)c);

				Int32Fields& i32 = *c->int32s[it->second.field_index];
				if (i32.target)
					dfm.AddMapping(i32.target->Data(), index, it->second.usage_index);
				i32.mapped[it->second.usage_index] = true;

				Int32Fields::FieldProperties& props = i32.properties[it->second.usage_index];
				props.flags = fp.flags;
				props.logical_min = fp.globals->logical_min;
				props.logical_max = fp.globals->logical_max;
#if HRP_ENABLE_PHYISICAL_UNITS
				props.physical_min = fp.globals->physical_min;
				props.physical_max = fp.globals->physical_max;
				props.unit = fp.globals->unit;
				props.unit_exponent = fp.globals->unit_exponent;
#endif
				found++;
				if (matched_usage_indexes)
					(*matched_usage_indexes)[index] = true;

				fi.int32_indexes.erase(it);
			}
			else {
				auto it2 = fi.bool_indexes.find(usage32);
				if (it2 != fi.bool_indexes.end() && TestBoolFieldFlags(c, it2->second, fp.flags)) {
					//HRP_DEBUGF("%08x %d->b%d.%d %x\n", usage32, (int)index, (int)it2->second.field_index, (int)it2->second.usage_index, (int)(int64_t)c);

					BoolFields& b = *c->bools[it2->second.field_index];
					if (b.target)
						dfm.AddMapping(b.target->Data(), index, it2->second.usage_index);
					b.mapped[it2->second.usage_index] = true;

					found++;
					if (matched_usage_indexes)
						(*matched_usage_indexes)[index] = true;
					fi.bool_indexes.erase(it2);
				}
				else {
					//HRP_DEBUGF("%08x - unmatched usage32\n", usage32);
				}
			}
		}

		return found;
	}

	uint8_t detect_common_input_device_type(const void* desc, size_t desc_size) {
		uint8_t types;
		CommonInputDeviceTypeDetector d;
		if (0 == d.Detect(desc, desc_size, types))
			return types;
		return 0;
	}


} // namespace hidrp
