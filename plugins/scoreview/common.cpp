#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>
#include <charconv>

namespace ScoreView {

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

static inline string trim_string(const string& str)
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

static inline string normalize_path_separators(const string& szPath)
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

string find_case_insensitive_file_path(const string& szPath)
{
   auto fn = [&](auto& self, const string& s) -> string {
      string path = normalize_path_separators(s);
      std::filesystem::path p = std::filesystem::path(path).lexically_normal();
      std::error_code ec;

      if (std::filesystem::exists(p, ec))
         return p.string();

      auto parent = p.parent_path();
      string base;
      if (parent.empty() || parent == p) {
         base = ".";
      } else {
         base = self(self, parent.string());
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
   };

   string result = fn(fn, szPath);
   if (!result.empty()) {
      std::filesystem::path p = std::filesystem::absolute(result);
      return p.string();
   }
   return string();
}

string PathFromFilename(const string& filename)
{
   const int len = (int)filename.length();
   // find the last '\' in the filename
   int end;
   for (end = len; end >= 0; end--)
   {
      if (filename[end] == PATH_SEPARATOR_CHAR)
         break;
   }

   if (end == 0)
      end = len - 1;

   // copy from the start of the string to the end (or last '\')
   const char* szT = filename.c_str();
   int count = end + 1;

   string path;
   while (count--)
   {
      path.push_back(*szT++);
   }
   return path;
}

string GetPluginPath()
{
    string pathBuf;
#ifdef _WIN32
    HMODULE hm = nullptr;
    if (GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            _T("ScoreViewPluginLoad"), &hm) == 0)
        return string();

    TCHAR buf[MAX_PATH];
    if (GetModuleFileName(hm, buf, MAX_PATH) == 0)
        return string();

#ifdef _UNICODE
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, buf, -1, NULL, 0, NULL, NULL);
    pathBuf.resize(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, buf, -1, pathBuf.data(), size_needed, NULL, NULL);
#else
    pathBuf = buf;
#endif
#else
    Dl_info info{};
    if (dladdr((void*)&GetPluginPath, &info) == 0 || !info.dli_fname)
        return string();

    char realBuf[PATH_MAX];
    if (!realpath(info.dli_fname, realBuf))
        return string();

    pathBuf = realBuf;
#endif

    if (pathBuf.empty())
        return string();

    size_t lastSep = pathBuf.find_last_of(PATH_SEPARATOR_CHAR);
    if (lastSep == string::npos)
        return string();

    return pathBuf.substr(0, lastSep + 1);
}

}
