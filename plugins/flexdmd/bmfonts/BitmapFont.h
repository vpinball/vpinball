#pragma once

#include <unordered_dense.h>

#include "common.h"
#include "Kerning.h"
#include "Character.h"
#include "Page.h"
#include <SDL3/SDL_surface.h>

namespace Flex {

class BitmapFont final
{
public:
   BitmapFont();
   ~BitmapFont();

   static constexpr int NoMaxWidth = -1;

   static BitmapFont* Create(const string& filename);

   void Load(const string& filename);
   int GetKerning(char previous, char current);
   int GetLineHeight() const { return m_lineHeight; }
   int GetBaseHeight() const { return m_baseHeight; }
   Page* GetPage(int index) const { return m_pages[index]; }
   int GetPageCount() const { return static_cast<int>(m_pages.size()); }
   const ankerl::unordered_dense::map<char, Character*>& GetCharacters() const { return m_characters; }
   Character* GetCharacter(char key) const { const auto it = m_characters.find(key); return it != m_characters.end() ? it->second : nullptr; }
   void SetCharacter(char key, Character* character) { m_characters[key] = character; }

   SDL_Rect MeasureFont(const string& text, double maxWidth = NoMaxWidth);

   static string GetNamedString(const ankerl::unordered_dense::map<string, string>& parts, const string& name);
   static int GetNamedInt(const ankerl::unordered_dense::map<string, string>& parts, const string& name, int defaultValue = 0);
   static bool GetNamedBool(const ankerl::unordered_dense::map<string, string>& parts, const string& name, bool defaultValue = false);
   static SDL_Rect ParsePadding(const string& s);
   static SDL_Point ParsePoint(const string& s);
   static ankerl::unordered_dense::map<string, string> ParseParts(const string& line);

private:
   int m_alphaChannel;
   int m_redChannel;
   int m_greenChannel;
   int m_blueChannel;
   bool m_bold;
   bool m_italic;
   bool m_packed;
   bool m_smoothed;
   bool m_unicode;

   string m_szCharset;
   string m_szFamilyName;
   int m_fontSize;
   ankerl::unordered_dense::map<int, Kerning*> m_kernings;
   ankerl::unordered_dense::map<char, Character*> m_characters;
   int m_baseHeight;
   int m_lineHeight;
   int m_stretchedHeight;
   int m_outlineSize;
   SDL_Rect m_padding;
   vector<Page*> m_pages;
   SDL_Point m_spacing;
   int m_superSampling;
   SDL_Rect m_textureSize;
};

}
