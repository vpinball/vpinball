#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>

inline char cLower(char c)
{
   if (c >= 'A' && c <= 'Z')
      c ^= 32; //ASCII convention
   return c;
}

string string_to_lower(string str)
{
   std::transform(str.begin(), str.end(), str.begin(), cLower);
   return str;
}

string find_directory_case_insensitive(const string& szParentPath, const string& szDirName)
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
         return szMatch;
      }
   }

   return string();
}

