// license:GPLv3+

#include "PUPFont.h"

#include <fstream>
#include <vector>

namespace PUP {

namespace {

uint16_t ReadU16BE(const uint8_t* p) { return (uint16_t(p[0]) << 8) | p[1]; }
uint32_t ReadU32BE(const uint8_t* p) { return (uint32_t(p[0]) << 24) | (uint32_t(p[1]) << 16) | (uint32_t(p[2]) << 8) | p[3]; }

bool ParseOS2Metrics(const std::filesystem::path& path, float& winScale, float& winAscentRatio)
{
   std::ifstream f(path, std::ios::binary);
   if (!f)
      return false;
   uint8_t header[12];
   if (!f.read(reinterpret_cast<char*>(header), sizeof(header)))
      return false;
   const uint16_t numTables = ReadU16BE(header + 4);
   if (numTables == 0 || numTables > 64)
      return false;
   std::vector<uint8_t> dir(static_cast<size_t>(numTables) * 16);
   if (!f.read(reinterpret_cast<char*>(dir.data()), dir.size()))
      return false;
   uint32_t headOffset = 0, os2Offset = 0;
   uint32_t headLength = 0, os2Length = 0;
   for (size_t i = 0; i < numTables; ++i)
   {
      const uint8_t* entry = dir.data() + i * 16;
      const uint32_t tag = ReadU32BE(entry);
      const uint32_t offset = ReadU32BE(entry + 8);
      const uint32_t length = ReadU32BE(entry + 12);
      if (tag == 0x68656164u) { headOffset = offset; headLength = length; } // 'head'
      else if (tag == 0x4F532F32u) { os2Offset = offset; os2Length = length; } // 'OS/2'
   }
   if (headOffset == 0 || os2Offset == 0 || headLength < 54 || os2Length < 78)
      return false;

   f.seekg(headOffset + 18);
   uint8_t upem[2];
   if (!f.read(reinterpret_cast<char*>(upem), 2))
      return false;
   const uint16_t unitsPerEm = ReadU16BE(upem);

   f.seekg(os2Offset + 74);
   uint8_t winMetrics[4];
   if (!f.read(reinterpret_cast<char*>(winMetrics), 4))
      return false;
   const uint16_t winAscent = ReadU16BE(winMetrics);
   const uint16_t winDescent = ReadU16BE(winMetrics + 2);

   if (unitsPerEm == 0 || (winAscent + winDescent) == 0)
      return false;
   winScale = static_cast<float>(unitsPerEm) / static_cast<float>(winAscent + winDescent);
   winAscentRatio = static_cast<float>(winAscent) / static_cast<float>(winAscent + winDescent);
   return true;
}

constexpr float kProbeSize = 100.0f;

}

PUPFont::PUPFont(TTF_Font* ttfFont, const std::filesystem::path& path)
   : m_pTTFFont(ttfFont)
{
   ParseOS2Metrics(path, m_winScale, m_winAscentRatio);

   if (m_pTTFFont)
   {
      TTF_SetFontSize(m_pTTFFont, kProbeSize);
      const int ascent = TTF_GetFontAscent(m_pTTFFont);
      const int height = TTF_GetFontHeight(m_pTTFFont);
      const int lineSkip = TTF_GetFontLineSkip(m_pTTFFont);
      if (ascent > 0)
         m_sdlAscentRatio = static_cast<float>(ascent) / kProbeSize;
      if (lineSkip > height)
         m_lineGapRatio = static_cast<float>(lineSkip - height) / kProbeSize;
   }
}

PUPFont::~PUPFont()
{
   if (m_pTTFFont)
      TTF_CloseFont(m_pTTFFont);
}

PUPFont::SizeMetrics PUPFont::Compute(float fontHeightPx) const
{
   SizeMetrics m;
   m.ptsize = fontHeightPx * m_winScale;
   m.sdlAscent = m.ptsize * m_sdlAscentRatio;
   m.winAscent = fontHeightPx * m_winAscentRatio;
   m.lineGap = m.ptsize * m_lineGapRatio;
   return m;
}

PUPFont::SizeMetrics PUPFont::Apply(float fontHeightPx)
{
   const SizeMetrics m = Compute(fontHeightPx);
   if (m_pTTFFont)
      TTF_SetFontSize(m_pTTFFont, m.ptsize);
   return m;
}

}
