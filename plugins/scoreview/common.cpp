// license:GPLv3+

#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>
#include <charconv>

namespace ScoreView {

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

static inline string trim_string(const string& str)
{
   size_t start = 0;
   size_t end = str.length();
   while (start < end && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n'))
      ++start;
   while (end > start && (str[end - 1] == ' ' || str[end - 1] == '\t' || str[end - 1] == '\r' || str[end - 1] == '\n'))
      --end;
   return str.substr(start, end - start);
}

string TrimLeading(const string& str, const string& whitespace)
{
   if (str.empty())
      return string();
   const auto strBegin = str.find_first_not_of(whitespace);
   if (strBegin == std::string::npos)
      return string();
   return string(str.cbegin() + strBegin, str.cend());
}

string TrimTrailing(const string& str, const string& whitespace)
{
   if (str.empty())
      return string();
   const auto pos = str.find_last_not_of(whitespace);
   if (pos == string::npos)
      return string();
   const auto strBegin = str.cbegin();
   const auto strEnd = strBegin + pos + 1;
   return string(strBegin, strEnd);
}

bool try_parse_float(const string& str, float& value)
{
   const string tmp = trim_string(str);
#if defined(__clang__)
   const char* const p = tmp.c_str();
   char* e;
   value = std::strtof(p, &e);
   return (p != e);
#else
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc{});
#endif
}

bool try_parse_int(const string& str, int& value)
{
   const string tmp = trim_string(str);
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc{});
}

string PathFromFilename(const string &filename)
{
   const size_t pos = filename.find_last_of(PATH_SEPARATOR_CHAR);
   return (pos == string::npos) ? string() : filename.substr(0, pos + 1); // previously returned filename if no separator found, but i guess that just worked because filename was then also constantly ""
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

string GetPluginPath()
{
#ifdef _WIN32
    HMODULE hm = nullptr;
    if (GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            _T("ScoreViewPluginLoad"), &hm) == 0)
        return string();

#ifdef _UNICODE
    const std::wstring buf = GetModulePath<std::wstring>(hm);
    if (buf.empty())
       return string();
    const int size_needed = WideCharToMultiByte(CP_UTF8, 0, buf.c_str(), -1, nullptr, 0, nullptr, nullptr);
    string pathBuf(size_needed - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, buf.c_str(), -1, pathBuf.data(), size_needed, nullptr, nullptr);
#else
    const string pathBuf = GetModulePath<string>(hm);
#endif
#else
    Dl_info info{};
    if (dladdr((void*)&GetPluginPath, &info) == 0 || !info.dli_fname)
        return string();

    char realBuf[PATH_MAX];
    if (!realpath(info.dli_fname, realBuf))
        return string();

    const string pathBuf(realBuf);
#endif

    if (pathBuf.empty())
        return string();

    const size_t lastSep = pathBuf.find_last_of(PATH_SEPARATOR_CHAR);
    if (lastSep == string::npos)
        return string();

    return pathBuf.substr(0, lastSep + 1);
}

}
