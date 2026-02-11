// license:GPLv3+

#include "common.h"

#include <sstream>
#include <algorithm>
#include <filesystem>
#include <charconv>
#include <utility>

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

static bool try_parse_named_color(const string& str, ColorRGBA32& value)
{
   static constexpr std::pair<const char*, ColorRGBA32> namedColors[] = {
      {"aliceblue", RGB(240, 248, 255)},
      {"antiquewhite", RGB(250, 235, 215)},
      {"aqua", RGB(0, 255, 255)},
      {"aquamarine", RGB(127, 255, 212)},
      {"azure", RGB(240, 255, 255)},
      {"beige", RGB(245, 245, 220)},
      {"bisque", RGB(255, 228, 196)},
      {"black", RGB(0, 0, 0)},
      {"blanchedalmond", RGB(255, 235, 205)},
      {"blue", RGB(0, 0, 255)},
      {"blueviolet", RGB(138, 43, 226)},
      {"brown", RGB(165, 42, 42)},
      {"burlywood", RGB(222, 184, 135)},
      {"cadetblue", RGB(95, 158, 160)},
      {"chartreuse", RGB(127, 255, 0)},
      {"chocolate", RGB(210, 105, 30)},
      {"coral", RGB(255, 127, 80)},
      {"cornflowerblue", RGB(100, 149, 237)},
      {"cornsilk", RGB(255, 248, 220)},
      {"crimson", RGB(220, 20, 60)},
      {"cyan", RGB(0, 255, 255)},
      {"darkblue", RGB(0, 0, 139)},
      {"darkcyan", RGB(0, 139, 139)},
      {"darkgoldenrod", RGB(184, 134, 11)},
      {"darkgray", RGB(169, 169, 169)},
      {"darkgreen", RGB(0, 100, 0)},
      {"darkgrey", RGB(169, 169, 169)},
      {"darkkhaki", RGB(189, 183, 107)},
      {"darkmagenta", RGB(139, 0, 139)},
      {"darkolivegreen", RGB(85, 107, 47)},
      {"darkorange", RGB(255, 140, 0)},
      {"darkorchid", RGB(153, 50, 204)},
      {"darkred", RGB(139, 0, 0)},
      {"darksalmon", RGB(233, 150, 122)},
      {"darkseagreen", RGB(143, 188, 143)},
      {"darkslateblue", RGB(72, 61, 139)},
      {"darkslategray", RGB(47, 79, 79)},
      {"darkslategrey", RGB(47, 79, 79)},
      {"darkturquoise", RGB(0, 206, 209)},
      {"darkviolet", RGB(148, 0, 211)},
      {"deeppink", RGB(255, 20, 147)},
      {"deepskyblue", RGB(0, 191, 255)},
      {"dimgray", RGB(105, 105, 105)},
      {"dimgrey", RGB(105, 105, 105)},
      {"dodgerblue", RGB(30, 144, 255)},
      {"firebrick", RGB(178, 34, 34)},
      {"floralwhite", RGB(255, 250, 240)},
      {"forestgreen", RGB(34, 139, 34)},
      {"fuchsia", RGB(255, 0, 255)},
      {"gainsboro", RGB(220, 220, 220)},
      {"ghostwhite", RGB(248, 248, 255)},
      {"gold", RGB(255, 215, 0)},
      {"goldenrod", RGB(218, 165, 32)},
      {"gray", RGB(128, 128, 128)},
      {"green", RGB(0, 128, 0)},
      {"greenyellow", RGB(173, 255, 47)},
      {"grey", RGB(128, 128, 128)},
      {"honeydew", RGB(240, 255, 240)},
      {"hotpink", RGB(255, 105, 180)},
      {"indianred", RGB(205, 92, 92)},
      {"indigo", RGB(75, 0, 130)},
      {"ivory", RGB(255, 255, 240)},
      {"khaki", RGB(240, 230, 140)},
      {"lavender", RGB(230, 230, 250)},
      {"lavenderblush", RGB(255, 240, 245)},
      {"lawngreen", RGB(124, 252, 0)},
      {"lemonchiffon", RGB(255, 250, 205)},
      {"lightblue", RGB(173, 216, 230)},
      {"lightcoral", RGB(240, 128, 128)},
      {"lightcyan", RGB(224, 255, 255)},
      {"lightgoldenrodyellow", RGB(250, 250, 210)},
      {"lightgray", RGB(211, 211, 211)},
      {"lightgreen", RGB(144, 238, 144)},
      {"lightgrey", RGB(211, 211, 211)},
      {"lightpink", RGB(255, 182, 193)},
      {"lightsalmon", RGB(255, 160, 122)},
      {"lightseagreen", RGB(32, 178, 170)},
      {"lightskyblue", RGB(135, 206, 250)},
      {"lightslategray", RGB(119, 136, 153)},
      {"lightslategrey", RGB(119, 136, 153)},
      {"lightsteelblue", RGB(176, 196, 222)},
      {"lightyellow", RGB(255, 255, 224)},
      {"lime", RGB(0, 255, 0)},
      {"limegreen", RGB(50, 205, 50)},
      {"linen", RGB(250, 240, 230)},
      {"magenta", RGB(255, 0, 255)},
      {"maroon", RGB(128, 0, 0)},
      {"mediumaquamarine", RGB(102, 205, 170)},
      {"mediumblue", RGB(0, 0, 205)},
      {"mediumorchid", RGB(186, 85, 211)},
      {"mediumpurple", RGB(147, 112, 219)},
      {"mediumseagreen", RGB(60, 179, 113)},
      {"mediumslateblue", RGB(123, 104, 238)},
      {"mediumspringgreen", RGB(0, 250, 154)},
      {"mediumturquoise", RGB(72, 209, 204)},
      {"mediumvioletred", RGB(199, 21, 133)},
      {"midnightblue", RGB(25, 25, 112)},
      {"mintcream", RGB(245, 255, 250)},
      {"mistyrose", RGB(255, 228, 225)},
      {"moccasin", RGB(255, 228, 181)},
      {"navajowhite", RGB(255, 222, 173)},
      {"navy", RGB(0, 0, 128)},
      {"oldlace", RGB(253, 245, 230)},
      {"olive", RGB(128, 128, 0)},
      {"olivedrab", RGB(107, 142, 35)},
      {"orange", RGB(255, 165, 0)},
      {"orangered", RGB(255, 69, 0)},
      {"orchid", RGB(218, 112, 214)},
      {"palegoldenrod", RGB(238, 232, 170)},
      {"palegreen", RGB(152, 251, 152)},
      {"paleturquoise", RGB(175, 238, 238)},
      {"palevioletred", RGB(219, 112, 147)},
      {"papayawhip", RGB(255, 239, 213)},
      {"peachpuff", RGB(255, 218, 185)},
      {"peru", RGB(205, 133, 63)},
      {"pink", RGB(255, 192, 203)},
      {"plum", RGB(221, 160, 221)},
      {"powderblue", RGB(176, 224, 230)},
      {"purple", RGB(128, 0, 128)},
      {"rebeccapurple", RGB(102, 51, 153)},
      {"red", RGB(255, 0, 0)},
      {"rosybrown", RGB(188, 143, 143)},
      {"royalblue", RGB(65, 105, 225)},
      {"saddlebrown", RGB(139, 69, 19)},
      {"salmon", RGB(250, 128, 114)},
      {"sandybrown", RGB(244, 164, 96)},
      {"seagreen", RGB(46, 139, 87)},
      {"seashell", RGB(255, 245, 238)},
      {"sienna", RGB(160, 82, 45)},
      {"silver", RGB(192, 192, 192)},
      {"skyblue", RGB(135, 206, 235)},
      {"slateblue", RGB(106, 90, 205)},
      {"slategray", RGB(112, 128, 144)},
      {"slategrey", RGB(112, 128, 144)},
      {"snow", RGB(255, 250, 250)},
      {"springgreen", RGB(0, 255, 127)},
      {"steelblue", RGB(70, 130, 180)},
      {"tan", RGB(210, 180, 140)},
      {"teal", RGB(0, 128, 128)},
      {"thistle", RGB(216, 191, 216)},
      {"tomato", RGB(255, 99, 71)},
      {"turquoise", RGB(64, 224, 208)},
      {"violet", RGB(238, 130, 238)},
      {"wheat", RGB(245, 222, 179)},
      {"white", RGB(255, 255, 255)},
      {"whitesmoke", RGB(245, 245, 245)},
      {"yellow", RGB(255, 255, 0)},
      {"yellowgreen", RGB(154, 205, 50)},
      // Legacy UltraDMD options include this pseudo color.
      {"transparent", RGB(0, 0, 0)},
   };

   const string name = string_to_lower(trim_string(str));
   if (name.empty())
      return false;

   for (const auto& namedColor : namedColors)
   {
      if (name == namedColor.first)
      {
         value = namedColor.second;
         return true;
      }
   }

   return false;
}

bool try_parse_color(const string& str, ColorRGBA32& value)
{
   const string input = trim_string(str);
   if (input.empty())
      return false;

   string hexStr;
   if (input[0] == '#')
      hexStr = input.substr(1);
   else
      hexStr = input;

   if (hexStr.size() == 6)
      hexStr += "FF";

   if (hexStr.size() == 8)
   {
      uint32_t rgba;
      std::stringstream ss;
      ss << std::hex << hexStr;
      if (ss >> rgba)
      {
         const uint8_t r = (rgba >> 24) & 0xFF;
         const uint8_t g = (rgba >> 16) & 0xFF;
         const uint8_t b = (rgba >> 8)  & 0xFF;

         value = r | (g << 8) | (b << 16);
         return true;
      }
   }

   return try_parse_named_color(input, value);
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
   auto fn = [&](auto& self, const string& s) -> string {
      string path = normalize_path_separators(s);
      std::filesystem::path p = std::filesystem::path(path).lexically_normal();
      std::error_code ec;

      if (std::filesystem::exists(p, ec))
         return p.string();

      auto parent = p.parent_path();
      string base;
      if (parent.empty() || parent == p) {
         base = "."s;
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
            _T("FlexDMDPluginLoad"), &hm) == 0)
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
