// license:GPLv3+

#include "common.h"

#include <algorithm>
#include <filesystem>
#include <charconv>

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

namespace B2S
{

constexpr inline char cLower(char c)
{
   if (c >= 'A' && c <= 'Z')
      c ^= 32; //ASCII convention
   return c;
}

static inline bool StrCompareNoCase(const string& strA, const string& strB)
{
   return strA.length() == strB.length() && std::equal(strA.begin(), strA.end(), strB.begin(), [](char a, char b) { return cLower(a) == cLower(b); });
}

string string_to_lower(string str)
{
   std::ranges::transform(str.begin(), str.end(), str.begin(), cLower);
   return str;
}

std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile)
{
   auto fn = [](const auto& self, std::filesystem::path path)
   {
      std::error_code ec;
      path = path.lexically_normal();
      if (std::filesystem::exists(path, ec))
         return path;

      const auto& parent = path.parent_path();
      std::filesystem::path base = (parent.empty() || parent == path) ? std::filesystem::path("."s) : self(self, parent);
      if (base.empty())
         return base;

      for (const auto& ent : std::filesystem::directory_iterator(base, ec))
      {
         if (!ec && StrCompareNoCase(ent.path().filename().string(), path.filename().string()))
         {
            const auto& found = ent.path();
            if (found != path)
            {
               LOGI("case insensitive file match: requested \"%s\", actual \"%s\"", path.c_str(), found.c_str());
            }
            return found;
         }
      }

      return std::filesystem::path();
   };

   const std::filesystem::path result = fn(fn, searchedFile);
   return result.empty() ? result : std::filesystem::absolute(result);
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

}
