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

string normalize_path_separators(const string& szPath)
{
   string szResult = szPath;

   #if '/' == PATH_SEPARATOR_CHAR
      std::ranges::replace(szResult.begin(), szResult.end(), '\\', PATH_SEPARATOR_CHAR);
   #else
      std::ranges::replace(szResult.begin(), szResult.end(), '/', PATH_SEPARATOR_CHAR);
   #endif

   auto end = std::unique(szResult.begin(), szResult.end(),
      [](char a, char b) { return a == b && a == PATH_SEPARATOR_CHAR; });
   szResult.erase(end, szResult.end());

   return szResult;
}

string extension_from_path(const string& path)
{
   const size_t pos = path.find_last_of('.');
   return pos != string::npos ? string_to_lower(path.substr(pos + 1)) : string();
}

// same as removing the file extension
string TitleAndPathFromFilename(const string& filename)
{
   return filename.substr(0, filename.find_last_of('.')); // in case no '.' is found, will then copy full filename
}

string find_case_insensitive_file_path(const string& szPath)
{
   auto fn = [&](auto& self, const string& s) -> string {
      string path = normalize_path_separators(s);
      std::filesystem::path p = std::filesystem::path(path).lexically_normal();
      std::error_code ec;

      if (std::filesystem::exists(p, ec))
         return p.string();

      auto parent = p.parent_path();
      string base;
      if (parent.empty() || parent == p) {
         base = "."s;
      } else {
         base = self(self, parent.string());
         if (base.empty())
            return string();
      }

      for (auto& ent : std::filesystem::directory_iterator(base, ec)) {
         if (!ec && StrCompareNoCase(ent.path().filename().string(), p.filename().string())) {
            auto found = ent.path().string();
            if (found != path) {
               LOGI("case insensitive file match: requested \"%s\", actual \"%s\"", path.c_str(), found.c_str());
            }
            return found;
         }
      }

      return string();
   };

   string result = fn(fn, szPath);
   if (!result.empty()) {
      std::filesystem::path p = std::filesystem::absolute(result);
      return p.string();
   }
   return string();
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
