#include "stdafx.h"

#include "SurfaceGraphics.h"

#include <SDL2/SDL_image.h>

namespace VP {

SurfaceGraphics::SurfaceGraphics(SDL_Surface* pSurface)
{
   m_pSurface = pSurface;

   m_width = pSurface->w;
   m_height = pSurface->h;
   m_color = RGB(0, 0, 0);
   m_alpha = 255;

   m_translateX = 0;
   m_translateY = 0;
}

SurfaceGraphics::~SurfaceGraphics()
{
}

void SurfaceGraphics::SetColor(OLE_COLOR color, UINT8 alpha)
{
   m_color = color;
   m_alpha = alpha;
}

void SurfaceGraphics::Clear()
{
   FillRectangle({ 0, 0, m_pSurface->w, m_pSurface->h });
}

void SurfaceGraphics::DrawImage(SDL_Surface* pImage, SDL_Rect* pSrcRect, SDL_Rect* pDstRect)
{
   SDL_Rect rect = { pDstRect->x + m_translateX, pDstRect->y + m_translateY, pDstRect->w, pDstRect->h };
   SDL_BlitScaled(pImage, pSrcRect, m_pSurface, &rect);
}

void SurfaceGraphics::FillRectangle(const SDL_Rect& rect)
{
   SDL_Rect dstRect = { rect.x + m_translateX, rect.y + m_translateY, rect.w, rect.h };

   if (m_alpha == 255)
      SDL_FillRect(m_pSurface, &dstRect, SDL_MapRGB(m_pSurface->format, GetRValue(m_color), GetGValue(m_color), GetBValue(m_color)));
   else {
      SDL_Surface* pSource = SDL_CreateRGBSurfaceWithFormat(0, m_pSurface->w, m_pSurface->h, 32, SDL_PIXELFORMAT_RGBA32);
      SDL_FillRect(pSource, NULL, SDL_MapRGBA(pSource->format, GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), m_alpha));
      SDL_BlitScaled(pSource, NULL, m_pSurface, &dstRect);
      SDL_FreeSurface(pSource);
   }
}

void SurfaceGraphics::SetClip(const SDL_Rect& rect)
{
   SDL_Rect dstRect = { m_translateX + rect.x, m_translateY + rect.y, rect.w, rect.h };
   SDL_SetClipRect(m_pSurface, &dstRect);
}

void SurfaceGraphics::ResetClip()
{
   SDL_SetClipRect(m_pSurface, nullptr);
}

void SurfaceGraphics::TranslateTransform(int x, int y)
{
   m_translateX += x;
   m_translateY += y;
}

}
