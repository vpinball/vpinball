// license:GPLv3+

#include "core/stdafx.h"
#include "PinFontRegistrar.h"

#include "parts/pinbinary.h"

void PinFontRegistrar::Register(PinFont* font)
{
   if (font == nullptr || m_registered.contains(font))
      return;

   char tempPath[MAX_PATH];
   char tempFileName[MAX_PATH];
   GetTempPathA(MAX_PATH, tempPath);
   const string tempFile = (GetTempFileNameA(tempPath, "VP", 0, tempFileName) != 0) ? tempFileName : "VPTemp0.ttf"s;

   font->WriteToFile(tempFile);
   AddFontResource(tempFile.c_str());
   m_registered[font] = tempFile;
}

void PinFontRegistrar::Unregister(PinFont* font)
{
   const auto it = m_registered.find(font);
   if (it == m_registered.end())
      return;
   RemoveFontResource(it->second.c_str());
   DeleteFile(it->second.c_str());
   m_registered.erase(it);
}

void PinFontRegistrar::UnregisterAll()
{
   for (const auto& [font, tempFile] : m_registered)
   {
      RemoveFontResource(tempFile.c_str());
      DeleteFile(tempFile.c_str());
   }
   m_registered.clear();
}
