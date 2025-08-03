#include "common.h"
#include <filesystem>
#include <algorithm>
#include <charconv>

namespace B2SLegacy
{

constexpr inline char cLower(char c)
{
   if (c >= 'A' && c <= 'Z')
      c ^= 32; // ASCII convention
   return c;
}

bool StrCompareNoCase(const string& strA, const string& strB)
{
   return strA.length() == strB.length()
      && std::equal(strA.begin(), strA.end(), strB.begin(),
         [](char a, char b) { return cLower(a) == cLower(b); });
}

string find_case_insensitive_file_path(const string &szPath)
{
   auto fn = [&](auto& self, const string& s) -> string {
      std::filesystem::path p = std::filesystem::path(s).lexically_normal();
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

vector<unsigned char> base64_decode(const string &encoded_string)
{
   static const string base64_chars =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
      "0123456789+/"s;

   string input = encoded_string;
   input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
   input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());

   int in_len = static_cast<int>(input.size());
   int i = 0, in_ = 0;
   unsigned char char_array_4[4], char_array_3[3];
   vector<unsigned char> ret;

   while (in_len-- && (input[in_] != '=') && (std::isalnum(input[in_]) || (input[in_] == '+') || (input[in_] == '/'))) {
      char_array_4[i++] = input[in_];
      in_++;
      if (i == 4) {
         for (i = 0; i < 4; i++)
            char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

         char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
         char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
         char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

         for (i = 0; i < 3; i++)
            ret.push_back(char_array_3[i]);
         i = 0;
     }
   }

   if (i) {
      for (int j = i; j < 4; j++)
         char_array_4[j] = 0;

      for (int j = 0; j < 4; j++)
         char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

      char_array_3[0] =  (char_array_4[0]        << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

      for (int j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
   }

   return ret;
}

bool string_starts_with_case_insensitive(const string& str, const string& prefix)
{
   if (prefix.length() > str.length())
      return false;
   return std::equal(prefix.begin(), prefix.end(), str.begin(),
      [](char a, char b) { return cLower(a) == cLower(b); });
}

int string_to_int(const string& str, int defaultValue)
{
   int value;
   if (std::from_chars(str.c_str(), str.c_str() + str.length(), value).ec == std::errc{})
      return value;
   else
      return defaultValue;
}

string title_and_path_from_filename(const string& filename)
{
   return filename.substr(0, filename.find_last_of('.'));
}

bool is_string_numeric(const string& str)
{
   if (str.empty()) return false;
   for (char c : str) {
      if (!std::isdigit(c)) return false;
   }
   return true;
}

}
