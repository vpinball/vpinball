#include "RegionFilter.h"

void RegionFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* dst = SDL_CreateSurface(m_width, m_height, SDL_PIXELFORMAT_RGBA32);
   SDL_FillSurfaceRect(dst, NULL, SDL_MapSurfaceRGBA(dst, 0, 0, 0, 0));
   SDL_Rect srcRect = { m_x, m_y, m_width, m_height };
   SDL_BlitSurfaceScaled(src, &srcRect, dst, NULL, SDL_SCALEMODE_NEAREST);

   pBitmap->SetData(dst);
}
