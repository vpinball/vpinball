// license:GPLv3+

#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>
#include <charconv>

namespace Flex {

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

string string_to_lower(string str)
{
   std::ranges::transform(str.begin(), str.end(), str.begin(), cLower);
   return str;
}

string trim_string(const string& str)
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
bool try_parse_int(const string& str, int& value)
{
   const string tmp = trim_string(str);
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc{});
}

bool try_parse_color(const string& str, ColorRGBA32& value)
{
   const size_t start = (!str.empty() && str[0] == '#') ? 1 : 0;
   string hexStr(str, start);

   if (hexStr.size() == 6)
      hexStr += "FF"sv;
   else
      if (hexStr.size() != 8)
         return false;

   uint32_t rgba;
   std::stringstream ss;
   ss << std::hex << hexStr;
   if (!(ss >> rgba))
      return false;

   const uint8_t r = (rgba >> 24) & 0xFF;
   const uint8_t g = (rgba >> 16) & 0xFF;
   const uint8_t b = (rgba >> 8)  & 0xFF;

   value = r | (g << 8) | (b << 16);

   return true;
}

// trims leading whitespace or similar
int string_to_int(const string& str, int defaultValue)
{
   int value;
   return try_parse_int(str, value) ? value : defaultValue;
}

string normalize_path_separators(const string& szPath)
{
   string szResult = szPath;

   #if '/' == PATH_SEPARATOR_CHAR
      std::ranges::replace(szResult.begin(), szResult.end(), '\\', PATH_SEPARATOR_CHAR);
   #else
      std::ranges::replace(szResult.begin(), szResult.end(), '/', PATH_SEPARATOR_CHAR);
   #endif

   const bool isUNC = szResult.size() >= 2 && szResult[0] == PATH_SEPARATOR_CHAR && szResult[1] == PATH_SEPARATOR_CHAR;
   auto end = std::unique(isUNC ? szResult.begin() + 2 : szResult.begin(), szResult.end(),
      [](char a, char b) { return a == b && a == PATH_SEPARATOR_CHAR; });
   szResult.erase(end, szResult.end());

   return szResult;
}

string extension_from_path(const string& path)
{
   const size_t pos = path.find_last_of('.');
   return pos != string::npos ? string_to_lower(path.substr(pos + 1)) : string();
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
      std::filesystem::path base = (parent.empty() || parent == path) ? std::filesystem::path(".") : self(self, parent);
      if (base.empty())
         return base;

      for (const auto& ent : std::filesystem::directory_iterator(base, ec))
      {
         if (!ec && StrCompareNoCase(ent.path().filename().string(), path.filename().string()))
            return ent.path();
      }

      return std::filesystem::path();
   };

   const std::filesystem::path result = fn(fn, searchedFile);
   return result.empty() ? result : std::filesystem::absolute(result);
}

#ifdef _WIN32
template <class T>
static T GetModulePath(HMODULE hModule)
{
   T path;
   DWORD size = MAX_PATH;
   while (true)
   {
      path.resize(size);
      DWORD length;
      if constexpr (std::is_same_v<T, std::string>)
         length = ::GetModuleFileNameA(hModule, path.data(), size);
      else
         length = ::GetModuleFileNameW(hModule, path.data(), size);
      if (length == 0)
         return {};
      if (length < size)
      {
         path.resize(length); // Trim excess
         return path;
      }
      // length == size could both mean that it just did fit in, or it was truncated, so try again with a bigger buffer
      size *= 2;
   }
}
#endif

std::filesystem::path GetPluginPath()
{
#ifdef _WIN32
   HMODULE hm = nullptr;
   if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, _T("FlexDMDPluginLoad"), &hm) == 0)
      return std::filesystem::path();

#ifdef _UNICODE
   const std::wstring pathBuf = GetModulePath<std::wstring>(hm);
#else
   const string pathBuf = GetModulePath<string>(hm);
#endif
#else
   Dl_info info {};
   if (dladdr((void*)&GetPluginPath, &info) == 0 || !info.dli_fname)
      return string();

   char pathBuf[PATH_MAX];
   if (!realpath(info.dli_fname, pathBuf))
      return string();
#endif

   std::filesystem::path path(pathBuf);
   return path.empty() ? path : path.parent_path();
}

}
