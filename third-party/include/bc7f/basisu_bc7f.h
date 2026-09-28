// clang-format off
// basisu_bc7f.h - Self-contained BC7 block encoder extracted from Binomial LLC's
// basis_universal (https://github.com/BinomialLLC/basis_universal), commit 99f52d6.
// The code is the "bc7f" real-time analytical encoder from
// transcoder/basisu_transcoder.cpp (basist::bc7f, plus the basist::bc7u logical/physical
// BC7 block packing code it depends on), with only the minimal types/helpers it needs
// from transcoder/basisu.h, transcoder/basisu_transcoder_internal.h and
// transcoder/basisu_transcoder_uastc.h, so it can be built without the rest of the
// transcoder. The ASTC (fast_pack_astc) output path and the UASTC transcoding paths
// were removed during extraction. See LICENSE for the Apache 2.0 license.
#pragma once

#include <assert.h>
#include <stdint.h>
#include <string.h>

// Minimal subset of the basisu namespace helpers (from transcoder/basisu.h) needed
// by the color32/color_rgba inline methods below.
namespace basisu
{
	template <typename S> inline S clamp(S value, S low, S high) { return (value < low) ? low : ((value > high) ? high : value); }

	template <typename S> inline S maximum(S a, S b) { return (a > b) ? a : b; }
	template <typename S> inline S minimum(S a, S b) { return (a < b) ? a : b; }
}

namespace basist
{
	inline uint8_t clamp255(int32_t i)
	{
		return (uint8_t)((i & 0xFFFFFF00U) ? (~(i >> 31)) : i);
	}

	enum eNoClamp
	{
		cNoClamp = 0
	};

	struct color32
	{
		union
		{
			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};

			uint8_t c[4];
			
			uint32_t m;
		};

		color32() = default;

		// both constructors are no clamping now (not this classes original intention - optimization)
		color32(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }
		color32(eNoClamp unused, uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { (void)unused; set_noclamp_rgba(vr, vg, vb, va); }

		// no clamping
		void set(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { c[0] = static_cast<uint8_t>(vr); c[1] = static_cast<uint8_t>(vg); c[2] = static_cast<uint8_t>(vb); c[3] = static_cast<uint8_t>(va); }

		void set_noclamp_rgb(uint32_t vr, uint32_t vg, uint32_t vb) { c[0] = static_cast<uint8_t>(vr); c[1] = static_cast<uint8_t>(vg); c[2] = static_cast<uint8_t>(vb); }
		void set_noclamp_rgba(uint32_t vr, uint32_t vg, uint32_t vb, uint32_t va) { set(vr, vg, vb, va); }

		// clamped to [0,255]
		void set_clamped(int vr, int vg, int vb, int va) { c[0] = clamp255(vr); c[1] = clamp255(vg); c[2] = clamp255(vb); c[3] = clamp255(va); }

		uint8_t operator[] (uint32_t idx) const { assert(idx < 4); return c[idx]; }
		uint8_t &operator[] (uint32_t idx) { assert(idx < 4); return c[idx]; }

		bool operator== (const color32&rhs) const { return m == rhs.m; }

		static color32 comp_min(const color32& a, const color32& b) { return color32(cNoClamp, basisu::minimum(a[0], b[0]), basisu::minimum(a[1], b[1]), basisu::minimum(a[2], b[2]), basisu::minimum(a[3], b[3])); }
		static color32 comp_max(const color32& a, const color32& b) { return color32(cNoClamp, basisu::maximum(a[0], b[0]), basisu::maximum(a[1], b[1]), basisu::maximum(a[2], b[2]), basisu::maximum(a[3], b[3])); }
	};
	// This duplicates key functionality in the encoder library's color_rgba class. Porting and retesting code that uses it to color32 is impractical.
	class color_rgba
	{
	public:
		union
		{
			uint8_t m_comps[4];

			struct
			{
				uint8_t r;
				uint8_t g;
				uint8_t b;
				uint8_t a;
			};
		};

		inline color_rgba()
		{
			static_assert(sizeof(*this) == 4, "sizeof(*this) != 4");
			static_assert(sizeof(*this) == sizeof(color32), "sizeof(*this) != sizeof(basist::color32)");
		}

		inline color_rgba(const color32& other) :
			r(other.r),
			g(other.g),
			b(other.b),
			a(other.a)
		{
		}

		color_rgba& operator= (const basist::color32& rhs)
		{
			r = rhs.r;
			g = rhs.g;
			b = rhs.b;
			a = rhs.a;
			return *this;
		}

		inline color_rgba(int y)
		{
			set(y);
		}

		inline color_rgba(int y, int na)
		{
			set(y, na);
		}

		inline color_rgba(int sr, int sg, int sb, int sa)
		{
			set(sr, sg, sb, sa);
		}

		inline color_rgba(eNoClamp, int sr, int sg, int sb, int sa)
		{
			set_noclamp_rgba((uint8_t)sr, (uint8_t)sg, (uint8_t)sb, (uint8_t)sa);
		}

		inline color_rgba& set_noclamp_y(int y)
		{
			m_comps[0] = (uint8_t)y;
			m_comps[1] = (uint8_t)y;
			m_comps[2] = (uint8_t)y;
			m_comps[3] = (uint8_t)255;
			return *this;
		}

		inline color_rgba& set_noclamp_rgba(int sr, int sg, int sb, int sa)
		{
			m_comps[0] = (uint8_t)sr;
			m_comps[1] = (uint8_t)sg;
			m_comps[2] = (uint8_t)sb;
			m_comps[3] = (uint8_t)sa;
			return *this;
		}

		inline color_rgba& set(int y)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(y, 0, 255));
			m_comps[1] = m_comps[0];
			m_comps[2] = m_comps[0];
			m_comps[3] = 255;
			return *this;
		}

		inline color_rgba& set(int y, int na)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(y, 0, 255));
			m_comps[1] = m_comps[0];
			m_comps[2] = m_comps[0];
			m_comps[3] = static_cast<uint8_t>(basisu::clamp<int>(na, 0, 255));
			return *this;
		}

		inline color_rgba& set(int sr, int sg, int sb, int sa)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(sr, 0, 255));
			m_comps[1] = static_cast<uint8_t>(basisu::clamp<int>(sg, 0, 255));
			m_comps[2] = static_cast<uint8_t>(basisu::clamp<int>(sb, 0, 255));
			m_comps[3] = static_cast<uint8_t>(basisu::clamp<int>(sa, 0, 255));
			return *this;
		}

		inline color_rgba& set_rgb(int sr, int sg, int sb)
		{
			m_comps[0] = static_cast<uint8_t>(basisu::clamp<int>(sr, 0, 255));
			m_comps[1] = static_cast<uint8_t>(basisu::clamp<int>(sg, 0, 255));
			m_comps[2] = static_cast<uint8_t>(basisu::clamp<int>(sb, 0, 255));
			return *this;
		}

		inline color_rgba& set_rgb(const color_rgba& other)
		{
			r = other.r;
			g = other.g;
			b = other.b;
			return *this;
		}

		inline const uint8_t& operator[] (uint32_t index) const { assert(index < 4); return m_comps[index]; }
		inline uint8_t& operator[] (uint32_t index) { assert(index < 4); return m_comps[index]; }

		inline void clear()
		{
			m_comps[0] = 0;
			m_comps[1] = 0;
			m_comps[2] = 0;
			m_comps[3] = 0;
		}

		inline bool operator== (const color_rgba& rhs) const
		{
			if (m_comps[0] != rhs.m_comps[0]) return false;
			if (m_comps[1] != rhs.m_comps[1]) return false;
			if (m_comps[2] != rhs.m_comps[2]) return false;
			if (m_comps[3] != rhs.m_comps[3]) return false;
			return true;
		}

		inline bool operator!= (const color_rgba& rhs) const
		{
			return !(*this == rhs);
		}

		inline bool operator<(const color_rgba& rhs) const
		{
			for (int i = 0; i < 4; i++)
			{
				if (m_comps[i] < rhs.m_comps[i])
					return true;
				else if (m_comps[i] != rhs.m_comps[i])
					return false;
			}
			return false;
		}

		inline color32 get_color32() const
		{
			return color32(r, g, b, a);
		}

		inline int get_709_luma() const { return (13938U * m_comps[0] + 46869U * m_comps[1] + 4729U * m_comps[2] + 32768U) >> 16U; }
	};

	struct bc7_block
	{
		uint64_t m_qwords[2];
	};

	namespace bc7f
	{
		enum
		{
			// Low-level BC7 encoder configuration flags.
			cPackBC7FlagUse2SubsetsRGB = 1, // use mode 1/3 for RGB blocks
			cPackBC7FlagUse2SubsetsRGBA = 2, // use mode 7 for RGBA blocks

			cPackBC7FlagUse3SubsetsRGB = 4, // also use mode 0/2, cPackBC7FlagUse2SubsetsRGB MUST be enabled too

			cPackBC7FlagUseDualPlaneRGB = 8, // enable mode 4/5 usage for RGB blocks
			cPackBC7FlagUseDualPlaneRGBA = 16, // enable mode 4/5 usage for RGBA blocks

			cPackBC7FlagPBitOpt = 32, // enable to disable usage of fixed p-bits on some modes; slower
			cPackBC7FlagPBitOptMode6 = 64, // enable to disable usage of fixed p-bits on mode 6, alpha on fully opaque blocks may be 254 however; slower

			cPackBC7FlagUseTrivialMode6 = 128, // enable trivial fast mode 6 encoder on blocks with very low variances (highly recommended)

			cPackBC7FlagPartiallyAnalyticalRGB = 256, // partially analytical mode for RGB blocks, slower but higher quality, computes actual SSE's on complex blocks to resolve which mode to use vs. predictions
			cPackBC7FlagPartiallyAnalyticalRGBA = 512, // partially analytical mode for RGBA blocks, slower but higher quality, computes actual SSE's on complex blocks to resolve which mode to use vs. predictions

			// Non-analytical is really still partially analytical on the mode pairs (0 vs. 2, 1 vs 3, 4 vs. 5).
			cPackBC7FlagNonAnalyticalRGB = 1024, // very slow/brute force, totally abuses the encoder, MUST use with cPackBC7FlagPartiallyAnalyticalRGB flag
			cPackBC7FlagNonAnalyticalRGBA = 2048, // very slow/brute force, totally abuses the encoder, MUST use with cPackBC7FlagPartiallyAnalyticalRGBA flag

			cPackBC7FlagASTCCompatible = 4096, // disallow 2/3 partition patterns not in common with ASTC LDR 4x4

			cPackBC7FlagDisableRGBDualPlane = 8192, // don't use RGB dual plane channels (but A is OK)

			// Default to use first:

			// Decent analytical BC7 defaults
			cPackBC7FlagDefaultFastest = cPackBC7FlagUseTrivialMode6, // very weak particularly on alpha, mode 6 only for RGB/RGBA, 

			// Mode 6 with pbits for RGB, Modes 4,5,6 for alpha.
			cPackBC7FlagDefaultFaster = cPackBC7FlagPBitOpt | cPackBC7FlagUseDualPlaneRGBA | cPackBC7FlagUseTrivialMode6,

			cPackBC7FlagDefaultFast = cPackBC7FlagUse2SubsetsRGB | cPackBC7FlagUse2SubsetsRGBA | cPackBC7FlagUseDualPlaneRGBA |
				cPackBC7FlagPBitOpt | cPackBC7FlagUseTrivialMode6,

			// Reasonable defaults, but not highest quality but fast
			cPackBC7FlagDefault = (cPackBC7FlagUse2SubsetsRGB | cPackBC7FlagUse2SubsetsRGBA | cPackBC7FlagUse3SubsetsRGB) |
				(cPackBC7FlagUseDualPlaneRGB | cPackBC7FlagUseDualPlaneRGBA) |
				(cPackBC7FlagPBitOpt | cPackBC7FlagPBitOptMode6) |
				cPackBC7FlagUseTrivialMode6,

			// Default partially analytical BC7 defaults (slower)
			cPackBC7FlagDefaultPartiallyAnalytical = cPackBC7FlagDefault | (cPackBC7FlagPartiallyAnalyticalRGB | cPackBC7FlagPartiallyAnalyticalRGBA),

			// Default non-analytical BC7 defaults (very slow). In reality the encoder is still analytical on the mode pairs, but at the highest level is non-analytical.
			cPackBC7FlagDefaultNonAnalytical = (cPackBC7FlagDefaultPartiallyAnalytical | (cPackBC7FlagNonAnalyticalRGB | cPackBC7FlagNonAnalyticalRGBA)) & ~cPackBC7FlagUseTrivialMode6
		};

		void init();

		// Asumes block has NO ALPHA.
		void fast_pack_bc7_rgb_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);
		uint32_t fast_pack_bc7_rgb_partial_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);

		// Assumes block HAS ALPHA: Importantly, note if the block doesn't actually have alpha, this isn't what you want because it won't utilize the 2-3 subset RGB configs.
		void fast_pack_bc7_rgba_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);
		uint32_t fast_pack_bc7_rgba_partial_analytical(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);

		// Automatically determines if the block has alpha and dispatches to the appropriate function (RGB vs. RGBA).
		uint32_t fast_pack_bc7_auto_rgba(uint8_t* pBlock, const color_rgba* pPixels, uint32_t flags);

		void print_perf_stats();

		uint32_t calc_sse(const uint8_t* pBlock, const color_rgba* pPixels);

	} // namespace bc7f

} // namespace basist
