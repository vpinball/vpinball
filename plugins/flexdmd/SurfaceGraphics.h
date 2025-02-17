#pragma once

#include "common.h"
#include <SDL3/SDL_surface.h>

namespace VP {

class SurfaceGraphics final {
public:
   SurfaceGraphics(SDL_Surface* pSurface);
   ~SurfaceGraphics();

   void Clear();
   void SetColor(ColorRGBA32 color, uint8_t alpha = 255);
   void DrawImage(SDL_Surface* pImage, SDL_Rect* pSrcRect, SDL_Rect* pDstRect);
   void FillRectangle(const SDL_Rect& rect);
   void SetClip(const SDL_Rect& rect);
   void ResetClip();
   void TranslateTransform(int x, int y);
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }
   SDL_Surface* GetSurface() const { return m_pSurface; }

private:
   SDL_Surface* m_pSurface;

   int m_width;
   int m_height;
   int m_translateX;
   int m_translateY;
   ColorRGBA32 m_color;
   uint8_t m_alpha;
};

}
