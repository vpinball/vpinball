// Scaled DrawImage derived from Wine's GDI+ (LGPL-2.1+), to match the default GDI+ rendering used by the original FlexDMD:
// - blend_colors, resample_bitmap_pixel and GdipDrawImagePointsRect in https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/gdiplus/graphics.c

#include "SurfaceGraphics.h"

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

static uint32_t BlendColors(uint32_t start, uint32_t end, uint32_t pos)
{
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

   // FlexDMD renders to an RGB24 surface: the scaled pixels are blended directly into it, clipped like SDL_BlitSurface would
   if (m_pSurface->format != SDL_PIXELFORMAT_RGB24)
   {
      assert(false);
      SDL_BlitSurfaceScaled(pImage, pSrcRect, m_pSurface, &rect, SDL_SCALEMODE_NEAREST);
      return;
   }
   SDL_Rect clip, visible;
   SDL_GetSurfaceClipRect(m_pSurface, &clip);
   if (!SDL_GetRectIntersection(&rect, &clip, &visible))
      return;

   SDL_Surface* const pSrc = (pImage->format == SDL_PIXELFORMAT_ABGR8888) ? pImage : SDL_ConvertSurface(pImage, SDL_PIXELFORMAT_ABGR8888);
   if (pSrc == nullptr)
      return;
   if (!SDL_LockSurface(pSrc))
   {
      if (pSrc != pImage)
         SDL_DestroySurface(pSrc);
      return;
   }
   if (!SDL_LockSurface(m_pSurface))
   {
      SDL_UnlockSurface(pSrc);
      if (pSrc != pImage)
         SDL_DestroySurface(pSrc);
      return;
   }

   // Same rounding as SDL's ALPHA_BLEND_CHANNEL, so the result matches SDL_BlitSurface with SDL_BLENDMODE_BLEND
   const auto blendChannel = [](uint32_t s, uint8_t& d, uint32_t a)
   {
      uint16_t x = static_cast<uint16_t>(((static_cast<int>(s) - d) * static_cast<int>(a)) + ((d << 8) - d));
      x += 1;
      x += x >> 8;
      d = static_cast<uint8_t>(x >> 8);
   };
   // One channel of the bilinear filter when the 4 samples share the same alpha: BlendColors then reduces exactly to this integer lerp
   const auto lerpChannel = [](uint32_t tl, uint32_t tr, uint32_t bl, uint32_t br, uint32_t shift, uint32_t posX, uint32_t posY)
   {
      const uint32_t t = (((tl >> shift) & 0xFFu) * (posX ^ 0xFFu) + ((tr >> shift) & 0xFFu) * posX) / 0xFFu;
      const uint32_t b = (((bl >> shift) & 0xFFu) * (posX ^ 0xFFu) + ((br >> shift) & 0xFFu) * posX) / 0xFFu;
      return (t * (posY ^ 0xFFu) + b * posY) / 0xFFu;
   };
   // Destination pixel x samples source position x * src.w / rect.w (GDI+ default pixel offset mode), stepped in 16.16 fixed point
   // like SDL_stretch.c, with the fraction rounded to an 8 bit weight. Samples past the right or bottom source edge are transparent.
   const auto blendPos = [](int64_t fp) { return static_cast<uint32_t>(((fp & 0xFFFF) * 255 + 0x8000) >> 16); };
   const int64_t stepX = (static_cast<int64_t>(src.w) << 16) / rect.w;
   const int64_t stepY = (static_cast<int64_t>(src.h) << 16) / rect.h;
   const int srcStride = pSrc->pitch / 4;
   const uint32_t* const __restrict pSrcPixels = static_cast<const uint32_t*>(pSrc->pixels) + src.y * srcStride + src.x;
   const int xStart = visible.x - rect.x;
   for (int y = visible.y - rect.y; y < visible.y + visible.h - rect.y; y++)
   {
      const int64_t fpY = y * stepY;
      const int top = static_cast<int>(fpY >> 16);
      const uint32_t posY = blendPos(fpY);
      const uint32_t* const __restrict pTop = pSrcPixels + top * srcStride;
      const uint32_t* const __restrict pBottom = (top + 1 < src.h) ? pTop + srcStride : nullptr;
      uint8_t* __restrict pDstPixel = static_cast<uint8_t*>(m_pSurface->pixels) + (rect.y + y) * m_pSurface->pitch + visible.x * 3;
      int64_t fpX = xStart * stepX;
      for (int x = 0; x < visible.w; x++, pDstPixel += 3, fpX += stepX)
      {
         const int left = static_cast<int>(fpX >> 16);
         const uint32_t posX = blendPos(fpX);
         const uint32_t tl = pTop[left];
         uint32_t color;
         if (posX == 0 && posY == 0)
            color = tl;
         else
         {
            const bool hasRight = left + 1 < src.w;
            const uint32_t tr = hasRight ? pTop[left + 1] : 0u;
            const uint32_t bl = pBottom ? pBottom[left] : 0u;
            const uint32_t br = (pBottom && hasRight) ? pBottom[left + 1] : 0u;
            const uint32_t alpha = tl >> 24;
            if (alpha == (tr >> 24) && alpha == (bl >> 24) && alpha == (br >> 24))
            {
               // Opaque images, inside of sprites
               if (alpha == 0)
                  continue;
               color = (alpha << 24) | (lerpChannel(tl, tr, bl, br, 16, posX, posY) << 16) | (lerpChannel(tl, tr, bl, br, 8, posX, posY) << 8) | lerpChannel(tl, tr, bl, br, 0, posX, posY);
            }
            else
               color = BlendColors(BlendColors(tl, tr, posX), BlendColors(bl, br, posX), posY);
         }
         const uint32_t finalAlpha = color >> 24;
         if (finalAlpha == 0)
            continue;
         // ABGR8888 source (R in the low byte), RGB24 destination (R first). An opaque pixel gives the same result as the blend.
         if (finalAlpha == 0xFFu)
         {
            pDstPixel[0] = static_cast<uint8_t>(color);
            pDstPixel[1] = static_cast<uint8_t>(color >> 8);
            pDstPixel[2] = static_cast<uint8_t>(color >> 16);
            continue;
         }
         blendChannel(color & 0xFFu, pDstPixel[0], finalAlpha);
         blendChannel((color >> 8) & 0xFFu, pDstPixel[1], finalAlpha);
         blendChannel((color >> 16) & 0xFFu, pDstPixel[2], finalAlpha);
      }
   }

   SDL_UnlockSurface(m_pSurface);
   SDL_UnlockSurface(pSrc);
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
