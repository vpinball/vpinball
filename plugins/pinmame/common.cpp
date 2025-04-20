#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>

static inline char cLower(char c)
{
   if (c >= 'A' && c <= 'Z')
      c ^= 32; //ASCII convention
   return c;
}

static inline bool StrCompareNoCase(const string& strA, const string& strB)
{
   return strA.length() == strB.length()
      && std::equal(strA.begin(), strA.end(), strB.begin(),
         [](char a, char b) { return cLower(a) == cLower(b); });
}

string normalize_path_separators(const string& szPath)
{
   string szResult = szPath;

   if (PATH_SEPARATOR_CHAR == '/')
      std::ranges::replace(szResult.begin(), szResult.end(), '\\', PATH_SEPARATOR_CHAR);
   else
      std::ranges::replace(szResult.begin(), szResult.end(), '/', PATH_SEPARATOR_CHAR);

   auto end = std::unique(szResult.begin(), szResult.end(),
      [](char a, char b) { return a == b && a == PATH_SEPARATOR_CHAR; });
   szResult.erase(end, szResult.end());

   return szResult;
}

string find_case_insensitive_directory_path(const string& szPath)
{
   std::filesystem::path p(normalize_path_separators(szPath));
   if (p.is_relative())
      p = std::filesystem::current_path() / p;
   p = p.lexically_normal();

   if (std::filesystem::exists(p) && std::filesystem::is_directory(p)) {
      auto realPath = std::filesystem::canonical(p);
      string match = realPath.string();
      if (!match.empty() && match.back() != PATH_SEPARATOR_CHAR)
         match.push_back(PATH_SEPARATOR_CHAR);
      return match;
   }

   std::filesystem::path result = p.root_path();
   if (result.empty())
      result = std::filesystem::current_path().root_path();

   for (auto it = std::next(p.begin()); it != p.end(); ++it) {
      const string name = it->string();
      if (!std::filesystem::exists(result) || !std::filesystem::is_directory(result))
         return string();

      bool matched = false;
      for (const auto& entry : std::filesystem::directory_iterator(result)) {
         const string fname = entry.path().filename().string();
         if (StrCompareNoCase(fname, name)) {
            result /= entry.path().filename();
            matched = true;
            break;
         }
      }
      if (!matched)
         return string();
   }

   if (!std::filesystem::exists(result) || !std::filesystem::is_directory(result))
      return string();

   auto realPath = std::filesystem::canonical(result);
   string match = realPath.string();
   if (!match.empty() && match.back() != PATH_SEPARATOR_CHAR)
      match.push_back(PATH_SEPARATOR_CHAR);

   LOGI("exact directory not found, but a case-insensitive directory match was found: szPath=%s, match=%s", szPath.c_str(), match.c_str());
   return match;
}
