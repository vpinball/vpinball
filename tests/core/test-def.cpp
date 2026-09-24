// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "doctest.h"

TEST_CASE("def.h helpers")
{
   SUBCASE("min, max, clamp, lerp, saturate")
   {
      CHECK(min(3, 7) == 3);
      CHECK(max(3, 7) == 7);
      CHECK(min(2.5f, 1.5f) == 1.5f);
      CHECK(clamp(5, 0, 10) == 5);
      CHECK(clamp(-3, 0, 10) == 0);
      CHECK(clamp(13, 0, 10) == 10);
      CHECK(clamp(2.5f, 0.f, 1.f) == 1.f);
      CHECK(lerp(0.f, 10.f, 0.5f) == 5.f);
      CHECK(lerp(10.f, 20.f, 0.f) == 10.f);
      CHECK(saturate(0.5f) == 0.5f);
      CHECK(saturate(-1.f) == 0.f);
      CHECK(saturate(2.f) == 1.f);
   }

   SUBCASE("smoothstep")
   {
      CHECK(smoothstep(0.f, 1.f, 0.f) == 0.f);
      CHECK(smoothstep(0.f, 1.f, 1.f) == 1.f);
      CHECK(smoothstep(0.f, 1.f, 0.5f) == doctest::Approx(0.5f));
      CHECK(smoothstep(0.f, 1.f, -1.f) == 0.f); // clamped below the range
      CHECK(smoothstep(0.f, 1.f, 2.f) == 1.f); // clamped above the range
      CHECK(smoothstep(1.f, 1.f, 0.5f) == 0.f); // degenerate range acts as a step
      CHECK(smoothstep(1.f, 1.f, 1.f) == 1.f);
   }

   SUBCASE("float classification")
   {
      CHECK(sgn(4.f) == 1.f);
      CHECK(sgn(-4.f) == -1.f);
      CHECK(sgn(0.f) == 0.f);
      CHECK(sign(-0.25f));
      CHECK_FALSE(sign(0.25f));
      CHECK(inf(std::numeric_limits<float>::infinity()));
      CHECK_FALSE(inf(FLT_MAX));
      CHECK(infNaN(std::numeric_limits<float>::infinity()));
      CHECK(infNaN(std::numeric_limits<float>::quiet_NaN()));
      CHECK_FALSE(infNaN(1.f));
      CHECK(NaN(std::numeric_limits<float>::quiet_NaN()));
      CHECK_FALSE(NaN(std::numeric_limits<float>::infinity()));
      CHECK(deNorm(std::numeric_limits<float>::denorm_min()));
      CHECK_FALSE(deNorm(0.f));
      CHECK_FALSE(deNorm(1.f));
   }

   SUBCASE("byte order and low discrepancy sequences")
   {
      CHECK(swap_byteorder(0x12345678u) == 0x78563412u);
      CHECK(map_u32_to_unifloat(0u) == 0.f);
      CHECK(map_u32_to_unifloat(0xFFFFFFFFu) < 1.f);
      CHECK(map_u32_to_unifloat(0xFFFFFFFFu) > 0.9f);
      CHECK(radical_inverse(1u) == doctest::Approx(0.5f));
      CHECK(radical_inverse(2u) == doctest::Approx(0.25f));
      CHECK(sobol(1u) == doctest::Approx(0.5f));
      CHECK(sobol(2u) == doctest::Approx(0.75f));
   }

   SUBCASE("vector helpers")
   {
      vector<int> v { 1, 2, 3, 2 };
      CHECK(FindIndexOf(v, 2) == 1);
      CHECK(FindIndexOf(v, 9) == -1);
      RemoveFromVectorSingle(v, 2);
      CHECK(v == vector<int> { 1, 3, 2 });
      RemoveFromVectorSingle(v, 9);
      CHECK(v.size() == 3);
   }

   SUBCASE("VP unit conversions")
   {
      CHECK(VPUTOINCHES(INCHESTOVPU(20.25f)) == doctest::Approx(20.25f));
      CHECK(VPUTOMM(MMTOVPU(540.f)) == doctest::Approx(540.f));
      CHECK(VPUTOCM(CMTOVPU(117.f)) == doctest::Approx(117.f));
      CHECK(inchesToVPUnits(1.0625f) == doctest::Approx(50.f));
      CHECK(vpUnitsToInches(50.f) == doctest::Approx(1.0625f));
      CHECK(millimetersToVPUnits(vpUnitsToMillimeters(123.45f)) == doctest::Approx(123.45f));
   }

   SUBCASE("string case helpers")
   {
      CHECK(cLower('A') == 'a');
      CHECK(cLower('a') == 'a');
      CHECK(cUpper('a') == 'A');
      CHECK(lowerCase("AbC dE"s) == "abc de");
      CHECK(upperCase("AbC dE"s) == "ABC DE");
      CHECK(lowerCase(L"AbC"s) == L"abc");
      CHECK(StrCompareNoCase("Ground"s, "ground"s));
      CHECK(StrCompareNoCase("Ground"s, "GROUND"));
      CHECK_FALSE(StrCompareNoCase("Ground"s, "grounder"s));
      CHECK(StrFindNoCase("the Quick brown fox"s, "BROWN"s) == 10);
      CHECK(StrFindNoCase("abc"s, "z"s) == string::npos);
      string s = "MiXeD"s;
      StrToLower(s);
      CHECK(s == "mixed");
      StrToUpper(s);
      CHECK(s == "MIXED");
   }

   SUBCASE("trim and parse helpers")
   {
      CHECK(trim_string("  42 \t\n"s) == "42");
      int i = 0;
      CHECK(try_parse_int("42"s, i));
      CHECK(i == 42);
      CHECK(try_parse_int(" -7 "s, i));
      CHECK(i == -7);
      CHECK_FALSE(try_parse_int("x4"s, i));
      CHECK_FALSE(try_parse_int(""s, i));
      float f = 0.f;
      CHECK(try_parse_float("1.5"s, f));
      CHECK(f == 1.5f);
      CHECK(try_parse_float(" -2.25"s, f));
      CHECK(f == -2.25f);
      CHECK_FALSE(try_parse_float("abc"s, f));
   }

   SUBCASE("path helpers")
   {
      CHECK(TitleFromFilename("folder/MyTable.vpx"s) == "MyTable");
      CHECK(extension_from_path("folder/File.PNG"s) == "png");
      CHECK(path_has_extension("file.webp"s, "WEBP"s));
      CHECK_FALSE(path_has_extension("file.webp"s, "png"s));
   }

   SUBCASE("safe string copy")
   {
      char buf[8];
      strncpy_s(buf, sizeof(buf), "hello");
      CHECK(string(buf) == "hello");
      strncpy_s(buf, sizeof(buf), "1234567"); // exact fit (dest_size - 1 chars)
      CHECK(string(buf) == "1234567");
      strncpy_s(buf, sizeof(buf), nullptr);
      CHECK(buf[0] == '\0');
   }
}
