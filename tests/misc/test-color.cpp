// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

#include "utils/color.h"

// InvsRGB8() and sRGB8() use tables precomputed with /fp:precise. These checks use the sRGB curve in double, so that they hold
// whatever the float model of the build (/fp:fast, -ffast-math), and catch a damaged or outdated table
namespace
{
double InvsRGBDouble(const double c) { return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4); }
double sRGBDouble(const double l) { return l <= 0.0031308 ? 12.92 * l : 1.055 * pow(l, 1.0 / 2.4) - 0.055; }
float NextBelow(const float f) { return nextafterf(f, 0.f); }
}

TEST_CASE("InvsRGB8 table")
{
   CHECK(InvsRGB8(0) == 0.f);
   CHECK(InvsRGB8(255) == 1.f);
   for (int i = 0; i < 256; i++)
   {
      // Computed in float from a rounded c, so within a few float roundings of the exact value
      const double exact = InvsRGBDouble(static_cast<double>(static_cast<float>(i) / 255.f));
      CHECK_MESSAGE(fabs(InvsRGB8(static_cast<uint8_t>(i)) - exact) <= exact * 4e-7 + 1e-12, "entry ", i);
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

   // Each output step sits where the curve crosses k - 0.5: find it by bisection on sRGB8 itself, then check it against the curve
   for (int k = 1; k < 256; k++)
   {
      float lo = 0.f, hi = 1.f; // sRGB8(lo) < k <= sRGB8(hi)
      while (NextBelow(hi) > lo)
      {
         const float mid = 0.5f * (lo + hi);
         if (mid <= lo || mid >= hi)
            break;
         (sRGB8(mid) >= k ? hi : lo) = mid;
      }
      CHECK_MESSAGE(sRGB8(hi) == k, "step ", k);
      CHECK_MESSAGE(sRGB8(NextBelow(hi)) == k - 1, "step ", k);
      CHECK_MESSAGE(fabs(sRGBDouble(hi) * 255.0 - (k - 0.5)) < 1e-4, "step ", k);
   }

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
