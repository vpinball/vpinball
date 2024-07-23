#include "stdafx.h"
#include "BitmapFont.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>

BitmapFont::BitmapFont()
{
   m_padding = { 0, 0, 0, 0 };
   m_spacing = { 0, 0 };
   m_textureSize = { 0, 0, 0, 0 };
}

BitmapFont::~BitmapFont()
{
   std::unordered_set<Character*> characters;
   for(const auto& pair : m_characters)
      characters.insert(pair.second);

   for(Character* character : characters)
      delete character;

   m_characters.clear();

   for (auto it = m_kernings.begin(); it != m_kernings.end(); ++it)
      delete it->second;

   m_kernings.clear();

   for (auto it = m_pages.begin(); it != m_pages.end(); ++it)
      delete *it;

   m_pages.clear();
}

BitmapFont* BitmapFont::Create(string fileName)
{
   BitmapFont* obj = new BitmapFont();
   obj->Load(fileName);

   return obj;
}

int BitmapFont::GetKerning(char previous, char current)
{
    int result = 0;

    Kerning key;
    key.SetFirstCharacter(previous);
    key.SetSecondCharacter(current);

    auto it = m_kernings.find(key.GetHash());
    if (it != m_kernings.end())
       result = it->second->GetAmount();

    return result;
}

void BitmapFont::Load(string fileName)
{
   std::ifstream fontFile;
   fontFile.open(fileName, std::ifstream::in);

   if (!fontFile.is_open()) {
      PLOGE.printf("Failed to open bitmap font file: %s", fileName.c_str());
      return;
   }

   string line;
   while (std::getline(fontFile, line)) {
      std::map<string, string> parts = ParseParts(trim_string(line));

      if (parts.size() == 0)
         continue;

      if (parts["section"] == "info") {
         m_szFamilyName = GetNamedString(parts, "face");
         m_fontSize = GetNamedInt(parts, "size");
         m_bold = GetNamedBool(parts, "bold");
         m_italic = GetNamedBool(parts, "italic");
         m_szCharset = GetNamedString(parts, "charset");
         m_unicode = GetNamedBool(parts, "unicode");
         m_stretchedHeight = GetNamedInt(parts, "stretchH");
         m_smoothed = GetNamedBool(parts, "smooth");
         m_superSampling = GetNamedInt(parts, "aa");
         m_padding = ParsePadding(GetNamedString(parts, "padding"));
         m_spacing = ParsePoint(GetNamedString(parts, "spacing"));
         m_outlineSize = GetNamedInt(parts, "outline");
      }
      else if (parts["section"] == "common") {
         m_lineHeight = GetNamedInt(parts, "lineHeight");
         m_baseHeight = GetNamedInt(parts, "base");
         m_textureSize = { 0, 0, GetNamedInt(parts, "scaleW"), GetNamedInt(parts, "scaleH") };
         m_packed = GetNamedBool(parts, "packed");
         m_alphaChannel = GetNamedInt(parts, "alphaChnl");
         m_redChannel = GetNamedInt(parts, "redChnl");
         m_greenChannel = GetNamedInt(parts, "greenChnl");
         m_blueChannel = GetNamedInt(parts, "blueChnl");
      }
      else if (parts["section"] == "page") {
         Page* pPage = new Page();
         pPage->SetId(GetNamedInt(parts, "id"));
         pPage->SetFilename(GetNamedString(parts, "file"));

         m_pages.push_back(pPage);
      }
      else if (parts["section"] == "char") {
         char character = (char)GetNamedInt(parts, "id");

         Character* pCharacter = new Character();
         pCharacter->SetChar(character);
         pCharacter->SetBounds({ GetNamedInt(parts, "x"), GetNamedInt(parts, "y"), GetNamedInt(parts, "width"), GetNamedInt(parts, "height") });
         pCharacter->SetOffset({ GetNamedInt(parts, "xoffset"), GetNamedInt(parts, "yoffset") });
         pCharacter->SetXAdvance(GetNamedInt(parts, "xadvance"));
         pCharacter->SetTexturePage(GetNamedInt(parts, "page"));
         pCharacter->SetChannel(GetNamedInt(parts, "chnl"));

         m_characters[character] = pCharacter;
      }
      else if (parts["section"] == "kerning") {
         Kerning* pKerning = new Kerning();
         pKerning->SetFirstCharacter((char)GetNamedInt(parts, "first"));
         pKerning->SetSecondCharacter((char)GetNamedInt(parts, "second"));
         pKerning->SetAmount(GetNamedInt(parts, "amount"));

         auto it = m_kernings.find(pKerning->GetHash());
         if (it == m_kernings.end())
            m_kernings[pKerning->GetHash()] = pKerning;
      }
   }

   std::sort(m_pages.begin(), m_pages.end(), [](const Page* a, const Page* b) {
     return a->GetId() < b->GetId();
   });
}

SDL_Rect BitmapFont::MeasureFont(const string& text, double maxWidth)
{
   if (text.length() == 0)
      return { 0, 0, 0, 0 };

   char previousCharacter = ' ';
   int currentLineWidth = 0;
   int currentLineHeight = m_lineHeight;
   int blockWidth = 0;
   int blockHeight = 0;
   int length = text.length();
   vector<int> lineHeights;
   int i;

   for (i = 0; i < length; i++) {
      const char* chars = text.c_str();

      char character = chars[i];

      if (character == '\n' || character == '\r') {
         if (character == '\n' || i + 1 == length || chars[i + 1] != '\n') {
            lineHeights.push_back(currentLineHeight);
            blockWidth = max(blockWidth, currentLineWidth);
            currentLineWidth = 0;
            currentLineHeight = m_lineHeight;
         }
      }
      else {
         auto it = m_characters.find(character);
         if (it != m_characters.end()) {
            Character* data = m_characters[character];
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

   if (currentLineHeight != 0)
      lineHeights.push_back(currentLineHeight);

   for (i = 0; i < (int)lineHeights.size() - 1; i++)
      lineHeights[i] = m_lineHeight;

   for (int lineHeight : lineHeights)
     blockHeight += lineHeight;

   return { 0, 0, max(currentLineWidth, blockWidth), blockHeight };
}

string BitmapFont::GetNamedString(const std::map<string, string>& parts, const string& name)
{
   auto it = parts.find(name);
   return (it != parts.end()) ? it->second : "";
}

int BitmapFont::GetNamedInt(const std::map<string, string>& parts, const string& name, int defaultValue)
{
   return string_to_int(GetNamedString(parts, name), defaultValue);
}

bool BitmapFont::GetNamedBool(const std::map<string, string>& parts, const string& name, bool defaultValue)
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

std::map<string, string> BitmapFont::ParseParts(const string &line)
{
   std::map<std::string, std::string> result;
   std::istringstream iss(line);

   string token;
   string key;
   string value;
   bool isKey = true;
   bool isQuote = false;

   iss >> token;
   result["section"] = token;

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
