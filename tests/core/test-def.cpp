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
      CHECK(StrLessNoCase("apple"s, "Banana"s));
      CHECK_FALSE(StrLessNoCase("Banana"s, "apple"s));
      CHECK(StrLessNoCase("Lights"s, "lights GI"s)); // a prefix comes first
      CHECK_FALSE(StrLessNoCase("ABC"s, "abc"s)); // equal ignoring case
      CHECK(StrLessNoCase("Z"s, "\xC3\x89" "cran"s)); // non-ASCII bytes order after ASCII (no sign extension)
      CHECK(IsASCIIAlnum('a'));
      CHECK(IsASCIIAlnum('Z'));
      CHECK(IsASCIIAlnum('5'));
      CHECK_FALSE(IsASCIIAlnum('.'));
      CHECK_FALSE(IsASCIIAlnum('\xC3'));
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

   SUBCASE("UTF-8 or legacy Windows-1252 text")
   {
      const auto convert = [](const string& s) { return string_from_utf8_or_cp1252(s.data(), s.size()); };
      CHECK(convert("plain ASCII"s) == "plain ASCII");
      CHECK(convert("Caf\xC3\xA9"s) == "Caf\xC3\xA9"); // valid UTF-8 is kept
      CHECK(convert("Caf\xE9"s) == "Caf\xC3\xA9"); // Latin-1 range
      CHECK(convert("\x80 \x93quote\x94 \x96"s) == "\xE2\x82\xAC \xE2\x80\x9Cquote\xE2\x80\x9D \xE2\x80\x93"); // Windows-1252 specific range
      CHECK(convert("\x81"s) == "\xC2\x81"); // unassigned, mapped like Windows does
      CHECK(string_from_utf8_or_cp1252("Caf\xC3\xA9"s) == "Caf\xC3\xA9"); // in place variant
      CHECK(string_from_utf8_or_cp1252("Caf\xE9"s) == "Caf\xC3\xA9");
      CHECK(convert("Caf\xC3"s) == "Caf\xC3\x83"); // a UTF-8 sequence cut at the end is legacy text too
      string cp1252;
      CHECK(utf8_to_cp1252("Caf\xC3\xA9 \xE2\x82\xAC \xC2\x81"s, cp1252)); // back to Windows-1252, incl. the 0x80..0x9F range
      CHECK(cp1252 == "Caf\xE9 \x80 \x81");
      CHECK(convert(cp1252) == "Caf\xC3\xA9 \xE2\x82\xAC \xC2\x81"); // round trip
      CHECK_FALSE(utf8_to_cp1252("\xD0\x96"s, cp1252)); // Cyrillic is not in Windows-1252
      CHECK_FALSE(utf8_to_cp1252("Caf\xC3"s, cp1252)); // invalid UTF-8
      CHECK(string_from_utf8_or_cp1252("Caf\xC3"s) == "Caf\xC3\x83");
   }

   SUBCASE("UTF-8 and wide text conversions")
   {
      CHECK(MakeString(L"Caf\u00E9 \u2605"s) == "Caf\xC3\xA9 \xE2\x98\x85");
      CHECK(MakeWString("Caf\xC3\xA9 \xE2\x98\x85"s) == L"Caf\u00E9 \u2605");
      CHECK(MakeString(static_cast<const WCHAR*>(nullptr)).empty());
      CHECK(MakeWString(static_cast<const char*>(nullptr)).empty());
      CHECK(MakeString(wstring()).empty());
      CHECK(MakeWString(string()).empty());
      WCHAR buffer[] = L"Plain \u00E9"; // Not a BSTR: must not be read with a BSTR length prefix
      CHECK(MakeString(buffer) == "Plain \xC3\xA9");
      const BSTR bstr = MakeWideBSTR("B\xC3\xA9"s);
      CHECK(SysStringLen(bstr) == 2);
      CHECK(MakeString(bstr) == "B\xC3\xA9");
      SysFreeString(bstr);
   }

   SUBCASE("UTF-8 truncation to a UTF-16 length")
   {
      CHECK(TruncateToUTF16Length("abc"s, 2) == "ab");
      CHECK(TruncateToUTF16Length("abc"s, 5) == "abc");
      CHECK(TruncateToUTF16Length("Caf\xC3\xA9"s, 4) == "Caf\xC3\xA9"); // 2 bytes, 1 unit
      CHECK(TruncateToUTF16Length("Caf\xC3\xA9"s, 3) == "Caf");
      CHECK(TruncateToUTF16Length("a\xE2\x82\xAC"s, 2) == "a\xE2\x82\xAC"); // 3 bytes, 1 unit
      CHECK(TruncateToUTF16Length("a\xF0\x9F\x8E\xB1"s, 3) == "a\xF0\x9F\x8E\xB1"); // 4 bytes, surrogate pair
      CHECK(TruncateToUTF16Length("a\xF0\x9F\x8E\xB1"s, 2) == "a"); // a surrogate pair is not split
      CHECK(TruncateToUTF16Length(""s, 3).empty());
   }

   SUBCASE("UTF-8 paths")
   {
      const std::filesystem::path path = PathFromUTF8("Caf\xC3\xA9/T\xC3\xA9st.vpx"s);
      CHECK(PathToUTF8(path) == "Caf\xC3\xA9/T\xC3\xA9st.vpx");
      CHECK(PathToUTF8(path.filename()) == "T\xC3\xA9st.vpx");
      CHECK(PathToUTF8(path.parent_path()) == "Caf\xC3\xA9");
      CHECK(PathToUTF8(std::filesystem::path()).empty());
      CHECK_NOTHROW(PathFromUTF8("invalid \xFF UTF-8"s)); // replaced, not thrown
      CHECK(PathFromUTF8OrString("Caf\xC3\xA9/T.vpx"s) == path.parent_path() / "T.vpx");

      // Files open with any name (characters outside Windows-1252 check that no legacy ANSI API is used)
      const std::filesystem::path file = GetTestTmpDir() / PathFromUTF8("open_file \xD0\x96\xE2\x98\x85.txt"s);
      FILE* f = open_file(file, "wb");
      REQUIRE(f != nullptr);
      fputs("data", f);
      fclose(f);
      f = open_file(file, "rb");
      REQUIRE(f != nullptr);
      char data[8] = {};
      CHECK(fread(data, 1, sizeof(data) - 1, f) == 4);
      CHECK(string(data) == "data");
      fclose(f);
      std::error_code ec;
      std::filesystem::remove(file, ec);
      CHECK(open_file(file, "rb") == nullptr);
   }
}
