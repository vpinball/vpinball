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

static inline string string_to_lower(string str)
{
   std::transform(str.begin(), str.end(), str.begin(), cLower);
   return str;
}

string find_directory_case_insensitive(const std::string& szParentPath, const std::string& szDirName)
{
   std::filesystem::path parentPath(szParentPath);
   if (!std::filesystem::exists(parentPath) || !std::filesystem::is_directory(parentPath))
      return string();

   std::filesystem::path fullPath = parentPath / szDirName;
   if (std::filesystem::exists(fullPath) && std::filesystem::is_directory(fullPath))
      return fullPath.string() + PATH_SEPARATOR_CHAR;

   string szDirLower = string_to_lower(szDirName);
   for (const auto& entry : std::filesystem::directory_iterator(parentPath)) {
      if (!std::filesystem::is_directory(entry.status()))
         continue;

      if (string_to_lower(entry.path().filename().string()) == szDirLower) {
         string szMatch = entry.path().string() + PATH_SEPARATOR_CHAR;
         LOGI("case-insensitive match was found: szParentPath=%s, szDirName=%s, match=%s",
            szParentPath.c_str(), szDirName.c_str(), szMatch.c_str());
         return szMatch;
      }
   }

   return string();
}
