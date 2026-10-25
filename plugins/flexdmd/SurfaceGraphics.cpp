// Scaled DrawImage derived from Wine's GDI+ (LGPL-2.1+), to match the default GDI+ rendering used by the original FlexDMD:
// - blend_colors, resample_bitmap_pixel and GdipDrawImagePointsRect in https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/gdiplus/graphics.c

#include "SurfaceGraphics.h"

#include <cmath>

namespace Flex {

SurfaceGraphics::SurfaceGraphics(SDL_Surface* pSurface)
{
   m_pSurface = pSurface;

   m_width = pSurface->w;
   m_height = pSurface->h;
   m_color = RGB(0, 0, 0);
   m_alpha = 255;

   m_translateX = 0;
   m_translateY = 0;
}

SurfaceGraphics::~SurfaceGraphics()
{
}

void SurfaceGraphics::SetColor(ColorRGBA32 color, uint8_t alpha)
{
   m_color = color;
   m_alpha = alpha;
}

void SurfaceGraphics::Clear()
{
   FillRectangle({ 0, 0, m_pSurface->w, m_pSurface->h });
}

static uint32_t BlendColors(uint32_t start, uint32_t end, float position)
{
   const uint32_t pos = static_cast<uint32_t>(floorf(position * 255.f + 0.5f));
   const uint32_t startA = (start >> 24) * (pos ^ 0xFFu);
   const uint32_t endA = (end >> 24) * pos;
   const uint32_t finalA = startA + endA;
   if (finalA < 0xFFu)
      return 0;
   return ((finalA / 0xFFu) << 24)
      | (((((start >> 16) & 0xFFu) * startA + ((end >> 16) & 0xFFu) * endA) / finalA) << 16)
      | (((((start >> 8) & 0xFFu) * startA + ((end >> 8) & 0xFFu) * endA) / finalA) << 8)
      | (((start & 0xFFu) * startA + (end & 0xFFu) * endA) / finalA);
}

void SurfaceGraphics::DrawImage(SDL_Surface* pImage, SDL_Rect* pSrcRect, SDL_Rect* pDstRect)
{
   SDL_Rect rect = { pDstRect->x + m_translateX, pDstRect->y + m_translateY, pDstRect->w, pDstRect->h };
   const SDL_Rect src = pSrcRect ? *pSrcRect : SDL_Rect { 0, 0, pImage->w, pImage->h };
   if ((src.w == rect.w && src.h == rect.h) || rect.w <= 0 || rect.h <= 0 || src.w <= 0 || src.h <= 0)
   {
      SDL_BlitSurfaceScaled(pImage, pSrcRect, m_pSurface, &rect, SDL_SCALEMODE_NEAREST);
      return;
   }

   SDL_Surface* const pSrc = (pImage->format == SDL_PIXELFORMAT_ABGR8888) ? pImage : SDL_ConvertSurface(pImage, SDL_PIXELFORMAT_ABGR8888);
   if (pSrc == nullptr)
      return;
   SDL_Surface* const pDst = SDL_CreateSurface(rect.w, rect.h, SDL_PIXELFORMAT_ABGR8888);
   if (pDst == nullptr || !SDL_LockSurface(pSrc))
   {
      SDL_DestroySurface(pDst);
      if (pSrc != pImage)
         SDL_DestroySurface(pSrc);
      return;
   }

   const auto sample = [pSrc, &src](int x, int y) -> uint32_t
   {
      if (x < src.x || y < src.y || x >= src.x + src.w || y >= src.y + src.h)
         return 0;
      return static_cast<const uint32_t*>(pSrc->pixels)[y * (pSrc->pitch / 4) + x];
   };
   const float scaleX = static_cast<float>(src.w) / static_cast<float>(rect.w);
   const float scaleY = static_cast<float>(src.h) / static_cast<float>(rect.h);
   for (int y = 0; y < rect.h; y++)
   {
      uint32_t* const pDstRow = static_cast<uint32_t*>(pDst->pixels) + y * (pDst->pitch / 4);
      const float sy = static_cast<float>(src.y) + static_cast<float>(y) * scaleY;
      const float topF = floorf(sy);
      const int top = static_cast<int>(topF);
      const int bottom = static_cast<int>(ceilf(sy));
      for (int x = 0; x < rect.w; x++)
      {
         const float sx = static_cast<float>(src.x) + static_cast<float>(x) * scaleX;
         const float leftF = floorf(sx);
         const int left = static_cast<int>(leftF);
         const int right = static_cast<int>(ceilf(sx));
         if (left == right && top == bottom)
            pDstRow[x] = sample(left, top);
         else
         {
            const uint32_t t = BlendColors(sample(left, top), sample(right, top), sx - leftF);
            const uint32_t b = BlendColors(sample(left, bottom), sample(right, bottom), sx - leftF);
            pDstRow[x] = BlendColors(t, b, sy - topF);
         }
      }
   }

   SDL_UnlockSurface(pSrc);
   SDL_SetSurfaceBlendMode(pDst, SDL_BLENDMODE_BLEND);
   SDL_BlitSurface(pDst, nullptr, m_pSurface, &rect);
   SDL_DestroySurface(pDst);
   if (pSrc != pImage)
      SDL_DestroySurface(pSrc);
}

void SurfaceGraphics::FillRectangle(const SDL_Rect& rect)
{
   SDL_Rect dstRect = { rect.x + m_translateX, rect.y + m_translateY, rect.w, rect.h };

   if (m_alpha == 255)
      SDL_FillSurfaceRect(m_pSurface, &dstRect, SDL_MapSurfaceRGB(m_pSurface, GetRValue(m_color), GetGValue(m_color), GetBValue(m_color)));
   else if (dstRect.w > 0 && dstRect.h > 0) {
      SDL_Surface* pSource = SDL_CreateSurface(dstRect.w, dstRect.h, SDL_PIXELFORMAT_RGBA32);
      if (pSource) {
         SDL_FillSurfaceRect(pSource, nullptr, SDL_MapSurfaceRGBA(pSource, GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), m_alpha));
         SDL_BlitSurface(pSource, nullptr, m_pSurface, &dstRect);
         SDL_DestroySurface(pSource);
      }
   }
}

void SurfaceGraphics::SetClip(const SDL_Rect& rect)
{
   SDL_Rect dstRect = { m_translateX + rect.x, m_translateY + rect.y, rect.w, rect.h };
   SDL_SetSurfaceClipRect(m_pSurface, &dstRect);
}

void SurfaceGraphics::ResetClip()
{
   SDL_SetSurfaceClipRect(m_pSurface, nullptr);
}

void SurfaceGraphics::TranslateTransform(int x, int y)
{
   m_translateX += x;
   m_translateY += y;
}

}
