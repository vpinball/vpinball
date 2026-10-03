// license:GPLv3+

#include "common.h"
#include <filesystem>
#include <algorithm>
#include <ranges>

namespace WMP {

static constexpr inline char cLower(char c)
{
   if (c >= 'A' && c <= 'Z')
      c ^= 32;
   return c;
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
   const std::filesystem::path path = PluginStrings::PathFromUTF8(normalize_path_separators(szPath));
   const std::filesystem::path result = PluginStrings::FindPathNoCase(path);
   if (result.empty())
      return string();
   std::error_code ec;
   if (!std::filesystem::exists(path, ec))
      LOGI(std::format("Case insensitive file match: requested \"{}\", actual \"{}\"", PluginStrings::PathToUTF8(path), PluginStrings::PathToUTF8(result)));
   return PluginStrings::PathToUTF8(result);
}

}
