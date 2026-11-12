// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "utils/color.h"

// The sRGB tables (plugins/ColorSpace.h, behind InvsRGB8, sRGB8 and InvsRGBPercent) are precomputed from the exact curve. These checks
// use the curve in double, which is far more precise than a float rounding, so that they hold whatever the float model of the build
// (/fp:fast, -ffast-math), and catch a damaged or outdated table
namespace
{
double InvsRGBDouble(const double c) { return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4); }
double sRGBDouble(const double l) { return l <= 0.0031308 ? 12.92 * l : 1.055 * pow(l, 1.0 / 2.4) - 0.055; }
float NextBelow(const float f) { return nextafterf(f, 0.f); }
float NextAbove(const float f) { return nextafterf(f, 2.f); }

// f is the float nearest to exact
bool IsNearestFloat(const float f, const double exact)
{
   const double d = fabs(f - exact);
   return d <= fabs(NextBelow(f) - exact) && d <= fabs(NextAbove(f) - exact);
}

// Checks that each code of an encode with 'codes' steps starts at the smallest float whose exact encode rounds to it: found by
// bisection on the encode itself, then checked against the curve in double
void CheckEncodeSteps(const std::function<int(float)> &encode, const int codes)
{
   for (int k = 1; k <= codes; k++)
   {
      float lo = 0.f, hi = 1.f; // encode(lo) < k <= encode(hi)
      while (NextBelow(hi) > lo)
      {
         const float mid = 0.5f * (lo + hi);
         if (mid <= lo || mid >= hi)
            break;
         (encode(mid) >= k ? hi : lo) = mid;
      }
      CHECK_MESSAGE(encode(hi) == k, "step ", k);
      CHECK_MESSAGE(encode(NextBelow(hi)) == k - 1, "step ", k);
      CHECK_MESSAGE(sRGBDouble(hi) * codes >= k - 0.5, "step ", k);
      CHECK_MESSAGE(sRGBDouble(NextBelow(hi)) * codes < k - 0.5, "step ", k);
   }
}
}

TEST_CASE("InvsRGB8 table")
{
   CHECK(InvsRGB8(0) == 0.f);
   CHECK(InvsRGB8(255) == 1.f);
   for (int i = 0; i < 256; i++)
   {
      CHECK_MESSAGE(IsNearestFloat(InvsRGB8(static_cast<uint8_t>(i)), InvsRGBDouble(i / 255.)), "entry ", i);
      if (i > 0)
         CHECK(InvsRGB8(static_cast<uint8_t>(i)) > InvsRGB8(static_cast<uint8_t>(i - 1)));
   }
}

TEST_CASE("sRGB8 table")
{
   CHECK(sRGB8(0.f) == 0);
   CHECK(sRGB8(1.f) == 255);

   // Each 8 bit value round trips
   for (int i = 0; i < 256; i++)
      CHECK_MESSAGE(sRGB8(InvsRGB8(static_cast<uint8_t>(i))) == i, "value ", i);

   CheckEncodeSteps([](const float c) { return static_cast<int>(sRGB8(c)); }, 255);

   // Monotonic over a dense sample
   int decreasing = 0;
   uint8_t prev = 0;
   for (int i = 0; i <= 1 << 20; i++)
   {
      const uint8_t v = sRGB8(static_cast<float>(i) / static_cast<float>(1 << 20));
      decreasing += v < prev;
      prev = v;
   }
   CHECK(decreasing == 0);
}

TEST_CASE("Brightness percentage tables")
{
   CHECK(InvsRGBPercent(0) == 0.f);
   CHECK(InvsRGBPercent(100) == 1.f);
   for (int i = 0; i < 256; i++) // Above 100, the curve is extrapolated
   {
      CHECK_MESSAGE(IsNearestFloat(InvsRGBPercent(static_cast<uint8_t>(i)), InvsRGBDouble(i / 100.)), "entry ", i);
      if (i > 0)
         CHECK(InvsRGBPercent(static_cast<uint8_t>(i)) > InvsRGBPercent(static_cast<uint8_t>(i - 1)));
   }

   // Each percentage round trips
   for (int i = 0; i <= 100; i++)
      CHECK_MESSAGE(VPXColorSpace::LinearToPercent(InvsRGBPercent(static_cast<uint8_t>(i))) == i, "value ", i);

   CheckEncodeSteps([](const float c) { return static_cast<int>(VPXColorSpace::LinearToPercent(c)); }, 100);
}

TEST_CASE("Linear to sRGB clamps")
{
   CHECK(VPXColorSpace::LinearToSRGB(-1.f) == 0);
   CHECK(VPXColorSpace::LinearToSRGB(2.f) == 255);
   CHECK(VPXColorSpace::LinearToSRGB(std::numeric_limits<float>::infinity()) == 255);
   CHECK(VPXColorSpace::LinearToSRGB(std::numeric_limits<float>::quiet_NaN()) == 0);
   CHECK(VPXColorSpace::LinearToPercent(2.f) == 100);
   CHECK(VPXColorSpace::LinearToPercent(std::numeric_limits<float>::quiet_NaN()) == 0);
}
