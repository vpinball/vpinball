// license:GPLv3+

#include "common.h"

#include <algorithm>
#include <filesystem>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif

namespace Serum {
   
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <locale>
void SetThreadName(const std::string& name)
{
   const int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
   if (size_needed <= 1)
      return;
   std::wstring wstr(size_needed - 1, L'\0');
   if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
      return;
   HRESULT hr = SetThreadDescription(GetCurrentThread(), wstr.c_str());
}
#else
void SetThreadName(const std::string& name)
{
#ifdef __APPLE__
   pthread_setname_np(name.c_str());
#elif defined(__linux__) || defined(__ANDROID__)
   pthread_setname_np(pthread_self(), name.c_str());
#endif
}
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

}
