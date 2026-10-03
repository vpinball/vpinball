// license:GPLv3+

#include "common.h"

#include <algorithm>
#include <filesystem>
#include <charconv>
#include <cmath>
#include <climits>
#include <bit>

#include <cstddef> // for size_t, ptrdiff_t
// Define ssize_t for Windows
#if defined(_WIN32) && !defined(__SSIZE_T_DEFINED)
#if defined(_WIN64)
typedef __int64 ssize_t;
#else
typedef int ssize_t;
#endif
#define __SSIZE_T_DEFINED
#endif

#include "base64.h"

#include "plugins/PluginStrings.h"

namespace B2S
{

#ifndef __clang__
#define double_as_int64(x) std::bit_cast<int64_t>(x)
#else // for whatever reason apple/clang is special again
#define double_as_int64(x) __builtin_bit_cast(int64_t, x)
#endif
static constexpr bool infNaN(const double a) { return ((double_as_int64(a) & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL); }

string trim_string(const string& str)
{
   size_t start = 0;
   size_t end = str.length();
   while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n'))
      ++start;
   while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n'))
      --end;
   return str.substr(start, end - start);
}

string string_to_lower(string str)
{
   std::ranges::transform(str.begin(), str.end(), str.begin(), cLower);
   return str;
}

std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile)
{
   const std::filesystem::path found = PluginStrings::FindPathNoCase(searchedFile, false);
   if (std::error_code ec; !found.empty() && !std::filesystem::exists(searchedFile, ec))
   {
      LOGI(std::format("Case insensitive file match: requested \"{}\", actual \"{}\"", PluginStrings::PathToUTF8(searchedFile), PluginStrings::PathToUTF8(found)));
   }
   return found;
}

// Wraps up https://github.com/czkz/base64 public domain decoder (plus extensions/optimizations)
vector<uint8_t> base64_decode(const char * const __restrict value, const size_t size_bytes)
{
   vector<uint8_t> ret(size_bytes);

   // First remove any newlines or carriage returns from the input
   uint8_t* __restrict dst = ret.data();
   for (size_t i = 0; i < size_bytes; ++i)
   {
      const char c = value[i];
      if (c != '\r' && c != '\n')
         *dst++ = c;
   }

   const size_t newLen = from_base64_inplace(ret.data(), dst - ret.data());
   ret.resize(newLen);
   return ret; // will be moved
}

// trims leading whitespace or similar, this is needed as e.g. B2S reels feature leading whitespace(s)
int string_to_int(const string& str, int defaultValue)
{
   int result;
   return is_string_numeric(str, &result) ? result : defaultValue;
}

bool is_string_numeric(const string& str, int* const __restrict result)
{
   const string tmp = trim_string(str);
   if (tmp.empty())
      return false;
   char* end = nullptr;
   const double valued = std::nearbyint(std::strtod(tmp.c_str(), &end));
   if (infNaN(valued) || valued < static_cast<double>(INT_MIN) || valued > static_cast<double>(INT_MAX))
      return false;
   const int valuei = static_cast<int>(valued);
   if (result)
      *result = valuei;
   return end == tmp.c_str() + tmp.length();
}

}
