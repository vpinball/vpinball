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
   SDL_Surface* src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* dst = SDL_CreateSurface(src->w, src->h, SDL_PIXELFORMAT_RGBA32);

   UINT8 cr;
   UINT8 cg;
   UINT8 cb;
   UINT8 ca;
   UINT32 pixel;
   UINT32* pixels = (Uint32*)dst->pixels;

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