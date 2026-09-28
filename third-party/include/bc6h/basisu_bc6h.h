// clang-format off
// basisu_bc6h.h - Self-contained BC6H (unsigned) block encoder extracted from
// Binomial LLC's basis_universal (https://github.com/BinomialLLC/basis_universal).
// The code is the "bc6hf" real-time analytical encoder from
// transcoder/basisu_transcoder.cpp (basist::astc_6x6_hdr::fast_encode_bc6h),
// with only the minimal types/helpers it needs, so it can be built without the
// rest of the transcoder. See LICENSE for the Apache 2.0 license.
#pragma once

#include <stdint.h>

namespace basist
{
	typedef uint16_t half_float;

	extern const double MAX_HALF_FLOAT; // largest normal half-float value (65504.0)

	struct bc6h_block
	{
		uint8_t m_bytes[16];
	};

	namespace astc_6x6_hdr
	{
		const uint32_t BC6H_NUM_DIFF_ENDPOINT_MODES_TO_TRY_2 = 2;
		const uint32_t BC6H_NUM_DIFF_ENDPOINT_MODES_TO_TRY_4 = 4;
		const uint32_t BC6H_NUM_DIFF_ENDPOINT_MODES_TO_TRY_9 = 9;

		struct fast_bc6h_params
		{
			uint32_t m_num_diff_endpoint_modes_to_try;
			uint32_t m_max_2subset_pats_to_try;

			bool m_hq_ls;
			bool m_brute_force_weight4_assignment;

			fast_bc6h_params()
			{
				init();
			}

			void init()
			{
				m_hq_ls = true;
				m_num_diff_endpoint_modes_to_try = BC6H_NUM_DIFF_ENDPOINT_MODES_TO_TRY_2;
				m_max_2subset_pats_to_try = 1;
				m_brute_force_weight4_assignment = false;
			}
		};

		// One-time init of the encoder's internal tables. Must be called before fast_encode_bc6h().
		void fast_encode_bc6h_init();

		// Encodes to BC6H (unsigned variant).
		// pPixels: pointer to 16 RGB half-float/FP16 values (48 total half-floats), in raster order.
		// Max encodable value is (in float) basist::MAX_HALF_FLOAT.
		void fast_encode_bc6h(const basist::half_float* pPixels, basist::bc6h_block* pBlock, const fast_bc6h_params& params);

	} // namespace astc_6x6_hdr

} // namespace basist
