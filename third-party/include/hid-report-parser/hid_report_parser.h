// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText:  2022 Istvan Pasztor
// https://github.com/pasztorpisti/hid-report-parser/tree/master (with private VPX modifications)

#pragma once

#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <vector>
#include <map>
#include <set>


// The accepted max value of a REPORT_SIZE item in the descriptor.
// The Linux HID driver uses a very high number (0x100) for some reason.
//
// The widest data type you can extract from an input report with this library
// is int32_t (REPORT_SIZE=0x20) but the descriptor parser won't return an error
// if it encounters a larger REPORT_SIZE value that isn't greater than
// HRP_MAX_REPORT_SIZE. I guess those high REPORT_SIZE values are needed
// for less frequently used features like FLAG_FIELD_BUFFERED_BYTES.
#ifndef HRP_MAX_REPORT_SIZE
#  define HRP_MAX_REPORT_SIZE 0x100
#endif

// The accepted max value of a REPORT_COUNT item in the descriptor.
// The Linux HID driver uses a fairly high number: 0x3000.
//
// NKRO gaming keyboards and other gaming- and simulation-related devices may
// send a large number of keys/buttons in a single bitfield. For example the
// keys of my current keyboard are sent in a bitfield with REPORT_COUNT=0xE8.
//
// The report parser in this library deals only with INPUT reports but the
// descriptor parser runs through and validates OUTPUT and FEATURE items too.
// For this reason the HRP_MAX_REPORT_COUNT has to consider OUTPUT and FEATURE
// too that sometimes use very high REPORT_COUNT values.
// As an example: one of my mice has a FEATURE field with REPORT_COUNT=1144.
#ifndef HRP_MAX_REPORT_COUNT
#  define HRP_MAX_REPORT_COUNT 0x3000
#endif

// The maximum number of USAGE_MIN/USAGE_MAX pairs per main item (INPUT/OUTPUT/FEATURE).
// A normal USAGE item is also counted as a range (with MIN and MAX set to the same value).
// Most input devices have HID descriptors that can be parsed with a very low
// HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM value (5 or less). However, some devices
// may require a significantly higher value. Increasing the value of
// HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM adds at least 6 bytes to the size of the
// DescriptorParser class (which is the total memory usage of the descriptor parser).
#ifndef HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM
#  define HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM 0x100
#endif

// Maximum stack size for the PUSH/POP global items that save/restore the globals.
// I have quite a few input devices and none of their HID descriptors use PUSH/POP,
// it seems to be a rarely used feature. The Linux kernel also uses a stack size of 4.
#ifndef HRP_MAX_PUSH_POP_STACK_SIZE
#  define HRP_MAX_PUSH_POP_STACK_SIZE 4
#endif

// Turns on/off the support for the PHYSICAL_MIN/PHSYSICAL_MAX/UNIT/UNIT_EXPONENT
// fields in the descriptor parser. Most applications ignore these fields.
#ifndef HRP_ENABLE_PHYISICAL_UNITS
#  define HRP_ENABLE_PHYISICAL_UNITS 0
#endif

// IGNORE_LONELY_USAGE_MIN_OR_MAX=1 ignores unmatched USAGE_MIN and USAGE_MAX
// items instead of stopping with a parser error.
#ifndef HRP_IGNORE_LONELY_USAGE_MIN_OR_MAX
#  define HRP_IGNORE_LONELY_USAGE_MIN_OR_MAX 1
#endif

#ifndef HRP_DEBUG_PRINTF_ENABLED
#  define HRP_DEBUG_PRINTF_ENABLED 0
#endif

#if HRP_DEBUG_PRINTF_ENABLED
#  define HRP_DEBUGF(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#  define HRP_DEBUGF(fmt, ...)
#endif


namespace hidrp {

	// Error codes returned by the library:
	// Zero -> success.
	// Nonzero -> failure.

	// Returns default_str if the error_code isn't recognised.
	const char* str_error(int error_code, const char* default_str=nullptr);

	static constexpr int ERR_SUCCESS = 0;
	// ERR_UNSPECIFIED is usually returned in places where an error should
	// never normally occur unless it is caused by something unforeseen.
	static constexpr int ERR_UNSPECIFIED = -1;
	// Reached the end of the report descriptor before fully parsing an item.
	static constexpr int ERR_INCOMPLETE_ITEM = -2;
	// Parsed the "reserved" value instead of main, global or local.
	static constexpr int ERR_INVALID_ITEM_TYPE = -3;
	static constexpr int ERR_UNCLOSED_COLLECTION = -4;
	static constexpr int ERR_PUSH_WITHOUT_POP = -5;
	// The collection type is greater than 255.
	static constexpr int ERR_INVALID_COLLECTION_TYPE = -6;
	// Found an END_COLLECTION item without a matching BEGIN_COLLECTION.
	static constexpr int ERR_NO_COLLECTION_TO_CLOSE = -7;
	// USAGE_MIN without a matching USAGE_MAX or USAGE_MAX without a matching USAGE_MIN.
	// This error can be returned only when HRP_IGNORE_LONELY_USAGE_MIN_OR_MAX is nonzero.
	static constexpr int ERR_LONELY_USAGE_MIN_MAX = -8;
	// You can't declare only some fields with (non-zero) report IDs.
	// You have to either declare all fields with report IDs or declare all of
	// them without report IDs.
	static constexpr int ERR_BAD_REPORT_ID_ASSIGNMENT = -9;
	static constexpr int ERR_LOGICAL_MIN_IS_GREATER_THAN_MAX = -10;
	// The usage page can't fit into an uint16_t.
	static constexpr int ERR_INVALID_USAGE_PAGE = -11;
	// The REPORT_SIZE exceeds HRP_MAX_REPORT_SIZE.
	static constexpr int ERR_REPORT_SIZE_TOO_LARGE = -12;
	// The REPORT_COUNT exceeds HRP_MAX_REPORT_COUNT.
	static constexpr int ERR_REPORT_COUNT_TOO_LARGE = -13;
	// The REPORT_ID is zero or greater than 255.
	static constexpr int ERR_INVALID_REPORT_ID = -14;
	// The number of unpopped PUSHes exceeds HRP_MAX_PUSH_POP_STACK_SIZE.
	static constexpr int ERR_PUSH_STACK_OVERFLOW = -15;
	// Encountered a POP instruction with an empty stack.
	// POP without a matching PUSH.
	static constexpr int ERR_NOTHING_TO_POP = -16;
	// The number of declared usage ranges exceeds HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM.
	static constexpr int ERR_TOO_MANY_USAGES = -17;
	// Encountered an extended USAGE_MIN/USAGE_MAX pair with different usage page parameters.
	static constexpr int ERR_EXTENDED_USAGE_MIN_MAX_PAGE_MISMATCH = -18;
	// USAGE_MIN is greater than USAGE_MAX.
	static constexpr int ERR_INVALID_USAGE_MIN_MAX_RANGE = -19;
	// Invalid function parameters.
	static constexpr int ERR_INVALID_PARAMETERS = -20;
	// Returned by the SelectiveInputReportParser::Init method to indicate that
	// none of the descriptor fields could be mapped.
	static constexpr int ERR_COULD_NOT_MAP_ANY_USAGES = -21;
	// Trying to call SelectiveInputReportParser::Parse on a parser that hasn't
	// been initialised by a SelectiveInputReportParser::Init call.
	static constexpr int ERR_UNINITIALISED_PARSER = -22;
	// Returned by SelectiveInputReportParser::Parse to indicate that the report
	// contains no fields the application is interested in.
	// This can easily happen in case of a device that uses report IDs. Imagine
	// a mouse that presents itself as a mouse and also as a keyboard in its
	// HID descriptor to be able to simulate keyboard macros. A device like that
	// would send the mouse and keyboard inputs in separate reports under two
	// different report IDs. If your application is interested in mouse input
	// and the device sends a report with keyboard changes then the
	// SelectiveInputReportParser::Parse method can find no useful information
	// in the report and returns ERR_NOTHING_CHANGED.
	// In a situation like that the int32 and bool variables are left unchanged
	// so the application should not waste time on trying to look for changes.
	// That could result even in buggy behaviour for example in case of relative
	// fields like the delta X/Y movement of a mouse because processing the
	// previously received delta values again would be an error.
	static constexpr int ERR_NOTHING_CHANGED = -23;
	// The size of the report is different from the size specified in the report
	// descriptor.
	static constexpr int ERR_INVALID_REPORT_SIZE = -24;
	// This error indicates that the descriptor specified at least one USAGE but
	// failed to specify a USAGE_PAGE before reaching an INPUT, OUTPUT or
	// FEATURE item.
	static constexpr int ERR_UNDEFINED_USAGE_PAGE = -25;
	// Too many string usages
    static constexpr int ERR_TOO_MANY_STRING_USAGES = -26;


	// Usage page and usage ID constants copied from hut1_5.pdf:
	// Below you can find all of the usage page constants and some of the most
	// frequently utilised usage IDs. Consult the latest HID usage tables PDF
	// if you need something that isn't listed here.

	// HID usage pages:

	static constexpr uint16_t PAGE_GENERIC_DESKTOP = 0x01;
	static constexpr uint16_t PAGE_SIMULATION_CONTROLS = 0x02;
	static constexpr uint16_t PAGE_VR_CONTROLS = 0x03;
	static constexpr uint16_t PAGE_SPORT_CONTROLS = 0x04;
	static constexpr uint16_t PAGE_GAME_CONTROLS = 0x05;
	static constexpr uint16_t PAGE_GENERIC_DEVICE_CONTROLS = 0x06;
	static constexpr uint16_t PAGE_KEYBOARD = 0x07;
	static constexpr uint16_t PAGE_LED = 0x08;
	static constexpr uint16_t PAGE_BUTTON = 0x09;
	static constexpr uint16_t PAGE_ORDINAL = 0x0A;
	static constexpr uint16_t PAGE_TELEPHONY_DEVICE = 0x0B;
	static constexpr uint16_t PAGE_CONSUMER = 0x0C;
	static constexpr uint16_t PAGE_DIGITIZERS = 0x0D;
	static constexpr uint16_t PAGE_HAPTICS = 0x0E;
	static constexpr uint16_t PAGE_PHYSICAL_INPUT_DEVICE = 0x0F;
	static constexpr uint16_t PAGE_UNICODE = 0x10;
	static constexpr uint16_t PAGE_SOC = 0x11;
	static constexpr uint16_t PAGE_EYE_AND_HEAD_TRACKERS = 0x12;
	static constexpr uint16_t PAGE_AUXILIARY_DISPLAY = 0x14;
	static constexpr uint16_t PAGE_SENSORS = 0x20;
	static constexpr uint16_t PAGE_MEDICAL_INSTRUMENT = 0x40;
	static constexpr uint16_t PAGE_BRAILLE_DISPLAY = 0x41;
	static constexpr uint16_t PAGE_LIGHTING_AND_ILLUMINATION = 0x59;
	static constexpr uint16_t PAGE_MONITOR = 0x80;
	static constexpr uint16_t PAGE_MONITOR_ENUMERATED = 0x81;
	static constexpr uint16_t PAGE_VESA_VIRTUAL_CONTROLS = 0x82;
	static constexpr uint16_t PAGE_POWER = 0x84;
	static constexpr uint16_t PAGE_BATTERY_SYSTEM = 0x85;
	static constexpr uint16_t PAGE_BARCODE_SCANNER = 0x8C;
	static constexpr uint16_t PAGE_SCALES = 0x8D;
	static constexpr uint16_t PAGE_MAGNETIC_STRIPE_READER = 0x8E;
	static constexpr uint16_t PAGE_CAMERA_CONTROL = 0x90;
	static constexpr uint16_t PAGE_ARCADE = 0x91;
	static constexpr uint16_t PAGE_GAMING_DEVICE = 0x92;
	static constexpr uint16_t PAGE_FIDO_ALLIANCE = 0xF1D0;
	static constexpr uint16_t PAGE_VENDOR_DEFINED_MIN = 0xFF00;
	static constexpr uint16_t PAGE_VENDOR_DEFINED_MAX = 0xFFFF;

	// Generic Desktop Page (0x01)

	static constexpr uint16_t USAGE_POINTER = 0x01; // Collection(Physical)
	static constexpr uint16_t USAGE_MOUSE = 0x02; // Collection(Application)
	static constexpr uint16_t USAGE_JOYSTICK = 0x04; // Collection(Application)
	static constexpr uint16_t USAGE_GAMEPAD = 0x05; // Collection(Application)
	static constexpr uint16_t USAGE_KEYBOARD = 0x06; // Collection(Application)
	static constexpr uint16_t USAGE_KEYPAD = 0x07; // Collection(Application)
	static constexpr uint16_t USAGE_MULTI_AXIS_CONTROLLER = 0x08; // Collection(Application)
	static constexpr uint16_t USAGE_TABLET_PC_SYSTEM_CONTROLS = 0x09; // Collection(Application)
	static constexpr uint16_t USAGE_WATER_COOLING_DEVICE = 0x0A; // Collection(Application)
	static constexpr uint16_t USAGE_COMPUTER_CHASSIS_DEVICE = 0x0B; // Collection(Application)
	static constexpr uint16_t USAGE_WIRELESS_RADIO_CONTROLS = 0x0C; // Collection(Application)
	static constexpr uint16_t USAGE_PORTABLE_DEVICE_CONTROL = 0x0D; // Collection(Application)
	static constexpr uint16_t USAGE_SYSTEM_MULTI_AXIS_CONTROLLER = 0x0E; // Collection(Application)
	static constexpr uint16_t USAGE_SPATIAL_CONTROLLER = 0x0F; // Collection(Application)
	static constexpr uint16_t USAGE_ASSISTIVE_CONTROL = 0x10; // Collection(Application)
	static constexpr uint16_t USAGE_DEVICE_DOCK = 0x11; // Collection(Application)
	static constexpr uint16_t USAGE_DOCKABLE_DEVICE = 0x12; // Collection(Application)
	static constexpr uint16_t USAGE_CALL_STATE_MANAGEMENT_CONTROL = 0x13; // Collection(Application)
	static constexpr uint16_t USAGE_X = 0x30;
	static constexpr uint16_t USAGE_Y = 0x31;
	static constexpr uint16_t USAGE_Z = 0x32;
	static constexpr uint16_t USAGE_RX = 0x33;
	static constexpr uint16_t USAGE_RY = 0x34;
	static constexpr uint16_t USAGE_RZ = 0x35;
	static constexpr uint16_t USAGE_SLIDER = 0x36;
	static constexpr uint16_t USAGE_DIAL = 0x37;
	static constexpr uint16_t USAGE_WHEEL = 0x38;
	static constexpr uint16_t USAGE_HAT_SWITCH = 0x39;
	static constexpr uint16_t USAGE_COUNTED_BUFFER = 0x3A;
	static constexpr uint16_t USAGE_BYTE_COUNT = 0x3B;
	static constexpr uint16_t USAGE_MOTION_WAKEUP = 0x3C;
	static constexpr uint16_t USAGE_START = 0x3D;
	static constexpr uint16_t USAGE_SELECT = 0x3E;
	static constexpr uint16_t USAGE_VX = 0x40;
	static constexpr uint16_t USAGE_VY = 0x41;
	static constexpr uint16_t USAGE_VZ = 0x42;
	static constexpr uint16_t USAGE_VBRX = 0x43;
	static constexpr uint16_t USAGE_VBRY = 0x44;
	static constexpr uint16_t USAGE_VBRZ = 0x45;
	static constexpr uint16_t USAGE_VNO = 0x46;
	static constexpr uint16_t USAGE_FEATURE_NOTIFICATION = 0x47;
	static constexpr uint16_t USAGE_RESOLUTION_MULTIPLIER = 0x48;
	static constexpr uint16_t USAGE_QX = 0x49;
	static constexpr uint16_t USAGE_QY = 0x4A;
	static constexpr uint16_t USAGE_QZ = 0x4B;
	static constexpr uint16_t USAGE_QW = 0x4C;
	static constexpr uint16_t USAGE_SYSTEM_CONTROL = 0x80;
	static constexpr uint16_t USAGE_SYSTEM_POWER_DOWN = 0x81;
	static constexpr uint16_t USAGE_SYSTEM_SLEEP = 0x82;
	static constexpr uint16_t USAGE_SYSTEM_WAKEUP = 0x83;
	static constexpr uint16_t USAGE_SYSTEM_CONTEXT_MENU = 0x84;
	static constexpr uint16_t USAGE_SYSTEM_MAIN_MENU = 0x85;
	static constexpr uint16_t USAGE_SYSTEM_APP_MENU = 0x86;
	static constexpr uint16_t USAGE_SYSTEM_MENU_HELP = 0x87;
	static constexpr uint16_t USAGE_SYSTEM_MENU_EXIT = 0x88;
	static constexpr uint16_t USAGE_SYSTEM_MENU_SELECT = 0x89;
	static constexpr uint16_t USAGE_SYSTEM_MENU_RIGHT = 0x8A;
	static constexpr uint16_t USAGE_SYSTEM_MENU_LEFT = 0x8B;
	static constexpr uint16_t USAGE_SYSTEM_MENU_UP = 0x8C;
	static constexpr uint16_t USAGE_SYSTEM_MENU_DOWN = 0x8D;
	static constexpr uint16_t USAGE_SYSTEM_COLD_RESTART = 0x8E;
	static constexpr uint16_t USAGE_SYSTEM_WARM_RESTART = 0x8F;
	static constexpr uint16_t USAGE_DPAD_UP = 0x90;
	static constexpr uint16_t USAGE_DPAD_DOWN = 0x91;
	static constexpr uint16_t USAGE_DPAD_RIGHT = 0x92;
	static constexpr uint16_t USAGE_DPAD_LEFT = 0x93;
	static constexpr uint16_t USAGE_INDEX_TRIGGER = 0x94;
	static constexpr uint16_t USAGE_PALM_TRIGGER = 0x95;
	static constexpr uint16_t USAGE_THUMBSTICK = 0x96;
	static constexpr uint16_t USAGE_SYSTEM_FUNCTION_SHIFT = 0x97;
	static constexpr uint16_t USAGE_SYSTEM_FUNCTION_SHIFT_LOCK = 0x98;
	static constexpr uint16_t USAGE_SYSTEM_FUNCTION_SHIFT_LOCK_INDICATOR = 0x99;
	static constexpr uint16_t USAGE_SYSTEM_DISMISS_NOTIFICATION = 0x9A;
	static constexpr uint16_t USAGE_SYSTEM_DO_NOT_DISTURB = 0x9B;

	// Simulation Controls Page (0x02)

	static constexpr uint16_t USAGE_FLIGHT_SIMULATION_DEVICE = 0x01; // Collection(Application)
	static constexpr uint16_t USAGE_AUTOMOBILE_SIMULATION_DEVICE = 0x02; // Collection(Application)
	static constexpr uint16_t USAGE_TANK_SIMULATION_DEVICE = 0x03; // Collection(Application)
	static constexpr uint16_t USAGE_SPACESHIP_SIMULATION_DEVICE = 0x04; // Collection(Application)
	static constexpr uint16_t USAGE_SUBMARINE_SIMULATION_DEVICE = 0x05; // Collection(Application)
	static constexpr uint16_t USAGE_SAILING_SIMULATION_DEVICE = 0x06; // Collection(Application)
	static constexpr uint16_t USAGE_MOTORCYCLE_SIMULATION_DEVICE = 0x07; // Collection(Application)
	static constexpr uint16_t USAGE_SPORTS_SIMULATION_DEVICE = 0x08; // Collection(Application)
	static constexpr uint16_t USAGE_AIRPLANE_SIMULATION_DEVICE = 0x09; // Collection(Application)
	static constexpr uint16_t USAGE_HELICOPTER_SIMULATION_DEVICE = 0x0A; // Collection(Application)
	static constexpr uint16_t USAGE_MAGIC_CARPET_SIMULATION_DEVICE = 0x0B; // Collection(Application)
	static constexpr uint16_t USAGE_BICYCLE_SIMULATION_DEVICE = 0x0C; // Collection(Application)
	static constexpr uint16_t USAGE_SIMULATION_FLIGHT_CONTROL_STICK = 0x20; // Collection(Application)
	static constexpr uint16_t USAGE_SIMULATION_FLIGHT_STICK = 0x21; // Collection(Application)
	static constexpr uint16_t USAGE_SIMULATION_CYCLIC_CONTROL = 0x22; // Collection(Physical)
	static constexpr uint16_t USAGE_SIMULATION_CYCLIC_TRIM = 0x23; // Collection(Physical)
	static constexpr uint16_t USAGE_SIMULATION_FLIGHT_YOKE = 0x24; // Collection(Application)
	static constexpr uint16_t USAGE_SIMULATION_TRACK_CONTROL = 0x25; // Collection(Physical)
	static constexpr uint16_t USAGE_SIMULATION_AILERON = 0xB0;
	static constexpr uint16_t USAGE_SIMULATION_AILERON_TRIM = 0xB1;
	static constexpr uint16_t USAGE_SIMULATION_ANTI_TORQUE_CONTROL = 0xB2;
	static constexpr uint16_t USAGE_SIMULATION_AUTOPILOT_ENABLE = 0xB3;
	static constexpr uint16_t USAGE_SIMULATION_CHAFF_RELEASE = 0xB4;
	static constexpr uint16_t USAGE_SIMULATION_COLLECTIVE_CONTROL = 0xB5;
	static constexpr uint16_t USAGE_SIMULATION_DIVE_BREAK = 0xB6;
	static constexpr uint16_t USAGE_SIMULATION_ELECTRONIC_COUNTERMEASURES = 0xB7;
	static constexpr uint16_t USAGE_SIMULATION_ELEVATOR = 0xB8;
	static constexpr uint16_t USAGE_SIMULATION_ELEVATOR_TRIM = 0xB9;
	static constexpr uint16_t USAGE_SIMULATION_RUDDER = 0xBA;
	static constexpr uint16_t USAGE_SIMULATION_THROTTLE = 0xBB;
	static constexpr uint16_t USAGE_SIMULATION_FLIGHT_COMMUNICATIONS = 0xBC;
	static constexpr uint16_t USAGE_SIMULATION_FLARE_RELEASE = 0xBD;
	static constexpr uint16_t USAGE_SIMULATION_LANDING_GEAR = 0xBE;
	static constexpr uint16_t USAGE_SIMULATION_TOE_BRAKE = 0xBF;
	static constexpr uint16_t USAGE_SIMULATION_TRIGGER = 0xC0;
	static constexpr uint16_t USAGE_SIMULATION_WEAPONS_ARM = 0xC1;
	static constexpr uint16_t USAGE_SIMULATION_WEAPONS_SELECT = 0xC2;
	static constexpr uint16_t USAGE_SIMULATION_WING_FLAPS = 0xC3;
	static constexpr uint16_t USAGE_SIMULATION_ACCELERATOR = 0xC4;
	static constexpr uint16_t USAGE_SIMULATION_BRAKE = 0xC5;
	static constexpr uint16_t USAGE_SIMULATION_CLUTCH = 0xC6;
	static constexpr uint16_t USAGE_SIMULATION_SHIFTER = 0xC7;
	static constexpr uint16_t USAGE_SIMULATION_STEERING = 0xC8;
	static constexpr uint16_t USAGE_SIMULATION_TURRET_DIRECTION = 0xC9;
	static constexpr uint16_t USAGE_SIMULATION_BARREL_ELEVATION = 0xCA;
	static constexpr uint16_t USAGE_SIMULATION_DIVE_PLANE = 0xCB;
	static constexpr uint16_t USAGE_SIMULATION_BALLAST = 0xCC;
	static constexpr uint16_t USAGE_SIMULATION_BICYCLE_CRANK = 0xCD;
	static constexpr uint16_t USAGE_SIMULATION_HANDLE_BARS = 0xCE;
	static constexpr uint16_t USAGE_SIMULATION_FRONT_BRAKE = 0xCF;
	static constexpr uint16_t USAGE_SIMULATION_REAR_BRAKE = 0xD0;

	// VR Controls Page (0x03)

	static constexpr uint16_t USAGE_VR_BELT = 0x01; // Collection(Application)
	static constexpr uint16_t USAGE_VR_BODY_SUIT = 0x02; // Collection(Application)
	static constexpr uint16_t USAGE_VR_FLEXOR = 0x03; // Collection(Physical)
	static constexpr uint16_t USAGE_VR_GLOVE = 0x04; // Collection(Application)
	static constexpr uint16_t USAGE_VR_HEAD_TRACKER = 0x05; // Collection(Physical)
	static constexpr uint16_t USAGE_VR_HEAD_MOUNTED_DISPLAY = 0x06; // Collection(Application)
	static constexpr uint16_t USAGE_VR_HAND_TRACKER = 0x07; // Collection(Application)
	static constexpr uint16_t USAGE_VR_OCULOMETER = 0x08; // Collection(Application)
	static constexpr uint16_t USAGE_VR_VEST = 0x09; // Collection(Application)
	static constexpr uint16_t USAGE_VR_ANIMATRONIC_DEVICE = 0x0A; // Collection(Application)
	static constexpr uint16_t USAGE_VR_STEREO_ENABLE = 0x20;
	static constexpr uint16_t USAGE_VR_DISPLAY_ENABLE = 0x21;

	// Game Controls Page (0x05)

	static constexpr uint16_t USAGE_GAME_CONTROLLER = 0x01; // Collection(Application)
	static constexpr uint16_t USAGE_PINBALL_DEVICE = 0x02; // Collection(Application)
	static constexpr uint16_t USAGE_GUN_DEVICE = 0x03; // Collection(Application)
	// 0x04-0x1F Reserved
	static constexpr uint16_t USAGE_GAME_CONTROLS_POINT_OF_VIEW = 0x20; // Collection(Physical)
	static constexpr uint16_t USAGE_GAME_CONTROLS_TURN_RIGHT_LEFT = 0x21;
	static constexpr uint16_t USAGE_GAME_CONTROLS_PITCH_FORWARD_BACKWARD = 0x22;
	static constexpr uint16_t USAGE_GAME_CONTROLS_ROLL_RIGHT_LEFT = 0x23;
	static constexpr uint16_t USAGE_GAME_CONTROLS_MOVE_RIGHT_LEFT = 0x24;
	static constexpr uint16_t USAGE_GAME_CONTROLS_MOVE_FORWARD_BACKWARD = 0x25;
	static constexpr uint16_t USAGE_GAME_CONTROLS_MOVE_UP_DOWN = 0x26;
	static constexpr uint16_t USAGE_GAME_CONTROLS_LEAN_RIGHT_LEFT = 0x27;
	static constexpr uint16_t USAGE_GAME_CONTROLS_LEAN_FORWARD_BACKWARD = 0x28;
	static constexpr uint16_t USAGE_GAME_CONTROLS_HEIGHT_OF_POV = 0x29;
	static constexpr uint16_t USAGE_GAME_CONTROLS_FLIPPER = 0x2A;
	static constexpr uint16_t USAGE_GAME_CONTROLS_SECONDARY_FLIPPER = 0x2B;
	static constexpr uint16_t USAGE_GAME_CONTROLS_BUMP = 0x2C;
	static constexpr uint16_t USAGE_GAME_CONTROLS_NEW_GAME = 0x2D;
	static constexpr uint16_t USAGE_GAME_CONTROLS_SHOOT_BALL = 0x2E;
	static constexpr uint16_t USAGE_GAME_CONTROLS_PLAYER = 0x2F;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GUN_BOLT = 0x30;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GUN_CLIP = 0x31;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GUN_SELECTOR = 0x32;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GUN_SINGLE_SHOT = 0x33;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GUN_BURST = 0x34;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GUN_AUTOMATIC = 0x35;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GUN_SAFETY = 0x36;
	static constexpr uint16_t USAGE_GAME_CONTROLS_GAMEPAD_FIRE_JUMP = 0x37;
	// 0x38 Reserved
	static constexpr uint16_t USAGE_GAME_CONTROLS_GAMEPAD_TRIGGER = 0x39;
	static constexpr uint16_t USAGE_GAME_CONTROLS_FORM_FITTING_GAMEPAD = 0x3A;
	// 0x3B-0xFFFF Reserved

	// Keyboard/Keypad Page (0x07)

	static constexpr uint16_t USAGE_KEYBOARD_ERROR_ROLLOVER = 0x01;
	static constexpr uint16_t USAGE_KEYBOARD_POSTFAIL = 0x02;
	static constexpr uint16_t USAGE_KEYBOARD_ERROR_UNDEFINED = 0x03;
	static constexpr uint16_t USAGE_KEYBOARD_A = 0x04;
	static constexpr uint16_t USAGE_KEYBOARD_B = 0x05;
	static constexpr uint16_t USAGE_KEYBOARD_C = 0x06;
	static constexpr uint16_t USAGE_KEYBOARD_D = 0x07;
	static constexpr uint16_t USAGE_KEYBOARD_E = 0x08;
	static constexpr uint16_t USAGE_KEYBOARD_F = 0x09;
	static constexpr uint16_t USAGE_KEYBOARD_G = 0x0A;
	static constexpr uint16_t USAGE_KEYBOARD_H = 0x0B;
	static constexpr uint16_t USAGE_KEYBOARD_I = 0x0C;
	static constexpr uint16_t USAGE_KEYBOARD_J = 0x0D;
	static constexpr uint16_t USAGE_KEYBOARD_K = 0x0E;
	static constexpr uint16_t USAGE_KEYBOARD_L = 0x0F;
	static constexpr uint16_t USAGE_KEYBOARD_M = 0x10;
	static constexpr uint16_t USAGE_KEYBOARD_N = 0x11;
	static constexpr uint16_t USAGE_KEYBOARD_O = 0x12;
	static constexpr uint16_t USAGE_KEYBOARD_P = 0x13;
	static constexpr uint16_t USAGE_KEYBOARD_Q = 0x14;
	static constexpr uint16_t USAGE_KEYBOARD_R = 0x15;
	static constexpr uint16_t USAGE_KEYBOARD_S = 0x16;
	static constexpr uint16_t USAGE_KEYBOARD_T = 0x17;
	static constexpr uint16_t USAGE_KEYBOARD_U = 0x18;
	static constexpr uint16_t USAGE_KEYBOARD_V = 0x19;
	static constexpr uint16_t USAGE_KEYBOARD_W = 0x1A;
	static constexpr uint16_t USAGE_KEYBOARD_X = 0x1B;
	static constexpr uint16_t USAGE_KEYBOARD_Y = 0x1C;
	static constexpr uint16_t USAGE_KEYBOARD_Z = 0x1D;
	static constexpr uint16_t USAGE_KEYBOARD_1 = 0x1E;
	static constexpr uint16_t USAGE_KEYBOARD_2 = 0x1F;
	static constexpr uint16_t USAGE_KEYBOARD_3 = 0x20;
	static constexpr uint16_t USAGE_KEYBOARD_4 = 0x21;
	static constexpr uint16_t USAGE_KEYBOARD_5 = 0x22;
	static constexpr uint16_t USAGE_KEYBOARD_6 = 0x23;
	static constexpr uint16_t USAGE_KEYBOARD_7 = 0x24;
	static constexpr uint16_t USAGE_KEYBOARD_8 = 0x25;
	static constexpr uint16_t USAGE_KEYBOARD_9 = 0x26;
	static constexpr uint16_t USAGE_KEYBOARD_0 = 0x27;
	static constexpr uint16_t USAGE_KEYBOARD_ENTER = 0x28;
	static constexpr uint16_t USAGE_KEYBOARD_ESCAPE = 0x29;
	static constexpr uint16_t USAGE_KEYBOARD_BACKSPACE = 0x2A;
	static constexpr uint16_t USAGE_KEYBOARD_TAB = 0x2B;
	static constexpr uint16_t USAGE_KEYBOARD_SPACEBAR = 0x2C;
	static constexpr uint16_t USAGE_KEYBOARD_MINUS = 0x2D;
	static constexpr uint16_t USAGE_KEYBOARD_EQUALS = 0x2E;
	static constexpr uint16_t USAGE_KEYBOARD_SQAURE_BRACKET_OPEN = 0x2F;
	static constexpr uint16_t USAGE_KEYBOARD_SQUARE_BRACKET_CLOSE = 0x30;
	static constexpr uint16_t USAGE_KEYBOARD_BACKSLASH = 0x31;
	static constexpr uint16_t USAGE_KEYBOARD_HASHMARK = 0x32;
	static constexpr uint16_t USAGE_KEYBOARD_SEMICOLON = 0x33;
	static constexpr uint16_t USAGE_KEYBOARD_APOSTROPHE = 0x34;
	static constexpr uint16_t USAGE_KEYBOARD_GRAVE_ACCENT = 0x35;
	static constexpr uint16_t USAGE_KEYBOARD_COMMA = 0x36;
	static constexpr uint16_t USAGE_KEYBOARD_PERIOD = 0x37;
	static constexpr uint16_t USAGE_KEYBOARD_SLASH = 0x38;
	static constexpr uint16_t USAGE_KEYBOARD_CAPS_LOCK = 0x39;
	static constexpr uint16_t USAGE_KEYBOARD_F1 = 0x3A;
	static constexpr uint16_t USAGE_KEYBOARD_F2 = 0x3B;
	static constexpr uint16_t USAGE_KEYBOARD_F3 = 0x3C;
	static constexpr uint16_t USAGE_KEYBOARD_F4 = 0x3D;
	static constexpr uint16_t USAGE_KEYBOARD_F5 = 0x3E;
	static constexpr uint16_t USAGE_KEYBOARD_F6 = 0x3F;
	static constexpr uint16_t USAGE_KEYBOARD_F7 = 0x40;
	static constexpr uint16_t USAGE_KEYBOARD_F8 = 0x41;
	static constexpr uint16_t USAGE_KEYBOARD_F9 = 0x42;
	static constexpr uint16_t USAGE_KEYBOARD_F10 = 0x43;
	static constexpr uint16_t USAGE_KEYBOARD_F11 = 0x44;
	static constexpr uint16_t USAGE_KEYBOARD_F12 = 0x45;
	static constexpr uint16_t USAGE_KEYBOARD_PRINTSCREEN = 0x46;
	static constexpr uint16_t USAGE_KEYBOARD_SCROLL_LOCK = 0x47;
	static constexpr uint16_t USAGE_KEYBOARD_PAUSE = 0x48;
	static constexpr uint16_t USAGE_KEYBOARD_INSERT = 0x49;
	static constexpr uint16_t USAGE_KEYBOARD_HOME = 0x4A;
	static constexpr uint16_t USAGE_KEYBOARD_PAGEUP = 0x4B;
	static constexpr uint16_t USAGE_KEYBOARD_DELETE = 0x4C;
	static constexpr uint16_t USAGE_KEYBOARD_END = 0x4D;
	static constexpr uint16_t USAGE_KEYBOARD_PAGEDOWN = 0x4E;
	static constexpr uint16_t USAGE_KEYBOARD_RIGHTARROW = 0x4F;
	static constexpr uint16_t USAGE_KEYBOARD_LEFTARROW = 0x50;
	static constexpr uint16_t USAGE_KEYBOARD_DOWNARROW = 0x51;
	static constexpr uint16_t USAGE_KEYBOARD_UPARROW = 0x52;
	static constexpr uint16_t USAGE_KEYPAD_NUM_LOCK = 0x53;
	static constexpr uint16_t USAGE_KEYPAD_DIVIDE = 0x54;
	static constexpr uint16_t USAGE_KEYPAD_TIMES = 0x55;
	static constexpr uint16_t USAGE_KEYPAD_MINUS = 0x56;
	static constexpr uint16_t USAGE_KEYPAD_PLUS = 0x57;
	static constexpr uint16_t USAGE_KEYPAD_ENTER = 0x58;
	static constexpr uint16_t USAGE_KEYPAD_1 = 0x59;
	static constexpr uint16_t USAGE_KEYPAD_2 = 0x5A;
	static constexpr uint16_t USAGE_KEYPAD_3 = 0x5B;
	static constexpr uint16_t USAGE_KEYPAD_4 = 0x5C;
	static constexpr uint16_t USAGE_KEYPAD_5 = 0x5D;
	static constexpr uint16_t USAGE_KEYPAD_6 = 0x5E;
	static constexpr uint16_t USAGE_KEYPAD_7 = 0x5F;
	static constexpr uint16_t USAGE_KEYPAD_8 = 0x60;
	static constexpr uint16_t USAGE_KEYPAD_9 = 0x61;
	static constexpr uint16_t USAGE_KEYPAD_0 = 0x62;
	static constexpr uint16_t USAGE_KEYPAD_PERIOD = 0x63;
	static constexpr uint16_t USAGE_KEYBOARD_NON_US_BACKSLASH = 0x64;
	static constexpr uint16_t USAGE_KEYBOARD_APPLICATION = 0x65;
	static constexpr uint16_t USAGE_KEYBOARD_POWER = 0x66;
	static constexpr uint16_t USAGE_KEYPAD_EQUALS = 0x67;
	static constexpr uint16_t USAGE_KEYBOARD_F13 = 0x68;
	static constexpr uint16_t USAGE_KEYBOARD_F14 = 0x69;
	static constexpr uint16_t USAGE_KEYBOARD_F15 = 0x6A;
	static constexpr uint16_t USAGE_KEYBOARD_F16 = 0x6B;
	static constexpr uint16_t USAGE_KEYBOARD_F17 = 0x6C;
	static constexpr uint16_t USAGE_KEYBOARD_F18 = 0x6D;
	static constexpr uint16_t USAGE_KEYBOARD_F19 = 0x6E;
	static constexpr uint16_t USAGE_KEYBOARD_F20 = 0x6F;
	static constexpr uint16_t USAGE_KEYBOARD_F21 = 0x70;
	static constexpr uint16_t USAGE_KEYBOARD_F22 = 0x71;
	static constexpr uint16_t USAGE_KEYBOARD_F23 = 0x72;
	static constexpr uint16_t USAGE_KEYBOARD_F24 = 0x73;
	static constexpr uint16_t USAGE_KEYBOARD_EXECUTE = 0x74;
	static constexpr uint16_t USAGE_KEYBOARD_HELP = 0x75;
	static constexpr uint16_t USAGE_KEYBOARD_MENU = 0x76;
	static constexpr uint16_t USAGE_KEYBOARD_SELECT = 0x77;
	static constexpr uint16_t USAGE_KEYBOARD_STOP = 0x78;
	static constexpr uint16_t USAGE_KEYBOARD_AGAIN = 0x79;
	static constexpr uint16_t USAGE_KEYBOARD_UNDO = 0x7A;
	static constexpr uint16_t USAGE_KEYBOARD_CUT = 0x7B;
	static constexpr uint16_t USAGE_KEYBOARD_COPY = 0x7C;
	static constexpr uint16_t USAGE_KEYBOARD_PASTE = 0x7D;
	static constexpr uint16_t USAGE_KEYBOARD_FIND = 0x7E;
	static constexpr uint16_t USAGE_KEYBOARD_MUTE = 0x7F;
	static constexpr uint16_t USAGE_KEYBOARD_VOLUME_UP = 0x80;
	static constexpr uint16_t USAGE_KEYBOARD_VOLUME_DOWN = 0x81;
	static constexpr uint16_t USAGE_KEYBOARD_LOCKING_CAPS_LOCK = 0x82;
	static constexpr uint16_t USAGE_KEYBOARD_LOCKING_NUM_LOCK = 0x83;
	static constexpr uint16_t USAGE_KEYBOARD_LOCKING_SCROLL_LOCK = 0x84;
	static constexpr uint16_t USAGE_KEYPAD_COMMA = 0x85;
	static constexpr uint16_t USAGE_KEYPAD_EQUAL = 0x86;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_1 = 0x87;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_2 = 0x88;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_3 = 0x89;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_4 = 0x8A;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_5 = 0x8B;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_6 = 0x8C;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_7 = 0x8D;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_8 = 0x8E;
	static constexpr uint16_t USAGE_KEYBOARD_INTERNATIONAL_9 = 0x8F;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_1 = 0x90;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_2 = 0x91;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_3 = 0x92;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_4 = 0x93;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_5 = 0x94;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_6 = 0x95;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_7 = 0x96;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_8 = 0x97;
	static constexpr uint16_t USAGE_KEYBOARD_LANG_9 = 0x98;
	static constexpr uint16_t USAGE_KEYBOARD_ALTERNATE_ERASE = 0x99;
	static constexpr uint16_t USAGE_KEYBOARD_SYSREQ = 0x9A;
	static constexpr uint16_t USAGE_KEYBOARD_CANCEL = 0x9B;
	static constexpr uint16_t USAGE_KEYBOARD_CLEAR = 0x9C;
	static constexpr uint16_t USAGE_KEYBOARD_PRIOR = 0x9D;
	static constexpr uint16_t USAGE_KEYBOARD_RETURN = 0x9E;
	static constexpr uint16_t USAGE_KEYBOARD_SEPARATOR = 0x9F;
	static constexpr uint16_t USAGE_KEYBOARD_OUT = 0xA0;
	static constexpr uint16_t USAGE_KEYBOARD_OPER = 0xA1;
	static constexpr uint16_t USAGE_KEYBOARD_CLEAR_OR_AGAIN = 0xA2;
	static constexpr uint16_t USAGE_KEYBOARD_CRSEL = 0xA3;
	static constexpr uint16_t USAGE_KEYBOARD_EXSEL = 0xA4;
	static constexpr uint16_t USAGE_KEYPAD_00 = 0xB0;
	static constexpr uint16_t USAGE_KEYPAD_000 = 0xB1;
	static constexpr uint16_t USAGE_THOUSANDS_SEPARATOR = 0xB2;
	static constexpr uint16_t USAGE_DECIMAL_SEPARATOR = 0xB3;
	static constexpr uint16_t USAGE_CURRENCY_UNIT = 0xB4;
	static constexpr uint16_t USAGE_CURRENCY_SUB_UNIT = 0xB5;
	static constexpr uint16_t USAGE_KEYPAD_ROUND_BRACKET_OPEN = 0xB6;
	static constexpr uint16_t USAGE_KEYPAD_ROUND_BRACKET_CLOSE = 0xB7;
	static constexpr uint16_t USAGE_KEYPAD_CURLY_BRACKET_OPEN = 0xB8;
	static constexpr uint16_t USAGE_KEYPAD_CURLY_BRACKET_CLOSE = 0xB9;
	static constexpr uint16_t USAGE_KEYPAD_TAB = 0xBA;
	static constexpr uint16_t USAGE_KEYPAD_BACKSPACE = 0xBB;
	static constexpr uint16_t USAGE_KEYPAD_A = 0xBC;
	static constexpr uint16_t USAGE_KEYPAD_B = 0xBD;
	static constexpr uint16_t USAGE_KEYPAD_C = 0xBE;
	static constexpr uint16_t USAGE_KEYPAD_D = 0xBF;
	static constexpr uint16_t USAGE_KEYPAD_E = 0xC0;
	static constexpr uint16_t USAGE_KEYPAD_F = 0xC1;
	static constexpr uint16_t USAGE_KEYPAD_XOR = 0xC2;
	static constexpr uint16_t USAGE_KEYPAD_CARET = 0xC3;
	static constexpr uint16_t USAGE_KEYPAD_PERCENT = 0xC4;
	static constexpr uint16_t USAGE_KEYPAD_LESS_THAN = 0xC5;
	static constexpr uint16_t USAGE_KEYPAD_GREATER_THAN = 0xC6;
	static constexpr uint16_t USAGE_KEYPAD_BITWISE_AND = 0xC7;
	static constexpr uint16_t USAGE_KEYPAD_LOGICAL_AND = 0xC8;
	static constexpr uint16_t USAGE_KEYPAD_BITWISE_OR = 0xC9;
	static constexpr uint16_t USAGE_KEYPAD_LOGICAL_OR = 0xCA;
	static constexpr uint16_t USAGE_KEYPAD_COLON = 0xCB;
	static constexpr uint16_t USAGE_KEYPAD_HASHMARK = 0xCC;
	static constexpr uint16_t USAGE_KEYPAD_SPACE = 0xCD;
	static constexpr uint16_t USAGE_KEYPAD_AT = 0xCE;
	static constexpr uint16_t USAGE_KEYPAD_EXCLAMATION_MARK = 0xCF;
	static constexpr uint16_t USAGE_KEYPAD_MEMORY_STORE = 0xD0;
	static constexpr uint16_t USAGE_KEYPAD_MEMORY_RECALL = 0xD1;
	static constexpr uint16_t USAGE_KEYPAD_MEMORY_CLEAR = 0xD2;
	static constexpr uint16_t USAGE_KEYPAD_MEMORY_ADD = 0xD3;
	static constexpr uint16_t USAGE_KEYPAD_MEMORY_SUBTRACT = 0xD4;
	static constexpr uint16_t USAGE_KEYPAD_MEMORY_MULTIPLY = 0xD5;
	static constexpr uint16_t USAGE_KEYPAD_MEMORY_DIVIDE = 0xD6;
	static constexpr uint16_t USAGE_KEYPAD_PLUS_MINUS = 0xD7;
	static constexpr uint16_t USAGE_KEYPAD_CLEAR = 0xD8;
	static constexpr uint16_t USAGE_KEYPAD_CLEAR_ENTRY = 0xD9;
	static constexpr uint16_t USAGE_KEYPAD_BINARY = 0xDA;
	static constexpr uint16_t USAGE_KEYPAD_OCTAL = 0xDB;
	static constexpr uint16_t USAGE_KEYPAD_DECIMAL = 0xDC;
	static constexpr uint16_t USAGE_KEYPAD_HEXADECIMAL = 0xDD;
	static constexpr uint16_t USAGE_KEYBOARD_LEFT_CONTROL = 0xE0;
	static constexpr uint16_t USAGE_KEYBOARD_LEFT_SHIFT = 0xE1;
	static constexpr uint16_t USAGE_KEYBOARD_LEFT_ALT = 0xE2;
	static constexpr uint16_t USAGE_KEYBOARD_LEFT_GUI = 0xE3;
	static constexpr uint16_t USAGE_KEYBOARD_RIGHT_CONTROL = 0xE4;
	static constexpr uint16_t USAGE_KEYBOARD_RIGHT_SHIFT = 0xE5;
	static constexpr uint16_t USAGE_KEYBOARD_RIGHT_ALT = 0xE6;
	static constexpr uint16_t USAGE_KEYBOARD_RIGHT_GUI = 0xE7;

	// The below keyboard page usages are non-standard and aren't part of the
	// official HID Usage Tables document (hut1_5.pdf). These usages seem to
	// work only on Linux. I tested them on Ubuntu 20.04 LTS that interpreted
	// and executed almost all of these usage codes as expected.
	// MacOS, Windows 10 and Windows XP don't recognise these codes.
	//
	// All of these non-standard codes have standard equivalents on the consumer
	// usage page and those work on all major up-to-date desktop operating
	// systems so new hardware should probably opt for sending those instead.
	//
	// Source: https://github.com/greatscottgadgets/facedancer/blob/e1095030aa6dce764bc03c2f99a13473e3269de6/facedancer/classes/hid/keyboard.py

	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_PLAYPAUSE = 0xE8;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_STOPCD = 0xE9;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_PREVIOUSSONG = 0xEA;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_NEXTSONG = 0xEB;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_EJECTCD = 0xEC;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_VOLUMEUP = 0xED;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_VOLUMEDOWN = 0xEE;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_MUTE = 0xEF;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_WWW = 0xF0;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_BACK = 0xF1;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_FORWARD = 0xF2;
	// The xev tool logged the Cancel symbol as a response to USAGE_KEYBOARD_MEDIA_STOP.
	// My Ubuntu 20.04 Firefox seemed to ignore it.
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_STOP = 0xF3;
	// The xev log showed the Find symbol.
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_FIND = 0xF4;
	// USAGE_KEYBOARD_MEDIA_SCROLLUP and USAGE_KEYBOARD_MEDIA_SCROLLDOWN didn't
	// work in my Ubuntu 20.04 Firefox browser but the xev log showed the
	// ScrollUp and ScrollDown symbols.
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_SCROLLUP = 0xF5;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_SCROLLDOWN = 0xF6;
	// The xev tool logged no symbol as a response to USAGE_KEYBOARD_MEDIA_EDIT.
	// This was the only non-standard symbol that had no effect on Linux.
	// All other symbols worked or at least translated to a symbol in xev log.
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_EDIT = 0xF7;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_SLEEP = 0xF8;
	// USAGE_KEYBOARD_MEDIA_COFFEE seems to go to the Lock screen like GUI+L.
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_COFFEE = 0xF9;
	// The xev tool logged the Reload symbol as a response to USAGE_KEYBOARD_MEDIA_REFRESH.
	// The browser recognised and executed it.
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_REFRESH = 0xFA;
	static constexpr uint16_t USAGE_KEYBOARD_MEDIA_CALC = 0xFB;

	// LED Page (0x08)

	static constexpr uint16_t USAGE_LED_NUM_LOCK = 0x01;
	static constexpr uint16_t USAGE_LED_CAPS_LOCK = 0x2;
	static constexpr uint16_t USAGE_LED_SCROLL_LOCK = 0x3;
	static constexpr uint16_t USAGE_LED_COMPOSE = 0x04;
	static constexpr uint16_t USAGE_LED_KANA = 0x05;
	static constexpr uint16_t USAGE_LED_POWER = 0x06;
	static constexpr uint16_t USAGE_LED_SHIFT = 0x07;

	// Consumer Page (0x0C)

	static constexpr uint16_t USAGE_CONSUMER_CONTROL = 0x0001; // Collection(Application)
	static constexpr uint16_t USAGE_CONSUMER_NUMERIC_KEY_PAD = 0x0002;
	static constexpr uint16_t USAGE_CONSUMER_PROGRAMMABLE_BUTTONS = 0x0003;
	static constexpr uint16_t USAGE_CONSUMER_MICROPHONE = 0x0004; // Collection(Application)
	static constexpr uint16_t USAGE_CONSUMER_HEADPHONE = 0x0005; // Collection(Application)
	static constexpr uint16_t USAGE_CONSUMER_GRAPHIC_EQUALIZER = 0x0006; // Collection(Application)
	// 0x0007-0x001F Reserved
	static constexpr uint16_t USAGE_CONSUMER_PLUS_10 = 0x0020;
	static constexpr uint16_t USAGE_CONSUMER_PLUS_100 = 0x0021;
	static constexpr uint16_t USAGE_CONSUMER_AM_PM = 0x0022;
	// 0x0023-0x002F Reserved
	static constexpr uint16_t USAGE_CONSUMER_POWER = 0x0030;
	static constexpr uint16_t USAGE_CONSUMER_RESET = 0x0031;
	static constexpr uint16_t USAGE_CONSUMER_SLEEP = 0x0032;
	static constexpr uint16_t USAGE_CONSUMER_SLEEP_AFTER = 0x0033;
	static constexpr uint16_t USAGE_CONSUMER_SLEEP_MODE = 0x0034;
	static constexpr uint16_t USAGE_CONSUMER_ILLUMINATION = 0x0035;
	static constexpr uint16_t USAGE_CONSUMER_FUNCTION_BUTTONS = 0x0036;
	// 0x0037-0x003F Reserved
	static constexpr uint16_t USAGE_CONSUMER_MENU = 0x0040;
	static constexpr uint16_t USAGE_CONSUMER_MENU_PICK = 0x0041;
	static constexpr uint16_t USAGE_CONSUMER_MENU_UP = 0x0042;
	static constexpr uint16_t USAGE_CONSUMER_MENU_DOWN = 0x0043;
	static constexpr uint16_t USAGE_CONSUMER_MENU_LEFT = 0x0044;
	static constexpr uint16_t USAGE_CONSUMER_MENU_RIGHT = 0x0045;
	static constexpr uint16_t USAGE_CONSUMER_MENU_ESCAPE = 0x0046;
	static constexpr uint16_t USAGE_CONSUMER_MENU_VALUE_INCREASE = 0x0047;
	static constexpr uint16_t USAGE_CONSUMER_MENU_VALUE_DECREASE = 0x0048;
	// 0x0049-0x005F Reserved
	static constexpr uint16_t USAGE_CONSUMER_DATA_ON_SCREEN = 0x0060;
	static constexpr uint16_t USAGE_CONSUMER_CLOSED_CAPTION = 0x0061;
	static constexpr uint16_t USAGE_CONSUMER_CLOSED_CAPTION_SELECT = 0x0062;
	static constexpr uint16_t USAGE_CONSUMER_VCR_TV = 0x0063;
	static constexpr uint16_t USAGE_CONSUMER_BROADCAST_MODE = 0x0064;
	static constexpr uint16_t USAGE_CONSUMER_SNAPSHOT = 0x0065;
	static constexpr uint16_t USAGE_CONSUMER_STILL = 0x0066;
	static constexpr uint16_t USAGE_CONSUMER_PICTURE_IN_PICTURE_TOGGLE = 0x0067;
	static constexpr uint16_t USAGE_CONSUMER_PICTURE_IN_PICTURE_SWAP = 0x0068;
	static constexpr uint16_t USAGE_CONSUMER_RED_MENU_BUTTON = 0x0069;
	static constexpr uint16_t USAGE_CONSUMER_GREEN_MENU_BUTTON = 0x006A;
	static constexpr uint16_t USAGE_CONSUMER_BLUE_MENU_BUTTON = 0x006B;
	static constexpr uint16_t USAGE_CONSUMER_YELLOW_MENU_BUTTON = 0x006C;
	static constexpr uint16_t USAGE_CONSUMER_ASPECT = 0x006D;
	static constexpr uint16_t USAGE_CONSUMER_3D_MODE_SELECT = 0x006E;
	static constexpr uint16_t USAGE_CONSUMER_DISPLAY_BRIGHTNESS_INCREMENT = 0x006F;
	static constexpr uint16_t USAGE_CONSUMER_DISPLAY_BRIGHTNESS_DECREMENT = 0x0070;
	static constexpr uint16_t USAGE_CONSUMER_DISPLAY_BRIGHTNESS = 0x0071;
	static constexpr uint16_t USAGE_CONSUMER_DISPLAY_BACKLIGHT_TOGGLE = 0x0072;
	static constexpr uint16_t USAGE_CONSUMER_DISPLAY_SET_BRIGHTNESS_TO_MINIMUM = 0x0073;
	static constexpr uint16_t USAGE_CONSUMER_DISPLAY_SET_BRIGHTNESS_TO_MAXIMUM = 0x0074;
	static constexpr uint16_t USAGE_CONSUMER_DISPLAY_SET_AUTO_BRIGHTNESS = 0x0075;
	static constexpr uint16_t USAGE_CONSUMER_CAMERA_ACCESS_ENABLED = 0x0076;
	static constexpr uint16_t USAGE_CONSUMER_CAMERA_ACCESS_DISABLED = 0x0077;
	static constexpr uint16_t USAGE_CONSUMER_CAMERA_ACCESS_TOGGLE = 0x0078;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_BRIGHTNESS_INCREMENT = 0x0079;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_BRIGHTNESS_DECREMENT = 0x007A;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_BACKLIGHT_SET_LEVEL = 0x007B;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_BACKLIGHT_OOC = 0x007C;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_BACKLIGHT_SET_MINIMUM = 0x007D;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_BACKLIGHT_SET_MAXIMUM = 0x007E;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_BACKLIGHT_AUTO = 0x007F;
	static constexpr uint16_t USAGE_CONSUMER_SELECTION = 0x0080;
	static constexpr uint16_t USAGE_CONSUMER_ASSIGN_SELECTION = 0x0081;
	static constexpr uint16_t USAGE_CONSUMER_MODE_STEP = 0x0082;
	static constexpr uint16_t USAGE_CONSUMER_RECALL_LAST = 0x0083;
	static constexpr uint16_t USAGE_CONSUMER_ENTER_CHANNEL = 0x0084;
	static constexpr uint16_t USAGE_CONSUMER_ORDER_MOVIE = 0x0085;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL = 0x0086;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECTION = 0x0087;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_COMPUTER = 0x0088;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_TV = 0x0089;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_WWW = 0x008A;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_DVD = 0x008B;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_TELEPHONE = 0x008C;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_PROGRAM_GUIDE = 0x008D;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_VIDEO_PHONE = 0x008E;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_GAMES = 0x008F;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_MESSAGES = 0x0090;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_CD = 0x0091;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_VCR = 0x0092;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_TUNER = 0x0093;
	static constexpr uint16_t USAGE_CONSUMER_QUIT = 0x0094;
	static constexpr uint16_t USAGE_CONSUMER_HELP = 0x0095;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_TAPE = 0x0096;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_CABLE = 0x0097;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_SATELLITE = 0x0098;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_SECURITY = 0x0099;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_HOME = 0x009A;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_CALL = 0x009B;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_INCREMENT = 0x009C;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_DECREMENT = 0x009D;
	static constexpr uint16_t USAGE_CONSUMER_MEDIA_SELECT_SAP = 0x009E;
	// 0x009F-0x009F Reserved
	static constexpr uint16_t USAGE_CONSUMER_VCR_PLUS = 0x00A0;
	static constexpr uint16_t USAGE_CONSUMER_ONCE = 0x00A1;
	static constexpr uint16_t USAGE_CONSUMER_DAILY = 0x00A2;
	static constexpr uint16_t USAGE_CONSUMER_WEEKLY = 0x00A3;
	static constexpr uint16_t USAGE_CONSUMER_MONTHLY = 0x00A4;
	// 0x00A5-0x00AF Reserved
	static constexpr uint16_t USAGE_CONSUMER_PLAY = 0x00B0;
	static constexpr uint16_t USAGE_CONSUMER_PAUSE = 0x00B1;
	static constexpr uint16_t USAGE_CONSUMER_RECORD = 0x00B2;
	static constexpr uint16_t USAGE_CONSUMER_FAST_FORWARD = 0x00B3;
	static constexpr uint16_t USAGE_CONSUMER_REWIND = 0x00B4;
	static constexpr uint16_t USAGE_CONSUMER_SCAN_NEXT_TRACK = 0x00B5;
	static constexpr uint16_t USAGE_CONSUMER_SCAN_PREVIOUS_TRACK = 0x00B6;
	static constexpr uint16_t USAGE_CONSUMER_STOP = 0x00B7;
	static constexpr uint16_t USAGE_CONSUMER_EJECT = 0x00B8;
	static constexpr uint16_t USAGE_CONSUMER_RANDOM_PLAY = 0x00B9;
	static constexpr uint16_t USAGE_CONSUMER_SELECT_DISC = 0x00BA;
	static constexpr uint16_t USAGE_CONSUMER_ENTER_DISC = 0x00BB;
	static constexpr uint16_t USAGE_CONSUMER_REPEAT = 0x00BC;
	static constexpr uint16_t USAGE_CONSUMER_TRACKING = 0x00BD;
	static constexpr uint16_t USAGE_CONSUMER_TRACK_NORMAL = 0x00BE;
	static constexpr uint16_t USAGE_CONSUMER_SLOW_TRACKING = 0x00BF;
	static constexpr uint16_t USAGE_CONSUMER_FRAME_FORWARD = 0x00C0;
	static constexpr uint16_t USAGE_CONSUMER_FRAME_BACK = 0x00C1;
	static constexpr uint16_t USAGE_CONSUMER_MARK = 0x00C2;
	static constexpr uint16_t USAGE_CONSUMER_CLEAR_MARK = 0x00C3;
	static constexpr uint16_t USAGE_CONSUMER_REPEAT_FROM_MARK = 0x00C4;
	static constexpr uint16_t USAGE_CONSUMER_RETURN_TO_MARK = 0x00C5;
	static constexpr uint16_t USAGE_CONSUMER_SEARCH_MARK_FORWARD = 0x00C6;
	static constexpr uint16_t USAGE_CONSUMER_SEARCH_MARK_BACKWARDS = 0x00C7;
	static constexpr uint16_t USAGE_CONSUMER_COUNTER_RESET = 0x00C8;
	static constexpr uint16_t USAGE_CONSUMER_SHOW_COUNTER = 0x00C9;
	static constexpr uint16_t USAGE_CONSUMER_TRACKING_INCREMENT = 0x00CA;
	static constexpr uint16_t USAGE_CONSUMER_TRACKING_DECREMENT = 0x00CB;
	static constexpr uint16_t USAGE_CONSUMER_STOP_EJECT = 0x00CC;
	static constexpr uint16_t USAGE_CONSUMER_PLAY_PAUSE = 0x00CD;
	static constexpr uint16_t USAGE_CONSUMER_PLAY_SKIP = 0x00CE;
	static constexpr uint16_t USAGE_CONSUMER_VOICE_COMMAND = 0x00CF;
	static constexpr uint16_t USAGE_CONSUMER_INVOKE_CAPTURE_INTERFACE = 0x00D0;
	static constexpr uint16_t USAGE_CONSUMER_START_OR_STOP_GAME_RECORDING = 0x00D1;
	static constexpr uint16_t USAGE_CONSUMER_HISTORICAL_GAME_CAPTURE = 0x00D2;
	static constexpr uint16_t USAGE_CONSUMER_CAPTURE_GAME_SCREENSHOT = 0x00D3;
	static constexpr uint16_t USAGE_CONSUMER_SHOW_OR_HIDE_RECORDING_INDICATOR = 0x00D4;
	static constexpr uint16_t USAGE_CONSUMER_START_OR_STOP_MICROPHONE_CAPTURE = 0x00D5;
	static constexpr uint16_t USAGE_CONSUMER_START_OR_STOP_CAMERA_CAPTURE = 0x00D6;
	static constexpr uint16_t USAGE_CONSUMER_START_OR_STOP_GAME_BROADCAST = 0x00D7;
	static constexpr uint16_t USAGE_CONSUMER_START_OR_STOP_VOICE_DICTATION_SESSION = 0x00D8;
	static constexpr uint16_t USAGE_CONSUMER_INVOKE_DISMISS_EMOJI_PICKER = 0x00D9;
	// 0x00DA-0x00DF Reserved
	static constexpr uint16_t USAGE_CONSUMER_VOLUME = 0x00E0;
	static constexpr uint16_t USAGE_CONSUMER_BALANCE = 0x00E1;
	static constexpr uint16_t USAGE_CONSUMER_MUTE = 0x00E2;
	static constexpr uint16_t USAGE_CONSUMER_BASS = 0x00E3;
	static constexpr uint16_t USAGE_CONSUMER_TREBLE = 0x00E4;
	static constexpr uint16_t USAGE_CONSUMER_BASS_BOOST = 0x00E5;
	static constexpr uint16_t USAGE_CONSUMER_SURROUND_MODE = 0x00E6;
	static constexpr uint16_t USAGE_CONSUMER_LOUDNESS = 0x00E7;
	static constexpr uint16_t USAGE_CONSUMER_MPX = 0x00E8;
	static constexpr uint16_t USAGE_CONSUMER_VOLUME_INCREMENT = 0x00E9;
	static constexpr uint16_t USAGE_CONSUMER_VOLUME_DECREMENT = 0x00EA;
	// 0x00EB-0x00EF Reserved
	static constexpr uint16_t USAGE_CONSUMER_SPEED_SELECT = 0x00F0;
	static constexpr uint16_t USAGE_CONSUMER_PLAYBACK_SPEED = 0x00F1;
	static constexpr uint16_t USAGE_CONSUMER_STANDARD_PLAY = 0x00F2;
	static constexpr uint16_t USAGE_CONSUMER_LONG_PLAY = 0x00F3;
	static constexpr uint16_t USAGE_CONSUMER_EXTENDED_PLAY = 0x00F4;
	static constexpr uint16_t USAGE_CONSUMER_SLOW = 0x00F5;
	// 0x00F6-0x00FF Reserved
	static constexpr uint16_t USAGE_CONSUMER_FAN_ENABLE = 0x0100;
	static constexpr uint16_t USAGE_CONSUMER_FAN_SPEED = 0x0101;
	static constexpr uint16_t USAGE_CONSUMER_LIGHT_ENABLE = 0x0102;
	static constexpr uint16_t USAGE_CONSUMER_LIGHT_ILLUMINATION_LEVEL = 0x0103;
	static constexpr uint16_t USAGE_CONSUMER_CLIMATE_CONTROL_ENABLE = 0x0104;
	static constexpr uint16_t USAGE_CONSUMER_ROOM_TEMPERATURE = 0x0105;
	static constexpr uint16_t USAGE_CONSUMER_SECURITY_ENABLE = 0x0106;
	static constexpr uint16_t USAGE_CONSUMER_FIRE_ALARM = 0x0107;
	static constexpr uint16_t USAGE_CONSUMER_POLICE_ALARM = 0x0108;
	static constexpr uint16_t USAGE_CONSUMER_PROXIMITY = 0x0109;
	static constexpr uint16_t USAGE_CONSUMER_MOTION = 0x010A;
	static constexpr uint16_t USAGE_CONSUMER_DURESS_ALARM = 0x010B;
	static constexpr uint16_t USAGE_CONSUMER_HOLDUP_ALARM = 0x010C;
	static constexpr uint16_t USAGE_CONSUMER_MEDICAL_ALARM = 0x010D;
	// 0x010E-0x014F Reserved
	static constexpr uint16_t USAGE_CONSUMER_BALANCE_RIGHT = 0x0150;
	static constexpr uint16_t USAGE_CONSUMER_BALANCE_LEFT = 0x0151;
	static constexpr uint16_t USAGE_CONSUMER_BASS_INCREMENT = 0x0152;
	static constexpr uint16_t USAGE_CONSUMER_BASS_DECREMENT = 0x0153;
	static constexpr uint16_t USAGE_CONSUMER_TREBLE_INCREMENT = 0x0154;
	static constexpr uint16_t USAGE_CONSUMER_TREBLE_DECREMENT = 0x0155;
	// 0x0156-0x015F Reserved
	static constexpr uint16_t USAGE_CONSUMER_SPEAKER_SYSTEM = 0x0160;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_LEFT = 0x0161;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_RIGHT = 0x0162;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_CENTER = 0x0163;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_FRONT = 0x0164;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_CENTER_FRONT = 0x0165;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_SIDE = 0x0166;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_SURROUND = 0x0167;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_LOW_FREQUENCY_ENHANCEMENT = 0x0168;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_TOP = 0x0169;
	static constexpr uint16_t USAGE_CONSUMER_CHANNEL_UNKNOWN = 0x016A;
	// 0x016B-0x016F Reserved
	static constexpr uint16_t USAGE_CONSUMER_SUBCHANNEL = 0x0170;
	static constexpr uint16_t USAGE_CONSUMER_SUBCHANNEL_INCREMENT = 0x0171;
	static constexpr uint16_t USAGE_CONSUMER_SUBCHANNEL_DECREMENT = 0x0172;
	static constexpr uint16_t USAGE_CONSUMER_ALTERNATE_AUDIO_INCREMENT = 0x0173;
	static constexpr uint16_t USAGE_CONSUMER_ALTERNATE_AUDIO_DECREMENT = 0x0174;
	// 0x0175-0x017F Reserved
	static constexpr uint16_t USAGE_CONSUMER_APPLICATION_LAUNCH_BUTTONS = 0x0180;
	static constexpr uint16_t USAGE_CONSUMER_AL_LAUNCH_BUTTON_CONFIGURATION_TOOL = 0x0181;
	static constexpr uint16_t USAGE_CONSUMER_AL_PROGRAMMABLE_BUTTON_CONFIGURATION = 0x0182;
	static constexpr uint16_t USAGE_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION = 0x0183;
	static constexpr uint16_t USAGE_CONSUMER_AL_WORD_PROCESSOR = 0x0184;
	static constexpr uint16_t USAGE_CONSUMER_AL_TEXT_EDITOR = 0x0185;
	static constexpr uint16_t USAGE_CONSUMER_AL_SPREADSHEET = 0x0186;
	static constexpr uint16_t USAGE_CONSUMER_AL_GRAPHICS_EDITOR = 0x0187;
	static constexpr uint16_t USAGE_CONSUMER_AL_PRESENTATION_APP = 0x0188;
	static constexpr uint16_t USAGE_CONSUMER_AL_DATABASE_APP = 0x0189;
	static constexpr uint16_t USAGE_CONSUMER_AL_EMAIL_READER = 0x018A;
	static constexpr uint16_t USAGE_CONSUMER_AL_NEWSREADER = 0x018B;
	static constexpr uint16_t USAGE_CONSUMER_AL_VOICEMAIL = 0x018C;
	static constexpr uint16_t USAGE_CONSUMER_AL_CONTACTS_ADDRESS_BOOK = 0x018D;
	static constexpr uint16_t USAGE_CONSUMER_AL_CALENDAR_SCHEDULE = 0x018E;
	static constexpr uint16_t USAGE_CONSUMER_AL_TASK_PROJECT_MANAGER = 0x018F;
	static constexpr uint16_t USAGE_CONSUMER_AL_LOG_JOURNAL_TIMECARD = 0x0190;
	static constexpr uint16_t USAGE_CONSUMER_AL_CHECKBOOK_FINANCE = 0x0191;
	static constexpr uint16_t USAGE_CONSUMER_AL_CALCULATOR = 0x0192;
	static constexpr uint16_t USAGE_CONSUMER_AL_AV_CAPTURE_PLAYBACK = 0x0193;
	static constexpr uint16_t USAGE_CONSUMER_AL_LOCAL_MACHINE_BROWSER = 0x0194;
	static constexpr uint16_t USAGE_CONSUMER_AL_LAN_WAN_BROWSER = 0x0195;
	static constexpr uint16_t USAGE_CONSUMER_AL_INTERNET_BROWSER = 0x0196;
	static constexpr uint16_t USAGE_CONSUMER_AL_REMOTE_NETWORKING_ISP_CONNECT = 0x0197;
	static constexpr uint16_t USAGE_CONSUMER_AL_NETWORK_CONFERENCE = 0x0198;
	static constexpr uint16_t USAGE_CONSUMER_AL_NETWORK_CHAT = 0x0199;
	static constexpr uint16_t USAGE_CONSUMER_AL_TELEPHONY_DIALER = 0x019A;
	static constexpr uint16_t USAGE_CONSUMER_AL_LOGON = 0x019B;
	static constexpr uint16_t USAGE_CONSUMER_AL_LOGOFF = 0x019C;
	static constexpr uint16_t USAGE_CONSUMER_AL_LOGON_LOGOFF = 0x019D;
	static constexpr uint16_t USAGE_CONSUMER_AL_TERMINAL_LOCK_SCREENSAVER = 0x019E;
	static constexpr uint16_t USAGE_CONSUMER_AL_CONTROL_PANEL = 0x019F;
	static constexpr uint16_t USAGE_CONSUMER_AL_COMMAND_LINE_PROCESSOR_RUN = 0x01A0;
	static constexpr uint16_t USAGE_CONSUMER_AL_PROCESS_TASK_MANAGER = 0x01A1;
	static constexpr uint16_t USAGE_CONSUMER_AL_SELECT_TASK_APPLICATION = 0x01A2;
	static constexpr uint16_t USAGE_CONSUMER_AL_NEXT_TASK_APPLICATION = 0x01A3;
	static constexpr uint16_t USAGE_CONSUMER_AL_PREVIOUS_TASK_APPLICATION = 0x01A4;
	static constexpr uint16_t USAGE_CONSUMER_AL_PREEMPTIVE_HALT_TASK_APPLICATION = 0x01A5;
	static constexpr uint16_t USAGE_CONSUMER_AL_INTEGRATED_HELP_CENTER = 0x01A6;
	static constexpr uint16_t USAGE_CONSUMER_AL_DOCUMENTS = 0x01A7;
	static constexpr uint16_t USAGE_CONSUMER_AL_THESAURUS = 0x01A8;
	static constexpr uint16_t USAGE_CONSUMER_AL_DICTIONARY = 0x01A9;
	static constexpr uint16_t USAGE_CONSUMER_AL_DESKTOP = 0x01AA;
	static constexpr uint16_t USAGE_CONSUMER_AL_SPELL_CHECK = 0x01AB;
	static constexpr uint16_t USAGE_CONSUMER_AL_GRAMMAR_CHECK = 0x01AC;
	static constexpr uint16_t USAGE_CONSUMER_AL_WIRELESS_STATUS = 0x01AD;
	static constexpr uint16_t USAGE_CONSUMER_AL_KEYBOARD_LAYOUT = 0x01AE;
	static constexpr uint16_t USAGE_CONSUMER_AL_VIRUS_PROTECTION = 0x01AF;
	static constexpr uint16_t USAGE_CONSUMER_AL_ENCRYPTION = 0x01B0;
	static constexpr uint16_t USAGE_CONSUMER_AL_SCREEN_SAVER = 0x01B1;
	static constexpr uint16_t USAGE_CONSUMER_AL_ALARMS = 0x01B2;
	static constexpr uint16_t USAGE_CONSUMER_AL_CLOCK = 0x01B3;
	static constexpr uint16_t USAGE_CONSUMER_AL_FILE_BROWSER = 0x01B4;
	static constexpr uint16_t USAGE_CONSUMER_AL_POWER_STATUS = 0x01B5;
	static constexpr uint16_t USAGE_CONSUMER_AL_IMAGE_BROWSER = 0x01B6;
	static constexpr uint16_t USAGE_CONSUMER_AL_AUDIO_BROWSER = 0x01B7;
	static constexpr uint16_t USAGE_CONSUMER_AL_MOVIE_BROWSER = 0x01B8;
	static constexpr uint16_t USAGE_CONSUMER_AL_DIGITAL_RIGHTS_MANAGER = 0x01B9;
	static constexpr uint16_t USAGE_CONSUMER_AL_DIGITAL_WALLET = 0x01BA;
	// 0x01BB-0x01BB Reserved
	static constexpr uint16_t USAGE_CONSUMER_AL_INSTANT_MESSAGING = 0x01BC;
	static constexpr uint16_t USAGE_CONSUMER_AL_OEM_FEATURES_TIPS_TUTORIAL_BROWSER = 0x01BD;
	static constexpr uint16_t USAGE_CONSUMER_AL_OEM_HELP = 0x01BE;
	static constexpr uint16_t USAGE_CONSUMER_AL_ONLINE_COMMUNITY = 0x01BF;
	static constexpr uint16_t USAGE_CONSUMER_AL_ENTERTAINMENT_CONTENT_BROWSER = 0x01C0;
	static constexpr uint16_t USAGE_CONSUMER_AL_ONLINE_SHOPPING_BROWSER = 0x01C1;
	static constexpr uint16_t USAGE_CONSUMER_AL_SMARTCARD_INFORMATION_HELP = 0x01C2;
	static constexpr uint16_t USAGE_CONSUMER_AL_MARKET_MONITOR_FINANCE_BROWSER = 0x01C3;
	static constexpr uint16_t USAGE_CONSUMER_AL_CUSTOMIZED_CORPORATE_NEWS_BROWSER = 0x01C4;
	static constexpr uint16_t USAGE_CONSUMER_AL_ONLINE_ACTIVITY_BROWSER = 0x01C5;
	static constexpr uint16_t USAGE_CONSUMER_AL_RESEARCH_SEARCH_BROWSER = 0x01C6;
	static constexpr uint16_t USAGE_CONSUMER_AL_AUDIO_PLAYER = 0x01C7;
	static constexpr uint16_t USAGE_CONSUMER_AL_MESSAGE_STATUS = 0x01C8;
	static constexpr uint16_t USAGE_CONSUMER_AL_CONTACT_SYNC = 0x01C9;
	static constexpr uint16_t USAGE_CONSUMER_AL_NAVIGATION = 0x01CA;
	static constexpr uint16_t USAGE_CONSUMER_AL_CONTEXT_AWARE_DESKTOP_ASSISTANT = 0x01CB;
	// 0x01CC-0x01FF Reserved
	static constexpr uint16_t USAGE_CONSUMER_GENERIC_GUI_APPLICATION_CONTROLS = 0x0200;
	static constexpr uint16_t USAGE_CONSUMER_AC_NEW = 0x0201;
	static constexpr uint16_t USAGE_CONSUMER_AC_OPEN = 0x0202;
	static constexpr uint16_t USAGE_CONSUMER_AC_CLOSE = 0x0203;
	static constexpr uint16_t USAGE_CONSUMER_AC_EXIT = 0x0204;
	static constexpr uint16_t USAGE_CONSUMER_AC_MAXIMIZE = 0x0205;
	static constexpr uint16_t USAGE_CONSUMER_AC_MINIMIZE = 0x0206;
	static constexpr uint16_t USAGE_CONSUMER_AC_SAVE = 0x0207;
	static constexpr uint16_t USAGE_CONSUMER_AC_PRINT = 0x0208;
	static constexpr uint16_t USAGE_CONSUMER_AC_PROPERTIES = 0x0209;
	// 0x020A-0x0219 Reserved
	static constexpr uint16_t USAGE_CONSUMER_AC_UNDO = 0x021A;
	static constexpr uint16_t USAGE_CONSUMER_AC_COPY = 0x021B;
	static constexpr uint16_t USAGE_CONSUMER_AC_CUT = 0x021C;
	static constexpr uint16_t USAGE_CONSUMER_AC_PASTE = 0x021D;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_ALL = 0x021E;
	static constexpr uint16_t USAGE_CONSUMER_AC_FIND = 0x021F;
	static constexpr uint16_t USAGE_CONSUMER_AC_FIND_AND_REPLACE = 0x0220;
	static constexpr uint16_t USAGE_CONSUMER_AC_SEARCH = 0x0221;
	static constexpr uint16_t USAGE_CONSUMER_AC_GO_TO = 0x0222;
	static constexpr uint16_t USAGE_CONSUMER_AC_HOME = 0x0223;
	static constexpr uint16_t USAGE_CONSUMER_AC_BACK = 0x0224;
	static constexpr uint16_t USAGE_CONSUMER_AC_FORWARD = 0x0225;
	static constexpr uint16_t USAGE_CONSUMER_AC_STOP = 0x0226;
	static constexpr uint16_t USAGE_CONSUMER_AC_REFRESH = 0x0227;
	static constexpr uint16_t USAGE_CONSUMER_AC_PREVIOUS_LINK = 0x0228;
	static constexpr uint16_t USAGE_CONSUMER_AC_NEXT_LINK = 0x0229;
	static constexpr uint16_t USAGE_CONSUMER_AC_BOOKMARKS = 0x022A;
	static constexpr uint16_t USAGE_CONSUMER_AC_HISTORY = 0x022B;
	static constexpr uint16_t USAGE_CONSUMER_AC_SUBSCRIPTIONS = 0x022C;
	static constexpr uint16_t USAGE_CONSUMER_AC_ZOOM_IN = 0x022D;
	static constexpr uint16_t USAGE_CONSUMER_AC_ZOOM_OUT = 0x022E;
	static constexpr uint16_t USAGE_CONSUMER_AC_ZOOM = 0x022F;
	static constexpr uint16_t USAGE_CONSUMER_AC_FULL_SCREEN_VIEW = 0x0230;
	static constexpr uint16_t USAGE_CONSUMER_AC_NORMAL_VIEW = 0x0231;
	static constexpr uint16_t USAGE_CONSUMER_AC_VIEW_TOGGLE = 0x0232;
	static constexpr uint16_t USAGE_CONSUMER_AC_SCROLL_UP = 0x0233;
	static constexpr uint16_t USAGE_CONSUMER_AC_SCROLL_DOWN = 0x0234;
	static constexpr uint16_t USAGE_CONSUMER_AC_SCROLL = 0x0235;
	static constexpr uint16_t USAGE_CONSUMER_AC_PAN_LEFT = 0x0236;
	static constexpr uint16_t USAGE_CONSUMER_AC_PAN_RIGHT = 0x0237;
	// USAGE_CONSUMER_AC_PAN is sometimes used in mice to implement horizontal scroll
	static constexpr uint16_t USAGE_CONSUMER_AC_PAN = 0x0238;
	static constexpr uint16_t USAGE_CONSUMER_AC_NEW_WINDOW = 0x0239;
	static constexpr uint16_t USAGE_CONSUMER_AC_TILE_HORIZONTALLY = 0x023A;
	static constexpr uint16_t USAGE_CONSUMER_AC_TILE_VERTICALLY = 0x023B;
	static constexpr uint16_t USAGE_CONSUMER_AC_FORMAT = 0x023C;
	static constexpr uint16_t USAGE_CONSUMER_AC_EDIT = 0x023D;
	static constexpr uint16_t USAGE_CONSUMER_AC_BOLD = 0x023E;
	static constexpr uint16_t USAGE_CONSUMER_AC_ITALICS = 0x023F;
	static constexpr uint16_t USAGE_CONSUMER_AC_UNDERLINE = 0x0240;
	static constexpr uint16_t USAGE_CONSUMER_AC_STRIKETHROUGH = 0x0241;
	static constexpr uint16_t USAGE_CONSUMER_AC_SUBSCRIPT = 0x0242;
	static constexpr uint16_t USAGE_CONSUMER_AC_SUPERSCRIPT = 0x0243;
	static constexpr uint16_t USAGE_CONSUMER_AC_ALL_CAPS = 0x0244;
	static constexpr uint16_t USAGE_CONSUMER_AC_ROTATE = 0x0245;
	static constexpr uint16_t USAGE_CONSUMER_AC_RESIZE = 0x0246;
	static constexpr uint16_t USAGE_CONSUMER_AC_FLIP_HORIZONTAL = 0x0247;
	static constexpr uint16_t USAGE_CONSUMER_AC_FLIP_VERTICAL = 0x0248;
	static constexpr uint16_t USAGE_CONSUMER_AC_MIRROR_HORIZONTAL = 0x0249;
	static constexpr uint16_t USAGE_CONSUMER_AC_MIRROR_VERTICAL = 0x024A;
	static constexpr uint16_t USAGE_CONSUMER_AC_FONT_SELECT = 0x024B;
	static constexpr uint16_t USAGE_CONSUMER_AC_FONT_COLOR = 0x024C;
	static constexpr uint16_t USAGE_CONSUMER_AC_FONT_SIZE = 0x024D;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_LEFT = 0x024E;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_CENTER_H = 0x024F;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_RIGHT = 0x0250;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_BLOCK_H = 0x0251;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_TOP = 0x0252;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_CENTER_V = 0x0253;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_BOTTOM = 0x0254;
	static constexpr uint16_t USAGE_CONSUMER_AC_JUSTIFY_BLOCK_V = 0x0255;
	static constexpr uint16_t USAGE_CONSUMER_AC_INDENT_DECREASE = 0x0256;
	static constexpr uint16_t USAGE_CONSUMER_AC_INDENT_INCREASE = 0x0257;
	static constexpr uint16_t USAGE_CONSUMER_AC_NUMBERED_LIST = 0x0258;
	static constexpr uint16_t USAGE_CONSUMER_AC_RESTART_NUMBERING = 0x0259;
	static constexpr uint16_t USAGE_CONSUMER_AC_BULLETED_LIST = 0x025A;
	static constexpr uint16_t USAGE_CONSUMER_AC_PROMOTE = 0x025B;
	static constexpr uint16_t USAGE_CONSUMER_AC_DEMOTE = 0x025C;
	static constexpr uint16_t USAGE_CONSUMER_AC_YES = 0x025D;
	static constexpr uint16_t USAGE_CONSUMER_AC_NO = 0x025E;
	static constexpr uint16_t USAGE_CONSUMER_AC_CANCEL = 0x025F;
	static constexpr uint16_t USAGE_CONSUMER_AC_CATALOG = 0x0260;
	static constexpr uint16_t USAGE_CONSUMER_AC_BUY_CHECKOUT = 0x0261;
	static constexpr uint16_t USAGE_CONSUMER_AC_ADD_TO_CART = 0x0262;
	static constexpr uint16_t USAGE_CONSUMER_AC_EXPAND = 0x0263;
	static constexpr uint16_t USAGE_CONSUMER_AC_EXPAND_ALL = 0x0264;
	static constexpr uint16_t USAGE_CONSUMER_AC_COLLAPSE = 0x0265;
	static constexpr uint16_t USAGE_CONSUMER_AC_COLLAPSE_ALL = 0x0266;
	static constexpr uint16_t USAGE_CONSUMER_AC_PRINT_PREVIEW = 0x0267;
	static constexpr uint16_t USAGE_CONSUMER_AC_PASTE_SPECIAL = 0x0268;
	static constexpr uint16_t USAGE_CONSUMER_AC_INSERT_MODE = 0x0269;
	static constexpr uint16_t USAGE_CONSUMER_AC_DELETE = 0x026A;
	static constexpr uint16_t USAGE_CONSUMER_AC_LOCK = 0x026B;
	static constexpr uint16_t USAGE_CONSUMER_AC_UNLOCK = 0x026C;
	static constexpr uint16_t USAGE_CONSUMER_AC_PROTECT = 0x026D;
	static constexpr uint16_t USAGE_CONSUMER_AC_UNPROTECT = 0x026E;
	static constexpr uint16_t USAGE_CONSUMER_AC_ATTACH_COMMENT = 0x026F;
	static constexpr uint16_t USAGE_CONSUMER_AC_DELETE_COMMENT = 0x0270;
	static constexpr uint16_t USAGE_CONSUMER_AC_VIEW_COMMENT = 0x0271;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_WORD = 0x0272;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_SENTENCE = 0x0273;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_PARAGRAPH = 0x0274;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_COLUMN = 0x0275;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_ROW = 0x0276;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_TABLE = 0x0277;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_OBJECT = 0x0278;
	static constexpr uint16_t USAGE_CONSUMER_AC_REDO_REPEAT = 0x0279;
	static constexpr uint16_t USAGE_CONSUMER_AC_SORT = 0x027A;
	static constexpr uint16_t USAGE_CONSUMER_AC_SORT_ASCENDING = 0x027B;
	static constexpr uint16_t USAGE_CONSUMER_AC_SORT_DESCENDING = 0x027C;
	static constexpr uint16_t USAGE_CONSUMER_AC_FILTER = 0x027D;
	static constexpr uint16_t USAGE_CONSUMER_AC_SET_CLOCK = 0x027E;
	static constexpr uint16_t USAGE_CONSUMER_AC_VIEW_CLOCK = 0x027F;
	static constexpr uint16_t USAGE_CONSUMER_AC_SELECT_TIME_ZONE = 0x0280;
	static constexpr uint16_t USAGE_CONSUMER_AC_EDIT_TIME_ZONES = 0x0281;
	static constexpr uint16_t USAGE_CONSUMER_AC_SET_ALARM = 0x0282;
	static constexpr uint16_t USAGE_CONSUMER_AC_CLEAR_ALARM = 0x0283;
	static constexpr uint16_t USAGE_CONSUMER_AC_SNOOZE_ALARM = 0x0284;
	static constexpr uint16_t USAGE_CONSUMER_AC_RESET_ALARM = 0x0285;
	static constexpr uint16_t USAGE_CONSUMER_AC_SYNCHRONIZE = 0x0286;
	static constexpr uint16_t USAGE_CONSUMER_AC_SEND_RECEIVE = 0x0287;
	static constexpr uint16_t USAGE_CONSUMER_AC_SEND_TO = 0x0288;
	static constexpr uint16_t USAGE_CONSUMER_AC_REPLY = 0x0289;
	static constexpr uint16_t USAGE_CONSUMER_AC_REPLY_ALL = 0x028A;
	static constexpr uint16_t USAGE_CONSUMER_AC_FORWARD_MSG = 0x028B;
	static constexpr uint16_t USAGE_CONSUMER_AC_SEND = 0x028C;
	static constexpr uint16_t USAGE_CONSUMER_AC_ATTACH_FILE = 0x028D;
	static constexpr uint16_t USAGE_CONSUMER_AC_UPLOAD = 0x028E;
	static constexpr uint16_t USAGE_CONSUMER_AC_DOWNLOAD = 0x028F;
	static constexpr uint16_t USAGE_CONSUMER_AC_SET_BORDERS = 0x0290;
	static constexpr uint16_t USAGE_CONSUMER_AC_INSERT_ROW = 0x0291;
	static constexpr uint16_t USAGE_CONSUMER_AC_INSERT_COLUMN = 0x0292;
	static constexpr uint16_t USAGE_CONSUMER_AC_INSERT_FILE = 0x0293;
	static constexpr uint16_t USAGE_CONSUMER_AC_INSERT_PICTURE = 0x0294;
	static constexpr uint16_t USAGE_CONSUMER_AC_INSERT_OBJECT = 0x0295;
	static constexpr uint16_t USAGE_CONSUMER_AC_INSERT_SYMBOL = 0x0296;
	static constexpr uint16_t USAGE_CONSUMER_AC_SAVE_AND_CLOSE = 0x0297;
	static constexpr uint16_t USAGE_CONSUMER_AC_RENAME = 0x0298;
	static constexpr uint16_t USAGE_CONSUMER_AC_MERGE = 0x0299;
	static constexpr uint16_t USAGE_CONSUMER_AC_SPLIT = 0x029A;
	static constexpr uint16_t USAGE_CONSUMER_AC_DISRIBUTE_HORIZONTALLY = 0x029B;
	static constexpr uint16_t USAGE_CONSUMER_AC_DISTRIBUTE_VERTICALLY = 0x029C;
	static constexpr uint16_t USAGE_CONSUMER_AC_NEXT_KEYBOARD_LAYOUT_SELECT = 0x029D;
	static constexpr uint16_t USAGE_CONSUMER_AC_NAVIGATION_GUIDANCE = 0x029E;
	static constexpr uint16_t USAGE_CONSUMER_AC_DESKTOP_SHOW_ALL_WINDOWS = 0x029F;
	static constexpr uint16_t USAGE_CONSUMER_AC_SOFT_KEY_LEFT = 0x02A0;
	static constexpr uint16_t USAGE_CONSUMER_AC_SOFT_KEY_RIGHT = 0x02A1;
	static constexpr uint16_t USAGE_CONSUMER_AC_DESKTOP_SHOW_ALL_APPLICATIONS = 0x02A2;
	// 0x02A3-0x02AF Reserved
	static constexpr uint16_t USAGE_CONSUMER_AC_IDLE_KEEP_ALIVE = 0x02B0;
	// 0x02B1-0x02BF Reserved
	static constexpr uint16_t USAGE_CONSUMER_EXTENDED_KEYBOARD_ATTRIBUTES_COLLECTION = 0x02C0;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_FORM_FACTOR = 0x02C1;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_KEY_TYPE = 0x02C2;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_PHYSICAL_LAYOUT = 0x02C3;
	static constexpr uint16_t USAGE_CONSUMER_VENDOR_SPECIFIC_KEYBOARD_PHYSICAL_LAYOUT = 0x02C4;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_IETF_LANGUAGE_TAG_INDEX = 0x02C5;
	static constexpr uint16_t USAGE_CONSUMER_IMPLEMENTED_KEYBOARD_INPUT_ASSIST_CONTROLS = 0x02C6;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_INPUT_ASSIST_PREVIOUS = 0x02C7;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_INPUT_ASSIST_NEXT = 0x02C8;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_INPUT_ASSIST_PREVIOUS_GROUP = 0x02C9;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_INPUT_ASSIST_NEXT_GROUP = 0x02CA;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_INPUT_ASSIST_ACCEPT = 0x02CB;
	static constexpr uint16_t USAGE_CONSUMER_KEYBOARD_INPUT_ASSIST_CANCEL = 0x02CC;
	// 0x02CD-0x02CF Reserved
	static constexpr uint16_t USAGE_CONSUMER_PRIVACY_SCREEN_TOGGLE = 0x02D0;
	static constexpr uint16_t USAGE_CONSUMER_PRIVACY_SCREEN_LEVEL_DECREMENT = 0x02D1;
	static constexpr uint16_t USAGE_CONSUMER_PRIVACY_SCREEN_LEVEL_INCREMENT = 0x02D2;
	static constexpr uint16_t USAGE_CONSUMER_PRIVACY_SCREEN_LEVEL_MINIMUM = 0x02D3;
	static constexpr uint16_t USAGE_CONSUMER_PRIVACY_SCREEN_LEVEL_MAXIMUM = 0x02D4;
	// 0x02D5-0x04FF Reserved
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_EDITED = 0x0500;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_ADDED = 0x0501;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_RECORD_ACTIVE = 0x0502;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_INDEX = 0x0503;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_NICKNAME = 0x0504;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_FIRST_NAME = 0x0505;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_LAST_NAME = 0x0506;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_FULL_NAME = 0x0507;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_PHONE_NUMBER_PERSONAL = 0x0508;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_PHONE_NUMBER_BUSINESS = 0x0509;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_PHONE_NUMBER_MOBILE = 0x050A;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_PHONE_NUMBER_PAGER = 0x050B;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_PHONE_NUMBER_FAX = 0x050C;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_PHONE_NUMBER_OTHER = 0x050D;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_EMAIL_PERSONAL = 0x050E;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_EMAIL_BUSINESS = 0x050F;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_EMAIL_OTHER = 0x0510;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_EMAIL_MAIN = 0x0511;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_SPEED_DIAL_NUMBER = 0x0512;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_STATUS_FLAG = 0x0513;
	static constexpr uint16_t USAGE_CONSUMER_CONTACT_MISC = 0x0514;
	// 0x0515-0xFFFF Reserved


	template <typename T>
	const T& _hrp_min(const T& a, const T& b) {
		return a < b ? a : b;
	}

	template <typename T>
	const T& _hrp_max(const T& a, const T& b) {
		return a > b ? a : b;
	}

	// In case of a C++14 compiler this template could be moved into the body of
	// _narrowest_unsigned_integer to avoid the pollution of this namespace.
	template<int BITS> struct _t_uint;
	template<> struct _t_uint <8> { typedef uint8_t type; };
	template<> struct _t_uint<16> { typedef uint16_t type; };
	template<> struct _t_uint<32> { typedef uint32_t type; };
	template<> struct _t_uint<64> { typedef uint64_t type; };

	template<uint64_t MAX_VALUE>
	struct _narrowest_unsigned_integer {
		enum {
			REQUIRED_BITS = MAX_VALUE <= 0xff ? 8 : MAX_VALUE <= 0xffff ? 16 : MAX_VALUE <= 0xffffffff ? 32 : 64
		};
		typedef typename _t_uint<REQUIRED_BITS>::type type;
	};

	struct UsageRange {
		uint16_t usage_page;
		uint16_t usage_min;
		uint16_t usage_max;
	};
	
	struct StringRange {
        uint8_t string_min;
        uint8_t string_max;
	};

	static constexpr uint8_t COLLECTION_TYPE_PHYSICAL       = 0x00;
	static constexpr uint8_t COLLECTION_TYPE_APPLICATION    = 0x01;
	static constexpr uint8_t COLLECTION_TYPE_LOGICAL        = 0x02;
	static constexpr uint8_t COLLECTION_TYPE_REPORT         = 0x03;
	static constexpr uint8_t COLLECTION_TYPE_NAMED_ARRAY    = 0x04;
	static constexpr uint8_t COLLECTION_TYPE_USAGE_SWITCH   = 0x05;
	static constexpr uint8_t COLLECTION_TYPE_USAGE_MODIFIER = 0x06;
	static constexpr uint8_t COLLECTION_TYPE_VENDOR_MIN     = 0x80;
	static constexpr uint8_t COLLECTION_TYPE_VENDOR_MAX     = 0xFF;

	static constexpr uint16_t FLAG_FIELD_CONST          = 0x0001;  // 0: data         1: const
	static constexpr uint16_t FLAG_FIELD_VARIABLE       = 0x0002;  // 0: array        1: var
	static constexpr uint16_t FLAG_FIELD_RELATIVE       = 0x0004;  // 0: abs          1: rel
	static constexpr uint16_t FLAG_FIELD_WRAP           = 0x0008;  // 0: no_wrap      1: wrap
	static constexpr uint16_t FLAG_FIELD_NON_LINEAR     = 0x0010;  // 0: linear       1: non_linear
	static constexpr uint16_t FLAG_FIELD_NO_PREFERRED   = 0x0020;  // 0: preferred    1: no_preferred
	static constexpr uint16_t FLAG_FIELD_NULL_STATE     = 0x0040;  // 0: no_null      1: null
	static constexpr uint16_t FLAG_FIELD_VOLATILE       = 0x0080;  // 0: non_volatile 1: volatile
	static constexpr uint16_t FLAG_FIELD_BUFFERED_BYTES = 0x0100;  // 0: bit_field    1: buffered_bytes

	enum class ReportType : uint8_t { input = 0, output = 1, feature = 2, count = 3 };


	// DescriptorParser parses a binary HID report descriptor and passes the
	// parsed main items and their parameters to callback functions.
	//
	// The callbacks can use the received parameters to:
	// - Build a HID report parser
	// - Build a HID report composer
	// - Record only specific data points of interest (for example the usages
	//   of the top-level application containers)
	//
	// Some applications may need all or most of the building blocks emitted by
	// the parser. In certain situations you need only some specific pieces (for
	// example the the most essential controls of a mouse: 2-3 buttons and the
	// X/Y movement) so you can save memory by ignoring the rest of the descriptor.
	//
	// This parser doesn't perform dynamic memory allocation - it works with a
	// fixed amount of memory (sizeof(DescriptorParser)) and a limited amount of
	// stack space to execute the Parse method. This is by design to make the
	// parser usable on microcontrollers (Arduinos) with limited resources.
	//
	// Unsupported items that are ignored/skipped by this parser:
	// - LONG items
	// - STRING_INDEX, STRING_MIN, STRING_MAX
	// - DESIGNATOR INDEX, DESIGNATOR_MIN, DESIGNATOR_MAX
	// - DELIMITER: A delimiter set can assign multiple alternative usages to a
	//   single control. I haven't yet seen a real-life example to a device
	//   utilising delimiter sets. Perhaps because mapping controls to usages
	//   is best done by applications that are easier to modify than the
	//   firmware (which includes the HID descriptors) of an input device.
	//
	//   Linux, MacOS and Win10 implement delimiters in 3 completely different
	//   ways and none of them seem to follow the HID specification but I can't
	//   blame them for not caring about a feature that isn't useful and isn't
	//   used in practice by HID descriptors. The spec doesn't show how to use
	//   delimiters so implementing a parser involves some guesswork too.
	//   - MacOS simply ignores DELIMITER OPEN/CLOSE items and processes the
	//     enclosed USAGEs like the ones that are outside of delimiter sets.
	//     This is exactly what our parser will also do.
	//   - Linux parses all USAGEs before the second DELIMITER(OPEN) item and
	//     ignores the rest of the USAGEs (including those that aren't inside
	//     delimiter sets) until the next INPUT/OUTPUT/FEATURE item.
	//     It processes all USAGEs in the first DELIMITER set the same way it
	//     normally does without delimiters. I have no idea what the logic is
	//     behind this behaviour but it may be a misinterpretation of the spec.
	//     My interpretation of the HID specification: A delimiter set assigns
	//     one USAGE to one control but provides optional alternative USAGEs.
	//     Parsers that don't want to support alternative USAGEs should pick the
	//     first USAGE and ignore the rest of the USAGES declared inside of the
	//     delimiter set. When the set closes continue parsing USAGEs as usual.
	//   - Windows 10 seemed to ignore all USAGEs that I declared inside
	//     delimiter sets.
	class DescriptorParser {
	public:
		struct Globals {
			uint8_t report_id; // zero means no report_id

			uint16_t usage_page;

			int32_t logical_min;
			int32_t logical_max;
#if HRP_ENABLE_PHYISICAL_UNITS
			int32_t physical_min;
			int32_t physical_max;
			uint32_t unit;
			uint32_t unit_exponent;
#endif
			uint32_t report_size;
			uint32_t report_count;

			void Reset() {
				memset((void*)this, 0, sizeof(*this));
			}
		};

		struct FieldParams {
			ReportType report_type;

			// A combination of FLAG_FIELD_* constants.
			uint16_t flags;

			// globals.logical_min is guaranteed to be less than globals.logical_max.
			// If globals.logical_min isn't negative then globals.logical_max
			// should be used as an uint32_t.
			// The max value of globals.report_count is HRP_MAX_REPORT_COUNT.
			// The max value of globals.report_size is HRP_MAX_REPORT_SIZE.
			const Globals* globals;

			// bit_size is never zero.
			// Equals to globals->report_size * globals->report_count.
			uint32_t bit_size;

			// All fields are set in all UsageRanges and usage_max is never
			// less than usage_min.
			// This array isn't const because it will be reset anyway when the
			// field callback returns. For this reason the callback is allowed
			// to modify it if that helps.
			UsageRange* usage_ranges;

			// num_usage_ranges is never zero
			uint16_t num_usage_ranges;

			// string usages
            StringRange* string_ranges;

			// number of string usages
            uint16_t num_string_ranges;
		};

		// If you use the DescriptorParser with a DescriptorParser::EventHandler
		// implementation provided by your application then its methods can also
		// return errors. Any non-zero error code returned by your EventHandler
		// is returned unmodified by the DescriptorParser::Parse method. You can
		// distinguish your own error codes from library error codes by using
		// positive values because this library defines only negative ones.
		class EventHandler {
			friend class DescriptorParser;
		protected:
			virtual int Field(const FieldParams& fp) { return 0; }
			virtual int Padding(ReportType rt, uint8_t report_id, uint32_t bit_size) { return 0; }

			// The lowest depth value received by BeginCollection and EndCollection
			// calls is 1. The parser increases depth before each BeginCollection
			// call and decreases it after each EndCollection call.
			// The collection_type parameter may be one of the COLLECTION_TYPE_*
			// constants or a vendor-defined value.
			virtual int BeginCollection(uint8_t collection_type, uint16_t usage_page, uint16_t usage, uint32_t depth) { return 0; }
			// Unmatched END_COLLECTION items are checked by the parser so this
			// callback doesn't have to.
			virtual int EndCollection(uint32_t depth) { return 0; }
		};

		// DescriptorParser instances are reusable.
		// You can call Parse more than once on the same instance.
		int Parse(const void* descriptor, size_t descriptor_size, EventHandler* handler);

	private:
		int AssertMinMaxItemsAreMatched();
		int ParseMainItems(uint8_t item, const uint8_t* p_data, uint8_t data_size, EventHandler* handler);
		int AddField(ReportType rt, const uint8_t* p_data, uint8_t data_size, EventHandler* handler);
		int ParseGlobalItems(uint8_t item, const uint8_t* p_data, uint8_t data_size);
		int ParseLocalItems(uint8_t item, const uint8_t* p_data, uint8_t data_size);

		struct Locals {
			static constexpr uint8_t FLAG_USAGE_MIN = 1;	// set after finding a USAGE_MIN, reset after finding the related USAGE_MAX
			static constexpr uint8_t FLAG_USAGE_MAX = 2;	// set after finding a USAGE_MAX, reset after finding the related USAGE_MIN
			uint8_t flags;

			_narrowest_unsigned_integer<HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM>::type num_usage_ranges;
			UsageRange usage_ranges[HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM];

			_narrowest_unsigned_integer<HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM>::type num_string_ranges;
			StringRange string_ranges[HRP_MAX_USAGE_RANGES_PER_MAIN_ITEM];
			

			void Reset() {
				memset(this, 0, sizeof(*this));
			}

			uint16_t FirstUsage() const {
				return num_usage_ranges ? usage_ranges[0].usage_min : 0;
			}

			bool AddUsageRange(uint16_t usage_min, uint16_t usage_max, uint16_t usage_page);

			bool AddStringRange(uint8_t string_min, uint8_t string_max);
		};

		void Reset() {
			_locals.Reset();
			_globals.Reset();
			_collection_depth = 0;
			_first_field_processed = false;
			_first_field_has_report_id = false;
			_globals_stack_size = 0;

		}

	private:
		Locals _locals;
		Globals _globals;

		uint32_t _collection_depth;

		bool _first_field_processed: 1;
		bool _first_field_has_report_id: 1;

		_narrowest_unsigned_integer<HRP_MAX_PUSH_POP_STACK_SIZE>::type _globals_stack_size;
		Globals _global_stack[HRP_MAX_PUSH_POP_STACK_SIZE];
	};


	// The methods of IInt32Target are called only by SelectiveInputReportParser::Init.
	// After Init the IInt32Target instance isn't anymore needed but the buffer
	// pointer returned by its Data() method may be stored and used by the
	// SelectiveInputReportParser instance whenever you call its
	// SelectiveInputReportParser::Parse method to parse an input report.
	class IInt32Target {
		friend class SelectiveInputReportParser;
	protected:
		// Reset allocates a buffer for the int32_t array if necessary and
		// resets all values in it to zero.
		virtual void Reset(size_t num_int32s) = 0;
		// Returns a pointer to the int32_t array that was initialised by Reset.
		virtual int32_t* Data() = 0;
	};


	// The methods of IBoolTarget are called only by SelectiveInputReportParser::Init.
	// After Init the IBoolTarget instance isn't anymore needed but the buffer
	// pointer returned by its Data() method may be stored and used by the
	// SelectiveInputReportParser instance whenever you call its
	// SelectiveInputReportParser::Parse method to parse an input report.
	class IBoolTarget {
		friend class SelectiveInputReportParser;
	protected:
		// Reset allocates a buffer for the bitfield if necessary and resets all
		// bool values in it to false.
		virtual void Reset(size_t bit_size) = 0;
		// Returns a pointer to the bitfield that was initialised by Reset.
		virtual uint8_t* Data() = 0;
	};


	// Int32ArrayRef provides a low level interface to specify the exact memory
	// locations of the int32 variables that are updated when an input report is
	// parsed by the SelectiveInputReportParser::Parse method.
	//
	// The Int32ArrayRef instance is used only by the
	// SelectiveInputReportParser::Init call that maps report fields onto
	// variables. After the Init call it can be deleted just like the other
	// parts of the mapping configuration used by Init: that includes
	// Collection, Int32Fields and BoolFields instances.
	template <size_t SIZE>
	class Int32ArrayRef : public IInt32Target {
	public:
		Int32ArrayRef(int32_t* ref) : _ref(ref) {}
	protected:
		void Reset(size_t num_int32s) override {
			assert(num_int32s <= SIZE);
			memset(_ref, 0, sizeof(int32_t)*num_int32s);
		}
		int32_t* Data() override {
			return _ref;
		}
	private:
		int32_t* _ref;
	};

	template <size_t SIZE>
	Int32ArrayRef<SIZE> int32_array_ref(int32_t(&arr)[SIZE]) {
		return Int32ArrayRef<SIZE>(arr);
	}


	// Optional convenience template that declares an int32_t array with bounds
	// checking and a utility method to create an Int32ArrayRef.
	// Using this template is optional. Instead you can define your own int32_t
	// array and reference it with an Int32ArrayRef.
	template <size_t SIZE_>
	struct Int32Array {
		static constexpr size_t SIZE = SIZE_;
		int32_t items[SIZE];

		// The number of valid zero-based indexes depends on the number of
		// usages declared in the 'usages' field of the Int32Fields instance
		// that references this object through its 'target' field.
		int32_t operator[](size_t index) const {
			assert(index < SIZE);
			return items[index];
		}

		Int32ArrayRef<SIZE> Ref() {
			return Int32ArrayRef<SIZE>(items);
		}
	};


	// Int32Vector is a convenient but wasteful way to specify the target
	// variables for an Int32Fields mapping configuration.
	// It is wasteful because internally it uses an std::vector and has a vptr
	// as a consequence of implementing the IBoolTarget interface.
	// It is convenient because it gets resized automatically by
	// SelectiveInputReportParser::Init calls so you don't have to fiddle with
	// array sizes while prototyping. The tests of the library make heavy use
	// of this class.
	//
	// A more efficient alternative is Int32ArrayRef that gives lower level
	// control over memory management.
	class Int32Vector : public IInt32Target {
	public:
		// The number of valid zero-based indexes depends on the number of
		// usages declared in the 'usages' field of the Int32Fields instance
		// that references this object through its 'target' field.
		int32_t operator[](size_t index) const {
			return _ints[index];
		}

	protected:
		void Reset(size_t num_int32s) override {
			_ints.assign(num_int32s, 0);
		}
		int32_t* Data() override {
			return _ints.data();
		}

	private:
		std::vector<int32_t> _ints;
	};


	// BitFieldRef provides a low level interface to specify the exact memory
	// location of a bit field that is updated when an input report is parsed by
	// the SelectiveInputReportParser::Parse method.
	//
	// The BitFieldRef instance is used only by the
	// SelectiveInputReportParser::Init call that maps report fields onto
	// variables. After the Init call it can be deleted just like the other
	// parts of the mapping configuration used by Init: that includes
	// Collection, Int32Fields and BoolFields instances.
	template <size_t BIT_SIZE>
	class BitFieldRef : public IBoolTarget {
	public:
		BitFieldRef(uint8_t* ref) : _ref(ref) {}
	protected:
		void Reset(size_t bit_size) override {
			assert(bit_size <= BIT_SIZE);
			memset(_ref, 0, (bit_size+7)/8);
		}
		uint8_t* Data() override {
			return _ref;
		}
	private:
		uint8_t* _ref;
	};

	template <size_t BYTE_SIZE>
	BitFieldRef<BYTE_SIZE*8> bitfield_ref(uint8_t(&arr)[BYTE_SIZE]) {
		return BitFieldRef<BYTE_SIZE*8>(arr);
	}


	// Optional convenience template that declares a bitfield with bounds
	// checking and a utility method to create a BitFieldRef.
	// Using this template is optional. Instead you can define your own byte
	// array and reference it with a BitFieldRef.
	template <size_t BIT_SIZE_>
	struct BitField {
		static constexpr size_t BIT_SIZE = BIT_SIZE_;
		static constexpr size_t BYTE_SIZE = (BIT_SIZE + 7) / 8;
		uint8_t bytes[BYTE_SIZE];

		bool operator[](size_t bit_index) const {
			assert(bit_index < BIT_SIZE);
			return 0 != (uint8_t)(bytes[bit_index >> 3] & (1 << (bit_index & 7)));
		}

		// Flags makes it possible to access the bitfield as an integer array.
		// For example Flags<uint64_t>(0), Flags<uint64_t>(1) and Flags<uint64_t>(2)
		// can be used to access the first 192 bits of a bitfield.
		// The Flags<uint64_t>(1) call is valid only if the length of the
		// bitfield is at least 64+1. If the length is only 64 bits or less then
		// the only valid index is zero through the Flags<uint64_t>(0) call.
		// Indexing works similarly with other integer types.
		// The least significant bits of the returned integer correspond to
		// lower indexes of the bitfield.
		template <typename t_integer>
		t_integer Flags(size_t index) const {
			assert(sizeof(t_integer)*index < BYTE_SIZE);

			if (sizeof(t_integer) == 1)
				return bytes[index];

			size_t e = index * sizeof(t_integer) - 1;
			size_t i = _hrp_min(e + sizeof(t_integer), BYTE_SIZE-1);
			t_integer v = bytes[i--];
			while (i != e) {
				v <<= 8;
				v |= bytes[i--];
			}
			return v;
		}

		BitFieldRef<BIT_SIZE> Ref() {
			return BitFieldRef<BIT_SIZE>(bytes);
		}
	};


	// BoolVector is a convenient but wasteful way to specify the target
	// variables for a BoolFields mapping configuration.
	// It is wasteful because internally it uses an std::vector and has a vptr
	// as a consequence of implementing the IBoolTarget interface.
	// It is convenient because it gets resized automatically by
	// SelectiveInputReportParser::Init calls so you don't have to fiddle with
	// array sizes while prototyping. The tests of the library make heavy use
	// of this class.
	//
	// A more efficient alternative is BitFieldRef that gives lower level
	// control over memory management.
	class BoolVector : public IBoolTarget {
	public:
		// The number of valid zero-based indexes depends on the number of
		// usages declared in the 'usages' field of the BoolFields instance
		// that references this object through its 'target' field.
		bool operator[](size_t index) const {
			return 0 != (uint8_t)(_bytes[index >> 3] & (1 << (index & 7)));
		}

		// Flags makes it possible to access the bitfield as an integer array.
		// For example Flags<uint64_t>(0), Flags<uint64_t>(1) and Flags<uint64_t>(2)
		// can be used to access the first 192 bits of a bitfield.
		// The Flags<uint64_t>(1) call is valid only if the length of the
		// bitfield is at least 64+1. If the length is only 64 bits or less then
		// the only valid index is zero through the Flags<uint64_t>(0) call.
		// Indexing works similarly with other integer types.
		// The least significant bits of the returned integer correspond to
		// lower indexes of the bitfield.
		template <typename t_integer>
		t_integer Flags(size_t index) const {
			if (sizeof(t_integer) == 1)
				return _bytes[index];

			size_t e = index * sizeof(t_integer);
			assert(e < _bytes.size());
			e -= 1;
			size_t i = _hrp_min(e + sizeof(t_integer), _bytes.size()-1);
			t_integer v = _bytes[i--];
			while (i != e) {
				v <<= 8;
				v |= _bytes[i--];
			}
			return v;
		}

	protected:
		void Reset(size_t bit_size) override {
			_bytes.assign((bit_size+7)/8, 0);
		}
		uint8_t* Data() override {
			return _bytes.data();
		}

	private:
		std::vector<uint8_t> _bytes;
	};


	// Int32Fields defines mappings between int32 variables of the application
	// and integer fields of the report that can be narrower than 32 bits. The
	// report fields get zero- or sign-extended to 32 bit integers.
	// Signedness depends on the logical min/max values of the field.
	//
	// Int32Fields, BoolFields and Collection instances together provide the
	// mapping configuration that is used by the SelectiveInputReportParser::Init
	// method to create the mapping used by SelectiveInputReportParser::Parse.
	struct Int32Fields {
		// The target field specifies the int32 variables into which the
		// SelectiveInputReportParser::Parse method will store the parsed report
		// field values. If you don't set a target the related report fields
		// aren't even mapped by the SelectiveInputReportParser::Init method.
		// Two or more Int32Fields instances must not reference the same target.
		IInt32Target* target = nullptr;
		// you can leave UsageRange.usage_max zero if you want to specify only one usage (in usage_min)
		std::vector<UsageRange> usages;

		// The specified mask and flags are used the following way to filter the fields:
		// field_included = ((field_flags & mask) == flags)
		uint16_t mask = 0;
		uint16_t flags = 0;

		// In most cases you are better off with designated initializers than
		// the convenience methods below.

		Int32Fields& SetTarget(IInt32Target* t) { target = t; return *this; }
		Int32Fields& SetFlags(uint16_t mask_, uint16_t flags_) { mask = mask_; flags = flags_; return *this; }

		Int32Fields& AddUsages(UsageRange&& r) { usages.push_back(std::move(r)); return *this; }
		Int32Fields& AddUsages(std::initializer_list<UsageRange> a) { usages.insert(usages.end(), a); return *this; }

		struct FieldProperties {
			uint16_t flags;
			int32_t logical_min;
			int32_t logical_max;
#if HRP_ENABLE_PHYISICAL_UNITS
			int32_t physical_min;
			int32_t physical_max;
			uint32_t unit;
			uint32_t unit_exponent;
#endif
		};

		// The 'properties' and 'mapped' vectors are populated by successful
		// calls to SelectiveInputReportParser::Init().
		// The number of items in these vectors equals to the number of usages
		// you specified in the 'usages' field.

		std::vector<FieldProperties> properties;
		std::vector<bool> mapped;
	};


	// BoolFields defines mappings between bool variables (a bitfield) of the
	// application and bool fields of the report. Long bitfields are very common
	// in reports that transfer the states of gamepad buttons or keyboard keys.
	// Handling those bool values as larger blocks (as bitfields) is more
	// efficient in most cases.
	//
	// Int32Fields, BoolFields and Collection instances together provide the
	// mapping configuration that is used by the SelectiveInputReportParser::Init
	// method to create the mapping used by SelectiveInputReportParser::Parse.
	struct BoolFields {
		// The target field specifies the bool variables into which the
		// SelectiveInputReportParser::Parse method will store the parsed report
		// field values. If you don't set a target the related report fields
		// aren't even mapped by the SelectiveInputReportParser::Init method.
		// Two or more BoolFields instances must not reference the same target.
		IBoolTarget* target = nullptr;
		// you can leave UsageRange.usage_max zero if you want to specify only one usage (in usage_min)
		std::vector<UsageRange> usages;

		// The specified mask and flags are used the following way to filter the fields:
		// field_included = ((field_flags & mask) == flags)
		uint16_t mask = 0;
		uint16_t flags = 0;

		// In most cases you are better off with designated initializers than
		// the convenience methods below.

		BoolFields& SetTarget(IBoolTarget* t) { target = t; return *this; }
		BoolFields& SetFlags(uint16_t mask_, uint16_t flags_) { mask = mask_; flags = flags_; return *this; };

		BoolFields& AddUsages(UsageRange&& r) { usages.push_back(std::move(r)); return *this; }
		BoolFields& AddUsages(std::initializer_list<UsageRange> a) { usages.insert(usages.end(), a); return *this; }

		// The 'mapped' vector is populated by successful calls to
		// SelectiveInputReportParser::Init().
		// The number of items in this vector equals to the number of usages
		// you specified in the 'usages' field.

		std::vector<bool> mapped;
	};

	// The SelectiveInputReportParser::Init method expects a Collection instance
	// (the "config root") as one of its parameters to specify which fields we
	// want to extract from input reports and to filter for certain attributes
	// of those fields. That config can be used to filter based on the flags of
	// a descriptor field and/or its approximate location within the descriptor
	// (e.g.: a button field somewhere inside a collection that is tagged with
	// the Desktop/Mouse usage).
	//
	// The config Collection hierarchy is matched against the descriptor's
	// collection hierarchy in a relaxed way:
	// - An int32 or bool field of the config hierarchy (basically a single
	//   usage declared within an Int32Fields or BoolFields instance) can be
	//   mapped onto a descriptor field only if all of its direct and indirect
	//   parent Collection instances (up to root Collection instance) can be
	//   mapped onto the direct and indirect parent collections of the
	//   descriptor field. A direct parent-child relationship in the config
	//   hierarchy can be mapped onto an indirect parent-child relationship in
	//   the descriptor. A direct parent-child relationship between two config
	//   Collection instances can be mapped even onto a single descriptor
	//   collection if both config Collection instances match the descriptor
	//   collection with their filters (collection type and usages).
	// - The root Collection of the config hierarchy can be mapped onto
	//   descriptor collections that aren't top level collections. The root of
	//   the config hierarchy isn't "anchored" to the root of the descriptor.
	// - As a special case the usages of the Int32Fields and BoolFields of the
	//   root Collection instance can be mapped onto top-level descriptor fields
	//   (that aren't inside descriptor collections) but only if the root config
	//   Collection defines no collection type or usage filters. Such top level
	//   descriptor fields aren't allowed by the HID specification but some HID
	//   drivers (like the one in Linux) accept them in some cases. Fortunately
	//   devices never seem to declare such top level descriptor fields.
	// - If a descriptor field can be mapped onto more than one usage (declared
	//   inside Int32Fields and/or BoolFields instances) in the config hierarchy
	//   then the deeper (more specific) instance wins. If they have the same
	//   depth in the hierarchy then we pick the first one based on the simplest
	//   possible ordering.
	struct Collection {
		// type==0 doesn't filter for collection type.
		uint8_t type = 0;
		// An empty usages vector doesn't filter for usages.
		// You can set UsageRange.usage_min and leave UsageRange.usage_max zero
		// if you want to specify only one usage.
		// If you specify more than one usages then any one of them can match
		// the type of a descriptor collection.
		std::vector<UsageRange> usages;

		// Child config collections can be mapped either onto same descriptor
		// collection as the parent or a direct/indirection child of that
		// descriptor collection.
		std::vector<Collection*> collections;

		// Int32Fields and BoolFields can be mapped onto descriptor fields that
		// are direct or indirect children of the descriptor collection that is
		// mapped onto the config Collection that contains these Int32Fields and
		// BoolFields instances.
		//
		// A single Int32Fields or BoolFields instance can be used to map
		// multiple fields. You need two or more Int32Fields or BoolFields
		// instances only if you want to specify different mask/flags in them.
		std::vector<Int32Fields*> int32s;
		std::vector<BoolFields*> bools;

		// In most cases you are better off with designated initializers than
		// the convenience methods below.

		Collection& SetType(uint8_t t) { type = t; return *this; }
		Collection& AddUsages(UsageRange&& r) { usages.push_back(std::move(r)); return *this; }
		Collection& AddUsages(std::initializer_list<UsageRange> a) { usages.insert(usages.end(), a); return *this; }

		Collection& AddCollection(Collection* c) { collections.push_back(c); return *this; }
		Collection& AddCollections(std::initializer_list<Collection*> a) { collections.insert(collections.end(), a); return *this; }

		Collection& AddInt32Fields(Int32Fields* f) { int32s.push_back(f); return *this; }
		Collection& AddInt32Fields(std::initializer_list<Int32Fields*> a) { int32s.insert(int32s.end(), a); return *this; }

		Collection& AddBoolFields(BoolFields* f) { bools.push_back(f); return *this; }
		Collection& AddBoolFields(std::initializer_list<BoolFields*> a) { bools.insert(bools.end(), a); return *this; }
	};


	// SelectiveInputParser tries to find specific fields in input reports and
	// extract them by ignoring everything else. The field values are parsed into
	// variables you define for your application with the help of the Collection,
	// Int32Fields and BoolFields classes.
	//
	// Situations in which this report parser can help:
	// - You need access to specific fields (like mouse movement, X/Y position
	//   of a gamepad, button events) of an input report received from an input
	//   device.
	//
	// Situations in which this report parser can't help:
	// - You want to enumerate all report fields declared in a HID descriptor.
	// - You want to create an output or feature report (based on a HID descriptor)
	//   and set the values of its fields.
	class SelectiveInputReportParser {
	public:
		// Returns zero (ERR_SUCCESS) on success.
		// Returns ERR_COULD_NOT_MAP_ANY_USAGES if none of the descriptor fields
		// can be mapped to the int32 and bool variables of your program.
		int Init(Collection* input_fields, const void* descriptor, size_t descriptor_size);

		// Reset removes any mapping configuration created by Init.
		void Reset() {
			_mapping.clear();
		}

		// If Parse returns zero (ERR_SUCCESS) you have to process the mapped
		// int32 and bool variables that receive the input state changes.
		// In case of a nonzero return value you mustn't process the values of
		// of your variables, they may contain the state received in a previous
		// report. Processing relative variables (like X/Y deltas of a mouse)
		// more than once is a programming error.
		//
		// The ERR_NOTHING_CHANGED return value is specific to this Parse method.
		// It indicates that no error happened but the report contained no mapped fields.
		// However, handling the ERR_NOTHING_CHANGED error code specially is unnecessary.
		// You may be better off handling all errors the same way (as if you received
		// no report in the first place) because you may receive invalid reports
		// every now and then even from an otherwise healthy device with a healthy
		// USB connection. As an example:
		// One of my favourite gaming mice is the Zowie FK2 that has a very simple
		// HID report descriptor and sends 6-byte reports (1 byte: buttons,
		// 2x2 bytes: X and Y delta, 1 byte: wheel delta). When it is plugged in
		// the first report it sends is always an invalid report that consists
		// of four bytes (all of them are zeros). Occasionally it sends zero
		// sized reports too. The mouse still works perfectly with all major
		// desktop operating systems because they seem to forgive these errors.
		int Parse(const void* report, size_t report_size);

	private:
		struct ReportFieldMapping;
		struct UsageIndexRange;
		struct DescFieldMappings;
		class DescriptorMapper;

		// Mapping between input report fields and the program's int32/bool variables.
		struct ReportMapper {
			// report size in bits not including the report_id byte if present
			uint32_t bit_size = 0;
			std::vector<ReportFieldMapping> fields;
		};

		// key: report_id
		// the zero report_id belongs to structs that don't have a report_id
		typedef std::map<uint8_t, ReportMapper> mapping_t;
		mapping_t _mapping;
		bool _have_report_ids = false;
	};

	struct SelectiveInputReportParser::UsageIndexRange {
		size_t desc_min; // minimum usage index for the descriptor field
		size_t val_min;  // minimum usage index for IInt32Target or IBoolTarget
		size_t length;   // number of indexes both for the descriptor field and the values
	};

	struct SelectiveInputReportParser::DescFieldMappings {
		std::map<int32_t*, std::vector<UsageIndexRange>> int32_values;
		std::map<uint8_t*, std::vector<UsageIndexRange>> bool_values;

		bool AddMapping(int32_t* v, size_t desc_usage_index, size_t values_usage_index) {
			assert(v);
			return AppendUsageIndex(int32_values[v], desc_usage_index, values_usage_index);
		}

		bool AddMapping(uint8_t* v, size_t desc_usage_index, size_t values_usage_index) {
			assert(v);
			return AppendUsageIndex(bool_values[v], desc_usage_index, values_usage_index);
		}

	private:
		bool AppendUsageIndex(std::vector<UsageIndexRange>& ranges, size_t desc_usage_index, size_t values_usage_index) {
			// The logic that tries map descriptor fields onto the application's
			// variables (the FindFieldUsagesInCollection method) works by
			// iterating through the usages found in the descriptor and trying
			// to match them against the usages defined in the application's
			// mapping config. Usages are processed one-by-one.
			//
			// The code below makes sure that consecutive usages are handled as
			// one block when their source and destination happen to be the same
			// descriptor field and application variable. This improves the
			// performance of the report parser in case of large bit fields.
			// A typical example is 100+ keyboard keys declared as a bitfield
			// with a USAGE MIN/MAX both in the descriptor and the application's
			// mapping config. Matching the usages one-by-one helps in finding
			// large consecutive usage blocks even when the USAGE MIN/MAX ranges
			// aren't identical in the descriptor and mapping config - it's
			// enough to have an overlap.
			if (!ranges.empty()) {
				UsageIndexRange& r = ranges.back();
				if (r.desc_min + r.length == desc_usage_index &&
					r.val_min + r.length == values_usage_index) {
					r.length++;
					return true;
				}
			}
			ranges.push_back({ desc_usage_index, values_usage_index, 1 });
			return true;
		}
	};

	// Maps one report field to one or more Int32Value and BoolValue instances.
	struct SelectiveInputReportParser::ReportFieldMapping {
		DescFieldMappings mappings;
		// offset within the report not including the report_id byte if present
		uint32_t bit_offset;
		uint32_t report_size;
		uint32_t report_count;
		int32_t logical_min;
		int32_t logical_max;
		bool variable : 1;
		bool relative : 1;
		bool signed_ : 1;  // true if logical_min is below zero
		bool first_usage_is_zero : 1; // used only in case of array fields
		bool byte_aligned : 1;  // true if both bit_offset and report_size are a multiple of 8

		static int ParseVarFields(const ReportFieldMapping& m, const uint8_t* report);
		static int ParseArrayFields(const ReportFieldMapping& m, const uint8_t* report);
		static void ProcessArrayItem(const ReportFieldMapping& m, uint32_t index);

		static void ResetFields(const ReportFieldMapping& m);
	};


	class SelectiveInputReportParser::DescriptorMapper : private DescriptorParser::EventHandler {
	public:
		DescriptorMapper(mapping_t* m, Collection* input_fields) : _mapping(m), _root(input_fields) {}
		int MapFields(const void* descriptor, size_t descriptor_size);

	private:
		void ResizeVectors(Collection* c);

		int Field(const DescriptorParser::FieldParams& fp) override;
		int Padding(ReportType rt, uint8_t report_id, uint32_t bit_size) override;
		int BeginCollection(uint8_t collection_type, uint16_t usage_page, uint16_t usage, uint32_t depth) override;
		int EndCollection(uint32_t depth) override;

		void RecursiveAdd(Collection* c, uint8_t collection_type, uint16_t usage_page, uint16_t usage);
		void RecursiveAddChildren(Collection* c, uint8_t collection_type, uint16_t usage_page, uint16_t usage);
		static bool CollectionMatch(Collection* c, uint8_t collection_type, uint16_t usage_page, uint16_t usage);
		static bool UsagesMatch(const std::vector<UsageRange>& usages, uint16_t usage_page, uint16_t usage);
		static bool UsageMatch(const UsageRange& ur, uint16_t usage_page, uint16_t usage);
		static size_t CountUsages(const UsageRange* usage_ranges, size_t num_usage_ranges);

		struct FieldIndex {
			// index into Collection::int32s or Collection::bools
			size_t field_index;
			// index into an IInt32Target or IBoolTarget that is referenced
			// by an Int32Fields or BoolFields instance identified by field_index
			size_t usage_index;
		};

		typedef std::multimap<uint32_t, FieldIndex> usage_to_field_index;
		struct FieldIndexes {
			usage_to_field_index int32_indexes;
			usage_to_field_index bool_indexes;
		};

		static void AddFieldIndexes(usage_to_field_index& m, size_t field_index, const std::vector<UsageRange>& usages);
		FieldIndexes& GetFieldIndexes(Collection* c);
		static bool TestInt32FieldFlags(Collection* c, const FieldIndex& fi, uint16_t flags);
		static bool TestBoolFieldFlags(Collection* c, const FieldIndex& fi, uint16_t flags);

		// Returns the number of found/mapped usages or a negative error code.
		int32_t FindFieldUsagesInCollection(Collection* c, const DescriptorParser::FieldParams& fp,
			DescFieldMappings& dfm, std::vector<bool>* matched_usage_indexes = nullptr);

	private:
		mapping_t* _mapping;

		Collection* _root;
		std::set<Collection*> _matched_set;
		std::vector<Collection*> _matched;
		std::vector<size_t> _prev_matched_size;

		std::map<Collection*, FieldIndexes> _collection_field_indexes;
	};


	static constexpr uint8_t SCAN_INPUT   = uint8_t(1 << uint8_t(ReportType::input));
	static constexpr uint8_t SCAN_OUTPUT  = uint8_t(1 << uint8_t(ReportType::output));
	static constexpr uint8_t SCAN_FEATURE = uint8_t(1 << uint8_t(ReportType::feature));

	template <uint8_t SCAN_FLAGS=SCAN_INPUT, typename t_report_size=uint16_t, uint8_t MAX_REPORT_ID=255>
	class ReportSizeScanner : public DescriptorParser::EventHandler {
		static constexpr uint16_t INPUT_IDX = 0;
		static constexpr uint16_t OUTPUT_IDX = INPUT_IDX + ((SCAN_FLAGS & SCAN_INPUT) ? 1 : 0);
		static constexpr uint16_t FEATURE_IDX = OUTPUT_IDX + ((SCAN_FLAGS & SCAN_OUTPUT) ? 1 : 0);
		static constexpr uint16_t NUM_REPORT_TYPES = FEATURE_IDX + ((SCAN_FLAGS & SCAN_FEATURE) ? 1 : 0);
		static_assert(NUM_REPORT_TYPES > 0, "You have to specify at least one of the following flags: SCAN_INPUT, SCAN_OUTPUT, SCAN_FEATURE");

		// _arr maps a report struct size to each (report_type, report_id) pair.
		t_report_size _arr[NUM_REPORT_TYPES][MAX_REPORT_ID + 1];
		uint8_t _max_report_id[NUM_REPORT_TYPES];
	public:
		ReportSizeScanner() {
			Reset();
		}

		void Reset() {
			memset(_arr, 0, sizeof(_arr));
			memset(_max_report_id, 0, sizeof(_max_report_id));
		}

		// A naive solution can call ReportSize() on all possible report_ids
		// [0..255] and ignore those that return a report size of zero.
		//
		// Most HID report descriptors declare zero or only a few report_ids
		// with low values. For this reason scanning the report_ids only in the
		// range [0..GetReportID()] is usually faster. The used report_ids don't
		// necessarily form a continuous range so you have to check all report
		// sizes in the range [0..GetReportID()] even after encountering a zero.
		//
		// The zero report_id is invalid. We use it to record the report size
		// for fields that have been declared without a report ID. A HID report
		// descriptor either declares all of its fields without a report_id or
		// reports all of them with non-zero report_ids. For this reason a
		// descriptor with ReportSize(X, 0)>0 and MaxReportID(X)>0 would be
		// invalid but such cases fail early: the descriptor parser returns an
		// error so you don't have to deal with the scenario here.
		//
		// If you use a MAX_REPORT_ID template parameter value below 255 to
		// save memory then MaxReportID() may return a larger value indicating
		// that we don't have the report struct sizes for all report_ids.
		uint8_t MaxReportID(ReportType rt) const {
			auto idx = Index(rt);
			return idx < 0 ? 0 : _max_report_id[idx];
		}

		// Returns the report struct size for the specified (report_type, report_id) pair.
		// Returns zero if:
		// - The HID report descriptor doesn't define any fields for the given
		//   (report_type, report_id) pair
		// - The report_id is greater than the MAX_REPORT_ID template parameter.
		t_report_size ReportSize(ReportType rt, uint8_t report_id) const {
			auto idx = Index(rt, report_id);
			return idx < 0 ? 0 : _arr[idx][report_id];
		}

	protected:
		static int8_t Index(ReportType rt, uint8_t report_id) {
			int8_t idx = Index(rt);
			return report_id <= MAX_REPORT_ID ? idx : -1;
		}

		static int8_t Index(ReportType rt) {
			switch (rt) {
			case ReportType::input: return (SCAN_FLAGS & SCAN_INPUT) ? INPUT_IDX : -1;
			case ReportType::output: return (SCAN_FLAGS & SCAN_OUTPUT) ? OUTPUT_IDX : -1;
			case ReportType::feature: return (SCAN_FLAGS & SCAN_FEATURE) ? FEATURE_IDX : -1;
			default: return -1;
			}
		}

		int CommonHandler(ReportType rt, uint8_t report_id, uint32_t bit_size) {
			auto idx = Index(rt);
			if (idx < 0)
				return 0;
			_max_report_id[idx] = _hrp_max(_max_report_id[idx], report_id);
			if (report_id <= MAX_REPORT_ID)
				_arr[idx][report_id] += bit_size;
			return 0;
		}

		int Field(const DescriptorParser::FieldParams& fp) override {
			return CommonHandler(fp.report_type, fp.globals->report_id, fp.bit_size);
		}

		int Padding(ReportType rt, uint8_t report_id, uint32_t bit_size) override {
			return CommonHandler(rt, report_id, bit_size);
		}
	};


	// UsageExtractor extracts the usages associated with collections and fields
	// in a report descriptor. The usages associated with the top level application
	// containers usually reveal the types of devices while the usages associated
	// with fields shed light on the types of controls (like buttons, sticks, ...).
	class UsageExtractor : private DescriptorParser::EventHandler {
	public:
		struct Collection {
			// Collection type.
			// One of the COLLECTION_TYPE_* constants or a vendor defined value.
			uint8_t type;

			// The page+usage pair associated with the collection.
			// These can be zero (which means undefined) but that would be
			// highly unusual in case of a top level application collection.
			// Quote from the HID specification:
			//    a Usage item tag must be associated with
			//    any collection (such as a mouse or throttle)
			//
			// Despite the strict rules of the HID specification I've seen some
			// nested logical and report type collections declared without
			// usages in the descriptors of some gamepads and they were accepted
			// by the HID drivers.
			uint16_t usage_page;
			uint16_t usage;

			// UsageRange::usage_max is never below UsgeRange::usage_min.
			std::vector<UsageRange> field_usages[size_t(ReportType::count)];

			// Nested collections.
			// this is guaranteed to be empty when ScanDescriptor() is called
			// with collapse_collections=true. In that situation the field
			// usages of the nested collections go into the field_usages field
			// of this top level collection.
			std::vector<Collection> collections;
		};

		struct Report {
			// Top level collections.
			//
			// Quote from the HID specification:
			//   Each top level collection must be an application collection
			//   and reports may not span more than one top level collection.
			//
			// Virtually all real-world devices follow this part of the HID
			// specification so a top level collection is usually of type
			// COLLECTION_TYPE_APPLICATION and the page+usage value pair attached
			// to at least one of the top level application collections in a
			// descriptor is usually one of the following combinations in case
			// of commonly used input devices:
			// - PAGE_GENERIC_DESKTOP / USAGE_MOUSE
			// - PAGE_GENERIC_DESKTOP / USAGE_KEYBOARD
			// - PAGE_GENERIC_DESKTOP / USAGE_KEYPAD
			// - PAGE_GENERIC_DESKTOP / USAGE_JOYSTICK
			// - PAGE_GENERIC_DESKTOP / USAGE_GAMEPAD
			// - PAGE_CONSUMER / USAGE_CONSUMER_CONTROL
			std::vector<Collection> collections;

			// The usages of fields defined at the top level of the descriptor
			// outside of collections. I have never seen a real-world device
			// defining its fields outside of top level collections but in
			// practice it is possible to do so and a permissive HID driver
			// (like the Linux one) may accept them in some cases.
			//
			// You don't lose much by ignoring these fields. A descriptor that
			// defines fields at this level is a pathological case that doesn't
			// follow the HID standard. Strict HID drivers (like the one in
			// Windows 10) reject them.
			//
			// UsageRange::usage_min is always less than UsageRange::usage_max.
			std::vector<UsageRange> field_usages[size_t(ReportType::count)];
		};

		// If collapse_collections==true then nested collections and their types/usages
		// aren't recorded into the Collection::collections field of the root
		// collections but the usages of the fields of the nested collections
		// are still recorded into the Collection::field_usages of the root collection.
		// Returns 0 on error.
		int ScanDescriptor(const void* desc, size_t desc_size, Report& report,
				uint8_t report_types=SCAN_INPUT, bool collapse_collections=true) {

			_report = &report;
			_report_types = report_types;
			_collapse_collections = collapse_collections;
			_collection_stack.clear();

			DescriptorParser p;
			return p.Parse(desc, desc_size, this);
		}

	private:
		int Field(const DescriptorParser::FieldParams& fp) override {
			if (0 == (_report_types & uint8_t(1 << uint8_t(fp.report_type))))
				return 0;

			std::vector<UsageRange>& field_usages = _collection_stack.empty()
				? _report->field_usages[size_t(fp.report_type)]
				: _collection_stack.back()->field_usages[size_t(fp.report_type)];

			field_usages.insert(field_usages.end(), fp.usage_ranges, fp.usage_ranges+fp.num_usage_ranges);
			return 0;
		}

		int BeginCollection(uint8_t collection_type, uint16_t usage_page, uint16_t usage, uint32_t depth) override {
			if (_collapse_collections && depth > 1)
				return 0;

			std::vector<Collection>& a = (depth == 1) ? _report->collections : _collection_stack.back()->collections;
			a.push_back({ collection_type, usage_page, usage });
			_collection_stack.push_back(&a.back());
			return 0;
		}

		int EndCollection(uint32_t depth) {
			if (_collapse_collections && depth > 1)
				return 0;
			_collection_stack.pop_back();
			return 0;
		}

		Report* _report;
		uint8_t _report_types;
		bool _collapse_collections;
		std::vector<Collection*> _collection_stack;
	};


	static constexpr uint8_t FLAG_KEYBOARD = 0x01;
	static constexpr uint8_t FLAG_KEYPAD   = 0x02;
	static constexpr uint8_t FLAG_CONSUMER = 0x04; // multimedia keys
	static constexpr uint8_t FLAG_MOUSE    = 0x08;
	static constexpr uint8_t FLAG_GAMEPAD  = 0x10;
	static constexpr uint8_t FLAG_JOYSTICK = 0x20;

	// Returns zero or a combination of FLAG_KEYBOARD,etc... flags
	uint8_t detect_common_input_device_type(const void* desc, size_t desc_size);


	class CommonInputDeviceTypeDetector : private DescriptorParser::EventHandler {
	public:
		int Detect(const void* desc, size_t desc_size, uint8_t& detected_device_types) {
			detected_device_types = 0;
			_detected_device_types = &detected_device_types;

			DescriptorParser p;
			return p.Parse(desc, desc_size, this);
		}

	private:
		static constexpr uint32_t usage32(uint16_t usage_page, uint16_t usage) {
			return ((uint32_t)usage_page << 16) | (uint32_t)usage;
		}

		int BeginCollection(uint8_t collection_type, uint16_t usage_page, uint16_t usage, uint32_t depth) override {
			if (depth != 1 || collection_type != COLLECTION_TYPE_APPLICATION)
				return 0;
			switch (usage32(usage_page, usage)) {
			case usage32(PAGE_GENERIC_DESKTOP, USAGE_KEYBOARD):  *_detected_device_types |= FLAG_KEYBOARD; break;
			case usage32(PAGE_GENERIC_DESKTOP, USAGE_KEYPAD):    *_detected_device_types |= FLAG_KEYPAD;   break;
			case usage32(PAGE_GENERIC_DESKTOP, USAGE_MOUSE):     *_detected_device_types |= FLAG_MOUSE;    break;
			case usage32(PAGE_GENERIC_DESKTOP, USAGE_GAMEPAD):   *_detected_device_types |= FLAG_GAMEPAD;  break;
			case usage32(PAGE_GENERIC_DESKTOP, USAGE_JOYSTICK):  *_detected_device_types |= FLAG_JOYSTICK; break;
			case usage32(PAGE_CONSUMER, USAGE_CONSUMER_CONTROL): *_detected_device_types |= FLAG_CONSUMER; break;
			}
			return 0;
		}

		uint8_t* _detected_device_types;
	};


	struct MouseConfig {
		// Indexes into the IBoolTarget that receives the button states.
		static constexpr uint8_t BTN_LEFT = 0;
		static constexpr uint8_t BTN_RIGHT = 1;
		static constexpr uint8_t BTN_MIDDLE = 2;
		static constexpr uint8_t BTN_BACK = 3;    // side button
		static constexpr uint8_t BTN_FORWARD = 4; // side button

		static constexpr uint8_t NUM_BUTTONS = 5;

		BoolFields buttons {
			.usages { { PAGE_BUTTON, 1, 5 } },
		};

		// Indexes into the IInt32Target that receives the axis deltas.
		static constexpr uint8_t X = 0;
		static constexpr uint8_t Y = 1;
		static constexpr uint8_t V_SCROLL = 2;
		static constexpr uint8_t H_SCROLL = 3;

		static constexpr uint8_t NUM_AXES = 4;

		Int32Fields axes {
			.usages {
				{ PAGE_GENERIC_DESKTOP, USAGE_X, USAGE_Y },
				{ PAGE_GENERIC_DESKTOP, USAGE_WHEEL },    // vertical scroll
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_PAN }  // horizontal scroll
			},
			.mask = FLAG_FIELD_CONST | FLAG_FIELD_VARIABLE | FLAG_FIELD_RELATIVE,
			.flags = FLAG_FIELD_VARIABLE | FLAG_FIELD_RELATIVE,
		};
		Collection application {
			.type = COLLECTION_TYPE_APPLICATION,
			.usages { { PAGE_GENERIC_DESKTOP, USAGE_MOUSE } },
			.bools { &buttons },
		};
		Collection root {
			.collections { &application },
			.int32s { &axes },
		};

		Collection* Init(IBoolTarget* buttons_, IInt32Target* axes_, bool permissive=false) {
			buttons.target = buttons_;
			axes.target = axes_;

			if (!permissive) {
				root.type = COLLECTION_TYPE_APPLICATION;
				root.usages = { {PAGE_GENERIC_DESKTOP, USAGE_MOUSE} };
				return &root;
			}

			// permissive==false:
			//
			// This way the axes can be declared anywhere in the HID report
			// descriptor and still get accepted even outside of the
			// application container of the mouse. Permissive HID drivers
			// (like the one in Linux) work this way. Stricter HID drivers
			// (like the one in Win10) that follow the HID specification
			// more closely would not accept axis deltas declared that way.
			//
			// Fortunately this part of the HID specification (declaring the
			// input controls inside an application container that is tagged
			// with the device type) is well respected by real-world input
			// devices so being too permissive isn't necessary.
			//
			// If you don't have experience with parsing HID report descriptors
			// you may think about being permissive as something that has only
			// upsides but that isn't the case. If you aren't permissive and
			// accept mouse controls only when they are declared inside an
			// application container tagged with Usage(Mouse) then the
			// SelectiveInputReportParser::Init method returns
			// ERR_COULD_NOT_MAP_ANY_USAGES in case of non-mouse devices instead
			// of mapping only a few random controls like the buttons of a
			// joystick or the scroll wheel of a random non-mouse input device.
			// If a non-permissive mapping succeeds (because the device declared
			// its controls inside an application container tagged with Usage(Mouse))
			// then you can be confident that it provides at least the most
			// basic/essential mouse controls that include relative X/Y deltas
			// and 2 buttons.
			//
			// If your goal is being very permissive then you should either try
			// to map a descriptor with several non-permissive configurations
			// (tailored to standard/well-known input devices) or create a
			// single permissive mapping config declaring all input controls
			// that can be utilised by your application.
			root.type = 0;
			root.usages.clear();
			return &root;
		}
	};


	// Commonly used multimedia keys.
	struct MediaKeys {
		// media player
		static constexpr uint8_t KEY_PLAY = 0;
		static constexpr uint8_t KEY_PAUSE = 1;
		static constexpr uint8_t KEY_FAST_FORWARD = 2;
		static constexpr uint8_t KEY_REWIND = 3;
		static constexpr uint8_t KEY_SCAN_NEXT_TRACK = 4;
		static constexpr uint8_t KEY_SCAN_PREVIOUS_TRACK = 5;
		static constexpr uint8_t KEY_STOP = 6;
		static constexpr uint8_t KEY_EJECT = 7;
		static constexpr uint8_t KEY_PLAY_PAUSE = 8;
		static constexpr uint8_t KEY_MUTE = 9;
		static constexpr uint8_t KEY_VOLUME_INCREMENT = 10;
		static constexpr uint8_t KEY_VOLUME_DECREMENT = 11;
		// web browser
		static constexpr uint8_t KEY_AC_HOME = 12;
		static constexpr uint8_t KEY_AC_BACK = 13;
		static constexpr uint8_t KEY_AC_FORWARD = 14;
		static constexpr uint8_t KEY_AC_STOP = 15;
		static constexpr uint8_t KEY_AC_REFRESH = 16;
		static constexpr uint8_t KEY_AC_PREVIOUS_LINK = 17;
		static constexpr uint8_t KEY_AC_NEXT_LINK = 18;
		static constexpr uint8_t KEY_AC_BOOKMARKS = 19;
		static constexpr uint8_t KEY_AC_HISTORY = 20;
		// application launch
		static constexpr uint8_t KEY_AL_EMAIL_READER = 21;
		static constexpr uint8_t KEY_AL_CALCULATOR = 22;
		static constexpr uint8_t KEY_AL_LOCAL_MACHINE_BROWSER = 23;
		static constexpr uint8_t KEY_AL_INTERNET_BROWSER = 24;
		static constexpr uint8_t KEY_AL_AUDIO_PLAYER = 25;

		static constexpr uint8_t NUM_MEDIA_KEYS = 26;

		// The IBoolTarget associated with the keys has to be indexed with the
		// KEY_* constants declared above.
		// Including all consumer usages would result in a huge bit field.
		// For this reason we include only those that appear often on a typical
		// gaming/home/office keyboard.
		BoolFields media_keys {
			.usages {
				// media player
				{ PAGE_CONSUMER, USAGE_CONSUMER_PLAY },
				{ PAGE_CONSUMER, USAGE_CONSUMER_PAUSE },
				{ PAGE_CONSUMER, USAGE_CONSUMER_FAST_FORWARD },
				{ PAGE_CONSUMER, USAGE_CONSUMER_REWIND },
				{ PAGE_CONSUMER, USAGE_CONSUMER_SCAN_NEXT_TRACK },
				{ PAGE_CONSUMER, USAGE_CONSUMER_SCAN_PREVIOUS_TRACK },
				{ PAGE_CONSUMER, USAGE_CONSUMER_STOP },
				{ PAGE_CONSUMER, USAGE_CONSUMER_EJECT },
				{ PAGE_CONSUMER, USAGE_CONSUMER_PLAY_PAUSE },
				{ PAGE_CONSUMER, USAGE_CONSUMER_MUTE },
				{ PAGE_CONSUMER, USAGE_CONSUMER_VOLUME_INCREMENT },
				{ PAGE_CONSUMER, USAGE_CONSUMER_VOLUME_DECREMENT },
				// web browser
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_HOME },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_BACK },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_FORWARD },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_STOP },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_REFRESH },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_PREVIOUS_LINK },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_NEXT_LINK },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_BOOKMARKS },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AC_HISTORY },
				// application launch
				{ PAGE_CONSUMER, USAGE_CONSUMER_AL_EMAIL_READER },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AL_CALCULATOR },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AL_LOCAL_MACHINE_BROWSER },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AL_INTERNET_BROWSER },
				{ PAGE_CONSUMER, USAGE_CONSUMER_AL_AUDIO_PLAYER },
			},
			// We don't filter for the array/variable and relative/absolute flags
			// because this way the mapper will automatically map both array and
			// var fields onto our bitfield. Restricting the value of the
			// relative/absolute flag is also a bad idea because some keys that
			// are marked as OSC (one-shot control) in the HID usage tables
			// documentation may have their relative flags set while most other
			// keys and buttons usually have their relative flags cleared.
			//
			// An example to an OSC key is the Play/Pause consumer control.
			// Real-world applications often overlook this part of the HID
			// documentation and add the OSC keys too without the relative flag
			// set. Example: The COOLERMASTER_MASTERKEYS_S_3 descriptor adds a
			// Play/Pause button with its relative flag cleared.
			// The RASPBERRY_PI_2 descriptor adds the Play/Pause button with its
			// relative flag set (as recommended by the HID specification).
		};
	};


	// Some keyboards expose their multimedia keys on a separate HID interface.
	struct MediaKeyConfig : public MediaKeys {
		Collection root {
			.bools { &media_keys },
		};

		Collection* Init(IBoolTarget* media_keys_, bool permissive=false) {
			media_keys.target = media_keys_;

			if (!permissive) {
				root.type = COLLECTION_TYPE_APPLICATION;
				root.usages = {
					{ PAGE_GENERIC_DESKTOP, USAGE_KEYBOARD, USAGE_KEYPAD },
					{ PAGE_CONSUMER, USAGE_CONSUMER_CONTROL },
				};
				return &root;
			}

			root.type = 0;
			root.usages.clear();
			return &root;
		}
	};


	struct KeyboardConfig {
		static constexpr uint16_t NUM_KEYS = 0x100;

		// The IBoolTarget associated with the keys has to be indexed with the
		// USAGE_KEYBOARD_* and USAGE_KEYPAD_* constants of the keyboard usage page.
		BoolFields keys {
			.usages { { PAGE_KEYBOARD, 0, 0xff } },
		};

		Collection root {
			.bools { &keys },
		};

		Collection* Init(IBoolTarget* keys_, bool permissive=false) {
			keys.target = keys_;

			if (!permissive) {
				root.type = COLLECTION_TYPE_APPLICATION;
				root.usages = {
					{ PAGE_GENERIC_DESKTOP, USAGE_KEYBOARD, USAGE_KEYPAD },
				};
				return &root;
			}

			root.type = 0;
			root.usages.clear();
			return &root;
		}
	};


	struct MultimediaKeyboardConfig : public MediaKeys {
		static constexpr uint16_t NUM_KEYS = 0x100;

		// The IBoolTarget associated with the keys has to be indexed with the
		// USAGE_KEYBOARD_* and USAGE_KEYPAD_* constants of the keyboard usage page.
		BoolFields keys {
			.usages { { PAGE_KEYBOARD, 0, 0xff } },
		};

		Collection key_collection {
			.bools { &keys },
		};

		Collection root {
			.collections { &key_collection },
			.bools { &media_keys },
		};

		Collection* Init(IBoolTarget* keys_, IBoolTarget* media_keys_, bool permissive=false) {
			keys.target = keys_;
			media_keys.target = media_keys_;

			if (!permissive) {
				key_collection.type = COLLECTION_TYPE_APPLICATION;
				key_collection.usages = {
					{ PAGE_GENERIC_DESKTOP, USAGE_KEYBOARD, USAGE_KEYPAD },
				};
				// The root contains the media keys because those have a broader
				// usage filter which includes USAGE_CONSUMER_CONTROL.
				// An alternative solution would use 3 collections: one root
				// (without any type/usage filters) with 2 child collections for
				// regular and media keys but using 3 collections would not
				// yield better results in this case.
				root.type = COLLECTION_TYPE_APPLICATION;
				root.usages = {
					{ PAGE_GENERIC_DESKTOP, USAGE_KEYBOARD, USAGE_KEYPAD },
					{ PAGE_CONSUMER, USAGE_CONSUMER_CONTROL },
				};
				return &root;
			}

			key_collection.type = 0;
			key_collection.usages.clear();
			root.type = 0;
			root.usages.clear();
			return &root;
		}
	};


	struct GamepadConfig {
		static constexpr uint8_t NUM_BUTTONS = 32;

		// Up to 32 buttons.
		// Valid indexes into the IBoolTarget that receives the button states: 0-31.
		BoolFields buttons {
			.usages { { PAGE_BUTTON, 1, 32 } },
		};

		// Indexes into the IInt32Target that receives the absolute axis values:
		static constexpr uint8_t X = 0;
		static constexpr uint8_t Y = 1;
		static constexpr uint8_t Z = 2;
		static constexpr uint8_t RX = 3;
		static constexpr uint8_t RY = 4;
		static constexpr uint8_t RZ = 5;
		static constexpr uint8_t SLIDER = 6;
		static constexpr uint8_t DIAL = 7;
		static constexpr uint8_t WHEEL = 8;
		static constexpr uint8_t HAT_SWITCH = 9;

		static constexpr uint8_t NUM_AXES = 10;

		// The flags are restricted to absolute values.
		// Relative values (like mouse x/y deltas) won't be mapped.
		Int32Fields axes {
			.usages {
				{ PAGE_GENERIC_DESKTOP, USAGE_X, USAGE_HAT_SWITCH },
			},
			.mask = FLAG_FIELD_CONST | FLAG_FIELD_VARIABLE | FLAG_FIELD_RELATIVE,
			.flags = FLAG_FIELD_VARIABLE,
		};

		// We accept a wide range of axis controls with absolute values.
		//
		// Game controllers can have a lot of different usages associated with
		// their top level application collections but the two most frequent
		// values in case of low-cost game controllers are:
		// - PAGE_GENERIC_DESKTOP / USAGE_GAMEPAD
		// - PAGE_GENERIC_DESKTOP / USAGE_JOYSTICK
		//
		// If you decide to filter for specific container usages then take a
		// look at the possible vales on the following usage pages as well:
		// PAGE_SIMULATION_CONTROLS, PAGE_VR_CONTROLS, PAGE_SPORT_CONTROLS,
		// PAGE_GAME_CONTROLS
		Collection root {
			.int32s { &axes },
			.bools { &buttons },
		};

		Collection* Init(IBoolTarget* buttons_, IInt32Target* axes_, bool permissive=false) {
			buttons.target = buttons_;
			axes.target = axes_;

			if (!permissive) {
				root.type = COLLECTION_TYPE_APPLICATION;
				root.usages = {
					{ PAGE_GENERIC_DESKTOP, USAGE_JOYSTICK, USAGE_GAMEPAD},
				};
				return &root;
			}

			root.type = 0;
			root.usages.clear();
			return &root;
		}
	};


	// Gamepad with support for more buttons and axes.
	struct BigGamepadConfig {
		static constexpr uint8_t NUM_BUTTONS = 64;

		// Up to 64 buttons.
		// Valid indexes into the IBoolTarget that receives the button states: 0-63.
		BoolFields buttons {
			.usages { { PAGE_BUTTON, 1, 64 } },
		};

		// Indexes into the IInt32Target that receives the absolute axis values:
		static constexpr uint8_t X = 0;
		static constexpr uint8_t Y = 1;
		static constexpr uint8_t Z = 2;
		static constexpr uint8_t RX = 3;
		static constexpr uint8_t RY = 4;
		static constexpr uint8_t RZ = 5;
		static constexpr uint8_t SLIDER = 6;
		static constexpr uint8_t DIAL = 7;
		static constexpr uint8_t WHEEL = 8;
		static constexpr uint8_t HAT_SWITCH = 9;

		static constexpr uint8_t X_2 = 10;
		static constexpr uint8_t Y_2 = 11;
		static constexpr uint8_t Z_2 = 12;
		static constexpr uint8_t RX_2 = 13;
		static constexpr uint8_t RY_2 = 14;
		static constexpr uint8_t RZ_2 = 15;
		static constexpr uint8_t SLIDER_2 = 16;
		static constexpr uint8_t DIAL_2 = 17;
		static constexpr uint8_t WHEEL_2 = 18;
		static constexpr uint8_t HAT_SWITCH_2 = 19;

		static constexpr uint8_t TURN_RIGHT_LEFT = 20;
		static constexpr uint8_t PITCH_FORWARD_BACKWARD = 21;
		static constexpr uint8_t ROLL_RIGHT_LEFT = 22;
		static constexpr uint8_t MOVE_RIGHT_LEFT = 23;
		static constexpr uint8_t MOVE_FORWARD_BACKWARD = 24;
		static constexpr uint8_t MOVE_UP_DOWN = 25;
		static constexpr uint8_t LEAN_RIGHT_LEFT = 26;
		static constexpr uint8_t LEAN_FORWARD_BACKWARD = 27;
		static constexpr uint8_t HEIGHT_OF_POV = 28;

		static constexpr uint8_t NUM_AXES = 29;

		// The flags are restricted to absolute values.
		// Relative values (like mouse x/y deltas) won't be mapped.
		Int32Fields axes {
			.usages {
				{ PAGE_GENERIC_DESKTOP, USAGE_X, USAGE_HAT_SWITCH },
				{ PAGE_GENERIC_DESKTOP, USAGE_X, USAGE_HAT_SWITCH },
				{ PAGE_GAME_CONTROLS, USAGE_GAME_CONTROLS_TURN_RIGHT_LEFT, USAGE_GAME_CONTROLS_HEIGHT_OF_POV },
			},
			.mask = FLAG_FIELD_CONST | FLAG_FIELD_VARIABLE | FLAG_FIELD_RELATIVE,
			.flags = FLAG_FIELD_VARIABLE,
		};

		// We accept a wide range of axis controls with absolute values.
		//
		// Game controllers can have a lot of different usages associated with
		// their top level application collections but the two most frequent
		// values in case of low-cost game controllers are:
		// - PAGE_GENERIC_DESKTOP / USAGE_GAMEPAD
		// - PAGE_GENERIC_DESKTOP / USAGE_JOYSTICK
		//
		// If you decide to filter for specific container usages then take a
		// look at the possible vales on the following usage pages as well:
		// PAGE_SIMULATION_CONTROLS, PAGE_VR_CONTROLS, PAGE_SPORT_CONTROLS,
		// PAGE_GAME_CONTROLS
		Collection root {
			.int32s { &axes },
			.bools { &buttons },
		};

		Collection* Init(IBoolTarget* buttons_, IInt32Target* axes_, bool permissive=false) {
			buttons.target = buttons_;
			axes.target = axes_;

			if (!permissive) {
				root.type = COLLECTION_TYPE_APPLICATION;
				root.usages = {
					{ PAGE_GENERIC_DESKTOP, USAGE_JOYSTICK, USAGE_GAMEPAD},
				};
				return &root;
			}

			root.type = 0;
			root.usages.clear();
			return &root;
		}
	};


} // namespace hidrp


