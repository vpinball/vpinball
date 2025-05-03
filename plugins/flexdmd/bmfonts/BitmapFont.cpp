#include "BitmapFont.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <unordered_dense.h>

BitmapFont::BitmapFont()
{
   m_padding = { 0, 0, 0, 0 };
   m_spacing = { 0, 0 };
   m_textureSize = { 0, 0, 0, 0 };
}

BitmapFont::~BitmapFont()
{
   {
   ankerl::unordered_dense::set<Character*> characters;
   for(const auto& pair : m_characters)
      characters.insert(pair.second);

   for(Character* character : characters)
      delete character;
   }

   m_characters.clear();

   for (auto& it : m_kernings)
      delete it.second;

   m_kernings.clear();

   for (auto it : m_pages)
      delete it;

   m_pages.clear();
}

BitmapFont* BitmapFont::Create(const string& fileName)
{
   BitmapFont* obj = new BitmapFont();
   obj->Load(fileName);

   return obj;
}

int BitmapFont::GetKerning(char previous, char current)
{
    Kerning key;
    key.SetFirstCharacter(previous);
    key.SetSecondCharacter(current);

    const auto it = m_kernings.find(key.GetHash());
    if (it != m_kernings.end())
       return it->second->GetAmount();

    return 0;
}

void BitmapFont::Load(const string& fileName)
{
   std::ifstream fontFile;
   fontFile.open(fileName, std::ifstream::in);

   if (!fontFile.is_open()) {
      // PLOGE.printf("Failed to open bitmap font file: %s", fileName.c_str());
      return;
   }

   string line;
   while (std::getline(fontFile, line)) {
      const ankerl::unordered_dense::map<string, string> parts = ParseParts(trim_string(line));

      if (parts.empty())
         continue;

      const auto& sec = parts.at("section"s);
      if (sec == "info") {
         m_szFamilyName = GetNamedString(parts, "face"s);
         m_fontSize = GetNamedInt(parts, "size"s);
         m_bold = GetNamedBool(parts, "bold"s);
         m_italic = GetNamedBool(parts, "italic"s);
         m_szCharset = GetNamedString(parts, "charset"s);
         m_unicode = GetNamedBool(parts, "unicode"s);
         m_stretchedHeight = GetNamedInt(parts, "stretchH"s);
         m_smoothed = GetNamedBool(parts, "smooth"s);
         m_superSampling = GetNamedInt(parts, "aa"s);
         m_padding = ParsePadding(GetNamedString(parts, "padding"s));
         m_spacing = ParsePoint(GetNamedString(parts, "spacing"s));
         m_outlineSize = GetNamedInt(parts, "outline"s);
      }
      else if (sec == "common") {
         m_lineHeight = GetNamedInt(parts, "lineHeight"s);
         m_baseHeight = GetNamedInt(parts, "base"s);
         m_textureSize = { 0, 0, GetNamedInt(parts, "scaleW"s), GetNamedInt(parts, "scaleH"s) };
         m_packed = GetNamedBool(parts, "packed"s);
         m_alphaChannel = GetNamedInt(parts, "alphaChnl"s);
         m_redChannel = GetNamedInt(parts, "redChnl"s);
         m_greenChannel = GetNamedInt(parts, "greenChnl"s);
         m_blueChannel = GetNamedInt(parts, "blueChnl"s);
      }
      else if (sec == "page") {
         Page* const pPage = new Page();
         pPage->SetId(GetNamedInt(parts, "id"s));
         pPage->SetFilename(GetNamedString(parts, "file"s));

         m_pages.push_back(pPage);
      }
      else if (sec == "char") {
         char character = (char)GetNamedInt(parts, "id"s);

         Character* const pCharacter = new Character();
         pCharacter->SetChar(character);
         pCharacter->SetBounds({ GetNamedInt(parts, "x"s), GetNamedInt(parts, "y"s), GetNamedInt(parts, "width"s), GetNamedInt(parts, "height"s) });
         pCharacter->SetOffset({ GetNamedInt(parts, "xoffset"s), GetNamedInt(parts, "yoffset"s) });
         pCharacter->SetXAdvance(GetNamedInt(parts, "xadvance"s));
         pCharacter->SetTexturePage(GetNamedInt(parts, "page"s));
         pCharacter->SetChannel(GetNamedInt(parts, "chnl"s));

         m_characters[character] = pCharacter;
      }
      else if (sec == "kerning") {
         Kerning* const pKerning = new Kerning();
         pKerning->SetFirstCharacter((char)GetNamedInt(parts, "first"s));
         pKerning->SetSecondCharacter((char)GetNamedInt(parts, "second"s));
         pKerning->SetAmount(GetNamedInt(parts, "amount"s));

         const int h = pKerning->GetHash();
         if (!m_kernings.contains(h))
            m_kernings[h] = pKerning;
      }
   }

   std::ranges::sort(m_pages.begin(), m_pages.end(), [](const Page* a, const Page* b) {
     return a->GetId() < b->GetId();
   });
}

SDL_Rect BitmapFont::MeasureFont(const string& text, double maxWidth)
{
   if (text.empty())
      return { 0, 0, 0, 0 };

   char previousCharacter = ' ';
   int currentLineWidth = 0;
   int currentLineHeight = m_lineHeight;
   int blockWidth = 0;
   int blockHeight = 0;
   size_t length = text.length();
   vector<int> lineHeights;

   for (size_t i = 0; i < length; i++)
   {
      const char* const chars = text.c_str();

      const char character = chars[i];

      if (character == '\n' || character == '\r') {
         if (character == '\n' || i + 1 == length || chars[i + 1] != '\n') {
            lineHeights.push_back(currentLineHeight);
            blockWidth = max(blockWidth, currentLineWidth);
            currentLineWidth = 0;
            currentLineHeight = m_lineHeight;
         }
      }
      else {
         const auto it = m_characters.find(character);
         if (it != m_characters.end()) {
            Character* data = it->second;
            int width = data->GetXAdvance() + GetKerning(previousCharacter, character);
            if (maxWidth != NoMaxWidth && currentLineWidth + width >= maxWidth) {
               lineHeights.push_back(currentLineHeight);
               blockWidth = max(blockWidth, currentLineWidth);
               currentLineWidth = 0;
               currentLineHeight = m_lineHeight;
            }

            currentLineWidth += width;
            currentLineHeight = max(currentLineHeight, data->GetBounds().h + data->GetOffset().y);
         }
         previousCharacter = character;
      }
   }

   // finish off the current line if required
   if (currentLineHeight != 0)
      lineHeights.push_back(currentLineHeight);

   // reduce any lines other than the last back to the base
   for (int i = 0; i < (int)lineHeights.size() - 1; i++)
      lineHeights[i] = m_lineHeight;

   // calculate the final block height
   for (int lineHeight : lineHeights)
     blockHeight += lineHeight;

   return { 0, 0, max(currentLineWidth, blockWidth), blockHeight };
}

string BitmapFont::GetNamedString(const ankerl::unordered_dense::map<string, string>& parts, const string& name)
{
   const auto it = parts.find(name);
   return (it != parts.end()) ? it->second : string();
}

int BitmapFont::GetNamedInt(const ankerl::unordered_dense::map<string, string>& parts, const string& name, int defaultValue)
{
   return string_to_int(GetNamedString(parts, name), defaultValue);
}

bool BitmapFont::GetNamedBool(const ankerl::unordered_dense::map<string, string>& parts, const string& name, bool defaultValue)
{
   return string_to_int(GetNamedString(parts, name), defaultValue ? 1 : 0) > 0;
}

SDL_Rect BitmapFont::ParsePadding(const string& s)
{
   vector<int> parts;
   std::stringstream ss(s);
   string token;
   while (std::getline(ss, token, ','))
      parts.push_back(string_to_int(token));

   return { parts[3], parts[0], parts[1], parts[2] };
}

SDL_Point BitmapFont::ParsePoint(const string& s)
{
   vector<int> parts;
   std::stringstream ss(s);
   string token;
   while (std::getline(ss, token, ','))
      parts.push_back(string_to_int(token));

   return { parts[0], parts[1] };
}

ankerl::unordered_dense::map<string, string> BitmapFont::ParseParts(const string& line)
{
   ankerl::unordered_dense::map<string, string> result;
   std::istringstream iss(line);

   string token;
   string key;
   string value;
   bool isKey = true;
   bool isQuote = false;

   iss >> token;
   result["section"s] = token;

   while (iss) {
      char c;
      iss.get(c);

      if (c == '=')
         isKey = false;
      else if (c == ' ' && !isQuote) {
         if (!key.empty())
            result[key] = value;

         key.clear();
         value.clear();
         isKey = true;
      }
      else if (c == '"')
         isQuote = !isQuote;
      else {
         if (isKey)
            key += c;
         else
            value += c;
      }
   }

   if (!key.empty())
      result[key] = value;

   return result;
}
