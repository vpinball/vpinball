#include "DotFilter.h"

void DotFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* const src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* const dst = SDL_CreateSurface(src->w / m_dotSize, src->h / m_dotSize, SDL_PIXELFORMAT_RGBA32);

   ColorRGBA32* const pixels = (Uint32*)dst->pixels;

   for (int y = 0; y < dst->h; y++) {
      for (int x = 0; x < dst->w; x++) {
         int r = 0;
         int g = 0;
         int b = 0;
         int a = 0;

         for (int i = 0; i < m_dotSize; i++) {
            for (int j = 0; j < m_dotSize; j++) {
               ColorRGBA32 pixel = GetPixel(src, x * m_dotSize + i, y * m_dotSize + j);
               uint8_t cr,cg,cb,ca;
               SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(src->format), SDL_GetSurfacePalette(src), &cr, &cg, &cb, &ca);
               r += cr;
               g += cg;
               b += cb;
               a += ca;
            }
         }
         float bright = 1.0f + m_dotSize * m_dotSize / 1.8f;
         pixels[(y * dst->w) + x] = SDL_MapSurfaceRGBA(dst, 
            (uint8_t)SDL_min((r / bright), 255),
            (uint8_t)SDL_min((g / bright), 255),
            (uint8_t)SDL_min((b / bright), 255),
            (uint8_t)SDL_min((a / bright), 255));
      }
   }

   pBitmap->SetData(dst);
}