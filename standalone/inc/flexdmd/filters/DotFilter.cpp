#include "core/stdafx.h"
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
   SDL_Surface* const src = pBitmap->GetSurface();

   if (!src)
      return;

   const SDL_PixelFormatDetails* const pfd = SDL_GetPixelFormatDetails(src->format);
   SDL_Palette* const pal = SDL_GetSurfacePalette(src);

   SDL_Surface* const dst = SDL_CreateSurface(src->w / m_dotSize, src->h / m_dotSize, SDL_PIXELFORMAT_RGBA32);

   const int w = dst_w;
   const int h = dst_h;

   UINT32* const __restrict pixels = (Uint32*)dst->pixels;

   for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
         int r = 0;
         int g = 0;
         int b = 0;
         int a = 0;

         for (int i = 0; i < m_dotSize; i++) {
            for (int j = 0; j < m_dotSize; j++) {
               UINT32 pixel = GetPixel(src, x * m_dotSize + i, y * m_dotSize + j);
               UINT8 cr,cg,cb,ca;
               SDL_GetRGBA(pixel, pfd, pal, &cr, &cg, &cb, &ca);
               r += cr;
               g += cg;
               b += cb;
               a += ca;
            }
         }
         const float bright = 1.0f + (float)(m_dotSize * m_dotSize) / 1.8f;
         pixels[y * w + x] = SDL_MapSurfaceRGBA(dst, 
            (UINT8)min(((float)r / bright), 255.f),
            (UINT8)min(((float)g / bright), 255.f),
            (UINT8)min(((float)b / bright), 255.f),
            (UINT8)min(((float)a / bright), 255.f));
      }
   }

   pBitmap->SetData(dst);
}
