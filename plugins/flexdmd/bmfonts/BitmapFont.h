#pragma once

#include "common.h"
#include "Kerning.h"
#include "Character.h"
#include "Page.h"
#include <SDL3/SDL_surface.h>

#include <unordered_map>

class BitmapFont final
{
public:
   BitmapFont();
   ~BitmapFont();
   
   static constexpr int NoMaxWidth = -1;

   static BitmapFont* Create(const string& fileName);

   void Load(const string& fileName);
   int GetKerning(char previous, char current);
   int GetLineHeight() const { return m_lineHeight; }
   int GetBaseHeight() const { return m_baseHeight; }
   Page* GetPage(int index) const { return m_pages[index]; }   
   int GetPageCount() const { return static_cast<int>(m_pages.size()); }
   const std::unordered_map<char, Character*>& GetCharacters() const { return m_characters; }
   Character* GetCharacter(char key) const { auto it = m_characters.find(key); return it != m_characters.end() ? it->second : nullptr; }
   void SetCharacter(char key, Character* character) { m_characters[key] = character; }

   SDL_Rect MeasureFont(const string& text, double maxWidth = NoMaxWidth);

   static string GetNamedString(const std::unordered_map<string, string>& parts, const string& name);
   static int GetNamedInt(const std::unordered_map<string, string>& parts, const string& name, int defaultValue = 0);
   static bool GetNamedBool(const std::unordered_map<string, string>& parts, const string& name, bool defaultValue = false);
   static SDL_Rect ParsePadding(const string& s);
   static SDL_Point ParsePoint(const string& s);
   static std::unordered_map<string, string> ParseParts(const string& line);

private:
   int m_alphaChannel;
   int m_baseHeight;
   int m_blueChannel;
   bool m_bold;

   string m_szCharset;
   string m_szFamilyName;
   int m_fontSize;
   int m_greenChannel;
   bool m_italic;
   std::unordered_map<int, Kerning*> m_kernings;
   std::unordered_map<char, Character*> m_characters;
   int m_lineHeight;
   int m_outlineSize;
   bool m_packed;
   SDL_Rect m_padding;
   vector<Page*> m_pages;
   int m_redChannel;
   bool m_smoothed;
   SDL_Point m_spacing;
   int m_stretchedHeight;
   int m_superSampling;
   SDL_Rect m_textureSize;
   bool m_unicode;
};
