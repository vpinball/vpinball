#include "AdditiveFilter.h"

void AdditiveFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* dst = SDL_CreateSurface(src->w, src->h, SDL_PIXELFORMAT_RGBA32);

   uint8_t cr;
   uint8_t cg;
   uint8_t cb;
   uint8_t ca;
   ColorRGBA32 pixel;
   ColorRGBA32* pixels = (ColorRGBA32*)dst->pixels;

   for (int y = 0; y < dst->h; y++) {
      for (int x = 0; x < dst->w; x++) {
         pixel = GetPixel(src, x, y);
         SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(src->format), SDL_GetSurfacePalette(src), &cr, &cg, &cb, &ca);
         if (cr < 64 && cg < 64 && cb < 64)
            pixels[(y * dst->w) + x] = SDL_MapSurfaceRGBA(src, cr, cg, cb, 0);
         else
            pixels[(y * dst->w) + x] = SDL_MapSurfaceRGBA(src, cr, cg, cb, ca);
      }
   }

   pBitmap->SetData(dst);
}