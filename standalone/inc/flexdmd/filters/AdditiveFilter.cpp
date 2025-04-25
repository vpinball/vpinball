#include "core/stdafx.h"
#include "AdditiveFilter.h"

AdditiveFilter::AdditiveFilter()
{
}

AdditiveFilter::~AdditiveFilter()
{
}

void AdditiveFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* const src = pBitmap->GetSurface();

   if (!src)
      return;

   const SDL_PixelFormatDetails* const pfd = SDL_GetPixelFormatDetails(src->format);
   SDL_Palette* const pal = SDL_GetSurfacePalette(src);

   SDL_Surface* const dst = SDL_CreateSurface(src->w, src->h, SDL_PIXELFORMAT_RGBA32);

   const int w = dst->w;
   const int h = dst->h;

   UINT32* const __restrict pixels = (UINT32*)dst->pixels;

   for (int y = 0; y < h; y++) {
      for (int x = 0; x < w; x++) {
         UINT32 pixel = GetPixel(src, x, y);
         UINT8 cr,cg,cb,ca;
         SDL_GetRGBA(pixel, pfd, pal, &cr, &cg, &cb, &ca);
         if (cr < 64 && cg < 64 && cb < 64)
            pixels[y * w + x] = SDL_MapSurfaceRGBA(src, cr, cg, cb, 0);
         else
            pixels[y * w + x] = SDL_MapSurfaceRGBA(src, cr, cg, cb, ca);
      }
   }

   pBitmap->SetData(dst);
}
