// license:GPLv3+

#include "core/stdafx.h"
#include "TextRenderer.h"

#include "core/VPApp.h"
#include "parts/pintable.h"
#include "utils/fileio.h"

#include <SDL3_ttf/SDL_ttf.h>

#ifdef VPX_HAS_WIN32_SDK
// File of an installed Windows font, from the registry list of installed fonts. Its entries are named
// '<face>[ & <face>...] (TrueType|OpenType)', with a file name relative to the Windows font folder, or
// a full path for the fonts installed for the current user only
static std::filesystem::path FindWindowsFontFile(const string &face)
{
   const wstring wface = MakeWString(face);
   for (const HKEY root : { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE })
   {
      HKEY key;
      if (RegOpenKeyExW(root, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &key) != ERROR_SUCCESS)
         continue;
      wstring file;
      WCHAR name[512];
      WCHAR data[MAX_PATH];
      for (DWORD i = 0; file.empty(); i++)
      {
         DWORD nameLength = static_cast<DWORD>(std::size(name));
         DWORD dataSize = sizeof(data);
         DWORD type;
         const LSTATUS result = RegEnumValueW(key, i, name, &nameLength, nullptr, &type, reinterpret_cast<LPBYTE>(data), &dataSize);
         if (result == ERROR_NO_MORE_ITEMS)
            break;
         if (result != ERROR_SUCCESS || type != REG_SZ)
            continue;
         wstring faces(name, nameLength);
         if (const size_t format = faces.rfind(L" ("); format != wstring::npos)
            faces.resize(format);
         for (size_t start = 0; start <= faces.size();)
         {
            const size_t end = std::min(faces.find(L" & ", start), faces.size());
            if (_wcsicmp(faces.substr(start, end - start).c_str(), wface.c_str()) == 0)
            {
               file.assign(data, wcsnlen(data, dataSize / sizeof(WCHAR)));
               break;
            }
            start = end + 3;
         }
      }
      RegCloseKey(key);
      if (!file.empty())
      {
         std::filesystem::path path(file);
         if (path.is_relative())
         {
            WCHAR windowsDir[MAX_PATH];
            const UINT length = GetWindowsDirectoryW(windowsDir, static_cast<UINT>(std::size(windowsDir)));
            path = std::filesystem::path(wstring(windowsDir, length)) / L"Fonts" / path;
         }
         return path;
      }
   }
   return {};
}
#endif

// Embedded font whose name matches the requested face (single face, styles are synthesized by SDL_ttf)
static const PinFont *FindEmbeddedFont(const PinTable *const table, const string &fontName)
{
   const auto normalize = [](const string &s)
   {
      string result;
      for (const char c : s)
         if (c != ' ' && c != '-' && c != '_')
            result += (char)tolower((unsigned char)c);
      return result;
   };
   const string face = normalize(fontName);
   for (const PinFont *const font : table->GetFontList())
   {
      string embeddedName = font->m_name;
      // Stored names are file names such as 'Impact.ttf'; compare against the stem
      if (const size_t dot = embeddedName.rfind('.'); dot != string::npos)
         embeddedName.resize(dot);
      // Match 'Impact' as well as style suffixed names such as 'Impact Bold' or 'Impact-Bold'
      if (normalize(embeddedName).starts_with(face))
         return font;
   }
   return nullptr;
}

TTF_Font *TextRenderer::LoadFont(const FontDesc &font, const PinTable *table)
{
   if (!TTF_Init())
   {
      PLOGW << "Unable to initialize SDL_ttf: " << SDL_GetError();
      return nullptr;
   }

   TTF_Font *pFont = nullptr;
   const float ptSize = (float)(font.size / 10000.);

   string fontName = font.name;
   std::erase(fontName, ' ');

   // Font faces to look for, best match first, with the file suffix used for the fonts provided along the table and the
   // face name suffix of installed fonts. The style a face lacks is synthesized by SDL_ttf (unless it is already built in)
   struct FontStyle
   {
      string fileSuffix;
      string faceSuffix;
      bool bold, italic;
   };
   vector<FontStyle> styles;
   if (font.IsBold() && font.IsItalic())
      styles.push_back({ "-BoldItalic"s, " Bold Italic"s, true, true });
   if (font.IsBold())
      styles.push_back({ "-Bold"s, " Bold"s, true, false });
   if (font.IsItalic())
      styles.push_back({ "-Italic"s, " Italic"s, false, true });
   styles.push_back({ "-Regular"s, string(), false, false });

   const FontStyle *faceStyle = &styles.back();
   if (table)
   {
      const std::filesystem::path tablePath = PathFromFilename(table->m_filename);
      std::filesystem::path path;
      for (const auto &style : styles)
      {
         path = find_case_insensitive_file_path(tablePath / (fontName + style.fileSuffix + ".ttf"));
         if (!path.empty())
         {
            pFont = TTF_OpenFont(PathToUTF8(path).c_str(), ptSize); // SDL expects UTF-8
            if (pFont)
            {
               PLOGI << "Font loaded: path=" << PathToUTF8(path);
               faceStyle = &style;
               break;
            }
         }
      }
   }

   if (!pFont && table)
   {
      if (const PinFont *const embedded = FindEmbeddedFont(table, fontName); embedded && !embedded->m_buffer.empty())
      {
         pFont = TTF_OpenFontIO(SDL_IOFromConstMem(embedded->m_buffer.data(), embedded->m_buffer.size()), true, ptSize);
         if (pFont)
            PLOGI << "Embedded font loaded: name=" << embedded->m_name;
      }
   }

#ifdef VPX_HAS_WIN32_SDK
   if (!pFont)
   {
      std::filesystem::path path;
      for (const auto &style : styles)
      {
         path = FindWindowsFontFile(font.name + style.faceSuffix);
         if (!path.empty())
         {
            pFont = TTF_OpenFont(PathToUTF8(path).c_str(), ptSize); // SDL expects UTF-8
            if (pFont)
            {
               PLOGI << "Font loaded: path=" << PathToUTF8(path);
               faceStyle = &style;
               break;
            }
         }
      }
   }
#endif

   if (!pFont)
   {
      if (table)
      {
         const std::filesystem::path tablePath = PathFromFilename(table->m_filename);
         PLOGW << "Unable to locate font: path=" << PathToUTF8(tablePath / (fontName + styles[0].fileSuffix + ".ttf"));
      }

      const std::filesystem::path path = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "LiberationSans-Regular.ttf"sv;
      pFont = TTF_OpenFont(PathToUTF8(path).c_str(), ptSize); // SDL expects UTF-8
      if (pFont)
      {
         PLOGW << "Default font loaded: path=" << PathToUTF8(path);
      }
      else
      {
         PLOGW << "Unable to load font: path=" << PathToUTF8(path);
         TTF_Quit();
         return nullptr;
      }
   }

   TTF_FontStyleFlags style = TTF_STYLE_NORMAL;
   if (font.IsBold() && !faceStyle->bold)
      style |= TTF_STYLE_BOLD;
   if (font.IsItalic() && !faceStyle->italic)
      style |= TTF_STYLE_ITALIC;
   if (font.IsUnderline())
      style |= TTF_STYLE_UNDERLINE;
   if (font.IsStrikeThrough())
      style |= TTF_STYLE_STRIKETHROUGH;
   TTF_SetFontStyle(pFont, style);

   return pFont;
}
