// clang-format off
// basisu_bc6h.cpp - Self-contained BC6H (unsigned) block encoder extracted from
// Binomial LLC's basis_universal (https://github.com/BinomialLLC/basis_universal).
// The code is the "bc6hf" real-time analytical encoder from
// transcoder/basisu_transcoder.cpp (basist::astc_6x6_hdr::fast_encode_bc6h) along
// with the minimal types/helpers it needs from transcoder/basisu.h,
// transcoder/basisu_containers.h and transcoder/basisu_transcoder_internal.h, so
// it can be built without the rest of the transcoder.
// See LICENSE for the Apache 2.0 license.

#include "basisu_bc6h.h"

#include <algorithm>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <stdint.h>
#include <string.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#if defined(_MSC_VER) && !defined(__clang__)
#define BASISU_FORCE_INLINE __forceinline
#else
#define BASISU_FORCE_INLINE inline
#endif

#define BASISU_NOTE_UNUSED(x) (void)(x)

// Minimal subset of the basisu namespace helpers (from transcoder/basisu.h and
// transcoder/basisu_containers.h) needed by the BC6H encoder.
namespace basisu
{
	constexpr float REALLY_SMALL_FLOAT_VAL = .000000125f;
	constexpr float SMALL_FLOAT_VAL = .0000125f;
	constexpr float BIG_FLOAT_VAL = 1e+30f;

	template <typename S> inline S clamp(S value, S low, S high) { return (value < low) ? low : ((value > high) ? high : value); }

	template <typename S> inline S maximum(S a, S b) { return (a > b) ? a : b; }
	template <typename S> inline S maximum(S a, S b, S c) { return maximum(maximum(a, b), c); }
	template <typename S> inline S maximum(S a, S b, S c, S d) { return maximum(maximum(maximum(a, b), c), d); }

	template <typename S> inline S minimum(S a, S b) { return (a < b) ? a : b; }
	template <typename S> inline S minimum(S a, S b, S c) { return minimum(minimum(a, b), c); }
	template <typename S> inline S minimum(S a, S b, S c, S d) { return minimum(minimum(minimum(a, b), c), d); }

	inline float minimumf(float a, float b) { return (a < b) ? a : b; }
	inline float squaref(float i) { return i * i; }
	inline uint32_t iabs(int32_t i) { return (i < 0) ? static_cast<uint32_t>(-i) : static_cast<uint32_t>(i); }

	template <typename T0, typename T1> inline T0 lerp(T0 a, T0 b, T1 c) { return a + (b - a) * c; }

	template <typename T> inline void clear_obj(T& obj) { memset((void *)&obj, 0, sizeof(obj)); }

	static inline void write_le_dword(uint8_t* pBytes, uint32_t val)
	{
		pBytes[0] = (uint8_t)val;
		pBytes[1] = (uint8_t)(val >> 8U);
		pBytes[2] = (uint8_t)(val >> 16U);
		pBytes[3] = (uint8_t)(val >> 24U);
	}
}

// Minimal subset of the basist namespace types/helpers (from
// transcoder/basisu_transcoder_internal.h) needed by the BC6H encoder.
namespace basist
{
	const double MIN_DENORM_HALF_FLOAT = 0.000000059604645; // smallest positive subnormal number
	const double MIN_HALF_FLOAT = 0.00006103515625; // smallest positive normal number
	const double MAX_HALF_FLOAT = 65504.0; // largest normal number
	const uint32_t MAX_HALF_FLOAT_AS_INT_BITS = 0x7BFF; // the half float rep for 65504.0

	inline uint32_t get_bits(uint32_t val, int low, int high)
	{
		const int num_bits = (high - low) + 1;
		assert((num_bits >= 1) && (num_bits <= 32));

		val >>= low;
		if (num_bits != 32)
			val &= ((1u << num_bits) - 1);

		return val;
	}

	inline bool is_half_inf_or_nan(half_float v)
	{
		return get_bits(v, 10, 14) == 31;
	}

	inline bool half_is_signed(half_float v)
	{
		return (v & 0x8000) != 0;
	}

	const uint32_t NUM_BC6H_MODES = 14;
	const uint32_t BC6H_LAST_MODE_INDEX = 13;
	const uint32_t BC6H_FIRST_1SUBSET_MODE_INDEX = 10; // in the MS docs, this is "mode 11" (where the first mode is 1), 60 bits for endpoints (10.10, 10.10, 10.10), 63 bits for weights
	const uint32_t TOTAL_BC6H_PARTITION_PATTERNS = 32;

	struct bc6h_bit_layout
	{
		int8_t m_comp; // R=0,G=1,B=2,D=3 (D=partition index)
		int8_t m_index; // 0-3, 0-1 Low/High subset 1, 2-3 Low/High subset 2, -1=partition index (d)
		int8_t m_last_bit;
		int8_t m_first_bit; // may be -1 if a single bit, may be >m_last_bit if reversed
	};

	const uint32_t MAX_BC6H_LAYOUT_INDEX = 25;

	const uint32_t MAX_BC6H_HALF_FLOAT_AS_UINT = 0x7BFF;

	// Inverts bc6h_blog16_to_half().
	// Returns the nearest blog16 given a half value.
	inline uint32_t bc6h_half_to_blog16(half_float h)
	{
		assert(h <= MAX_BC6H_HALF_FLOAT_AS_UINT);
		return (h * 64 + 30) / 31;
	}

	// Suboptimal, but very close.
	inline uint32_t bc6h_half_to_blog(half_float h, uint32_t num_bits)
	{
		assert(h <= MAX_BC6H_HALF_FLOAT_AS_UINT);
		return (h * 64 + 30) / (31 * (1 << (16 - num_bits)));
	}

	struct bc6h_logical_block
	{
		uint32_t m_mode;
		uint32_t m_partition_pattern;	// must be 0 if 1 subset
		uint32_t m_endpoints[3][4];		// [comp][subset*2+lh_index] - must be already properly packed
		uint8_t m_weights[16];			// weights must be of the proper size, taking into account skipped MSB's which must be 0

		void clear()
		{
			basisu::clear_obj(*this);
		}
	};

} // namespace basist

// Everything below is extracted verbatim from transcoder/basisu_transcoder.cpp.
namespace basist
{

	struct vec3F
	{
		float c[3];

		inline vec3F() {}

		inline vec3F(float s) { c[0] = s; c[1] = s; c[2] = s; }
		inline vec3F(float x, float y, float z) { c[0] = x; c[1] = y; c[2] = z; }

		inline void set(float x, float y, float z) { c[0] = x; c[1] = y; c[2] = z; }

		inline float dot(const vec3F& o) const { return (c[0] * o.c[0]) + (c[1] * o.c[1]) + (c[2] * o.c[2]); }

		inline float operator[] (uint32_t index) const { assert(index < 3); return c[index]; }
		inline float &operator[] (uint32_t index) { assert(index < 3); return c[index]; }

		inline vec3F& clamp(float l, float h)
		{
			c[0] = basisu::clamp(c[0], l, h);
			c[1] = basisu::clamp(c[1], l, h);
			c[2] = basisu::clamp(c[2], l, h);
			return *this;
		}

		static vec3F lerp(const vec3F& a, const vec3F& b, float s)
		{
			vec3F res;
			for (uint32_t i = 0; i < 3; i++)
				res[i] = basisu::lerp(a[i], b[i], s);
			return res;
		}

		inline float norm() const { return dot(*this); }
	};

	struct vec4F
	{
		float c[4];

		inline void set(float x, float y, float z, float w) { c[0] = x; c[1] = y; c[2] = z; c[3] = w; }

		float operator[] (uint32_t index) const { assert(index < 4); return c[index]; }
		float& operator[] (uint32_t index) { assert(index < 4); return c[index]; }
	};

	const uint32_t g_bc7_weights3[8] = { 0, 9, 18, 27, 37, 46, 55, 64 };
	const uint8_t g_bc7_partition2[64 * 16] =
	{
		0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,		0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,		0,1,1,1,0,1,1,1,0,1,1,1,0,1,1,1,		0,0,0,1,0,0,1,1,0,0,1,1,0,1,1,1,		0,0,0,0,0,0,0,1,0,0,0,1,0,0,1,1,		0,0,1,1,0,1,1,1,0,1,1,1,1,1,1,1,		0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,		0,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1,
		0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,		0,0,1,1,0,1,1,1,1,1,1,1,1,1,1,1,		0,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,		0,0,0,0,0,0,0,0,0,0,0,1,0,1,1,1,		0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,		0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,		0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,		0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,
		0,0,0,0,1,0,0,0,1,1,1,0,1,1,1,1,		0,1,1,1,0,0,0,1,0,0,0,0,0,0,0,0,		0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,0,		0,1,1,1,0,0,1,1,0,0,0,1,0,0,0,0,		0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,		0,0,0,0,1,0,0,0,1,1,0,0,1,1,1,0,		0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,		0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,1,
		0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,0,		0,0,0,0,1,0,0,0,1,0,0,0,1,1,0,0,		0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,		0,0,1,1,0,1,1,0,0,1,1,0,1,1,0,0,		0,0,0,1,0,1,1,1,1,1,1,0,1,0,0,0,		0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,		0,1,1,1,0,0,0,1,1,0,0,0,1,1,1,0,		0,0,1,1,1,0,0,1,1,0,0,1,1,1,0,0,
		0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,		0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,		0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,		0,0,1,1,0,0,1,1,1,1,0,0,1,1,0,0,		0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,		0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,		0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,		0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,
		0,1,1,1,0,0,1,1,1,1,0,0,1,1,1,0,		0,0,0,1,0,0,1,1,1,1,0,0,1,0,0,0,		0,0,1,1,0,0,1,0,0,1,0,0,1,1,0,0,		0,0,1,1,1,0,1,1,1,1,0,1,1,1,0,0,		0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,		0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,		0,1,1,0,0,1,1,0,1,0,0,1,1,0,0,1,		0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,
		0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,0,		0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,0,		0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,		0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,		0,1,1,0,1,1,0,0,1,0,0,1,0,0,1,1,		0,0,1,1,0,1,1,0,1,1,0,0,1,0,0,1,		0,1,1,0,0,0,1,1,1,0,0,1,1,1,0,0,		0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0,
		0,1,1,0,1,1,0,0,1,1,0,0,1,0,0,1,		0,1,1,0,0,0,1,1,0,0,1,1,1,0,0,1,		0,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,		0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,1,		0,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,		0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,		0,0,1,0,0,0,1,0,1,1,1,0,1,1,1,0,		0,1,0,0,0,1,0,0,0,1,1,1,0,1,1,1
	};
	const uint8_t g_bc7_table_anchor_index_second_subset[64] = { 15,15,15,15,15,15,15,15,		15,15,15,15,15,15,15,15,		15, 2, 8, 2, 2, 8, 8,15,		2, 8, 2, 2, 8, 8, 2, 2,		15,15, 6, 8, 2, 8,15,15,		2, 8, 2, 2, 2,15,15, 6,		6, 2, 6, 8,15,15, 2, 2,		15,15,15,15,15, 2, 2,15 };

	// Originally from bc6h_enc.cpp
	// BC6H decoder fuzzed vs. DirectXTex's for unsigned/signed

	const uint8_t g_bc6h_mode_sig_bits[NUM_BC6H_MODES][4] = // base bits, r, g, b
	{
		// 2 subsets
		{ 10, 5, 5, 5, },	// 0, mode 1 in MS/D3D docs
		{ 7, 6, 6, 6, },	// 1
		{ 11, 5, 4, 4, },	// 2
		{ 11, 4, 5, 4, },	// 3
		{ 11, 4, 4, 5, },	// 4
		{ 9, 5, 5, 5, },	// 5
		{ 8, 6, 5, 5, },	// 6
		{ 8, 5, 6, 5, },	// 7
		{ 8, 5, 5, 6, },	// 8
		{ 6, 6, 6, 6, },	// 9, endpoints not delta encoded, mode 10 in MS/D3D docs
		// 1 subset
		{ 10, 10, 10, 10, }, // 10, endpoints not delta encoded, mode 11 in MS/D3D docs
		{ 11, 9, 9, 9, },	// 11
		{ 12, 8, 8, 8, },	// 12
		{ 16, 4, 4, 4, }	// 13, also useful for solid blocks
	};

	const int8_t g_bc6h_mode_lookup[32] = { 0, 1, 2, 10, 0, 1, 3, 11, 0, 1, 4, 12, 0, 1, 5, 13, 0, 1, 6, -1, 0, 1, 7, -1, 0, 1, 8, -1, 0, 1, 9, -1 };

	const bc6h_bit_layout g_bc6h_bit_layouts[NUM_BC6H_MODES][MAX_BC6H_LAYOUT_INDEX] =
	{
		// comp_index, subset*2+lh_index, last_bit, first_bit
		//------------------------        mode 0: 2 subsets, Weight bits: 46 bits, Endpoint bits: 75 bits (10.555, 10.555, 10.555), delta            
		{ { 1, 2, 4, -1 }, { 2, 2, 4, -1 }, { 2, 3, 4, -1 }, { 0, 0, 9, 0 }, { 1, 0, 9, 0 }, { 2, 0, 9, 0 }, { 0, 1, 4, 0 },
		{ 1, 3, 4, -1 }, { 1, 2, 3, 0 }, { 1, 1, 4, 0 }, { 2, 3, 0, -1 }, { 1, 3, 3, 0 }, { 2, 1, 4, 0 }, { 2, 3, 1, -1 },
		{ 2, 2, 3, 0 }, { 0, 2, 4, 0 }, { 2, 3, 2, -1 }, { 0, 3, 4, 0 }, { 2, 3, 3, -1 }, { 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 1: 2 subsets, Weight bits: 46 bits, Endpoint bits: 75 bits (7.666, 7.666, 7.666), delta
		{ { 1, 2, 5, -1 },{ 1, 3, 4, -1 },{ 1, 3, 5, -1 },{ 0, 0, 6, 0 },{ 2, 3, 0, -1 },{ 2, 3, 1, -1 },{ 2, 2, 4, -1 },
		{ 1, 0, 6, 0 },{ 2, 2, 5, -1 },{ 2, 3, 2, -1 },{ 1, 2, 4, -1 },{ 2, 0, 6, 0 },{ 2, 3, 3, -1 },{ 2, 3, 5, -1 },
		{ 2, 3, 4, -1 },{ 0, 1, 5, 0 },{ 1, 2, 3, 0 },{ 1, 1, 5, 0 },{ 1, 3, 3, 0 },{ 2, 1, 5, 0 },{ 2, 2, 3, 0 },{ 0, 2, 5, 0 },
		{ 0, 3, 5, 0 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 2: 2 subsets, Weight bits: 46 bits, Endpoint bits: 72 bits (11.555, 11.444, 11.444), delta
		{ { 0, 0, 9, 0 },{ 1, 0, 9, 0 },{ 2, 0, 9, 0 },{ 0, 1, 4, 0 },{ 0, 0, 10, -1 },{ 1, 2, 3, 0 },{ 1, 1, 3, 0 },{ 1, 0, 10, -1 },
		{ 2, 3, 0, -1 },{ 1, 3, 3, 0 },{ 2, 1, 3, 0 },{ 2, 0, 10, -1 },{ 2, 3, 1, -1 },{ 2, 2, 3, 0 },{ 0, 2, 4, 0 },{ 2, 3, 2, -1 },
		{ 0, 3, 4, 0 },{ 2, 3, 3, -1 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 3: 2 subsets, Weight bits: 46 bits, Endpoint bits: 72 bits (11.444, 11.555, 11.444), delta
		{ { 0, 0, 9, 0 },{ 1, 0, 9, 0 },{ 2, 0, 9, 0 },{ 0, 1, 3, 0 },{ 0, 0, 10, -1 },{ 1, 3, 4, -1 },{ 1, 2, 3, 0 },{ 1, 1, 4, 0 },
		{ 1, 0, 10, -1 },{ 1, 3, 3, 0 },{ 2, 1, 3, 0 },{ 2, 0, 10, -1 },{ 2, 3, 1, -1 },{ 2, 2, 3, 0 },{ 0, 2, 3, 0 },{ 2, 3, 0, -1 },
		{ 2, 3, 2, -1 },{ 0, 3, 3, 0 },{ 1, 2, 4, -1 },{ 2, 3, 3, -1 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 4: 2 subsets, Weight bits: 46 bits, Endpoint bits: 72 bits (11.444, 11.444, 11.555), delta
		{ { 0, 0, 9, 0 },{ 1, 0, 9, 0 },{ 2, 0, 9, 0 },{ 0, 1, 3, 0 },{ 0, 0, 10, -1 },{ 2, 2, 4, -1 },{ 1, 2, 3, 0 },{ 1, 1, 3, 0 },
		{ 1, 0, 10, -1 },{ 2, 3, 0, -1 },{ 1, 3, 3, 0 },{ 2, 1, 4, 0 },{ 2, 0, 10, -1 },{ 2, 2, 3, 0 },{ 0, 2, 3, 0 },{ 2, 3, 1, -1 },
		{ 2, 3, 2, -1 },{ 0, 3, 3, 0 },{ 2, 3, 4, -1 },{ 2, 3, 3, -1 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 5: 2 subsets, Weight bits: 46 bits, Endpoint bits: 72 bits (9.555, 9.555, 9.555), delta
		{ { 0, 0, 8, 0 },{ 2, 2, 4, -1 },{ 1, 0, 8, 0 },{ 1, 2, 4, -1 },{ 2, 0, 8, 0 },{ 2, 3, 4, -1 },{ 0, 1, 4, 0 },{ 1, 3, 4, -1 },
		{ 1, 2, 3, 0 },{ 1, 1, 4, 0 },{ 2, 3, 0, -1 },{ 1, 3, 3, 0 },{ 2, 1, 4, 0 },{ 2, 3, 1, -1 },{ 2, 2, 3, 0 },{ 0, 2, 4, 0 },
		{ 2, 3, 2, -1 },{ 0, 3, 4, 0 },{ 2, 3, 3, -1 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 6: 2 subsets, Weight bits: 46 bits, Endpoint bits: 72 bits (8.666, 8.555, 8.555), delta
		{ { 0, 0, 7, 0 },{ 1, 3, 4, -1 },{ 2, 2, 4, -1 },{ 1, 0, 7, 0 },{ 2, 3, 2, -1 },{ 1, 2, 4, -1 },{ 2, 0, 7, 0 },{ 2, 3, 3, -1 },
		{ 2, 3, 4, -1 },{ 0, 1, 5, 0 },{ 1, 2, 3, 0 },{ 1, 1, 4, 0 },{ 2, 3, 0, -1 },{ 1, 3, 3, 0 },{ 2, 1, 4, 0 },{ 2, 3, 1, -1 },
		{ 2, 2, 3, 0 },{ 0, 2, 5, 0 },{ 0, 3, 5, 0 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 7: 2 subsets, Weight bits: 46 bits, Endpoints bits: 72 bits (8.555, 8.666, 8.555), delta
		{ { 0, 0, 7, 0 },{ 2, 3, 0, -1 },{ 2, 2, 4, -1 },{ 1, 0, 7, 0 },{ 1, 2, 5, -1 },{ 1, 2, 4, -1 },{ 2, 0, 7, 0 },{ 1, 3, 5, -1 },
		{ 2, 3, 4, -1 },{ 0, 1, 4, 0 },{ 1, 3, 4, -1 },{ 1, 2, 3, 0 },{ 1, 1, 5, 0 },{ 1, 3, 3, 0 },{ 2, 1, 4, 0 },{ 2, 3, 1, -1 },
		{ 2, 2, 3, 0 },{ 0, 2, 4, 0 },{ 2, 3, 2, -1 },{ 0, 3, 4, 0 },{ 2, 3, 3, -1 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 8: 2 subsets, Weight bits: 46 bits, Endpoint bits: 72 bits (8.555, 8.555, 8.666), delta
		{ { 0, 0, 7, 0 },{ 2, 3, 1, -1 },{ 2, 2, 4, -1 },{ 1, 0, 7, 0 },{ 2, 2, 5, -1 },{ 1, 2, 4, -1 },{ 2, 0, 7, 0 },{ 2, 3, 5, -1 },
		{ 2, 3, 4, -1 },{ 0, 1, 4, 0 },{ 1, 3, 4, -1 },{ 1, 2, 3, 0 },{ 1, 1, 4, 0 },{ 2, 3, 0, -1 },{ 1, 3, 3, 0 },{ 2, 1, 5, 0 },
		{ 2, 2, 3, 0 },{ 0, 2, 4, 0 },{ 2, 3, 2, -1 },{ 0, 3, 4, 0 },{ 2, 3, 3, -1 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 9: 2 subsets, Weight bits: 46 bits, Endpoint bits: 72 bits (6.6.6.6, 6.6.6.6, 6.6.6.6), NO delta
		{ { 0, 0, 5, 0 },{ 1, 3, 4, -1 },{ 2, 3, 0, -1 },{ 2, 3, 1, -1 },{ 2, 2, 4, -1 },{ 1, 0, 5, 0 },{ 1, 2, 5, -1 },{ 2, 2, 5, -1 },
		{ 2, 3, 2, -1 },{ 1, 2, 4, -1 },{ 2, 0, 5, 0 },{ 1, 3, 5, -1 },{ 2, 3, 3, -1 },{ 2, 3, 5, -1 },{ 2, 3, 4, -1 },{ 0, 1, 5, 0 },
		{ 1, 2, 3, 0 },{ 1, 1, 5, 0 },{ 1, 3, 3, 0 },{ 2, 1, 5, 0 },{ 2, 2, 3, 0 },{ 0, 2, 5, 0 },{ 0, 3, 5, 0 },{ 3, -1, 4, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 10: 1 subset, Weight bits: 63 bits, Endpoint bits: 60 bits (10.10, 10.10, 10.10), NO delta
		{ { 0, 0, 9, 0 },{ 1, 0, 9, 0 },{ 2, 0, 9, 0 },{ 0, 1, 9, 0 },{ 1, 1, 9, 0 },{ 2, 1, 9, 0 }, {-1, 0, 0, 0} },
		//------------------------        mode 11: 1 subset, Weight bits: 63 bits, Endpoint bits: 60 bits (11.9, 11.9, 11.9), delta
		{ { 0, 0, 9, 0 },{ 1, 0, 9, 0 },{ 2, 0, 9, 0 },{ 0, 1, 8, 0 },{ 0, 0, 10, -1 },{ 1, 1, 8, 0 },{ 1, 0, 10, -1 },{ 2, 1, 8, 0 },{ 2, 0, 10, -1 }, {-1, 0, 0, 0} },
		//------------------------        mode 12: 1 subset, Weight bits: 63 bits, Endpoint bits: 60 bits (12.8, 12.8, 12.8), delta
		{ { 0, 0, 9, 0 },{ 1, 0, 9, 0 },{ 2, 0, 9, 0 },{ 0, 1, 7, 0 },{ 0, 0, 10, 11 },{ 1, 1, 7, 0 },{ 1, 0, 10, 11 },{ 2, 1, 7, 0 },{ 2, 0, 10, 11 }, {-1, 0, 0, 0} },
		//------------------------        mode 13: 1 subset, Weight bits: 63 bits, Endpoint bits: 60 bits (16.4, 16.4, 16.4), delta
		{ { 0, 0, 9, 0 },{ 1, 0, 9, 0 },{ 2, 0, 9, 0 },{ 0, 1, 3, 0 },{ 0, 0, 10, 15 },{ 1, 1, 3, 0 },{ 1, 0, 10, 15 },{ 2, 1, 3, 0 },{ 2, 0, 10, 15 }, {-1, 0, 0, 0} }
	};

	// The same as the first 32 2-subset patterns in BC7. 
	// Bit 7 is a flag indicating that the weight uses 1 less bit than usual.
	const uint8_t g_bc6h_2subset_patterns[TOTAL_BC6H_PARTITION_PATTERNS][4][4] = // [pat][y][x]
	{
		{ {0x80, 0, 1, 1}, { 0, 0, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 1, 0x81 }}, { {0x80, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 0x81} },
		{ {0x80, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 0x81} }, { {0x80, 0, 0, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 1, 1, 0x81} },
		{ {0x80, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 1, 0x81} }, { {0x80, 0, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 0x81} },
		{ {0x80, 0, 0, 1}, {0, 0, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 0x81} }, { {0x80, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 1}, {0, 1, 1, 0x81} },
		{ {0x80, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 0x81} }, { {0x80, 0, 1, 1}, {0, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 0x81} },
		{ {0x80, 0, 0, 0}, {0, 0, 0, 1}, {0, 1, 1, 1}, {1, 1, 1, 0x81} }, { {0x80, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 1, 1, 0x81} },
		{ {0x80, 0, 0, 1}, {0, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 0x81} }, { {0x80, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 1}, {1, 1, 1, 0x81} },
		{ {0x80, 0, 0, 0}, {1, 1, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 0x81} }, { {0x80, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 1, 1, 0x81} },
		{ {0x80, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {1, 1, 1, 0x81} }, { {0x80, 1, 0x81, 1}, {0, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0} },
		{ {0x80, 0, 0, 0}, {0, 0, 0, 0}, {0x81, 0, 0, 0}, {1, 1, 1, 0} }, { {0x80, 1, 0x81, 1}, {0, 0, 1, 1}, {0, 0, 0, 1}, {0, 0, 0, 0} },
		{ {0x80, 0, 0x81, 1}, {0, 0, 0, 1}, {0, 0, 0, 0}, {0, 0, 0, 0} }, { {0x80, 0, 0, 0}, {1, 0, 0, 0}, {0x81, 1, 0, 0}, {1, 1, 1, 0} },
		{ {0x80, 0, 0, 0}, {0, 0, 0, 0}, {0x81, 0, 0, 0}, {1, 1, 0, 0} }, { {0x80, 1, 1, 1}, {0, 0, 1, 1}, {  0, 0, 1, 1}, {0, 0, 0, 0x81} },
		{ {0x80, 0, 0x81, 1}, {0, 0, 0, 1}, {0, 0, 0, 1}, {0, 0, 0, 0} }, { {0x80, 0, 0, 0}, {1, 0, 0, 0}, {0x81, 0, 0, 0}, {1, 1, 0, 0} },
		{ {0x80, 1, 0x81, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 1, 1, 0} }, { {0x80, 0, 0x81, 1}, {0, 1, 1, 0}, {0, 1, 1, 0}, {1, 1, 0, 0} },
		{ {0x80, 0, 0, 1}, {0, 1, 1, 1}, {0x81, 1, 1, 0}, {1, 0, 0, 0} }, { {0x80, 0, 0, 0}, {1, 1, 1, 1}, {0x81, 1, 1, 1}, {0, 0, 0, 0} },
		{ {0x80, 1, 0x81, 1}, {0, 0, 0, 1}, {1, 0, 0, 0}, {1, 1, 1, 0} }, { {0x80, 0, 0x81, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 1, 0, 0} }
	};

	const uint8_t g_bc6h_weight3[8] = { 0, 9, 18, 27, 37, 46, 55, 64 };
	const uint8_t g_bc6h_weight4[16] = { 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 };
	
	static inline void write_bits(uint64_t val, uint32_t num_bits, uint32_t& bit_pos, uint64_t& l, uint64_t& h)
	{
		assert((num_bits) && (num_bits < 64) && (bit_pos < 128));
		assert(val < (1ULL << num_bits));

		if (bit_pos < 64)
		{
			l |= (val << bit_pos);

			if ((bit_pos + num_bits) > 64)
				h |= (val >> (64 - bit_pos));
		}
		else
		{
			h |= (val << (bit_pos - 64));
		}

		bit_pos += num_bits;
		assert(bit_pos <= 128);
	}

	static inline void write_rev_bits(uint64_t val, uint32_t num_bits, uint32_t& bit_pos, uint64_t& l, uint64_t& h)
	{
		assert((num_bits) && (num_bits < 64) && (bit_pos < 128));
		assert(val < (1ULL << num_bits));

		for (uint32_t i = 0; i < num_bits; i++)
			write_bits((val >> (num_bits - 1u - i)) & 1, 1, bit_pos, l, h);
	}

	void pack_bc6h_block(bc6h_block& dst_blk, bc6h_logical_block& log_blk)
	{
		const uint8_t s_mode_bits[NUM_BC6H_MODES] = { 0b00, 0b01, 0b00010, 0b00110, 0b01010, 0b01110, 0b10010, 0b10110, 0b11010, 0b11110, 0b00011, 0b00111, 0b01011, 0b01111 };

		const uint32_t mode = log_blk.m_mode;
		assert(mode < NUM_BC6H_MODES);

		uint64_t l = s_mode_bits[mode], h = 0;
		uint32_t bit_pos = (mode >= 2) ? 5 : 2;

		const uint32_t num_subsets = (mode >= BC6H_FIRST_1SUBSET_MODE_INDEX) ? 1 : 2;

		assert(((num_subsets == 2) && (log_blk.m_partition_pattern < TOTAL_BC6H_PARTITION_PATTERNS)) ||
			((num_subsets == 1) && (!log_blk.m_partition_pattern)));

		// Sanity checks
		for (uint32_t c = 0; c < 3; c++)
		{
			assert(log_blk.m_endpoints[c][0] < (1u << g_bc6h_mode_sig_bits[mode][0]));	   // 1st subset l, base bits
			assert(log_blk.m_endpoints[c][1] < (1u << g_bc6h_mode_sig_bits[mode][c + 1])); // 1st subset h, these are deltas except for modes 9,10
			assert(log_blk.m_endpoints[c][2] < (1u << g_bc6h_mode_sig_bits[mode][c + 1])); // 2nd subset l
			assert(log_blk.m_endpoints[c][3] < (1u << g_bc6h_mode_sig_bits[mode][c + 1])); // 2nd subset h
		}

		const bc6h_bit_layout* pLayout = &g_bc6h_bit_layouts[mode][0];

		while (pLayout->m_comp != -1)
		{
			uint32_t v = (pLayout->m_comp == 3) ? log_blk.m_partition_pattern : log_blk.m_endpoints[pLayout->m_comp][pLayout->m_index];

			if (pLayout->m_first_bit == -1)
			{
				write_bits((v >> pLayout->m_last_bit) & 1, 1, bit_pos, l, h);
			}
			else
			{
				const uint32_t total_bits = basisu::iabs(pLayout->m_last_bit - pLayout->m_first_bit) + 1;

				v >>= basisu::minimum(pLayout->m_first_bit, pLayout->m_last_bit);
				v &= ((1 << total_bits) - 1);

				if (pLayout->m_first_bit > pLayout->m_last_bit)
					write_rev_bits(v, total_bits, bit_pos, l, h);
				else
					write_bits(v, total_bits, bit_pos, l, h);
			}

			pLayout++;
		}

		const uint32_t num_mode_sel_bits = (num_subsets == 1) ? 4 : 3;
		const uint8_t* pPat = &g_bc6h_2subset_patterns[log_blk.m_partition_pattern][0][0];

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t sel = log_blk.m_weights[i];

			uint32_t num_bits = num_mode_sel_bits;
			if (num_subsets == 2)
			{
				const uint32_t subset_index = pPat[i];
				num_bits -= (subset_index >> 7);
			}
			else if (!i)
			{
				num_bits--;
			}

			assert(sel < (1u << num_bits));

			write_bits(sel, num_bits, bit_pos, l, h);
		}

		assert(bit_pos == 128);

		basisu::write_le_dword(&dst_blk.m_bytes[0], (uint32_t)l);
		basisu::write_le_dword(&dst_blk.m_bytes[4], (uint32_t)(l >> 32u));
		basisu::write_le_dword(&dst_blk.m_bytes[8], (uint32_t)h);
		basisu::write_le_dword(&dst_blk.m_bytes[12], (uint32_t)(h >> 32u));
	}


	namespace astc_6x6_hdr
	{
		const uint32_t g_bc6h_weights4[16] = { 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 };

#if 0
		static BASISU_FORCE_INLINE int pos_lrintf(float x)
		{
			assert(x >= 0.0f);
			return (int)(x + .5f);
		}

		static BASISU_FORCE_INLINE basist::half_float fast_float_to_half_non_neg_no_nan_inf(float val)
		{
			union { float f; int32_t i; uint32_t u; } fi = { val };
			const int flt_m = fi.i & 0x7FFFFF, flt_e = (fi.i >> 23) & 0xFF;
			int e = 0, m = 0;

			assert(((fi.i >> 31) == 0) && (flt_e != 0xFF));

			// not zero or denormal
			if (flt_e != 0)
			{
				int new_exp = flt_e - 127;
				if (new_exp > 15)
					e = 31;
				else if (new_exp < -14)
					m = pos_lrintf((1 << 24) * fabsf(fi.f));
				else
				{
					e = new_exp + 15;
					m = pos_lrintf(flt_m * (1.0f / ((float)(1 << 13))));
				}
			}

			assert((0 <= m) && (m <= 1024));
			if (m == 1024)
			{
				e++;
				m = 0;
			}

			assert((e >= 0) && (e <= 31));
			assert((m >= 0) && (m <= 1023));

			basist::half_float result = (basist::half_float)((e << 10) | m);
			return result;
		}
#endif

		union fu32
		{
			uint32_t u;
			float f;
		};

		static BASISU_FORCE_INLINE basist::half_float fast_float_to_half_no_clamp_neg_nan_or_inf(float f)
		{
			assert(!isnan(f) && !isinf(f));
			assert((f >= 0.0f) && (f <= basist::MAX_HALF_FLOAT));

			// Sutract 112 from the exponent, to change the bias from 127 to 15.
			static const fu32 g_f_to_h{ 0x7800000 };

			fu32 fu;

			fu.f = f * g_f_to_h.f;

			uint32_t h = (basist::half_float)((fu.u >> (23 - 10)) & 0x7FFF);

			// round to even
			uint32_t mant = fu.u & 8191; // examine lowest 13 bits
			h += (mant > 4096);

			if (h > basist::MAX_HALF_FLOAT_AS_INT_BITS)
				h = basist::MAX_HALF_FLOAT_AS_INT_BITS;

			return (basist::half_float)h;
		}

		static BASISU_FORCE_INLINE float ftoh(float f)
		{
			//float res = (float)fast_float_to_half_non_neg_no_nan_inf(fabsf(f)) * ((f < 0.0f) ? -1.0f : 1.0f);
			float res = (float)fast_float_to_half_no_clamp_neg_nan_or_inf(fabsf(f)) * ((f < 0.0f) ? -1.0f : 1.0f);
			return res;
		}
		
		// Supports positive and denormals only. No NaN or Inf.
		static BASISU_FORCE_INLINE float fast_half_to_float_pos_not_inf_or_nan(basist::half_float h)
		{
			assert(!basist::half_is_signed(h) && !basist::is_half_inf_or_nan(h));

			// add 112 to the exponent (112+half float's exp bias of 15=float32's bias of 127)
			static const fu32 K = { 0x77800000 };

			fu32 o;
			o.u = h << 13;
			o.f *= K.f;

			return o.f;
		}

		static BASISU_FORCE_INLINE float inv_sqrt(float v)
		{
			union
			{
				float flt;
				uint32_t ui;
			} un;

			un.flt = v;
			un.ui = 0x5F1FFFF9UL - (un.ui >> 1);

			return 0.703952253f * un.flt * (2.38924456f - v * (un.flt * un.flt));
		}

		static const int FAST_BC6H_STD_DEV_THRESH = 256;
		static const int FAST_BC6H_COMPLEX_STD_DEV_THRESH = 512;
		static const int FAST_BC6H_VERY_COMPLEX_STD_DEV_THRESH = 2048;
						
		static double assign_weights_4(
			const vec3F* pFloat_pixels, const float* pPixel_scales,
			uint8_t* pWeights,
			int min_r, int min_g, int min_b,
			int max_r, int max_g, int max_b, int64_t block_max_var, bool try_2subsets_flag, 
			const fast_bc6h_params& params)
		{
			float cr[16], cg[16], cb[16];

			for (uint32_t i = 0; i < 16; i++)
			{
				const uint32_t w = g_bc6h_weights4[i];

				cr[i] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_r * (64 - w) + max_r * w + 32) >> 6));
				cg[i] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_g * (64 - w) + max_g * w + 32) >> 6));
				cb[i] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_b * (64 - w) + max_b * w + 32) >> 6));
			}

			double total_err = 0.0f;

			if (params.m_brute_force_weight4_assignment)
			{
				for (uint32_t i = 0; i < 16; i++)
				{
					const float qr = pFloat_pixels[i].c[0], qg = pFloat_pixels[i].c[1], qb = pFloat_pixels[i].c[2];

					float best_err = basisu::squaref(cr[0] - qr) + basisu::squaref(cg[0] - qg) + basisu::squaref(cb[0] - qb);
					uint32_t best_idx = 0;

					for (uint32_t j = 1; j < 16; j++)
					{
						float rd = cr[j] - qr, gd = cg[j] - qg, bd = cb[j] - qb;
						float e = rd * rd + gd * gd + bd * bd;

						if (e < best_err)
						{
							best_err = e;
							best_idx = j;
						}
					}

					pWeights[i] = (uint8_t)best_idx;

					total_err += best_err * pPixel_scales[i];
				}
			}
			else
			{
				const float dir_r = cr[15] - cr[0], dir_g = cg[15] - cg[0], dir_b = cb[15] - cb[0];

				float dots[16];
				for (uint32_t i = 0; i < 16; i++)
					dots[i] = cr[i] * dir_r + cg[i] * dir_g + cb[i] * dir_b;

				float mid_dots[15];
				bool monotonically_increasing = true;
				for (uint32_t i = 0; i < 15; i++)
				{
					mid_dots[i] = (dots[i] + dots[i + 1]) * .5f;

					if (dots[i] > dots[i + 1])
						monotonically_increasing = false;
				}

				const bool check_more_colors = block_max_var > (FAST_BC6H_VERY_COMPLEX_STD_DEV_THRESH * FAST_BC6H_VERY_COMPLEX_STD_DEV_THRESH * 16); // watch prec

				if (!monotonically_increasing)
				{
					// Seems very rare, not worth optimizing the other cases
					for (uint32_t i = 0; i < 16; i++)
					{
						const float qr = pFloat_pixels[i].c[0], qg = pFloat_pixels[i].c[1], qb = pFloat_pixels[i].c[2];

						float d = qr * dir_r + qg * dir_g + qb * dir_b;

						float best_e = fabsf(d - dots[0]);
						int best_idx = 0;

						for (int j = 1; j < 16; j++)
						{
							float e = fabsf(d - dots[j]);
							if (e < best_e)
							{
								best_e = e;
								best_idx = j;
							}
						}

						assert((best_idx >= 0) && (best_idx <= 15));

						pWeights[i] = (uint8_t)best_idx;

						float err = basisu::squaref(qr - cr[best_idx]) + basisu::squaref(qg - cg[best_idx]) + basisu::squaref(qb - cb[best_idx]);
						total_err += err * pPixel_scales[i];
					}
				}
				else if ((!try_2subsets_flag) || (!check_more_colors))
				{
					for (uint32_t i = 0; i < 16; i++)
					{
						const float qr = pFloat_pixels[i].c[0], qg = pFloat_pixels[i].c[1], qb = pFloat_pixels[i].c[2];

						uint32_t best_idx = 0;

						float d = qr * dir_r + qg * dir_g + qb * dir_b;

						int low = 0;

						int mid = low + 7;
						if (d >= mid_dots[mid]) low = mid + 1;
						mid = low + 3;
						if (d >= mid_dots[mid]) low = mid + 1;
						mid = low + 1;
						if (d >= mid_dots[mid]) low = mid + 1;
						mid = low;
						if (d >= mid_dots[mid]) low = mid + 1;

						best_idx = low;
						assert((best_idx <= 15));

						pWeights[i] = (uint8_t)best_idx;

						// Giesen's MRSSE (Mean Relative Sum of Squared Errors). 
						// Our ASTC HDR encoder uses slightly slower approx. MSLE, and it's too late/risky to eval the difference vs. MRSSE on the larger ASTC HDR blocks.
						float err = basisu::squaref(qr - cr[best_idx]) + basisu::squaref(qg - cg[best_idx]) + basisu::squaref(qb - cb[best_idx]);
						total_err += err * pPixel_scales[i];
					}
				}
				else
				{
					for (uint32_t i = 0; i < 16; i++)
					{
						const float qr = pFloat_pixels[i].c[0], qg = pFloat_pixels[i].c[1], qb = pFloat_pixels[i].c[2];

						uint32_t best_idx = 0;

						float d = qr * dir_r + qg * dir_g + qb * dir_b;

						int low = 0;

						int mid = low + 7;
						if (d >= mid_dots[mid]) low = mid + 1;
						mid = low + 3;
						if (d >= mid_dots[mid]) low = mid + 1;
						mid = low + 1;
						if (d >= mid_dots[mid]) low = mid + 1;
						mid = low;
						if (d >= mid_dots[mid]) low = mid + 1;

						best_idx = low;
						assert((best_idx <= 15));

						float err = basisu::squaref(qr - cr[best_idx]) + basisu::squaref(qg - cg[best_idx]) + basisu::squaref(qb - cb[best_idx]);

						{
							int alt_idx = best_idx + 1;
							if (alt_idx > 15)
								alt_idx = 13;

							float alt_err = basisu::squaref(qr - cr[alt_idx]) + basisu::squaref(qg - cg[alt_idx]) + basisu::squaref(qb - cb[alt_idx]);
							if (alt_err < err)
							{
								err = alt_err;
								best_idx = alt_idx;
							}
						}

						{
							int alt_idx2 = best_idx - 1;
							if (alt_idx2 < 0)
								alt_idx2 = 2;
							float alt_err2 = basisu::squaref(qr - cr[alt_idx2]) + basisu::squaref(qg - cg[alt_idx2]) + basisu::squaref(qb - cb[alt_idx2]);
							if (alt_err2 < err)
							{
								err = alt_err2;
								best_idx = alt_idx2;
							}
						}

						pWeights[i] = (uint8_t)best_idx;

						total_err += err * pPixel_scales[i];
					}
				}
			}

			return total_err;
		}

		static void assign_weights_simple_4(
			const basist::half_float* pPixels,
			uint8_t* pWeights,
			int min_r, int min_g, int min_b,
			int max_r, int max_g, int max_b, int64_t block_max_var, 
			const fast_bc6h_params& params)
		{
			BASISU_NOTE_UNUSED(block_max_var);

			float fmin_r = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)min_r);
			float fmin_g = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)min_g);
			float fmin_b = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)min_b);

			float fmax_r = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)max_r);
			float fmax_g = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)max_g);
			float fmax_b = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)max_b);

			float fdir_r = fmax_r - fmin_r;
			float fdir_g = fmax_g - fmin_g;
			float fdir_b = fmax_b - fmin_b;

			float l = inv_sqrt(fdir_r * fdir_r + fdir_g * fdir_g + fdir_b * fdir_b);
			if (l != 0.0f)
			{
				fdir_r *= l;
				fdir_g *= l;
				fdir_b *= l;
			}

			float lf = fmin_r * fdir_r + fmin_g * fdir_g + fmin_b * fdir_b;
			float hf = fmax_r * fdir_r + fmax_g * fdir_g + fmax_b * fdir_b;

			if ((lf >= basist::MAX_HALF_FLOAT) || (hf >= basist::MAX_HALF_FLOAT))
			{
				// v2.1: Can't use the faster half float based tricks below, need some sort of backup
				vec3F float_pixels[16];
				float pixel_scales[16];

				for (uint32_t i = 0; i < 16; i++)
				{
					float_pixels[i].c[0] = fast_half_to_float_pos_not_inf_or_nan(pPixels[i * 3 + 0]);
					float_pixels[i].c[1] = fast_half_to_float_pos_not_inf_or_nan(pPixels[i * 3 + 1]);
					float_pixels[i].c[2] = fast_half_to_float_pos_not_inf_or_nan(pPixels[i * 3 + 2]);
					
					pixel_scales[i] = 1.0f / (basisu::squaref(float_pixels[i].c[0]) + basisu::squaref(float_pixels[i].c[1]) + basisu::squaref(float_pixels[i].c[2]) + (float)MIN_HALF_FLOAT);
				}

				assign_weights_4(
					float_pixels, pixel_scales,
					pWeights,
					min_r, min_g, min_b,
					max_r, max_g, max_b, block_max_var, false,
					params);
				
				return;
			}

			float lr = ftoh(lf);
			float hr = ftoh(hf);

			float frr = (hr == lr) ? 0.0f : (14.93333f / (float)(hr - lr));

			lr = (-lr * frr) + 0.53333f;
			for (uint32_t i = 0; i < 16; i++)
			{
				const float r = fast_half_to_float_pos_not_inf_or_nan(pPixels[i * 3 + 0]);
				const float g = fast_half_to_float_pos_not_inf_or_nan(pPixels[i * 3 + 1]);
				const float b = fast_half_to_float_pos_not_inf_or_nan(pPixels[i * 3 + 2]);
				const float w = ftoh(basisu::minimumf(r * fdir_r + g * fdir_g + b * fdir_b, basist::MAX_HALF_FLOAT));

				pWeights[i] = (uint8_t)basisu::clamp((int)(w * frr + lr), 0, 15);
			}
		}

		static void assign_weights3(uint8_t trial_weights[16],
			uint32_t best_pat_bits,
			uint32_t subset_min_r[2], uint32_t subset_min_g[2], uint32_t subset_min_b[2],
			uint32_t subset_max_r[2], uint32_t subset_max_g[2], uint32_t subset_max_b[2],
			const vec3F* pFloat_pixels)
		{
			float subset_cr[2][8], subset_cg[2][8], subset_cb[2][8];

			for (uint32_t subset = 0; subset < 2; subset++)
			{
				const uint32_t min_r = subset_min_r[subset], min_g = subset_min_g[subset], min_b = subset_min_b[subset];
				const uint32_t max_r = subset_max_r[subset], max_g = subset_max_g[subset], max_b = subset_max_b[subset];

				for (uint32_t j = 0; j < 8; j++)
				{
					const uint32_t w = g_bc7_weights3[j];

					subset_cr[subset][j] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_r * (64 - w) + max_r * w + 32) >> 6));
					subset_cg[subset][j] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_g * (64 - w) + max_g * w + 32) >> 6));
					subset_cb[subset][j] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_b * (64 - w) + max_b * w + 32) >> 6));
				} // j

			} // subset

			// TODO: Plane optimization?

			for (uint32_t i = 0; i < 16; i++)
			{
				const uint32_t subset = (best_pat_bits >> i) & 1;
				const float qr = pFloat_pixels[i].c[0], qg = pFloat_pixels[i].c[1], qb = pFloat_pixels[i].c[2];

				float best_error = basisu::squaref(subset_cr[subset][0] - qr) + basisu::squaref(subset_cg[subset][0] - qg) + basisu::squaref(subset_cb[subset][0] - qb);
				uint32_t best_idx = 0;
								
				for (uint32_t j = 1; j < 8; j++)
				{
					float e = basisu::squaref(subset_cr[subset][j] - qr) + basisu::squaref(subset_cg[subset][j] - qg) + basisu::squaref(subset_cb[subset][j] - qb);
					if (e < best_error)
					{
						best_error = e;
						best_idx = j;
					}
				}

				trial_weights[i] = (uint8_t)best_idx;

			} // i
		}

		static double assign_weights_error_3(uint8_t trial_weights[16],
			uint32_t best_pat_bits,
			uint32_t subset_min_r[2], uint32_t subset_min_g[2], uint32_t subset_min_b[2],
			uint32_t subset_max_r[2], uint32_t subset_max_g[2], uint32_t subset_max_b[2],
			const vec3F* pFloat_pixels, const float* pPixel_scales)
		{
			float subset_cr[2][8], subset_cg[2][8], subset_cb[2][8];

			for (uint32_t subset = 0; subset < 2; subset++)
			{
				const uint32_t min_r = subset_min_r[subset], min_g = subset_min_g[subset], min_b = subset_min_b[subset];
				const uint32_t max_r = subset_max_r[subset], max_g = subset_max_g[subset], max_b = subset_max_b[subset];

				for (uint32_t j = 0; j < 8; j++)
				{
					const uint32_t w = g_bc7_weights3[j];

					subset_cr[subset][j] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_r * (64 - w) + max_r * w + 32) >> 6));
					subset_cg[subset][j] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_g * (64 - w) + max_g * w + 32) >> 6));
					subset_cb[subset][j] = fast_half_to_float_pos_not_inf_or_nan((basist::half_float)((min_b * (64 - w) + max_b * w + 32) >> 6));
				} // j

			} // subset

			double trial_error = 0.0f;

			// TODO: Plane optimization?

			for (uint32_t i = 0; i < 16; i++)
			{
				const uint32_t subset = (best_pat_bits >> i) & 1;
				const float qr = pFloat_pixels[i].c[0], qg = pFloat_pixels[i].c[1], qb = pFloat_pixels[i].c[2];

				float best_error = basisu::squaref(subset_cr[subset][0] - qr) + basisu::squaref(subset_cg[subset][0] - qg) + basisu::squaref(subset_cb[subset][0] - qb);
				uint32_t best_idx = 0;

				for (uint32_t j = 1; j < 8; j++)
				{
					float e = basisu::squaref(subset_cr[subset][j] - qr) + basisu::squaref(subset_cg[subset][j] - qg) + basisu::squaref(subset_cb[subset][j] - qb);
					if (e < best_error)
					{
						best_error = e;
						best_idx = j;
					}
				}

				trial_weights[i] = (uint8_t)best_idx;

				trial_error += best_error * pPixel_scales[i];

			} // i

			return trial_error;
		}

		static basist::vec4F g_bc6h_ls_weights_3[8];
		static basist::vec4F g_bc6h_ls_weights_4[16];
				
		const uint32_t BC6H_NUM_PATS = 32;
		static uint32_t g_bc6h_pats2[BC6H_NUM_PATS];

		void fast_encode_bc6h_init()
		{
			for (uint32_t i = 0; i < 8; i++)
			{
				const float w = (float)g_bc7_weights3[i] * (1.0f / 64.0f);
				g_bc6h_ls_weights_3[i].set(w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w);
			}

			for (uint32_t i = 0; i < 16; i++)
			{
				const float w = (float)g_bc6h_weights4[i] * (1.0f / 64.0f);
				g_bc6h_ls_weights_4[i].set(w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w);
			}

			for (uint32_t pat_index = 0; pat_index < BC6H_NUM_PATS; pat_index++)
			{
				uint32_t pat_bits = 0;

				for (uint32_t j = 0; j < 16; j++)
					pat_bits |= (g_bc7_partition2[pat_index * 16 + j] << j);

				g_bc6h_pats2[pat_index] = pat_bits;
			}
		}

		static int bc6h_dequantize(int val, int bits)
		{
			assert(val < (1 << bits));

			int result;
			if (bits >= 15)
				result = val;
			else if (!val)
				result = 0;
			else if (val == ((1 << bits) - 1))
				result = 0xFFFF;
			else
				result = ((val << 16) + 0x8000) >> bits;
			return result;
		}

		static inline basist::half_float bc6h_convert_to_half(int val)
		{
			assert(val < 65536);

			// scale by 31/64
			return (basist::half_float)((val * 31) >> 6);
		}

		static void bc6h_quant_dequant_endpoints(uint32_t& min_r, uint32_t& min_g, uint32_t& min_b, uint32_t& max_r, uint32_t& max_g, uint32_t& max_b, int bits) // bits=10
		{
			min_r = bc6h_convert_to_half(bc6h_dequantize(basist::bc6h_half_to_blog((basist::half_float)min_r, bits), bits));
			min_g = bc6h_convert_to_half(bc6h_dequantize(basist::bc6h_half_to_blog((basist::half_float)min_g, bits), bits));
			min_b = bc6h_convert_to_half(bc6h_dequantize(basist::bc6h_half_to_blog((basist::half_float)min_b, bits), bits));

			max_r = bc6h_convert_to_half(bc6h_dequantize(basist::bc6h_half_to_blog((basist::half_float)max_r, bits), bits));
			max_g = bc6h_convert_to_half(bc6h_dequantize(basist::bc6h_half_to_blog((basist::half_float)max_g, bits), bits));
			max_b = bc6h_convert_to_half(bc6h_dequantize(basist::bc6h_half_to_blog((basist::half_float)max_b, bits), bits));
		}

		static void bc6h_quant_endpoints(
			uint32_t min_hr, uint32_t min_hg, uint32_t min_hb, uint32_t max_hr, uint32_t max_hg, uint32_t max_hb,
			uint32_t& min_r, uint32_t& min_g, uint32_t& min_b, uint32_t& max_r, uint32_t& max_g, uint32_t& max_b, 
			int bits)
		{
			min_r = basist::bc6h_half_to_blog((basist::half_float)min_hr, bits);
			min_g = basist::bc6h_half_to_blog((basist::half_float)min_hg, bits);
			min_b = basist::bc6h_half_to_blog((basist::half_float)min_hb, bits);

			max_r = basist::bc6h_half_to_blog((basist::half_float)max_hr, bits);
			max_g = basist::bc6h_half_to_blog((basist::half_float)max_hg, bits);
			max_b = basist::bc6h_half_to_blog((basist::half_float)max_hb, bits);
		}

		static void bc6h_dequant_endpoints(
			uint32_t min_br, uint32_t min_bg, uint32_t min_bb, uint32_t max_br, uint32_t max_bg, uint32_t max_bb,
			uint32_t& min_hr, uint32_t& min_hg, uint32_t& min_hb, uint32_t& max_hr, uint32_t& max_hg, uint32_t& max_hb,
			int bits)
		{
			min_hr = bc6h_convert_to_half(bc6h_dequantize(min_br, bits));
			min_hg = bc6h_convert_to_half(bc6h_dequantize(min_bg, bits));
			min_hb = bc6h_convert_to_half(bc6h_dequantize(min_bb, bits));

			max_hr = bc6h_convert_to_half(bc6h_dequantize(max_br, bits));
			max_hg = bc6h_convert_to_half(bc6h_dequantize(max_bg, bits));
			max_hb = bc6h_convert_to_half(bc6h_dequantize(max_bb, bits));
		}

		static BASISU_FORCE_INLINE int popcount32(uint32_t x) 
		{
#if defined(__EMSCRIPTEN__) || defined(__clang__) || defined(__GNUC__)
			return __builtin_popcount(x);
#elif defined(_MSC_VER)
			return __popcnt(x);
#else
			int count = 0;
			while (x) 
			{
				x &= (x - 1);
				++count;
			}
			return count;
#endif
		}

		static BASISU_FORCE_INLINE int fast_roundf_int(float x)
		{
			return (x >= 0.0f) ? (int)(x + 0.5f) : (int)(x - 0.5f);
		}
												
		static void fast_encode_bc6h_2subsets_pattern(
			uint32_t best_pat_index, uint32_t best_pat_bits,
			const basist::half_float* pPixels, const vec3F* pFloat_pixels, const float* pPixel_scales,
			double& cur_error, basist::bc6h_logical_block& log_blk,
			int64_t block_max_var,
			int mean_r, int mean_g, int mean_b, 
			const fast_bc6h_params& params)
		{
			BASISU_NOTE_UNUSED(block_max_var);
						
			uint32_t subset_means[2][3] = { { 0 } };
			for (uint32_t i = 0; i < 16; i++)
			{
				const uint32_t subset_index = (best_pat_bits >> i) & 1;
				const uint32_t r = pPixels[i * 3 + 0], g = pPixels[i * 3 + 1], b = pPixels[i * 3 + 2];
				
				subset_means[subset_index][0] += r;
				subset_means[subset_index][1] += g;
				subset_means[subset_index][2] += b;
			}

			for (uint32_t s = 0; s < 2; s++)
				for (uint32_t c = 0; c < 3; c++)
					subset_means[s][c] = (subset_means[s][c] + 8) / 16;

			int64_t subset_icov[2][6] = { { 0 } };

			for (uint32_t i = 0; i < 16; i++)
			{
				const uint32_t subset_index = (best_pat_bits >> i) & 1;
				const int r = (int)pPixels[i * 3 + 0] - mean_r, g = (int)pPixels[i * 3 + 1] - mean_g, b = (int)pPixels[i * 3 + 2] - mean_b;

				subset_icov[subset_index][0] += r * r;
				subset_icov[subset_index][1] += r * g;
				subset_icov[subset_index][2] += r * b;
				subset_icov[subset_index][3] += g * g;
				subset_icov[subset_index][4] += g * b;
				subset_icov[subset_index][5] += b * b;
			}

			vec3F subset_axis[2];

			for (uint32_t subset_index = 0; subset_index < 2; subset_index++)
			{
				float cov[6];
				for (uint32_t i = 0; i < 6; i++)
					cov[i] = (float)subset_icov[subset_index][i];

				const float sc = 1.0f / (basisu::maximum(cov[0], cov[3], cov[5]) + basisu::REALLY_SMALL_FLOAT_VAL);
				const float wx = sc * cov[0], wy = sc * cov[3], wz = sc * cov[5];

				const float alt_xr = cov[0] * wx + cov[1] * wy + cov[2] * wz;
				const float alt_xg = cov[1] * wx + cov[3] * wy + cov[4] * wz;
				const float alt_xb = cov[2] * wx + cov[4] * wy + cov[5] * wz;

				float l = basisu::squaref(alt_xr) + basisu::squaref(alt_xg) + basisu::squaref(alt_xb);

				float axis_r = 0.57735027f, axis_g = 0.57735027f, axis_b = 0.57735027f;
				if (fabs(l) >= basisu::SMALL_FLOAT_VAL)
				{
					const float inv_l = inv_sqrt(l);
					axis_r = alt_xr * inv_l;
					axis_g = alt_xg * inv_l;
					axis_b = alt_xb * inv_l;
				}

				subset_axis[subset_index].set(axis_r, axis_g, axis_b);
			} // s
						
			float subset_min_dot[2] = { basisu::BIG_FLOAT_VAL, basisu::BIG_FLOAT_VAL };
			float subset_max_dot[2] = { -basisu::BIG_FLOAT_VAL, -basisu::BIG_FLOAT_VAL };
			int subset_min_idx[2] = { 0 }, subset_max_idx[2] = { 0 };

			for (uint32_t i = 0; i < 16; i++)
			{
				const uint32_t subset_index = (best_pat_bits >> i) & 1;
				const float r = (float)pPixels[i * 3 + 0], g = (float)pPixels[i * 3 + 1], b = (float)pPixels[i * 3 + 2];
				const float dot = r * subset_axis[subset_index].c[0] + g * subset_axis[subset_index].c[1] + b * subset_axis[subset_index].c[2];

				if (dot < subset_min_dot[subset_index])
				{
					subset_min_dot[subset_index] = dot;
					subset_min_idx[subset_index] = i;
				}

				if (dot > subset_max_dot[subset_index])
				{
					subset_max_dot[subset_index] = dot;
					subset_max_idx[subset_index] = i;
				}
			} // i

			uint32_t subset_min_r[2], subset_min_g[2], subset_min_b[2];
			uint32_t subset_max_r[2], subset_max_g[2], subset_max_b[2];

			for (uint32_t subset_index = 0; subset_index < 2; subset_index++)
			{
				const uint32_t min_index = subset_min_idx[subset_index] * 3, max_index = subset_max_idx[subset_index] * 3;

				subset_min_r[subset_index] = pPixels[min_index + 0];
				subset_min_g[subset_index] = pPixels[min_index + 1];
				subset_min_b[subset_index] = pPixels[min_index + 2];

				subset_max_r[subset_index] = pPixels[max_index + 0];
				subset_max_g[subset_index] = pPixels[max_index + 1];
				subset_max_b[subset_index] = pPixels[max_index + 2];

			} // subset_index

			// least squares with unquantized endpoints
			const bool use_ls = true;
			if (use_ls)
			{
				uint8_t trial_weights[16];
				assign_weights3(trial_weights, best_pat_bits, subset_min_r, subset_min_g, subset_min_b, subset_max_r, subset_max_g, subset_max_b, pFloat_pixels);

				float z00[2] = { 0.0f }, z01[2] = { 0.0f }, z10[2] = { 0.0f }, z11[2] = { 0.0f };
				float q00_r[2] = { 0.0f }, q10_r[2] = { 0.0f }, t_r[2] = { 0.0f };
				float q00_g[2] = { 0.0f }, q10_g[2] = { 0.0f }, t_g[2] = { 0.0f };
				float q00_b[2] = { 0.0f }, q10_b[2] = { 0.0f }, t_b[2] = { 0.0f };

				for (uint32_t i = 0; i < 16; i++)
				{
					const uint32_t subset = (best_pat_bits >> i) & 1;

					float r = (float)pPixels[i * 3 + 0];
					float g = (float)pPixels[i * 3 + 1];
					float b = (float)pPixels[i * 3 + 2];

					const uint32_t sel = trial_weights[i];

					z00[subset] += g_bc6h_ls_weights_3[sel][0];
					z10[subset] += g_bc6h_ls_weights_3[sel][1];
					z11[subset] += g_bc6h_ls_weights_3[sel][2];

					float w = g_bc6h_ls_weights_3[sel][3];

					q00_r[subset] += w * r;
					t_r[subset] += r;

					q00_g[subset] += w * g;
					t_g[subset] += g;

					q00_b[subset] += w * b;
					t_b[subset] += b;
				}

				for (uint32_t subset = 0; subset < 2; subset++)
				{
					q10_r[subset] = t_r[subset] - q00_r[subset];
					q10_g[subset] = t_g[subset] - q00_g[subset];
					q10_b[subset] = t_b[subset] - q00_b[subset];

					z01[subset] = z10[subset];

					float det = z00[subset] * z11[subset] - z01[subset] * z10[subset];
					if (fabs(det) >= basisu::SMALL_FLOAT_VAL)
					{
						det = 1.0f / det;

						float iz00 = z11[subset] * det;
						float iz01 = -z01[subset] * det;
						float iz10 = -z10[subset] * det;
						float iz11 = z00[subset] * det;

						subset_max_r[subset] = basisu::clamp<int>(fast_roundf_int(iz00 * q00_r[subset] + iz01 * q10_r[subset]), 0, (int)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);
						subset_min_r[subset] = basisu::clamp<int>(fast_roundf_int(iz10 * q00_r[subset] + iz11 * q10_r[subset]), 0, (int)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);

						subset_max_g[subset] = basisu::clamp<int>(fast_roundf_int(iz00 * q00_g[subset] + iz01 * q10_g[subset]), 0, (int)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);
						subset_min_g[subset] = basisu::clamp<int>(fast_roundf_int(iz10 * q00_g[subset] + iz11 * q10_g[subset]), 0, (int)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);

						subset_max_b[subset] = basisu::clamp<int>(fast_roundf_int(iz00 * q00_b[subset] + iz01 * q10_b[subset]), 0, (int)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);
						subset_min_b[subset] = basisu::clamp<int>(fast_roundf_int(iz10 * q00_b[subset] + iz11 * q10_b[subset]), 0, (int)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);
					}
				} // subset
			}

			const int BC6H_2SUBSET_ABS_ENDPOINT_MODE = 9;

			int bc6h_mode_index = BC6H_2SUBSET_ABS_ENDPOINT_MODE, num_endpoint_bits = 6;
			uint32_t abs_blog_endpoints[3][4];

			if (params.m_num_diff_endpoint_modes_to_try)
			{
				// ordered from largest base bits to least
				static const int s_bc6h_mode_order2[2] = { 5, 1 }; 
				static const int s_bc6h_mode_order4[4] = { 0, 5, 7, 1 };
				static const int s_bc6h_mode_order9[9] = { 2, 3, 4, 0,  5, 6, 7, 8,  1 };

				uint32_t num_endpoint_modes = 2;
				const int* pBC6H_mode_order = s_bc6h_mode_order2;

				if (params.m_num_diff_endpoint_modes_to_try >= 9)
				{
					num_endpoint_modes = 9;
					pBC6H_mode_order = s_bc6h_mode_order9;
				}
				else if (params.m_num_diff_endpoint_modes_to_try >= 4)
				{
					num_endpoint_modes = 4;
					pBC6H_mode_order = s_bc6h_mode_order4;
				}

				// Find the BC6H mode that will conservatively encode our trial endpoints. The mode chosen will handle any endpoint swaps.
				for (uint32_t bc6h_mode_iter = 0; bc6h_mode_iter < num_endpoint_modes; bc6h_mode_iter++)
				{
					const uint32_t mode = pBC6H_mode_order[bc6h_mode_iter];

					const uint32_t num_base_bits = g_bc6h_mode_sig_bits[mode][0];
					const int base_bitmask = (1 << num_base_bits) - 1;
					BASISU_NOTE_UNUSED(base_bitmask);

					const uint32_t num_delta_bits[3] = { g_bc6h_mode_sig_bits[mode][1], g_bc6h_mode_sig_bits[mode][2], g_bc6h_mode_sig_bits[mode][3] };
					//const int delta_bitmasks[3] = { (1 << num_delta_bits[0]) - 1, (1 << num_delta_bits[1]) - 1, (1 << num_delta_bits[2]) - 1 };

					for (uint32_t subset_index = 0; subset_index < 2; subset_index++)
					{
						bc6h_quant_endpoints(
							subset_min_r[subset_index], subset_min_g[subset_index], subset_min_b[subset_index], subset_max_r[subset_index], subset_max_g[subset_index], subset_max_b[subset_index],
							abs_blog_endpoints[0][subset_index * 2 + 0], abs_blog_endpoints[1][subset_index * 2 + 0], abs_blog_endpoints[2][subset_index * 2 + 0],
							abs_blog_endpoints[0][subset_index * 2 + 1], abs_blog_endpoints[1][subset_index * 2 + 1], abs_blog_endpoints[2][subset_index * 2 + 1],
							num_base_bits);
					}

					uint32_t c;
					for (c = 0; c < 3; c++)
					{
						// a very conservative check because we don't have the weight indices yet, so we don't know how to swap end point values
						// purposely enforcing a symmetric limit here so we can invert any endpoints later if needed
						const int max_delta = (1 << (num_delta_bits[c] - 1)) - 1;
						const int min_delta = -max_delta;

						int delta0 = (int)abs_blog_endpoints[c][1] - (int)abs_blog_endpoints[c][0];
						if ((delta0 < min_delta) || (delta0 > max_delta))
							break;

						int delta1 = (int)abs_blog_endpoints[c][2] - (int)abs_blog_endpoints[c][0];
						if ((delta1 < min_delta) || (delta1 > max_delta))
							break;

						int delta2 = (int)abs_blog_endpoints[c][3] - (int)abs_blog_endpoints[c][0];
						if ((delta2 < min_delta) || (delta2 > max_delta))
							break;

						// in case the endpoints are swapped
						int delta3 = (int)abs_blog_endpoints[c][2] - (int)abs_blog_endpoints[c][1];
						if ((delta3 < min_delta) || (delta3 > max_delta))
							break;

						int delta4 = (int)abs_blog_endpoints[c][3] - (int)abs_blog_endpoints[c][1];
						if ((delta4 < min_delta) || (delta4 > max_delta))
							break;
					}

					if (c == 3)
					{
						bc6h_mode_index = mode;
						num_endpoint_bits = num_base_bits;
						break;
					}
				}
			}

			if (bc6h_mode_index == BC6H_2SUBSET_ABS_ENDPOINT_MODE)
			{
				for (uint32_t subset_index = 0; subset_index < 2; subset_index++)
				{
					bc6h_quant_endpoints(
						subset_min_r[subset_index], subset_min_g[subset_index], subset_min_b[subset_index], subset_max_r[subset_index], subset_max_g[subset_index], subset_max_b[subset_index],
						abs_blog_endpoints[0][subset_index * 2 + 0], abs_blog_endpoints[1][subset_index * 2 + 0], abs_blog_endpoints[2][subset_index * 2 + 0],
						abs_blog_endpoints[0][subset_index * 2 + 1], abs_blog_endpoints[1][subset_index * 2 + 1], abs_blog_endpoints[2][subset_index * 2 + 1],
						num_endpoint_bits);
				}
			}

			for (uint32_t subset_index = 0; subset_index < 2; subset_index++)
			{
				bc6h_dequant_endpoints(
					abs_blog_endpoints[0][subset_index * 2 + 0], abs_blog_endpoints[1][subset_index * 2 + 0], abs_blog_endpoints[2][subset_index * 2 + 0],
					abs_blog_endpoints[0][subset_index * 2 + 1], abs_blog_endpoints[1][subset_index * 2 + 1], abs_blog_endpoints[2][subset_index * 2 + 1],
					subset_min_r[subset_index], subset_min_g[subset_index], subset_min_b[subset_index],
					subset_max_r[subset_index], subset_max_g[subset_index], subset_max_b[subset_index], num_endpoint_bits);
			}

			uint8_t trial_weights[16];
			double trial_error = assign_weights_error_3(trial_weights, best_pat_bits, subset_min_r, subset_min_g, subset_min_b, subset_max_r, subset_max_g, subset_max_b, pFloat_pixels, pPixel_scales);

			if (trial_error < cur_error)
			{
				basist::bc6h_logical_block trial_log_blk;

				trial_log_blk.m_mode = bc6h_mode_index;
				trial_log_blk.m_partition_pattern = best_pat_index;
				
				memcpy(trial_log_blk.m_endpoints, abs_blog_endpoints, sizeof(trial_log_blk.m_endpoints));
				memcpy(trial_log_blk.m_weights, trial_weights, 16);
							
				if (trial_log_blk.m_weights[0] & 4)
				{
					for (uint32_t c = 0; c < 3; c++)
						std::swap(trial_log_blk.m_endpoints[c][0], trial_log_blk.m_endpoints[c][1]);

					for (uint32_t i = 0; i < 16; i++)
					{
						const uint32_t subset_index = (best_pat_bits >> i) & 1;
						if (subset_index == 0)
							trial_log_blk.m_weights[i] = 7 - trial_log_blk.m_weights[i];
					}
				}

				const uint32_t subset2_anchor_index = g_bc7_table_anchor_index_second_subset[best_pat_index];
				if (trial_log_blk.m_weights[subset2_anchor_index] & 4)
				{
					for (uint32_t c = 0; c < 3; c++)
						std::swap(trial_log_blk.m_endpoints[c][2], trial_log_blk.m_endpoints[c][3]);

					for (uint32_t i = 0; i < 16; i++)
					{
						const uint32_t subset_index = (best_pat_bits >> i) & 1;
						if (subset_index == 1)
							trial_log_blk.m_weights[i] = 7 - trial_log_blk.m_weights[i];
					}
				}
								
				if (bc6h_mode_index != BC6H_2SUBSET_ABS_ENDPOINT_MODE)
				{
					const uint32_t num_delta_bits[3] = { g_bc6h_mode_sig_bits[bc6h_mode_index][1], g_bc6h_mode_sig_bits[bc6h_mode_index][2], g_bc6h_mode_sig_bits[bc6h_mode_index][3] };
					const int delta_bitmasks[3] = { (1 << num_delta_bits[0]) - 1, (1 << num_delta_bits[1]) - 1, (1 << num_delta_bits[2]) - 1 };

					for (uint32_t c = 0; c < 3; c++)
					{
						const int delta0 = (int)trial_log_blk.m_endpoints[c][1] - (int)trial_log_blk.m_endpoints[c][0];
						const int delta1 = (int)trial_log_blk.m_endpoints[c][2] - (int)trial_log_blk.m_endpoints[c][0];
						const int delta2 = (int)trial_log_blk.m_endpoints[c][3] - (int)trial_log_blk.m_endpoints[c][0];

#ifdef _DEBUG
						// sanity check the final endpoints
						const int max_delta = (1 << (num_delta_bits[c] - 1)) - 1;
						const int min_delta = -(max_delta + 1);
						assert((max_delta - min_delta) == delta_bitmasks[c]);

						if ((delta0 < min_delta) || (delta0 > max_delta) || (delta1 < min_delta) || (delta1 > max_delta) || (delta2 < min_delta) || (delta2 > max_delta))
						{
							assert(0);
							break;
						}
#endif

						trial_log_blk.m_endpoints[c][1] = delta0 & delta_bitmasks[c];
						trial_log_blk.m_endpoints[c][2] = delta1 & delta_bitmasks[c];
						trial_log_blk.m_endpoints[c][3] = delta2 & delta_bitmasks[c];

					} // c
				}

				cur_error = trial_error;
				log_blk = trial_log_blk;
			}
		}

		static void fast_encode_bc6h_2subsets(
			const basist::half_float* pPixels, const vec3F* pFloat_pixels, const float* pPixel_scales,
			double& cur_error, basist::bc6h_logical_block& log_blk,
			int64_t block_max_var,
			int mean_r, int mean_g, int mean_b, float block_axis_r, float block_axis_g, float block_axis_b, 
			const fast_bc6h_params& params)
		{
			assert((params.m_max_2subset_pats_to_try > 0) && (params.m_max_2subset_pats_to_try <= BC6H_NUM_PATS));

			if (params.m_max_2subset_pats_to_try == BC6H_NUM_PATS)
			{
				for (uint32_t i = 0; i < BC6H_NUM_PATS; i++)
				{
					const uint32_t best_pat_index = i;
					const uint32_t best_pat_bits = g_bc6h_pats2[best_pat_index];

					fast_encode_bc6h_2subsets_pattern(
						best_pat_index, best_pat_bits,
						pPixels, pFloat_pixels, pPixel_scales,
						cur_error, log_blk,
						block_max_var,
						mean_r, mean_g, mean_b, params);
				}
				return;
			}
			
			uint32_t desired_pat_bits = 0;
			for (uint32_t i = 0; i < 16; i++)
			{
				float f = (float)(pPixels[i * 3 + 0] - mean_r) * block_axis_r +
					(float)(pPixels[i * 3 + 1] - mean_g) * block_axis_g +
					(float)(pPixels[i * 3 + 2] - mean_b) * block_axis_b;

				desired_pat_bits |= (((f >= 0.0f) ? 1 : 0) << i);
			} // i

			if (params.m_max_2subset_pats_to_try == 1)
			{
				uint32_t best_diff = UINT32_MAX;
				for (uint32_t p = 0; p < BC6H_NUM_PATS; p++)
				{
					const uint32_t bc6h_pat_bits = g_bc6h_pats2[p];

					int diff = popcount32(bc6h_pat_bits ^ desired_pat_bits);
					int diff_inv = 16 - diff;

					uint32_t min_diff = (basisu::minimum<int>(diff, diff_inv) << 8) | p;
					if (min_diff < best_diff)
						best_diff = min_diff;
				} // p

				const uint32_t best_pat_index = best_diff & 0xFF;
				const uint32_t best_pat_bits = g_bc6h_pats2[best_pat_index];

				fast_encode_bc6h_2subsets_pattern(
					best_pat_index, best_pat_bits,
					pPixels, pFloat_pixels, pPixel_scales,
					cur_error, log_blk,
					block_max_var,
					mean_r, mean_g, mean_b, params);
			}
			else
			{
				assert(params.m_max_2subset_pats_to_try <= BC6H_NUM_PATS);
				uint32_t pat_diffs[BC6H_NUM_PATS];

				for (uint32_t p = 0; p < BC6H_NUM_PATS; p++)
				{
					const uint32_t bc6h_pat_bits = g_bc6h_pats2[p];

					int diff = popcount32(bc6h_pat_bits ^ desired_pat_bits);
					int diff_inv = 16 - diff;

					pat_diffs[p] = (basisu::minimum<int>(diff, diff_inv) << 8) | p;
				} // p

				std::sort(pat_diffs, pat_diffs + BC6H_NUM_PATS);

				for (uint32_t pat_iter = 0; pat_iter < params.m_max_2subset_pats_to_try; pat_iter++)
				{
					const uint32_t best_pat_index = pat_diffs[pat_iter] & 0xFF;
					const uint32_t best_pat_bits = g_bc6h_pats2[best_pat_index];

					fast_encode_bc6h_2subsets_pattern(
						best_pat_index, best_pat_bits,
						pPixels, pFloat_pixels, pPixel_scales,
						cur_error, log_blk,
						block_max_var,
						mean_r, mean_g, mean_b, params);
				}
			}
		}

		void fast_encode_bc6h(const basist::half_float* pPixels, basist::bc6h_block* pBlock, const fast_bc6h_params &params)
		{
			basist::bc6h_logical_block log_blk;
			log_blk.clear();

			log_blk.m_mode = basist::BC6H_FIRST_1SUBSET_MODE_INDEX;

			uint32_t omin_r = UINT32_MAX, omin_g = UINT32_MAX, omin_b = UINT32_MAX;
			uint32_t omax_r = 0, omax_g = 0, omax_b = 0;
			uint32_t total_r = 0, total_g = 0, total_b = 0;
						
			for (uint32_t i = 0; i < 16; i++)
			{
				uint32_t r = pPixels[i * 3 + 0];
				uint32_t g = pPixels[i * 3 + 1];
				uint32_t b = pPixels[i * 3 + 2];
								
				total_r += r;
				total_g += g;
				total_b += b;

				omin_r = basisu::minimum(omin_r, r);
				omin_g = basisu::minimum(omin_g, g);
				omin_b = basisu::minimum(omin_b, b);

				omax_r = basisu::maximum(omax_r, r);
				omax_g = basisu::maximum(omax_g, g);
				omax_b = basisu::maximum(omax_b, b);
			}

			if ((omin_r == omax_r) && (omin_g == omax_g) && (omin_b == omax_b))
			{
				// Solid block
				log_blk.m_endpoints[0][0] = basist::bc6h_half_to_blog16((basist::half_float)omin_r);
				log_blk.m_endpoints[0][1] = 0;

				log_blk.m_endpoints[1][0] = basist::bc6h_half_to_blog16((basist::half_float)omin_g);
				log_blk.m_endpoints[1][1] = 0;

				log_blk.m_endpoints[2][0] = basist::bc6h_half_to_blog16((basist::half_float)omin_b);
				log_blk.m_endpoints[2][1] = 0;
				
				log_blk.m_mode = 13;
				pack_bc6h_block(*pBlock, log_blk);

				return;
			}
			
			uint32_t min_r, min_g, min_b, max_r, max_g, max_b;

			int mean_r = (total_r + 8) / 16;
			int mean_g = (total_g + 8) / 16;
			int mean_b = (total_b + 8) / 16;

			int64_t icov[6] = { 0, 0, 0, 0, 0, 0 };

			for (uint32_t i = 0; i < 16; i++)
			{
				int r = (int)pPixels[i * 3 + 0] - mean_r;
				int g = (int)pPixels[i * 3 + 1] - mean_g;
				int b = (int)pPixels[i * 3 + 2] - mean_b;

				icov[0] += r * r;
				icov[1] += r * g;
				icov[2] += r * b;
				icov[3] += g * g;
				icov[4] += g * b;
				icov[5] += b * b;
			}
						
			int64_t block_max_var = basisu::maximum(icov[0], icov[3], icov[5]); // not divided by 16, i.e. scaled by 16
			
			if (block_max_var < (FAST_BC6H_STD_DEV_THRESH * FAST_BC6H_STD_DEV_THRESH * 16))
			{
				// Simple block
				min_r = (omax_r - omin_r) / 32 + omin_r;
				min_g = (omax_g - omin_g) / 32 + omin_g;
				min_b = (omax_b - omin_b) / 32 + omin_b;

				max_r = ((omax_r - omin_r) * 31) / 32 + omin_r;
				max_g = ((omax_g - omin_g) * 31) / 32 + omin_g;
				max_b = ((omax_b - omin_b) * 31) / 32 + omin_b;

				assert((max_r < MAX_HALF_FLOAT_AS_INT_BITS) && (max_g < MAX_HALF_FLOAT_AS_INT_BITS) && (max_b < MAX_HALF_FLOAT_AS_INT_BITS));

				bc6h_quant_dequant_endpoints(min_r, min_g, min_b, max_r, max_g, max_b, 10);

				assign_weights_simple_4(pPixels, log_blk.m_weights, min_r, min_g, min_b, max_r, max_g, max_b, block_max_var, params);
				
				log_blk.m_endpoints[0][0] = basist::bc6h_half_to_blog((basist::half_float)min_r, 10);
				log_blk.m_endpoints[0][1] = basist::bc6h_half_to_blog((basist::half_float)max_r, 10);

				log_blk.m_endpoints[1][0] = basist::bc6h_half_to_blog((basist::half_float)min_g, 10);
				log_blk.m_endpoints[1][1] = basist::bc6h_half_to_blog((basist::half_float)max_g, 10);

				log_blk.m_endpoints[2][0] = basist::bc6h_half_to_blog((basist::half_float)min_b, 10);
				log_blk.m_endpoints[2][1] = basist::bc6h_half_to_blog((basist::half_float)max_b, 10);

				if (log_blk.m_weights[0] & 8)
				{
					for (uint32_t i = 0; i < 16; i++)
						log_blk.m_weights[i] = 15 - log_blk.m_weights[i];

					for (uint32_t c = 0; c < 3; c++)
					{
						std::swap(log_blk.m_endpoints[c][0], log_blk.m_endpoints[c][1]);
					}
				}

				pack_bc6h_block(*pBlock, log_blk);

				return;
			}

			// block_max_var cannot be 0 here, also trace cannot be 0

			// Complex block (edges/strong gradients)
			bool try_2subsets = false;
			double cur_err = 0.0f;
			vec3F float_pixels[16];
			float pixel_scales[16];

			// covar rows are:
			// 0, 1, 2
			// 1, 3, 4
			// 2, 4, 5
			float cov[6];
			for (uint32_t i = 0; i < 6; i++)
				cov[i] = (float)icov[i];

			const float sc = 1.0f / (float)block_max_var;
			const float wx = sc * cov[0], wy = sc * cov[3], wz = sc * cov[5];

			const float alt_xr = cov[0] * wx + cov[1] * wy + cov[2] * wz;
			const float alt_xg = cov[1] * wx + cov[3] * wy + cov[4] * wz;
			const float alt_xb = cov[2] * wx + cov[4] * wy + cov[5] * wz;

			float l = basisu::squaref(alt_xr) + basisu::squaref(alt_xg) + basisu::squaref(alt_xb);

			float axis_r = 0.57735027f, axis_g = 0.57735027f, axis_b = 0.57735027f;
			if (fabs(l) >= basisu::SMALL_FLOAT_VAL)
			{
				const float inv_l = inv_sqrt(l);
				axis_r = alt_xr * inv_l;
				axis_g = alt_xg * inv_l;
				axis_b = alt_xb * inv_l;
			}

			const float tr = axis_r * cov[0] + axis_g * cov[1] + axis_b * cov[2];
			const float tg = axis_r * cov[1] + axis_g * cov[3] + axis_b * cov[4];
			const float tb = axis_r * cov[2] + axis_g * cov[4] + axis_b * cov[5];
			const float principle_axis_var = tr * axis_r + tg * axis_g + tb * axis_b;

			const float inv_principle_axis_var = 1.0f / (principle_axis_var + basisu::REALLY_SMALL_FLOAT_VAL);
			axis_r = tr * inv_principle_axis_var;
			axis_g = tg * inv_principle_axis_var;
			axis_b = tb * inv_principle_axis_var;

			float total_var = cov[0] + cov[3] + cov[5];

			// If the principle axis variance vs. the block's total variance accounts for less than this threshold, it's a "very complex" block that may benefit from 2 subsets.
			const float COMPLEX_BLOCK_PRINCIPLE_AXIS_FRACT_THRESH = .995f;
			try_2subsets = principle_axis_var < (total_var * COMPLEX_BLOCK_PRINCIPLE_AXIS_FRACT_THRESH);

			uint32_t min_idx = 0, max_idx = 0;
			float min_dot = basisu::BIG_FLOAT_VAL, max_dot = -basisu::BIG_FLOAT_VAL;
								
			for (uint32_t i = 0; i < 16; i++)
			{
				float r = (float)pPixels[i * 3 + 0];
				float g = (float)pPixels[i * 3 + 1];
				float b = (float)pPixels[i * 3 + 2];

				float_pixels[i].c[0] = fast_half_to_float_pos_not_inf_or_nan((half_float)r);
				float_pixels[i].c[1] = fast_half_to_float_pos_not_inf_or_nan((half_float)g);
				float_pixels[i].c[2] = fast_half_to_float_pos_not_inf_or_nan((half_float)b);

				pixel_scales[i] = 1.0f / (basisu::squaref(float_pixels[i].c[0]) + basisu::squaref(float_pixels[i].c[1]) + basisu::squaref(float_pixels[i].c[2]) + (float)MIN_HALF_FLOAT);

				float dot = r * axis_r + g * axis_g + b * axis_b;

				if (dot < min_dot)
				{
					min_dot = dot;
					min_idx = i;
				}

				if (dot > max_dot)
				{
					max_dot = dot;
					max_idx = i;
				}
			}

			min_r = pPixels[min_idx * 3 + 0];
			min_g = pPixels[min_idx * 3 + 1];
			min_b = pPixels[min_idx * 3 + 2];

			max_r = pPixels[max_idx * 3 + 0];
			max_g = pPixels[max_idx * 3 + 1];
			max_b = pPixels[max_idx * 3 + 2];

			//assert((max_r < MAX_HALF_FLOAT_AS_INT_BITS) && (max_g < MAX_HALF_FLOAT_AS_INT_BITS) && (max_b < MAX_HALF_FLOAT_AS_INT_BITS));
			assert((max_r <= MAX_HALF_FLOAT_AS_INT_BITS) && (max_g <= MAX_HALF_FLOAT_AS_INT_BITS) && (max_b <= MAX_HALF_FLOAT_AS_INT_BITS));

			bc6h_quant_dequant_endpoints(min_r, min_g, min_b, max_r, max_g, max_b, 10);

			cur_err = assign_weights_4(float_pixels, pixel_scales, log_blk.m_weights, min_r, min_g, min_b, max_r, max_g, max_b, block_max_var, try_2subsets, params);
						
			const uint32_t MAX_LS_PASSES = params.m_hq_ls ? 2 : 1;
			for (uint32_t pass = 0; pass < MAX_LS_PASSES; pass++)
			{
				float z00 = 0.0f, z01 = 0.0f, z10 = 0.0f, z11 = 0.0f;
				float q00_r = 0.0f, q10_r = 0.0f, t_r = 0.0f;
				float q00_g = 0.0f, q10_g = 0.0f, t_g = 0.0f;
				float q00_b = 0.0f, q10_b = 0.0f, t_b = 0.0f;

				for (uint32_t i = 0; i < 16; i++)
				{
					float r = (float)pPixels[i * 3 + 0];
					float g = (float)pPixels[i * 3 + 1];
					float b = (float)pPixels[i * 3 + 2];

					const uint32_t sel = log_blk.m_weights[i];

					z00 += g_bc6h_ls_weights_4[sel][0];
					z10 += g_bc6h_ls_weights_4[sel][1];
					z11 += g_bc6h_ls_weights_4[sel][2];

					float w = g_bc6h_ls_weights_4[sel][3];

					q00_r += w * r;
					t_r += r;

					q00_g += w * g;
					t_g += g;

					q00_b += w * b;
					t_b += b;
				}

				q10_r = t_r - q00_r;
				q10_g = t_g - q00_g;
				q10_b = t_b - q00_b;

				z01 = z10;

				float det = z00 * z11 - z01 * z10;
				if (fabs(det) < basisu::SMALL_FLOAT_VAL)
					break;

				det = 1.0f / det;

				float iz00 = z11 * det;
				float iz01 = -z01 * det;
				float iz10 = -z10 * det;
				float iz11 = z00 * det;

				uint32_t trial_max_r = (int)basisu::clamp<float>(std::round(iz00 * q00_r + iz01 * q10_r), 0, (float)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);
				uint32_t trial_min_r = (int)basisu::clamp<float>(std::round(iz10 * q00_r + iz11 * q10_r), 0, (float)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);

				uint32_t trial_max_g = (int)basisu::clamp<float>(std::round(iz00 * q00_g + iz01 * q10_g), 0, (float)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);
				uint32_t trial_min_g = (int)basisu::clamp<float>(std::round(iz10 * q00_g + iz11 * q10_g), 0, (float)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);

				uint32_t trial_max_b = (int)basisu::clamp<float>(std::round(iz00 * q00_b + iz01 * q10_b), 0, (float)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);
				uint32_t trial_min_b = (int)basisu::clamp<float>(std::round(iz10 * q00_b + iz11 * q10_b), 0, (float)basist::MAX_BC6H_HALF_FLOAT_AS_UINT);

				bc6h_quant_dequant_endpoints(trial_min_r, trial_min_g, trial_min_b, trial_max_r, trial_max_g, trial_max_b, 10);

				uint8_t trial_weights[16];
				double trial_err = assign_weights_4(float_pixels, pixel_scales, trial_weights, trial_min_r, trial_min_g, trial_min_b, trial_max_r, trial_max_g, trial_max_b, block_max_var, try_2subsets, params);

				if (trial_err < cur_err)
				{
					cur_err = trial_err;

					min_r = trial_min_r;
					max_r = trial_max_r;

					min_g = trial_min_g;
					max_g = trial_max_g;

					min_b = trial_min_b;
					max_b = trial_max_b;
												
					memcpy(log_blk.m_weights, trial_weights, 16);
				}
				else
				{
					break;
				}

			} // pass

#if 0
			//if (full_flag)
			if ((try_2subsets) && (block_max_var > (FAST_BC6H_COMPLEX_STD_DEV_THRESH * FAST_BC6H_COMPLEX_STD_DEV_THRESH * 16)))
			{
				min_r = 0;
				max_r = 0;
				min_g = 0;
				max_g = 0;
				min_b = 0;
				max_b = 0;
			}
#endif

			log_blk.m_endpoints[0][0] = basist::bc6h_half_to_blog((basist::half_float)min_r, 10);
			log_blk.m_endpoints[0][1] = basist::bc6h_half_to_blog((basist::half_float)max_r, 10);

			log_blk.m_endpoints[1][0] = basist::bc6h_half_to_blog((basist::half_float)min_g, 10);
			log_blk.m_endpoints[1][1] = basist::bc6h_half_to_blog((basist::half_float)max_g, 10);

			log_blk.m_endpoints[2][0] = basist::bc6h_half_to_blog((basist::half_float)min_b, 10);
			log_blk.m_endpoints[2][1] = basist::bc6h_half_to_blog((basist::half_float)max_b, 10);

			if (log_blk.m_weights[0] & 8)
			{
				for (uint32_t i = 0; i < 16; i++)
					log_blk.m_weights[i] = 15 - log_blk.m_weights[i];

				for (uint32_t c = 0; c < 3; c++)
				{
					std::swap(log_blk.m_endpoints[c][0], log_blk.m_endpoints[c][1]);
				}
			}
			
			if ((params.m_max_2subset_pats_to_try > 0) && ((try_2subsets) && (block_max_var > (FAST_BC6H_COMPLEX_STD_DEV_THRESH * FAST_BC6H_COMPLEX_STD_DEV_THRESH * 16))))
			{
				fast_encode_bc6h_2subsets(pPixels, float_pixels, pixel_scales, cur_err, log_blk, block_max_var, mean_r, mean_g, mean_b, axis_r, axis_g, axis_b, params);
			}

			pack_bc6h_block(*pBlock, log_blk);
		}
	} // namespace astc_6x6_hdr

} // namespace basist
