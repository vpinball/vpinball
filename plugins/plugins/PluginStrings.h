// license:GPLv3+

#pragma once

// String and path helpers for the plugins, which can not use the VPX core ones (see the string contract in src/core/def.h).
// Narrow strings are UTF-8, except the VPXPluginAPI table and application paths (native narrow, see PathFromNative). On Windows,
// path(std::string) and path::string() use the ANSI code page (string() may even throw): never use them, convert with these helpers

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

namespace PluginStrings
{

// Decodes the UTF-8 character at i into cp and advances i, or returns false for an invalid sequence (advancing i by one byte)
inline bool DecodeUTF8(std::string_view text, size_t& i, uint32_t& cp)
{
   const uint8_t lead = static_cast<uint8_t>(text[i]);
   size_t n;
   uint32_t c;
   if (lead < 0x80) { cp = lead; i++; return true; }
   else if ((lead & 0xE0) == 0xC0) { n = 1; c = lead & 0x1F; }
   else if ((lead & 0xF0) == 0xE0) { n = 2; c = lead & 0x0F; }
   else if ((lead & 0xF8) == 0xF0) { n = 3; c = lead & 0x07; }
   else { i++; return false; }
   if (text.size() - i <= n) { i++; return false; } // Sequence cut at the end
   for (size_t k = 1; k <= n; k++)
   {
      const uint8_t b = static_cast<uint8_t>(text[i + k]);
      if ((b & 0xC0) != 0x80) { i++; return false; }
      c = (c << 6) | (b & 0x3F);
   }
   static constexpr uint32_t minCodepoint[4] = { 0, 0x80, 0x800, 0x10000 };
   if (c < minCodepoint[n] || c > 0x10FFFF || (c >= 0xD800 && c <= 0xDFFF)) { i++; return false; } // Overlong, out of range or surrogate
   cp = c;
   i += n + 1;
   return true;
}

inline void AppendUTF8(std::string& text, uint32_t cp)
{
   if (cp < 0x80)
      text += static_cast<char>(cp);
   else if (cp < 0x800)
   {
      text += static_cast<char>(0xC0 | (cp >> 6));
      text += static_cast<char>(0x80 | (cp & 0x3F));
   }
   else if (cp < 0x10000)
   {
      text += static_cast<char>(0xE0 | (cp >> 12));
      text += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      text += static_cast<char>(0x80 | (cp & 0x3F));
   }
   else
   {
      text += static_cast<char>(0xF0 | (cp >> 18));
      text += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
      text += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
      text += static_cast<char>(0x80 | (cp & 0x3F));
   }
}

inline bool IsValidUTF8(std::string_view text)
{
   uint32_t cp;
   for (size_t i = 0; i < text.size();)
      if (!DecodeUTF8(text, i, cp))
         return false;
   return true;
}

// Text of data files (CSV, INI,...): UTF-8 (a BOM is skipped), or else legacy Windows-1252 converted to UTF-8
inline std::string TextFromUTF8OrCP1252(std::string_view text)
{
   if (text.starts_with("\xEF\xBB\xBF"))
      text.remove_prefix(3);
   if (IsValidUTF8(text))
      return std::string(text);
   static constexpr uint16_t cp1252_80_9f[32] = {
      0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
      0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178 };
   std::string utf8;
   utf8.reserve(text.size() * 3 / 2);
   for (const char ch : text)
   {
      const uint8_t b = static_cast<uint8_t>(ch);
      AppendUTF8(utf8, (b >= 0x80 && b <= 0x9F) ? cp1252_80_9f[b - 0x80] : b);
   }
   return utf8;
}

// UTF-8 paths: invalid characters become U+FFFD instead of throwing. Converted by hand on Windows, as the standard library throws
// for invalid input and windows.h would leak its macros into every plugin source
inline std::filesystem::path PathFromUTF8(std::string_view utf8)
{
#ifdef _WIN32
   std::wstring wide;
   wide.reserve(utf8.size());
   uint32_t cp;
   for (size_t i = 0; i < utf8.size();)
   {
      if (!DecodeUTF8(utf8, i, cp))
         cp = 0xFFFD;
      if (cp >= 0x10000)
      {
         wide += static_cast<wchar_t>(0xD800 + ((cp - 0x10000) >> 10));
         wide += static_cast<wchar_t>(0xDC00 + ((cp - 0x10000) & 0x3FF));
      }
      else
         wide += static_cast<wchar_t>(cp);
   }
   return wide;
#else
   return std::filesystem::path(std::string(utf8));
#endif
}

inline std::string PathToUTF8(const std::filesystem::path& path)
{
#ifdef _WIN32
   const std::wstring& wide = path.native();
   std::string utf8;
   utf8.reserve(wide.size());
   for (size_t i = 0; i < wide.size(); i++)
   {
      uint32_t cp = static_cast<uint16_t>(wide[i]);
      if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < wide.size() && static_cast<uint16_t>(wide[i + 1]) >= 0xDC00 && static_cast<uint16_t>(wide[i + 1]) <= 0xDFFF)
         cp = 0x10000 + ((cp - 0xD800) << 10) + (static_cast<uint16_t>(wide[++i]) - 0xDC00);
      else if (cp >= 0xD800 && cp <= 0xDFFF)
         cp = 0xFFFD; // Lone surrogate
      AppendUTF8(utf8, cp);
   }
   return utf8;
#else
   return path.native();
#endif
}

// Native narrow paths, as VPX provides the VPXPluginAPI table and application paths (VPXTableInfo, VPXInfo): the process code page with
// MSVC (legacy ANSI, or UTF-8 with the manifest), UTF-8 elsewhere. nullptr gives an empty path
inline std::filesystem::path PathFromNative(const char* native)
{
   if (native == nullptr)
      return {};
   try
   {
      return std::filesystem::path(native);
   }
   catch (...) // Not valid in the process code page
   {
      return PathFromUTF8(native);
   }
}

// Native narrow path for libraries opening files with narrow APIs (e.g. libpinmame). Characters the process code page can not represent
// (path::string() throws for them) become '_', so the result may only approximate the path
inline std::string PathToNative(const std::filesystem::path& path)
{
   try
   {
      return path.string();
   }
   catch (...)
   {
      std::string narrow = PathToUTF8(path);
      for (char& c : narrow)
         if (static_cast<uint8_t>(c) >= 0x80)
            c = '_';
      return narrow;
   }
}

// Settings written by older versions may hold native narrow paths instead of UTF-8
inline std::filesystem::path PathFromUTF8OrNative(std::string_view text)
{
   if (IsValidUTF8(text))
      return PathFromUTF8(text);
   return PathFromNative(std::string(text).c_str());
}

// Compares names ignoring ASCII case (file names on Windows, VBScript), byte or unit wise for anything else
template <class C> inline bool EqualsNoCase(std::basic_string_view<C> a, std::basic_string_view<C> b)
{
   if (a.size() != b.size())
      return false;
   for (size_t i = 0; i < a.size(); i++)
   {
      C ca = a[i], cb = b[i];
      if (ca >= C('A') && ca <= C('Z'))
         ca = static_cast<C>(ca - C('A') + C('a'));
      if (cb >= C('A') && cb <= C('Z'))
         cb = static_cast<C>(cb - C('A') + C('a'));
      if (ca != cb)
         return false;
   }
   return true;
}

// Resolves a path whose parts may differ in case from the file system (case sensitive file systems, tables made on Windows).
// Returns the existing absolute path, or an empty path if not found. Never throws
inline std::filesystem::path FindPathNoCase(const std::filesystem::path& searched, const bool directory = false)
{
   auto find = [directory](const auto& self, std::filesystem::path path) -> std::filesystem::path
   {
      std::error_code ec;
      path = path.lexically_normal();
      if (std::filesystem::exists(path, ec) && (!directory || std::filesystem::is_directory(path, ec)))
         return path;

      const std::filesystem::path parent = path.parent_path();
      const std::filesystem::path base = (parent.empty() || parent == path) ? std::filesystem::path(".") : self(self, parent);
      if (base.empty())
         return {};

      const std::filesystem::path::string_type name = path.filename().native(); // A copy: filename() is a temporary
      for (std::filesystem::directory_iterator it(base, ec), end; !ec && it != end; it.increment(ec))
      {
         std::error_code entryError;
         if (directory && !it->is_directory(entryError))
            continue;
         if (EqualsNoCase<std::filesystem::path::value_type>(it->path().filename().native(), name))
            return it->path();
      }
      return {};
   };

   const std::filesystem::path result = find(find, searched);
   if (result.empty())
      return result;
   std::error_code ec;
   std::filesystem::path absolute = std::filesystem::absolute(result, ec);
   return ec ? result : absolute;
}

}
