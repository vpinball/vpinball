#pragma once

#include "Kerning.h"
#include "Character.h"
#include "Page.h"

#include <map>

class BitmapFont
{
public:
   BitmapFont();
   ~BitmapFont();
   
   static const int NoMaxWidth = -1;

   static BitmapFont* Create(string fileName);

   void Load(string fileName);
   int GetKerning(char previous, char current);
   int GetLineHeight() { return m_lineHeight; }
   int GetBaseHeight() { return m_baseHeight; }
   Page* GetPage(int index) { return m_pages[index]; }   
   int GetPageCount() { return m_pages.size(); }
   const std::map<char, Character*>& GetCharacters() const { return m_characters; }
   Character* GetCharacter(char key) const { auto it = m_characters.find(key); return it != m_characters.end() ? it->second : NULL; }
   void SetCharacter(char key, Character* character) { m_characters[key] = character; }

   SDL_Rect MeasureFont(const string& text, double maxWidth = NoMaxWidth);

   static string GetNamedString(const std::map<string, string>& parts, const string& name);
   static int GetNamedInt(const std::map<string, string>& parts, const string& name, int defaultValue = 0);
   static bool GetNamedBool(const std::map<string, string>& parts, const string& name, bool defaultValue = false);
   static SDL_Rect ParsePadding(const string& s);
   static SDL_Point ParsePoint(const string& s);
   static std::map<string, string> ParseParts(const string& line);

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
   std::map<int, Kerning*> m_kernings;
   std::map<char, Character*> m_characters;
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