// license:GPLv3+

#pragma once

#include <filesystem>

#include <SDL3_ttf/SDL_ttf.h>

namespace PUP {

class PUPFont final
{
public:
   PUPFont(TTF_Font* ttfFont, const std::filesystem::path& path);
   ~PUPFont();

   PUPFont(const PUPFont&) = delete;
   PUPFont& operator=(const PUPFont&) = delete;

   TTF_Font* GetTTFFont() const { return m_pTTFFont; }
   float GetWinScale() const { return m_winScale; }
   float GetWinAscentRatio() const { return m_winAscentRatio; }

   struct SizeMetrics
   {
      float ptsize = 0.f;    // value to pass to TTF_SetFontSize
      float sdlAscent = 0.f; // SDL_ttf pixel ascent at this size
      float winAscent = 0.f; // Windows-equivalent pixel ascent (fontHeight × winAscentRatio)
      float lineGap = 0.f;   // SDL_ttf surface padding below glyphs (lineSkip - height) at this size
   };

   SizeMetrics Compute(float fontHeightPx) const;
   SizeMetrics Apply(float fontHeightPx);

private:
   TTF_Font* m_pTTFFont = nullptr;
   float m_winScale = 1.0f;
   float m_winAscentRatio = 0.8f;
   float m_sdlAscentRatio = 0.8f;
   float m_lineGapRatio = 0.0f;
};

}
