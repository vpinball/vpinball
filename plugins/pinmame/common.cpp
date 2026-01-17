// license:GPLv3+

#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>

namespace PinMAME {

constexpr inline char cLower(char c)
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

std::filesystem::path find_case_insensitive_directory_path(const std::filesystem::path& searchedFile)
{
   auto fn = [](const auto& self, std::filesystem::path path)
   {
      std::error_code ec;
      path = path.lexically_normal();
      if (std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec))
         return path;

      const auto& parent = path.parent_path();
      std::filesystem::path base = (parent.empty() || parent == path) ? std::filesystem::path("."s) : self(self, parent);
      if (base.empty())
         return base;

      for (const auto& ent : std::filesystem::directory_iterator(base, ec))
      {
         if (ec || !ent.is_directory(ec))
            continue;
         if (ec || !StrCompareNoCase(ent.path().filename().string(), path.filename().string()))
            continue;
         const auto& found = ent.path();
         if (found != path)
         {
            LOGI("case insensitive directory match: requested \"%s\", actual \"%s\"", path.string().c_str(), found.string().c_str());
         }
         return found;
      }

      return std::filesystem::path();
   };

   const std::filesystem::path result = fn(fn, searchedFile);
   return result.empty() ? result : std::filesystem::absolute(result);
}

}
