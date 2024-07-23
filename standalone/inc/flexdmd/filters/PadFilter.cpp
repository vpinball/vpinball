#include "stdafx.h"
#include "PadFilter.h"

PadFilter::PadFilter()
{
   m_left = 0;
   m_top = 0;
   m_right = 0;
   m_bottom = 0;
}

PadFilter::~PadFilter()
{
}

void PadFilter::Filter(Bitmap* pBitmap)
{
   SDL_Surface* src = pBitmap->GetSurface();

   if (!src)
      return;

   SDL_Surface* dst = SDL_CreateRGBSurfaceWithFormat(0, src->w + m_left + m_right, src->h + m_top + m_bottom, 32, SDL_PIXELFORMAT_RGBA32);
   SDL_FillRect(dst, NULL, SDL_MapRGBA(dst->format, 0, 0, 0, 0));
   SDL_Rect dstRect = { m_left, m_top, src->w, src->h };
   SDL_BlitScaled(src, NULL, dst, &dstRect);

   pBitmap->SetData(dst);
}