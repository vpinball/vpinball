// license:GPLv3+

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

#include "plugins/ColorSpace.h"

#include <cmath>

// Reference implementations, independent from the lookup tables under test
namespace
{

float RefSRGBToLinear(const float c) { return (c <= 0.04045f) ? (c * (1.f / 12.92f)) : static_cast<float>(pow((c + 0.055) * (1. / 1.055), 2.4)); }

int RefLinearToSRGB(const float linear)
{
   const double l = linear;
   const double e = (l <= 0.0031308) ? (l * 12.92) : (1.055 * pow(l, 1. / 2.4) - 0.055);
   return static_cast<int>(floor(e * 255. + 0.5));
}

} // namespace

TEST_CASE("ColorSpace sRGB transfer helpers")
{
   using namespace VPXColorSpace;

   SUBCASE("decode table is exact at the ends and monotonic")
   {
      CHECK(SRGBToLinear(0) == 0.f);
      CHECK(SRGBToLinear(255) == 1.f);
      float prev = -1.f;
      for (int i = 0; i < 256; i++)
      {
         const float v = SRGBToLinear(static_cast<uint8_t>(i));
         CHECK(v == doctest::Approx(RefSRGBToLinear(i * (1.f / 255.f))));
         CHECK(v > prev);
         prev = v;
      }
   }

   SUBCASE("luminance applies Rec.709 weights on linear components")
   {
      CHECK(SRGBToLuminance(0, 0, 0) == 0.f);
      CHECK(SRGBToLuminance(255, 255, 255) == doctest::Approx(1.0));
      CHECK(SRGBToLuminance(255, 0, 0) == doctest::Approx(0.2126));
      CHECK(SRGBToLuminance(0, 255, 0) == doctest::Approx(0.7152));
      CHECK(SRGBToLuminance(0, 0, 255) == doctest::Approx(0.0722));
   }

   SUBCASE("encode round trips every sRGB code exactly")
   {
      for (int i = 0; i < 256; i++)
         CHECK(LinearToSRGB(SRGBToLinear(static_cast<uint8_t>(i))) == i);
   }

   SUBCASE("encode round trips every brightness percentage exactly")
   {
      for (int i = 0; i <= 100; i++)
         CHECK(LinearToPercent(RefSRGBToLinear(i * 0.01f)) == i);
   }

   SUBCASE("encode lands on the nearest code")
   {
      // Sweep the whole 0..1 range, including the table bins and their midpoints
      for (int i = 0; i <= 8192; i++)
      {
         const float l = i * (1.f / 8192.f);
         CHECK(LinearToSRGB(l) == RefLinearToSRGB(l));
      }
      CHECK(LinearToSRGB(0.f) == 0);
      CHECK(LinearToSRGB(1.f) == 255);
   }

   SUBCASE("encode is monotonic")
   {
      int prev = -1;
      for (int i = 0; i <= 4096; i++)
      {
         const int code = LinearToSRGB(i * (1.f / 4096.f));
         CHECK(code >= prev);
         prev = code;
      }
   }

   SUBCASE("encode clamps out of range and NaN inputs")
   {
      CHECK(LinearToSRGB(-0.5f) == 0);
      CHECK(LinearToSRGB(-FLT_MAX) == 0);
      CHECK(LinearToSRGB(2.f) == 255);
      CHECK(LinearToSRGB(FLT_MAX) == 255);
      CHECK(LinearToSRGB(std::nanf("")) == 0);
      CHECK(LinearToPercent(-0.5f) == 0);
      CHECK(LinearToPercent(2.f) == 100);
      CHECK(LinearToPercent(std::nanf("")) == 0);
   }
}
