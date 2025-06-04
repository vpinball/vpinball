#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>
#include <charconv>

namespace Flex {

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

string string_to_lower(string str)
{
   std::ranges::transform(str.begin(), str.end(), str.begin(), cLower);
   return str;
}

string trim_string(const string& str)
{
   string s;
   try
   {
      const size_t pos = str.find_first_not_of(" \t\r\n");
      s = str.substr(pos, str.find_last_not_of(" \t\r\n") - pos + 1);
   }
   catch (...)
   {
      //s.clear();
   }
   return s;
}

bool try_parse_int(const string& str, int& value)
{
   const string tmp = trim_string(str);
   return (std::from_chars(tmp.c_str(), tmp.c_str() + tmp.length(), value).ec == std::errc{});
}

bool try_parse_color(const string& str, ColorRGBA32& value)
{
   string hexStr;
   if (str[0] == '#')
      hexStr = str.substr(1);
   else
      hexStr = str;

   if (hexStr.size() == 6)
      hexStr += "FF";

   if (hexStr.size() != 8)
      return false;

   uint32_t rgba;
   std::stringstream ss;
   ss << std::hex << hexStr;
   if (!(ss >> rgba))
      return false;

   uint8_t r = (rgba >> 24) & 0xFF;
   uint8_t g = (rgba >> 16) & 0xFF;
   uint8_t b = (rgba >> 8) & 0xFF;

   value = r | (g << 8) | (b << 16);

   return true;
}

int string_to_int(const string& str, int defaultValue)
{
   int value;
   return try_parse_int(str, value) ? value : defaultValue;
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

string extension_from_path(const string& path)
{
   const size_t pos = path.find_last_of('.');
   return pos != string::npos ? string_to_lower(path.substr(pos + 1)) : string();
}

string find_case_insensitive_file_path(const string& szPath)
{
   string path = normalize_path_separators(szPath);
   std::filesystem::path p = std::filesystem::path(path).lexically_normal();
   std::error_code ec;

   if (std::filesystem::exists(p, ec))
      return path;

   auto parent = p.parent_path();
   string base;
   if (parent.empty() || parent == p)
      base = '.';
   else {
      base = find_case_insensitive_file_path(parent.string());
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
}

string GetPluginPath()
{
    string pathBuf;
#ifdef _WIN32
    HMODULE hm = nullptr;
    if (GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            _T("FlexDMDPluginLoad"), &hm) == 0)
        return string();

    TCHAR buf[MAX_PATH];
    if (GetModuleFileName(hm, buf, MAX_PATH) == 0)
        return string();

#ifdef _UNICODE
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
    pathBuf.resize(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, pathBuf.data(), size_needed, NULL, NULL);
#else
    pathBuf = string(buf);
#endif
#else
    Dl_info info{};
    if (dladdr((void*)&GetPluginPath, &info) == 0 || !info.dli_fname)
        return string();

    char realBuf[PATH_MAX];
    if (!realpath(info.dli_fname, realBuf))
        return string();

    pathBuf = string(realBuf);
#endif

    if (pathBuf.empty())
        return string();

    size_t lastSep = pathBuf.find_last_of(PATH_SEPARATOR_CHAR);
    if (lastSep == string::npos)
        return string();

    return pathBuf.substr(0, lastSep + 1);
}

}
