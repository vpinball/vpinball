#include "stdafx.h"
#include "DotFilter.h"

DotFilter::DotFilter()
{
   m_dotSize = 2;
   m_offset = 0;
}

DotFilter::~DotFilter()
{
}

void DotFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, src->w / m_dotSize, src->h / m_dotSize, 32, SDL_PIXELFORMAT_RGBA32);

   UINT8 cr;
   UINT8 cg;
   UINT8 cb;
   UINT8 ca;
   UINT32 pixel;
   UINT32* pixels = (Uint32*)dst->pixels;

   for (int y = 0; y < dst->h; y++) {
      for (int x = 0; x < dst->w; x++) {
         int r = 0;
         int g = 0;
         int b = 0;
         int a = 0;

         for (int i = 0; i < m_dotSize; i++) {
            for (int j = 0; j < m_dotSize; j++) {
               pixel = GetPixel(src, x * m_dotSize + i, y * m_dotSize + j);
               SDL_GetRGBA(pixel, src->format, &cr, &cg, &cb, &ca);
               r += cr;
               g += cg;
               b += cb;
               a += ca;
            }
         }
         float bright = 1.0f + m_dotSize * m_dotSize / 1.8f;
         pixels[(y * dst->w) + x] = SDL_MapRGBA(dst->format, 
            (UINT8)SDL_min((r / bright), 255),
            (UINT8)SDL_min((g / bright), 255),
            (UINT8)SDL_min((b / bright), 255),
            (UINT8)SDL_min((a / bright), 255));
      }
   }

   pBitmap->SetData(dst);
}