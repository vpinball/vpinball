#include "AdditiveFilter.h"

void AdditiveFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* const src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* const dst = SDL_CreateSurface(src->w, src->h, SDL_PIXELFORMAT_RGBA32);

   ColorRGBA32* const pixels = (ColorRGBA32*)dst->pixels;

   for (int y = 0; y < dst->h; y++) {
      for (int x = 0; x < dst->w; x++) {
         ColorRGBA32 pixel = GetPixel(src, x, y);
         uint8_t cr,cg,cb,ca;
         SDL_GetRGBA(pixel, SDL_GetPixelFormatDetails(src->format), SDL_GetSurfacePalette(src), &cr, &cg, &cb, &ca);
         if (cr < 64 && cg < 64 && cb < 64)
            pixels[(y * dst->w) + x] = SDL_MapSurfaceRGBA(src, cr, cg, cb, 0);
         else
            pixels[(y * dst->w) + x] = SDL_MapSurfaceRGBA(src, cr, cg, cb, ca);
      }
   }

   pBitmap->SetData(dst);
}
