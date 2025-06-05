#include "common.h"

#include <algorithm>
#include <filesystem>

namespace Serum {
   
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <locale>
void SetThreadName(const std::string& name)
{
   int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, NULL, 0);
   if (size_needed == 0)
      return;
   std::wstring wstr(size_needed, 0);
   if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
      return;
   HRESULT hr = SetThreadDescription(GetCurrentThread(), wstr.c_str());
}
#else
void SetThreadName(const std::string& name) { }
#endif

inline char cLower(char c)
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

string find_case_insensitive_directory_path_internal(const string& szPath)
{
   string path = normalize_path_separators(szPath);
   std::filesystem::path p = std::filesystem::path(path).lexically_normal();
   std::error_code ec;

   if (std::filesystem::exists(p, ec) && std::filesystem::is_directory(p, ec)) {
      string exact = p.string();
      if (!exact.empty() && exact.back() != PATH_SEPARATOR_CHAR)
         exact.push_back(PATH_SEPARATOR_CHAR);
      return exact;
   }

   auto parent = p.parent_path();
   string base;
   if (parent.empty() || parent == p)
      base = '.';
   else {
      base = find_case_insensitive_directory_path_internal(parent.string());
      if (base.empty())
         return string();
   }

   for (auto& ent : std::filesystem::directory_iterator(base, ec)) {
      if (ec || !ent.is_directory(ec))
         continue;
      if (StrCompareNoCase(ent.path().filename().string(), p.filename().string())) {
         string found = ent.path().string();
         if (!found.empty() && found.back() != PATH_SEPARATOR_CHAR)
            found.push_back(PATH_SEPARATOR_CHAR);
         if (found != path) {
            LOGI("case insensitive directory match: requested \"%s\", actual \"%s\"", path.c_str(), found.c_str());
         }
         return found;
      }
   }

   return string();
}

string find_case_insensitive_directory_path(const string& szPath)
{
   string path = find_case_insensitive_directory_path_internal(szPath);
   if (!path.empty()) {
      std::filesystem::path p = std::filesystem::absolute(path);
      string exact = p.string();
      if (!exact.empty() && exact.back() != PATH_SEPARATOR_CHAR)
         exact.push_back(PATH_SEPARATOR_CHAR);
      return exact;
   }
   return string();
}

}