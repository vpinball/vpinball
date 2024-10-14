#include "core/stdafx.h"
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

   SDL_Surface* dst = SDL_CreateSurface(src->w + m_left + m_right, src->h + m_top + m_bottom, SDL_PIXELFORMAT_RGBA32);
   SDL_FillSurfaceRect(dst, NULL, SDL_MapSurfaceRGBA(dst, 0, 0, 0, 0));
   SDL_Rect dstRect = { m_left, m_top, src->w, src->h };
   SDL_BlitSurfaceScaled(src, NULL, dst, &dstRect, SDL_SCALEMODE_NEAREST);

   pBitmap->SetData(dst);
}