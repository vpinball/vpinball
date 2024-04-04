#include "stdafx.h"
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

   SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, src->w, src->h, 32, SDL_PIXELFORMAT_RGBA32);

   UINT8 cr;
   UINT8 cg;
   UINT8 cb;
   UINT8 ca;
   UINT32 pixel;
   UINT32* pixels = (Uint32*)dst->pixels;

   for (int y = 0; y < dst->h; y++) {
      for (int x = 0; x < dst->w; x++) {
         pixel = GetPixel(src, x, y);
         SDL_GetRGBA(pixel, src->format, &cr, &cg, &cb, &ca);

         if (cr < 64 && cg < 64 && cb < 64)
            pixels[(y * dst->w) + x] = SDL_MapRGBA(dst->format, cr, cg, cb, 0);
         else
            pixels[(y * dst->w) + x] = SDL_MapRGBA(dst->format, cr, cg, cb, ca);
      }
   }

   pBitmap->SetData(dst);
}