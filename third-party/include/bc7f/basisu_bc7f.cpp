// clang-format off
// basisu_bc7f.cpp - Self-contained BC7 block encoder extracted from Binomial LLC's
// basis_universal (https://github.com/BinomialLLC/basis_universal), commit 99f52d6.
// The code is the "bc7f" real-time analytical encoder from
// transcoder/basisu_transcoder.cpp (basist::bc7f, plus the basist::bc7u logical/physical
// BC7 block packing code it depends on), with only the minimal types/helpers it needs
// from transcoder/basisu.h, transcoder/basisu_transcoder_internal.h and
// transcoder/basisu_transcoder_uastc.h, so it can be built without the rest of the
// transcoder.
//
// Modified during extraction (Apache 2.0 license section 4(b)):
//  - The ASTC output path (pack_from_astc_*, fast_pack_astc_*, cem_encode_*) and the
//    UASTC/XUASTC transcoding paths were removed.
//  - bc7f::init() also initializes the BC7 weight quantization and optimal endpoint
//    tables (upstream initializes them in basisu_transcoder_init()/uastc_init()).
// See LICENSE for the Apache 2.0 license.

#include "basisu_bc7f.h"

#include <algorithm>
#include <assert.h>
#include <math.h>
#include <cmath>
#include <climits>
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

// Additional minimal subset of the basisu namespace helpers (from
// transcoder/basisu.h) needed by the BC7 encoder implementation.
namespace basisu
{
	constexpr float REALLY_SMALL_FLOAT_VAL = .000000125f;
	constexpr float SMALL_FLOAT_VAL = .0000125f;
	constexpr float BIG_FLOAT_VAL = 1e+30f;

	template <typename S> inline S maximum(S a, S b, S c) { return maximum(maximum(a, b), c); }
	template <typename S> inline S maximum(S a, S b, S c, S d) { return maximum(maximum(a, b), c, d); }

	template <typename S> inline S minimum(S a, S b, S c) { return minimum(minimum(a, b), c); }
	template <typename S> inline S minimum(S a, S b, S c, S d) { return minimum(minimum(a, b), c, d); }

	inline float minimumf(float a, float b) { return (a < b) ? a : b; }
	inline float maximumf(float a, float b) { return (a > b) ? a : b; }
	inline float squaref(float i) { return i * i; }
	inline int squarei(int i) { return i * i; }
	inline uint32_t iabs(int32_t i) { return (i < 0) ? static_cast<uint32_t>(-i) : static_cast<uint32_t>(i); }

	template <typename T> inline void clear_obj(T& obj) { memset((void*)&obj, 0, sizeof(obj)); }
}

namespace basist
{
	struct vec4F
	{
		float c[4];

		inline void set(float x, float y, float z, float w) { c[0] = x; c[1] = y; c[2] = z; c[3] = w; }

		float operator[] (uint32_t index) const { assert(index < 4); return c[index]; }
		float& operator[] (uint32_t index) { assert(index < 4); return c[index]; }
	};

	struct endpoint_err
	{
		uint16_t m_error; uint8_t m_lo; uint8_t m_hi;
	};

	const uint32_t BC7ENC_MODE_6_OPTIMAL_INDEX = 5;
	const uint32_t BC7ENC_MODE_5_OPTIMAL_INDEX = 1;

	static inline int32_t clampi(int32_t value, int32_t low, int32_t high) { if (value < low) value = low; else if (value > high) value = high;	return value; }

	// Upstream resolves bc7u's unqualified fast_roundf_int() call through a file-scope
	// "using namespace basist::astc_6x6_hdr"; provided here at basist scope instead.
	static BASISU_FORCE_INLINE int fast_roundf_int(float x)
	{
		return (x >= 0.0f) ? (int)(x + 0.5f) : (int)(x - 0.5f);
	}

	// ASTC/BC7 common partition tables (from transcoder/basisu_transcoder_uastc.h
	// and basisu_transcoder.cpp), only needed by the cPackBC7FlagASTCCompatible flag path.
	const uint32_t TOTAL_ASTC_BC7_COMMON_PARTITIONS2 = 30;
	const uint32_t TOTAL_ASTC_BC7_COMMON_PARTITIONS3 = 11;

	struct astc_bc7_common_partition2_desc
	{
		uint8_t m_bc7;
		uint16_t m_astc;
		bool m_invert;
	};

	struct astc_bc7_common_partition3_desc
	{
		uint8_t m_bc7;
		uint16_t m_astc;
		uint8_t m_astc_to_bc7_perm; // converts ASTC to BC7 partition using g_astc_bc7_partition_index_perm_tables[][]
	};

	const astc_bc7_common_partition2_desc g_astc_bc7_common_partitions2[TOTAL_ASTC_BC7_COMMON_PARTITIONS2] =
	{
		{ 0, 28, false  }, { 1, 20, false }, { 2, 16, true }, { 3, 29, false },
		{ 4, 91, true }, { 5, 9, false }, { 6, 107, true }, { 7, 72, true },
		{ 8, 149, false }, { 9, 204, true }, { 10, 50, false }, { 11, 114, true },
		{ 12, 496, true }, { 13, 17, true }, { 14, 78, false }, { 15, 39, true },
		{ 17, 252, true }, { 18, 828, true }, { 19, 43, false }, { 20, 156, false },
		{ 21, 116, false }, { 22, 210, true }, { 23, 476, true }, { 24, 273, false },
		{ 25, 684, true }, { 26, 359, false }, { 29, 246, true }, { 32, 195, true },
		{ 33, 694, true }, { 52, 524, true }
	};

	const astc_bc7_common_partition3_desc g_astc_bc7_common_partitions3[TOTAL_ASTC_BC7_COMMON_PARTITIONS3] =
	{
		{ 4, 260, 0 }, { 8, 74, 5 }, { 9, 32, 5 }, { 10, 156, 2 },
		{ 11, 183, 2 }, { 12, 15, 0 }, { 13, 745, 4 }, { 20, 0, 1 },
		{ 35, 335, 1 }, { 36, 902, 5 }, { 57, 254, 0 }
	};

	const uint32_t g_bc7_weights1[2] = { 0, 64 };
	const uint32_t g_bc7_weights2[4] = { 0, 21, 43, 64 };
	const uint32_t g_bc7_weights3[8] = { 0, 9, 18, 27, 37, 46, 55, 64 };
	const uint32_t g_bc7_weights4[16] = { 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 };
	const uint32_t g_astc_weights4[16] = { 0, 4, 8, 12, 17, 21, 25, 29, 35, 39, 43, 47, 52, 56, 60, 64 };
	const uint32_t g_astc_weights5[32] = { 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 64 };
	const uint32_t g_astc_weights_3levels[3] = { 0, 32, 64 };

	const uint8_t g_bc7_partition1[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

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

	const uint8_t g_bc7_partition3[64 * 16] =
	{
		0,0,1,1,0,0,1,1,0,2,2,1,2,2,2,2,		0,0,0,1,0,0,1,1,2,2,1,1,2,2,2,1,		0,0,0,0,2,0,0,1,2,2,1,1,2,2,1,1,		0,2,2,2,0,0,2,2,0,0,1,1,0,1,1,1,		0,0,0,0,0,0,0,0,1,1,2,2,1,1,2,2,		0,0,1,1,0,0,1,1,0,0,2,2,0,0,2,2,		0,0,2,2,0,0,2,2,1,1,1,1,1,1,1,1,		0,0,1,1,0,0,1,1,2,2,1,1,2,2,1,1,
		0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,		0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,		0,0,0,0,1,1,1,1,2,2,2,2,2,2,2,2,		0,0,1,2,0,0,1,2,0,0,1,2,0,0,1,2,		0,1,1,2,0,1,1,2,0,1,1,2,0,1,1,2,		0,1,2,2,0,1,2,2,0,1,2,2,0,1,2,2,		0,0,1,1,0,1,1,2,1,1,2,2,1,2,2,2,		0,0,1,1,2,0,0,1,2,2,0,0,2,2,2,0,
		0,0,0,1,0,0,1,1,0,1,1,2,1,1,2,2,		0,1,1,1,0,0,1,1,2,0,0,1,2,2,0,0,		0,0,0,0,1,1,2,2,1,1,2,2,1,1,2,2,		0,0,2,2,0,0,2,2,0,0,2,2,1,1,1,1,		0,1,1,1,0,1,1,1,0,2,2,2,0,2,2,2,		0,0,0,1,0,0,0,1,2,2,2,1,2,2,2,1,		0,0,0,0,0,0,1,1,0,1,2,2,0,1,2,2,		0,0,0,0,1,1,0,0,2,2,1,0,2,2,1,0,
		0,1,2,2,0,1,2,2,0,0,1,1,0,0,0,0,		0,0,1,2,0,0,1,2,1,1,2,2,2,2,2,2,		0,1,1,0,1,2,2,1,1,2,2,1,0,1,1,0,		0,0,0,0,0,1,1,0,1,2,2,1,1,2,2,1,		0,0,2,2,1,1,0,2,1,1,0,2,0,0,2,2,		0,1,1,0,0,1,1,0,2,0,0,2,2,2,2,2,		0,0,1,1,0,1,2,2,0,1,2,2,0,0,1,1,		0,0,0,0,2,0,0,0,2,2,1,1,2,2,2,1,
		0,0,0,0,0,0,0,2,1,1,2,2,1,2,2,2,		0,2,2,2,0,0,2,2,0,0,1,2,0,0,1,1,		0,0,1,1,0,0,1,2,0,0,2,2,0,2,2,2,		0,1,2,0,0,1,2,0,0,1,2,0,0,1,2,0,		0,0,0,0,1,1,1,1,2,2,2,2,0,0,0,0,		0,1,2,0,1,2,0,1,2,0,1,2,0,1,2,0,		0,1,2,0,2,0,1,2,1,2,0,1,0,1,2,0,		0,0,1,1,2,2,0,0,1,1,2,2,0,0,1,1,
		0,0,1,1,1,1,2,2,2,2,0,0,0,0,1,1,		0,1,0,1,0,1,0,1,2,2,2,2,2,2,2,2,		0,0,0,0,0,0,0,0,2,1,2,1,2,1,2,1,		0,0,2,2,1,1,2,2,0,0,2,2,1,1,2,2,		0,0,2,2,0,0,1,1,0,0,2,2,0,0,1,1,		0,2,2,0,1,2,2,1,0,2,2,0,1,2,2,1,		0,1,0,1,2,2,2,2,2,2,2,2,0,1,0,1,		0,0,0,0,2,1,2,1,2,1,2,1,2,1,2,1,
		0,1,0,1,0,1,0,1,0,1,0,1,2,2,2,2,		0,2,2,2,0,1,1,1,0,2,2,2,0,1,1,1,		0,0,0,2,1,1,1,2,0,0,0,2,1,1,1,2,		0,0,0,0,2,1,1,2,2,1,1,2,2,1,1,2,		0,2,2,2,0,1,1,1,0,1,1,1,0,2,2,2,		0,0,0,2,1,1,1,2,1,1,1,2,0,0,0,2,		0,1,1,0,0,1,1,0,0,1,1,0,2,2,2,2,		0,0,0,0,0,0,0,0,2,1,1,2,2,1,1,2,
		0,1,1,0,0,1,1,0,2,2,2,2,2,2,2,2,		0,0,2,2,0,0,1,1,0,0,1,1,0,0,2,2,		0,0,2,2,1,1,2,2,1,1,2,2,0,0,2,2,		0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,2,		0,0,0,2,0,0,0,1,0,0,0,2,0,0,0,1,		0,2,2,2,1,2,2,2,0,2,2,2,1,2,2,2,		0,1,0,1,2,2,2,2,2,2,2,2,2,2,2,2,		0,1,1,1,2,0,1,1,2,2,0,1,2,2,2,0,
	};

	const uint8_t g_bc7_table_anchor_index_second_subset[64] = { 15,15,15,15,15,15,15,15,		15,15,15,15,15,15,15,15,		15, 2, 8, 2, 2, 8, 8,15,		2, 8, 2, 2, 8, 8, 2, 2,		15,15, 6, 8, 2, 8,15,15,		2, 8, 2, 2, 2,15,15, 6,		6, 2, 6, 8,15,15, 2, 2,		15,15,15,15,15, 2, 2,15 };

	const uint8_t g_bc7_table_anchor_index_third_subset_1[64] =
	{
		3, 3,15,15, 8, 3,15,15,		8, 8, 6, 6, 6, 5, 3, 3,		3, 3, 8,15, 3, 3, 6,10,		5, 8, 8, 6, 8, 5,15,15,		8,15, 3, 5, 6,10, 8,15,		15, 3,15, 5,15,15,15,15,		3,15, 5, 5, 5, 8, 5,10,		5,10, 8,13,15,12, 3, 3
	};

	const uint8_t g_bc7_table_anchor_index_third_subset_2[64] =
	{
		15, 8, 8, 3,15,15, 3, 8,		15,15,15,15,15,15,15, 8,		15, 8,15, 3,15, 8,15, 8,		3,15, 6,10,15,15,10, 8,		15, 3,15,10,10, 8, 9,10,		6,15, 8,15, 3, 6, 6, 8,		15, 3,15,15,15,15,15,15,		15,15,15,15, 3,15,15, 8
	};

	const uint8_t g_bc7_num_subsets[8] = { 3, 2, 3, 2, 1, 1, 1, 2 };
	const uint8_t g_bc7_partition_bits[8] = { 4, 6, 6, 6, 0, 0, 0, 6 };
	const uint8_t g_bc7_color_index_bitcount[8] = { 3, 3, 2, 2, 2, 2, 4, 2 };

	const uint8_t g_bc7_mode_has_p_bits[8] = { 1, 1, 0, 1, 0, 0, 1, 1 };
	const uint8_t g_bc7_mode_has_shared_p_bits[8] = { 0, 1, 0, 0, 0, 0, 0, 0 };
	const uint8_t g_bc7_color_precision_table[8] = { 4, 6, 5, 7, 5, 7, 7, 5 };
	const int8_t g_bc7_alpha_precision_table[8] = { 0, 0, 0, 0, 6, 8, 7, 5 };

	const uint8_t g_bc7_alpha_index_bitcount[8] = { 0, 0, 0, 0, 3, 2, 4, 2 }; // note mode 6 has 4 and mode 7 has 2, not really correct as they have a single plane

	endpoint_err g_bc7_mode_6_optimal_endpoints[256][2]; // [c][pbit]
	endpoint_err g_bc7_mode_5_optimal_endpoints[256]; // [c]

	// Initializes g_bc7_mode_5_optimal_endpoints/g_bc7_mode_6_optimal_endpoints.
	// Extracted from basist::uastc_init() (only the BC7 parts; the ASTC unquant
	// tables are not needed by the BC7 encoder).
	static void init_bc7_optimal_endpoints()
	{
		// BC7 777.1
		for (int c = 0; c < 256; c++)
		{
			for (uint32_t lp = 0; lp < 2; lp++)
			{
				endpoint_err best;
				best.m_error = (uint16_t)UINT16_MAX;

				for (uint32_t l = 0; l < 128; l++)
				{
					const uint32_t low = (l << 1) | lp;

					for (uint32_t h = 0; h < 128; h++)
					{
						const uint32_t high = (h << 1) | lp;

						const int k = (low * (64 - g_bc7_weights4[BC7ENC_MODE_6_OPTIMAL_INDEX]) + high * g_bc7_weights4[BC7ENC_MODE_6_OPTIMAL_INDEX] + 32) >> 6;

						const int err = (k - c) * (k - c);
						if (err < best.m_error)
						{
							best.m_error = (uint16_t)err;
							best.m_lo = (uint8_t)l;
							best.m_hi = (uint8_t)h;
						}
					} // h
				} // l

				g_bc7_mode_6_optimal_endpoints[c][lp] = best;
			} // lp

		} // c

		// BC7 777
		for (int c = 0; c < 256; c++)
		{
			endpoint_err best;
			best.m_error = (uint16_t)UINT16_MAX;

			for (uint32_t l = 0; l < 128; l++)
			{
				const uint32_t low = (l << 1) | (l >> 6);

				for (uint32_t h = 0; h < 128; h++)
				{
					const uint32_t high = (h << 1) | (h >> 6);

					const int k = (low * (64 - g_bc7_weights2[BC7ENC_MODE_5_OPTIMAL_INDEX]) + high * g_bc7_weights2[BC7ENC_MODE_5_OPTIMAL_INDEX] + 32) >> 6;

					const int err = (k - c) * (k - c);
					if (err < best.m_error)
					{
						best.m_error = (uint16_t)err;
						best.m_lo = (uint8_t)l;
						best.m_hi = (uint8_t)h;
					}
				} // h
			} // l

			g_bc7_mode_5_optimal_endpoints[c] = best;

		} // c
	}

	namespace bc7u
	{
		int determine_bc7_mode(const void* pBlock);
		int determine_bc7_mode_4_index_mode(const void* pBlock);
		int determine_bc7_mode_4_or_5_rotation(const void* pBlock);
		bool unpack_bc7_mode6(const void* pBlock_bits, color_rgba* pPixels);
		bool unpack_bc7(const void* pBlock, color_rgba* pPixels);

		extern uint8_t g_weight_quant[3][65]; // [2-4][0-64] - maps [0,64] normalized weight to nearest quantized value

		void init_weight_quant();

		// A logical BC7 block directly corresponds to the data packed into each physical block.
		// Unpacking from physical and then packing back to physical is always 100% lossless.
		struct log_bc7_block
		{
			int8_t m_mode; // 0-7, -1=invalid

			uint8_t m_num_partitions; // 1-3 subsets
			uint8_t m_pattern_bits; // 0, 4 or 6
			uint8_t m_pattern_index; // 0-15 (mode 0) or 0-63 (modes 1-3)

			uint8_t m_num_planes; // 1-2
			uint8_t m_dp_rotation_index; // 0-3 for mode 4/5, 0 for other modes

			// For dual plane (mode 4-5): normally weight plane 0 is for RGB (vector plane) and weight plane 1 is for A (scalar plane), but for mode 4 this can be inverted.
			// This is the vector plane's weight plane (BEFORE final component rotation/swapping).
			// Could also be called "m_dp_rgb_vector_weight_plane_index".
			uint8_t m_mode4_index_selector; // 0 or 1 for mode 4 (which weight plane's weights has RGB, the other plane has alpha), otherwise always 0

			uint8_t m_endpoint_bits[2]; // [rgb or a], 0 or [4,7] - note for mode 4/5 (dual plane) this is BEFORE the component rotation/swapping at the end of decoding
			uint8_t m_endpoints[3][2][4]; // [subset][l or h][component]

			// For mode 4: weight plane 0 is always 2-bits, plane 1 is 3-bits. For mode 5, both are always 2-bits. This allocation is per-mode and is not affected by the mode 4 index selector bit.
			// For mode 4: m_mode4_index_selector is the index of the vector plane. For mode 5, plane 0=vector, plane 1=scalar.
			uint8_t m_weight_bits[2]; // [plane], [2,4] - note for mode 4 this are always fixed to 2,3 (i.e. m_mode4_index_selector determines which plane contains the RGB weights)
			uint8_t m_weights[2][16]; // [plane][texel_index]

			uint8_t m_num_pbits; // 0-6, total p-bits for all subsets
			bool m_shared_pbits; // true: p-bits shared per endpoint pair, false: unique p-bit per endpoint
			uint8_t m_pbits[6];

			bool is_valid() const { return m_mode >= 0; }

			bool is_dual_plane() const { return m_num_planes == 2; }

			uint32_t get_num_weight_vals(uint32_t plane_index) const { assert(plane_index < 2); return 1u << m_weight_bits[plane_index]; }

			uint32_t get_num_endpoint_vals(uint32_t rgb_or_a_index) const { assert(rgb_or_a_index < 2); return 1u << m_endpoint_bits[rgb_or_a_index]; }

			uint32_t get_vector_weight_plane_index() const
			{
				return m_mode4_index_selector;
			}

			uint32_t get_scalar_weight_plane_index() const
			{
				return 1 - m_mode4_index_selector;
			}

			// returns the color channel separately interpolated on the scalar plane (not necessarily plane 1!)
			int get_color_component_selector() const
			{
				if (!is_dual_plane())
					return -1;
				else
					return (m_dp_rotation_index + 3) & 3;
			} 

			// For endpoint channel c (NOT decoded texel channel c): which weight plane will be used to interpolate it at the endpoint level
			uint32_t get_endpoint_channel_weight_plane(uint32_t c) const
			{
				assert(c <= 3);

				// always 0 in SP
				if (!is_dual_plane())
					return 0;

				// scalar plane index is always endpoint channel 3 (alpha): until the final mode 4/5 channel rotation/swap
				if (c == 3)
					return 1 - m_mode4_index_selector; 

				return m_mode4_index_selector; // vector plane index
			}
		
			// For each fully decoded pixel: Returns the weight plane index [0,1] that is used for decoded pixel channel c [0,3], taking into account mode 4/5 channel rotation and the mode 4 index selector bit
			uint32_t get_decoded_channel_weight_plane(uint32_t c) const
			{
				assert(c <= 3);

				// always 0 in SP
				if (!is_dual_plane())
					return 0; 

				// determine which channel uses the other scalar plane
				const uint32_t ccs_index = (m_dp_rotation_index + 3) & 3;

				if (c != ccs_index)
					return m_mode4_index_selector; // vector plane index

				return 1 - m_mode4_index_selector; // scalar plane index
			}
			
			bool has_alpha() const { return m_endpoint_bits[1] != 0; }

			// this is the # of endpoint comps
			uint32_t get_num_comps() const { return m_endpoint_bits[1] ? 4 : 3; }

			uint32_t get_num_pbits_per_subset() const { return m_num_pbits ? (m_shared_pbits ? 1 : 2) : 0; }

			void clear()
			{
				memset(this, 0, sizeof(*this));
			}
		};

		struct phys_bc7_block
		{
			uint8_t m_bytes[16];
		};

		bool unpack_bc7(const void* pPhys_block, log_bc7_block& log_blk);

		void unpack_endpoints(const log_bc7_block& log_blk, color_rgba pEndpoints[2], uint32_t subset); // returns raw endpoints (note for dual plane 1 component may be swapped with a, which is handled during decoding)

		struct endpoint_format
		{
			uint8_t m_num_rgb_bits;
			uint8_t m_num_a_bits;
			uint8_t m_num_pbits; // p-bits per-subset: 0, 1 (shared) or 2 (unique)
		};

		extern const endpoint_format g_endpoint_formats[8];

		// dequantizes a BC7 weight to a normalized [0,64] value; w should be in the range [0, 2^num_weight_bits - 1]
		int dequant_weight(uint32_t w, uint32_t num_weight_bits);
				
		// quantizes a normalized [0,64] weight to nearest quantized BC7 weight value
		// val should range from [0,64] (it's silently clamped)
		// num_weight_bits should be [2,4]
		inline uint8_t quant_weight(int val, uint32_t num_weight_bits)
		{
			assert((num_weight_bits >= 2) && (num_weight_bits <= 4));
			assert(g_weight_quant[2][64]);
			
			val = basisu::clamp<int>(val, 0, 64);

			return g_weight_quant[num_weight_bits - 2][val];
		}

		void init_log_blk(log_bc7_block& log_blk, uint32_t mode);

		void create_solid_blk(log_bc7_block& log_blk, const color_rgba& c);

		bool is_solid_blk(const log_bc7_block& log_blk);

		bool validate_log_blk(const log_bc7_block& log_blk);

		void pack_endpoints(
			uint32_t mode_index,
			const color_rgba pSrc_endpoints[2], // always 8-bits (unpacked)
			color_rgba pDst_endpoints[2], uint8_t pDst_pbits[2]); // BC7 packed with optional shared or unique p-bits, not this does not take into account any mode 4/5 channel swapping with A

		void endpoint_dpcm(
			bool decode_flag,
			const log_bc7_block& predictor_log_blk, uint32_t predictor_subset_index, // the logical block we are predicting from
			log_bc7_block& log_blk, uint32_t subset_index, // the logical block which has the endpoint we want to code/decode
			uint8_t* pResiduals, uint32_t& num_residuals, uint8_t* pBits, uint32_t& num_pbits); // the DPCM encoded values, in RR, GG, BB, AA order, pResiduals[] is always modified!

		void set_endpoints(log_bc7_block& log_blk, uint32_t subset, const color_rgba pEndpoints[2], const uint8_t pbits[2]); // must be packed

		bool unpack_bc7(const log_bc7_block& log_blk, color_rgba* pPixels);

		bool unpack_bc7_texel(const log_bc7_block& log_blk, color_rgba& pixel, uint32_t x, uint32_t y);

		uint32_t get_texel_subset(log_bc7_block& log_blk, uint32_t x, uint32_t y);

		void canonicalize_endpoints(log_bc7_block& log_blk);
		
		// note the packing code may swap endpoints if needed to pack the weights, which is lossless for decoding to pixels
		bool pack_bc7(const log_bc7_block& log_blk, void* pPhys_block);

		bool compare_block_configs(const log_bc7_block& a, const log_bc7_block& b, bool compare_partition_index = true); // true if a's mode/config matches b's (everything EXCEPT endpoint/pbits/weight)
		
		bool compare_block_endpoints(const log_bc7_block& a, const log_bc7_block& b); 

		bool compare_block_pbits(const log_bc7_block& a, const log_bc7_block& b);

		bool compare_block_weights(const log_bc7_block& a, const log_bc7_block& b);

		bool compare_block_full(const log_bc7_block& a, const log_bc7_block& b);

	} // namespace bc7u
namespace bc7u
{
	//------------------------------------------------------------------------------------------------
	// BC7 mode 0-7 decompression.
	// Instead of one monster routine to unpack all the BC7 modes, we're lumping the 3 subset, 2 subset, 1 subset, and dual plane modes together into simple shared routines.

	static inline uint32_t bc7_dequant(uint32_t val, uint32_t pbit, uint32_t val_bits) { assert(val < (1U << val_bits)); assert(pbit < 2); assert(val_bits >= 4 && val_bits <= 8); const uint32_t total_bits = val_bits + 1; val = (val << 1) | pbit; val <<= (8 - total_bits); val |= (val >> total_bits); assert(val <= 255); return val; }
	static inline uint32_t bc7_dequant(uint32_t val, uint32_t val_bits) { assert(val < (1U << val_bits)); assert(val_bits >= 4 && val_bits <= 8); val <<= (8 - val_bits); val |= (val >> val_bits); assert(val <= 255); return val; }

	static inline uint32_t bc7_interp2(uint32_t l, uint32_t h, uint32_t w) { assert(w < 4); return (l * (64 - basist::g_bc7_weights2[w]) + h * basist::g_bc7_weights2[w] + 32) >> 6; }
	static inline uint32_t bc7_interp3(uint32_t l, uint32_t h, uint32_t w) { assert(w < 8); return (l * (64 - basist::g_bc7_weights3[w]) + h * basist::g_bc7_weights3[w] + 32) >> 6; }
	static inline uint32_t bc7_interp4(uint32_t l, uint32_t h, uint32_t w) { assert(w < 16); return (l * (64 - basist::g_bc7_weights4[w]) + h * basist::g_bc7_weights4[w] + 32) >> 6; }
	static inline uint32_t bc7_interp(uint32_t l, uint32_t h, uint32_t w, uint32_t bits)
	{
		assert(l <= 255 && h <= 255);
		switch (bits)
		{
		case 2: return bc7_interp2(l, h, w);
		case 3: return bc7_interp3(l, h, w);
		case 4: return bc7_interp4(l, h, w);
		default:
			break;
		}
		return 0;
	}

	inline uint32_t read_bits32(const uint8_t* pBuf, uint32_t& bit_offset, uint32_t codesize)
	{
		assert(codesize <= 32);
		uint32_t bits = 0;
		uint32_t total_bits = 0;

		while (total_bits < codesize)
		{
			uint32_t byte_bit_offset = bit_offset & 7;
			uint32_t bits_to_read = basisu::minimum<int>(codesize - total_bits, 8 - byte_bit_offset);

			uint32_t byte_bits = pBuf[bit_offset >> 3] >> byte_bit_offset;
			byte_bits &= ((1 << bits_to_read) - 1);

			bits |= (byte_bits << total_bits);

			total_bits += bits_to_read;
			bit_offset += bits_to_read;
		}

		return bits;
	}

	bool unpack_bc7_mode0_2(uint32_t mode, const void* pBlock_bits, color_rgba* pPixels)
	{
		//const uint32_t SUBSETS = 3;
		const uint32_t ENDPOINTS = 6;
		const uint32_t COMPS = 3;
		const uint32_t WEIGHT_BITS = (mode == 0) ? 3 : 2;
		const uint32_t ENDPOINT_BITS = (mode == 0) ? 4 : 5;
		const uint32_t PBITS = (mode == 0) ? 6 : 0;
		const uint32_t WEIGHT_VALS = 1 << WEIGHT_BITS;

		uint32_t bit_offset = 0;
		const uint8_t* pBuf = static_cast<const uint8_t*>(pBlock_bits);

		if (read_bits32(pBuf, bit_offset, mode + 1) != (1U << mode)) return false;

		const uint32_t part = read_bits32(pBuf, bit_offset, (mode == 0) ? 4 : 6);

		color_rgba endpoints[ENDPOINTS];
		for (uint32_t c = 0; c < COMPS; c++)
			for (uint32_t e = 0; e < ENDPOINTS; e++)
				endpoints[e][c] = (uint8_t)read_bits32(pBuf, bit_offset, ENDPOINT_BITS);

		uint32_t pbits[6];
		for (uint32_t p = 0; p < PBITS; p++)
			pbits[p] = read_bits32(pBuf, bit_offset, 1);

		uint32_t weights[16];
		for (uint32_t i = 0; i < 16; i++)
			weights[i] = read_bits32(pBuf, bit_offset, ((!i) || (i == basist::g_bc7_table_anchor_index_third_subset_1[part]) || (i == basist::g_bc7_table_anchor_index_third_subset_2[part])) ? (WEIGHT_BITS - 1) : WEIGHT_BITS);

		assert(bit_offset == 128);

		for (uint32_t e = 0; e < ENDPOINTS; e++)
			for (uint32_t c = 0; c < 4; c++)
				endpoints[e][c] = (uint8_t)((c == 3) ? 255 : (PBITS ? bc7_dequant(endpoints[e][c], pbits[e], ENDPOINT_BITS) : bc7_dequant(endpoints[e][c], ENDPOINT_BITS)));

		color_rgba block_colors[3][8];
		for (uint32_t s = 0; s < 3; s++)
			for (uint32_t i = 0; i < WEIGHT_VALS; i++)
			{
				for (uint32_t c = 0; c < 3; c++)
					block_colors[s][i][c] = (uint8_t)bc7_interp(endpoints[s * 2 + 0][c], endpoints[s * 2 + 1][c], i, WEIGHT_BITS);
				block_colors[s][i][3] = 255;
			}

		for (uint32_t i = 0; i < 16; i++)
			pPixels[i] = block_colors[basist::g_bc7_partition3[part * 16 + i]][weights[i]];

		return true;
	}

	bool unpack_bc7_mode1_3_7(uint32_t mode, const void* pBlock_bits, color_rgba* pPixels)
	{
		//const uint32_t SUBSETS = 2;
		const uint32_t ENDPOINTS = 4;
		const uint32_t COMPS = (mode == 7) ? 4 : 3;
		const uint32_t WEIGHT_BITS = (mode == 1) ? 3 : 2;
		const uint32_t ENDPOINT_BITS = (mode == 7) ? 5 : ((mode == 1) ? 6 : 7);
		const uint32_t PBITS = (mode == 1) ? 2 : 4;
		const uint32_t SHARED_PBITS = (mode == 1) ? true : false;
		const uint32_t WEIGHT_VALS = 1 << WEIGHT_BITS;

		uint32_t bit_offset = 0;
		const uint8_t* pBuf = static_cast<const uint8_t*>(pBlock_bits);

		if (read_bits32(pBuf, bit_offset, mode + 1) != (1U << mode)) return false;

		const uint32_t part = read_bits32(pBuf, bit_offset, 6);

		color_rgba endpoints[ENDPOINTS];
		for (uint32_t c = 0; c < COMPS; c++)
			for (uint32_t e = 0; e < ENDPOINTS; e++)
				endpoints[e][c] = (uint8_t)read_bits32(pBuf, bit_offset, ENDPOINT_BITS);

		uint32_t pbits[4];
		for (uint32_t p = 0; p < PBITS; p++)
			pbits[p] = read_bits32(pBuf, bit_offset, 1);

		uint32_t weights[16];
		for (uint32_t i = 0; i < 16; i++)
			weights[i] = read_bits32(pBuf, bit_offset, ((!i) || (i == basist::g_bc7_table_anchor_index_second_subset[part])) ? (WEIGHT_BITS - 1) : WEIGHT_BITS);

		assert(bit_offset == 128);

		for (uint32_t e = 0; e < ENDPOINTS; e++)
			for (uint32_t c = 0; c < 4; c++)
				endpoints[e][c] = (uint8_t)((c == ((mode == 7U) ? 4U : 3U)) ? 255 : bc7_dequant(endpoints[e][c], pbits[SHARED_PBITS ? (e >> 1) : e], ENDPOINT_BITS));

		color_rgba block_colors[2][8];
		for (uint32_t s = 0; s < 2; s++)
			for (uint32_t i = 0; i < WEIGHT_VALS; i++)
			{
				for (uint32_t c = 0; c < COMPS; c++)
					block_colors[s][i][c] = (uint8_t)bc7_interp(endpoints[s * 2 + 0][c], endpoints[s * 2 + 1][c], i, WEIGHT_BITS);
				block_colors[s][i][3] = (COMPS == 3) ? 255 : block_colors[s][i][3];
			}

		for (uint32_t i = 0; i < 16; i++)
			pPixels[i] = block_colors[basist::g_bc7_partition2[part * 16 + i]][weights[i]];

		return true;
	}
		
	bool unpack_bc7_mode4_5(uint32_t mode, const void* pBlock_bits, color_rgba* pPixels)
	{
		const uint32_t ENDPOINTS = 2;
		const uint32_t COMPS = 4;
		const uint32_t WEIGHT_BITS = 2;
		const uint32_t A_WEIGHT_BITS = (mode == 4) ? 3 : 2;
		const uint32_t ENDPOINT_BITS = (mode == 4) ? 5 : 7;
		const uint32_t A_ENDPOINT_BITS = (mode == 4) ? 6 : 8;
		//const uint32_t WEIGHT_VALS = 1 << WEIGHT_BITS;
		//const uint32_t A_WEIGHT_VALS = 1 << A_WEIGHT_BITS;

		uint32_t bit_offset = 0;
		const uint8_t* pBuf = static_cast<const uint8_t*>(pBlock_bits);

		if (read_bits32(pBuf, bit_offset, mode + 1) != (1U << mode)) return false;

		const uint32_t comp_rot = read_bits32(pBuf, bit_offset, 2);
		const uint32_t index_mode = (mode == 4) ? read_bits32(pBuf, bit_offset, 1) : 0;

		color_rgba endpoints[ENDPOINTS];
		for (uint32_t c = 0; c < COMPS; c++)
			for (uint32_t e = 0; e < ENDPOINTS; e++)
				endpoints[e][c] = (uint8_t)read_bits32(pBuf, bit_offset, (c == 3) ? A_ENDPOINT_BITS : ENDPOINT_BITS);

		const uint32_t weight_bits[2] = { index_mode ? A_WEIGHT_BITS : WEIGHT_BITS,  index_mode ? WEIGHT_BITS : A_WEIGHT_BITS };

		uint32_t weights[16], a_weights[16];

		for (uint32_t i = 0; i < 16; i++)
			(index_mode ? a_weights : weights)[i] = read_bits32(pBuf, bit_offset, weight_bits[index_mode] - ((!i) ? 1 : 0));

		for (uint32_t i = 0; i < 16; i++)
			(index_mode ? weights : a_weights)[i] = read_bits32(pBuf, bit_offset, weight_bits[1 - index_mode] - ((!i) ? 1 : 0));

		assert(bit_offset == 128);

		for (uint32_t e = 0; e < ENDPOINTS; e++)
			for (uint32_t c = 0; c < 4; c++)
				endpoints[e][c] = (uint8_t)bc7_dequant(endpoints[e][c], (c == 3) ? A_ENDPOINT_BITS : ENDPOINT_BITS);

		color_rgba block_colors[8];
		for (uint32_t i = 0; i < (1U << weight_bits[0]); i++)
			for (uint32_t c = 0; c < 3; c++)
				block_colors[i][c] = (uint8_t)bc7_interp(endpoints[0][c], endpoints[1][c], i, weight_bits[0]);

		for (uint32_t i = 0; i < (1U << weight_bits[1]); i++)
			block_colors[i][3] = (uint8_t)bc7_interp(endpoints[0][3], endpoints[1][3], i, weight_bits[1]);

		for (uint32_t i = 0; i < 16; i++)
		{
			pPixels[i] = block_colors[weights[i]];
			pPixels[i].a = block_colors[a_weights[i]].a;
			if (comp_rot >= 1)
				std::swap(pPixels[i].a, pPixels[i].m_comps[comp_rot - 1]);
		}

		return true;
	}

	struct bc7_mode_6
	{
		struct
		{
			uint64_t m_mode : 7;
			uint64_t m_r0 : 7;
			uint64_t m_r1 : 7;
			uint64_t m_g0 : 7;
			uint64_t m_g1 : 7;
			uint64_t m_b0 : 7;
			uint64_t m_b1 : 7;
			uint64_t m_a0 : 7;
			uint64_t m_a1 : 7;
			uint64_t m_p0 : 1;
		} m_lo;

		union
		{
			struct
			{
				uint64_t m_p1 : 1;
				uint64_t m_s00 : 3;
				uint64_t m_s10 : 4;
				uint64_t m_s20 : 4;
				uint64_t m_s30 : 4;

				uint64_t m_s01 : 4;
				uint64_t m_s11 : 4;
				uint64_t m_s21 : 4;
				uint64_t m_s31 : 4;

				uint64_t m_s02 : 4;
				uint64_t m_s12 : 4;
				uint64_t m_s22 : 4;
				uint64_t m_s32 : 4;

				uint64_t m_s03 : 4;
				uint64_t m_s13 : 4;
				uint64_t m_s23 : 4;
				uint64_t m_s33 : 4;

			} m_hi;

			uint64_t m_hi_bits;
		};
	};

	bool unpack_bc7_mode6(const void* pBlock_bits, color_rgba* pPixels)
	{
		static_assert(sizeof(bc7_mode_6) == 16, "sizeof(bc7_mode_6) == 16");

		const bc7_mode_6& block = *static_cast<const bc7_mode_6*>(pBlock_bits);

		if (block.m_lo.m_mode != (1 << 6))
			return false;

		const uint32_t r0 = (uint32_t)((block.m_lo.m_r0 << 1) | block.m_lo.m_p0);
		const uint32_t g0 = (uint32_t)((block.m_lo.m_g0 << 1) | block.m_lo.m_p0);
		const uint32_t b0 = (uint32_t)((block.m_lo.m_b0 << 1) | block.m_lo.m_p0);
		const uint32_t a0 = (uint32_t)((block.m_lo.m_a0 << 1) | block.m_lo.m_p0);
		const uint32_t r1 = (uint32_t)((block.m_lo.m_r1 << 1) | block.m_hi.m_p1);
		const uint32_t g1 = (uint32_t)((block.m_lo.m_g1 << 1) | block.m_hi.m_p1);
		const uint32_t b1 = (uint32_t)((block.m_lo.m_b1 << 1) | block.m_hi.m_p1);
		const uint32_t a1 = (uint32_t)((block.m_lo.m_a1 << 1) | block.m_hi.m_p1);

		color_rgba vals[16];
		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t w = basist::g_bc7_weights4[i];
			const uint32_t iw = 64 - w;
			vals[i].set_noclamp_rgba(
				(r0 * iw + r1 * w + 32) >> 6,
				(g0 * iw + g1 * w + 32) >> 6,
				(b0 * iw + b1 * w + 32) >> 6,
				(a0 * iw + a1 * w + 32) >> 6);
		}

		pPixels[0] = vals[block.m_hi.m_s00];
		pPixels[1] = vals[block.m_hi.m_s10];
		pPixels[2] = vals[block.m_hi.m_s20];
		pPixels[3] = vals[block.m_hi.m_s30];

		pPixels[4] = vals[block.m_hi.m_s01];
		pPixels[5] = vals[block.m_hi.m_s11];
		pPixels[6] = vals[block.m_hi.m_s21];
		pPixels[7] = vals[block.m_hi.m_s31];

		pPixels[8] = vals[block.m_hi.m_s02];
		pPixels[9] = vals[block.m_hi.m_s12];
		pPixels[10] = vals[block.m_hi.m_s22];
		pPixels[11] = vals[block.m_hi.m_s32];

		pPixels[12] = vals[block.m_hi.m_s03];
		pPixels[13] = vals[block.m_hi.m_s13];
		pPixels[14] = vals[block.m_hi.m_s23];
		pPixels[15] = vals[block.m_hi.m_s33];

		return true;
	}

	int determine_bc7_mode(const void* pBlock)
	{
		const uint32_t b = static_cast<const uint8_t*>(pBlock)[0];

		if (!b) 
			return -1;

#if defined(_MSC_VER)
		unsigned long idx;
		_BitScanForward(&idx, b);
		return (int)idx;
#else
		return __builtin_ctz(b);
#endif
	}

	int determine_bc7_mode_4_index_mode(const void* pBlock)
	{
		const uint32_t first_byte = static_cast<const uint8_t*>(pBlock)[0];

		// check for mode 4
		if ((first_byte & 31) != 0b10000)
			return -1;

		return (first_byte >> 7);
	}

	int determine_bc7_mode_4_or_5_rotation(const void* pBlock)
	{
		const uint32_t first_byte = static_cast<const uint8_t*>(pBlock)[0];
		if ((first_byte & 31) == 0b10000)
		{
			// mode 4
			return (first_byte >> 5) & 3;
		}

		if ((first_byte & 63) == 0b100000)
		{
			// mode 5
			return first_byte >> 6;
		}

		return -1;
	}

	bool unpack_bc7(const void* pBlock, color_rgba* pPixels)
	{
		const uint32_t first_byte = static_cast<const uint8_t*>(pBlock)[0];

		for (uint32_t mode = 0; mode <= 7; mode++)
		{
			if (first_byte & (1U << mode))
			{
				switch (mode)
				{
				case 0:
				case 2:
					return unpack_bc7_mode0_2(mode, pBlock, pPixels);
				case 1:
				case 3:
				case 7:
					return unpack_bc7_mode1_3_7(mode, pBlock, pPixels);
				case 4:
				case 5:
					return unpack_bc7_mode4_5(mode, pBlock, pPixels);
				case 6:
					return unpack_bc7_mode6(pBlock, pPixels);
				default:
					break;
				}
			}
		}

		return false;
	}
			
} // namespace bc7u

// BASISU_BC7F_USE_SSE41 - only very minimally tested. Needs more testing and more variants. Only improves transcoding perf by ~10% in native so far.
#define BASISU_BC7F_USE_SSE41 (0)
#define BASISU_BC7F_PERF_STATS (0)

namespace bc7f
{
	const uint32_t MAX_PATTERNS2_TO_CHECK = 64;
	const uint32_t MAX_PATTERNS3_TO_CHECK = 64;

	const float UNIQUE_PBIT_DISCOUNT = .85f;
	const float SHARED_PBIT_DISCOUNT = .95f;

	//static inline uint8_t mul_8(uint32_t v, uint32_t q) { v = v * q + 128; return (uint8_t)((v + (v >> 8)) >> 8); }
	//static inline int mul_8bit(int a, int b) { int t = a * b + 128; return (t + (t >> 8)) >> 8; }
	//static inline int lerp_8bit(int a, int b, int s) { assert(a >= 0 && a <= 255); assert(b >= 0 && b <= 255); assert(s >= 0 && s <= 255); return a + mul_8bit(b - a, s); }

	static int popcount32(uint32_t x)
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

#if BASISU_BC7F_PERF_STATS
	// not thread safe (no need/for dev)
	uint32_t g_total_rgb_calls;
	uint32_t g_total_rgba_calls;
	uint32_t g_total_solid_blocks;

	uint32_t g_total_trivial_mode6_blocks;

	uint32_t g_total_dp_valid_chans_rgb;
	uint32_t g_total_dp_valid_chans_a;
	uint32_t g_total_high_ortho_energy;

	uint32_t g_total_mode02_evals;
	uint32_t g_total_mode02_bailouts;

	uint32_t g_total_mode13_evals;
	uint32_t g_total_mode13_bailouts;

	uint32_t g_total_mode45_evals;
	uint32_t g_total_mode45_bailouts;

	uint32_t g_total_mode7_evals;
	uint32_t g_total_mode7_bailouts;
#endif

	inline int fast_roundf_pos_int(float x)
	{
		assert(x >= 0.0f);
		return (int)(x + 0.5f);
	}

	inline int fast_roundf_int(float x)
	{
		return (x >= 0.0f) ? (int)(x + 0.5f) : (int)(x - 0.5f);
	}

	inline int fast_floorf_int(float x)
	{
		int xi = (int)x;  // Truncate towards zero
		return ((x < 0.0f) && (x != (float)xi)) ? (xi - 1) : xi;
	}

	static inline uint32_t from_7(uint32_t v)
	{
		assert(v < 128);
		return (v << 1) | (v >> 6);
	}

	static inline uint32_t from_7(uint32_t v, uint32_t p)
	{
		assert((v < 128) && (p <= 1));
		return (v << 1) | p;
	}

	static inline int to_7(int c8, int pbit)
	{
		assert((c8 >= 0) && (c8 <= 255) && (pbit >= 0) && (pbit <= 1));
		uint32_t e = (uint32_t(c8) + uint32_t(pbit ^ 1)) >> 1;
		return basisu::minimum<uint32_t>(127, e);
	}

	static inline int to_7(int c8)
	{
		assert((c8 >= 0) && (c8 <= 255));
		return (c8 * 127 + 127) / 255;
	}

	static inline int to_7(float c, int pbit)
	{
		assert((c >= 0) && (c <= 255.0f));
		return to_7(fast_roundf_pos_int(c), pbit);
	}

	static inline int to_7_clamp(float c, int pbit)
	{
		return to_7(basisu::clamp<int>(fast_roundf_int(c), 0, 255), pbit);
	}

	static inline int to_5(int c8)
	{
		assert((c8 >= 0) && (c8 <= 255));
		return (c8 * 31 + 127) / 255;
	}

	static inline int to_5_clamp(float c)
	{
		return basisu::clamp<int>(fast_roundf_int(c * (31.0f / 255.0f)), 0, 31);
	}

	static inline int to_6(int c8)
	{
		assert((c8 >= 0) && (c8 <= 255));
		return (c8 * 63 + 127) / 255;
	}

	static inline int to_6(int c8, int pbit)
	{
		assert((c8 >= 0) && (c8 <= 255));
		assert((pbit == 0) || (pbit == 1));

		int q7 = (c8 * 127 + 127) / 255;

		if ((q7 & 1) != pbit)
		{
			const int lhs = c8 * 127;
			const int rhs = 255 * q7;

			if (lhs >= rhs)
			{
				q7 = (q7 < 127) ? (q7 + 1) : (q7 - 1);
			}
			else
			{
				q7 = (q7 > 0) ? (q7 - 1) : (q7 + 1);
			}
		}

		return q7 >> 1;
	}

	static inline int to_6_clamp(float c, int pbit)
	{
		return to_6(basisu::clamp<int>(fast_roundf_int(c), 0, 255), pbit);
	}

	static inline uint32_t from_6(uint32_t v, uint32_t p)
	{
		assert((v < 64) && (p <= 1));
		v = (v << 1) | p;
		v = (v << 1) | (v >> 6);
		return v;
	}

	static inline uint32_t from_4(uint32_t v, uint32_t p)
	{
		assert((v < 16) && (p <= 1));
		v = (v << 1) | p;
		v = (v << 3) | (v >> 2);
		return v;
	}

	static inline uint32_t from_5(uint32_t v)
	{
		assert(v < 32);
		v = (v << 3) | (v >> 2);
		return v;
	}

	static inline uint32_t from_5(uint32_t v, uint32_t p)
	{
		assert((v < 32) && (p <= 1));
		v = (v << 1) | p;
		v = (v << 2) | (v >> 4);
		return v;
	}

	static inline uint32_t from_6(uint32_t v)
	{
		assert(v < 64);
		v = (v << 2) | (v >> 4);
		return v;
	}

	static inline int to_5(int c8, int pbit)
	{
		assert((c8 >= 0) && (c8 <= 255));
		assert((pbit == 0) || (pbit == 1));

		int q6 = (c8 * 63 + 127) / 255;

		if ((q6 & 1) != pbit)
		{
			const int lhs = c8 * 63;
			const int rhs = 255 * q6;

			if (lhs >= rhs)
			{
				q6 = (q6 < 63) ? (q6 + 1) : (q6 - 1);
			}
			else
			{
				q6 = (q6 > 0) ? (q6 - 1) : (q6 + 1);
			}
		}

		return q6 >> 1;
	}

#if 0
	static inline int to_5(float c, int pbit)
	{
		assert((c >= 0.0f) && (c <= 255.0f));
		return to_5((int)fast_roundf_pos_int(c), pbit);
	}
#endif

	static inline int to_5_clamp(float c, uint32_t pbit)
	{
		return to_5(basisu::clamp<int>(fast_roundf_int(c), 0, 255), pbit);
	}

	//static inline uint32_t bc7_interp(uint32_t l, uint32_t h, uint32_t w) { assert(w <= 64); return (l * (64 - w) + h * w + 32) >> 6; }
	//static inline uint32_t bc7_interp2(uint32_t l, uint32_t h, uint32_t w) { assert(w <= 64); int d = h - l; return (int)l + ((d * (int)w + 32) >> 6); }
	//static inline uint32_t bc7_interp3(int l, int d, uint32_t w) { assert(w <= 64); return l + ((d * (int)w + 32) >> 6); }

	static vec4F g_bc7_2bit_ls_tab[4];
	static vec4F g_bc7_3bit_ls_tab[8];
	static vec4F g_bc7_4bit_ls_tab[16];
	static uint16_t g_bc7_part2_bitmasks[64];
	static uint32_t g_part3_bitmasks[64];

	void init()
	{
		// [vendored] upstream initializes these tables in basisu_transcoder_init()/uastc_init();
		// folded into bc7f::init() here so this file is the only one-time init entry point.
		bc7u::init_weight_quant();
		init_bc7_optimal_endpoints();

		for (uint32_t i = 0; i < 4; i++)
		{
			float w = (float)basist::g_bc7_weights2[i] * (1.0f / 64.0f);
			g_bc7_2bit_ls_tab[i].set(w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w);
		}

		for (uint32_t i = 0; i < 8; i++)
		{
			float w = (float)basist::g_bc7_weights3[i] * (1.0f / 64.0f);
			g_bc7_3bit_ls_tab[i].set(w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w);
		}

		for (uint32_t i = 0; i < 16; i++)
		{
			float w = (float)basist::g_bc7_weights4[i] * (1.0f / 64.0f);
			g_bc7_4bit_ls_tab[i].set(w * w, (1.0f - w) * w, (1.0f - w) * (1.0f - w), w);
		}

		for (uint32_t i = 0; i < 64; i++)
		{
			uint16_t y = 0;

			for (uint32_t x = 0; x < 16; x++)
				y |= (g_bc7_partition2[i * 16 + x] << x);

			g_bc7_part2_bitmasks[i] = y;
		}

		for (uint32_t i = 0; i < 64; i++)
		{
			const uint8_t* pPat = &g_bc7_partition3[i * 16];

			for (uint32_t j = 0; j < 16; j++)
			{
				const uint32_t s = pPat[j];

				if (s == 0)
					g_part3_bitmasks[i] |= (1 << j);
				else if (s == 1)
					g_part3_bitmasks[i] |= (0x10000 << j);
			}
		}
	}

	void encode_mode0_rgb_block(uint8_t* pBlock, uint32_t part_id, // 3 subsets, 4-bits part ID
		uint32_t lr[3], uint32_t lg[3], uint32_t lb[3], // 4 bit endpoints
		uint32_t hr[3], uint32_t hg[3], uint32_t hb[3],
		uint32_t p[6],
		const uint8_t* pWeights) // 3-bit weights
	{
		assert(part_id < 16);
		assert((lr[0] | lr[1] | lr[2] | lg[0] | lg[1] | lg[2] | lb[0] | lb[1] | lb[2]) <= 15);
		assert((hr[0] | hr[1] | hr[2] | hg[0] | hg[1] | hg[2] | hb[0] | hb[1] | hb[2]) <= 15);
		assert((p[0] | p[1] | p[2] | p[3] | p[4] | p[5]) <= 1);

		const uint8_t* pPart_map = &g_bc7_partition3[part_id * 16];
		const uint32_t anchor_index0 = g_bc7_table_anchor_index_third_subset_1[part_id];
		const uint32_t anchor_index1 = g_bc7_table_anchor_index_third_subset_2[part_id];

		uint32_t weight_inv[3] = { 0, 0, 0 };

		if (pWeights[0] & 4)
		{
			std::swap(lr[0], hr[0]);
			std::swap(lg[0], hg[0]);
			std::swap(lb[0], hb[0]);
			std::swap(p[0], p[1]);
			weight_inv[0] = 7;
		}

		if (pWeights[anchor_index0] & 4)
		{
			std::swap(lr[1], hr[1]);
			std::swap(lg[1], hg[1]);
			std::swap(lb[1], hb[1]);
			std::swap(p[2], p[3]);
			weight_inv[1] = 7;
		}

		if (pWeights[anchor_index1] & 4)
		{
			std::swap(lr[2], hr[2]);
			std::swap(lg[2], hg[2]);
			std::swap(lb[2], hb[2]);
			std::swap(p[4], p[5]);
			weight_inv[2] = 7;
		}

		uint64_t low = 1ULL | ((part_id) << 1) |
			((lr[0]) << 5) | ((hr[0]) << 9) |
			((lr[1]) << 13) | ((hr[1]) << 17) |
			((lr[2]) << 21) | ((hr[2]) << 25) |
			(uint64_t(lg[0]) << 29) | (uint64_t(hg[0]) << 33) |
			(uint64_t(lg[1]) << 37) | (uint64_t(hg[1]) << 41) |
			(uint64_t(lg[2]) << 45) | (uint64_t(hg[2]) << 49) |
			(uint64_t(lb[0]) << 53) | (uint64_t(hb[0]) << 57) |
			(uint64_t(lb[1]) << 61);

		pBlock[0] = (uint8_t)low;
		pBlock[1] = (uint8_t)(low >> 8);
		pBlock[2] = (uint8_t)(low >> 16);
		pBlock[3] = (uint8_t)(low >> 24);
		pBlock[4] = (uint8_t)(low >> 32);
		pBlock[5] = (uint8_t)(low >> 40);
		pBlock[6] = (uint8_t)(low >> 48);
		pBlock[7] = (uint8_t)(low >> 56);

		uint64_t high = (lb[1] >> 3) | ((hb[1]) << 1) | ((lb[2]) << 5) | ((hb[2]) << 9) |
			((p[0]) << 13) | ((p[1]) << 14) | ((p[2]) << 15) | ((p[3]) << 16) | ((p[4]) << 17) | ((p[5]) << 18);

		uint32_t ofs = 19;
		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];
			uint64_t w = pWeights[i] ^ weight_inv[subset_index];

#ifdef _DEBUG
			assert(w <= 7);
			if ((i == 0) || (i == anchor_index0) || (i == anchor_index1))
			{
				assert((w & 4) == 0);
			}
#endif

			high |= (w << ofs);
			ofs += (3 - ((i == 0) || (i == anchor_index0) || (i == anchor_index1)));
		}
		assert(64 == ofs);

		pBlock[8] = (uint8_t)high;
		pBlock[9] = (uint8_t)(high >> 8);
		pBlock[10] = (uint8_t)(high >> 16);
		pBlock[11] = (uint8_t)(high >> 24);
		pBlock[12] = (uint8_t)(high >> 32);
		pBlock[13] = (uint8_t)(high >> 40);
		pBlock[14] = (uint8_t)(high >> 48);
		pBlock[15] = (uint8_t)(high >> 56);
	}

	void encode_mode1_rgb_block(uint8_t* pBlock, uint32_t part_id, // 2 subsets, 6-bits part ID
		uint32_t lr[2], uint32_t lg[2], uint32_t lb[2], // 6-bit endpoints, 2 shared pbits
		uint32_t hr[2], uint32_t hg[2], uint32_t hb[2],
		uint32_t p0, uint32_t p1,
		const uint8_t* pWeights) // 3-bit weights
	{
		assert(part_id < 64);
		assert((lr[0] | lr[1] | lg[0] | lg[1] | lb[0] | lb[1]) <= 63);
		assert((hr[0] | hr[1] | hg[0] | hg[1] | hb[0] | hb[1]) <= 63);
		assert((p0 | p1) <= 1);

		const uint8_t* pPart_map = &g_bc7_partition2[part_id * 16];
		const uint32_t anchor_index = g_bc7_table_anchor_index_second_subset[part_id];

		uint32_t weight_inv[2] = { 0, 0 };
		if (pWeights[0] & 4)
		{
			std::swap(lr[0], hr[0]);
			std::swap(lg[0], hg[0]);
			std::swap(lb[0], hb[0]);
			weight_inv[0] = 7;
		}

		if (pWeights[anchor_index] & 4)
		{
			std::swap(lr[1], hr[1]);
			std::swap(lg[1], hg[1]);
			std::swap(lb[1], hb[1]);
			weight_inv[1] = 7;
		}

		pBlock[0] = (uint8_t)(0b10 | (part_id << 2));

		uint64_t x = lr[0] | (hr[0] << (6 * 1));
		x |= (lr[1] << (6 * 2)) | (hr[1] << (6 * 3));

		x |= (lg[0] << (6 * 4)) | (uint64_t(hg[0]) << (6 * 5));
		x |= (uint64_t(lg[1]) << (6 * 6)) | (uint64_t(hg[1]) << (6 * 7));

		x |= (uint64_t(lb[0]) << (6 * 8)) | (uint64_t(hb[0]) << (6 * 9));
		x |= (uint64_t(lb[1]) << (6 * 10));

		// 11*6=66 bits total, write first 64

		pBlock[1] = (uint8_t)x;
		pBlock[2] = (uint8_t)(x >> 8);
		pBlock[3] = (uint8_t)(x >> 16);
		pBlock[4] = (uint8_t)(x >> 24);

		pBlock[5] = (uint8_t)(x >> 32);
		pBlock[6] = (uint8_t)(x >> 40);
		pBlock[7] = (uint8_t)(x >> 48);
		pBlock[8] = (uint8_t)(x >> 56);

		pBlock[9] = (uint8_t)((lb[1] >> 4) | (hb[1] << 2));

		uint64_t y = p0 | (p1 << 1);
		uint32_t ofs = 2;

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];

			uint64_t w = pWeights[i] ^ weight_inv[subset_index];

#ifdef _DEBUG
			assert(w <= 7);
			if ((i == 0) || (i == anchor_index))
			{
				assert((w & 4) == 0);
			}
#endif
			y |= (w << ofs);

			ofs += (3 - ((i == 0) || (i == anchor_index)));
		}
		assert(48 == ofs);

		pBlock[10] = (uint8_t)y;
		pBlock[11] = (uint8_t)(y >> 8);
		pBlock[12] = (uint8_t)(y >> 16);
		pBlock[13] = (uint8_t)(y >> 24);
		pBlock[14] = (uint8_t)(y >> 32);
		pBlock[15] = (uint8_t)(y >> 40);
	}

	void encode_mode2_rgb_block(uint8_t* pBlock, uint32_t part_id, // 3 subsets, 6-bits part ID
		uint32_t lr[3], uint32_t lg[3], uint32_t lb[3], // 5 bit endpoints, no pbits
		uint32_t hr[3], uint32_t hg[3], uint32_t hb[3],
		const uint8_t* pWeights) // 2-bit weights
	{
		assert(part_id < 64);
		assert((lr[0] | lr[1] | lr[2] | lg[0] | lg[1] | lg[2] | lb[0] | lb[1] | lb[2]) <= 31);
		assert((hr[0] | hr[1] | hr[2] | hg[0] | hg[1] | hg[2] | hb[0] | hb[1] | hb[2]) <= 31);

		const uint8_t* pPart_map = &g_bc7_partition3[part_id * 16];

		uint32_t weight_inv[3] = { 0 };
		if (pWeights[0] & 2)
		{
			std::swap(lr[0], hr[0]);
			std::swap(lg[0], hg[0]);
			std::swap(lb[0], hb[0]);
			weight_inv[0] = 3;
		}

		const uint32_t anchor_index0 = g_bc7_table_anchor_index_third_subset_1[part_id];
		if (pWeights[anchor_index0] & 2)
		{
			std::swap(lr[1], hr[1]);
			std::swap(lg[1], hg[1]);
			std::swap(lb[1], hb[1]);
			weight_inv[1] = 3;
		}

		const uint32_t anchor_index1 = g_bc7_table_anchor_index_third_subset_2[part_id];
		if (pWeights[anchor_index1] & 2)
		{
			std::swap(lr[2], hr[2]);
			std::swap(lg[2], hg[2]);
			std::swap(lb[2], hb[2]);
			weight_inv[2] = 3;
		}

		uint64_t v = 0b100 | (part_id << 3);
		v |= (lr[0] << 9) | (hr[0] << (9 + 5 * 1));
		v |= (lr[1] << (9 + 5 * 2)) | (hr[1] << (9 + 5 * 3));
		v |= (uint64_t(lr[2]) << (9 + 5 * 4)) | (uint64_t(hr[2]) << (9 + 5 * 5));

		v |= (uint64_t(lg[0]) << (9 + 5 * 6)) | (uint64_t(hg[0]) << (9 + 5 * 7));
		v |= (uint64_t(lg[1]) << (9 + 5 * 8)) | (uint64_t(hg[1]) << (9 + 5 * 9));
		v |= (uint64_t(lg[2]) << (9 + 5 * 10));

		pBlock[0] = (uint8_t)v;
		pBlock[1] = (uint8_t)(v >> 8);
		pBlock[2] = (uint8_t)(v >> 16);
		pBlock[3] = (uint8_t)(v >> 24);
		pBlock[4] = (uint8_t)(v >> 32);
		pBlock[5] = (uint8_t)(v >> 40);
		pBlock[6] = (uint8_t)(v >> 48);
		pBlock[7] = (uint8_t)(v >> 56);

		uint64_t v1 = hg[2];
		v1 |= (lb[0] << (5 * 1)) | (hb[0] << (5 * 2));
		v1 |= (lb[1] << (5 * 3)) | (hb[1] << (5 * 4));
		v1 |= (lb[2] << (5 * 5)) | (uint64_t(hb[2]) << (5 * 6));

		pBlock[8] = (uint8_t)(v1);
		pBlock[9] = (uint8_t)(v1 >> 8);
		pBlock[10] = (uint8_t)(v1 >> 16);
		pBlock[11] = (uint8_t)(v1 >> 24);

		v1 >>= 32;

		// 3 bits left over
		uint32_t ofs = 3;

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];

			uint64_t w = pWeights[i] ^ weight_inv[subset_index];

#ifdef _DEBUG
			assert(w <= 3);
			if ((i == 0) || (i == anchor_index0) || (i == anchor_index1))
			{
				assert((w & 2) == 0);
			}
#endif
			v1 |= (w << ofs);

			ofs += (2 - ((i == 0) || (i == anchor_index0) || (i == anchor_index1)));
		}
		assert(32 == ofs);

		pBlock[12] = (uint8_t)v1;
		pBlock[13] = (uint8_t)(v1 >> 8);
		pBlock[14] = (uint8_t)(v1 >> 16);
		pBlock[15] = (uint8_t)(v1 >> 24);
	}

	void encode_mode3_rgb_block(uint8_t* pBlock, uint32_t part_id, // 2 subsets, 6-bits part ID
		uint32_t lr[2], uint32_t lg[2], uint32_t lb[2], // 7-bit endpoints, 4 unique pbits
		uint32_t hr[2], uint32_t hg[2], uint32_t hb[2],
		uint32_t p[4],
		const uint8_t* pWeights) // 2-bit weights
	{
		assert(part_id < 64);
		assert((lr[0] | lr[1] | lg[0] | lg[1] | lb[0] | lb[1]) <= 127);
		assert((hr[0] | hr[1] | hg[0] | hg[1] | hb[0] | hb[1]) <= 127);
		assert((p[0] | p[1] | p[2] | p[3]) <= 1);

		const uint8_t* pPart_map = &g_bc7_partition2[part_id * 16];
		const uint32_t anchor_index = g_bc7_table_anchor_index_second_subset[part_id];

		uint32_t weight_inv[2] = { 0, 0 };
		if (pWeights[0] & 2)
		{
			std::swap(lr[0], hr[0]);
			std::swap(lg[0], hg[0]);
			std::swap(lb[0], hb[0]);
			std::swap(p[0], p[1]);
			weight_inv[0] = 3;
		}

		if (pWeights[anchor_index] & 2)
		{
			std::swap(lr[1], hr[1]);
			std::swap(lg[1], hg[1]);
			std::swap(lb[1], hb[1]);
			std::swap(p[2], p[3]);
			weight_inv[1] = 3;
		}

		uint64_t x = 0b1000 | (part_id << 4) |
			(lr[0] << 10) | (hr[0] << 17) |
			(lr[1] << 24) | (uint64_t(hr[1]) << 31) |
			(uint64_t(lg[0]) << 38) | (uint64_t(hg[0]) << 45) |
			(uint64_t(lg[1]) << 52) | (uint64_t(hg[1]) << 59);

		pBlock[0] = (uint8_t)x;
		pBlock[1] = (uint8_t)(x >> 8);
		pBlock[2] = (uint8_t)(x >> 16);
		pBlock[3] = (uint8_t)(x >> 24);
		pBlock[4] = (uint8_t)(x >> 32);
		pBlock[5] = (uint8_t)(x >> 40);
		pBlock[6] = (uint8_t)(x >> 48);
		pBlock[7] = (uint8_t)(x >> 56);

		// 2 bits of hg[1] remaining to pack

		uint64_t y = (hg[1] >> 5) | (lb[0] << 2) | (hb[0] << 9) |
			(lb[1] << (9 + 7 * 1)) | (hb[1] << (9 + 7 * 2)) |
			(uint64_t(p[0]) << (9 + 7 * 3)) | (uint64_t(p[1]) << (9 + 7 * 3 + 1)) |
			(uint64_t(p[2]) << (9 + 7 * 3 + 2)) | (uint64_t(p[3]) << (9 + 7 * 3 + 3));

		// now 34 total bits

		uint32_t ofs = 34;
		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];
			uint64_t w = pWeights[i] ^ weight_inv[subset_index];

#ifdef _DEBUG
			assert(w <= 3);
			if ((i == 0) || (i == anchor_index))
			{
				assert((w & 2) == 0);
			}
#endif

			y |= (w << ofs);
			ofs += (2 - ((i == 0) || (i == anchor_index)));
		}
		assert(64 == ofs);

		pBlock[8] = (uint8_t)y;
		pBlock[9] = (uint8_t)(y >> 8);
		pBlock[10] = (uint8_t)(y >> 16);
		pBlock[11] = (uint8_t)(y >> 24);
		pBlock[12] = (uint8_t)(y >> 32);
		pBlock[13] = (uint8_t)(y >> 40);
		pBlock[14] = (uint8_t)(y >> 48);
		pBlock[15] = (uint8_t)(y >> 56);
	}

	void encode_mode4_rgba_block(uint8_t* pBlock,
		uint32_t lr, uint32_t lg, uint32_t lb, uint32_t la, // 5-bit RGB endpoints, 6-bit A endpoints, no p-bits
		uint32_t hr, uint32_t hg, uint32_t hb, uint32_t ha,
		const uint8_t* pWeights0, const uint8_t* pWeights1, // weights0=rgb weights1=alpha, index_flag controls which ones are 2 or 3 bits, for 0 it's 2-bit RGB/3-bit alpha
		uint32_t rot_index, uint32_t index_flag) // rot_index=0 no rotation, if index_flag is 1, the 3-bit indices are for RGB
	{
		assert((lr | lg | lb | hr | hg | hb) <= 31);
		assert((la | ha) <= 63);
		assert(rot_index <= 3);
		assert(index_flag <= 1);

		// defaults: 2nd plane=always alpha, RGB=3-bit indices, A=2-bits (favoring RGB)
		//const uint32_t rot_index = 0, index_flag = 1;

		uint32_t weights_inv[2] = { };

		const uint8_t* p2BitWeights = index_flag ? pWeights1 : pWeights0;
		const uint8_t* p3BitWeights = index_flag ? pWeights0 : pWeights1;

		// 3-bits
		if (p3BitWeights[0] & 4)
		{
			weights_inv[0] = 7;
			if (index_flag)
			{
				std::swap(lr, hr);
				std::swap(lg, hg);
				std::swap(lb, hb);
			}
			else
			{
				std::swap(la, ha);
			}
		}

		// 2-bits
		if (p2BitWeights[0] & 2)
		{
			weights_inv[1] = 3;
			if (index_flag)
			{
				std::swap(la, ha);
			}
			else
			{
				std::swap(lr, hr);
				std::swap(lg, hg);
				std::swap(lb, hb);
			}
		}

		pBlock[0] = (uint8_t)(0b10000 | (rot_index << 5) | (index_flag << 7));

		// 6*5+6*2=42 bits
		uint64_t x = lr | (hr << (5 * 1));
		x |= (lg << (5 * 2)) | (hg << (5 * 3));
		x |= (lb << (5 * 4)) | (hb << (5 * 5));
		x |= (uint64_t(la) << (5 * 6)) | (uint64_t(ha) << (5 * 6 + 6));

		pBlock[1] = (uint8_t)x;
		pBlock[2] = (uint8_t)(x >> 8);
		pBlock[3] = (uint8_t)(x >> 16);
		pBlock[4] = (uint8_t)(x >> 24);

		pBlock[5] = (uint8_t)(x >> 32);

		// 2 leftover bits
		x >>= 40;
		uint32_t ofs0 = 2;

		// alpha indices (2-bits)
		for (uint32_t i = 0; i < 16; i++)
		{
			assert(p2BitWeights[i] <= 3);
			uint64_t w = p2BitWeights[i] ^ weights_inv[1];

			assert(i || ((w & 2) == 0));

			x |= (w << ofs0);

			ofs0 += 2 - (i == 0);
		}

		// x = 31+2=33 bits

		pBlock[6] = (uint8_t)x;
		pBlock[7] = (uint8_t)(x >> 8);
		pBlock[8] = (uint8_t)(x >> 16);
		pBlock[9] = (uint8_t)(x >> 24);

		x >>= 32;

		// x = 1 bits
		uint32_t ofs1 = 1;

		// rgb indices (3-bits)
		for (uint32_t i = 0; i < 16; i++)
		{
			assert(p3BitWeights[i] <= 7);
			uint64_t w = p3BitWeights[i] ^ weights_inv[0];

			assert(i || ((w & 4) == 0));

			x |= (w << ofs1);

			ofs1 += 3 - (i == 0);
		}

		assert(ofs1 == 48);

		// x=48 bits
		pBlock[10] = (uint8_t)x;
		pBlock[11] = (uint8_t)(x >> 8);
		pBlock[12] = (uint8_t)(x >> 16);
		pBlock[13] = (uint8_t)(x >> 24);
		pBlock[14] = (uint8_t)(x >> 32);
		pBlock[15] = (uint8_t)(x >> 40);
	}

	// lossless in RGBA
	void pack_mode5_solid(uint8_t* pBlock, const color_rgba& c)
	{
		pBlock[0] = 0b00100000;

		uint32_t lr = basist::g_bc7_mode_5_optimal_endpoints[c[0]].m_lo;
		uint32_t hr = basist::g_bc7_mode_5_optimal_endpoints[c[0]].m_hi;

		uint32_t lg = basist::g_bc7_mode_5_optimal_endpoints[c[1]].m_lo;
		uint32_t hg = basist::g_bc7_mode_5_optimal_endpoints[c[1]].m_hi;

		uint32_t lb = basist::g_bc7_mode_5_optimal_endpoints[c[2]].m_lo;
		uint32_t hb = basist::g_bc7_mode_5_optimal_endpoints[c[2]].m_hi;

		// 8 endpoints are 8-bits, nothing fancy needed
		uint32_t a = c[3];

		// 58 total bits
		uint64_t x = lr | (hr << (7 * 1));
		x |= (lg << (7 * 2)) | (hg << (7 * 3));
		x |= (((uint64_t)lb) << (7 * 4)) | (((uint64_t)hb) << (7 * 5));
		x |= (((uint64_t)a) << (7 * 6)) | (((uint64_t)a) << (7 * 6 + 8));

		// write 56 bits, leaving 2 left over
		pBlock[1] = (uint8_t)(x);
		pBlock[2] = (uint8_t)(x >> 8);
		pBlock[3] = (uint8_t)(x >> 16);
		pBlock[4] = (uint8_t)(x >> 24);
		pBlock[5] = (uint8_t)(x >> 32);
		pBlock[6] = (uint8_t)(x >> 40);
		pBlock[7] = (uint8_t)(x >> 48);

		x >>= 56;
		assert(x <= 3);

#if 0
		x |= (0b0101010101010101010101010101011ull << 2);

		pBlock[8] = (uint8_t)(x);
		pBlock[9] = (uint8_t)(x >> 8);
		pBlock[10] = (uint8_t)(x >> 16);
		pBlock[11] = (uint8_t)(x >> 24);
		pBlock[12] = (uint8_t)(x >> 32);
		pBlock[13] = 0;
		pBlock[14] = 0;
		pBlock[15] = 0;
#elif 0
		// 0xaaaaaaac | x
		pBlock[8] = (uint8_t)(x) | 0xAC;

		static const uint8_t s_tail_bytes[7] = { 0xaa, 0xaa, 0xaa, 0, 0, 0, 0 };
		memcpy(pBlock + 9, s_tail_bytes, 7);
#elif 1
		static const uint8_t s_tail_bytes[8] = { 0xac, 0xaa, 0xaa, 0xaa, 0, 0, 0, 0 };
		memcpy(pBlock + 8, s_tail_bytes, 8);
		pBlock[8] |= (uint8_t)x;
#endif
	}

	void encode_mode5_rgba_block(uint8_t* pBlock,
		uint32_t lr, uint32_t lg, uint32_t lb, uint32_t la, // 7-bit RGB endpoints, 8-bit alpha endpoints
		uint32_t hr, uint32_t hg, uint32_t hb, uint32_t ha,
		const uint8_t* pColorWeights, const uint8_t* pAlphaWeights, // both 2-bit weights
		uint32_t rot_index = 0) // rot_index=0 no rotation
	{
		assert((lr | lg | lb | hr | hg | hb) <= 127);
		assert((la | ha) <= 255);
		assert(rot_index <= 3);

		uint32_t color_inv = 0, alpha_inv = 0;

		if (pColorWeights[0] & 2)
		{
			std::swap(lr, hr);
			std::swap(lg, hg);
			std::swap(lb, hb);
			color_inv = 3;
		}

		if (pAlphaWeights[0] & 2)
		{
			std::swap(la, ha);
			alpha_inv = 3;
		}

		uint64_t low = (1ULL << 5) | (rot_index << 6) |
			(lr << 8) | (hr << 15) |
			(lg << 22) | (uint64_t(hg) << 29) |
			(uint64_t(lb) << 36) | (uint64_t(hb) << 43) |
			(uint64_t(la) << 50) | (uint64_t(ha) << 58);

		pBlock[0] = (uint8_t)low;
		pBlock[1] = (uint8_t)(low >> 8);
		pBlock[2] = (uint8_t)(low >> 16);
		pBlock[3] = (uint8_t)(low >> 24);
		pBlock[4] = (uint8_t)(low >> 32);
		pBlock[5] = (uint8_t)(low >> 40);
		pBlock[6] = (uint8_t)(low >> 48);
		pBlock[7] = (uint8_t)(low >> 56);

		uint64_t high = (ha >> 6) & 3;

		uint32_t ofs = 2;

		for (uint32_t i = 0; i < 16; i++)
		{
			uint64_t w = pColorWeights[i] ^ color_inv;
#ifdef _DEBUG
			assert(w <= 3);
			if (i == 0)
			{
				assert((w & 2) == 0);
			}
#endif    
			high |= (w << ofs);
			ofs += (2 - (i == 0));
		}

		assert(33 == ofs);

		for (uint32_t i = 0; i < 16; i++)
		{
			uint64_t w = pAlphaWeights[i] ^ alpha_inv;
#ifdef _DEBUG
			assert(w <= 3);
			if (i == 0)
			{
				assert((w & 2) == 0);
			}
#endif    
			high |= (w << ofs);
			ofs += (2 - (i == 0));
		}

		assert(64 == ofs);

		pBlock[8] = (uint8_t)high;
		pBlock[9] = (uint8_t)(high >> 8);
		pBlock[10] = (uint8_t)(high >> 16);
		pBlock[11] = (uint8_t)(high >> 24);
		pBlock[12] = (uint8_t)(high >> 32);
		pBlock[13] = (uint8_t)(high >> 40);
		pBlock[14] = (uint8_t)(high >> 48);
		pBlock[15] = (uint8_t)(high >> 56);
	}

	void encode_mode6_rgba_block(uint8_t* pBlock,
		uint32_t lr, uint32_t lg, uint32_t lb, uint32_t la, uint32_t p0, // 7-bit endpoints, 2 shared p-bits
		uint32_t hr, uint32_t hg, uint32_t hb, uint32_t ha, uint32_t p1,
		const uint8_t* pWeights) // 4-bit weights
	{
		assert((lr | lg | lb | la | hr | hg | hb | ha) <= 127);
		assert((p0 | p1) <= 1);

		uint32_t weight_inv = 0;
		if (pWeights[0] & 8)
		{
			std::swap(lr, hr);
			std::swap(lg, hg);
			std::swap(lb, hb);
			std::swap(la, ha);
			std::swap(p0, p1);
			weight_inv = 15;
		}

		// 9*7=63 bits
		uint64_t x = 0b1000000 | (lr << (7 * 1)) | (hr << (7 * 2));
		x |= (lg << (7 * 3)) | (uint64_t(hg) << (7 * 4));
		x |= (uint64_t(lb) << (7 * 5)) | (uint64_t(hb) << (7 * 6));
		x |= (uint64_t(la) << (7 * 7)) | (uint64_t(ha) << (7 * 8));

		pBlock[0] = (uint8_t)x;
		pBlock[1] = (uint8_t)(x >> 8);
		pBlock[2] = (uint8_t)(x >> 16);
		pBlock[3] = (uint8_t)(x >> 24);

		pBlock[4] = (uint8_t)(x >> 32);
		pBlock[5] = (uint8_t)(x >> 40);
		pBlock[6] = (uint8_t)(x >> 48);
		x >>= 56;

		// x=7 bits
		x |= (p0 << 7);
		pBlock[7] = (uint8_t)x;

		uint64_t y = p1;
		uint32_t ofs = 1;
		// TODO: Unroll/optimize
		for (uint32_t i = 0; i < 16; i++)
		{
			uint64_t w = pWeights[i] ^ weight_inv;
			assert(w <= 15);
			assert(i || ((w & 8) == 0));
			y |= (w << ofs);
			ofs += 3 + (i > 0);
		}
		assert(64 == ofs);

		pBlock[8] = (uint8_t)y;
		pBlock[9] = (uint8_t)(y >> 8);
		pBlock[10] = (uint8_t)(y >> 16);
		pBlock[11] = (uint8_t)(y >> 24);
		pBlock[12] = (uint8_t)(y >> 32);
		pBlock[13] = (uint8_t)(y >> 40);
		pBlock[14] = (uint8_t)(y >> 48);
		pBlock[15] = (uint8_t)(y >> 56);
	}

	void encode_mode7_rgba_block(uint8_t* pBlock, uint32_t part_id, // 2 subsets, 6-bits part ID
		uint32_t lr[2], uint32_t lg[2], uint32_t lb[2], uint32_t la[2], // 5-bit endpoints, unique pbits
		uint32_t hr[2], uint32_t hg[2], uint32_t hb[2], uint32_t ha[2],
		uint32_t p[4],
		const uint8_t* pWeights) // 2-bit weights
	{
		assert(part_id < 64);
		assert((lr[0] | lr[1] | lg[0] | lg[1] | lb[0] | lb[1] | la[0] | la[1]) <= 31);
		assert((hr[0] | hr[1] | hg[0] | hg[1] | hb[0] | hb[1] | ha[0] | ha[1]) <= 31);
		assert((p[0] | p[1] | p[2] | p[3]) <= 1);

		const uint8_t* pPart_map = &g_bc7_partition2[part_id * 16];
		const uint32_t anchor_index = g_bc7_table_anchor_index_second_subset[part_id];

		uint32_t weight_inv[2] = { 0, 0 };
		if (pWeights[0] & 2)
		{
			std::swap(lr[0], hr[0]); std::swap(lg[0], hg[0]); std::swap(lb[0], hb[0]); std::swap(la[0], ha[0]);
			std::swap(p[0], p[1]);
			weight_inv[0] = 3;
		}

		if (pWeights[anchor_index] & 2)
		{
			std::swap(lr[1], hr[1]); std::swap(lg[1], hg[1]); std::swap(lb[1], hb[1]); std::swap(la[1], ha[1]);
			std::swap(p[2], p[3]);
			weight_inv[1] = 3;
		}

		uint64_t x = 0x80ULL | (part_id << 8) |
			(lr[0] << 14) | (hr[0] << 19) | (lr[1] << 24) | (uint64_t(hr[1]) << 29) |
			(uint64_t(lg[0]) << 34) | (uint64_t(hg[0]) << 39) | (uint64_t(lg[1]) << 44) | (uint64_t(hg[1]) << 49) |
			(uint64_t(lb[0]) << 54) | (uint64_t(hb[0]) << 59);

		pBlock[0] = (uint8_t)x;
		pBlock[1] = (uint8_t)(x >> 8);
		pBlock[2] = (uint8_t)(x >> 16);
		pBlock[3] = (uint8_t)(x >> 24);

		pBlock[4] = (uint8_t)(x >> 32);
		pBlock[5] = (uint8_t)(x >> 40);
		pBlock[6] = (uint8_t)(x >> 48);
		pBlock[7] = (uint8_t)(x >> 56);

		uint64_t y = (lb[1] << 0) | (hb[1] << 5) |
			(la[0] << 10) | (ha[0] << 15) | (la[1] << 20) | (ha[1] << 25) |
			(uint64_t(p[0]) << 30) | (uint64_t(p[1]) << 31) | (uint64_t(p[2]) << 32) | (uint64_t(p[3]) << 33);

		uint32_t ofs = 34;
		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];
			uint64_t w = pWeights[i] ^ weight_inv[subset_index];

#ifdef _DEBUG
			assert(w <= 3);
			if ((i == 0) || (i == anchor_index))
			{
				assert((w & 2) == 0);
			}
#endif

			y |= (w << ofs);
			ofs += (2 - ((i == 0) || (i == anchor_index)));
		}
		assert(64 == ofs);

		pBlock[8] = (uint8_t)y;
		pBlock[9] = (uint8_t)(y >> 8);
		pBlock[10] = (uint8_t)(y >> 16);
		pBlock[11] = (uint8_t)(y >> 24);

		pBlock[12] = (uint8_t)(y >> 32);
		pBlock[13] = (uint8_t)(y >> 40);
		pBlock[14] = (uint8_t)(y >> 48);
		pBlock[15] = (uint8_t)(y >> 56);
	}
		
	static bool compute_least_squares_endpoints_1D(
		uint32_t N, const uint8_t* pWeights, uint32_t num_weights,
		const vec4F* pSelector_weights,
		float& xl, float& xh,
		const color_rgba* pColors, uint32_t comp_index,
		float t_r)
	{
		BASISU_NOTE_UNUSED(num_weights);

		float z00 = 0.0f, z10 = 0.0f, z11 = 0.0f;
		float q00_r = 0.0f;

		for (uint32_t i = 0; i < N; i++)
		{
			const uint32_t sel = pWeights[i];
			assert(sel < num_weights);

			z00 += pSelector_weights[sel][0];
			z10 += pSelector_weights[sel][1];
			z11 += pSelector_weights[sel][2];

			const float w = pSelector_weights[sel][3];

			q00_r += w * (float)pColors[i][comp_index];
		}

		float q10_r = t_r - q00_r;

		float z01 = z10;

		float det = z00 * z11 - z01 * z10;
		if (fabs(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float iz00, iz01, iz10, iz11;
		iz00 = z11 * det;
		iz01 = -z01 * det;
		iz10 = -z10 * det;
		iz11 = z00 * det;

		xh = basisu::clamp(iz00 * q00_r + iz01 * q10_r, 0.0f, 255.0f);
		xl = basisu::clamp(iz10 * q00_r + iz11 * q10_r, 0.0f, 255.0f);

		return true;
	}

	static bool compute_least_squares_endpoints_3D(
		uint32_t N, const uint8_t* pWeights, uint32_t num_weights,
		const vec4F* pSelector_weights,
		vec4F& xl, vec4F& xh,
		const color_rgba* pColors,
		float t_r, float t_g, float t_b)
	{
		BASISU_NOTE_UNUSED(num_weights);

		float z00 = 0.0f, z10 = 0.0f, z11 = 0.0f;
		float q00_r = 0.0f, q00_g = 0.0f, q00_b = 0.0f;

		for (uint32_t i = 0; i < N; i++)
		{
			const uint32_t sel = pWeights[i];
			assert(sel < num_weights);

			z00 += pSelector_weights[sel][0];
			z10 += pSelector_weights[sel][1];
			z11 += pSelector_weights[sel][2];

			const float w = pSelector_weights[sel][3];

			q00_r += w * (float)pColors[i][0];
			q00_g += w * (float)pColors[i][1];
			q00_b += w * (float)pColors[i][2];
		}

		float q10_r = t_r - q00_r;
		float q10_g = t_g - q00_g;
		float q10_b = t_b - q00_b;

		float z01 = z10;

		float det = z00 * z11 - z01 * z10;
		if (fabs(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float iz00, iz01, iz10, iz11;
		iz00 = z11 * det;
		iz01 = -z01 * det;
		iz10 = -z10 * det;
		iz11 = z00 * det;

		xh[0] = basisu::clamp(iz00 * q00_r + iz01 * q10_r, 0.0f, 255.0f);
		xl[0] = basisu::clamp(iz10 * q00_r + iz11 * q10_r, 0.0f, 255.0f);

		xh[1] = basisu::clamp(iz00 * q00_g + iz01 * q10_g, 0.0f, 255.0f);
		xl[1] = basisu::clamp(iz10 * q00_g + iz11 * q10_g, 0.0f, 255.0f);

		xh[2] = basisu::clamp(iz00 * q00_b + iz01 * q10_b, 0.0f, 255.0f);
		xl[2] = basisu::clamp(iz10 * q00_b + iz11 * q10_b, 0.0f, 255.0f);

		xh[3] = 0;
		xl[3] = 0;

		return true;
	}

	static bool compute_least_squares_endpoints_4D(
		uint32_t N, const uint8_t* pWeights, uint32_t num_weights,
		const vec4F* pSelector_weights,
		vec4F& xl, vec4F& xh,
		const color_rgba* pColors,
		float t_r, float t_g, float t_b, float t_a)
	{
		BASISU_NOTE_UNUSED(num_weights);

		float z00 = 0.0f, z10 = 0.0f, z11 = 0.0f;
		float q00_r = 0.0f, q00_g = 0.0f, q00_b = 0.0f, q00_a = 0.0f;

		for (uint32_t i = 0; i < N; i++)
		{
			const uint32_t sel = pWeights[i];
			assert(sel < num_weights);

			z00 += pSelector_weights[sel][0];
			z10 += pSelector_weights[sel][1];
			z11 += pSelector_weights[sel][2];

			const float w = pSelector_weights[sel][3];

			q00_r += w * (float)pColors[i][0];
			q00_g += w * (float)pColors[i][1];
			q00_b += w * (float)pColors[i][2];
			q00_a += w * (float)pColors[i][3];
		}

		float q10_r = t_r - q00_r;
		float q10_g = t_g - q00_g;
		float q10_b = t_b - q00_b;
		float q10_a = t_a - q00_a;

		float z01 = z10;

		float det = z00 * z11 - z01 * z10;
		if (fabs(det) < 1e-8f)
			return false;

		det = 1.0f / det;

		float iz00, iz01, iz10, iz11;
		iz00 = z11 * det;
		iz01 = -z01 * det;
		iz10 = -z10 * det;
		iz11 = z00 * det;

		xh[0] = basisu::clamp(iz00 * q00_r + iz01 * q10_r, 0.0f, 255.0f);
		xl[0] = basisu::clamp(iz10 * q00_r + iz11 * q10_r, 0.0f, 255.0f);

		xh[1] = basisu::clamp(iz00 * q00_g + iz01 * q10_g, 0.0f, 255.0f);
		xl[1] = basisu::clamp(iz10 * q00_g + iz11 * q10_g, 0.0f, 255.0f);

		xh[2] = basisu::clamp(iz00 * q00_b + iz01 * q10_b, 0.0f, 255.0f);
		xl[2] = basisu::clamp(iz10 * q00_b + iz11 * q10_b, 0.0f, 255.0f);

		xh[3] = basisu::clamp(iz00 * q00_a + iz01 * q10_a, 0.0f, 255.0f);
		xl[3] = basisu::clamp(iz10 * q00_a + iz11 * q10_a, 0.0f, 255.0f);

		return true;
	}

#if BASISU_BC7F_USE_SSE41
	void bc7_proj_minmax_indices_sse41(const color_rgba* __restrict pPixels, int saxis_r, int saxis_g, int saxis_b, int* out_min_idx, int* out_max_idx)
	{
		__m128i coef32 = _mm_setr_epi32(saxis_r, saxis_g, saxis_b, 0);     // 32-bit lanes
		coef32 = _mm_srai_epi32(coef32, 4);                                // arithmetic >>4 in 32-bit
		__m128i COEF = _mm_packs_epi32(coef32, coef32);

		const __m128i ZERO = _mm_setzero_si128();

		__m128i vmin, vmax;
		{
			const __m128i px = _mm_loadu_si128((const __m128i*) & pPixels[0]);
			const __m128i lo16 = _mm_unpacklo_epi8(px, ZERO);  // [r0 g0 b0 a0 r1 g1 b1 a1]
			const __m128i hi16 = _mm_unpackhi_epi8(px, ZERO);  // [r2 g2 b2 a2 r3 g3 b3 a3]

			const __m128i lo32p = _mm_madd_epi16(lo16, COEF);
			const __m128i hi32p = _mm_madd_epi16(hi16, COEF);

			const __m128i lo_sum = _mm_add_epi32(lo32p, _mm_shuffle_epi32(lo32p, _MM_SHUFFLE(2, 3, 0, 1)));
			const __m128i hi_sum = _mm_add_epi32(hi32p, _mm_shuffle_epi32(hi32p, _MM_SHUFFLE(2, 3, 0, 1)));

			const __m128i pair01 = _mm_shuffle_epi32(lo_sum, _MM_SHUFFLE(2, 0, 2, 0));
			const __m128i pair23 = _mm_shuffle_epi32(hi_sum, _MM_SHUFFLE(2, 0, 2, 0));

			const __m128i p32p = _mm_unpacklo_epi64(pair01, pair23);

			const __m128i p32 = _mm_slli_epi32(p32p, 4);

			const __m128i keyed = _mm_add_epi32(p32, _mm_set_epi32(3, 2, 1, 0));

			vmin = keyed;
			vmax = keyed;
		}

		{
			const __m128i px = _mm_loadu_si128((const __m128i*) & pPixels[4]);
			const __m128i lo16 = _mm_unpacklo_epi8(px, ZERO);  // [r0 g0 b0 a0 r1 g1 b1 a1]
			const __m128i hi16 = _mm_unpackhi_epi8(px, ZERO);  // [r2 g2 b2 a2 r3 g3 b3 a3]

			const __m128i lo32p = _mm_madd_epi16(lo16, COEF);
			const __m128i hi32p = _mm_madd_epi16(hi16, COEF);

			const __m128i lo_sum = _mm_add_epi32(lo32p, _mm_shuffle_epi32(lo32p, _MM_SHUFFLE(2, 3, 0, 1)));
			const __m128i hi_sum = _mm_add_epi32(hi32p, _mm_shuffle_epi32(hi32p, _MM_SHUFFLE(2, 3, 0, 1)));

			const __m128i pair01 = _mm_shuffle_epi32(lo_sum, _MM_SHUFFLE(2, 0, 2, 0));
			const __m128i pair23 = _mm_shuffle_epi32(hi_sum, _MM_SHUFFLE(2, 0, 2, 0));

			const __m128i p32p = _mm_unpacklo_epi64(pair01, pair23);

			const __m128i p32 = _mm_slli_epi32(p32p, 4);

			const __m128i keyed = _mm_add_epi32(p32, _mm_set_epi32(7, 6, 5, 4));

			vmin = _mm_min_epi32(vmin, keyed);
			vmax = _mm_max_epi32(vmax, keyed);
		}

		{
			const __m128i px = _mm_loadu_si128((const __m128i*) & pPixels[8]);
			const __m128i lo16 = _mm_unpacklo_epi8(px, ZERO);  // [r0 g0 b0 a0 r1 g1 b1 a1]
			const __m128i hi16 = _mm_unpackhi_epi8(px, ZERO);  // [r2 g2 b2 a2 r3 g3 b3 a3]

			const __m128i lo32p = _mm_madd_epi16(lo16, COEF);
			const __m128i hi32p = _mm_madd_epi16(hi16, COEF);

			const __m128i lo_sum = _mm_add_epi32(lo32p, _mm_shuffle_epi32(lo32p, _MM_SHUFFLE(2, 3, 0, 1)));
			const __m128i hi_sum = _mm_add_epi32(hi32p, _mm_shuffle_epi32(hi32p, _MM_SHUFFLE(2, 3, 0, 1)));

			const __m128i pair01 = _mm_shuffle_epi32(lo_sum, _MM_SHUFFLE(2, 0, 2, 0));
			const __m128i pair23 = _mm_shuffle_epi32(hi_sum, _MM_SHUFFLE(2, 0, 2, 0));

			const __m128i p32p = _mm_unpacklo_epi64(pair01, pair23);

			const __m128i p32 = _mm_slli_epi32(p32p, 4);

			const __m128i keyed = _mm_add_epi32(p32, _mm_set_epi32(11, 10, 9, 8));

			vmin = _mm_min_epi32(vmin, keyed);
			vmax = _mm_max_epi32(vmax, keyed);
		}

		{
			const __m128i px = _mm_loadu_si128((const __m128i*) & pPixels[12]);
			const __m128i lo16 = _mm_unpacklo_epi8(px, ZERO);  // [r0 g0 b0 a0 r1 g1 b1 a1]
			const __m128i hi16 = _mm_unpackhi_epi8(px, ZERO);  // [r2 g2 b2 a2 r3 g3 b3 a3]

			const __m128i lo32p = _mm_madd_epi16(lo16, COEF);
			const __m128i hi32p = _mm_madd_epi16(hi16, COEF);

			const __m128i lo_sum = _mm_add_epi32(lo32p, _mm_shuffle_epi32(lo32p, _MM_SHUFFLE(2, 3, 0, 1)));
			const __m128i hi_sum = _mm_add_epi32(hi32p, _mm_shuffle_epi32(hi32p, _MM_SHUFFLE(2, 3, 0, 1)));

			const __m128i pair01 = _mm_shuffle_epi32(lo_sum, _MM_SHUFFLE(2, 0, 2, 0));
			const __m128i pair23 = _mm_shuffle_epi32(hi_sum, _MM_SHUFFLE(2, 0, 2, 0));

			const __m128i p32p = _mm_unpacklo_epi64(pair01, pair23);

			const __m128i p32 = _mm_slli_epi32(p32p, 4);

			const __m128i keyed = _mm_add_epi32(p32, _mm_set_epi32(15, 14, 13, 12));

			vmin = _mm_min_epi32(vmin, keyed);
			vmax = _mm_max_epi32(vmax, keyed);
		}

		__m128i t = _mm_shuffle_epi32(vmin, _MM_SHUFFLE(2, 3, 0, 1));
		vmin = _mm_min_epi32(vmin, t);
		t = _mm_shuffle_epi32(vmin, _MM_SHUFFLE(1, 0, 3, 2));
		vmin = _mm_min_epi32(vmin, t);
		const int min_keyed = _mm_cvtsi128_si32(vmin);

		t = _mm_shuffle_epi32(vmax, _MM_SHUFFLE(2, 3, 0, 1));
		vmax = _mm_max_epi32(vmax, t);
		t = _mm_shuffle_epi32(vmax, _MM_SHUFFLE(1, 0, 3, 2));
		vmax = _mm_max_epi32(vmax, t);
		const int max_keyed = _mm_cvtsi128_si32(vmax);

		*out_min_idx = (min_keyed & 0xF);
		*out_max_idx = (max_keyed & 0xF);
	}

	void eval_weights_mode6_rgb_sse41(
		const color_rgba* __restrict pPixels, uint8_t* __restrict pWeights,
		int lr, int lg, int lb,
		int hr, int hg, int hb,
		uint32_t p0, uint32_t p1)
	{
		lr = from_7(lr, p0); lg = from_7(lg, p0); lb = from_7(lb, p0);
		hr = from_7(hr, p1); hg = from_7(hg, p1); hb = from_7(hb, p1);

		const int dr = hr - lr;
		const int dg = hg - lg;
		const int db = hb - lb;

		const float denom = (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db)) + 0.00000125f;
		const float f = 15.0f / denom;

		const __m128i ZEROi = _mm_setzero_si128();
		const __m128i FIFTEEN = _mm_set1_epi32(15);
		const __m128  F = _mm_set1_ps(f);
		const __m128  HALF = _mm_set1_ps(0.5f);

		const __m128i EP16 = _mm_setr_epi16((short)lr, (short)lg, (short)lb, 0,
			(short)lr, (short)lg, (short)lb, 0);
		
		const __m128i COEF = _mm_setr_epi16((short)dr, (short)dg, (short)db, 0,
			(short)dr, (short)dg, (short)db, 0);

		for (int i = 0; i < 16; i += 4)
		{
			const __m128i px = _mm_loadu_si128((const __m128i*) & pPixels[i]);
						
			const __m128i lo16 = _mm_unpacklo_epi8(px, ZEROi);
			const __m128i hi16 = _mm_unpackhi_epi8(px, ZEROi);
						
			const __m128i lo_adj = _mm_sub_epi16(lo16, EP16);
			const __m128i hi_adj = _mm_sub_epi16(hi16, EP16);
						
			const __m128i lo32p = _mm_madd_epi16(lo_adj, COEF);
			const __m128i hi32p = _mm_madd_epi16(hi_adj, COEF);
						
			const __m128i lo_sum = _mm_add_epi32(lo32p, _mm_shuffle_epi32(lo32p, _MM_SHUFFLE(2, 3, 0, 1)));
			const __m128i hi_sum = _mm_add_epi32(hi32p, _mm_shuffle_epi32(hi32p, _MM_SHUFFLE(2, 3, 0, 1)));
			
			const __m128i pair01 = _mm_shuffle_epi32(lo_sum, _MM_SHUFFLE(2, 0, 2, 0));
			const __m128i pair23 = _mm_shuffle_epi32(hi_sum, _MM_SHUFFLE(2, 0, 2, 0));
			const __m128i dot32 = _mm_unpacklo_epi64(pair01, pair23);          
						
			__m128  y = _mm_add_ps(_mm_mul_ps(_mm_cvtepi32_ps(dot32), F), HALF);
			__m128i sel32 = _mm_cvttps_epi32(y);                                 
						
			sel32 = _mm_min_epi32(_mm_max_epi32(sel32, ZEROi), FIFTEEN);
						
			__m128i sel16 = _mm_packs_epi32(sel32, ZEROi);                     
			__m128i sel8 = _mm_packus_epi16(sel16, ZEROi);                     
			*(uint32_t*)&pWeights[i] = (uint32_t)_mm_cvtsi128_si32(sel8);
		}
	}
#endif

	BASISU_FORCE_INLINE uint32_t bc7_sse(
		int pr,
		int lr,
		int dr,
		int w)
	{
		assert((w >= 0) && (w <= 64));
		int re = pr - (lr + ((dr * (int)w + 32) >> 6));
		return (re * re);
	}

	BASISU_FORCE_INLINE uint32_t bc7_sse(
		int pr, int pg, int pb,
		int lr, int lg, int lb,
		int dr, int dg, int db,
		int w)
	{
		assert((w >= 0) && (w <= 64));
		int re = pr - (lr + ((dr * (int)w + 32) >> 6));
		int ge = pg - (lg + ((dg * (int)w + 32) >> 6));
		int be = pb - (lb + ((db * (int)w + 32) >> 6));
		return (re * re) + (ge * ge) + (be * be);
	}

	BASISU_FORCE_INLINE uint32_t bc7_sse(
		int pr, int pg, int pb, int pa,
		int lr, int lg, int lb, int la,
		int dr, int dg, int db, int da,
		int w)
	{
		assert((w >= 0) && (w <= 64));
		int re = pr - (lr + ((dr * (int)w + 32) >> 6));
		int ge = pg - (lg + ((dg * (int)w + 32) >> 6));
		int be = pb - (lb + ((db * (int)w + 32) >> 6));
		int ae = pa - (la + ((da * (int)w + 32) >> 6));
		return (re * re) + (ge * ge) + (be * be) + (ae * ae);
	}

	void eval_weights_mode6_rgb(const color_rgba* pPixels, uint8_t* pWeights, // 4-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb,
		uint32_t p0, uint32_t p1)
	{
		lr = from_7(lr, p0); lg = from_7(lg, p0); lb = from_7(lb, p0);
		hr = from_7(hr, p1); hg = from_7(hg, p1); hb = from_7(hb, p1);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 15.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = -(lr * dr + lg * dg + lb * db);

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)(float(pPixels[i + 0][0] * dr + pPixels[i + 0][1] * dg + pPixels[i + 0][2] * db + sofs) * f + .5f);
			int sel1 = (int)(float(pPixels[i + 1][0] * dr + pPixels[i + 1][1] * dg + pPixels[i + 1][2] * db + sofs) * f + .5f);
			int sel2 = (int)(float(pPixels[i + 2][0] * dr + pPixels[i + 2][1] * dg + pPixels[i + 2][2] * db + sofs) * f + .5f);
			int sel3 = (int)(float(pPixels[i + 3][0] * dr + pPixels[i + 3][1] * dg + pPixels[i + 3][2] * db + sofs) * f + .5f);

			if ((uint32_t)sel0 > 15) sel0 = (~sel0 >> 31) & 15;
			if ((uint32_t)sel1 > 15) sel1 = (~sel1 >> 31) & 15;
			if ((uint32_t)sel2 > 15) sel2 = (~sel2 >> 31) & 15;
			if ((uint32_t)sel3 > 15) sel3 = (~sel3 >> 31) & 15;

			pWeights[i + 0] = (uint8_t)sel0;
			pWeights[i + 1] = (uint8_t)sel1;
			pWeights[i + 2] = (uint8_t)sel2;
			pWeights[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode6_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights, // 4-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb,
		uint32_t p0, uint32_t p1)
	{
		lr = from_7(lr, p0); lg = from_7(lg, p0); lb = from_7(lb, p0);
		hr = from_7(hr, p1); hg = from_7(hg, p1); hb = from_7(hb, p1);

		// assumes packed a's are always 127
		const int la = from_7(127, p0);
		const int ha = from_7(127, p1);
		const int da = ha - la;

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 15.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = -(lr * dr + lg * dg + lb * db);

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)(float(pPixels[i + 0][0] * dr + pPixels[i + 0][1] * dg + pPixels[i + 0][2] * db + sofs) * f + .5f);
			int sel1 = (int)(float(pPixels[i + 1][0] * dr + pPixels[i + 1][1] * dg + pPixels[i + 1][2] * db + sofs) * f + .5f);
			int sel2 = (int)(float(pPixels[i + 2][0] * dr + pPixels[i + 2][1] * dg + pPixels[i + 2][2] * db + sofs) * f + .5f);
			int sel3 = (int)(float(pPixels[i + 3][0] * dr + pPixels[i + 3][1] * dg + pPixels[i + 3][2] * db + sofs) * f + .5f);

			if ((uint32_t)sel0 > 15) sel0 = (~sel0 >> 31) & 15;
			if ((uint32_t)sel1 > 15) sel1 = (~sel1 >> 31) & 15;
			if ((uint32_t)sel2 > 15) sel2 = (~sel2 >> 31) & 15;
			if ((uint32_t)sel3 > 15) sel3 = (~sel3 >> 31) & 15;

			pWeights[i + 0] = (uint8_t)sel0;
			pWeights[i + 1] = (uint8_t)sel1;
			pWeights[i + 2] = (uint8_t)sel2;
			pWeights[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][0], pPixels[i + 0][1], pPixels[i + 0][2], pPixels[i + 0][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel0]);
			sse += bc7_sse(pPixels[i + 1][0], pPixels[i + 1][1], pPixels[i + 1][2], pPixels[i + 1][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel1]);
			sse += bc7_sse(pPixels[i + 2][0], pPixels[i + 2][1], pPixels[i + 2][2], pPixels[i + 2][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel2]);
			sse += bc7_sse(pPixels[i + 3][0], pPixels[i + 3][1], pPixels[i + 3][2], pPixels[i + 3][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel3]);
		}

		return sse;
	}

	void eval_weights_mode6_rgba(const color_rgba* pPixels, uint8_t* pWeights, // 4-bits
		int lr, int lg, int lb, int la, int p0,
		int hr, int hg, int hb, int ha, int p1)
	{
		lr = from_7(lr, p0); lg = from_7(lg, p0); lb = from_7(lb, p0); la = from_7(la, p0);
		hr = from_7(hr, p1); hg = from_7(hg, p1); hb = from_7(hb, p1); ha = from_7(ha, p1);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;
		int da = ha - la;

		const float f = 15.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + basisu::squarei(da) + .00000125f);

		const int sofs = -(lr * dr + lg * dg + lb * db + la * da);

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)(float(pPixels[i + 0][0] * dr + pPixels[i + 0][1] * dg + pPixels[i + 0][2] * db + pPixels[i + 0][3] * da + sofs) * f + .5f);
			int sel1 = (int)(float(pPixels[i + 1][0] * dr + pPixels[i + 1][1] * dg + pPixels[i + 1][2] * db + pPixels[i + 1][3] * da + sofs) * f + .5f);
			int sel2 = (int)(float(pPixels[i + 2][0] * dr + pPixels[i + 2][1] * dg + pPixels[i + 2][2] * db + pPixels[i + 2][3] * da + sofs) * f + .5f);
			int sel3 = (int)(float(pPixels[i + 3][0] * dr + pPixels[i + 3][1] * dg + pPixels[i + 3][2] * db + pPixels[i + 3][3] * da + sofs) * f + .5f);

			if ((uint32_t)sel0 > 15) sel0 = (~sel0 >> 31) & 15;
			if ((uint32_t)sel1 > 15) sel1 = (~sel1 >> 31) & 15;
			if ((uint32_t)sel2 > 15) sel2 = (~sel2 >> 31) & 15;
			if ((uint32_t)sel3 > 15) sel3 = (~sel3 >> 31) & 15;

			pWeights[i + 0] = (uint8_t)sel0;
			pWeights[i + 1] = (uint8_t)sel1;
			pWeights[i + 2] = (uint8_t)sel2;
			pWeights[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode6_rgba_sse(const color_rgba* pPixels, uint8_t* pWeights, // 4-bits
		int lr, int lg, int lb, int la, int p0,
		int hr, int hg, int hb, int ha, int p1)
	{
		lr = from_7(lr, p0); lg = from_7(lg, p0); lb = from_7(lb, p0); la = from_7(la, p0);
		hr = from_7(hr, p1); hg = from_7(hg, p1); hb = from_7(hb, p1); ha = from_7(ha, p1);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;
		int da = ha - la;

		const float f = 15.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + basisu::squarei(da) + .00000125f);

		const int sofs = -(lr * dr + lg * dg + lb * db + la * da);

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)(float(pPixels[i + 0][0] * dr + pPixels[i + 0][1] * dg + pPixels[i + 0][2] * db + pPixels[i + 0][3] * da + sofs) * f + .5f);
			int sel1 = (int)(float(pPixels[i + 1][0] * dr + pPixels[i + 1][1] * dg + pPixels[i + 1][2] * db + pPixels[i + 1][3] * da + sofs) * f + .5f);
			int sel2 = (int)(float(pPixels[i + 2][0] * dr + pPixels[i + 2][1] * dg + pPixels[i + 2][2] * db + pPixels[i + 2][3] * da + sofs) * f + .5f);
			int sel3 = (int)(float(pPixels[i + 3][0] * dr + pPixels[i + 3][1] * dg + pPixels[i + 3][2] * db + pPixels[i + 3][3] * da + sofs) * f + .5f);

			if ((uint32_t)sel0 > 15) sel0 = (~sel0 >> 31) & 15;
			if ((uint32_t)sel1 > 15) sel1 = (~sel1 >> 31) & 15;
			if ((uint32_t)sel2 > 15) sel2 = (~sel2 >> 31) & 15;
			if ((uint32_t)sel3 > 15) sel3 = (~sel3 >> 31) & 15;

			pWeights[i + 0] = (uint8_t)sel0;
			pWeights[i + 1] = (uint8_t)sel1;
			pWeights[i + 2] = (uint8_t)sel2;
			pWeights[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][0], pPixels[i + 0][1], pPixels[i + 0][2], pPixels[i + 0][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel0]);
			sse += bc7_sse(pPixels[i + 1][0], pPixels[i + 1][1], pPixels[i + 1][2], pPixels[i + 1][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel1]);
			sse += bc7_sse(pPixels[i + 2][0], pPixels[i + 2][1], pPixels[i + 2][2], pPixels[i + 2][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel2]);
			sse += bc7_sse(pPixels[i + 3][0], pPixels[i + 3][1], pPixels[i + 3][2], pPixels[i + 3][3], lr, lg, lb, la, dr, dg, db, da, basist::g_bc7_weights4[sel3]);
		}

		return sse;
	}

	void eval_weights_mode1_rgb(const color_rgba* pPixels, uint8_t* pWeights, // 3-bits
		uint32_t blr[2], uint32_t blg[2], uint32_t blb[2], uint32_t bhr[2], uint32_t bhg[2], uint32_t bhb[2],
		uint32_t pbits[2], uint32_t subset_bitmask)
	{
		int lr[2], lg[2], lb[2], hr[2], hg[2], hb[2], dr[2], dg[2], db[2];

		for (uint32_t s = 0; s < 2; s++)
		{
			lr[s] = from_6(blr[s], pbits[s]);
			lg[s] = from_6(blg[s], pbits[s]);
			lb[s] = from_6(blb[s], pbits[s]);

			hr[s] = from_6(bhr[s], pbits[s]);
			hg[s] = from_6(bhg[s], pbits[s]);
			hb[s] = from_6(bhb[s], pbits[s]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[2] =
		{
			7.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			7.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f)
		};

		const int sofs[2] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1] };

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = (subset_bitmask >> i) & 1;

			int sel = (int)((float)(
				((int)pPixels[i][0]) * dr[subset_index] + ((int)pPixels[i][1]) * dg[subset_index] + ((int)pPixels[i][2]) * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 7)
				sel = (~sel >> 31) & 7;

			pWeights[i] = (uint8_t)sel;
		}
	}

	uint32_t eval_weights_mode1_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights, // 3-bits
		uint32_t blr[2], uint32_t blg[2], uint32_t blb[2], uint32_t bhr[2], uint32_t bhg[2], uint32_t bhb[2],
		uint32_t pbits[2], uint32_t subset_bitmask)
	{
		int lr[2], lg[2], lb[2], hr[2], hg[2], hb[2], dr[2], dg[2], db[2];

		for (uint32_t s = 0; s < 2; s++)
		{
			lr[s] = from_6(blr[s], pbits[s]);
			lg[s] = from_6(blg[s], pbits[s]);
			lb[s] = from_6(blb[s], pbits[s]);

			hr[s] = from_6(bhr[s], pbits[s]);
			hg[s] = from_6(bhg[s], pbits[s]);
			hb[s] = from_6(bhb[s], pbits[s]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[2] =
		{
			7.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			7.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f)
		};

		const int sofs[2] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1] };

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = (subset_bitmask >> i) & 1;

			int sel = (int)((float)(
				((int)pPixels[i][0]) * dr[subset_index] + ((int)pPixels[i][1]) * dg[subset_index] + ((int)pPixels[i][2]) * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 7)
				sel = (~sel >> 31) & 7;

			pWeights[i] = (uint8_t)sel;

			sse += bc7_sse(pPixels[i][0], pPixels[i][1], pPixels[i][2], lr[subset_index], lg[subset_index], lb[subset_index], dr[subset_index], dg[subset_index], db[subset_index], basist::g_bc7_weights3[sel]);
		}

		return sse;
	}

	void eval_weights_mode7_rgba(const color_rgba* pPixels, uint8_t* pWeights, // 2-bits
		uint32_t blr[2], uint32_t blg[2], uint32_t blb[2], uint32_t bla[2],
		uint32_t bhr[2], uint32_t bhg[2], uint32_t bhb[2], uint32_t bha[2],
		uint32_t pbits[4], uint32_t subset_bitmask)
	{
		int lr[2], lg[2], lb[2], la[2];
		int hr[2], hg[2], hb[2], ha[2];
		int dr[2], dg[2], db[2], da[2];

		for (uint32_t s = 0; s < 2; s++)
		{
			const uint32_t l_pbit = pbits[s * 2 + 0], h_pbit = pbits[s * 2 + 1];

			lr[s] = from_5(blr[s], l_pbit);
			lg[s] = from_5(blg[s], l_pbit);
			lb[s] = from_5(blb[s], l_pbit);
			la[s] = from_5(bla[s], l_pbit);

			hr[s] = from_5(bhr[s], h_pbit);
			hg[s] = from_5(bhg[s], h_pbit);
			hb[s] = from_5(bhb[s], h_pbit);
			ha[s] = from_5(bha[s], h_pbit);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
			da[s] = ha[s] - la[s];
		}

		const float f[2] =
		{
			3.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + basisu::squarei(da[0]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + basisu::squarei(da[1]) + .00000125f)
		};

		const int sofs[2] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0] + la[0] * da[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1] + la[1] * da[1] };

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = (subset_bitmask >> i) & 1;

			int sel = (int)((float)(
				((int)pPixels[i][0]) * dr[subset_index] + ((int)pPixels[i][1]) * dg[subset_index] + ((int)pPixels[i][2]) * db[subset_index] + ((int)pPixels[i][3]) * da[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 3)
				sel = (~sel >> 31) & 3;

			pWeights[i] = (uint8_t)sel;
		}
	}

	uint32_t eval_weights_mode7_rgba_sse(const color_rgba* pPixels, uint8_t* pWeights, // 2-bits
		uint32_t blr[2], uint32_t blg[2], uint32_t blb[2], uint32_t bla[2],
		uint32_t bhr[2], uint32_t bhg[2], uint32_t bhb[2], uint32_t bha[2],
		uint32_t pbits[4], uint32_t subset_bitmask)
	{
		int lr[2], lg[2], lb[2], la[2];
		int hr[2], hg[2], hb[2], ha[2];
		int dr[2], dg[2], db[2], da[2];

		for (uint32_t s = 0; s < 2; s++)
		{
			const uint32_t l_pbit = pbits[s * 2 + 0], h_pbit = pbits[s * 2 + 1];

			lr[s] = from_5(blr[s], l_pbit);
			lg[s] = from_5(blg[s], l_pbit);
			lb[s] = from_5(blb[s], l_pbit);
			la[s] = from_5(bla[s], l_pbit);

			hr[s] = from_5(bhr[s], h_pbit);
			hg[s] = from_5(bhg[s], h_pbit);
			hb[s] = from_5(bhb[s], h_pbit);
			ha[s] = from_5(bha[s], h_pbit);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
			da[s] = ha[s] - la[s];
		}

		const float f[2] =
		{
			3.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + basisu::squarei(da[0]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + basisu::squarei(da[1]) + .00000125f)
		};

		const int sofs[2] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0] + la[0] * da[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1] + la[1] * da[1] };

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = (subset_bitmask >> i) & 1;

			int sel = (int)((float)(
				((int)pPixels[i][0]) * dr[subset_index] + ((int)pPixels[i][1]) * dg[subset_index] + ((int)pPixels[i][2]) * db[subset_index] + ((int)pPixels[i][3]) * da[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 3)
				sel = (~sel >> 31) & 3;

			pWeights[i] = (uint8_t)sel;

			sse += bc7_sse(pPixels[i][0], pPixels[i][1], pPixels[i][2], pPixels[i][3],
				lr[subset_index], lg[subset_index], lb[subset_index], la[subset_index],
				dr[subset_index], dg[subset_index], db[subset_index], da[subset_index], basist::g_bc7_weights2[sel]);
		}

		return sse;
	}

	void eval_weights_mode3_rgb(const color_rgba* pPixels, uint8_t* pWeights, // 2-bits
		uint32_t blr[2], uint32_t blg[2], uint32_t blb[2], uint32_t bhr[2], uint32_t bhg[2], uint32_t bhb[2],
		uint32_t pbits[4], uint32_t subset_bitmask)
	{
		int lr[2], lg[2], lb[2], hr[2], hg[2], hb[2], dr[2], dg[2], db[2];

		for (uint32_t s = 0; s < 2; s++)
		{
			lr[s] = from_7(blr[s], pbits[s * 2 + 0]);
			lg[s] = from_7(blg[s], pbits[s * 2 + 0]);
			lb[s] = from_7(blb[s], pbits[s * 2 + 0]);

			hr[s] = from_7(bhr[s], pbits[s * 2 + 1]);
			hg[s] = from_7(bhg[s], pbits[s * 2 + 1]);
			hb[s] = from_7(bhb[s], pbits[s * 2 + 1]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[2] =
		{
			3.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f)
		};

		const int sofs[2] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1] };

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = (subset_bitmask >> i) & 1;

			int sel = (int)((float)(
				((int)pPixels[i][0]) * dr[subset_index] + ((int)pPixels[i][1]) * dg[subset_index] + ((int)pPixels[i][2]) * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 3)
				sel = (~sel >> 31) & 3;

			pWeights[i] = (uint8_t)sel;
		}
	}

	uint32_t eval_weights_mode3_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights, // 2-bits
		uint32_t blr[2], uint32_t blg[2], uint32_t blb[2], uint32_t bhr[2], uint32_t bhg[2], uint32_t bhb[2],
		uint32_t pbits[4], uint32_t subset_bitmask)
	{
		int lr[2], lg[2], lb[2], hr[2], hg[2], hb[2], dr[2], dg[2], db[2];

		for (uint32_t s = 0; s < 2; s++)
		{
			lr[s] = from_7(blr[s], pbits[s * 2 + 0]);
			lg[s] = from_7(blg[s], pbits[s * 2 + 0]);
			lb[s] = from_7(blb[s], pbits[s * 2 + 0]);

			hr[s] = from_7(bhr[s], pbits[s * 2 + 1]);
			hg[s] = from_7(bhg[s], pbits[s * 2 + 1]);
			hb[s] = from_7(bhb[s], pbits[s * 2 + 1]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[2] =
		{
			3.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f)
		};

		const int sofs[2] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1] };

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = (subset_bitmask >> i) & 1;

			int sel = (int)((float)(
				((int)pPixels[i][0]) * dr[subset_index] + ((int)pPixels[i][1]) * dg[subset_index] + ((int)pPixels[i][2]) * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 3)
				sel = (~sel >> 31) & 3;

			pWeights[i] = (uint8_t)sel;

			sse += bc7_sse(pPixels[i][0], pPixels[i][1], pPixels[i][2], lr[subset_index], lg[subset_index], lb[subset_index], dr[subset_index], dg[subset_index], db[subset_index], basist::g_bc7_weights2[sel]);
		}

		return sse;
	}

	void eval_weights_mode0_rgb(const color_rgba* pPixels, uint8_t* pWeights, // 3-bits
		uint32_t blr[3], uint32_t blg[3], uint32_t blb[3],
		uint32_t bhr[3], uint32_t bhg[3], uint32_t bhb[3],
		uint32_t pbits[6],
		uint32_t pat_index)
	{
		assert(pat_index <= 15);
		int lr[3], lg[3], lb[3], hr[3], hg[3], hb[3], dr[3], dg[3], db[3];

		for (uint32_t s = 0; s < 3; s++)
		{
			lr[s] = from_4(blr[s], pbits[s * 2 + 0]);
			lg[s] = from_4(blg[s], pbits[s * 2 + 0]);
			lb[s] = from_4(blb[s], pbits[s * 2 + 0]);

			hr[s] = from_4(bhr[s], pbits[s * 2 + 1]);
			hg[s] = from_4(bhg[s], pbits[s * 2 + 1]);
			hb[s] = from_4(bhb[s], pbits[s * 2 + 1]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[3] =
		{
			7.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			7.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f),
			7.0f / (float)(basisu::squarei(dr[2]) + basisu::squarei(dg[2]) + basisu::squarei(db[2]) + .00000125f)
		};

		const int sofs[3] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1],
			lr[2] * dr[2] + lg[2] * dg[2] + lb[2] * db[2] };

		const uint8_t* pPart_map = &g_bc7_partition3[pat_index * 16];

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];

			int sel = (int)((float)(
				(int)pPixels[i][0] * dr[subset_index] + (int)pPixels[i][1] * dg[subset_index] + (int)pPixels[i][2] * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 7)
				sel = (~sel >> 31) & 7;

			pWeights[i] = (uint8_t)sel;
		}
	}

	uint32_t eval_weights_mode0_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights, // 3-bits
		uint32_t blr[3], uint32_t blg[3], uint32_t blb[3],
		uint32_t bhr[3], uint32_t bhg[3], uint32_t bhb[3],
		uint32_t pbits[6],
		uint32_t pat_index)
	{
		assert(pat_index <= 15);
		int lr[3], lg[3], lb[3], hr[3], hg[3], hb[3], dr[3], dg[3], db[3];

		for (uint32_t s = 0; s < 3; s++)
		{
			lr[s] = from_4(blr[s], pbits[s * 2 + 0]);
			lg[s] = from_4(blg[s], pbits[s * 2 + 0]);
			lb[s] = from_4(blb[s], pbits[s * 2 + 0]);

			hr[s] = from_4(bhr[s], pbits[s * 2 + 1]);
			hg[s] = from_4(bhg[s], pbits[s * 2 + 1]);
			hb[s] = from_4(bhb[s], pbits[s * 2 + 1]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[3] =
		{
			7.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			7.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f),
			7.0f / (float)(basisu::squarei(dr[2]) + basisu::squarei(dg[2]) + basisu::squarei(db[2]) + .00000125f)
		};

		const int sofs[3] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1],
			lr[2] * dr[2] + lg[2] * dg[2] + lb[2] * db[2] };

		const uint8_t* pPart_map = &g_bc7_partition3[pat_index * 16];

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];

			int sel = (int)((float)(
				(int)pPixels[i][0] * dr[subset_index] + (int)pPixels[i][1] * dg[subset_index] + (int)pPixels[i][2] * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 7)
				sel = (~sel >> 31) & 7;

			pWeights[i] = (uint8_t)sel;

			sse += bc7_sse(pPixels[i][0], pPixels[i][1], pPixels[i][2],
				lr[subset_index], lg[subset_index], lb[subset_index],
				dr[subset_index], dg[subset_index], db[subset_index], basist::g_bc7_weights3[sel]);
		}

		return sse;
	}

	void eval_weights_mode2_rgb(const color_rgba* pPixels, uint8_t* pWeights, // 2-bits
		uint32_t blr[3], uint32_t blg[3], uint32_t blb[3], uint32_t bhr[3], uint32_t bhg[3], uint32_t bhb[3],
		uint32_t pat_index)
	{
		int lr[3], lg[3], lb[3], hr[3], hg[3], hb[3], dr[3], dg[3], db[3];

		for (uint32_t s = 0; s < 3; s++)
		{
			lr[s] = from_5(blr[s]);
			lg[s] = from_5(blg[s]);
			lb[s] = from_5(blb[s]);

			hr[s] = from_5(bhr[s]);
			hg[s] = from_5(bhg[s]);
			hb[s] = from_5(bhb[s]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[3] =
		{
			3.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[2]) + basisu::squarei(dg[2]) + basisu::squarei(db[2]) + .00000125f)
		};

		const int sofs[3] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1],
			lr[2] * dr[2] + lg[2] * dg[2] + lb[2] * db[2] };

		const uint8_t* pPart_map = &g_bc7_partition3[pat_index * 16];

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];

			int sel = (int)((float)(
				(int)pPixels[i][0] * dr[subset_index] + (int)pPixels[i][1] * dg[subset_index] + (int)pPixels[i][2] * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 3)
				sel = (~sel >> 31) & 3;

			pWeights[i] = (uint8_t)sel;
		}
	}

	uint32_t eval_weights_mode2_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights, // 2-bits
		uint32_t blr[3], uint32_t blg[3], uint32_t blb[3], uint32_t bhr[3], uint32_t bhg[3], uint32_t bhb[3],
		uint32_t pat_index)
	{
		int lr[3], lg[3], lb[3], hr[3], hg[3], hb[3], dr[3], dg[3], db[3];

		for (uint32_t s = 0; s < 3; s++)
		{
			lr[s] = from_5(blr[s]);
			lg[s] = from_5(blg[s]);
			lb[s] = from_5(blb[s]);

			hr[s] = from_5(bhr[s]);
			hg[s] = from_5(bhg[s]);
			hb[s] = from_5(bhb[s]);

			dr[s] = hr[s] - lr[s];
			dg[s] = hg[s] - lg[s];
			db[s] = hb[s] - lb[s];
		}

		const float f[3] =
		{
			3.0f / (float)(basisu::squarei(dr[0]) + basisu::squarei(dg[0]) + basisu::squarei(db[0]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[1]) + basisu::squarei(dg[1]) + basisu::squarei(db[1]) + .00000125f),
			3.0f / (float)(basisu::squarei(dr[2]) + basisu::squarei(dg[2]) + basisu::squarei(db[2]) + .00000125f)
		};

		const int sofs[3] = {
			lr[0] * dr[0] + lg[0] * dg[0] + lb[0] * db[0],
			lr[1] * dr[1] + lg[1] * dg[1] + lb[1] * db[1],
			lr[2] * dr[2] + lg[2] * dg[2] + lb[2] * db[2] };

		const uint8_t* pPart_map = &g_bc7_partition3[pat_index * 16];

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = pPart_map[i];

			int sel = (int)((float)(
				(int)pPixels[i][0] * dr[subset_index] + (int)pPixels[i][1] * dg[subset_index] + (int)pPixels[i][2] * db[subset_index] - sofs[subset_index]) * f[subset_index] + .5f);

			if ((uint32_t)sel > 3)
				sel = (~sel >> 31) & 3;

			pWeights[i] = (uint8_t)sel;

			sse += bc7_sse(pPixels[i][0], pPixels[i][1], pPixels[i][2],
				lr[subset_index], lg[subset_index], lb[subset_index],
				dr[subset_index], dg[subset_index], db[subset_index], basist::g_bc7_weights2[sel]);
		}

		return sse;
	}

	void eval_weights_mode4_3bit_rgb(const color_rgba* pPixels, uint8_t* pWeights0, // 3-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb)
	{
		lr = from_5(lr); lg = from_5(lg); lb = from_5(lb);
		hr = from_5(hr); hg = from_5(hg); hb = from_5(hb);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 7.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = lr * dr + lg * dg + lb * db;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)((int)pPixels[i + 0][0] * dr + (int)pPixels[i + 0][1] * dg + (int)pPixels[i + 0][2] * db - sofs) * f + .5f);
			int sel1 = (int)((float)((int)pPixels[i + 1][0] * dr + (int)pPixels[i + 1][1] * dg + (int)pPixels[i + 1][2] * db - sofs) * f + .5f);
			int sel2 = (int)((float)((int)pPixels[i + 2][0] * dr + (int)pPixels[i + 2][1] * dg + (int)pPixels[i + 2][2] * db - sofs) * f + .5f);
			int sel3 = (int)((float)((int)pPixels[i + 3][0] * dr + (int)pPixels[i + 3][1] * dg + (int)pPixels[i + 3][2] * db - sofs) * f + .5f);

			if ((uint32_t)sel0 > 7) sel0 = (~sel0 >> 31) & 7;
			if ((uint32_t)sel1 > 7) sel1 = (~sel1 >> 31) & 7;
			if ((uint32_t)sel2 > 7) sel2 = (~sel2 >> 31) & 7;
			if ((uint32_t)sel3 > 7) sel3 = (~sel3 >> 31) & 7;

			pWeights0[i + 0] = (uint8_t)sel0;
			pWeights0[i + 1] = (uint8_t)sel1;
			pWeights0[i + 2] = (uint8_t)sel2;
			pWeights0[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode4_3bit_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights0, // 3-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb)
	{
		lr = from_5(lr); lg = from_5(lg); lb = from_5(lb);
		hr = from_5(hr); hg = from_5(hg); hb = from_5(hb);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 7.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = lr * dr + lg * dg + lb * db;

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)((int)pPixels[i + 0][0] * dr + (int)pPixels[i + 0][1] * dg + (int)pPixels[i + 0][2] * db - sofs) * f + .5f);
			int sel1 = (int)((float)((int)pPixels[i + 1][0] * dr + (int)pPixels[i + 1][1] * dg + (int)pPixels[i + 1][2] * db - sofs) * f + .5f);
			int sel2 = (int)((float)((int)pPixels[i + 2][0] * dr + (int)pPixels[i + 2][1] * dg + (int)pPixels[i + 2][2] * db - sofs) * f + .5f);
			int sel3 = (int)((float)((int)pPixels[i + 3][0] * dr + (int)pPixels[i + 3][1] * dg + (int)pPixels[i + 3][2] * db - sofs) * f + .5f);

			if ((uint32_t)sel0 > 7) sel0 = (~sel0 >> 31) & 7;
			if ((uint32_t)sel1 > 7) sel1 = (~sel1 >> 31) & 7;
			if ((uint32_t)sel2 > 7) sel2 = (~sel2 >> 31) & 7;
			if ((uint32_t)sel3 > 7) sel3 = (~sel3 >> 31) & 7;

			pWeights0[i + 0] = (uint8_t)sel0;
			pWeights0[i + 1] = (uint8_t)sel1;
			pWeights0[i + 2] = (uint8_t)sel2;
			pWeights0[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][0], pPixels[i + 0][1], pPixels[i + 0][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights3[sel0]);
			sse += bc7_sse(pPixels[i + 1][0], pPixels[i + 1][1], pPixels[i + 1][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights3[sel1]);
			sse += bc7_sse(pPixels[i + 2][0], pPixels[i + 2][1], pPixels[i + 2][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights3[sel2]);
			sse += bc7_sse(pPixels[i + 3][0], pPixels[i + 3][1], pPixels[i + 3][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights3[sel3]);
		}

		return sse;
	}

	void eval_weights_mode4_2bit_rgb(const color_rgba* pPixels, uint8_t* pWeights0, // 2-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb)
	{
		lr = from_5(lr); lg = from_5(lg); lb = from_5(lb);
		hr = from_5(hr); hg = from_5(hg); hb = from_5(hb);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 3.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = lr * dr + lg * dg + lb * db;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)((int)pPixels[i + 0][0] * dr + (int)pPixels[i + 0][1] * dg + (int)pPixels[i + 0][2] * db - sofs) * f + .5f);
			int sel1 = (int)((float)((int)pPixels[i + 1][0] * dr + (int)pPixels[i + 1][1] * dg + (int)pPixels[i + 1][2] * db - sofs) * f + .5f);
			int sel2 = (int)((float)((int)pPixels[i + 2][0] * dr + (int)pPixels[i + 2][1] * dg + (int)pPixels[i + 2][2] * db - sofs) * f + .5f);
			int sel3 = (int)((float)((int)pPixels[i + 3][0] * dr + (int)pPixels[i + 3][1] * dg + (int)pPixels[i + 3][2] * db - sofs) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights0[i + 0] = (uint8_t)sel0;
			pWeights0[i + 1] = (uint8_t)sel1;
			pWeights0[i + 2] = (uint8_t)sel2;
			pWeights0[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode4_2bit_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights0, // 2-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb)
	{
		lr = from_5(lr); lg = from_5(lg); lb = from_5(lb);
		hr = from_5(hr); hg = from_5(hg); hb = from_5(hb);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 3.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = lr * dr + lg * dg + lb * db;

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)((int)pPixels[i + 0][0] * dr + (int)pPixels[i + 0][1] * dg + (int)pPixels[i + 0][2] * db - sofs) * f + .5f);
			int sel1 = (int)((float)((int)pPixels[i + 1][0] * dr + (int)pPixels[i + 1][1] * dg + (int)pPixels[i + 1][2] * db - sofs) * f + .5f);
			int sel2 = (int)((float)((int)pPixels[i + 2][0] * dr + (int)pPixels[i + 2][1] * dg + (int)pPixels[i + 2][2] * db - sofs) * f + .5f);
			int sel3 = (int)((float)((int)pPixels[i + 3][0] * dr + (int)pPixels[i + 3][1] * dg + (int)pPixels[i + 3][2] * db - sofs) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights0[i + 0] = (uint8_t)sel0;
			pWeights0[i + 1] = (uint8_t)sel1;
			pWeights0[i + 2] = (uint8_t)sel2;
			pWeights0[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][0], pPixels[i + 0][1], pPixels[i + 0][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel0]);
			sse += bc7_sse(pPixels[i + 1][0], pPixels[i + 1][1], pPixels[i + 1][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel1]);
			sse += bc7_sse(pPixels[i + 2][0], pPixels[i + 2][1], pPixels[i + 2][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel2]);
			sse += bc7_sse(pPixels[i + 3][0], pPixels[i + 3][1], pPixels[i + 3][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel3]);
		}

		return sse;
	}

	void eval_weights_mode4_2bit_a(const color_rgba* pPixels, uint8_t* pWeights1, // 2-bits
		int la, int ha)
	{
		la = from_6(la);
		ha = from_6(ha);

		int da = ha - la;

		const float f = 3.0f / (float)(da + .00000125f);

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)(pPixels[i + 0][3] - la) * f + .5f);
			int sel1 = (int)((float)(pPixels[i + 1][3] - la) * f + .5f);
			int sel2 = (int)((float)(pPixels[i + 2][3] - la) * f + .5f);
			int sel3 = (int)((float)(pPixels[i + 3][3] - la) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights1[i + 0] = (uint8_t)sel0;
			pWeights1[i + 1] = (uint8_t)sel1;
			pWeights1[i + 2] = (uint8_t)sel2;
			pWeights1[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode4_2bit_a_sse(const color_rgba* pPixels, uint8_t* pWeights1, // 2-bits
		int la, int ha)
	{
		la = from_6(la);
		ha = from_6(ha);

		int da = ha - la;

		const float f = 3.0f / (float)(da + .00000125f);

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)(pPixels[i + 0][3] - la) * f + .5f);
			int sel1 = (int)((float)(pPixels[i + 1][3] - la) * f + .5f);
			int sel2 = (int)((float)(pPixels[i + 2][3] - la) * f + .5f);
			int sel3 = (int)((float)(pPixels[i + 3][3] - la) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights1[i + 0] = (uint8_t)sel0;
			pWeights1[i + 1] = (uint8_t)sel1;
			pWeights1[i + 2] = (uint8_t)sel2;
			pWeights1[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][3], la, da, basist::g_bc7_weights2[sel0]);
			sse += bc7_sse(pPixels[i + 1][3], la, da, basist::g_bc7_weights2[sel1]);
			sse += bc7_sse(pPixels[i + 2][3], la, da, basist::g_bc7_weights2[sel2]);
			sse += bc7_sse(pPixels[i + 3][3], la, da, basist::g_bc7_weights2[sel3]);
		}

		return sse;
	}

	void eval_weights_mode4_3bit_a(const color_rgba* pPixels, uint8_t* pWeights1, // 3-bits
		int la, int ha)
	{
		la = from_6(la);
		ha = from_6(ha);

		int da = ha - la;

		const float f = 7.0f / (float)(da + .00000125f);

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)(pPixels[i + 0][3] - la) * f + .5f);
			int sel1 = (int)((float)(pPixels[i + 1][3] - la) * f + .5f);
			int sel2 = (int)((float)(pPixels[i + 2][3] - la) * f + .5f);
			int sel3 = (int)((float)(pPixels[i + 3][3] - la) * f + .5f);

			if ((uint32_t)sel0 > 7) sel0 = (~sel0 >> 31) & 7;
			if ((uint32_t)sel1 > 7) sel1 = (~sel1 >> 31) & 7;
			if ((uint32_t)sel2 > 7) sel2 = (~sel2 >> 31) & 7;
			if ((uint32_t)sel3 > 7) sel3 = (~sel3 >> 31) & 7;

			pWeights1[i + 0] = (uint8_t)sel0;
			pWeights1[i + 1] = (uint8_t)sel1;
			pWeights1[i + 2] = (uint8_t)sel2;
			pWeights1[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode4_3bit_a_sse(const color_rgba* pPixels, uint8_t* pWeights1, // 3-bits
		int la, int ha)
	{
		la = from_6(la);
		ha = from_6(ha);

		int da = ha - la;

		const float f = 7.0f / (float)(da + .00000125f);

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)(pPixels[i + 0][3] - la) * f + .5f);
			int sel1 = (int)((float)(pPixels[i + 1][3] - la) * f + .5f);
			int sel2 = (int)((float)(pPixels[i + 2][3] - la) * f + .5f);
			int sel3 = (int)((float)(pPixels[i + 3][3] - la) * f + .5f);

			if ((uint32_t)sel0 > 7) sel0 = (~sel0 >> 31) & 7;
			if ((uint32_t)sel1 > 7) sel1 = (~sel1 >> 31) & 7;
			if ((uint32_t)sel2 > 7) sel2 = (~sel2 >> 31) & 7;
			if ((uint32_t)sel3 > 7) sel3 = (~sel3 >> 31) & 7;

			pWeights1[i + 0] = (uint8_t)sel0;
			pWeights1[i + 1] = (uint8_t)sel1;
			pWeights1[i + 2] = (uint8_t)sel2;
			pWeights1[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][3], la, da, basist::g_bc7_weights3[sel0]);
			sse += bc7_sse(pPixels[i + 1][3], la, da, basist::g_bc7_weights3[sel1]);
			sse += bc7_sse(pPixels[i + 2][3], la, da, basist::g_bc7_weights3[sel2]);
			sse += bc7_sse(pPixels[i + 3][3], la, da, basist::g_bc7_weights3[sel3]);
		}

		return sse;
	}

	void eval_weights_mode5_2bit_rgb(const color_rgba* pPixels, uint8_t* pWeights0, // 2-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb)
	{
		lr = from_7(lr); lg = from_7(lg); lb = from_7(lb);
		hr = from_7(hr); hg = from_7(hg); hb = from_7(hb);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 3.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = lr * dr + lg * dg + lb * db;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)((int)pPixels[i + 0][0] * dr + (int)pPixels[i + 0][1] * dg + (int)pPixels[i + 0][2] * db - sofs) * f + .5f);
			int sel1 = (int)((float)((int)pPixels[i + 1][0] * dr + (int)pPixels[i + 1][1] * dg + (int)pPixels[i + 1][2] * db - sofs) * f + .5f);
			int sel2 = (int)((float)((int)pPixels[i + 2][0] * dr + (int)pPixels[i + 2][1] * dg + (int)pPixels[i + 2][2] * db - sofs) * f + .5f);
			int sel3 = (int)((float)((int)pPixels[i + 3][0] * dr + (int)pPixels[i + 3][1] * dg + (int)pPixels[i + 3][2] * db - sofs) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights0[i + 0] = (uint8_t)sel0;
			pWeights0[i + 1] = (uint8_t)sel1;
			pWeights0[i + 2] = (uint8_t)sel2;
			pWeights0[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode5_2bit_rgb_sse(const color_rgba* pPixels, uint8_t* pWeights0, // 2-bits
		int lr, int lg, int lb,
		int hr, int hg, int hb)
	{
		lr = from_7(lr); lg = from_7(lg); lb = from_7(lb);
		hr = from_7(hr); hg = from_7(hg); hb = from_7(hb);

		int dr = hr - lr;
		int dg = hg - lg;
		int db = hb - lb;

		const float f = 3.0f / (float)(basisu::squarei(dr) + basisu::squarei(dg) + basisu::squarei(db) + .00000125f);

		const int sofs = lr * dr + lg * dg + lb * db;

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)((int)pPixels[i + 0][0] * dr + (int)pPixels[i + 0][1] * dg + (int)pPixels[i + 0][2] * db - sofs) * f + .5f);
			int sel1 = (int)((float)((int)pPixels[i + 1][0] * dr + (int)pPixels[i + 1][1] * dg + (int)pPixels[i + 1][2] * db - sofs) * f + .5f);
			int sel2 = (int)((float)((int)pPixels[i + 2][0] * dr + (int)pPixels[i + 2][1] * dg + (int)pPixels[i + 2][2] * db - sofs) * f + .5f);
			int sel3 = (int)((float)((int)pPixels[i + 3][0] * dr + (int)pPixels[i + 3][1] * dg + (int)pPixels[i + 3][2] * db - sofs) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights0[i + 0] = (uint8_t)sel0;
			pWeights0[i + 1] = (uint8_t)sel1;
			pWeights0[i + 2] = (uint8_t)sel2;
			pWeights0[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][0], pPixels[i + 0][1], pPixels[i + 0][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel0]);
			sse += bc7_sse(pPixels[i + 1][0], pPixels[i + 1][1], pPixels[i + 1][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel1]);
			sse += bc7_sse(pPixels[i + 2][0], pPixels[i + 2][1], pPixels[i + 2][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel2]);
			sse += bc7_sse(pPixels[i + 3][0], pPixels[i + 3][1], pPixels[i + 3][2], lr, lg, lb, dr, dg, db, basist::g_bc7_weights2[sel3]);
		}

		return sse;
	}

	void eval_weights_mode5_2bit_a(const color_rgba* pPixels, uint8_t* pWeights1, // 2-bits
		int la, int ha)
	{
		int da = ha - la;

		const float f = 3.0f / (float)(da + .00000125f);

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)(pPixels[i + 0][3] - la) * f + .5f);
			int sel1 = (int)((float)(pPixels[i + 1][3] - la) * f + .5f);
			int sel2 = (int)((float)(pPixels[i + 2][3] - la) * f + .5f);
			int sel3 = (int)((float)(pPixels[i + 3][3] - la) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights1[i + 0] = (uint8_t)sel0;
			pWeights1[i + 1] = (uint8_t)sel1;
			pWeights1[i + 2] = (uint8_t)sel2;
			pWeights1[i + 3] = (uint8_t)sel3;
		}
	}

	uint32_t eval_weights_mode5_2bit_a_sse(const color_rgba* pPixels, uint8_t* pWeights1, // 2-bits
		int la, int ha)
	{
		int da = ha - la;

		const float f = 3.0f / (float)(da + .00000125f);

		uint32_t sse = 0;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int sel0 = (int)((float)(pPixels[i + 0][3] - la) * f + .5f);
			int sel1 = (int)((float)(pPixels[i + 1][3] - la) * f + .5f);
			int sel2 = (int)((float)(pPixels[i + 2][3] - la) * f + .5f);
			int sel3 = (int)((float)(pPixels[i + 3][3] - la) * f + .5f);

			if ((uint32_t)sel0 > 3) sel0 = (~sel0 >> 31) & 3;
			if ((uint32_t)sel1 > 3) sel1 = (~sel1 >> 31) & 3;
			if ((uint32_t)sel2 > 3) sel2 = (~sel2 >> 31) & 3;
			if ((uint32_t)sel3 > 3) sel3 = (~sel3 >> 31) & 3;

			pWeights1[i + 0] = (uint8_t)sel0;
			pWeights1[i + 1] = (uint8_t)sel1;
			pWeights1[i + 2] = (uint8_t)sel2;
			pWeights1[i + 3] = (uint8_t)sel3;

			sse += bc7_sse(pPixels[i + 0][3], la, da, basist::g_bc7_weights2[sel0]);
			sse += bc7_sse(pPixels[i + 1][3], la, da, basist::g_bc7_weights2[sel1]);
			sse += bc7_sse(pPixels[i + 2][3], la, da, basist::g_bc7_weights2[sel2]);
			sse += bc7_sse(pPixels[i + 3][3], la, da, basist::g_bc7_weights2[sel3]);
		}

		return sse;
	}

	// Determines the best unique pbits to use to encode xl/xh, which are [0,1]
	static void determine_unique_pbits(
		uint32_t total_comps, uint32_t comp_bits, float xl[4], float xh[4],
		color_rgba& bestMinColor, color_rgba& bestMaxColor, uint32_t best_pbits[2])
	{
#ifdef _DEBUG
		for (uint32_t c = 0; c < total_comps; c++)
		{
			assert((xl[c] >= 0.0f) && (xl[c] <= 1.0f));
			assert((xh[c] >= 0.0f) && (xh[c] <= 1.0f));
		}
#endif

		const uint32_t total_bits = comp_bits + 1;
		const int iscalep = (1 << total_bits) - 1;
		const float scalep = (float)iscalep;

		float best_err0 = 1e+9f;
		float best_err1 = 1e+9f;

		for (int p = 0; p < 2; p++)
		{
			color_rgba xMinColor, xMaxColor;

			for (uint32_t c = 0; c < 4; c++)
			{
				xMinColor[c] = (uint8_t)(clampi(((int)((xl[c] * scalep - p) * (1.0f / 2.0f) + .5f)) * 2 + p, p, iscalep - 1 + p));
				xMaxColor[c] = (uint8_t)(clampi(((int)((xh[c] * scalep - p) * (1.0f / 2.0f) + .5f)) * 2 + p, p, iscalep - 1 + p));
			}

			color_rgba scaledLow, scaledHigh;
			for (uint32_t i = 0; i < 4; i++)
			{
				scaledLow[i] = (xMinColor[i] << (8 - total_bits));
				scaledLow[i] |= (scaledLow[i] >> total_bits);
				assert(scaledLow[i] <= 255);

				scaledHigh[i] = (xMaxColor[i] << (8 - total_bits));
				scaledHigh[i] |= (scaledHigh[i] >> total_bits);
				assert(scaledHigh[i] <= 255);
			}

			float err0 = 0, err1 = 0;
			for (uint32_t i = 0; i < total_comps; i++)
			{
				err0 += basisu::squaref(scaledLow[i] - xl[i] * 255.0f);
				err1 += basisu::squaref(scaledHigh[i] - xh[i] * 255.0f);
			}

			if (err0 < best_err0)
			{
				best_err0 = err0;
				best_pbits[0] = p;

				bestMinColor[0] = xMinColor[0] >> 1;
				bestMinColor[1] = xMinColor[1] >> 1;
				bestMinColor[2] = xMinColor[2] >> 1;
				bestMinColor[3] = xMinColor[3] >> 1;
			}

			if (err1 < best_err1)
			{
				best_err1 = err1;
				best_pbits[1] = p;

				bestMaxColor[0] = xMaxColor[0] >> 1;
				bestMaxColor[1] = xMaxColor[1] >> 1;
				bestMaxColor[2] = xMaxColor[2] >> 1;
				bestMaxColor[3] = xMaxColor[3] >> 1;
			}
		}
	}

	// Determines the best shared pbits to use to encode xl/xh, which are [0,1]
	static void determine_shared_pbits(
		uint32_t total_comps, uint32_t comp_bits, float xl[4], float xh[4],
		color_rgba& bestMinColor, color_rgba& bestMaxColor, uint32_t best_pbits[2])
	{
#ifdef _DEBUG
		for (uint32_t c = 0; c < total_comps; c++)
		{
			assert((xl[c] >= 0.0f) && (xl[c] <= 1.0f));
			assert((xh[c] >= 0.0f) && (xh[c] <= 1.0f));
		}
#endif

		const uint32_t total_bits = comp_bits + 1;
		assert((total_bits >= 4) && (total_bits <= 8));

		const int iscalep = (1 << total_bits) - 1;
		const float scalep = (float)iscalep;

		float best_err = 1e+9f;

		for (int p = 0; p < 2; p++)
		{
			color_rgba xMinColor, xMaxColor;
			for (uint32_t c = 0; c < 4; c++)
			{
				xMinColor[c] = (uint8_t)(clampi(((int)((xl[c] * scalep - p) * (1.0f / 2.0f) + .5f)) * 2 + p, p, iscalep - 1 + p));
				xMaxColor[c] = (uint8_t)(clampi(((int)((xh[c] * scalep - p) * (1.0f / 2.0f) + .5f)) * 2 + p, p, iscalep - 1 + p));
			}

			color_rgba scaledLow, scaledHigh;

			for (uint32_t i = 0; i < 4; i++)
			{
				scaledLow[i] = (xMinColor[i] << (8 - total_bits));
				scaledLow[i] |= (scaledLow[i] >> total_bits);
				assert(scaledLow[i] <= 255);

				scaledHigh[i] = (xMaxColor[i] << (8 - total_bits));
				scaledHigh[i] |= (scaledHigh[i] >> total_bits);
				assert(scaledHigh[i] <= 255);
			}

			float err = 0;
			for (uint32_t i = 0; i < total_comps; i++)
				err += basisu::squaref((scaledLow[i] * (1.0f / 255.0f)) - xl[i]) + basisu::squaref((scaledHigh[i] * (1.0f / 255.0f)) - xh[i]);

			if (err < best_err)
			{
				best_err = err;
				best_pbits[0] = p;
				best_pbits[1] = p;
				for (uint32_t j = 0; j < 4; j++)
				{
					bestMinColor[j] = xMinColor[j] >> 1;
					bestMaxColor[j] = xMaxColor[j] >> 1;
				}
			}
		}
	}

#if 0
	// var must be variance (divided by N, # pixels), not SSE
	static inline int calc_span_est(int min_c, int max_c, int mean_c, float var)
	{
		// variance-implied span: span_var = ~sqrt(12 * var)
		int span_var = (int)fast_roundf_pos_int(std::sqrtf((float)(12.0f * var)));

		// take into account available headroom on the low/high end
		span_var = basisu::minimum<int>(span_var, 2 * basisu::minimum<int>(mean_c, 255 - mean_c));

		return basisu::minimum<int>(max_c - min_c, span_var);
	}
#endif

// doesn't seem to be a net win in quick testing, unlike ASTC
#define BASIST_USE_ALT_WEIGHT_QUANT_EST (0)

	// Multi-channel estimates
	// returns total SSE (pixel SSE * num_pixels), span_weights can be nullptr
	float analytical_quant_est_sse(int e_levels, int w_levels, int num_chans, const int spans[4], const float span_weights[4], float endpoint_weight_scale, int num_pixels)
	{
		assert((e_levels >= 2) && (e_levels <= 256) && (w_levels >= 2) && (num_chans));
		assert(spans);

		const float Dep = 1.0f / (float)(e_levels - 1); // endpoint quant step
#if BASIST_USE_ALT_WEIGHT_QUANT_EST
		const float Dw = 1.0f / (float)(w_levels); // weight quant step
#else
		const float Dw = 1.0f / (float)(w_levels - 1); // weight quant step
#endif

		// TODO: precompute
		const float N = float(w_levels);
		const float ab_sum = (2.0f * N - 1.0f) / (3.0f * (N - 1.0f));

		float pixel_sse = (e_levels == 256) ? 0.0f : ((Dep * Dep) * ((1.0f / 12.0f) * ab_sum * (255.0f * 255.0f)) * (float)num_chans * endpoint_weight_scale);

		const float k = (Dw * Dw) * (1.0f / 12.0f);
		for (int i = 0; i < num_chans; i++)
		{
			pixel_sse += k * (float)(spans[i] * spans[i]) * (span_weights ? span_weights[i] : 1.0f);
		}

		return pixel_sse * float(num_pixels);
	}

	// Single channel estimates
	float analytical_quant_est_sse(int e_levels, int w_levels, int span, float span_weight, float endpoint_weight_scale, int num_pixels)
	{
		assert((e_levels >= 2) && (e_levels <= 256) && (w_levels >= 2));

		const float Dep = 1.0f / (float)(e_levels - 1); // endpoint quant step
#if BASIST_USE_ALT_WEIGHT_QUANT_EST
		const float Dw = 1.0f / (float)(w_levels); // weight quant step
#else
		const float Dw = 1.0f / (float)(w_levels - 1); // weight quant step
#endif

		// TODO: precompute
		const float N = float(w_levels);
		const float ab_sum = (2.0f * N - 1.0f) / (3.0f * (N - 1.0f));

		float pixel_sse = (e_levels == 256) ? 0.0f : ((Dep * Dep) * ((1.0f / 12.0f) * ab_sum * (255.0f * 255.0f)) * endpoint_weight_scale);

		pixel_sse += (Dw * Dw) * (1.0f / 12.0f) * (float)(span * span) * span_weight;

		return pixel_sse * float(num_pixels);
	}

	// if cov[] wasn't divided by the # of pixels, this is SSE
	float estimate_slam_to_line_sse_3D(const float cov[6], float xr, float yr, float zr, float* pOrtho_ratio = nullptr)
	{
		// total var
		const float total_var = cov[0] + cov[3] + cov[5];

		float l = sqrtf(xr * xr + yr * yr + zr * zr);
		if (l < basisu::SMALL_FLOAT_VAL)
		{
			xr = yr = zr = 0.577350269f;
		}
		else
		{
			l = 1.0f / l;
			xr *= l; yr *= l; zr *= l;
		}

		float xr2 = cov[0] * xr + cov[1] * yr + cov[2] * zr;
		float xg2 = cov[1] * xr + cov[3] * yr + cov[4] * zr;
		float xb2 = cov[2] * xr + cov[4] * yr + cov[5] * zr;

		// Rayleigh quotient/est var of principal axis
		const float principal_axis_var = xr2 * xr + xg2 * yr + xb2 * zr;

		// Compute leftover var, this is the var unexplaind by the principal axis
		const float ortho_var = basisu::maximum(0.0f, total_var - principal_axis_var);

		if (pOrtho_ratio)
			*pOrtho_ratio = (total_var > basisu::SMALL_FLOAT_VAL) ? (ortho_var / total_var) : 0.0f;

		return ortho_var;
	}

	float estimate_slam_to_line_sse_4D(const float cov[10], float xr, float yr, float zr, float wr, float* pOrtho_ratio = nullptr)
	{
		// total var
		const float total_var = cov[0] + cov[4] + cov[7] + cov[9];

		float l = sqrtf(xr * xr + yr * yr + zr * zr + wr * wr);
		if (l < basisu::SMALL_FLOAT_VAL)
		{
			xr = yr = zr = wr = .5f;
		}
		else
		{
			l = 1.0f / l;
			xr *= l; yr *= l; zr *= l; wr *= l;
		}

		float xr2 = cov[0] * xr + cov[1] * yr + cov[2] * zr + cov[3] * wr;
		float xg2 = cov[1] * xr + cov[4] * yr + cov[5] * zr + cov[6] * wr;
		float xb2 = cov[2] * xr + cov[5] * yr + cov[7] * zr + cov[8] * wr;
		float xa2 = cov[3] * xr + cov[6] * yr + cov[8] * zr + cov[9] * wr;

		// Rayleigh quotient/est var of principal axis
		const float principal_axis_var = xr2 * xr + xg2 * yr + xb2 * zr + xa2 * wr;

		// Compute leftover var, this is the var unexplaind by the principal axis
		const float ortho_var = basisu::maximum(0.0f, total_var - principal_axis_var);

		if (pOrtho_ratio)
			*pOrtho_ratio = (total_var > basisu::SMALL_FLOAT_VAL) ? (ortho_var / total_var) : 0.0f;

		return ortho_var;
	}

	uint32_t calc_sse(const uint8_t* pBlock, const color_rgba* pPixels)
	{
		color_rgba unpacked_pixels[16];
		bool status = bc7u::unpack_bc7(pBlock, unpacked_pixels);
		if (!status)
		{
			assert(0);
			return UINT32_MAX;
		}

		uint32_t sse = 0;
		for (uint32_t i = 0; i < 16; i++)
			sse += basisu::squarei(pPixels[i][0] - unpacked_pixels[i][0]) + basisu::squarei(pPixels[i][1] - unpacked_pixels[i][1]) + basisu::squarei(pPixels[i][2] - unpacked_pixels[i][2]) + basisu::squarei(pPixels[i][3] - unpacked_pixels[i][3]);

		return sse;
	}

	bool pack_mode1_or_3_rgb(uint8_t* pBlock, const color_rgba* pPixels,
		float block_xr, float block_xg, float block_xb,
		int block_mean_r, int block_mean_g, int block_mean_b,
		float sse_est_to_beat, uint32_t flags,
		float* pFinal_sse_est = nullptr,
		uint32_t* pActual_sse = nullptr)
	{
#if BASISU_BC7F_PERF_STATS
		g_total_mode13_evals++;
#endif

		uint32_t desired_pat_bits = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const float r = (float)(pPixels[i].r - block_mean_r);
			const float g = (float)(pPixels[i].g - block_mean_g);
			const float b = (float)(pPixels[i].b - block_mean_b);

			const uint32_t subset = (r * block_xr + g * block_xg + b * block_xb) > 0.0f;

			desired_pat_bits |= (subset << i);
		}

		uint32_t best_diff = UINT32_MAX;

		if (flags & cPackBC7FlagASTCCompatible)
		{
			// We're actually encoding to ASTC 4x4, so we can only check the 2-subset patterns in common between BC7 and ASTC (the UASTC LDR 4x4 patterns).
			for (uint32_t c = 0; c < TOTAL_ASTC_BC7_COMMON_PARTITIONS2; c++)
			{
				const uint32_t p = g_astc_bc7_common_partitions2[c].m_bc7;

				const uint32_t bc6h_pat_bits = g_bc7_part2_bitmasks[p];

				int diff = popcount32(bc6h_pat_bits ^ desired_pat_bits);
				int diff_inv = 16 - diff;

				uint32_t min_diff = (basisu::minimum<int>(diff, diff_inv) << 8) | p;
				if (min_diff < best_diff)
					best_diff = min_diff;
			} // p
		}
		else
		{
			for (uint32_t p = 0; p < MAX_PATTERNS2_TO_CHECK; p++)
			{
				const uint32_t bc6h_pat_bits = g_bc7_part2_bitmasks[p];

				int diff = popcount32(bc6h_pat_bits ^ desired_pat_bits);
				int diff_inv = 16 - diff;

				uint32_t min_diff = (basisu::minimum<int>(diff, diff_inv) << 8) | p;
				if (min_diff < best_diff)
					best_diff = min_diff;
			} // p
		}

		const uint32_t best_pat_index = best_diff & 0xFF;
		const uint32_t best_pat_bits = g_bc7_part2_bitmasks[best_pat_index];

		int total_r[2] = { }, total_g[2] = { }, total_b[2] = { }, total_c[2] = { };
		for (uint32_t i = 0; i < 16; i++)
		{
			const int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b;
			const int subset = (best_pat_bits >> i) & 1;

			total_r[subset] += r; total_g[subset] += g; total_b[subset] += b;
			total_c[subset]++;
		}

		int mean_r[2], mean_g[2], mean_b[2];
		for (uint32_t s = 0; s < 2; s++)
		{
			const uint32_t t = total_c[s];
			const uint32_t h = (t >> 1);

			mean_r[s] = (total_r[s] + h) / t;
			mean_g[s] = (total_g[s] + h) / t;
			mean_b[s] = (total_b[s] + h) / t;
		}

		int icov[2][6] = { { }, { } };

		for (uint32_t i = 0; i < 16; i++)
		{
			const int subset = (best_pat_bits >> i) & 1;

			int r = (int)pPixels[i].r - mean_r[subset];
			int g = (int)pPixels[i].g - mean_g[subset];
			int b = (int)pPixels[i].b - mean_b[subset];
			icov[subset][0] += r * r; icov[subset][1] += r * g; icov[subset][2] += r * b;
			icov[subset][3] += g * g; icov[subset][4] += g * b;
			icov[subset][5] += b * b;
		}

		int ar[2], ag[2], ab[2];

		// Slam to line SSE estimate is the same for both mode 1 and 3.
		float slam_to_line_sse_est = 0.0f;

		for (uint32_t s = 0; s < 2; s++)
		{
			int block_max_var = basisu::maximum(icov[s][0], icov[s][3], icov[s][5]);

			float cov[6];
			for (uint32_t i = 0; i < 6; i++)
				cov[i] = (float)icov[s][i];

			const float sc = 1.0f / ((float)block_max_var + .0000125f);
			const float wx = sc * cov[0], wy = sc * cov[3], wz = sc * cov[5];

			const float alt_xr = cov[0] * wx + cov[1] * wy + cov[2] * wz;
			const float alt_xg = cov[1] * wx + cov[3] * wy + cov[4] * wz;
			const float alt_xb = cov[2] * wx + cov[4] * wy + cov[5] * wz;

			slam_to_line_sse_est += estimate_slam_to_line_sse_3D(cov, alt_xr, alt_xg, alt_xb);

			int saxis_r = 306, saxis_g = 601, saxis_b = 117;

			float k = basisu::maximum(fabsf(alt_xr), fabsf(alt_xg), fabsf(alt_xb));
			if (fabs(k) >= basisu::SMALL_FLOAT_VAL)
			{
				float m = 2048.0f / k;
				saxis_r = (int)(alt_xr * m);
				saxis_g = (int)(alt_xg * m);
				saxis_b = (int)(alt_xb * m);
			}

			ar[s] = (int)((uint32_t)saxis_r << 4U);
			ag[s] = (int)((uint32_t)saxis_g << 4U);
			ab[s] = (int)((uint32_t)saxis_b << 4U);
		} // s

		int low_dot[2] = { INT_MAX, INT_MAX };
		int high_dot[2] = { INT_MIN, INT_MIN };

		for (uint32_t i = 0; i < 16; i++)
		{
			const int subset = (best_pat_bits >> i) & 1;
			const int saxis_r = ar[subset], saxis_g = ag[subset], saxis_b = ab[subset];

			int dot = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b) + i;

			low_dot[subset] = basisu::minimum(low_dot[subset], dot);
			high_dot[subset] = basisu::maximum(high_dot[subset], dot);
		}

		int low_c[2] = { low_dot[0] & 15, low_dot[1] & 15 };
		int high_c[2] = { high_dot[0] & 15, high_dot[1] & 15 };

		int spans[4];
		spans[3] = 0;

		// Endpoint/weight quant error estimates for modes 1 and 3
		float quant_err_sse_est[2] = { };

		for (uint32_t subset = 0; subset < 2; subset++)
		{
			const uint32_t low_pixel = low_c[subset];
			const uint32_t high_pixel = high_c[subset];

			for (uint32_t c = 0; c < 3; c++)
				spans[c] = pPixels[high_pixel][c] - pPixels[low_pixel][c];

			// mode 1: 6-bit endpoints, unique pbits, 3 bit weights
			quant_err_sse_est[0] += analytical_quant_est_sse(64, 8, 3, spans, nullptr, (flags & cPackBC7FlagPBitOpt) ? UNIQUE_PBIT_DISCOUNT : 1.0f, total_c[subset]);

			// mode 3, 7-bit endpoints, shared pbits, 2-bit weights
			quant_err_sse_est[1] += analytical_quant_est_sse(128, 4, 3, spans, nullptr, (flags & cPackBC7FlagPBitOpt) ? SHARED_PBIT_DISCOUNT : 1.0f, total_c[subset]);

		} // subset

		const float total_mode1_est_sse = slam_to_line_sse_est + quant_err_sse_est[0];
		const float total_mode3_est_sse = slam_to_line_sse_est + quant_err_sse_est[1];

		if (total_mode1_est_sse < total_mode3_est_sse)
		{
			if (pFinal_sse_est)
				*pFinal_sse_est = total_mode1_est_sse;

			// Mode 1: Large span
			if (total_mode1_est_sse >= sse_est_to_beat)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_mode13_bailouts++;
#endif
				return false;
			}

			uint32_t lr[2], lg[2], lb[2];
			uint32_t hr[2], hg[2], hb[2];
			uint32_t pbits[2] = { 0, 0 };

			for (uint32_t s = 0; s < 2; s++)
			{
				const int lc = low_c[s], hc = high_c[s];

				if (flags & cPackBC7FlagPBitOpt)
				{
					const float q = 1.0f / 255.0f;
					float sxl[4] = { (float)pPixels[lc].r * q, (float)pPixels[lc].g * q, (float)pPixels[lc].b * q, 0 };
					float sxh[4] = { (float)pPixels[hc].r * q, (float)pPixels[hc].g * q, (float)pPixels[hc].b * q, 0 };

					color_rgba bestMinColor, bestMaxColor;
					uint32_t best_pbits[2];
					determine_shared_pbits(3, 6, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

					pbits[s] = best_pbits[0];
					lr[s] = bestMinColor.r, lg[s] = bestMinColor.g, lb[s] = bestMinColor.b;
					hr[s] = bestMaxColor.r, hg[s] = bestMaxColor.g, hb[s] = bestMaxColor.b;
				}
				else
				{
					int l = pPixels[lc].r + pPixels[lc].g + pPixels[lc].b;
					int h = pPixels[hc].r + pPixels[hc].g + pPixels[hc].b;

					if (basisu::maximum(l, h) >= 129 * 3)
						pbits[s] = 1;

					lr[s] = to_6(pPixels[lc].r, pbits[s]);
					lg[s] = to_6(pPixels[lc].g, pbits[s]);
					lb[s] = to_6(pPixels[lc].b, pbits[s]);

					hr[s] = to_6(pPixels[hc].r, pbits[s]);
					hg[s] = to_6(pPixels[hc].g, pbits[s]);
					hb[s] = to_6(pPixels[hc].b, pbits[s]);
				}
			} // s

			uint8_t cur_weights[16];
			eval_weights_mode1_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_bits);

			float z00[2] = { 0.0f }, z10[2] = { 0.0f }, z11[2] = { 0.0f };
			float q00_r[2] = { 0.0f };
			float q00_g[2] = { 0.0f };
			float q00_b[2] = { 0.0f };

			for (uint32_t i = 0; i < 16; i++)
			{
				const int subset = (best_pat_bits >> i) & 1;
				const uint32_t sel = cur_weights[i];
				assert(sel <= 7);

				z00[subset] += g_bc7_3bit_ls_tab[sel][0];
				z10[subset] += g_bc7_3bit_ls_tab[sel][1];
				z11[subset] += g_bc7_3bit_ls_tab[sel][2];

				const float w = g_bc7_3bit_ls_tab[sel][3];

				q00_r[subset] += w * (float)pPixels[i][0];
				q00_g[subset] += w * (float)pPixels[i][1];
				q00_b[subset] += w * (float)pPixels[i][2];
			} // i

			for (uint32_t s = 0; s < 2; s++)
			{
				float q10_r = (float)total_r[s] - q00_r[s];
				float q10_g = (float)total_g[s] - q00_g[s];
				float q10_b = (float)total_b[s] - q00_b[s];

				float z01 = z10[s];

				float det = z00[s] * z11[s] - z01 * z10[s];
				if (fabs(det) < 1e-8f)
					continue;

				det = 1.0f / det;

				float iz00, iz01, iz10, iz11;
				iz00 = z11[s] * det;
				iz01 = -z01 * det;
				iz10 = -z10[s] * det;
				iz11 = z00[s] * det;

				const float shr = iz00 * q00_r[s] + iz01 * q10_r;
				const float slr = iz10 * q00_r[s] + iz11 * q10_r;

				const float shg = iz00 * q00_g[s] + iz01 * q10_g;
				const float slg = iz10 * q00_g[s] + iz11 * q10_g;

				const float shb = iz00 * q00_b[s] + iz01 * q10_b;
				const float slb = iz10 * q00_b[s] + iz11 * q10_b;

				if (flags & cPackBC7FlagPBitOpt)
				{
					const float q = 1.0f / 255.0f;
					float sxl[4] = { basisu::clamp(slr * q, 0.0f, 1.0f), basisu::clamp(slg * q, 0.0f, 1.0f), basisu::clamp(slb * q, 0.0f, 1.0f), 0 };
					float sxh[4] = { basisu::clamp(shr * q, 0.0f, 1.0f), basisu::clamp(shg * q, 0.0f, 1.0f), basisu::clamp(shb * q, 0.0f, 1.0f), 0 };

					color_rgba bestMinColor, bestMaxColor;
					uint32_t best_pbits[2];
					determine_shared_pbits(3, 6, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

					pbits[s] = best_pbits[0];
					lr[s] = bestMinColor.r, lg[s] = bestMinColor.g, lb[s] = bestMinColor.b;
					hr[s] = bestMaxColor.r, hg[s] = bestMaxColor.g, hb[s] = bestMaxColor.b;
				}
				else
				{
					const float l = slr + slg + slb, h = shr + shg + shb;

					pbits[s] = (basisu::maximum(l, h) >= 129.0f * 3.0f);

					lr[s] = to_6_clamp(slr, pbits[s]);
					hr[s] = to_6_clamp(shr, pbits[s]);

					lg[s] = to_6_clamp(slg, pbits[s]);
					hg[s] = to_6_clamp(shg, pbits[s]);

					lb[s] = to_6_clamp(slb, pbits[s]);
					hb[s] = to_6_clamp(shb, pbits[s]);
				}

			} // s

			if (pActual_sse)
				*pActual_sse = eval_weights_mode1_rgb_sse(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_bits);
			else
				eval_weights_mode1_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_bits);

			encode_mode1_rgb_block(pBlock, best_pat_index,
				lr, lg, lb, hr, hg, hb, pbits[0], pbits[1], cur_weights);
		}
		else
		{
			// Mode 3: Small span
			if (pFinal_sse_est)
				*pFinal_sse_est = total_mode3_est_sse;

			if (total_mode3_est_sse >= sse_est_to_beat)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_mode13_bailouts++;
#endif
				return false;
			}

			uint32_t lr[2], lg[2], lb[2];
			uint32_t hr[2], hg[2], hb[2];
			uint32_t pbits[4];

			for (uint32_t s = 0; s < 2; s++)
			{
				const int lc = low_c[s];
				const int hc = high_c[s];

				if (flags & cPackBC7FlagPBitOpt)
				{
					const float q = 1.0f / 255.0f;
					float sxl[4] = { (float)pPixels[lc].r * q, (float)pPixels[lc].g * q, (float)pPixels[lc].b * q, 0 };
					float sxh[4] = { (float)pPixels[hc].r * q, (float)pPixels[hc].g * q, (float)pPixels[hc].b * q, 0 };

					color_rgba bestMinColor, bestMaxColor;
					uint32_t best_pbits[2];
					determine_unique_pbits(3, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

					pbits[s * 2 + 0] = best_pbits[0];
					pbits[s * 2 + 1] = best_pbits[1];
					lr[s] = bestMinColor.r, lg[s] = bestMinColor.g, lb[s] = bestMinColor.b;
					hr[s] = bestMaxColor.r, hg[s] = bestMaxColor.g, hb[s] = bestMaxColor.b;
				}
				else
				{
					const int l = pPixels[lc].r + pPixels[lc].g + pPixels[lc].b;
					const int l_pbit = (l >= 129);
					pbits[s * 2 + 0] = l_pbit;

					lr[s] = to_7(pPixels[lc].r, l_pbit);
					lg[s] = to_7(pPixels[lc].g, l_pbit);
					lb[s] = to_7(pPixels[lc].b, l_pbit);

					int h = pPixels[hc].r + pPixels[hc].g + pPixels[hc].b;
					const int h_pbit = (h >= 129);
					pbits[s * 2 + 1] = h_pbit;

					hr[s] = to_7(pPixels[hc].r, h_pbit);
					hg[s] = to_7(pPixels[hc].g, h_pbit);
					hb[s] = to_7(pPixels[hc].b, h_pbit);
				}
			} // s

			uint8_t cur_weights[16];
			eval_weights_mode3_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_bits);

			float z00[2] = { 0.0f }, z10[2] = { 0.0f }, z11[2] = { 0.0f };
			float q00_r[2] = { 0.0f };
			float q00_g[2] = { 0.0f };
			float q00_b[2] = { 0.0f };

			for (uint32_t i = 0; i < 16; i++)
			{
				const int subset = (best_pat_bits >> i) & 1;
				const uint32_t sel = cur_weights[i];
				assert(sel <= 3);

				z00[subset] += g_bc7_2bit_ls_tab[sel][0];
				z10[subset] += g_bc7_2bit_ls_tab[sel][1];
				z11[subset] += g_bc7_2bit_ls_tab[sel][2];

				const float w = g_bc7_2bit_ls_tab[sel][3];

				q00_r[subset] += w * (float)pPixels[i][0];
				q00_g[subset] += w * (float)pPixels[i][1];
				q00_b[subset] += w * (float)pPixels[i][2];
			} // i

			for (uint32_t s = 0; s < 2; s++)
			{
				float q10_r = (float)total_r[s] - q00_r[s];
				float q10_g = (float)total_g[s] - q00_g[s];
				float q10_b = (float)total_b[s] - q00_b[s];

				float z01 = z10[s];

				float det = z00[s] * z11[s] - z01 * z10[s];
				if (fabs(det) < 1e-8f)
					continue;

				det = 1.0f / det;

				float iz00, iz01, iz10, iz11;
				iz00 = z11[s] * det;
				iz01 = -z01 * det;
				iz10 = -z10[s] * det;
				iz11 = z00[s] * det;

				const float shr = iz00 * q00_r[s] + iz01 * q10_r;
				const float slr = iz10 * q00_r[s] + iz11 * q10_r;

				const float shg = iz00 * q00_g[s] + iz01 * q10_g;
				const float slg = iz10 * q00_g[s] + iz11 * q10_g;

				const float shb = iz00 * q00_b[s] + iz01 * q10_b;
				const float slb = iz10 * q00_b[s] + iz11 * q10_b;

				if (flags & cPackBC7FlagPBitOpt)
				{
					const float q = 1.0f / 255.0f;
					float sxl[4] = { basisu::clamp(slr * q, 0.0f, 1.0f), basisu::clamp(slg * q, 0.0f, 1.0f), basisu::clamp(slb * q, 0.0f, 1.0f), 0 };
					float sxh[4] = { basisu::clamp(shr * q, 0.0f, 1.0f), basisu::clamp(shg * q, 0.0f, 1.0f), basisu::clamp(shb * q, 0.0f, 1.0f), 0 };

					color_rgba bestMinColor, bestMaxColor;
					uint32_t best_pbits[2];
					determine_unique_pbits(3, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

					pbits[s * 2 + 0] = best_pbits[0];
					pbits[s * 2 + 1] = best_pbits[1];
					lr[s] = bestMinColor.r, lg[s] = bestMinColor.g, lb[s] = bestMinColor.b;
					hr[s] = bestMaxColor.r, hg[s] = bestMaxColor.g, hb[s] = bestMaxColor.b;
				}
				else
				{
					const float l = slr + slg + slb;
					const int l_pbit = (l >= 129.0f * 3.0f);
					pbits[s * 2 + 0] = l_pbit;

					lr[s] = to_7_clamp(slr, l_pbit);
					lg[s] = to_7_clamp(slg, l_pbit);
					lb[s] = to_7_clamp(slb, l_pbit);

					const float h = shr + shg + shb;
					const int h_pbit = (h >= 129.0f * 3.0f);
					pbits[s * 2 + 1] = h_pbit;

					hr[s] = to_7_clamp(shr, h_pbit);
					hg[s] = to_7_clamp(shg, h_pbit);
					hb[s] = to_7_clamp(shb, h_pbit);
				}

			} // s

			if (pActual_sse)
				*pActual_sse = eval_weights_mode3_rgb_sse(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_bits);
			else
				eval_weights_mode3_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_bits);

			encode_mode3_rgb_block(pBlock, best_pat_index,
				lr, lg, lb, hr, hg, hb, pbits, cur_weights);
		}

#ifdef _DEBUG
		if (pActual_sse)
		{
			const uint32_t expected_sse = calc_sse(pBlock, pPixels);
			assert(expected_sse == *pActual_sse);
		}
#endif

		return true;
	}

	inline int dist3(int lr, int lg, int lb, int hr, int hg, int hb)
	{
		return basisu::squarei(hr - lr) + basisu::squarei(hg - lg) + basisu::squarei(hb - lb);
	}

	bool determine_3subsets(uint8_t* pFinal_3subsets,
		const color_rgba* pPixels,
		float block_xr, float block_xg, float block_xb,
		int block_mean_r, int block_mean_g, int block_mean_b)
	{
		uint32_t subset_indices[16];
		int subset_means[2][3] = { };
		int subset_total[2] = { };

		for (uint32_t i = 0; i < 16; i++)
		{
			const int rd = pPixels[i].r - block_mean_r;
			const int gd = pPixels[i].g - block_mean_g;
			const int bd = pPixels[i].b - block_mean_b;

			const uint32_t subset_index = ((float)rd * block_xr + (float)gd * block_xg + (float)bd * block_xb) > 0.0f;

			subset_indices[i] = subset_index;

			subset_means[subset_index][0] += pPixels[i].r;
			subset_means[subset_index][1] += pPixels[i].g;
			subset_means[subset_index][2] += pPixels[i].b;

			subset_total[subset_index]++;
		}

		for (uint32_t i = 0; i < 2; i++)
		{
			const uint32_t t = subset_total[i];
			if (!t)
				return false;

			subset_means[i][0] = (subset_means[i][0] + (t >> 1)) / t;
			subset_means[i][1] = (subset_means[i][1] + (t >> 1)) / t;
			subset_means[i][2] = (subset_means[i][2] + (t >> 1)) / t;
		}

		int subset_sses[2] = { };
		for (uint32_t i = 0; i < 16; i++)
		{
			const uint32_t subset_index = subset_indices[i];

			subset_sses[subset_index] += dist3(pPixels[i].r, pPixels[i].g, pPixels[i].b, subset_means[subset_index][0], subset_means[subset_index][1], subset_means[subset_index][2]);
		}

		const uint32_t subset_to_split = (subset_sses[1] > subset_sses[0]);
		if (subset_total[subset_to_split] < 2)
			return false;

		int lo_y = INT_MAX, hi_y = 0;
		for (uint32_t i = 0; i < 16; i++)
		{
			if (subset_indices[i] != subset_to_split)
				continue;

			int y = ((pPixels[i].r + pPixels[i].g + pPixels[i].b) << 4) + i;

			lo_y = basisu::minimum(lo_y, y);
			hi_y = basisu::maximum(hi_y, y);
		}

		const int lo_y_index = lo_y & 15, hi_y_index = hi_y & 15;
		if (lo_y_index == hi_y_index)
			return false;

		const int lr = pPixels[lo_y_index].r, lg = pPixels[lo_y_index].g, lb = pPixels[lo_y_index].b;
		const int hr = pPixels[hi_y_index].r, hg = pPixels[hi_y_index].g, hb = pPixels[hi_y_index].b;

		memset(pFinal_3subsets, 2, 16);

		for (uint32_t i = 0; i < 16; i++)
		{
			if (subset_indices[i] == subset_to_split)
			{
				const int dist0 = dist3(lr, lg, lb, pPixels[i].r, pPixels[i].g, pPixels[i].b);
				const int dist1 = dist3(hr, hg, hb, pPixels[i].r, pPixels[i].g, pPixels[i].b);

				pFinal_3subsets[i] = dist1 > dist0;
			}
		}

		return true;
	}
		
	static inline int pop16(uint32_t x)
	{
#if defined(_MSC_VER)
		return __popcnt16((unsigned short)x);
#else
		return __builtin_popcount(x & 0xFFFFu);
#endif
	}

	int pick_3subset_pat_index(const uint8_t* pDesired_subsets, uint32_t& best_pat_index_first16, uint32_t flags)
	{
		best_pat_index_first16 = 0;

		uint16_t M[3];
		memset(M, 0, sizeof(M));

		for (uint32_t i = 0; i < 16; i++)
		{
			uint32_t s = pDesired_subsets[i];
			M[s] |= (1 << i);
		}

		const int n0 = pop16(M[0]), n1 = pop16(M[1]), n2 = 16 - n0 - n1;

		int best_score = -1;
		int best_pat = 0;

		if (flags & cPackBC7FlagASTCCompatible)
		{
			for (int t = 0; t < (int)TOTAL_ASTC_BC7_COMMON_PARTITIONS3; t++)
			{
				const int p = g_astc_bc7_common_partitions3[t].m_bc7;

				uint16_t S0 = (uint16_t)(g_part3_bitmasks[p] & 0xFFFFu);
				uint16_t S1 = (uint16_t)(g_part3_bitmasks[p] >> 16);

				// Row sums for subsets 0 and 1 via 6 popcnts; derive subset 2 by subtraction
				int C00 = pop16(M[0] & S0), C01 = pop16(M[0] & S1), C02 = n0 - C00 - C01;
				int C10 = pop16(M[1] & S0), C11 = pop16(M[1] & S1), C12 = n1 - C10 - C11;
				int C20 = pop16(M[2] & S0), C21 = pop16(M[2] & S1), C22 = n2 - C20 - C21;

				int s0 = C00 + C11 + C22; // (0,1,2)
				int s1 = C00 + C12 + C21; // (0,2,1)
				int s2 = C01 + C10 + C22; // (1,0,2)
				int s3 = C01 + C12 + C20; // (1,2,0)
				int s4 = C02 + C10 + C21; // (2,0,1)
				int s5 = C02 + C11 + C20; // (2,1,0)

				// Argmax over 6
				int s = s0;
				if (s1 > s) { s = s1; }
				if (s2 > s) { s = s2; }
				if (s3 > s) { s = s3; }
				if (s4 > s) { s = s4; }
				if (s5 > s) { s = s5; }

				if (s > best_score)
				{
					best_score = s;
					best_pat = p;

					if (s == 16)
					{
						// perfect match so early out
						if (p <= 15)
							best_pat_index_first16 = best_pat;
						break;
					}
				}

				if (t == 6)
				{
					assert(best_pat < 16);

					// for mode 0
					best_pat_index_first16 = best_pat;
				}
			} // t
		}
		else
		{
			for (int p = 0; p < (int)MAX_PATTERNS3_TO_CHECK; ++p)
			{
				uint16_t S0 = (uint16_t)(g_part3_bitmasks[p] & 0xFFFFu);
				uint16_t S1 = (uint16_t)(g_part3_bitmasks[p] >> 16);

				// Row sums for subsets 0 and 1 via 6 popcnts; derive subset 2 by subtraction
				int C00 = pop16(M[0] & S0), C01 = pop16(M[0] & S1), C02 = n0 - C00 - C01;
				int C10 = pop16(M[1] & S0), C11 = pop16(M[1] & S1), C12 = n1 - C10 - C11;
				int C20 = pop16(M[2] & S0), C21 = pop16(M[2] & S1), C22 = n2 - C20 - C21;

				int s0 = C00 + C11 + C22; // (0,1,2)
				int s1 = C00 + C12 + C21; // (0,2,1)
				int s2 = C01 + C10 + C22; // (1,0,2)
				int s3 = C01 + C12 + C20; // (1,2,0)
				int s4 = C02 + C10 + C21; // (2,0,1)
				int s5 = C02 + C11 + C20; // (2,1,0)

				// Argmax over 6
				int s = s0;
				if (s1 > s) { s = s1; }
				if (s2 > s) { s = s2; }
				if (s3 > s) { s = s3; }
				if (s4 > s) { s = s4; }
				if (s5 > s) { s = s5; }

				if (s > best_score)
				{
					best_score = s;
					best_pat = p;

					if (s == 16)
					{
						// perfect match so early out
						if (p <= 15)
							best_pat_index_first16 = best_pat;
						break;
					}
				}

				if (p == 15)
				{
					// for mode 0
					best_pat_index_first16 = best_pat;
				}
			}
		}

		return best_pat;
	}

#if 0
	static const uint8_t s_perms3[6][3] = { {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0} };

	int pick_3subset_pat_index_slow(const uint8_t* pDesired_subsets, uint32_t& best_pat_index_first16)
	{
		int best_pat = 0, best_dist = INT_MAX;

		for (uint32_t m = 0; m < 64; m++)
		{
			const uint8_t* pPat = &g_bc7_partition3[m * 16];

			for (uint32_t p = 0; p < 6; p++)
			{
				int trial_dist = 0;

				for (uint32_t i = 0; i < 16; i++)
				{
					uint32_t s = s_perms3[p][pDesired_subsets[i]];

					trial_dist += (s != pPat[i]);

				} // i

				if (trial_dist < best_dist)
				{
					best_dist = trial_dist;
					best_pat = m;
				}

			} // p

			if (m == 15)
				best_pat_index_first16 = best_pat;

		}  // m

		return best_pat;
	}
#endif

	// false if packing failed (not enough unique colors)
	bool pack_mode0_or_2_rgb(uint8_t* pBlock, const color_rgba* pPixels,
		float block_xr, float block_xg, float block_xb,
		int block_mean_r, int block_mean_g, int block_mean_b, float sse_est_to_beat, uint32_t flags,
		float* pFinal_sse_est = nullptr,
		uint32_t* pActual_sse = nullptr)
	{
		(void)flags;

#if BASISU_BC7F_PERF_STATS
		g_total_mode02_evals++;
#endif

		uint8_t desired_3subsets[16];
		if (!determine_3subsets(desired_3subsets, pPixels, block_xr, block_xg, block_xb, block_mean_r, block_mean_g, block_mean_b))
		{
#if BASISU_BC7F_PERF_STATS
			g_total_mode02_bailouts++;
#endif
			if (pFinal_sse_est)
				*pFinal_sse_est = 1e+9f;

			return false;
		}

		uint32_t best_pat_indices[2]; // mode 0 and 2
		best_pat_indices[1] = pick_3subset_pat_index(desired_3subsets, best_pat_indices[0], flags);

		assert((best_pat_indices[0] <= 15) && (best_pat_indices[1] <= 63));

		float total_quant_sse_mode[2] = { };
		float total_slam_to_line_sse_mode[2] = { };

		int mode_total_c[2][3] = { };
		int mode_low_c[2][3] = { }, mode_high_c[2][3] = { };
		int mode_total_r[2][3] = { }, mode_total_g[2][3] = { }, mode_total_b[2][3] = { };

		int spans[4] = { };

		for (uint32_t mode_iter = 0; mode_iter < 2; mode_iter++) // mode 0 vs. mode 2
		{
			if ((mode_iter) && (best_pat_indices[0] == best_pat_indices[1]))
			{
				for (uint32_t s = 0; s < 3; s++)
				{
					mode_total_c[1][s] = mode_total_c[0][s];

					mode_low_c[1][s] = mode_low_c[0][s];
					mode_high_c[1][s] = mode_high_c[0][s];

					mode_total_r[1][s] = mode_total_r[0][s];
					mode_total_g[1][s] = mode_total_g[0][s];
					mode_total_b[1][s] = mode_total_b[0][s];

					total_slam_to_line_sse_mode[1] = total_slam_to_line_sse_mode[0];

				} // subset
			}
			else
			{
				const uint32_t best_pat_index = best_pat_indices[mode_iter];
				const uint8_t* pBest_pat = &g_bc7_partition3[best_pat_index * 16];

				int* pTotal_r = &mode_total_r[mode_iter][0];
				int* pTotal_g = &mode_total_g[mode_iter][0];
				int* pTotal_b = &mode_total_b[mode_iter][0];

				int* pTotal_c = mode_total_c[mode_iter];

				for (uint32_t i = 0; i < 16; i++)
				{
					const int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b;
					const int subset = pBest_pat[i];

					pTotal_r[subset] += r; pTotal_g[subset] += g; pTotal_b[subset] += b;
					pTotal_c[subset]++;
				}

				int mean_r[3], mean_g[3], mean_b[3];
				for (uint32_t s = 0; s < 3; s++)
				{
					const uint32_t t = pTotal_c[s];
					const uint32_t h = (t >> 1);

					mean_r[s] = (pTotal_r[s] + h) / t;
					mean_g[s] = (pTotal_g[s] + h) / t;
					mean_b[s] = (pTotal_b[s] + h) / t;
				}

				int icov[3][6] = { };

				for (uint32_t i = 0; i < 16; i++)
				{
					const int subset = pBest_pat[i];

					int r = (int)pPixels[i].r - mean_r[subset];
					int g = (int)pPixels[i].g - mean_g[subset];
					int b = (int)pPixels[i].b - mean_b[subset];
					icov[subset][0] += r * r; icov[subset][1] += r * g; icov[subset][2] += r * b;
					icov[subset][3] += g * g; icov[subset][4] += g * b;
					icov[subset][5] += b * b;
				}

				int ar[3], ag[3], ab[3];

				float total_slam_to_line_sse = 0.0f;

				for (uint32_t s = 0; s < 3; s++)
				{
					int block_max_var = basisu::maximum(icov[s][0], icov[s][3], icov[s][5]);

					float cov[6];
					for (uint32_t i = 0; i < 6; i++)
						cov[i] = (float)icov[s][i];

					const float sc = 1.0f / ((float)block_max_var + .0000125f);
					const float wx = sc * cov[0], wy = sc * cov[3], wz = sc * cov[5];

					const float alt_xr = cov[0] * wx + cov[1] * wy + cov[2] * wz;
					const float alt_xg = cov[1] * wx + cov[3] * wy + cov[4] * wz;
					const float alt_xb = cov[2] * wx + cov[4] * wy + cov[5] * wz;

					total_slam_to_line_sse += estimate_slam_to_line_sse_3D(cov, alt_xr, alt_xg, alt_xb);

					int saxis_r = 306, saxis_g = 601, saxis_b = 117;

					float k = basisu::maximum(fabsf(alt_xr), fabsf(alt_xg), fabsf(alt_xb));
					if (fabs(k) >= basisu::SMALL_FLOAT_VAL)
					{
						float m = 2048.0f / k;
						saxis_r = (int)(alt_xr * m);
						saxis_g = (int)(alt_xg * m);
						saxis_b = (int)(alt_xb * m);
					}

					ar[s] = (int)((uint32_t)saxis_r << 4U);
					ag[s] = (int)((uint32_t)saxis_g << 4U);
					ab[s] = (int)((uint32_t)saxis_b << 4U);
				} // s

				total_slam_to_line_sse_mode[mode_iter] = total_slam_to_line_sse;

				int low_dot[3] = { INT_MAX, INT_MAX, INT_MAX };
				int high_dot[3] = { INT_MIN, INT_MIN, INT_MIN };

				for (uint32_t i = 0; i < 16; i++)
				{
					const int subset = pBest_pat[i];
					const int saxis_r = ar[subset], saxis_g = ag[subset], saxis_b = ab[subset];

					int dot = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b) + i;

					low_dot[subset] = basisu::minimum(low_dot[subset], dot);
					high_dot[subset] = basisu::maximum(high_dot[subset], dot);
				}

				for (uint32_t subset = 0; subset < 3; subset++)
				{
					mode_low_c[mode_iter][subset] = low_dot[subset] & 15;
					mode_high_c[mode_iter][subset] = high_dot[subset] & 15;
				} // subset

			} // if ((mode_iter) && (best_pat_indices[0] == best_pat_indices[1]))

			for (uint32_t subset = 0; subset < 3; subset++)
			{
				const uint32_t low_pixel = mode_low_c[mode_iter][subset];
				const uint32_t high_pixel = mode_high_c[mode_iter][subset];

				for (uint32_t c = 0; c < 3; c++)
					spans[c] = pPixels[high_pixel][c] - pPixels[low_pixel][c];

				float subset_sse;
				if (mode_iter == 0)
				{
					// mode 0: 4-bit endpoints, unique p-bits, 3-bit weights, slight p-bit endpoint scale factor
					subset_sse = analytical_quant_est_sse(16, 8, 3, spans, nullptr, UNIQUE_PBIT_DISCOUNT, mode_total_c[mode_iter][subset]);
				}
				else
				{
					// mode 2: 5-bit endpoints, no p-bits, 2-bit weights, no endpoint scale factor
					subset_sse = analytical_quant_est_sse(32, 4, 3, spans, nullptr, 1.0f, mode_total_c[mode_iter][subset]);
				}

				total_quant_sse_mode[mode_iter] += subset_sse;
			} // subset

		} // mode_iter

		const float total_sse_est_mode0 = total_quant_sse_mode[0] + total_slam_to_line_sse_mode[0];
		const float total_sse_est_mode2 = total_quant_sse_mode[1] + total_slam_to_line_sse_mode[1];

		if (total_sse_est_mode0 < total_sse_est_mode2)
		{
			if (pFinal_sse_est)
				*pFinal_sse_est = total_sse_est_mode0;

			// Use mode 0 (high span)
			if (total_sse_est_mode0 >= sse_est_to_beat)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_mode02_bailouts++;
#endif
				return false;
			}

			const uint32_t best_pat_index = best_pat_indices[0];
			const uint8_t* pBest_pat = &g_bc7_partition3[best_pat_index * 16];

			const int* pLow_c = &mode_low_c[0][0];
			const int* pHigh_c = &mode_high_c[0][0];

			const int* pTotal_r = &mode_total_r[0][0];
			const int* pTotal_g = &mode_total_g[0][0];
			const int* pTotal_b = &mode_total_b[0][0];

			float xl[3][4], xh[3][4];

			for (uint32_t s = 0; s < 3; s++)
			{
				const int lc = pLow_c[s];
				const int hc = pHigh_c[s];

				xl[s][0] = (float)pPixels[lc].r * (1.0f / 255.0f);
				xl[s][1] = (float)pPixels[lc].g * (1.0f / 255.0f);
				xl[s][2] = (float)pPixels[lc].b * (1.0f / 255.0f);
				xl[s][3] = 0.0f;

				xh[s][0] = (float)pPixels[hc].r * (1.0f / 255.0f);
				xh[s][1] = (float)pPixels[hc].g * (1.0f / 255.0f);
				xh[s][2] = (float)pPixels[hc].b * (1.0f / 255.0f);
				xh[s][3] = 0.0f;
			} // s

			uint32_t lr[3], lg[3], lb[3], hr[3], hg[3], hb[3], pbits[6];

			for (uint32_t s = 0; s < 3; s++)
			{
				color_rgba el, eh;
				determine_unique_pbits(3, 4, xl[s], xh[s], el, eh, &pbits[s << 1]);

				lr[s] = el[0]; lg[s] = el[1]; lb[s] = el[2];
				hr[s] = eh[0]; hg[s] = eh[1]; hb[s] = eh[2];

			} // s

			uint8_t cur_weights[16];
			eval_weights_mode0_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_index);

			float z00[3] = { 0.0f }, z10[3] = { 0.0f }, z11[3] = { 0.0f };
			float q00_r[3] = { 0.0f };
			float q00_g[3] = { 0.0f };
			float q00_b[3] = { 0.0f };

			for (uint32_t i = 0; i < 16; i++)
			{
				const int subset = pBest_pat[i];
				const uint32_t sel = cur_weights[i];
				assert(sel <= 7);

				z00[subset] += g_bc7_3bit_ls_tab[sel][0];
				z10[subset] += g_bc7_3bit_ls_tab[sel][1];
				z11[subset] += g_bc7_3bit_ls_tab[sel][2];

				const float w = g_bc7_3bit_ls_tab[sel][3];

				q00_r[subset] += w * (float)pPixels[i][0];
				q00_g[subset] += w * (float)pPixels[i][1];
				q00_b[subset] += w * (float)pPixels[i][2];
			} // i

			for (uint32_t s = 0; s < 3; s++)
			{
				float q10_r = (float)pTotal_r[s] - q00_r[s];
				float q10_g = (float)pTotal_g[s] - q00_g[s];
				float q10_b = (float)pTotal_b[s] - q00_b[s];

				float z01 = z10[s];

				float det = z00[s] * z11[s] - z01 * z10[s];
				if (fabs(det) < 1e-8f)
					continue;

				det = 1.0f / det;

				float iz00, iz01, iz10, iz11;
				iz00 = z11[s] * det;
				iz01 = -z01 * det;
				iz10 = -z10[s] * det;
				iz11 = z00[s] * det;

				const float q = 1.0f / 255.0f;

				xl[s][0] = basisu::clamp(q * (iz10 * q00_r[s] + iz11 * q10_r), 0.0f, 1.0f);
				xh[s][0] = basisu::clamp(q * (iz00 * q00_r[s] + iz01 * q10_r), 0.0f, 1.0f);

				xl[s][1] = basisu::clamp(q * (iz10 * q00_g[s] + iz11 * q10_g), 0.0f, 1.0f);
				xh[s][1] = basisu::clamp(q * (iz00 * q00_g[s] + iz01 * q10_g), 0.0f, 1.0f);

				xl[s][2] = basisu::clamp(q * (iz10 * q00_b[s] + iz11 * q10_b), 0.0f, 1.0f);
				xh[s][2] = basisu::clamp(q * (iz00 * q00_b[s] + iz01 * q10_b), 0.0f, 1.0f);
			} // s

			for (uint32_t s = 0; s < 3; s++)
			{
				color_rgba el, eh;
				determine_unique_pbits(3, 4, xl[s], xh[s], el, eh, &pbits[s << 1]); // fills in both pbit entries

				lr[s] = el[0]; lg[s] = el[1]; lb[s] = el[2];
				hr[s] = eh[0]; hg[s] = eh[1]; hb[s] = eh[2];

			} // s

			if (pActual_sse)
				*pActual_sse = eval_weights_mode0_rgb_sse(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_index);
			else
				eval_weights_mode0_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, pbits, best_pat_index);

			encode_mode0_rgb_block(pBlock, best_pat_index, lr, lg, lb, hr, hg, hb, pbits, cur_weights);
		}
		else
		{
			if (pFinal_sse_est)
				*pFinal_sse_est = total_sse_est_mode2;

			// Use mode 2 (low span)
			if (total_sse_est_mode2 >= sse_est_to_beat)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_mode02_bailouts++;
#endif
				return false;
			}

			const uint32_t best_pat_index = best_pat_indices[1];
			const uint8_t* pBest_pat = &g_bc7_partition3[best_pat_index * 16];

			const int* pLow_c = &mode_low_c[1][0];
			const int* pHigh_c = &mode_high_c[1][0];

			const int* pTotal_r = &mode_total_r[1][0];
			const int* pTotal_g = &mode_total_g[1][0];
			const int* pTotal_b = &mode_total_b[1][0];

			uint32_t lr[3], lg[3], lb[3];
			uint32_t hr[3], hg[3], hb[3];

			for (uint32_t s = 0; s < 3; s++)
			{
				const int lc = pLow_c[s];
				const int hc = pHigh_c[s];

				lr[s] = to_5(pPixels[lc].r);
				lg[s] = to_5(pPixels[lc].g);
				lb[s] = to_5(pPixels[lc].b);

				hr[s] = to_5(pPixels[hc].r);
				hg[s] = to_5(pPixels[hc].g);
				hb[s] = to_5(pPixels[hc].b);
			}

			uint8_t cur_weights[16];
			eval_weights_mode2_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, best_pat_index);

			float z00[3] = { 0.0f }, z10[3] = { 0.0f }, z11[3] = { 0.0f };
			float q00_r[3] = { 0.0f };
			float q00_g[3] = { 0.0f };
			float q00_b[3] = { 0.0f };

			for (uint32_t i = 0; i < 16; i++)
			{
				const int subset = pBest_pat[i];
				const uint32_t sel = cur_weights[i];
				assert(sel <= 3);

				z00[subset] += g_bc7_2bit_ls_tab[sel][0];
				z10[subset] += g_bc7_2bit_ls_tab[sel][1];
				z11[subset] += g_bc7_2bit_ls_tab[sel][2];

				const float w = g_bc7_2bit_ls_tab[sel][3];

				q00_r[subset] += w * (float)pPixels[i][0];
				q00_g[subset] += w * (float)pPixels[i][1];
				q00_b[subset] += w * (float)pPixels[i][2];
			} // i

			for (uint32_t s = 0; s < 3; s++)
			{
				float q10_r = (float)pTotal_r[s] - q00_r[s];
				float q10_g = (float)pTotal_g[s] - q00_g[s];
				float q10_b = (float)pTotal_b[s] - q00_b[s];

				float z01 = z10[s];

				float det = z00[s] * z11[s] - z01 * z10[s];
				if (fabs(det) < 1e-8f)
					continue;

				det = 1.0f / det;

				float iz00, iz01, iz10, iz11;
				iz00 = z11[s] * det;
				iz01 = -z01 * det;
				iz10 = -z10[s] * det;
				iz11 = z00[s] * det;

				hr[s] = to_5_clamp(iz00 * q00_r[s] + iz01 * q10_r);
				lr[s] = to_5_clamp(iz10 * q00_r[s] + iz11 * q10_r);

				hg[s] = to_5_clamp(iz00 * q00_g[s] + iz01 * q10_g);
				lg[s] = to_5_clamp(iz10 * q00_g[s] + iz11 * q10_g);

				hb[s] = to_5_clamp(iz00 * q00_b[s] + iz01 * q10_b);
				lb[s] = to_5_clamp(iz10 * q00_b[s] + iz11 * q10_b);
			} // s

			if (pActual_sse)
				*pActual_sse = eval_weights_mode2_rgb_sse(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, best_pat_index);
			else
				eval_weights_mode2_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, best_pat_index);

			encode_mode2_rgb_block(pBlock, best_pat_index,
				lr, lg, lb, hr, hg, hb, cur_weights);
		}

#ifdef _DEBUG
		if (pActual_sse)
		{
			const uint32_t expected_sse = calc_sse(pBlock, pPixels);
			assert(expected_sse == *pActual_sse);
		}
#endif

		return true;
	}

	bool pack_mode4_or_5(uint8_t* pBlock, const color_rgba* pOrig_pixels, uint32_t dp_chan_index, float sse_est_to_beat, uint32_t flags,
		float* pFinal_sse_est = nullptr,
		uint32_t* pActual_sse = nullptr)
	{
		(void)flags;

#if BASISU_BC7F_PERF_STATS
		g_total_mode45_evals++;
#endif

		color_rgba pixels[16];
		const color_rgba* pPixels = pOrig_pixels;

		if (dp_chan_index != 3)
		{
			memcpy(pixels, pOrig_pixels, sizeof(color_rgba) * 16);
			pPixels = pixels;

			for (uint32_t i = 0; i < 16; i++)
			{
				const uint8_t c = pixels[i][dp_chan_index];
				pixels[i][dp_chan_index] = pixels[i][3];
				pixels[i][3] = c;
			}
		}

		int total_r = 0, total_g = 0, total_b = 0, total_a = 0;

		int min_r = 255, min_g = 255, min_b = 255, min_a = 255;
		int max_r = 0, max_g = 0, max_b = 0, max_a = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b, a = pPixels[i].a;

			total_r += r; total_g += g; total_b += b; total_a += a;

			min_r = basisu::minimum(min_r, r); min_g = basisu::minimum(min_g, g); min_b = basisu::minimum(min_b, b); min_a = basisu::minimum(min_a, a);
			max_r = basisu::maximum(max_r, r); max_g = basisu::maximum(max_g, g); max_b = basisu::maximum(max_b, b); max_a = basisu::maximum(max_a, a);
		}

		int mean_r = (total_r + 8) >> 4, mean_g = (total_g + 8) >> 4, mean_b = (total_b + 8) >> 4;

		// covar rows are:
		// 0, 1, 2
		// 1, 3, 4
		// 2, 4, 5
		int icov[6] = { 0, 0, 0, 0, 0, 0 };

		for (uint32_t i = 0; i < 16; i++)
		{
			const int r = (int)pPixels[i].r - mean_r;
			const int g = (int)pPixels[i].g - mean_g;
			const int b = (int)pPixels[i].b - mean_b;
			icov[0] += r * r; icov[1] += r * g; icov[2] += r * b;
			icov[3] += g * g; icov[4] += g * b;
			icov[5] += b * b;
		}

		float cov3[6];
		for (uint32_t i = 0; i < 6; i++)
			cov3[i] = (float)icov[i];

		const int block_max_var3 = basisu::maximum(icov[0], icov[3], icov[5]); // not divided by 16, i.e. scaled by 16

		const float sc3 = block_max_var3 ? (1.0f / (float)block_max_var3) : 0;
		const float wx3 = sc3 * cov3[0], wy3 = sc3 * cov3[3], wz3 = sc3 * cov3[5];

		const float alt_xr = cov3[0] * wx3 + cov3[1] * wy3 + cov3[2] * wz3;
		const float alt_xg = cov3[1] * wx3 + cov3[3] * wy3 + cov3[4] * wz3;
		const float alt_xb = cov3[2] * wx3 + cov3[4] * wy3 + cov3[5] * wz3;

		// Same for mode 4/5
		const float rgb_slam_to_line_sse_est = estimate_slam_to_line_sse_3D(cov3, alt_xr, alt_xg, alt_xb);

		int saxis_r = 306, saxis_g = 601, saxis_b = 117;

		float k = basisu::maximum(fabsf(alt_xr), fabsf(alt_xg), fabsf(alt_xb));
		if (fabs(k) >= basisu::SMALL_FLOAT_VAL)
		{
			float m = 2048.0f / k;
			saxis_r = (int)(alt_xr * m);
			saxis_g = (int)(alt_xg * m);
			saxis_b = (int)(alt_xb * m);
		}

		saxis_r = (int)((uint32_t)saxis_r << 4U);
		saxis_g = (int)((uint32_t)saxis_g << 4U);
		saxis_b = (int)((uint32_t)saxis_b << 4U);

		int low_dot = INT_MAX, high_dot = INT_MIN;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			int dot0 = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b) + i;
			int dot1 = (pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b) + i + 1;
			int dot2 = (pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b) + i + 2;
			int dot3 = (pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b) + i + 3;

			int min_d01 = basisu::minimum(dot0, dot1);
			int max_d01 = basisu::maximum(dot0, dot1);

			int min_d23 = basisu::minimum(dot2, dot3);
			int max_d23 = basisu::maximum(dot2, dot3);

			int min_d = basisu::minimum(min_d01, min_d23);
			int max_d = basisu::maximum(max_d01, max_d23);

			low_dot = basisu::minimum(low_dot, min_d);
			high_dot = basisu::maximum(high_dot, max_d);
		}

		const int low_c = low_dot & 15;
		const int high_c = high_dot & 15;

		const int rgb_spans[4] = { pPixels[high_c][0] - pPixels[low_c][0], pPixels[high_c][1] - pPixels[low_c][1], pPixels[high_c][2] - pPixels[low_c][2], 0 };
		const int a_span = max_a - min_a;

		const float SECOND_PLANE_SPAN_WEIGHT = (dp_chan_index == 3) ? 1.0f : 1.0f;

		const float mode_4_rgb_3bit_quant_sse_est = analytical_quant_est_sse(32, 8, 3, rgb_spans, nullptr, 1.0f, 16); // mode 4 rgb: 5-bit endpoints, using 3-bit weights for RGB
		const float mode_4_a_2bit_quant_sse_est = analytical_quant_est_sse(64, 4, a_span, SECOND_PLANE_SPAN_WEIGHT, 1.0f, 16); // mode 4 a: 6-bit endpoints, using 2-bit weights for RGB

		const float mode_4_rgb_2bit_quant_sse_est = analytical_quant_est_sse(32, 4, 3, rgb_spans, nullptr, 1.0f, 16); // mode 4 rgb: 5-bit endpoints, using 2-bit weights for RGB
		const float mode_4_a_3bit_quant_sse_est = analytical_quant_est_sse(64, 8, a_span, SECOND_PLANE_SPAN_WEIGHT, 1.0f, 16); // mode 4 a: 6-bit endpoints, using 3-bit weights for RGB

		const float total_mode_4_rgb3_a2_sse_est = rgb_slam_to_line_sse_est + mode_4_rgb_3bit_quant_sse_est + mode_4_a_2bit_quant_sse_est;
		const float total_mode_4_rgb2_a3_sse_est = rgb_slam_to_line_sse_est + mode_4_rgb_2bit_quant_sse_est + mode_4_a_3bit_quant_sse_est;

		const float mode_5_rgb_quant_sse_est = analytical_quant_est_sse(128, 4, 3, rgb_spans, nullptr, 1.0f, 16); // mode 5 rgb: 7-bit endpoints, using 2-bit weights for RGB
		const float mode_5_a_quant_sse_est = analytical_quant_est_sse(256, 4, a_span, SECOND_PLANE_SPAN_WEIGHT, 1.0f, 16); // mode 5 a: 8-bit endpoints, using 2-bit weights for RGB
		const float total_mode_5_rgba_sse_est = rgb_slam_to_line_sse_est + mode_5_rgb_quant_sse_est + mode_5_a_quant_sse_est;

		if (total_mode_5_rgba_sse_est < basisu::minimum(total_mode_4_rgb3_a2_sse_est, total_mode_4_rgb2_a3_sse_est))
		{
			if (pFinal_sse_est)
				*pFinal_sse_est = total_mode_5_rgba_sse_est;

			// Mode 5 - low RGB/A span
			if (total_mode_5_rgba_sse_est >= sse_est_to_beat)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_mode45_bailouts++;
#endif
				return false;
			}

			int lr = to_7(pPixels[low_c].r), lg = to_7(pPixels[low_c].g), lb = to_7(pPixels[low_c].b), la = min_a;
			int hr = to_7(pPixels[high_c].r), hg = to_7(pPixels[high_c].g), hb = to_7(pPixels[high_c].b), ha = max_a;

			uint8_t cur_weights0[16]; // rgb 2-bits
			if (pActual_sse)
				*pActual_sse = eval_weights_mode5_2bit_rgb_sse(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
			else
				eval_weights_mode5_2bit_rgb(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);

			vec4F xl, xh;
			bool res = compute_least_squares_endpoints_3D(
				16, cur_weights0, 4,
				g_bc7_2bit_ls_tab,
				xl, xh,
				pPixels,
				(float)total_r, (float)total_g, (float)total_b);

			if (res)
			{
				lr = fast_roundf_int(xl[0] * (127.0f / 255.0f));
				lg = fast_roundf_int(xl[1] * (127.0f / 255.0f));
				lb = fast_roundf_int(xl[2] * (127.0f / 255.0f));

				hr = fast_roundf_int(xh[0] * (127.0f / 255.0f));
				hg = fast_roundf_int(xh[1] * (127.0f / 255.0f));
				hb = fast_roundf_int(xh[2] * (127.0f / 255.0f));

				if (pActual_sse)
					*pActual_sse = eval_weights_mode5_2bit_rgb_sse(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
				else
					eval_weights_mode5_2bit_rgb(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
			}

			uint8_t cur_weights1[16]; // alpha 2-bits
			uint32_t a_sse = 0;
			if (pActual_sse)
				a_sse = eval_weights_mode5_2bit_a_sse(pPixels, cur_weights1, la, ha);
			else
				eval_weights_mode5_2bit_a(pPixels, cur_weights1, la, ha);

			float nal, nah;
			if (compute_least_squares_endpoints_1D(
				16, cur_weights1, 4,
				g_bc7_2bit_ls_tab,
				nal, nah,
				pPixels, 3,
				(float)total_a))
			{
				la = fast_roundf_int(nal);
				ha = fast_roundf_int(nah);

				if (pActual_sse)
					a_sse = eval_weights_mode5_2bit_a_sse(pPixels, cur_weights1, la, ha);
				else
					eval_weights_mode5_2bit_a(pPixels, cur_weights1, la, ha);
			}

			if (pActual_sse)
				*pActual_sse += a_sse;

			encode_mode5_rgba_block(pBlock,
				lr, lg, lb, la,
				hr, hg, hb, ha,
				cur_weights0, cur_weights1, (dp_chan_index + 1) & 3);
		}
		else if (total_mode_4_rgb3_a2_sse_est < total_mode_4_rgb2_a3_sse_est)
		{
			if (pFinal_sse_est)
				*pFinal_sse_est = total_mode_4_rgb3_a2_sse_est;

			// mode 4, rgb 3-bits, alpha 2-bits - high span RGB, low span in A, index bit=1
			if (total_mode_4_rgb3_a2_sse_est >= sse_est_to_beat)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_mode45_bailouts++;
#endif
				return false;
			}

			int lr = to_5(pPixels[low_c].r), lg = to_5(pPixels[low_c].g), lb = to_5(pPixels[low_c].b), la = to_6(min_a);
			int hr = to_5(pPixels[high_c].r), hg = to_5(pPixels[high_c].g), hb = to_5(pPixels[high_c].b), ha = to_6(max_a);

			uint8_t cur_weights0[16]; // rgb 3-bits
			if (pActual_sse)
				*pActual_sse = eval_weights_mode4_3bit_rgb_sse(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
			else
				eval_weights_mode4_3bit_rgb(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);

			vec4F xl, xh;
			bool res = compute_least_squares_endpoints_3D(
				16, cur_weights0, 8,
				g_bc7_3bit_ls_tab,
				xl, xh,
				pPixels,
				(float)total_r, (float)total_g, (float)total_b);

			if (res)
			{
				lr = fast_roundf_int(xl[0] * (31.0f / 255.0f));
				lg = fast_roundf_int(xl[1] * (31.0f / 255.0f));
				lb = fast_roundf_int(xl[2] * (31.0f / 255.0f));

				hr = fast_roundf_int(xh[0] * (31.0f / 255.0f));
				hg = fast_roundf_int(xh[1] * (31.0f / 255.0f));
				hb = fast_roundf_int(xh[2] * (31.0f / 255.0f));

				if (pActual_sse)
					*pActual_sse = eval_weights_mode4_3bit_rgb_sse(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
				else
					eval_weights_mode4_3bit_rgb(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
			}

			uint8_t cur_weights1[16]; // alpha 2-bits

			uint32_t a_sse = 0;
			if (pActual_sse)
				a_sse = eval_weights_mode4_2bit_a_sse(pPixels, cur_weights1, la, ha);
			else
				eval_weights_mode4_2bit_a(pPixels, cur_weights1, la, ha);

			float nal, nah;
			if (compute_least_squares_endpoints_1D(
				16, cur_weights1, 4,
				g_bc7_2bit_ls_tab,
				nal, nah,
				pPixels, 3,
				(float)total_a))
			{
				la = fast_roundf_int(nal * (63.0f / 255.0f));
				ha = fast_roundf_int(nah * (63.0f / 255.0f));

				if (pActual_sse)
					a_sse = eval_weights_mode4_2bit_a_sse(pPixels, cur_weights1, la, ha);
				else
					eval_weights_mode4_2bit_a(pPixels, cur_weights1, la, ha);
			}

			if (pActual_sse)
				*pActual_sse += a_sse;

			encode_mode4_rgba_block(pBlock,
				lr, lg, lb, la,
				hr, hg, hb, ha,
				cur_weights0, cur_weights1, (dp_chan_index + 1) & 3, 1);
		}
		else
		{
			if (pFinal_sse_est)
				*pFinal_sse_est = total_mode_4_rgb2_a3_sse_est;

			// mode 4, rgb 2-bits, alpha 3-bits - low span RGB, high span in A, index bit=0
			if (total_mode_4_rgb2_a3_sse_est >= sse_est_to_beat)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_mode45_bailouts++;
#endif
				return false;
			}

			int lr = to_5(pPixels[low_c].r), lg = to_5(pPixels[low_c].g), lb = to_5(pPixels[low_c].b), la = to_6(min_a);
			int hr = to_5(pPixels[high_c].r), hg = to_5(pPixels[high_c].g), hb = to_5(pPixels[high_c].b), ha = to_6(max_a);

			uint8_t cur_weights0[16]; // rgb 2-bits
			if (pActual_sse)
				*pActual_sse = eval_weights_mode4_2bit_rgb_sse(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
			else
				eval_weights_mode4_2bit_rgb(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);

			vec4F xl, xh;
			bool res = compute_least_squares_endpoints_3D(
				16, cur_weights0, 4,
				g_bc7_2bit_ls_tab,
				xl, xh,
				pPixels,
				(float)total_r, (float)total_g, (float)total_b);

			if (res)
			{
				lr = fast_roundf_int(xl[0] * (31.0f / 255.0f));
				lg = fast_roundf_int(xl[1] * (31.0f / 255.0f));
				lb = fast_roundf_int(xl[2] * (31.0f / 255.0f));

				hr = fast_roundf_int(xh[0] * (31.0f / 255.0f));
				hg = fast_roundf_int(xh[1] * (31.0f / 255.0f));
				hb = fast_roundf_int(xh[2] * (31.0f / 255.0f));

				if (pActual_sse)
					*pActual_sse = eval_weights_mode4_2bit_rgb_sse(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
				else
					eval_weights_mode4_2bit_rgb(pPixels, cur_weights0, lr, lg, lb, hr, hg, hb);
			}

			uint8_t cur_weights1[16]; // alpha 2-bits
			uint32_t a_sse = 0;
			if (pActual_sse)
				a_sse = eval_weights_mode4_3bit_a_sse(pPixels, cur_weights1, la, ha);
			else
				eval_weights_mode4_3bit_a(pPixels, cur_weights1, la, ha);

			float nal, nah;
			if (compute_least_squares_endpoints_1D(
				16, cur_weights1, 8,
				g_bc7_3bit_ls_tab,
				nal, nah,
				pPixels, 3,
				(float)total_a))
			{
				la = fast_roundf_int(nal * (63.0f / 255.0f));
				ha = fast_roundf_int(nah * (63.0f / 255.0f));

				if (pActual_sse)
					a_sse = eval_weights_mode4_3bit_a_sse(pPixels, cur_weights1, la, ha);
				else
					eval_weights_mode4_3bit_a(pPixels, cur_weights1, la, ha);
			}

			if (pActual_sse)
				*pActual_sse += a_sse;

			encode_mode4_rgba_block(pBlock,
				lr, lg, lb, la,
				hr, hg, hb, ha,
				cur_weights0, cur_weights1, (dp_chan_index + 1) & 3, 0);
		}

#ifdef _DEBUG
		if (pActual_sse)
		{
			const uint32_t expected_sse = calc_sse(pBlock, pOrig_pixels);
			assert(expected_sse == *pActual_sse);
		}
#endif

		return true;
	}

	bool pack_mode7_rgba(uint8_t* pBlock, const color_rgba* pPixels,
		float block_xr, float block_xg, float block_xb, float block_xa,
		int block_mean_r, int block_mean_g, int block_mean_b, int block_mean_a,
		float sse_est_to_beat, uint32_t flags,
		float* pFinal_sse_est = nullptr,
		uint32_t* pActual_sse = nullptr)
	{
#if BASISU_BC7F_PERF_STATS
		g_total_mode7_evals++;
#endif

		uint32_t desired_pat_bits = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			const float r = (float)(pPixels[i].r - block_mean_r);
			const float g = (float)(pPixels[i].g - block_mean_g);
			const float b = (float)(pPixels[i].b - block_mean_b);
			const float a = (float)(pPixels[i].a - block_mean_a);

			const uint32_t subset = (r * block_xr + g * block_xg + b * block_xb + a * block_xa) > 0.0f;

			desired_pat_bits |= (subset << i);
		}

		uint32_t best_diff = UINT32_MAX;
		if (flags & cPackBC7FlagASTCCompatible)
		{
			// We're actually encoding to ASTC 4x4, so we can only check the 2-subset patterns in common between BC7 and ASTC (the UASTC LDR 4x4 patterns).
			for (uint32_t c = 0; c < TOTAL_ASTC_BC7_COMMON_PARTITIONS2; c++)
			{
				const uint32_t p = g_astc_bc7_common_partitions2[c].m_bc7;

				const uint32_t bc6h_pat_bits = g_bc7_part2_bitmasks[p];

				int diff = popcount32(bc6h_pat_bits ^ desired_pat_bits);
				int diff_inv = 16 - diff;

				uint32_t min_diff = (basisu::minimum<int>(diff, diff_inv) << 8) | p;
				if (min_diff < best_diff)
					best_diff = min_diff;
			} // c
		}
		else
		{
			for (uint32_t p = 0; p < MAX_PATTERNS2_TO_CHECK; p++)
			{
				const uint32_t bc6h_pat_bits = g_bc7_part2_bitmasks[p];

				int diff = popcount32(bc6h_pat_bits ^ desired_pat_bits);
				int diff_inv = 16 - diff;

				uint32_t min_diff = (basisu::minimum<int>(diff, diff_inv) << 8) | p;
				if (min_diff < best_diff)
					best_diff = min_diff;
			} // p
		}

		const uint32_t best_pat_index = best_diff & 0xFF;
		const uint32_t best_pat_bits = g_bc7_part2_bitmasks[best_pat_index];

		int total_r[2] = { }, total_g[2] = { }, total_b[2] = { }, total_a[2] = { }, total_c[2] = { };
		for (uint32_t i = 0; i < 16; i++)
		{
			const int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b, a = pPixels[i].a;
			const int subset = (best_pat_bits >> i) & 1;

			total_r[subset] += r; total_g[subset] += g; total_b[subset] += b; total_a[subset] += a;
			total_c[subset]++;
		}

		int mean_r[2], mean_g[2], mean_b[2], mean_a[2];
		for (uint32_t s = 0; s < 2; s++)
		{
			const uint32_t t = total_c[s];
			const uint32_t h = (t >> 1);

			mean_r[s] = (total_r[s] + h) / t;
			mean_g[s] = (total_g[s] + h) / t;
			mean_b[s] = (total_b[s] + h) / t;
			mean_a[s] = (total_a[s] + h) / t;
		}

		int icov4[2][10] = { { }, { } };

		// 0=rr
		// 1=rg
		// 2=rb
		// 3=ra
		// 
		// 4=gg
		// 5=gb
		// 6=ga
		// 
		// 7=bb
		// 8=ba
		// 
		// 9=aa

		// 0 1 2 3
		//   4 5 6
		//     7 8
		//       9

		// 0 1 2 3
		// 1 4 5 6
		// 2 5 7 8
		// 3 6 8 9

		// trace at 0,4,7,9

		for (uint32_t i = 0; i < 16; i++)
		{
			const int s = (best_pat_bits >> i) & 1;

			int r = (int)pPixels[i].r - mean_r[s];
			int g = (int)pPixels[i].g - mean_g[s];
			int b = (int)pPixels[i].b - mean_b[s];
			int a = (int)pPixels[i].a - mean_a[s];

			icov4[s][0] += r * r; icov4[s][1] += r * g; icov4[s][2] += r * b; icov4[s][3] += r * a;
			icov4[s][4] += g * g; icov4[s][5] += g * b; icov4[s][6] += g * a;
			icov4[s][7] += b * b; icov4[s][8] += b * a;
			icov4[s][9] += a * a;
		}

		int ar[2], ag[2], ab[2], aa[2];

		float slam_to_line_sse_est = 0.0f;

		for (uint32_t s = 0; s < 2; s++)
		{
			const int block_max_var4 = basisu::maximum(icov4[s][0], icov4[s][4], icov4[s][7], icov4[s][9]);

			float cov4[10];
			for (uint32_t i = 0; i < 10; i++)
				cov4[i] = (float)icov4[s][i];

			const float sc4 = block_max_var4 ? (1.0f / (float)block_max_var4) : 0;
			const float wx = sc4 * cov4[0], wy = sc4 * cov4[4], wz = sc4 * cov4[7], wa = sc4 * cov4[9];

			// 0 1 2 3
			// 1 4 5 6
			// 2 5 7 8
			// 3 6 8 9

			const float x0 = cov4[0] * wx + cov4[1] * wy + cov4[2] * wz + cov4[3] * wa;
			const float y0 = cov4[1] * wx + cov4[4] * wy + cov4[5] * wz + cov4[6] * wa;
			const float z0 = cov4[2] * wx + cov4[5] * wy + cov4[7] * wz + cov4[8] * wa;
			const float w0 = cov4[3] * wx + cov4[6] * wy + cov4[8] * wz + cov4[9] * wa;

			const float x1 = cov4[0] * x0 + cov4[1] * y0 + cov4[2] * z0 + cov4[3] * w0;
			const float y1 = cov4[1] * x0 + cov4[4] * y0 + cov4[5] * z0 + cov4[6] * w0;
			const float z1 = cov4[2] * x0 + cov4[5] * y0 + cov4[7] * z0 + cov4[8] * w0;
			const float w1 = cov4[3] * x0 + cov4[6] * y0 + cov4[8] * z0 + cov4[9] * w0;

			slam_to_line_sse_est += estimate_slam_to_line_sse_4D(cov4, x1, y1, z1, w1);

			int saxis_r = 256, saxis_g = 256, saxis_b = 256, saxis_a = 256;

			float k = basisu::maximum(fabsf(x1), fabsf(y1), fabsf(z1), fabsf(w1));
			if (fabsf(k) >= basisu::SMALL_FLOAT_VAL)
			{
				float m = 2048.0f / k;
				saxis_r = (int)(x1 * m);
				saxis_g = (int)(y1 * m);
				saxis_b = (int)(z1 * m);
				saxis_a = (int)(w1 * m);
			}

			ar[s] = (int)((uint32_t)saxis_r << 4U);
			ag[s] = (int)((uint32_t)saxis_g << 4U);
			ab[s] = (int)((uint32_t)saxis_b << 4U);
			aa[s] = (int)((uint32_t)saxis_a << 4U);
		} // s

		int low_dot[2] = { INT_MAX, INT_MAX };
		int high_dot[2] = { INT_MIN, INT_MIN };

		for (uint32_t i = 0; i < 16; i++)
		{
			const int subset = (best_pat_bits >> i) & 1;
			const int saxis_r = ar[subset], saxis_g = ag[subset], saxis_b = ab[subset], saxis_a = aa[subset];

			assert(((pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b + pPixels[i].a * saxis_a) & 0xF) == 0); // sanity
			const int dot = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b + pPixels[i].a * saxis_a) + i;

			low_dot[subset] = basisu::minimum(low_dot[subset], dot);
			high_dot[subset] = basisu::maximum(high_dot[subset], dot);
		}

		int low_c[2] = { low_dot[0] & 15, low_dot[1] & 15 };
		int high_c[2] = { high_dot[0] & 15, high_dot[1] & 15 };

		float quant_err_sse_est = 0;

		for (uint32_t subset = 0; subset < 2; subset++)
		{
			const uint32_t low_pixel = low_c[subset];
			const uint32_t high_pixel = high_c[subset];

			int spans[4];
			for (uint32_t c = 0; c < 4; c++)
				spans[c] = pPixels[high_pixel][c] - pPixels[low_pixel][c];

			// mode 7: 5-bit endpoints, unique pbits, 2 bit weights, 4 chans
			quant_err_sse_est += analytical_quant_est_sse(32, 4, 4, spans, nullptr, (flags & cPackBC7FlagPBitOpt) ? UNIQUE_PBIT_DISCOUNT : 1.0f, total_c[subset]);

		} // subset

		const float total_mode7_est_sse = slam_to_line_sse_est + quant_err_sse_est;

		if (pFinal_sse_est)
			*pFinal_sse_est = total_mode7_est_sse;

		if (total_mode7_est_sse >= sse_est_to_beat)
		{
#if BASISU_BC7F_PERF_STATS
			g_total_mode7_bailouts++;
#endif
			return false;
		}

		uint32_t lr[2], lg[2], lb[2], la[2];
		uint32_t hr[2], hg[2], hb[2], ha[2];
		uint32_t pbits[4];

		for (uint32_t s = 0; s < 2; s++)
		{
			const int lc = low_c[s], hc = high_c[s];

			if (flags & cPackBC7FlagPBitOpt)
			{
				const float q = 1.0f / 255.0f;
				float sxl[4] = { (float)pPixels[lc].r * q, (float)pPixels[lc].g * q, (float)pPixels[lc].b * q, (float)pPixels[lc].a * q };
				float sxh[4] = { (float)pPixels[hc].r * q, (float)pPixels[hc].g * q, (float)pPixels[hc].b * q, (float)pPixels[hc].a * q };

				color_rgba bestMinColor, bestMaxColor;
				determine_unique_pbits(4, 5, sxl, sxh, bestMinColor, bestMaxColor, &pbits[s * 2]);

				lr[s] = bestMinColor.r, lg[s] = bestMinColor.g, lb[s] = bestMinColor.b; la[s] = bestMinColor.a;
				hr[s] = bestMaxColor.r, hg[s] = bestMaxColor.g, hb[s] = bestMaxColor.b; ha[s] = bestMaxColor.a;
			}
			else
			{
				const uint32_t l_pbit = (pPixels[lc].a >= 129);
				const uint32_t h_pbit = (pPixels[hc].a >= 129);

				pbits[s * 2 + 0] = l_pbit;
				pbits[s * 2 + 1] = h_pbit;

				lr[s] = to_5(pPixels[lc].r, l_pbit);
				lg[s] = to_5(pPixels[lc].g, l_pbit);
				lb[s] = to_5(pPixels[lc].b, l_pbit);
				la[s] = to_5(pPixels[lc].a, l_pbit);

				hr[s] = to_5(pPixels[hc].r, h_pbit);
				hg[s] = to_5(pPixels[hc].g, h_pbit);
				hb[s] = to_5(pPixels[hc].b, h_pbit);
				ha[s] = to_5(pPixels[hc].a, h_pbit);
			}
		} // s

		uint8_t cur_weights[16];

		eval_weights_mode7_rgba(pPixels, cur_weights,
			lr, lg, lb, la,
			hr, hg, hb, ha,
			pbits, best_pat_bits);

		float z00[2] = { 0.0f }, z10[2] = { 0.0f }, z11[2] = { 0.0f };
		float q00_r[2] = { 0.0f };
		float q00_g[2] = { 0.0f };
		float q00_b[2] = { 0.0f };
		float q00_a[2] = { 0.0f };

		for (uint32_t i = 0; i < 16; i++)
		{
			const int subset = (best_pat_bits >> i) & 1;
			const uint32_t sel = cur_weights[i];
			assert(sel <= 3);

			z00[subset] += g_bc7_2bit_ls_tab[sel][0];
			z10[subset] += g_bc7_2bit_ls_tab[sel][1];
			z11[subset] += g_bc7_2bit_ls_tab[sel][2];

			const float w = g_bc7_2bit_ls_tab[sel][3];

			q00_r[subset] += w * (float)pPixels[i][0];
			q00_g[subset] += w * (float)pPixels[i][1];
			q00_b[subset] += w * (float)pPixels[i][2];
			q00_a[subset] += w * (float)pPixels[i][3];
		} // i

		for (uint32_t s = 0; s < 2; s++)
		{
			float q10_r = (float)total_r[s] - q00_r[s];
			float q10_g = (float)total_g[s] - q00_g[s];
			float q10_b = (float)total_b[s] - q00_b[s];
			float q10_a = (float)total_a[s] - q00_a[s];

			float z01 = z10[s];

			float det = z00[s] * z11[s] - z01 * z10[s];
			if (fabsf(det) < 1e-8f)
				continue;

			det = 1.0f / det;

			float iz00, iz01, iz10, iz11;
			iz00 = z11[s] * det;
			iz01 = -z01 * det;
			iz10 = -z10[s] * det;
			iz11 = z00[s] * det;

			const float slr = iz10 * q00_r[s] + iz11 * q10_r;
			const float shr = iz00 * q00_r[s] + iz01 * q10_r;

			const float slg = iz10 * q00_g[s] + iz11 * q10_g;
			const float shg = iz00 * q00_g[s] + iz01 * q10_g;

			const float slb = iz10 * q00_b[s] + iz11 * q10_b;
			const float shb = iz00 * q00_b[s] + iz01 * q10_b;

			const float sla = iz10 * q00_a[s] + iz11 * q10_a;
			const float sha = iz00 * q00_a[s] + iz01 * q10_a;

			if (flags & cPackBC7FlagPBitOpt)
			{
				const float q = 1.0f / 255.0f;
				float sxl[4] = { basisu::clamp(slr * q, 0.0f, 1.0f), basisu::clamp(slg * q, 0.0f, 1.0f), basisu::clamp(slb * q, 0.0f, 1.0f), basisu::clamp(sla * q, 0.0f, 1.0f) };
				float sxh[4] = { basisu::clamp(shr * q, 0.0f, 1.0f), basisu::clamp(shg * q, 0.0f, 1.0f), basisu::clamp(shb * q, 0.0f, 1.0f), basisu::clamp(sha * q, 0.0f, 1.0f) };

				color_rgba bestMinColor, bestMaxColor;
				determine_unique_pbits(4, 5, sxl, sxh, bestMinColor, bestMaxColor, &pbits[s * 2]);

				lr[s] = bestMinColor.r, lg[s] = bestMinColor.g, lb[s] = bestMinColor.b; la[s] = bestMinColor.a;
				hr[s] = bestMaxColor.r, hg[s] = bestMaxColor.g, hb[s] = bestMaxColor.b; ha[s] = bestMaxColor.a;
			}
			else
			{
				const uint32_t l_pbit = (sla >= 129.0f);
				const uint32_t h_pbit = (sha >= 129.0f);

				pbits[s * 2 + 0] = l_pbit;
				pbits[s * 2 + 1] = h_pbit;

				lr[s] = to_5_clamp(slr, l_pbit);
				lg[s] = to_5_clamp(slg, l_pbit);
				lb[s] = to_5_clamp(slb, l_pbit);
				la[s] = to_5_clamp(sla, l_pbit);

				hr[s] = to_5_clamp(shr, h_pbit);
				hg[s] = to_5_clamp(shg, h_pbit);
				hb[s] = to_5_clamp(shb, h_pbit);
				ha[s] = to_5_clamp(sha, h_pbit);
			}

		} // s

		if (pActual_sse)
		{
			*pActual_sse = eval_weights_mode7_rgba_sse(pPixels, cur_weights,
				lr, lg, lb, la,
				hr, hg, hb, ha,
				pbits, best_pat_bits);
		}
		else
		{
			eval_weights_mode7_rgba(pPixels, cur_weights,
				lr, lg, lb, la,
				hr, hg, hb, ha,
				pbits, best_pat_bits);
		}

		encode_mode7_rgba_block(pBlock, best_pat_index,
			lr, lg, lb, la,
			hr, hg, hb, ha,
			pbits, cur_weights);

#ifdef _DEBUG
		if (pActual_sse)
		{
			const uint32_t expected_sse = calc_sse(pBlock, pPixels);
			assert(expected_sse == *pActual_sse);
		}
#endif

		return true;
	}

	const int TRIVIAL_BLOCK_THRESH_RGB = 20 * 16; // skip PCA/LS threshold (uses trivial mode 6 encoder)
	const int TRIVIAL_BLOCK_THRESH_RGBA = 2 * 16;

	// dual plane
#if 0
	const int DP_BLOCK_VAR_THRESH = 1 * 16; // use dual plane threshold
	const float STRONG_CORR_THRESH = .98f;
#else
	const int DP_BLOCK_VAR_THRESH = 2 * 16; // use dual plane threshold
	const float STRONG_CORR_THRESH = .85f;
#endif

	// 2-3 subsets
#if 0
	const float HIGH_ORTHO_ENERGY_THRESH = 1.0f * 16.0f; // use 2+ subsets threshold
	const int MIN_BLOCK_MAX_VAR_23SUBSETS = 4 * 16;
	const float ORTHO_RATIO_23SUBSET_RATIO_THRESH = .004f;
#else
	const int MIN_BLOCK_MAX_VAR_23SUBSETS = 100 * 16;
	const float HIGH_ORTHO_ENERGY_THRESH = 1.0f * 16.0f; // use 2+ subsets threshold
	const float ORTHO_RATIO_23SUBSET_RATIO_THRESH = .004f;
#endif

#if 0
	const int DP_BLOCK_VAR_THRESH_RGBA = 2 * 16; // use dual plane threshold
	const float ALPHA_DECORR_THRESHOLD = .9f;
	const float STRONG_DECORR_THRESH_RGBA = .85f;
#else
	const int DP_BLOCK_VAR_THRESH_RGBA = 1 * 16; // use dual plane threshold
	//const float ALPHA_DECORR_THRESHOLD = .98f;
	const float ALPHA_DECORR_THRESHOLD = .995f;
	const float STRONG_DECORR_THRESH_RGBA = .85f;
#endif

	// 3 subsets
	const int MIN_BLOCK_MAX_VAR_3SUBSETS = 500 * 16; // use 3 subsets threshold

	//-------------------------------------------------------------------------------------------------------

	// Note: solid block check assumes A's all == 255.
	void fast_pack_bc7_rgb_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags)
	{
		assert(g_bc7_4bit_ls_tab[1][0]);

#if BASISU_BC7F_PERF_STATS
		g_total_rgb_calls++;
#endif

		const uint32_t fc = *(const uint32_t*)&pPixels[0];
		if (fc == *(const uint32_t*)&pPixels[15])
		{
			int k;
			for (k = 1; k < 15; k++)
				if (*(const uint32_t*)&pPixels[k] != fc)
					break;

			if (k == 15)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_solid_blocks++;
#endif

				pack_mode5_solid(pBlock, pPixels[0]);
				return;
			}
		}

		int total_r = 0, total_g = 0, total_b = 0;

		int min_r = 255, min_g = 255, min_b = 255;
		int max_r = 0, max_g = 0, max_b = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b;

			total_r += r; total_g += g; total_b += b;

			min_r = basisu::minimum(min_r, r); min_g = basisu::minimum(min_g, g); min_b = basisu::minimum(min_b, b);
			max_r = basisu::maximum(max_r, r); max_g = basisu::maximum(max_g, g); max_b = basisu::maximum(max_b, b);
		}

		int mean_r = (total_r + 8) >> 4, mean_g = (total_g + 8) >> 4, mean_b = (total_b + 8) >> 4;

		// covar rows are:
		// 0, 1, 2
		// 1, 3, 4
		// 2, 4, 5
		int icov[6] = { 0, 0, 0, 0, 0, 0 };

		for (uint32_t i = 0; i < 16; i++)
		{
			int r = (int)pPixels[i].r - mean_r;
			int g = (int)pPixels[i].g - mean_g;
			int b = (int)pPixels[i].b - mean_b;
			icov[0] += r * r; icov[1] += r * g; icov[2] += r * b;
			icov[3] += g * g; icov[4] += g * b;
			icov[5] += b * b;
		}

		int block_max_var = basisu::maximum(icov[0], icov[3], icov[5]); // not divided by 16, i.e. scaled by 16

		// not redundant due to uint32_t test above, which could be fooled by alpha accidentally passed in
		if (!block_max_var)
		{
#if BASISU_BC7F_PERF_STATS
			g_total_solid_blocks++;
#endif
			pack_mode5_solid(pBlock, pPixels[0]);
			return;
		}

		// check for dual plane, if a single component is very strongly decorrelated then switch to modes 4/5
		int desired_dp_chan = -1;

		if ((flags & cPackBC7FlagUseDualPlaneRGB) && (block_max_var >= DP_BLOCK_VAR_THRESH))
		{
			// 0,1
			// 0,2
			// 1,2
			const bool has_r = icov[0] > 16, has_g = icov[3] > 16, has_b = icov[5] > 16;

			const uint32_t total_active_chans = has_r + has_g + has_b;

			if (total_active_chans >= 2)
			{
				const float r_var = (float)icov[0], g_var = (float)icov[3], b_var = (float)icov[5];

				const float rg_corr = (has_r && has_g) ? fabs((float)icov[1] / sqrtf(r_var * g_var)) : 1.0f;
				const float rb_corr = (has_r && has_b) ? fabs((float)icov[2] / sqrtf(r_var * b_var)) : 1.0f;
				const float gb_corr = (has_g && has_b) ? fabs((float)icov[4] / sqrtf(g_var * b_var)) : 1.0f;

				float min_p = basisu::minimum(rg_corr, rb_corr, gb_corr);
				if (min_p < STRONG_CORR_THRESH)
				{
					if (total_active_chans == 2)
					{
						if (!has_r)
							desired_dp_chan = 1;
						else if (!has_g)
							desired_dp_chan = 0;
						else
							desired_dp_chan = 0;
					}
					else
					{
						// see if rg/rb is weakly correlated vs. gb
						if ((rg_corr < gb_corr) && (rb_corr < gb_corr))
							desired_dp_chan = 0;
						// see if gr/gb is weakly correlated vs. rb
						else if ((rg_corr < rb_corr) && (gb_corr < rb_corr))
							desired_dp_chan = 1;
						// assume b is weakest
						else
							desired_dp_chan = 2;
					}
#if BASISU_BC7F_PERF_STATS
					g_total_dp_valid_chans_rgb++;
#endif
				}
			}
		}

		if ((flags & cPackBC7FlagUseTrivialMode6) && ((desired_dp_chan == -1) && (block_max_var < TRIVIAL_BLOCK_THRESH_RGB)))
		{
			//pack_mode5_solid(pBlock, color_rgba(0, 255, 0, 255));
			//return;

			int low_c = INT_MAX, high_c = 0;

			for (uint32_t i = 0; i < 16; i++)
			{
				int y = ((16 * 2) * pPixels[i].r + (16 * 4) * pPixels[i].g + 16 * pPixels[i].b) + i;
				low_c = basisu::minimum(low_c, y);
				high_c = basisu::maximum(high_c, y);
			}

			low_c &= 0xF;
			high_c &= 0xF;

			int p0, p1, lr, lg, lb, hr, hg, hb;

			if (flags & cPackBC7FlagPBitOptMode6)
			{
				// An alternative would be to set A's=1.0 here and bias the p-bit optimizer to lower A RMSE.
				const float q = 1.0f / 255.0f;
				float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, 0 };
				float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, 0 };

				color_rgba bestMinColor, bestMaxColor;
				uint32_t best_pbits[2];
				determine_unique_pbits(3, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

				p0 = best_pbits[0], p1 = best_pbits[1];
				lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b;
				hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b;
			}
			else
			{
				p0 = 1;
				p1 = 1;

				lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0);
				hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1);
			}

			uint8_t cur_weights[16];

#if BASISU_BC7F_USE_SSE41
			eval_weights_mode6_rgb_sse41(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);
#else
			eval_weights_mode6_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);
#endif

			encode_mode6_rgba_block(pBlock,
				lr, lg, lb, 127, p0,
				hr, hg, hb, 127, p1,
				cur_weights);

#if BASISU_BC7F_PERF_STATS
			g_total_trivial_mode6_blocks++;
#endif
			return;
		}

		float cov[6];
		for (uint32_t i = 0; i < 6; i++)
			cov[i] = (float)icov[i];

		const float sc = block_max_var ? (1.0f / (float)block_max_var) : 0;
		const float wx = sc * cov[0], wy = sc * cov[3], wz = sc * cov[5];

		const float alt_xr = cov[0] * wx + cov[1] * wy + cov[2] * wz;
		const float alt_xg = cov[1] * wx + cov[3] * wy + cov[4] * wz;
		const float alt_xb = cov[2] * wx + cov[4] * wy + cov[5] * wz;

		// quite rough mode 6 SSE estimate (explictly higher bound): if some other mode can't even beat this, don't use it and we fall back to a decently strong mode 6
		const int spans[4] = { max_r - min_r, max_g - min_g, max_b - min_b, 0 };
				
		// need_sse_estimates MUST be set correctly or subtle mode selection issues will occur.
		const bool need_sse_estimates = ((flags & cPackBC7FlagUse2SubsetsRGB) != 0) || (desired_dp_chan >= 0);

		float mode6_ortho_ratio = 0;
		const float mode6_slam_to_line_sse_est = need_sse_estimates ? estimate_slam_to_line_sse_3D(cov, alt_xr, alt_xg, alt_xb, &mode6_ortho_ratio) : 0;
		const float mode6_sse_est = need_sse_estimates ? (mode6_slam_to_line_sse_est + analytical_quant_est_sse(128, 16, 3, spans, nullptr, 1.0f, 16)) : 0;

		// Prefer 2/3-subsets over dual plane
		// TODO: Use mode 6 sse est?
		if ((flags & cPackBC7FlagUse2SubsetsRGB) && (block_max_var >= MIN_BLOCK_MAX_VAR_23SUBSETS) && (mode6_ortho_ratio > ORTHO_RATIO_23SUBSET_RATIO_THRESH))
		{
			assert(need_sse_estimates);

			const bool high_ortho_energy_flag = (mode6_slam_to_line_sse_est >= HIGH_ORTHO_ENERGY_THRESH);

			if (high_ortho_energy_flag)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_high_ortho_energy++;
#endif
				//pack_mode5_solid(pBlock, color_rgba(255, 255, 0, 255));
				//return;

				if ((flags & cPackBC7FlagUse3SubsetsRGB) && (block_max_var >= MIN_BLOCK_MAX_VAR_3SUBSETS))
				{
					//pack_mode5_solid(pBlock, color_rgba(255, 0, 255, 255));
					//return;

#if 0
					if (pack_mode0_or_2_rgb(pBlock, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, mode6_sse_est, flags))
					{
						return;
					}
#else
					float mode0_or_2_sse_est = 1e+9f;
					if (pack_mode0_or_2_rgb(pBlock, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, mode6_sse_est, flags, &mode0_or_2_sse_est))
					{
						float mode1_or_3_sse_est = 1e+9f;

						uint8_t temp_2subset_block[sizeof(basist::bc7_block)];
						if (pack_mode1_or_3_rgb(temp_2subset_block, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, mode0_or_2_sse_est, flags, &mode1_or_3_sse_est))
						{
							assert(mode1_or_3_sse_est < mode0_or_2_sse_est);
							memcpy(pBlock, temp_2subset_block, sizeof(basist::bc7_block));
						}

						return;
					}
#endif
				}

				if (pack_mode1_or_3_rgb(pBlock, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, mode6_sse_est, flags))
					return;
			}
		}

		// Use dual plane over mode 6
		if (desired_dp_chan >= 0)
		{
			assert(need_sse_estimates);

			if (pack_mode4_or_5(pBlock, pPixels, desired_dp_chan, mode6_sse_est, flags))
				return;

		} // if (desired_dp_chan >= 0)

		int saxis_r = 306, saxis_g = 601, saxis_b = 117;

		float k = basisu::maximum(fabsf(alt_xr), fabsf(alt_xg), fabsf(alt_xb));
		if (fabs(k) >= basisu::SMALL_FLOAT_VAL)
		{
			float m = 2048.0f / k;
			saxis_r = (int)(alt_xr * m);
			saxis_g = (int)(alt_xg * m);
			saxis_b = (int)(alt_xb * m);
		}

		saxis_r = (int)((uint32_t)saxis_r << 4U);
		saxis_g = (int)((uint32_t)saxis_g << 4U);
		saxis_b = (int)((uint32_t)saxis_b << 4U);

		int low_dot = INT_MAX, high_dot = INT_MIN;

#if BASISU_BC7F_USE_SSE41
		int low_c, high_c;
		bc7_proj_minmax_indices_sse41(pPixels, saxis_r, saxis_g, saxis_b, &low_c, &high_c);
#else
		for (uint32_t i = 0; i < 16; i += 4)
		{
			assert(((pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b) & 0xF) == 0); // sanity
			assert(((pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b) & 0xF) == 0);
			assert(((pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b) & 0xF) == 0);
			assert(((pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b) & 0xF) == 0);

			const int dot0 = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b) + i;
			const int dot1 = (pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b) + i + 1;
			const int dot2 = (pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b) + i + 2;
			const int dot3 = (pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b) + i + 3;

			int min_d01 = basisu::minimum(dot0, dot1);
			int max_d01 = basisu::maximum(dot0, dot1);

			int min_d23 = basisu::minimum(dot2, dot3);
			int max_d23 = basisu::maximum(dot2, dot3);

			int min_d = basisu::minimum(min_d01, min_d23);
			int max_d = basisu::maximum(max_d01, max_d23);

			low_dot = basisu::minimum(low_dot, min_d);
			high_dot = basisu::maximum(high_dot, max_d);
		}

		int low_c = low_dot & 15;
		int high_c = high_dot & 15;
#endif

		int p0, p1, lr, lg, lb, hr, hg, hb;

		if (flags & cPackBC7FlagPBitOptMode6)
		{
			const float q = 1.0f / 255.0f;
			float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, 0 };
			float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, 0 };

			color_rgba bestMinColor, bestMaxColor;
			uint32_t best_pbits[2];
			determine_unique_pbits(3, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

			p0 = best_pbits[0], p1 = best_pbits[1];
			lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b;
			hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b;
		}
		else
		{
			// explictly force pbits to 1, that way alpha is always 255 and we don't slow down the entire encoder by 4-8% for a tiny ~.1 dB PSNR gain (not worth it)
			p0 = 1, p1 = 1;
			lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0);
			hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1);
		}

		uint8_t cur_weights[16];

#if BASISU_BC7F_USE_SSE41
		eval_weights_mode6_rgb_sse41(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);
#else
		eval_weights_mode6_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);
#endif

		vec4F xl, xh;
		bool res = compute_least_squares_endpoints_3D(
			16, cur_weights, 16,
			g_bc7_4bit_ls_tab,
			xl, xh,
			pPixels,
			(float)total_r, (float)total_g, (float)total_b);

		if (res)
		{
			if (flags & cPackBC7FlagPBitOptMode6)
			{
				const float q = 1.0f / 255.0f;
				float sxl[4] = { xl[0] * q, xl[1] * q, xl[2] * q, 0.0f };
				float sxh[4] = { xh[0] * q, xh[1] * q, xh[2] * q, 0.0f };

				color_rgba bestMinColor, bestMaxColor;
				uint32_t best_pbits[2];
				determine_unique_pbits(
					3, 7, sxl, sxh,
					bestMinColor, bestMaxColor, best_pbits);

				p0 = best_pbits[0], p1 = best_pbits[1];
				lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b;
				hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b;
			}
			else
			{
				p0 = 1; p1 = 1;
				lr = to_7(xl[0], p0);
				lg = to_7(xl[1], p0);
				lb = to_7(xl[2], p0);

				hr = to_7(xh[0], p1);
				hg = to_7(xh[1], p1);
				hb = to_7(xh[2], p1);
			}

#if BASISU_BC7F_USE_SSE41
			eval_weights_mode6_rgb_sse41(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);
#else
			eval_weights_mode6_rgb(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);
#endif
		}

		//pack_mode5_solid(pBlock, color_rgba(0, 0, 255, 255));
		//return;

		// pbits set to 1 to ensure alpha is always decoded to fully opaque (255)
		encode_mode6_rgba_block(pBlock,
			lr, lg, lb, 127, p0,
			hr, hg, hb, 127, p1,
			cur_weights);
	}

	//-------------------------------------------------------------------------------------------------------
	const int MIN_BLOCK_MAX_VAR_23SUBSETS_RGBA = 100 * 16;
	const float HIGH_ORTHO_ENERGY_THRESH_RGBA = 1.0f * 16.0f; // use 2+ subsets threshold
	const float ORTHO_RATIO_23SUBSET_RATIO_THRESH_RGBA = .004f;

	uint32_t fast_pack_bc7_rgb_partial_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags)
	{
		assert(g_bc7_4bit_ls_tab[1][0]);

#if BASISU_BC7F_PERF_STATS
		g_total_rgb_calls++;
#endif

		const uint32_t fc = *(const uint32_t*)&pPixels[0];
		if (fc == *(const uint32_t*)&pPixels[15])
		{
			int k;
			for (k = 1; k < 15; k++)
				if (*(const uint32_t*)&pPixels[k] != fc)
					break;

			if (k == 15)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_solid_blocks++;
#endif

				pack_mode5_solid(pBlock, pPixels[0]);
				return 0;
			}
		}

		int total_r = 0, total_g = 0, total_b = 0;

		int min_r = 255, min_g = 255, min_b = 255;
		int max_r = 0, max_g = 0, max_b = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b;

			total_r += r; total_g += g; total_b += b;

			min_r = basisu::minimum(min_r, r); min_g = basisu::minimum(min_g, g); min_b = basisu::minimum(min_b, b);
			max_r = basisu::maximum(max_r, r); max_g = basisu::maximum(max_g, g); max_b = basisu::maximum(max_b, b);
		}

		int mean_r = (total_r + 8) >> 4, mean_g = (total_g + 8) >> 4, mean_b = (total_b + 8) >> 4;

		// covar rows are:
		// 0, 1, 2
		// 1, 3, 4
		// 2, 4, 5
		int icov[6] = { 0, 0, 0, 0, 0, 0 };

		for (uint32_t i = 0; i < 16; i++)
		{
			int r = (int)pPixels[i].r - mean_r;
			int g = (int)pPixels[i].g - mean_g;
			int b = (int)pPixels[i].b - mean_b;
			icov[0] += r * r; icov[1] += r * g; icov[2] += r * b;
			icov[3] += g * g; icov[4] += g * b;
			icov[5] += b * b;
		}

		int block_max_var = basisu::maximum(icov[0], icov[3], icov[5]); // not divided by 16, i.e. scaled by 16

		// not redundant due to uint32_t test above, which could be fooled by alpha accidentally passed in
		if (!block_max_var)
		{
#if BASISU_BC7F_PERF_STATS
			g_total_solid_blocks++;
#endif
			pack_mode5_solid(pBlock, pPixels[0]);
			return 0;
		}

		// check for dual plane, if a single component is very strongly decorrelated then switch to modes 4/5
		int desired_dp_chan = -1;

		const bool non_analytical_flag = (flags & cPackBC7FlagNonAnalyticalRGB) != 0;
		if (non_analytical_flag)
		{
			assert(flags & cPackBC7FlagPartiallyAnalyticalRGB);
		}

		if ((flags & cPackBC7FlagUseDualPlaneRGB) &&
			((!non_analytical_flag && (block_max_var >= DP_BLOCK_VAR_THRESH)) || (non_analytical_flag && (block_max_var >= (DP_BLOCK_VAR_THRESH / 2)))))
		{
			// 0,1
			// 0,2
			// 1,2
			const bool has_r = icov[0] > 16, has_g = icov[3] > 16, has_b = icov[5] > 16;

			const uint32_t total_active_chans = has_r + has_g + has_b;

			if (total_active_chans >= 2)
			{
				const float r_var = (float)icov[0], g_var = (float)icov[3], b_var = (float)icov[5];

				const float rg_corr = (has_r && has_g) ? fabs((float)icov[1] / sqrtf(r_var * g_var)) : 1.0f;
				const float rb_corr = (has_r && has_b) ? fabs((float)icov[2] / sqrtf(r_var * b_var)) : 1.0f;
				const float gb_corr = (has_g && has_b) ? fabs((float)icov[4] / sqrtf(g_var * b_var)) : 1.0f;

				float min_p = basisu::minimum(rg_corr, rb_corr, gb_corr);

				const float corr_thresh = non_analytical_flag ? .999f : STRONG_CORR_THRESH;

				if (min_p < corr_thresh)
				{
					if (total_active_chans == 2)
					{
						if (!has_r)
							desired_dp_chan = 1;
						else if (!has_g)
							desired_dp_chan = 0;
						else
							desired_dp_chan = 0;
					}
					else
					{
						// see if rg/rb is weakly correlated vs. gb
						if ((rg_corr < gb_corr) && (rb_corr < gb_corr))
							desired_dp_chan = 0;
						// see if gr/gb is weakly correlated vs. rb
						else if ((rg_corr < rb_corr) && (gb_corr < rb_corr))
							desired_dp_chan = 1;
						// assume b is weakest
						else
							desired_dp_chan = 2;
					}
#if BASISU_BC7F_PERF_STATS
					g_total_dp_valid_chans_rgb++;
#endif
				}
			}
		}

		if ((flags & cPackBC7FlagUseTrivialMode6) && ((desired_dp_chan == -1) && (block_max_var < TRIVIAL_BLOCK_THRESH_RGB)))
		{
			int low_c = INT_MAX, high_c = 0;

			for (uint32_t i = 0; i < 16; i++)
			{
				int y = ((16 * 2) * pPixels[i].r + (16 * 4) * pPixels[i].g + 16 * pPixels[i].b) + i;
				low_c = basisu::minimum(low_c, y);
				high_c = basisu::maximum(high_c, y);
			}

			low_c &= 0xF;
			high_c &= 0xF;

			int p0, p1, lr, lg, lb, hr, hg, hb;

			if (flags & cPackBC7FlagPBitOptMode6)
			{
				// An alternative would be to set A's=1.0 here and bias the p-bit optimizer to lower A RMSE.
				const float q = 1.0f / 255.0f;
				float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, 0 };
				float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, 0 };

				color_rgba bestMinColor, bestMaxColor;
				uint32_t best_pbits[2];
				determine_unique_pbits(3, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

				p0 = best_pbits[0], p1 = best_pbits[1];
				lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b;
				hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b;
			}
			else
			{
				p0 = 1;
				p1 = 1;

				lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0);
				hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1);
			}

			uint8_t cur_weights[16];
			uint32_t mode6_actual_sse = eval_weights_mode6_rgb_sse(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);

			encode_mode6_rgba_block(pBlock,
				lr, lg, lb, 127, p0,
				hr, hg, hb, 127, p1,
				cur_weights);

#if BASISU_BC7F_PERF_STATS
			g_total_trivial_mode6_blocks++;
#endif

#ifdef _DEBUG
			{
				// Final sanity checking.
				uint32_t expected_actual_sse = calc_sse(pBlock, pPixels);
				assert(expected_actual_sse == mode6_actual_sse);
			}
#endif

			return mode6_actual_sse;
		}

		float cov[6];
		for (uint32_t i = 0; i < 6; i++)
			cov[i] = (float)icov[i];

		const float sc = block_max_var ? (1.0f / (float)block_max_var) : 0;
		const float wx = sc * cov[0], wy = sc * cov[3], wz = sc * cov[5];

		const float alt_xr = cov[0] * wx + cov[1] * wy + cov[2] * wz;
		const float alt_xg = cov[1] * wx + cov[3] * wy + cov[4] * wz;
		const float alt_xb = cov[2] * wx + cov[4] * wy + cov[5] * wz;

		int saxis_r = 306, saxis_g = 601, saxis_b = 117;

		float k = basisu::maximum(fabsf(alt_xr), fabsf(alt_xg), fabsf(alt_xb));
		if (fabs(k) >= basisu::SMALL_FLOAT_VAL)
		{
			float m = 2048.0f / k;
			saxis_r = (int)(alt_xr * m);
			saxis_g = (int)(alt_xg * m);
			saxis_b = (int)(alt_xb * m);
		}

		saxis_r = (int)((uint32_t)saxis_r << 4U);
		saxis_g = (int)((uint32_t)saxis_g << 4U);
		saxis_b = (int)((uint32_t)saxis_b << 4U);

		int low_dot = INT_MAX, high_dot = INT_MIN;

#if BASISU_BC7F_USE_SSE41
		int low_c, high_c;
		bc7_proj_minmax_indices_sse41(pPixels, saxis_r, saxis_g, saxis_b, &low_c, &high_c);
#else
		for (uint32_t i = 0; i < 16; i += 4)
		{
			assert(((pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b) & 0xF) == 0); // sanity
			assert(((pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b) & 0xF) == 0);
			assert(((pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b) & 0xF) == 0);
			assert(((pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b) & 0xF) == 0);

			const int dot0 = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b) + i;
			const int dot1 = (pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b) + i + 1;
			const int dot2 = (pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b) + i + 2;
			const int dot3 = (pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b) + i + 3;

			int min_d01 = basisu::minimum(dot0, dot1);
			int max_d01 = basisu::maximum(dot0, dot1);

			int min_d23 = basisu::minimum(dot2, dot3);
			int max_d23 = basisu::maximum(dot2, dot3);

			int min_d = basisu::minimum(min_d01, min_d23);
			int max_d = basisu::maximum(max_d01, max_d23);

			low_dot = basisu::minimum(low_dot, min_d);
			high_dot = basisu::maximum(high_dot, max_d);
		}

		int low_c = low_dot & 15;
		int high_c = high_dot & 15;
#endif

		int p0, p1, lr, lg, lb, hr, hg, hb;

		if (flags & cPackBC7FlagPBitOptMode6)
		{
			const float q = 1.0f / 255.0f;
			float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, 0 };
			float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, 0 };

			color_rgba bestMinColor, bestMaxColor;
			uint32_t best_pbits[2];
			determine_unique_pbits(3, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

			p0 = best_pbits[0], p1 = best_pbits[1];
			lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b;
			hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b;
		}
		else
		{
			// explictly force pbits to 1, that way alpha is always 255 and we don't slow down the entire encoder by 4-8% for a tiny ~.1 dB PSNR gain (not worth it)
			p0 = 1, p1 = 1;
			lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0);
			hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1);
		}

		uint8_t cur_weights[16];

		uint32_t mode6_actual_sse = eval_weights_mode6_rgb_sse(pPixels, cur_weights, lr, lg, lb, hr, hg, hb, p0, p1);

		if (mode6_actual_sse)
		{
			vec4F xl, xh;
			bool res = compute_least_squares_endpoints_3D(
				16, cur_weights, 16,
				g_bc7_4bit_ls_tab,
				xl, xh,
				pPixels,
				(float)total_r, (float)total_g, (float)total_b);

			if (res)
			{
				int trial_p0, trial_p1, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb;

				if (flags & cPackBC7FlagPBitOptMode6)
				{
					const float q = 1.0f / 255.0f;
					float sxl[4] = { xl[0] * q, xl[1] * q, xl[2] * q, 0.0f };
					float sxh[4] = { xh[0] * q, xh[1] * q, xh[2] * q, 0.0f };

					color_rgba bestMinColor, bestMaxColor;
					uint32_t best_pbits[2];
					determine_unique_pbits(
						3, 7, sxl, sxh,
						bestMinColor, bestMaxColor, best_pbits);

					trial_p0 = best_pbits[0], trial_p1 = best_pbits[1];
					trial_lr = bestMinColor.r, trial_lg = bestMinColor.g, trial_lb = bestMinColor.b;
					trial_hr = bestMaxColor.r, trial_hg = bestMaxColor.g, trial_hb = bestMaxColor.b;
				}
				else
				{
					trial_p0 = 1; trial_p1 = 1;
					trial_lr = to_7(xl[0], trial_p0);
					trial_lg = to_7(xl[1], trial_p0);
					trial_lb = to_7(xl[2], trial_p0);

					trial_hr = to_7(xh[0], trial_p1);
					trial_hg = to_7(xh[1], trial_p1);
					trial_hb = to_7(xh[2], trial_p1);
				}

				uint8_t trial_weights[16];
				uint32_t mode6_ls_actual_sse = eval_weights_mode6_rgb_sse(pPixels, trial_weights, trial_lr, trial_lg, trial_lb, trial_hr, trial_hg, trial_hb, trial_p0, trial_p1);
				if (mode6_ls_actual_sse < mode6_actual_sse)
				{
					mode6_actual_sse = mode6_ls_actual_sse;
					memcpy(cur_weights, trial_weights, 16);
					p0 = trial_p0; p1 = trial_p1;
					lr = trial_lr; lg = trial_lg; lb = trial_lb;
					hr = trial_hr; hg = trial_hg; hb = trial_hb;
				}
			}
		}

		uint32_t mode02_actual_sse = UINT32_MAX;
		uint8_t mode02_candidate_block[sizeof(basist::bc7_block)];

		uint32_t mode13_actual_sse = UINT32_MAX;
		uint8_t mode13_candidate_block[sizeof(basist::bc7_block)];

		uint32_t mode45_actual_sse = UINT32_MAX;
		uint8_t mode45_candidate_block[sizeof(basist::bc7_block)];

		if (mode6_actual_sse)
		{
			if (non_analytical_flag)
			{
				// No gates: very expensive.
				if (flags & cPackBC7FlagUse2SubsetsRGB)
				{
					if (flags & cPackBC7FlagUse3SubsetsRGB)
					{
						pack_mode0_or_2_rgb(mode02_candidate_block, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, 1e+9f, flags, nullptr, &mode02_actual_sse);
					}

					pack_mode1_or_3_rgb(mode13_candidate_block, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, 1e+9f, flags, nullptr, &mode13_actual_sse);
				}

				if (flags & cPackBC7FlagUseDualPlaneRGB)
					pack_mode4_or_5(mode45_candidate_block, pPixels, (desired_dp_chan >= 0) ? desired_dp_chan : 1, 1e+9f, flags, nullptr, &mode45_actual_sse); // todo: determine best def channel here
			}
			else
			{
				float mode6_ortho_ratio;
				const float mode6_slam_to_line_sse_est = estimate_slam_to_line_sse_3D(cov, alt_xr, alt_xg, alt_xb, &mode6_ortho_ratio);

				if ((flags & cPackBC7FlagUse2SubsetsRGB) && (block_max_var >= MIN_BLOCK_MAX_VAR_23SUBSETS) && (mode6_ortho_ratio > ORTHO_RATIO_23SUBSET_RATIO_THRESH))
				{
					const bool high_ortho_energy_flag = (mode6_slam_to_line_sse_est >= HIGH_ORTHO_ENERGY_THRESH);

					if (high_ortho_energy_flag)
					{
#if BASISU_BC7F_PERF_STATS
						g_total_high_ortho_energy++;
#endif

						if ((flags & cPackBC7FlagUse3SubsetsRGB) && (block_max_var >= MIN_BLOCK_MAX_VAR_3SUBSETS))
						{
							pack_mode0_or_2_rgb(mode02_candidate_block, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, 1e+9f, flags, nullptr, &mode02_actual_sse);
							pack_mode1_or_3_rgb(mode13_candidate_block, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, 1e+9f, flags, nullptr, &mode13_actual_sse);
						}
						else
						{
							pack_mode1_or_3_rgb(mode13_candidate_block, pPixels, alt_xr, alt_xg, alt_xb, mean_r, mean_g, mean_b, 1e+9f, flags, nullptr, &mode13_actual_sse);
						}
					}
				}

				if (desired_dp_chan >= 0)
				{
					assert(flags & cPackBC7FlagUseDualPlaneRGB);

					pack_mode4_or_5(mode45_candidate_block, pPixels, desired_dp_chan, 1e+9f, flags, nullptr, &mode45_actual_sse);

				} // if (desired_dp_chan >= 0)
			}
		}

		const uint32_t best_actual_sse = basisu::minimum(mode6_actual_sse, mode02_actual_sse, mode13_actual_sse, mode45_actual_sse);

		if (mode6_actual_sse == best_actual_sse)
		{
			// pbits set to 1 to ensure alpha is always decoded to fully opaque (255)
			encode_mode6_rgba_block(pBlock,
				lr, lg, lb, 127, p0,
				hr, hg, hb, 127, p1,
				cur_weights);
		}
		else if ((mode45_actual_sse != UINT32_MAX) && (best_actual_sse == mode45_actual_sse))
		{
			memcpy(pBlock, mode45_candidate_block, sizeof(basist::bc7_block));
		}
		else if ((mode02_actual_sse != UINT32_MAX) && (best_actual_sse == mode02_actual_sse))
		{
			memcpy(pBlock, mode02_candidate_block, sizeof(basist::bc7_block));
		}
		else if ((mode13_actual_sse != UINT32_MAX) && (best_actual_sse == mode13_actual_sse))
		{
			memcpy(pBlock, mode13_candidate_block, sizeof(basist::bc7_block));
		}
		else
		{
			assert(0);
		}

#ifdef _DEBUG
		{
			// Final sanity checking.
			uint32_t expected_actual_sse = calc_sse(pBlock, pPixels);
			assert(expected_actual_sse == best_actual_sse);
		}
#endif

		return best_actual_sse;
	}

	//-------------------------------------------------------------------------------------------------------

	void fast_pack_bc7_rgba_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags)
	{
		assert(g_bc7_4bit_ls_tab[1][0]);

#if BASISU_BC7F_PERF_STATS
		g_total_rgba_calls++;
#endif

		const uint32_t fc = *(const uint32_t*)&pPixels[0];
		if (fc == *(const uint32_t*)&pPixels[15])
		{
			int k;
			for (k = 1; k < 15; k++)
				if (*(const uint32_t*)&pPixels[k] != fc)
					break;

			if (k == 15)
			{
				pack_mode5_solid(pBlock, pPixels[0]);
				return;
			}
		}

		int total_r = 0, total_g = 0, total_b = 0, total_a = 0;
		int min_r = 255, min_g = 255, min_b = 255, min_a = 255;
		int max_r = 0, max_g = 0, max_b = 0, max_a = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b, a = pPixels[i].a;

			total_r += r; total_g += g; total_b += b; total_a += a;

			min_r = basisu::minimum(min_r, r); min_g = basisu::minimum(min_g, g); min_b = basisu::minimum(min_b, b); min_a = basisu::minimum(min_a, a);
			max_r = basisu::maximum(max_r, r); max_g = basisu::maximum(max_g, g); max_b = basisu::maximum(max_b, b); max_a = basisu::maximum(max_a, a);
		}

		assert((min_r != max_r) || (min_g != max_g) || (min_b != max_b) || (min_a != max_a));

		const int mean_r = (total_r + 8) >> 4, mean_g = (total_g + 8) >> 4, mean_b = (total_b + 8) >> 4, mean_a = (total_a + 8) >> 4;

		// covar rows are:
		int icov4[10] = { };

		// 0=rr
		// 1=rg
		// 2=rb
		// 3=ra
		// 
		// 4=gg
		// 5=gb
		// 6=ga
		// 
		// 7=bb
		// 8=ba
		// 
		// 9=aa

		// 0 1 2 3
		//   4 5 6
		//     7 8
		//       9

		// 0 1 2 3
		// 1 4 5 6
		// 2 5 7 8
		// 3 6 8 9

		// trace at 0,4,7,9

		for (uint32_t i = 0; i < 16; i++)
		{
			const int r = (int)pPixels[i].r - mean_r, g = (int)pPixels[i].g - mean_g, b = (int)pPixels[i].b - mean_b, a = (int)pPixels[i].a - mean_a;

			icov4[0] += r * r; icov4[1] += r * g; icov4[2] += r * b; icov4[3] += r * a;
			icov4[4] += g * g; icov4[5] += g * b; icov4[6] += g * a;
			icov4[7] += b * b; icov4[8] += b * a;
			icov4[9] += a * a;
		}

		const int block_max_var4 = basisu::maximum(icov4[0], icov4[4], icov4[7], icov4[9]); // not divided by 16, i.e. scaled by 16
		assert(block_max_var4); // solid blocks already filtered out

		// check for dual plane, if a single component is very strongly decorrelated then switch to modes 4/5
		int desired_dp_chan = -1;

		if ((flags & cPackBC7FlagUseDualPlaneRGBA) && (block_max_var4 >= DP_BLOCK_VAR_THRESH_RGBA))
		{
			// Prefer A, if not strongly decorrelated then check RGB.
			const float r_var = (float)icov4[0], g_var = (float)icov4[4], b_var = (float)icov4[7], a_var = (float)icov4[9];

			const bool has_a = icov4[9] > 0;

			if (has_a)
			{
				const float p_03 = icov4[0] ? fabs((float)icov4[3] / sqrtf(r_var * a_var)) : 1.0f;
				const float p_13 = icov4[4] ? fabs((float)icov4[6] / sqrtf(g_var * a_var)) : 1.0f;
				const float p_23 = icov4[7] ? fabs((float)icov4[8] / sqrtf(b_var * a_var)) : 1.0f;

				const float min_p = basisu::minimum(p_03, p_13, p_23);
				if (min_p < ALPHA_DECORR_THRESHOLD)
				{
					desired_dp_chan = 3;
#if BASISU_BC7F_PERF_STATS
					g_total_dp_valid_chans_a++;
#endif
				}
			}

			if ((desired_dp_chan < 0) && ((flags & cPackBC7FlagDisableRGBDualPlane) == 0))
			{
				const bool has_r = icov4[0] > 16, has_g = icov4[4] > 16, has_b = icov4[7] > 16;
				const uint32_t total_active_chans_rgb = has_r + has_g + has_b;

				if (total_active_chans_rgb >= 2)
				{
					const float rg_corr = (has_r && has_g) ? fabs((float)icov4[1] / sqrtf(r_var * g_var)) : 1.0f;
					const float rb_corr = (has_r && has_b) ? fabs((float)icov4[2] / sqrtf(r_var * b_var)) : 1.0f;
					const float gb_corr = (has_g && has_b) ? fabs((float)icov4[5] / sqrtf(g_var * b_var)) : 1.0f;

					float min_p = basisu::minimum(rg_corr, rb_corr, gb_corr);
					if (min_p < STRONG_DECORR_THRESH_RGBA)
					{
						if (total_active_chans_rgb == 2)
						{
							if (!has_r)
								desired_dp_chan = 1;
							else if (!has_g)
								desired_dp_chan = 0;
							else
								desired_dp_chan = 0;
						}
						else
						{
							// see if rg/rb is weakly correlated vs. gb
							if ((rg_corr < gb_corr) && (rb_corr < gb_corr))
								desired_dp_chan = 0;
							// see if gr/gb is weakly correlated vs. rb
							else if ((rg_corr < rb_corr) && (gb_corr < rb_corr))
								desired_dp_chan = 1;
							// assume b is weakest
							else
								desired_dp_chan = 2;
						}
#if BASISU_BC7F_PERF_STATS
						g_total_dp_valid_chans_rgb++;
#endif
					}
				}
			}
		}

		if ((flags & cPackBC7FlagUseTrivialMode6) && ((desired_dp_chan == -1) && (block_max_var4 < TRIVIAL_BLOCK_THRESH_RGBA)))
		{
			//pack_mode5_solid(pBlock, color_rgba(0, 255, 0, 255));
			//return;

			int low_c = INT_MAX, high_c = 0;

			for (uint32_t i = 0; i < 16; i++)
			{
				int y = ((16 * 2) * pPixels[i].r + (16 * 4) * pPixels[i].g + 16 * pPixels[i].b + (16 * 4) * pPixels[i].a);
				assert((y & 0xF) == 0);
				y += i;
				low_c = basisu::minimum(low_c, y);
				high_c = basisu::maximum(high_c, y);
			}

			low_c &= 0xF;
			high_c &= 0xF;

			int p0, p1, lr, lg, lb, la, hr, hg, hb, ha;

			if (flags & cPackBC7FlagPBitOptMode6)
			{
				const float q = 1.0f / 255.0f;
				float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, (float)pPixels[low_c].a * q };
				float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, (float)pPixels[high_c].a * q };

				color_rgba bestMinColor, bestMaxColor;
				uint32_t best_pbits[2];
				determine_unique_pbits(4, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

				p0 = best_pbits[0], p1 = best_pbits[1];
				lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b, la = bestMinColor.a;
				hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b, ha = bestMaxColor.a;
			}
			else
			{
				p0 = pPixels[low_c].a > 128;
				p1 = pPixels[high_c].a > 128;

				lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0), la = to_7(pPixels[low_c].a, p0);
				hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1), ha = to_7(pPixels[high_c].a, p1);
			}

			uint8_t cur_weights[16];
			eval_weights_mode6_rgba(pPixels, cur_weights,
				lr, lg, lb, la, p0,
				hr, hg, hb, ha, p1);

			encode_mode6_rgba_block(pBlock,
				lr, lg, lb, la, p0,
				hr, hg, hb, ha, p1,
				cur_weights);

#if BASISU_BC7F_PERF_STATS
			g_total_trivial_mode6_blocks++;
#endif
			return;
		}

		float cov4[10];
		for (uint32_t i = 0; i < 10; i++)
			cov4[i] = (float)icov4[i];

		// all channel pairs:
		// 0,1=1
		// 0,2=2
		// 0,3=3
		// 1,2=5
		// 1,3=6
		// 2,3=8
		//const float r_var = cov4[0], g_var = cov4[4], b_var = cov4[7], a_var = cov4[9];

		const float sc4 = block_max_var4 ? (1.0f / (float)block_max_var4) : 0;
		float wx = sc4 * cov4[0], wy = sc4 * cov4[4], wz = sc4 * cov4[7], wa = sc4 * cov4[9];

		// 0 1 2 3
		// 1 4 5 6
		// 2 5 7 8
		// 3 6 8 9

		// TODO
		float x1, y1, z1, w1;
		for (uint32_t i = 0; i < 4; i++)
		{
			x1 = cov4[0] * wx + cov4[1] * wy + cov4[2] * wz + cov4[3] * wa;
			y1 = cov4[1] * wx + cov4[4] * wy + cov4[5] * wz + cov4[6] * wa;
			z1 = cov4[2] * wx + cov4[5] * wy + cov4[7] * wz + cov4[8] * wa;
			w1 = cov4[3] * wx + cov4[6] * wy + cov4[8] * wz + cov4[9] * wa;

			float t = sqrtf(x1 * x1 + y1 * y1 + z1 * z1 + w1 * w1);
			if (t > basisu::SMALL_FLOAT_VAL)
			{
				t = 1.0f / t;
				x1 *= t; y1 *= t; z1 *= t; w1 *= t;
			}
			else
			{
				x1 = y1 = z1 = w1 = .25f;
			}

			wx = x1; wy = y1; wz = z1; wa = w1;
		}

		const int spans[4] = { max_r - min_r, max_g - min_g, max_b - min_b, max_a - min_a };

		float mode6_ortho_ratio;
		const float mode6_slam_to_line_sse_est = estimate_slam_to_line_sse_4D(cov4, x1, y1, z1, w1, &mode6_ortho_ratio);
		const float mode6_sse_est = (mode6_slam_to_line_sse_est + analytical_quant_est_sse(128, 16, 4, spans, nullptr, 1.0f, 16));

		float mode45_sse_est = 1e+9f, mode7_sse_est = 1e+9f;

		uint8_t mode45_block[sizeof(basist::bc7_block)];
		const bool mode45_valid_flag = (desired_dp_chan >= 0) ? pack_mode4_or_5(mode45_block, pPixels, desired_dp_chan, mode6_sse_est, flags, &mode45_sse_est) : false;
		BASISU_NOTE_UNUSED(mode45_valid_flag);

		uint8_t mode7_block[sizeof(basist::bc7_block)];
		bool mode7_valid_flag = false;
		BASISU_NOTE_UNUSED(mode7_valid_flag);

		if ((flags & cPackBC7FlagUse2SubsetsRGBA) && (block_max_var4 >= MIN_BLOCK_MAX_VAR_23SUBSETS_RGBA) && (mode6_ortho_ratio > ORTHO_RATIO_23SUBSET_RATIO_THRESH_RGBA))
		{
			const bool high_ortho_energy_flag = (mode6_slam_to_line_sse_est >= HIGH_ORTHO_ENERGY_THRESH_RGBA);

			if (high_ortho_energy_flag)
			{
#if BASISU_BC7F_PERF_STATS
				g_total_high_ortho_energy++;
#endif
				mode7_valid_flag = pack_mode7_rgba(mode7_block, pPixels, x1, y1, z1, w1, mean_r, mean_g, mean_b, mean_a, mode6_sse_est, flags, &mode7_sse_est);
			}
		}

		if ((mode45_sse_est < mode7_sse_est) && (mode45_sse_est < mode6_sse_est))
		{
			assert(mode45_valid_flag);
			memcpy(pBlock, mode45_block, sizeof(basist::bc7_block));
			return;
		}
		else if ((mode7_sse_est < mode45_sse_est) && (mode7_sse_est < mode6_sse_est))
		{
			assert(mode7_valid_flag);
			memcpy(pBlock, mode7_block, sizeof(basist::bc7_block));
			return;
		}

		// Fall back to mode 6
		int saxis_r = 256, saxis_g = 256, saxis_b = 256, saxis_a = 256;

		float k = basisu::maximum(fabsf(x1), fabsf(y1), fabsf(z1), fabsf(w1));
		if (fabs(k) >= basisu::SMALL_FLOAT_VAL)
		{
			float m = 2048.0f / k;
			saxis_r = (int)(x1 * m);
			saxis_g = (int)(y1 * m);
			saxis_b = (int)(z1 * m);
			saxis_a = (int)(w1 * m);
		}

		saxis_r = (int)((uint32_t)saxis_r << 4U);
		saxis_g = (int)((uint32_t)saxis_g << 4U);
		saxis_b = (int)((uint32_t)saxis_b << 4U);
		saxis_a = (int)((uint32_t)saxis_a << 4U);

		int low_dot = INT_MAX, high_dot = INT_MIN;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			assert(((pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b + pPixels[i].a * saxis_a) & 0xF) == 0); // sanity
			assert(((pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b + pPixels[i + 1].a * saxis_a) & 0xF) == 0);
			assert(((pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b + pPixels[i + 2].a * saxis_a) & 0xF) == 0);
			assert(((pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b + pPixels[i + 3].a * saxis_a) & 0xF) == 0);

			const int dot0 = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b + pPixels[i].a * saxis_a) + i;
			const int dot1 = (pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b + pPixels[i + 1].a * saxis_a) + i + 1;
			const int dot2 = (pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b + pPixels[i + 2].a * saxis_a) + i + 2;
			const int dot3 = (pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b + pPixels[i + 3].a * saxis_a) + i + 3;

			int min_d01 = basisu::minimum(dot0, dot1);
			int max_d01 = basisu::maximum(dot0, dot1);

			int min_d23 = basisu::minimum(dot2, dot3);
			int max_d23 = basisu::maximum(dot2, dot3);

			int min_d = basisu::minimum(min_d01, min_d23);
			int max_d = basisu::maximum(max_d01, max_d23);

			low_dot = basisu::minimum(low_dot, min_d);
			high_dot = basisu::maximum(high_dot, max_d);
		}

		const int low_c = low_dot & 15;
		const int high_c = high_dot & 15;

		int p0, p1, lr, lg, lb, la, hr, hg, hb, ha;

		if (flags & cPackBC7FlagPBitOptMode6)
		{
			const float q = 1.0f / 255.0f;
			float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, (float)pPixels[low_c].a * q };
			float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, (float)pPixels[high_c].a * q };

			color_rgba bestMinColor, bestMaxColor;
			uint32_t best_pbits[2];
			determine_unique_pbits(4, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

			p0 = best_pbits[0], p1 = best_pbits[1];
			lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b, la = bestMinColor.a;
			hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b, ha = bestMaxColor.a;
		}
		else
		{
			p0 = pPixels[low_c].a > 128;
			p1 = pPixels[high_c].a > 128;

			lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0), la = to_7(pPixels[low_c].a, p0);
			hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1), ha = to_7(pPixels[high_c].a, p1);
		}

		uint8_t cur_weights[16];
		eval_weights_mode6_rgba(pPixels, cur_weights,
			lr, lg, lb, la, p0,
			hr, hg, hb, ha, p1);

		vec4F xl, xh;
		bool res = compute_least_squares_endpoints_4D(
			16, cur_weights, 16,
			g_bc7_4bit_ls_tab,
			xl, xh,
			pPixels,
			(float)total_r, (float)total_g, (float)total_b, (float)total_a);

		if (res)
		{
			if (flags & cPackBC7FlagPBitOptMode6)
			{
				const float q = 1.0f / 255.0f;
				float sxl[4] = { xl[0] * q, xl[1] * q, xl[2] * q, xl[3] * q };
				float sxh[4] = { xh[0] * q, xh[1] * q, xh[2] * q, xh[3] * q };

				color_rgba bestMinColor, bestMaxColor;
				uint32_t best_pbits[2];
				determine_unique_pbits(4, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

				p0 = best_pbits[0], p1 = best_pbits[1];
				lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b, la = bestMinColor.a;
				hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b, ha = bestMaxColor.a;
			}
			else
			{
				p0 = (xl[3] >= 129.0f);
				lr = to_7(xl[0], p0);
				lg = to_7(xl[1], p0);
				lb = to_7(xl[2], p0);
				la = to_7(xl[3], p0);

				p1 = (xh[3] >= 129.0f);
				hr = to_7(xh[0], p1);
				hg = to_7(xh[1], p1);
				hb = to_7(xh[2], p1);
				ha = to_7(xh[3], p1);
			}

			eval_weights_mode6_rgba(pPixels, cur_weights,
				lr, lg, lb, la, p0,
				hr, hg, hb, ha, p1);
		}

		encode_mode6_rgba_block(pBlock,
			lr, lg, lb, la, p0,
			hr, hg, hb, ha, p1,
			cur_weights);
	}

	uint32_t fast_pack_bc7_rgba_partial_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags)
	{
		assert(g_bc7_4bit_ls_tab[1][0]);

#if BASISU_BC7F_PERF_STATS
		g_total_rgba_calls++;
#endif

		const uint32_t fc = *(const uint32_t*)&pPixels[0];
		if (fc == *(const uint32_t*)&pPixels[15])
		{
			int k;
			for (k = 1; k < 15; k++)
				if (*(const uint32_t*)&pPixels[k] != fc)
					break;

			if (k == 15)
			{
				pack_mode5_solid(pBlock, pPixels[0]);
				return 0;
			}
		}

		int total_r = 0, total_g = 0, total_b = 0, total_a = 0;
		int min_r = 255, min_g = 255, min_b = 255, min_a = 255;
		int max_r = 0, max_g = 0, max_b = 0, max_a = 0;

		for (uint32_t i = 0; i < 16; i++)
		{
			int r = pPixels[i].r, g = pPixels[i].g, b = pPixels[i].b, a = pPixels[i].a;

			total_r += r; total_g += g; total_b += b; total_a += a;

			min_r = basisu::minimum(min_r, r); min_g = basisu::minimum(min_g, g); min_b = basisu::minimum(min_b, b); min_a = basisu::minimum(min_a, a);
			max_r = basisu::maximum(max_r, r); max_g = basisu::maximum(max_g, g); max_b = basisu::maximum(max_b, b); max_a = basisu::maximum(max_a, a);
		}

		assert((min_r != max_r) || (min_g != max_g) || (min_b != max_b) || (min_a != max_a));

		const int mean_r = (total_r + 8) >> 4, mean_g = (total_g + 8) >> 4, mean_b = (total_b + 8) >> 4, mean_a = (total_a + 8) >> 4;

		// covar rows are:
		int icov4[10] = { };

		// 0=rr
		// 1=rg
		// 2=rb
		// 3=ra
		// 
		// 4=gg
		// 5=gb
		// 6=ga
		// 
		// 7=bb
		// 8=ba
		// 
		// 9=aa

		// 0 1 2 3
		//   4 5 6
		//     7 8
		//       9

		// 0 1 2 3
		// 1 4 5 6
		// 2 5 7 8
		// 3 6 8 9

		// trace at 0,4,7,9

		for (uint32_t i = 0; i < 16; i++)
		{
			const int r = (int)pPixels[i].r - mean_r, g = (int)pPixels[i].g - mean_g, b = (int)pPixels[i].b - mean_b, a = (int)pPixels[i].a - mean_a;

			icov4[0] += r * r; icov4[1] += r * g; icov4[2] += r * b; icov4[3] += r * a;
			icov4[4] += g * g; icov4[5] += g * b; icov4[6] += g * a;
			icov4[7] += b * b; icov4[8] += b * a;
			icov4[9] += a * a;
		}

		const int block_max_var4 = basisu::maximum(icov4[0], icov4[4], icov4[7], icov4[9]); // not divided by 16, i.e. scaled by 16
		assert(block_max_var4); // solid blocks already filtered out

		// check for dual plane, if a single component is very strongly decorrelated then switch to modes 4/5
		int desired_dp_chan = -1;

		const bool non_analytical_flag = (flags & cPackBC7FlagNonAnalyticalRGBA) != 0;
		if (non_analytical_flag)
		{
			assert(flags & cPackBC7FlagPartiallyAnalyticalRGBA);
		}

		if ((flags & cPackBC7FlagUseDualPlaneRGBA) &&
			((!non_analytical_flag && (block_max_var4 >= DP_BLOCK_VAR_THRESH_RGBA)) || (non_analytical_flag && (block_max_var4 > 16)))
			)
		{
			// Prefer A, if not strongly decorrelated then check RGB.
			const float r_var = (float)icov4[0], g_var = (float)icov4[4], b_var = (float)icov4[7], a_var = (float)icov4[9];

			const bool has_a = icov4[9] > 0;

			if (has_a)
			{
				const float p_03 = icov4[0] ? fabs((float)icov4[3] / sqrtf(r_var * a_var)) : 1.0f;
				const float p_13 = icov4[4] ? fabs((float)icov4[6] / sqrtf(g_var * a_var)) : 1.0f;
				const float p_23 = icov4[7] ? fabs((float)icov4[8] / sqrtf(b_var * a_var)) : 1.0f;

				const float min_p = basisu::minimum(p_03, p_13, p_23);
				if (min_p < ALPHA_DECORR_THRESHOLD)
				{
					desired_dp_chan = 3;
#if BASISU_BC7F_PERF_STATS
					g_total_dp_valid_chans_a++;
#endif
				}
			}

			if (desired_dp_chan < 0)
			{
				const bool has_r = icov4[0] > 16, has_g = icov4[4] > 16, has_b = icov4[7] > 16;
				const uint32_t total_active_chans_rgb = has_r + has_g + has_b;

				if (total_active_chans_rgb >= 2)
				{
					const float rg_corr = (has_r && has_g) ? fabs((float)icov4[1] / sqrtf(r_var * g_var)) : 1.0f;
					const float rb_corr = (has_r && has_b) ? fabs((float)icov4[2] / sqrtf(r_var * b_var)) : 1.0f;
					const float gb_corr = (has_g && has_b) ? fabs((float)icov4[5] / sqrtf(g_var * b_var)) : 1.0f;

					float min_p = basisu::minimum(rg_corr, rb_corr, gb_corr);

					const float decorr_thresh = non_analytical_flag ? .999f : STRONG_DECORR_THRESH_RGBA;
					if (min_p < decorr_thresh)
					{
						if (total_active_chans_rgb == 2)
						{
							if (!has_r)
								desired_dp_chan = 1;
							else if (!has_g)
								desired_dp_chan = 0;
							else
								desired_dp_chan = 0;
						}
						else
						{
							// see if rg/rb is weakly correlated vs. gb
							if ((rg_corr < gb_corr) && (rb_corr < gb_corr))
								desired_dp_chan = 0;
							// see if gr/gb is weakly correlated vs. rb
							else if ((rg_corr < rb_corr) && (gb_corr < rb_corr))
								desired_dp_chan = 1;
							// assume b is weakest
							else
								desired_dp_chan = 2;
						}
#if BASISU_BC7F_PERF_STATS
						g_total_dp_valid_chans_rgb++;
#endif
					}
				}
			}
		}

		if ((flags & cPackBC7FlagUseTrivialMode6) && ((desired_dp_chan == -1) && (block_max_var4 < TRIVIAL_BLOCK_THRESH_RGBA)))
		{
			int low_c = INT_MAX, high_c = 0;

			for (uint32_t i = 0; i < 16; i++)
			{
				int y = ((16 * 2) * pPixels[i].r + (16 * 4) * pPixels[i].g + 16 * pPixels[i].b + (16 * 4) * pPixels[i].a);
				assert((y & 0xF) == 0);
				y += i;
				low_c = basisu::minimum(low_c, y);
				high_c = basisu::maximum(high_c, y);
			}

			low_c &= 0xF;
			high_c &= 0xF;

			int p0, p1, lr, lg, lb, la, hr, hg, hb, ha;

			if (flags & cPackBC7FlagPBitOptMode6)
			{
				const float q = 1.0f / 255.0f;
				float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, (float)pPixels[low_c].a * q };
				float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, (float)pPixels[high_c].a * q };

				color_rgba bestMinColor, bestMaxColor;
				uint32_t best_pbits[2];
				determine_unique_pbits(4, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

				p0 = best_pbits[0], p1 = best_pbits[1];
				lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b, la = bestMinColor.a;
				hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b, ha = bestMaxColor.a;
			}
			else
			{
				p0 = pPixels[low_c].a > 128;
				p1 = pPixels[high_c].a > 128;

				lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0), la = to_7(pPixels[low_c].a, p0);
				hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1), ha = to_7(pPixels[high_c].a, p1);
			}

			uint8_t cur_weights[16];
			uint32_t mode6_actual_sse = eval_weights_mode6_rgba_sse(pPixels, cur_weights,
				lr, lg, lb, la, p0,
				hr, hg, hb, ha, p1);

			encode_mode6_rgba_block(pBlock,
				lr, lg, lb, la, p0,
				hr, hg, hb, ha, p1,
				cur_weights);

#if BASISU_BC7F_PERF_STATS
			g_total_trivial_mode6_blocks++;
#endif

#ifdef _DEBUG
			{
				// Final sanity checking.
				uint32_t expected_actual_sse = calc_sse(pBlock, pPixels);
				assert(expected_actual_sse == mode6_actual_sse);
			}
#endif

			return mode6_actual_sse;
		}

		float cov4[10];
		for (uint32_t i = 0; i < 10; i++)
			cov4[i] = (float)icov4[i];

		// all channel pairs:
		// 0,1=1
		// 0,2=2
		// 0,3=3
		// 1,2=5
		// 1,3=6
		// 2,3=8
		//const float r_var = cov4[0], g_var = cov4[4], b_var = cov4[7], a_var = cov4[9];

		const float sc4 = block_max_var4 ? (1.0f / (float)block_max_var4) : 0;
		float wx = sc4 * cov4[0], wy = sc4 * cov4[4], wz = sc4 * cov4[7], wa = sc4 * cov4[9];

		// 0 1 2 3
		// 1 4 5 6
		// 2 5 7 8
		// 3 6 8 9

		// TODO
		float x1, y1, z1, w1;
		for (uint32_t i = 0; i < 4; i++)
		{
			x1 = cov4[0] * wx + cov4[1] * wy + cov4[2] * wz + cov4[3] * wa;
			y1 = cov4[1] * wx + cov4[4] * wy + cov4[5] * wz + cov4[6] * wa;
			z1 = cov4[2] * wx + cov4[5] * wy + cov4[7] * wz + cov4[8] * wa;
			w1 = cov4[3] * wx + cov4[6] * wy + cov4[8] * wz + cov4[9] * wa;

			float t = sqrtf(x1 * x1 + y1 * y1 + z1 * z1 + w1 * w1);
			if (t > basisu::SMALL_FLOAT_VAL)
			{
				t = 1.0f / t;
				x1 *= t; y1 *= t; z1 *= t; w1 *= t;
			}
			else
			{
				x1 = y1 = z1 = w1 = .25f;
			}

			wx = x1; wy = y1; wz = z1; wa = w1;
		}

		// Fall back to mode 6
		int saxis_r = 256, saxis_g = 256, saxis_b = 256, saxis_a = 256;

		float k = basisu::maximum(fabsf(x1), fabsf(y1), fabsf(z1), fabsf(w1));
		if (fabs(k) >= basisu::SMALL_FLOAT_VAL)
		{
			float m = 2048.0f / k;
			saxis_r = (int)(x1 * m);
			saxis_g = (int)(y1 * m);
			saxis_b = (int)(z1 * m);
			saxis_a = (int)(w1 * m);
		}

		saxis_r = (int)((uint32_t)saxis_r << 4U);
		saxis_g = (int)((uint32_t)saxis_g << 4U);
		saxis_b = (int)((uint32_t)saxis_b << 4U);
		saxis_a = (int)((uint32_t)saxis_a << 4U);

		int low_dot = INT_MAX, high_dot = INT_MIN;

		for (uint32_t i = 0; i < 16; i += 4)
		{
			assert(((pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b + pPixels[i].a * saxis_a) & 0xF) == 0); // sanity
			assert(((pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b + pPixels[i + 1].a * saxis_a) & 0xF) == 0);
			assert(((pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b + pPixels[i + 2].a * saxis_a) & 0xF) == 0);
			assert(((pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b + pPixels[i + 3].a * saxis_a) & 0xF) == 0);

			const int dot0 = (pPixels[i].r * saxis_r + pPixels[i].g * saxis_g + pPixels[i].b * saxis_b + pPixels[i].a * saxis_a) + i;
			const int dot1 = (pPixels[i + 1].r * saxis_r + pPixels[i + 1].g * saxis_g + pPixels[i + 1].b * saxis_b + pPixels[i + 1].a * saxis_a) + i + 1;
			const int dot2 = (pPixels[i + 2].r * saxis_r + pPixels[i + 2].g * saxis_g + pPixels[i + 2].b * saxis_b + pPixels[i + 2].a * saxis_a) + i + 2;
			const int dot3 = (pPixels[i + 3].r * saxis_r + pPixels[i + 3].g * saxis_g + pPixels[i + 3].b * saxis_b + pPixels[i + 3].a * saxis_a) + i + 3;

			int min_d01 = basisu::minimum(dot0, dot1);
			int max_d01 = basisu::maximum(dot0, dot1);

			int min_d23 = basisu::minimum(dot2, dot3);
			int max_d23 = basisu::maximum(dot2, dot3);

			int min_d = basisu::minimum(min_d01, min_d23);
			int max_d = basisu::maximum(max_d01, max_d23);

			low_dot = basisu::minimum(low_dot, min_d);
			high_dot = basisu::maximum(high_dot, max_d);
		}

		const int low_c = low_dot & 15;
		const int high_c = high_dot & 15;

		int p0, p1, lr, lg, lb, la, hr, hg, hb, ha;

		if (flags & cPackBC7FlagPBitOptMode6)
		{
			const float q = 1.0f / 255.0f;
			float sxl[4] = { (float)pPixels[low_c].r * q, (float)pPixels[low_c].g * q, (float)pPixels[low_c].b * q, (float)pPixels[low_c].a * q };
			float sxh[4] = { (float)pPixels[high_c].r * q, (float)pPixels[high_c].g * q, (float)pPixels[high_c].b * q, (float)pPixels[high_c].a * q };

			color_rgba bestMinColor, bestMaxColor;
			uint32_t best_pbits[2];
			determine_unique_pbits(4, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

			p0 = best_pbits[0], p1 = best_pbits[1];
			lr = bestMinColor.r, lg = bestMinColor.g, lb = bestMinColor.b, la = bestMinColor.a;
			hr = bestMaxColor.r, hg = bestMaxColor.g, hb = bestMaxColor.b, ha = bestMaxColor.a;
		}
		else
		{
			p0 = pPixels[low_c].a > 128;
			p1 = pPixels[high_c].a > 128;

			lr = to_7(pPixels[low_c].r, p0), lg = to_7(pPixels[low_c].g, p0), lb = to_7(pPixels[low_c].b, p0), la = to_7(pPixels[low_c].a, p0);
			hr = to_7(pPixels[high_c].r, p1), hg = to_7(pPixels[high_c].g, p1), hb = to_7(pPixels[high_c].b, p1), ha = to_7(pPixels[high_c].a, p1);
		}

		uint8_t cur_weights[16];
		uint32_t mode6_actual_sse = eval_weights_mode6_rgba_sse(pPixels, cur_weights,
			lr, lg, lb, la, p0,
			hr, hg, hb, ha, p1);

		if (mode6_actual_sse)
		{
			vec4F xl, xh;
			bool res = compute_least_squares_endpoints_4D(
				16, cur_weights, 16,
				g_bc7_4bit_ls_tab,
				xl, xh,
				pPixels,
				(float)total_r, (float)total_g, (float)total_b, (float)total_a);

			if (res)
			{
				int trial_p0, trial_p1, trial_lr, trial_lg, trial_lb, trial_la, trial_hr, trial_hg, trial_hb, trial_ha;

				if (flags & cPackBC7FlagPBitOptMode6)
				{
					const float q = 1.0f / 255.0f;
					float sxl[4] = { xl[0] * q, xl[1] * q, xl[2] * q, xl[3] * q };
					float sxh[4] = { xh[0] * q, xh[1] * q, xh[2] * q, xh[3] * q };

					color_rgba bestMinColor, bestMaxColor;
					uint32_t best_pbits[2];
					determine_unique_pbits(4, 7, sxl, sxh, bestMinColor, bestMaxColor, best_pbits);

					trial_p0 = best_pbits[0], trial_p1 = best_pbits[1];
					trial_lr = bestMinColor.r, trial_lg = bestMinColor.g, trial_lb = bestMinColor.b, trial_la = bestMinColor.a;
					trial_hr = bestMaxColor.r, trial_hg = bestMaxColor.g, trial_hb = bestMaxColor.b, trial_ha = bestMaxColor.a;
				}
				else
				{
					trial_p0 = (xl[3] >= 129.0f);
					trial_lr = to_7(xl[0], trial_p0);
					trial_lg = to_7(xl[1], trial_p0);
					trial_lb = to_7(xl[2], trial_p0);
					trial_la = to_7(xl[3], trial_p0);

					trial_p1 = (xh[3] >= 129.0f);
					trial_hr = to_7(xh[0], trial_p1);
					trial_hg = to_7(xh[1], trial_p1);
					trial_hb = to_7(xh[2], trial_p1);
					trial_ha = to_7(xh[3], trial_p1);
				}

				uint8_t trial_weights[16];
				uint32_t mode6_ls_actual_sse = eval_weights_mode6_rgba_sse(pPixels, trial_weights,
					trial_lr, trial_lg, trial_lb, trial_la, trial_p0,
					trial_hr, trial_hg, trial_hb, trial_ha, trial_p1);

				if (mode6_ls_actual_sse < mode6_actual_sse)
				{
					mode6_actual_sse = mode6_ls_actual_sse;
					memcpy(cur_weights, trial_weights, 16);
					p0 = trial_p0; p1 = trial_p1;
					lr = trial_lr; lg = trial_lg; lb = trial_lb; la = trial_la;
					hr = trial_hr; hg = trial_hg; hb = trial_hb; ha = trial_ha;
				}
			}
		}

		uint32_t mode7_actual_sse = UINT32_MAX;
		uint8_t mode7_candidate_block[sizeof(basist::bc7_block)];

		uint32_t mode45_actual_sse = UINT32_MAX;
		uint8_t mode45_candidate_block[sizeof(basist::bc7_block)];

		if (mode6_actual_sse)
		{
			if (non_analytical_flag)
			{
				// No gates: very expensive.
				if (flags & cPackBC7FlagUse2SubsetsRGBA)
				{
					pack_mode7_rgba(mode7_candidate_block, pPixels, x1, y1, z1, w1, mean_r, mean_g, mean_b, mean_a, 1e+9f, flags, nullptr, &mode7_actual_sse);
				}

				if (flags & cPackBC7FlagUseDualPlaneRGBA)
					pack_mode4_or_5(mode45_candidate_block, pPixels, (desired_dp_chan >= 0) ? desired_dp_chan : 3, 1e+9f, flags, nullptr, &mode45_actual_sse); // todo: determine best def channel here
			}
			else
			{
				float mode6_ortho_ratio;
				const float mode6_slam_to_line_sse_est = estimate_slam_to_line_sse_4D(cov4, x1, y1, z1, w1, &mode6_ortho_ratio);

				if ((flags & cPackBC7FlagUse2SubsetsRGBA) && (block_max_var4 >= MIN_BLOCK_MAX_VAR_23SUBSETS_RGBA) && (mode6_ortho_ratio > ORTHO_RATIO_23SUBSET_RATIO_THRESH_RGBA))
				{
					const bool high_ortho_energy_flag = (mode6_slam_to_line_sse_est >= HIGH_ORTHO_ENERGY_THRESH_RGBA);

					if (high_ortho_energy_flag)
					{
#if BASISU_BC7F_PERF_STATS
						g_total_high_ortho_energy++;
#endif
						pack_mode7_rgba(mode7_candidate_block, pPixels, x1, y1, z1, w1, mean_r, mean_g, mean_b, mean_a, 1e+9f, flags, nullptr, &mode7_actual_sse);
					}
				}

				if (desired_dp_chan >= 0)
					pack_mode4_or_5(mode45_candidate_block, pPixels, desired_dp_chan, 1e+9f, flags, nullptr, &mode45_actual_sse);
			}
		}

		const uint32_t best_actual_sse = basisu::minimum(mode6_actual_sse, mode45_actual_sse, mode7_actual_sse);

		if ((mode45_actual_sse != UINT32_MAX) && (best_actual_sse == mode45_actual_sse))
		{
			memcpy(pBlock, mode45_candidate_block, sizeof(basist::bc7_block));
		}
		else if ((mode7_actual_sse != UINT32_MAX) && (best_actual_sse == mode7_actual_sse))
		{
			memcpy(pBlock, mode7_candidate_block, sizeof(basist::bc7_block));
		}
		else
		{
			assert(mode6_actual_sse == best_actual_sse);

			encode_mode6_rgba_block(pBlock,
				lr, lg, lb, la, p0,
				hr, hg, hb, ha, p1,
				cur_weights);
		}

#ifdef _DEBUG
		{
			// Final sanity checking.
			uint32_t expected_actual_sse = calc_sse(pBlock, pPixels);
			assert(expected_actual_sse == best_actual_sse);
		}
#endif

		return best_actual_sse;
	}

	// Routes to either rgb or rgba automatically
	uint32_t fast_pack_bc7_auto_rgba(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags)
	{
		for (uint32_t i = 0; i < 16; i += 4)
		{
			if ((pPixels[i].a < 255) || (pPixels[i + 1].a < 255) || (pPixels[i + 2].a < 255) || (pPixels[i + 3].a < 255))
			{
				if (flags & cPackBC7FlagPartiallyAnalyticalRGBA)
					return bc7f::fast_pack_bc7_rgba_partial_analytical(pBlock, pPixels, flags);
				else
				{
					bc7f::fast_pack_bc7_rgba_analytical(pBlock, pPixels, flags);
					return 0;
				}
			}
		}

		if (flags & cPackBC7FlagPartiallyAnalyticalRGB)
			return bc7f::fast_pack_bc7_rgb_partial_analytical(pBlock, pPixels, flags);
		else
		{
			bc7f::fast_pack_bc7_rgb_analytical(pBlock, pPixels, flags);
			return 0;
		}
	}

	// Source block cannot have alpha.
	uint32_t fast_pack_bc7_auto_rgb(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags)
	{
// Disabling this check here, because during fuzzing the ktx2 file may lie. In this case it's harmless to output an opaque block.
#if 0
#if defined(DEBUG) || defined(_DEBUG)
		for (uint32_t i = 0; i < 16; i += 4)
		{
			if ((pPixels[i].a < 255) || (pPixels[i + 1].a < 255) || (pPixels[i + 2].a < 255) || (pPixels[i + 3].a < 255))
			{
				// Block can't have alpha here, or the solid color detectors may misfire.
				assert(0);
			}
		}
#endif
#endif

		if (flags & cPackBC7FlagPartiallyAnalyticalRGB)
			return bc7f::fast_pack_bc7_rgb_partial_analytical(pBlock, pPixels, flags);
		else
		{
			bc7f::fast_pack_bc7_rgb_analytical(pBlock, pPixels, flags);
			return 0;
		}
	}

	void clear_perf_stats()
	{
#if BASISU_BC7F_PERF_STATS
#define BU_CLEAR_BLOCK_STAT(x) x = 0;
		BU_CLEAR_BLOCK_STAT(g_total_rgb_calls);
		BU_CLEAR_BLOCK_STAT(g_total_rgba_calls);
		BU_CLEAR_BLOCK_STAT(g_total_solid_blocks);
		BU_CLEAR_BLOCK_STAT(g_total_trivial_mode6_blocks);
		BU_CLEAR_BLOCK_STAT(g_total_dp_valid_chans_rgb);
		BU_CLEAR_BLOCK_STAT(g_total_dp_valid_chans_a);
		BU_CLEAR_BLOCK_STAT(g_total_high_ortho_energy);
		BU_CLEAR_BLOCK_STAT(g_total_mode02_evals);
		BU_CLEAR_BLOCK_STAT(g_total_mode02_bailouts);
		BU_CLEAR_BLOCK_STAT(g_total_mode13_evals);
		BU_CLEAR_BLOCK_STAT(g_total_mode13_bailouts);
		BU_CLEAR_BLOCK_STAT(g_total_mode45_evals);
		BU_CLEAR_BLOCK_STAT(g_total_mode45_bailouts);
		BU_CLEAR_BLOCK_STAT(g_total_mode7_evals);
		BU_CLEAR_BLOCK_STAT(g_total_mode7_bailouts);
#undef BU_CLEAR_BLOCK_STAT
#endif
	}

	void print_perf_stats()
	{
#if BASISU_BC7F_PERF_STATS
		const uint32_t total_bc7_blocks = g_total_rgb_calls + g_total_rgba_calls;
		if (!total_bc7_blocks)
			return;

#define BU_PRINT_BLOCK_STAT(x) printf(#x ": %u %3.2f%%\n", (uint32_t)x, static_cast<float>(x) * 100.0f / (float)total_bc7_blocks);
		BU_PRINT_BLOCK_STAT(g_total_rgb_calls);
		BU_PRINT_BLOCK_STAT(g_total_rgba_calls);
		BU_PRINT_BLOCK_STAT(g_total_solid_blocks);
		BU_PRINT_BLOCK_STAT(g_total_trivial_mode6_blocks);
		BU_PRINT_BLOCK_STAT(g_total_dp_valid_chans_rgb);
		BU_PRINT_BLOCK_STAT(g_total_dp_valid_chans_a);
		BU_PRINT_BLOCK_STAT(g_total_high_ortho_energy);
		BU_PRINT_BLOCK_STAT(g_total_mode02_evals);
		BU_PRINT_BLOCK_STAT(g_total_mode02_bailouts);
		BU_PRINT_BLOCK_STAT(g_total_mode13_evals);
		BU_PRINT_BLOCK_STAT(g_total_mode13_bailouts);
		BU_PRINT_BLOCK_STAT(g_total_mode45_evals);
		BU_PRINT_BLOCK_STAT(g_total_mode45_bailouts);
		BU_PRINT_BLOCK_STAT(g_total_mode7_evals);
		BU_PRINT_BLOCK_STAT(g_total_mode7_bailouts);
#undef BU_PRINT_BLOCK_STAT

#endif
	}

} // namespace bc7f

namespace bc7u
{
	int dequant_weight(uint32_t w, uint32_t num_weight_bits)
	{
		switch (num_weight_bits)
		{
		case 2: assert(w < 4); return basist::g_bc7_weights2[w];
		case 3: assert(w < 8); return basist::g_bc7_weights3[w];
		case 4: assert(w < 16); return basist::g_bc7_weights4[w];
		default:
			assert(0);
			break;
		}
		return 0;
	}

	uint8_t g_weight_quant[3][65]; // [2-4][0-64] - maps [0,64] normalized weight to nearest quantized value

	void init_weight_quant()
	{
		for (uint32_t num_weight_bits = 2; num_weight_bits <= 4; num_weight_bits++)
		{
			const uint32_t num_weight_vals = 1 << num_weight_bits;

			for (int de = 0; de <= 64; de++)
			{
				uint32_t best_idx = 0, best_err = UINT32_MAX;

				for (uint32_t idx = 0; idx < num_weight_vals; idx++)
				{
					const int dequant = dequant_weight(idx, num_weight_bits);

					const uint32_t err = basisu::iabs(de - dequant);
					if (err < best_err)
					{
						best_err = err;
						best_idx = idx;
					}
				} // idx

				g_weight_quant[num_weight_bits - 2][de] = (uint8_t)best_idx;

			} // de

		} // num_bits
	}

	// 0-8 bits only
	inline uint8_t fetch_bits(const void* p, uint32_t num_bits, uint32_t& bit_ofs)
	{
		if (!num_bits)
			return 0;

		assert((num_bits >= 1) && (num_bits <= 8));

		if ((bit_ofs + num_bits) > 128)
		{
			assert(0);
			return 0;
		}

		const uint8_t* pBytes = static_cast<const uint8_t*>(p);

		const uint32_t byte_ofs = bit_ofs >> 3;

		const uint8_t b0 = pBytes[byte_ofs];
		const uint8_t b1 = pBytes[basisu::minimum<uint32_t>(15, byte_ofs + 1)];

		const uint32_t b = (b0 | (b1 << 8)) >> (bit_ofs & 7);

		const uint32_t res = b & ((1u << num_bits) - 1u);

		bit_ofs += num_bits;

		return static_cast<uint8_t>(res);
	}

	bool unpack_bc7(const void* pPhys_block, log_bc7_block& log_blk)
	{
		log_blk.clear();

		const int mode = determine_bc7_mode(pPhys_block);
		log_blk.m_mode = (int8_t)mode;
		if (mode < 0)
			return false;

		uint32_t bit_ofs = mode + 1;

		switch (mode)
		{
		case 0:
		case 2:
		{
			log_blk.m_num_partitions = 3;
			log_blk.m_pattern_bits = (mode == 0) ? 4 : 6;
			log_blk.m_num_planes = 1;
			log_blk.m_endpoint_bits[0] = (mode == 0) ? 4 : 5;
			log_blk.m_num_pbits = (mode == 0) ? 6 : 0;
			log_blk.m_weight_bits[0] = (mode == 0) ? 3 : 2;

			log_blk.m_pattern_index = fetch_bits(pPhys_block, log_blk.m_pattern_bits, bit_ofs);

			for (uint32_t c = 0; c < 3; c++)
				for (uint32_t s = 0; s < 3; s++)
					for (uint32_t e = 0; e < 2; e++)
						log_blk.m_endpoints[s][e][c] = fetch_bits(pPhys_block, log_blk.m_endpoint_bits[0], bit_ofs);

			for (uint32_t p = 0; p < log_blk.m_num_pbits; p++)
				log_blk.m_pbits[p] = fetch_bits(pPhys_block, 1, bit_ofs);

			for (uint32_t i = 0; i < 16; i++)
			{
				log_blk.m_weights[0][i] = fetch_bits(pPhys_block,
					((!i) || (i == basist::g_bc7_table_anchor_index_third_subset_1[log_blk.m_pattern_index]) || (i == basist::g_bc7_table_anchor_index_third_subset_2[log_blk.m_pattern_index])) ? (log_blk.m_weight_bits[0] - 1) : log_blk.m_weight_bits[0],
					bit_ofs);
			}

			assert(bit_ofs == 128);

			break;
		}
		case 1:
		case 3:
		case 7:
		{
			log_blk.m_num_partitions = 2;
			log_blk.m_pattern_bits = 6;
			log_blk.m_num_planes = 1;
			log_blk.m_endpoint_bits[0] = (mode == 7) ? 5 : ((mode == 1) ? 6 : 7);
			log_blk.m_endpoint_bits[1] = (mode == 7) ? 5 : 0;
			log_blk.m_num_pbits = (mode == 1) ? 2 : 4;
			log_blk.m_shared_pbits = (mode == 1);
			log_blk.m_weight_bits[0] = (mode == 1) ? 3 : 2;

			log_blk.m_pattern_index = fetch_bits(pPhys_block, log_blk.m_pattern_bits, bit_ofs);

			const uint32_t num_comps = (mode == 7) ? 4 : 3;

			for (uint32_t c = 0; c < num_comps; c++)
				for (uint32_t s = 0; s < 2; s++)
					for (uint32_t e = 0; e < 2; e++)
						log_blk.m_endpoints[s][e][c] = fetch_bits(pPhys_block, log_blk.m_endpoint_bits[0], bit_ofs);

			for (uint32_t p = 0; p < log_blk.m_num_pbits; p++)
				log_blk.m_pbits[p] = fetch_bits(pPhys_block, 1, bit_ofs);

			for (uint32_t i = 0; i < 16; i++)
			{
				log_blk.m_weights[0][i] = fetch_bits(pPhys_block,
					((!i) || (i == basist::g_bc7_table_anchor_index_second_subset[log_blk.m_pattern_index])) ? (log_blk.m_weight_bits[0] - 1) : log_blk.m_weight_bits[0],
					bit_ofs);
			}

			assert(bit_ofs == 128);

			break;
		}
		case 4:
		case 5:
		{
			log_blk.m_num_partitions = 1;
			log_blk.m_num_planes = 2;
			log_blk.m_endpoint_bits[0] = (mode == 4) ? 5 : 7;
			log_blk.m_endpoint_bits[1] = (mode == 4) ? 6 : 8;
			log_blk.m_weight_bits[0] = 2;
			log_blk.m_weight_bits[1] = (mode == 4) ? 3 : 2;

			log_blk.m_dp_rotation_index = fetch_bits(pPhys_block, 2, bit_ofs);

			log_blk.m_mode4_index_selector = (mode == 4) ? fetch_bits(pPhys_block, 1, bit_ofs) : 0;

			for (uint32_t c = 0; c < 4; c++)
				for (uint32_t e = 0; e < 2; e++)
					log_blk.m_endpoints[0][e][c] = fetch_bits(pPhys_block, log_blk.m_endpoint_bits[(c == 3) ? 1 : 0], bit_ofs);

			for (uint32_t plane_index = 0; plane_index < 2; plane_index++)
			{
				for (uint32_t i = 0; i < 16; i++)
				{
					log_blk.m_weights[plane_index][i] = fetch_bits(pPhys_block,
						(!i) ? (log_blk.m_weight_bits[plane_index] - 1) : log_blk.m_weight_bits[plane_index],
						bit_ofs);
				}
			}

			assert(bit_ofs == 128);

			break;
		}
		case 6:
		{
			log_blk.m_num_partitions = 1;
			log_blk.m_num_planes = 1;
			log_blk.m_endpoint_bits[0] = 7;
			log_blk.m_endpoint_bits[1] = 7;
			log_blk.m_num_pbits = 2;
			log_blk.m_weight_bits[0] = 4;

			for (uint32_t c = 0; c < 4; c++)
			{
				log_blk.m_endpoints[0][0][c] = fetch_bits(pPhys_block, 7, bit_ofs);
				log_blk.m_endpoints[0][1][c] = fetch_bits(pPhys_block, 7, bit_ofs);
			}

			for (uint32_t p = 0; p < 2; p++)
				log_blk.m_pbits[p] = fetch_bits(pPhys_block, 1, bit_ofs);

			for (uint32_t w = 0; w < 16; w++)
				log_blk.m_weights[0][w] = fetch_bits(pPhys_block, (w == 0) ? 3 : 4, bit_ofs);

			assert(bit_ofs == 128);

			break;
		}
		default:
		{
			assert(0);
			break;
		}
		}

		return true;
	}

	void unpack_endpoints(const log_bc7_block& log_blk, color_rgba pEndpoints[2], uint32_t subset)
	{
		assert(subset < log_blk.m_num_partitions);

		const uint32_t num_comps = log_blk.get_num_comps();

		pEndpoints[0].a = 255;
		pEndpoints[1].a = 255;

		for (uint32_t e = 0; e < 2; e++)
		{
			for (uint32_t c = 0; c < num_comps; c++)
			{
				if (log_blk.m_num_pbits)
					pEndpoints[e][c] = (uint8_t)bc7_dequant(log_blk.m_endpoints[subset][e][c], log_blk.m_pbits[log_blk.m_shared_pbits ? subset : (subset * 2 + e)], log_blk.m_endpoint_bits[c == 3]);
				else
					pEndpoints[e][c] = (uint8_t)bc7_dequant(log_blk.m_endpoints[subset][e][c], log_blk.m_endpoint_bits[c == 3]);
			} // c
		} // e 
	}

	const endpoint_format g_endpoint_formats[8] =
	{
		{ 4, 0, 2 }, // 0 (unique pbits)
		{ 6, 0, 1 }, // 1 (shared pbits)
		{ 5, 0, 0 }, // 2
		{ 7, 0, 2 }, // 3 (unique pbits)

		{ 5, 6, 0 }, // 4
		{ 7, 8, 0 }, // 5
		{ 7, 7, 2 }, // 6 (unique pbits)
		{ 5, 5, 2 }  // 7 (unique pbits)
	};

	// Important note: Uses float math
	void pack_endpoints(
		uint32_t mode_index,
		const color_rgba pSrc_endpoints[2],
		color_rgba pDst_endpoints[2], uint8_t pDst_pbits[2])
	{
		assert(mode_index < 8);
		const auto& fmt = g_endpoint_formats[mode_index];

		const uint32_t num_rgb_bits = fmt.m_num_rgb_bits, num_a_bits = fmt.m_num_a_bits, num_pbits = fmt.m_num_pbits;
		const uint32_t num_comps = num_a_bits ? 4 : 3;

		const float S = 1.0f / 255.0f;

		float xl[4], xh[4];
		for (uint32_t c = 0; c < 4; c++)
		{
			xl[c] = (float)pSrc_endpoints[0][c] * S;
			xh[c] = (float)pSrc_endpoints[1][c] * S;
		}

		if (num_pbits == 0)
		{
			const float rgb_scale = (float)((1 << num_rgb_bits) - 1);
			const float a_scale = (float)((1 << num_a_bits) - 1);

			for (uint32_t c = 0; c < 4; c++)
			{
				const float scale = (c == 3) ? a_scale : rgb_scale;

				pDst_endpoints[0][c] = (uint8_t)basisu::clamp<int>(fast_roundf_int(xl[c] * scale), 0, 255);
				pDst_endpoints[1][c] = (uint8_t)basisu::clamp<int>(fast_roundf_int(xh[c] * scale), 0, 255);
			}

			pDst_pbits[0] = 0;
			pDst_pbits[1] = 0;
		}
		else if (num_pbits == 1)
		{
			assert(mode_index == 1);

			color_rgba bestMinColor, bestMaxColor;
			uint32_t best_pbits[2] = {};

			bc7f::determine_shared_pbits(num_comps, num_rgb_bits, xl, xh, bestMinColor, bestMaxColor, best_pbits); // always 3 comps

			for (uint32_t c = 0; c < 4; c++)
			{
				pDst_endpoints[0][c] = bestMinColor[c];
				pDst_endpoints[1][c] = bestMaxColor[c];
			}
			pDst_pbits[0] = (uint8_t)best_pbits[0];
			pDst_pbits[1] = (uint8_t)best_pbits[0];
		}
		else
		{
			assert(num_pbits == 2);

			color_rgba bestMinColor, bestMaxColor;
			uint32_t best_pbits[2] = {};

			bc7f::determine_unique_pbits(num_comps, num_rgb_bits, xl, xh, bestMinColor, bestMaxColor, best_pbits);

			for (uint32_t c = 0; c < 4; c++)
			{
				pDst_endpoints[0][c] = bestMinColor[c];
				pDst_endpoints[1][c] = bestMaxColor[c];
			}

			pDst_pbits[0] = (uint8_t)best_pbits[0];
			pDst_pbits[1] = (uint8_t)best_pbits[1];
		}
	}

	// Quantize an 8-bit value to a (comp_bits+1)-bit BC7 endpoint lattice
	// point with parity p (the pbit), round to nearest. 
	static inline int quant_endpoint_pbit(uint32_t v8, int p, int iscalep)
	{
		const int k = ((int)v8 * iscalep + 255 - 255 * p) / 510;
		return basisu::clamp<int>(k * 2 + p, p, iscalep - 1 + p);
	}

	// No-pbit modes (2, 4, 5): round-to-nearest into the field range.
	// Exact rational form of fast_roundf_int(v/255 * maxv).
	static inline uint32_t quant_endpoint(uint32_t v8, uint32_t num_bits)
	{
		const uint32_t maxv = (1u << num_bits) - 1u;
		return (v8 * maxv * 2u + 255u) / 510u;
	}

	// Exact integer determine_unique_pbits.
	static inline void determine_unique_pbits_int(
		uint32_t total_comps, uint32_t comp_bits, const uint8_t xl[4], const uint8_t xh[4],
		color_rgba& bestMinColor, color_rgba& bestMaxColor, uint32_t best_pbits[2])
	{
		const uint32_t total_bits = comp_bits + 1;
		const int iscalep = (1 << total_bits) - 1;

		uint64_t best_err0 = UINT64_MAX, best_err1 = UINT64_MAX;

		for (int p = 0; p < 2; p++)
		{
			int xMin[4], xMax[4], sL[4], sH[4];

			for (uint32_t c = 0; c < 4; c++)
			{
				xMin[c] = quant_endpoint_pbit(xl[c], p, iscalep);
				xMax[c] = quant_endpoint_pbit(xh[c], p, iscalep);
								
				sL[c] = (xMin[c] << (8 - total_bits)); sL[c] |= (sL[c] >> total_bits);
				sH[c] = (xMax[c] << (8 - total_bits)); sH[c] |= (sH[c] >> total_bits);
			}

			uint64_t err0 = 0, err1 = 0;
			for (uint32_t i = 0; i < total_comps; i++)
			{
				const int64_t d0 = sL[i] - (int)xl[i], d1 = sH[i] - (int)xh[i];
				err0 += (uint64_t)(d0 * d0);
				err1 += (uint64_t)(d1 * d1);
			}

			if (err0 < best_err0)
			{
				best_err0 = err0;
				best_pbits[0] = (uint32_t)p;
				for (uint32_t j = 0; j < 4; j++)
					bestMinColor[j] = (uint8_t)(xMin[j] >> 1);
			}

			if (err1 < best_err1)
			{
				best_err1 = err1;
				best_pbits[1] = (uint32_t)p;
				for (uint32_t j = 0; j < 4; j++)
					bestMaxColor[j] = (uint8_t)(xMax[j] >> 1);
			}
		}
	}

	// Exact integer determine_shared_pbits; same notes as above.
	static inline void determine_shared_pbits_int(
		uint32_t total_comps, uint32_t comp_bits, const uint8_t xl[4], const uint8_t xh[4],
		color_rgba& bestMinColor, color_rgba& bestMaxColor, uint32_t best_pbits[2])
	{
		const uint32_t total_bits = comp_bits + 1;
		const int iscalep = (1 << total_bits) - 1;

		uint64_t best_err = UINT64_MAX;

		for (int p = 0; p < 2; p++)
		{
			int xMin[4], xMax[4], sL[4], sH[4];

			for (uint32_t c = 0; c < 4; c++)
			{
				xMin[c] = quant_endpoint_pbit(xl[c], p, iscalep);
				xMax[c] = quant_endpoint_pbit(xh[c], p, iscalep);

				sL[c] = (xMin[c] << (8 - total_bits)); sL[c] |= (sL[c] >> total_bits);
				sH[c] = (xMax[c] << (8 - total_bits)); sH[c] |= (sH[c] >> total_bits);
			}

			uint64_t err = 0;
			for (uint32_t i = 0; i < total_comps; i++)
			{
				const int64_t d0 = sL[i] - (int)xl[i], d1 = sH[i] - (int)xh[i];
				err += (uint64_t)(d0 * d0 + d1 * d1);
			}

			if (err < best_err)
			{
				best_err = err;
				best_pbits[0] = (uint32_t)p;
				best_pbits[1] = (uint32_t)p;
				for (uint32_t j = 0; j < 4; j++)
				{
					bestMinColor[j] = (uint8_t)(xMin[j] >> 1);
					bestMaxColor[j] = (uint8_t)(xMax[j] >> 1);
				}
			}
		}
	}

	// Deterministic pack_endpoints: same shape as pack_endpoints() except all
	// integer (no float math at all). Used to requantize a predictor block's dequantized 8-bit RGBA
	// endpoints into the current block's mode format for endpoint DPCM.
	static inline void pack_endpoints_int(
		uint32_t mode_index,
		const color_rgba pSrc_endpoints[2], // 8-bit (unpacked)
		color_rgba pDst_endpoints[2], uint8_t pDst_pbits[2])
	{
		assert(mode_index < 8);
		const basist::bc7u::endpoint_format& fmt = basist::bc7u::g_endpoint_formats[mode_index];

		const uint32_t num_comps = fmt.m_num_a_bits ? 4 : 3;

		pDst_pbits[0] = 0;
		pDst_pbits[1] = 0;

		if (fmt.m_num_pbits == 0)
		{
			for (uint32_t e = 0; e < 2; e++)
			{
				for (uint32_t c = 0; c < 4; c++)
				{
					const uint32_t nbits = (c == 3) ? fmt.m_num_a_bits : fmt.m_num_rgb_bits;
					pDst_endpoints[e][c] = (uint8_t)(nbits ? quant_endpoint(pSrc_endpoints[e][c], nbits) : 0);
				}
			}
		}
		else
		{
			uint8_t l8[4], h8[4];
			for (uint32_t c = 0; c < 4; c++)
			{
				l8[c] = pSrc_endpoints[0][c];
				h8[c] = pSrc_endpoints[1][c];
			}

			uint32_t pbits[2] = { 0, 0 };

			if (fmt.m_num_pbits == 1)
				determine_shared_pbits_int(num_comps, fmt.m_num_rgb_bits, l8, h8, pDst_endpoints[0], pDst_endpoints[1], pbits);
			else
				determine_unique_pbits_int(num_comps, fmt.m_num_rgb_bits, l8, h8, pDst_endpoints[0], pDst_endpoints[1], pbits);

			pDst_pbits[0] = (uint8_t)pbits[0];
			pDst_pbits[1] = (uint8_t)pbits[1];
		}
	}

	// No float math here
	void endpoint_dpcm(
		bool decode_flag,
		const log_bc7_block& predictor_log_blk, uint32_t predictor_subset_index, // the logical block we are predicting from
		log_bc7_block& log_blk, uint32_t subset_index, // the logical block which has the endpoint we want to code/decode
		uint8_t* pResiduals, uint32_t& num_residuals, uint8_t *pBits, uint32_t &num_pbits) // the DPCM encoded values, in RR, GG, BB, AA order
	{
		assert(predictor_log_blk.is_valid());
		assert(log_blk.is_valid());

		const uint32_t mode_index = log_blk.m_mode;
		assert(mode_index < 8);

		const auto& fmt = g_endpoint_formats[mode_index];

		color_rgba pred_endpoints[2];
		unpack_endpoints(predictor_log_blk, pred_endpoints, predictor_subset_index);

		if (predictor_log_blk.is_dual_plane())
		{
			// convert to plain RGBA (A=the channel in the 2nd plane, we need to undo that)

			const uint32_t pred_ccs_index = predictor_log_blk.get_color_component_selector();
						
			std::swap(pred_endpoints[0][pred_ccs_index], pred_endpoints[0][3]);
			std::swap(pred_endpoints[1][pred_ccs_index], pred_endpoints[1][3]); 
		}

		const int ccs_index = log_blk.get_color_component_selector();

		if (log_blk.is_dual_plane())
		{
			// Convert from plain RGBA - put the channel on the 2nd plane into A
			std::swap(pred_endpoints[0][ccs_index], pred_endpoints[0][3]);
			std::swap(pred_endpoints[1][ccs_index], pred_endpoints[1][3]);
		}

		// Now pack our predicted endpoints into the local block's domain
		color_rgba packed_endpoints[2];
		uint8_t packed_pbits[2];
		pack_endpoints_int(log_blk.m_mode, pred_endpoints, packed_endpoints, packed_pbits);

		const uint32_t num_comps = log_blk.get_num_comps(); // 3 or 4

		num_residuals = num_comps * 2;
		num_pbits = fmt.m_num_pbits; // num pbits per subset
		assert(num_pbits * log_blk.m_num_partitions == log_blk.m_num_pbits);
				
		// subtract G from RB to mostly decorrelate (G unmodified) - but first, determine where G and A actually are taking into account dual plane modes
		int g_channel = 1, a_channel = 3;
		if (log_blk.is_dual_plane())
		{
			assert(num_comps == 4);

			a_channel = ccs_index;

			if (ccs_index == 1)
				g_channel = 3;
		}
		else
		{
			assert(num_comps >= 3);
		}

		if (!decode_flag)
		{
			// Compute deltas
			for (uint32_t c = 0; c < num_comps; c++)
			{
				pResiduals[c * 2 + 0] = (uint8_t)(log_blk.m_endpoints[subset_index][0][c] - packed_endpoints[0][c]);
				pResiduals[c * 2 + 1] = (uint8_t)(log_blk.m_endpoints[subset_index][1][c] - packed_endpoints[1][c]);
			}

			for (uint32_t c = 0; c < num_comps; c++)
			{
				if (((int)c == g_channel) || ((int)c == a_channel))
					continue;

				pResiduals[c * 2 + 0] = (uint8_t)(pResiduals[c * 2 + 0] - pResiduals[g_channel * 2 + 0]);
				pResiduals[c * 2 + 1] = (uint8_t)(pResiduals[c * 2 + 1] - pResiduals[g_channel * 2 + 1]);
			}

			// Compute delta pbits
			for (uint32_t p = 0; p < num_pbits; p++)
			{
				pBits[p] = (log_blk.m_pbits[subset_index * num_pbits + p] - packed_pbits[p]) & 1;
			}
		}
		else
		{
			// Don't modify the originals
			assert((num_comps * 2) <= 8);

			uint8_t temp_residuals[8];
			memcpy(temp_residuals, pResiduals, num_comps * 2);

			for (uint32_t c = 0; c < num_comps; c++)
			{
				if (((int)c == g_channel) || ((int)c == a_channel))
					continue;

				temp_residuals[c * 2 + 0] = (uint8_t)(temp_residuals[c * 2 + 0] + temp_residuals[g_channel * 2 + 0]);
				temp_residuals[c * 2 + 1] = (uint8_t)(temp_residuals[c * 2 + 1] + temp_residuals[g_channel * 2 + 1]);
			}

			for (uint32_t c = 0; c < num_comps; c++)
			{
				const uint32_t num_bits = log_blk.m_endpoint_bits[c == 3];
				const uint32_t mask = (1 << num_bits) - 1;

				log_blk.m_endpoints[subset_index][0][c] = (uint8_t)((temp_residuals[c * 2 + 0] + packed_endpoints[0][c]) & mask);
				log_blk.m_endpoints[subset_index][1][c] = (uint8_t)((temp_residuals[c * 2 + 1] + packed_endpoints[1][c]) & mask);
			}

			for (uint32_t p = 0; p < num_pbits; p++)
			{
				log_blk.m_pbits[subset_index * num_pbits + p] = (pBits[p] + packed_pbits[p]) & 1;
			}
		}
	}

	void init_log_blk(log_bc7_block& log_blk, uint32_t mode)
	{
		log_blk.clear();

		const auto& fmt = g_endpoint_formats[mode];

		log_blk.m_mode = (int8_t)mode;

		log_blk.m_num_partitions = basist::g_bc7_num_subsets[mode];
		log_blk.m_num_planes = ((mode == 4) || (mode == 5)) ? 2 : 1;
		log_blk.m_num_pbits = log_blk.m_num_partitions * fmt.m_num_pbits;
		log_blk.m_pattern_bits = basist::g_bc7_partition_bits[mode];
		log_blk.m_endpoint_bits[0] = fmt.m_num_rgb_bits;
		log_blk.m_endpoint_bits[1] = fmt.m_num_a_bits;
		log_blk.m_weight_bits[0] = g_bc7_color_index_bitcount[mode];
		log_blk.m_weight_bits[1] = (log_blk.m_num_planes == 2) ? g_bc7_alpha_index_bitcount[mode] : 0;
		log_blk.m_shared_pbits = (fmt.m_num_pbits == 1);
	}

	void create_solid_blk(log_bc7_block& log_blk, const color_rgba& c)
	{
		init_log_blk(log_blk, 5);

		const uint32_t lr = basist::g_bc7_mode_5_optimal_endpoints[c[0]].m_lo;
		const uint32_t hr = basist::g_bc7_mode_5_optimal_endpoints[c[0]].m_hi;

		const uint32_t lg = basist::g_bc7_mode_5_optimal_endpoints[c[1]].m_lo;
		const uint32_t hg = basist::g_bc7_mode_5_optimal_endpoints[c[1]].m_hi;

		const uint32_t lb = basist::g_bc7_mode_5_optimal_endpoints[c[2]].m_lo;
		const uint32_t hb = basist::g_bc7_mode_5_optimal_endpoints[c[2]].m_hi;

		log_blk.m_endpoints[0][0][0] = (uint8_t)lr;
		log_blk.m_endpoints[0][1][0] = (uint8_t)hr;

		log_blk.m_endpoints[0][0][1] = (uint8_t)lg;
		log_blk.m_endpoints[0][1][1] = (uint8_t)hg;

		log_blk.m_endpoints[0][0][2] = (uint8_t)lb;
		log_blk.m_endpoints[0][1][2] = (uint8_t)hb;

		log_blk.m_endpoints[0][0][3] = c.a;
		log_blk.m_endpoints[0][1][3] = c.a;

		memset(log_blk.m_weights[0], 1, 16);
	}

	bool is_solid_blk(const log_bc7_block& log_blk)
	{
		assert(log_blk.is_valid());

		// we could check to see if each subset's endpoints are all equal too
		if (log_blk.m_num_partitions > 1)
			return false;

		for (uint32_t p = 0; p < log_blk.m_num_planes; p++)
		{
			for (uint32_t w = 1; w < 16; w++)
				if (log_blk.m_weights[p][0] != log_blk.m_weights[p][w])
					return false;
		}

		return true;
	}

	bool validate_log_blk(const log_bc7_block& log_blk)
	{
		int mode = log_blk.m_mode;
		if ((mode < 0) || (mode > 7))
			return false;

		log_bc7_block temp_log_blk;
		init_log_blk(temp_log_blk, mode);

		if (log_blk.m_num_partitions != temp_log_blk.m_num_partitions)
			return false;
		if (log_blk.m_num_planes != temp_log_blk.m_num_planes)
			return false;
		if (log_blk.m_num_pbits != temp_log_blk.m_num_pbits)
			return false;
		if (log_blk.m_pattern_bits != temp_log_blk.m_pattern_bits)
			return false;
		if (log_blk.m_endpoint_bits[0] != temp_log_blk.m_endpoint_bits[0])
			return false;
		if (log_blk.m_endpoint_bits[1] != temp_log_blk.m_endpoint_bits[1])
			return false;
		if (log_blk.m_weight_bits[0] != temp_log_blk.m_weight_bits[0])
			return false;
		if (log_blk.m_weight_bits[1] != temp_log_blk.m_weight_bits[1])
			return false;
		if (log_blk.m_shared_pbits != temp_log_blk.m_shared_pbits)
			return false;

		if (log_blk.m_pattern_index >= (1u << log_blk.m_pattern_bits))
			return false;
		
		if (log_blk.m_num_planes == 2)
		{
			if (log_blk.m_dp_rotation_index >= 4)
				return false;
		}
		else
		{
			if (log_blk.m_dp_rotation_index > 0)
				return false;
		}

		for (uint32_t s = 0; s < log_blk.m_num_partitions; s++)
		{
			for (uint32_t e = 0; e < 2; e++)
			{
				for (uint32_t c = 0; c < 4; c++)
				{
					if (log_blk.m_endpoints[s][e][c] >= (1u << log_blk.m_endpoint_bits[c == 3]))
						return false;
				}
			}
		}

		for (uint32_t p = 0; p < log_blk.m_num_planes; p++)
		{
			const uint32_t total_weights = 1u << log_blk.m_weight_bits[p];

			for (uint32_t w = 0; w < 16; w++)
			{
				if (log_blk.m_weights[p][w] >= total_weights)
					return false;
			}
		}

		return true;
	}

	void set_endpoints(log_bc7_block& log_blk, uint32_t subset, const color_rgba pEndpoints[2], const uint8_t pbits[2])
	{
		assert(subset < log_blk.m_num_partitions);

		for (uint32_t e = 0; e < 2; e++)
		{
			for (uint32_t c = 0; c < 4; c++)
			{
				const uint32_t num_endpoint_bits = log_blk.m_endpoint_bits[c == 3];

				if (!num_endpoint_bits)
				{
					log_blk.m_endpoints[subset][e][c] = 0;
				}
				else
				{
					assert(pEndpoints[e][c] < (1u << num_endpoint_bits));
					log_blk.m_endpoints[subset][e][c] = pEndpoints[e][c];
				}
			}
		}

		if (log_blk.m_num_pbits)
		{
			const uint32_t num_pbits_per_subset = log_blk.get_num_pbits_per_subset();

			for (uint32_t p = 0; p < num_pbits_per_subset; p++)
			{
				assert(pbits[p] <= 1);
				log_blk.m_pbits[subset * num_pbits_per_subset + p] = pbits[p];
			}
		}
	}

	bool unpack_bc7(const log_bc7_block& log_blk, color_rgba* pPixels)
	{
		if (log_blk.m_mode < 0)
		{
			assert(0);
			memset((void *)pPixels, 0, sizeof(color_rgba) * 16);
			return false;
		}

		const uint32_t num_comps = log_blk.get_num_comps();

		color_rgba endpoints[3][2]; // [subset][color_index];
		
		for (uint32_t subset = 0; subset < log_blk.m_num_partitions; subset++)
			unpack_endpoints(log_blk, endpoints[subset], subset);

		for (uint32_t i = 0; i < 16; i++)
		{
			uint32_t subset = 0;

			if (log_blk.m_num_partitions == 2)
				subset = basist::g_bc7_partition2[log_blk.m_pattern_index * 16 + i];
			else if (log_blk.m_num_partitions == 3)
				subset = basist::g_bc7_partition3[log_blk.m_pattern_index * 16 + i];

			color_rgba res;
			
			for (uint32_t c = 0; c < 3; c++)
			{
				res[c] = (uint8_t)bc7_interp(endpoints[subset][0][c], endpoints[subset][1][c], 
					log_blk.m_weights[log_blk.m_mode4_index_selector][i], log_blk.m_weight_bits[log_blk.m_mode4_index_selector]);
			}

			res.a = 255;

			if (num_comps == 4)
			{
				if (log_blk.m_num_planes == 2)
				{
					res.a = (uint8_t)bc7_interp(endpoints[subset][0][3], endpoints[subset][1][3],
						log_blk.m_weights[1 - log_blk.m_mode4_index_selector][i], log_blk.m_weight_bits[1 - log_blk.m_mode4_index_selector]);
				}
				else
				{
					res.a = (uint8_t)bc7_interp(endpoints[subset][0][3], endpoints[subset][1][3],
						log_blk.m_weights[0][i], log_blk.m_weight_bits[0]);
				}
			}

			if (log_blk.m_dp_rotation_index)
				std::swap(res.m_comps[3], res.m_comps[log_blk.m_dp_rotation_index - 1]);

			pPixels[i] = res;
		}
	
		return true;
	}

	bool unpack_bc7_texel(const log_bc7_block& log_blk, color_rgba& pixel, uint32_t x, uint32_t y)
	{
		if (log_blk.m_mode < 0)
		{
			assert(0);
			pixel.clear();
			return false;
		}

		const uint32_t num_comps = log_blk.get_num_comps();

		const uint32_t i = x + y * 4;
		assert(i < 16);
				
		uint32_t subset = 0;

		if (log_blk.m_num_partitions == 2)
			subset = basist::g_bc7_partition2[log_blk.m_pattern_index * 16 + i];
		else if (log_blk.m_num_partitions == 3)
			subset = basist::g_bc7_partition3[log_blk.m_pattern_index * 16 + i];

		color_rgba endpoints[2];
		unpack_endpoints(log_blk, endpoints, subset);

		color_rgba res;

		for (uint32_t c = 0; c < 3; c++)
		{
			res[c] = (uint8_t)bc7_interp(endpoints[0][c], endpoints[1][c],
				log_blk.m_weights[log_blk.m_mode4_index_selector][i], log_blk.m_weight_bits[log_blk.m_mode4_index_selector]);
		}

		res.a = 255;

		if (num_comps == 4)
		{
			if (log_blk.m_num_planes == 2)
			{
				res.a = (uint8_t)bc7_interp(endpoints[0][3], endpoints[1][3],
					log_blk.m_weights[1 - log_blk.m_mode4_index_selector][i], log_blk.m_weight_bits[1 - log_blk.m_mode4_index_selector]);
			}
			else
			{
				res.a = (uint8_t)bc7_interp(endpoints[0][3], endpoints[1][3],
					log_blk.m_weights[0][i], log_blk.m_weight_bits[0]);
			}
		}

		if (log_blk.m_dp_rotation_index)
			std::swap(res.m_comps[3], res.m_comps[log_blk.m_dp_rotation_index - 1]);

		pixel = res;

		return true;
	}

	uint32_t get_texel_subset(log_bc7_block& log_blk, uint32_t x, uint32_t y)
	{
		const uint32_t i = x + y * 4;
		assert(i < 16);

		uint32_t subset = 0;

		if (log_blk.m_num_partitions == 2)
			subset = basist::g_bc7_partition2[log_blk.m_pattern_index * 16 + i];
		else if (log_blk.m_num_partitions == 3)
			subset = basist::g_bc7_partition3[log_blk.m_pattern_index * 16 + i];

		return subset;
	}

	// swaps endpoints and adjusts weights as needed to ensure endpoint 0 r+g+b+a < endpoint 1 r+g+b+a (we can do this safely as pack_bc7() will do the inverse as needed)
	void canonicalize_endpoints(log_bc7_block& log_blk)
	{
		assert(log_blk.is_valid());

		if (log_blk.m_num_planes > 1)
		{
			assert(log_blk.m_num_partitions == 1);
			assert(log_blk.m_num_pbits == 0);

			color_rgba e[2];
			unpack_endpoints(log_blk, e, 0);
						
			for (uint32_t p = 0; p < log_blk.m_num_planes; p++)
			{
				uint32_t l0 = 0, l1 = 0;

				for (uint32_t c = 0; c < 4; c++)
				{
					if (log_blk.get_endpoint_channel_weight_plane(c) == p)
					{
						l0 += e[0][c];
						l1 += e[1][c];
					}
				}

				if (l0 <= l1)
					continue;

				for (uint32_t c = 0; c < 4; c++)
				{
					if (log_blk.get_endpoint_channel_weight_plane(c) == p)
						std::swap(log_blk.m_endpoints[0][0][c], log_blk.m_endpoints[0][1][c]);
				}

				const uint32_t max_weight_val = log_blk.get_num_weight_vals(p) - 1;
				for (uint32_t w = 0; w < 16; w++)
					log_blk.m_weights[p][w] = (uint8_t)(max_weight_val - log_blk.m_weights[p][w]);
			}
		}
		else
		{
			const uint32_t num_comps = log_blk.get_num_comps();

			for (uint32_t s = 0; s < log_blk.m_num_partitions; s++)
			{
				color_rgba e[2];
				unpack_endpoints(log_blk, e, s);

				uint32_t l0 = 0, l1 = 0;
				for (uint32_t c = 0; c < num_comps; c++)
				{
					l0 += e[0][c];
					l1 += e[1][c];
				}

				if (l0 <= l1)
					continue;
				
				for (uint32_t c = 0; c < num_comps; c++)
					std::swap(log_blk.m_endpoints[s][0][c], log_blk.m_endpoints[s][1][c]);

				const uint32_t num_pbits_per_subset = log_blk.get_num_pbits_per_subset();
				if (num_pbits_per_subset == 2)
					std::swap(log_blk.m_pbits[s * 2 + 0], log_blk.m_pbits[s * 2 + 1]);

				const uint32_t max_weight_val = log_blk.get_num_weight_vals(0) - 1;
			
				if (log_blk.m_num_partitions == 1)
				{
					for (uint32_t w = 0; w < 16; w++)
						log_blk.m_weights[0][w] = (uint8_t)(max_weight_val - log_blk.m_weights[0][w]);
				}
				else
				{
					for (uint32_t w = 0; w < 16; w++)
					{
						const uint32_t texel_s = get_texel_subset(log_blk, w & 3, w >> 2);
						if (texel_s == s)
						{
							log_blk.m_weights[0][w] = (uint8_t)(max_weight_val - log_blk.m_weights[0][w]);
						}
					}
				}
			}
		}
	}

	// This function will invert endpoints/weights as needed to pack them to a physical BC7 block, which is always lossless in decoded pixel space (but NOT in the latent itself).
	bool pack_bc7(const log_bc7_block& log_blk, void* pPhys_block)
	{
		const int mode = log_blk.m_mode;

		if (mode < 0)
		{
			assert(0);
			memset(pPhys_block, 0, 16);
			return false;
		}

		switch (mode)
		{
		case 0:
		{
			uint32_t lr[3], lg[3], lb[3], hr[3], hg[3], hb[3];

			for (uint32_t s = 0; s < 3; s++)
			{
				lr[s] = log_blk.m_endpoints[s][0][0];
				lg[s] = log_blk.m_endpoints[s][0][1];
				lb[s] = log_blk.m_endpoints[s][0][2];
				hr[s] = log_blk.m_endpoints[s][1][0];
				hg[s] = log_blk.m_endpoints[s][1][1];
				hb[s] = log_blk.m_endpoints[s][1][2];
			}

			uint32_t pbits[6];
			for (uint32_t p = 0; p < 6; p++)
				pbits[p] = log_blk.m_pbits[p];

			bc7f::encode_mode0_rgb_block((uint8_t *)pPhys_block, log_blk.m_pattern_index, lr, lg, lb, hr, hg, hb, pbits, log_blk.m_weights[0]);

			break;
		}
		case 2:
		{
			uint32_t lr[3], lg[3], lb[3], hr[3], hg[3], hb[3];

			for (uint32_t s = 0; s < 3; s++)
			{
				lr[s] = log_blk.m_endpoints[s][0][0];
				lg[s] = log_blk.m_endpoints[s][0][1];
				lb[s] = log_blk.m_endpoints[s][0][2];
				hr[s] = log_blk.m_endpoints[s][1][0];
				hg[s] = log_blk.m_endpoints[s][1][1];
				hb[s] = log_blk.m_endpoints[s][1][2];
			}

			bc7f::encode_mode2_rgb_block((uint8_t*)pPhys_block, log_blk.m_pattern_index, lr, lg, lb, hr, hg, hb, log_blk.m_weights[0]);

			break;
		}
		case 1:
		{
			uint32_t lr[2], lg[2], lb[2], hr[2], hg[2], hb[2];

			for (uint32_t s = 0; s < 2; s++)
			{
				lr[s] = log_blk.m_endpoints[s][0][0];
				lg[s] = log_blk.m_endpoints[s][0][1];
				lb[s] = log_blk.m_endpoints[s][0][2];
				hr[s] = log_blk.m_endpoints[s][1][0];
				hg[s] = log_blk.m_endpoints[s][1][1];
				hb[s] = log_blk.m_endpoints[s][1][2];
			}

			bc7f::encode_mode1_rgb_block((uint8_t *)pPhys_block, log_blk.m_pattern_index, 
				lr, lg, lb, hr, hg, hb, log_blk.m_pbits[0], log_blk.m_pbits[1], log_blk.m_weights[0]);

			break;
		}
		case 3:
		{
			uint32_t lr[2], lg[2], lb[2], hr[2], hg[2], hb[2];

			for (uint32_t s = 0; s < 2; s++)
			{
				lr[s] = log_blk.m_endpoints[s][0][0];
				lg[s] = log_blk.m_endpoints[s][0][1];
				lb[s] = log_blk.m_endpoints[s][0][2];
				hr[s] = log_blk.m_endpoints[s][1][0];
				hg[s] = log_blk.m_endpoints[s][1][1];
				hb[s] = log_blk.m_endpoints[s][1][2];
			}

			uint32_t pbits[4];
			for (uint32_t p = 0; p < 4; p++)
				pbits[p] = log_blk.m_pbits[p];

			bc7f::encode_mode3_rgb_block((uint8_t*)pPhys_block, log_blk.m_pattern_index,
				lr, lg, lb, hr, hg, hb, pbits, log_blk.m_weights[0]);

			break;
		}
		case 4:
		{
			bc7f::encode_mode4_rgba_block((uint8_t *)pPhys_block, 
				log_blk.m_endpoints[0][0][0], log_blk.m_endpoints[0][0][1], log_blk.m_endpoints[0][0][2], log_blk.m_endpoints[0][0][3],
				log_blk.m_endpoints[0][1][0], log_blk.m_endpoints[0][1][1], log_blk.m_endpoints[0][1][2], log_blk.m_endpoints[0][1][3],
				log_blk.m_weights[log_blk.m_mode4_index_selector],  // it always wants the RGB weights first, then A
				log_blk.m_weights[1 - log_blk.m_mode4_index_selector],
				log_blk.m_dp_rotation_index, log_blk.m_mode4_index_selector);

			break;
		}
		case 5:
		{
			bc7f::encode_mode5_rgba_block((uint8_t*)pPhys_block,
				log_blk.m_endpoints[0][0][0], log_blk.m_endpoints[0][0][1], log_blk.m_endpoints[0][0][2], log_blk.m_endpoints[0][0][3],
				log_blk.m_endpoints[0][1][0], log_blk.m_endpoints[0][1][1], log_blk.m_endpoints[0][1][2], log_blk.m_endpoints[0][1][3],
				log_blk.m_weights[0], log_blk.m_weights[1], log_blk.m_dp_rotation_index);

			break;
		}
		case 6:
		{
			bc7f::encode_mode6_rgba_block((uint8_t*)pPhys_block,
				log_blk.m_endpoints[0][0][0], log_blk.m_endpoints[0][0][1], log_blk.m_endpoints[0][0][2], log_blk.m_endpoints[0][0][3], log_blk.m_pbits[0],
				log_blk.m_endpoints[0][1][0], log_blk.m_endpoints[0][1][1], log_blk.m_endpoints[0][1][2], log_blk.m_endpoints[0][1][3], log_blk.m_pbits[1],
				log_blk.m_weights[0]);

			break;
		}
		case 7:
		{
			uint32_t lr[2], lg[2], lb[2], la[2], hr[2], hg[2], hb[2], ha[2];

			for (uint32_t s = 0; s < 2; s++)
			{
				lr[s] = log_blk.m_endpoints[s][0][0];
				lg[s] = log_blk.m_endpoints[s][0][1];
				lb[s] = log_blk.m_endpoints[s][0][2];
				la[s] = log_blk.m_endpoints[s][0][3];

				hr[s] = log_blk.m_endpoints[s][1][0];
				hg[s] = log_blk.m_endpoints[s][1][1];
				hb[s] = log_blk.m_endpoints[s][1][2];
				ha[s] = log_blk.m_endpoints[s][1][3];
			}

			uint32_t pbits[4];
			for (uint32_t p = 0; p < 4; p++)
				pbits[p] = log_blk.m_pbits[p];

			bc7f::encode_mode7_rgba_block((uint8_t*)pPhys_block, log_blk.m_pattern_index,
				lr, lg, lb, la, hr, hg, hb, ha, pbits, log_blk.m_weights[0]);

			break;
		}
		default:
		{
			assert(0);
			break;
		}
		}

		return true;
	}

	bool compare_block_configs(const log_bc7_block& a, const log_bc7_block& b, bool compare_partition_index)
	{
		assert(a.is_valid() && b.is_valid());
		
		if ((!a.is_valid()) || (!b.is_valid()))
			return false;
		
		if (a.m_mode != b.m_mode)
			return false;
		
		// The logical blocks may not necessarily be BC7 compliant, so validate the rest of the config.
		if (a.m_num_partitions != b.m_num_partitions)
			return false;
				
		if (a.m_pattern_bits != b.m_pattern_bits)
			return false;

		if (compare_partition_index)
		{
			if (a.m_pattern_index != b.m_pattern_index)
				return false;
		}

		if (a.m_num_planes != b.m_num_planes)
			return false;

		if (a.m_dp_rotation_index != b.m_dp_rotation_index)
			return false;

		if (a.m_mode4_index_selector != b.m_mode4_index_selector)
			return false;

		if ((a.m_endpoint_bits[0] != b.m_endpoint_bits[0]) || (a.m_endpoint_bits[1] != b.m_endpoint_bits[1]))
			return false;

		if ((a.m_weight_bits[0] != b.m_weight_bits[0]) || (a.m_weight_bits[1] != b.m_weight_bits[1]))
			return false;

		if (a.m_num_pbits != b.m_num_pbits)
			return false;

		if (a.m_shared_pbits != b.m_shared_pbits)
			return false;

		return true;
	}

	bool compare_block_endpoints(const log_bc7_block& a, const log_bc7_block& b)
	{
		if (a.m_num_partitions != b.m_num_partitions)
			return false;
		
		if (a.m_endpoint_bits[0] != b.m_endpoint_bits[0])
			return false;

		if (a.m_endpoint_bits[1] != b.m_endpoint_bits[1])
			return false;

		for (uint32_t s = 0; s < a.m_num_partitions; s++)
		{
			for (uint32_t e = 0; e < 2; e++)
			{
				for (uint32_t c = 0; c < 4; c++)
				{
					if (a.m_endpoints[s][e][c] != b.m_endpoints[s][e][c])
						return false;
				} // c
			} // e
		} // s

		return true;
	}

	bool compare_block_pbits(const log_bc7_block& a, const log_bc7_block& b)
	{
		if (a.m_num_pbits != b.m_num_pbits)
			return false;

		if (a.m_shared_pbits != b.m_shared_pbits)
			return false;

		for (uint32_t p = 0; p < a.m_num_pbits; p++)
		{
			if (a.m_pbits[p] != b.m_pbits[p])
				return false;
		} // s

		return true;
	}

	bool compare_block_weights(const log_bc7_block& a, const log_bc7_block& b)
	{
		if (a.m_num_planes != b.m_num_planes)
			return false;
		
		if (a.m_weight_bits[0] != b.m_weight_bits[0])
			return false;
		if (a.m_weight_bits[1] != b.m_weight_bits[1])
			return false;

		for (uint32_t p = 0; p < a.m_num_planes; p++)
		{
			for (uint32_t w = 0; w < 16; w++)
			{
				if (a.m_weights[p][w] != b.m_weights[p][w])
					return false;
			} // w
		} // s

		return true;
	}

	bool compare_block_full(const log_bc7_block& a, const log_bc7_block& b)
	{
		if (!compare_block_configs(a, b))
			return false;

		if (!compare_block_endpoints(a, b))
			return false;

		if (!compare_block_pbits(a, b))
			return false;

		if (!compare_block_weights(a, b))
			return false;

		return true;
	}

} // namespace bc7u

} // namespace basist
