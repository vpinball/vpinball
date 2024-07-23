#include "stdafx.h"
#include "RegionFilter.h"

RegionFilter::RegionFilter()
{
   m_x = 0;
   m_y = 0;
   m_width = 0;
   m_height = 0;
}

RegionFilter::~RegionFilter()
{
}

void RegionFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, m_width, m_height, 32, SDL_PIXELFORMAT_RGBA32);
   SDL_FillRect(dst, NULL, SDL_MapRGBA(dst->format, 0, 0, 0, 0));
   SDL_Rect srcRect = { m_x, m_y, m_width, m_height };
   SDL_BlitScaled(src, &srcRect, dst, NULL);

   pBitmap->SetData(dst);
}