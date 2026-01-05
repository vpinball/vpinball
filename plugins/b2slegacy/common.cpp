// license:GPLv3+

#include "common.h"
#include <filesystem>
#include <algorithm>
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

namespace B2SLegacy
{

static string trim_string(const string& str)
{
   size_t start = 0;
   size_t end = str.length();
   while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n'))
      ++start;
   while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n'))
      --end;
   return str.substr(start, end - start);
}

// trims leading whitespace or similar
static bool try_parse_int(const string& str, int& value)
{
   const string tmp = trim_string(str);
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc{});
}

bool StrCompareNoCase(const string& strA, const string& strB)
{
   return strA.length() == strB.length()
      && std::equal(strA.begin(), strA.end(), strB.begin(),
         [](char a, char b) { return cLower(a) == cLower(b); });
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
         base = ".";
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

// Wraps up https://github.com/czkz/base64 public domain decoder
string base64_decode(char* value)
{
   // Modify string in place to remove CR/LF characters
   // This is not correct and breaks the original file but speeds up the process and saves memory
   const char* valueSrc = value;
   char* valueDst = value;
   for (;; valueSrc++)
   {
      char c = *valueSrc;
      if (c == '\0')
      {
         *valueDst = '\0';
         break;
      }
      if (c != '\r' && c != '\n')
      {
         *valueDst = c;
         valueDst++;
      }
   }
   string encoded_string = value;
   //std::erase(encoded_string, '\r');
   //std::erase(encoded_string, '\n');
   return from_base64(encoded_string);
}

bool string_starts_with_case_insensitive(const string& str, const string& prefix)
{
   if (prefix.length() > str.length())
      return false;
   return std::equal(prefix.begin(), prefix.end(), str.begin(),
      [](char a, char b) { return cLower(a) == cLower(b); });
}

// trims leading whitespace or similar, this is needed as e.g. B2S reels feature leading whitespace(s)
int string_to_int(const string& str, int defaultValue)
{
   int value;
   return try_parse_int(str, value) ? value : defaultValue;
}

string title_and_path_from_filename(const string& filename)
{
   return filename.substr(0, filename.find_last_of('.'));
}

bool is_string_numeric(const string& str)
{
   if (str.empty()) return false;
   for (char c : str) {
      if (!std::isdigit(c)) return false;
   }
   return true;
}

}
