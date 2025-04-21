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
      s = str.substr(str.find_first_not_of(" \t\r\n"), str.find_last_not_of(" \t\r\n") - str.find_first_not_of(" \t\r\n") + 1);
   }
   catch (...)
   {
      //s.clear();
   }
   return s;
}

bool try_parse_int(const string& str, int& value)
{
   std::stringstream sstr(trim_string(str));
   return ((sstr >> value) && sstr.eof());
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
   if (try_parse_int(str, value))
      return value;

   return defaultValue;
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
   return pos != string::npos ? string_to_lower(path).substr(pos + 1) : string();
}

string find_case_insensitive_file_path(const string& szPath)
{
   std::filesystem::path p(normalize_path_separators(szPath));
   if (p.is_relative())
      p = std::filesystem::current_path() / p;
   p = p.lexically_normal();

   if (std::filesystem::exists(p)) {
      if (std::filesystem::is_regular_file(p))
         return std::filesystem::canonical(p).string();
      else
         return string();
   }

   std::filesystem::path result = p.root_path();
   if (result.empty())
      result = std::filesystem::current_path().root_path();

   for (auto it = std::next(p.begin()); it != p.end(); ++it) {
      const string name = it->string();
      if (!std::filesystem::exists(result) || !std::filesystem::is_directory(result))
         return string();

      bool matched = false;
      for (const auto& entry : std::filesystem::directory_iterator(result)) {
         const string fname = entry.path().filename().string();
         if (StrCompareNoCase(fname, name)) {
            result /= entry.path().filename();
            matched = true;
            break;
         }
      }
      if (!matched)
         return string();
   }

   if (!std::filesystem::exists(result) || !std::filesystem::is_regular_file(result))
      return string();

   string match = std::filesystem::canonical(result).string();
   LOGI("exact directory not found, but a case-insensitive file match was found: szPath=%s, match=%s", szPath.c_str(), match.c_str());
   return match;
}