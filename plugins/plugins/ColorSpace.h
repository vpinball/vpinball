// license:GPLv3+

#pragma once

// sRGB transfer function helpers shared by the plugins that move DMD frames around.
//
// Display frame formats deliberately mix colorspaces: CTLPI_DISPLAY_FORMAT_LUM32F is linear luminance,
// SRGB888/SRGB565 is gamma encoded, and the legacy 0..100 brightness percentage of the VPinMAME controller is gamma encoded too.
//
// Every conversion here is a table lookup. The decodes are exact by construction. The encodes index a 4096 entry table by linear value, landing on the
// right output code or one either side, then step once onto it exactly - see LUT_SIZE for why one step always suffices, and the sentinels for why it is branchless

#include <cstdint>
#include <cfloat>
#include <cmath>

namespace VPXColorSpace
{

namespace detail
{

// sRGB EOTF, only ever evaluated while building the tables below
inline float sRGBToLinearF(const float c) { return (c <= 0.04045f) ? (c * (1.f / 12.92f)) : powf((c + 0.055f) * (1.f / 1.055f), 2.4f); }

// Bins of the linear indexed encode tables. A bin must be narrower than the narrowest gap
// between two output codes, so that it can straddle at most one code boundary and a single
// +-1 step is guaranteed to reach the exact answer. The narrowest gap is at black, where the
// transfer function is at its steepest: 1/(255*12.92) = 1/3295 for the 8 bit codes and
// 1/(100*12.92) = 1/1292 for the 0..100 ones, both wider than the 1/4095 bins below
constexpr int LUT_SIZE = 4096;

// Clamped, for out of range and NaN inputs (NaN should fail both comparisons -> 0)
inline int LutIndex(const float linear)
{
   const float c = (linear > 0.f) ? ((linear < 1.f) ? linear : 1.f) : 0.f;
   return static_cast<int>(c * static_cast<float>(LUT_SIZE - 1) + 0.5f);
}

struct Tables
{
   float srgbToLinear[256]; // linear value of each 8 bit code

   // Linear values half way between consecutive output codes: code(x) is the number of
   // thresholds x is at or above. Both arrays carry a saturating sentinel at either end, so
   // the correction step can read one entry past each end of the real range - that is what
   // lets it fold the bounds test into the comparison instead of branching on it
   float byteThresholdPad[1 + 255 + 1];
   float percentThresholdPad[1 + 100 + 1];

   uint8_t linearToByte[LUT_SIZE];
   uint8_t linearToPercent[LUT_SIZE];

   // Thresholds indexed by code, valid for -1 .. count
   const float *ByteThreshold() const { return byteThresholdPad + 1; }
   const float *PercentThreshold() const { return percentThresholdPad + 1; }

   Tables()
   {
      for (int i = 0; i < 256; i++)
         srgbToLinear[i] = sRGBToLinearF(static_cast<float>(i) * (float)(1. / 255.));

      float *const __restrict bt = byteThresholdPad + 1;
      float *const __restrict pt = percentThresholdPad + 1;
      for (int i = 0; i < 255; i++)
         bt[i] = sRGBToLinearF((static_cast<float>(i) + 0.5f) * (float)(1. / 255.));
      for (int i = 0; i < 100; i++)
         pt[i] = sRGBToLinearF((static_cast<float>(i) + 0.5f) * 0.01f);
      bt[-1]  = pt[-1]  = -FLT_MAX; // no value is below it, so the step down never fires
      bt[255] = pt[100] =  FLT_MAX; // no value reaches it, so the step up never fires

      // The thresholds are monotonic, so the code for each bin comes from walking them forward once rather than searching per entry
      for (int i = 0, b = 0, p = 0; i < LUT_SIZE; i++)
      {
         const float v = static_cast<float>(i) * (float)(1. / (LUT_SIZE - 1));
         while (v >= bt[b])
            b++;
         while (v >= pt[p])
            p++;
         linearToByte[i]    = static_cast<uint8_t>(b);
         linearToPercent[i] = static_cast<uint8_t>(p);
      }
   }
};

inline const Tables g_tables;

// One step towards the exact code. At most one of the two comparisons can hold, since that
// would need threshold[c] <= linear < threshold[c-1] on a monotonic table, so both are
// evaluated and summed rather than branched on. NaN fails both and stays put
inline int Correct(const int c, const float linear, const float *const threshold)
{
   return c + static_cast<int>(linear >= threshold[c]) - static_cast<int>(linear < threshold[c - 1]);
}

}

// Linear luminance of an 8 bit sRGB component. Exact
inline float SRGBToLinear(const uint8_t component) { return detail::g_tables.srgbToLinear[component]; }

// Linear luminance of an sRGB triplet
inline float SRGBToLuminance(const uint8_t r, const uint8_t g, const uint8_t b)
{
   return 0.2126f * SRGBToLinear(r) + 0.7152f * SRGBToLinear(g) + 0.0722f * SRGBToLinear(b);
}

// Nearest 8 bit sRGB code for a linear value, clamped to 0..1. Exact
inline uint8_t LinearToSRGB(const float linear)
{
   const detail::Tables &t = detail::g_tables;
   return static_cast<uint8_t>(detail::Correct(t.linearToByte[detail::LutIndex(linear)], linear, t.ByteThreshold()));
}

// Nearest 0..100 brightness percentage for a linear value, clamped to 0..1. Exact.
// The percentage is gamma encoded, so this is the same curve as LinearToSRGB, only
// quantized to the 101 codes the controller interface carries
inline uint8_t LinearToPercent(const float linear)
{
   const detail::Tables &t = detail::g_tables;
   return static_cast<uint8_t>(detail::Correct(t.linearToPercent[detail::LutIndex(linear)], linear, t.PercentThreshold()));
}

}
