#include "stdafx.h"

#include "Graphics.h"

Graphics::Graphics(int width, int height)
{
   m_pSurface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 24, SDL_PIXELFORMAT_RGB24);

   m_translateX = 0;
   m_translateY = 0;
}

Graphics::~Graphics()
{
   SDL_FreeSurface(m_pSurface);
}

void Graphics::TranslateTransform(int x, int y)
{
   m_translateX += x;
   m_translateY += y;
}

void Graphics::Clear(OLE_COLOR color)
{
   FillRectangle(color, 0, 0, m_pSurface->w, m_pSurface->h);
}

void Graphics::SetClip(int x, int y, int width, int height)
{
   SDL_Rect rect = { m_translateX + x, m_translateY + y, width, height };
   SDL_SetClipRect(m_pSurface, &rect);
}

void Graphics::ResetClip()
{
   SDL_SetClipRect(m_pSurface, NULL);
}

void Graphics::DrawImage(SDL_Surface* pImage, int x, int y, int width, int height)
{
   SDL_Rect rect = { x + m_translateX, y + m_translateY, width, height };
   SDL_BlitScaled(pImage, NULL, m_pSurface, &rect);
}

void Graphics::DrawImage(SDL_Surface* pImage, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh)
{
   SDL_Rect srcRect = { sx, sy, sw, sh };
   SDL_Rect dstRect = { dx + m_translateX, dy + m_translateY, dw, dh };
   SDL_BlitScaled(pImage, &srcRect, m_pSurface, &dstRect);
}

void Graphics::FillRectangle(OLE_COLOR color, int x, int y, int width, int height)
{
   SDL_Rect rect = { x + m_translateX, y + m_translateY, width, height };
   SDL_FillRect(m_pSurface, &rect, SDL_MapRGB(m_pSurface->format, GetRValue(color), GetGValue(color), GetBValue(color)));
}

void Graphics::FillRectangleAlpha(OLE_COLOR color, int alpha, int x, int y, int width, int height)
{
   SDL_Surface* src = SDL_CreateRGBSurfaceWithFormat(0, m_pSurface->w, m_pSurface->h, 32, SDL_PIXELFORMAT_RGBA32);
   SDL_FillRect(src, NULL, SDL_MapRGBA(src->format, GetRValue(color), GetGValue(color), GetBValue(color), alpha));
   SDL_Rect rect = { x + m_translateX, y + m_translateY, width, height };
   SDL_BlitScaled(src, NULL, m_pSurface, &rect);
   SDL_FreeSurface(src);
}