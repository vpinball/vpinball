/*
 * Portions of this code was derived from SDL_gfx and BBCSDL:
 *
 * https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx
 * https://github.com/rtrussell/BBCSDL/blob/master/src/SDL2_gfxPrimitives.c
 */

#include "stdafx.h"

#include "Graphics.h"

#include <SDL2/SDL_image.h>

#define MAX_GRAPHICS_POLYSIZE 16384

static int GraphicsCompareFloat(const void *a, const void *b)
{
   float diff = *((float*)a + 1) - *((float*)b + 1);
   if (diff != 0.f) return (diff > 0.f) - (diff < 0.f);
   diff = *(float*)a - *(float*)b;
   return (diff > 0.f) - (diff < 0.f);
}

namespace VP {

Graphics::Graphics(SDL_Renderer* pRenderer)
{
   m_pRenderer = pRenderer;
   m_pSurface = nullptr;

   SDL_RenderGetLogicalSize(pRenderer, &m_width, &m_height);
   m_color = RGB(0, 0, 0);
   m_alpha = 255;

   m_pModelMatrix = new Matrix();
   m_translateX = 0;
   m_translateY = 0;
}

Graphics::Graphics(SDL_Surface* pSurface)
{
   m_pSurface = pSurface;
   m_pRenderer = nullptr;

   m_width = pSurface->w;
   m_height = pSurface->h;
   m_color = RGB(0, 0, 0);
   m_alpha = 255;

   m_pModelMatrix = new Matrix();
   m_translateX = 0;
   m_translateY = 0;
}

Graphics::~Graphics()
{
   delete m_pModelMatrix;
}

void Graphics::SetColor(OLE_COLOR color, UINT8 alpha)
{
   m_color = color;
   m_alpha = alpha;
}

void Graphics::SetBlendMode(SDL_BlendMode blendMode)
{
   if (m_pRenderer)
      SDL_SetRenderDrawBlendMode(m_pRenderer, blendMode);
}

SDL_BlendMode Graphics::GetBlendMode()
{
   SDL_BlendMode blendMode = SDL_BLENDMODE_NONE;
   if (m_pRenderer)
      SDL_GetRenderDrawBlendMode(m_pRenderer, &blendMode);
   return blendMode;
}

void Graphics::Clear()
{
   if (m_pRenderer) {
      SDL_SetRenderDrawColor(m_pRenderer, GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), m_alpha);
      SDL_RenderClear(m_pRenderer);
   }
   else if (m_pSurface)
      FillRectangle({ 0, 0, m_pSurface->w, m_pSurface->h });
}

void Graphics::Present()
{
   if (!m_pRenderer)
      return;

   SDL_RenderPresent(m_pRenderer);
}

void Graphics::DrawPath(GraphicsPath* pPath)
{
   if (!m_pRenderer)
      return;

   const std::vector<SDL_FPoint>* const pPoints = pPath->GetPoints();
   if (pPoints->size() < 2)
      return;

   UINT8 r = GetRValue(m_color);
   UINT8 g = GetGValue(m_color);
   UINT8 b = GetBValue(m_color);
   UINT8 a = m_alpha;

   SDL_SetRenderDrawColor(m_pRenderer, r, g, b, a);

   for (size_t i = 0; i < pPoints->size() - 1; ++i) {
      SDL_FPoint p1 = (*pPoints)[i];
      SDL_FPoint p2 = (*pPoints)[i + 1];

      if (m_pModelMatrix) {
         m_pModelMatrix->TransformPoint(p1.x, p1.y);
         m_pModelMatrix->TransformPoint(p2.x, p2.y);
      }

      SDL_RenderDrawLine(m_pRenderer, (int)p1.x + m_translateX, (int)p1.y + m_translateY, (int)p2.x + m_translateX, (int)p2.y + m_translateY);
   }

   // Automatically close the path
   SDL_FPoint p1 = (*pPoints)[0];
   SDL_FPoint p2 = (*pPoints)[pPoints->size() - 1];

   if (m_pModelMatrix) {
      m_pModelMatrix->TransformPoint(p1.x, p1.y);
      m_pModelMatrix->TransformPoint(p2.x, p2.y);
   }

   SDL_RenderDrawLine(m_pRenderer, (int)p1.x + m_translateX, (int)p1.y + m_translateY, (int)p2.x + m_translateX, (int)p2.y + m_translateY);
}

void Graphics::FillPath(GraphicsPath* pPath)
{
   if (!m_pRenderer)
      return;

   const std::vector<SDL_FPoint>* const pPoints = pPath->GetPoints();

   int n = pPoints->size();
   if (n < 3)
      return;

   double vx[n];
   double vy[n];

   int i = 0;
   for (const auto &point : *pPoints) {
      float x = point.x;
      float y = point.y;
      if (m_pModelMatrix)
         m_pModelMatrix->TransformPoint(x, y);
      vx[i] = static_cast<double>(x) + m_translateX;
      vy[i++] = static_cast<double>(y) + m_translateY;
   }

   int xi, yi;
   double y0, y1;
   double minx = 99999.0;
   double maxx = -99999.0;
   double prec = 0.00001;
   UINT8 r = GetRValue(m_color);
   UINT8 g = GetGValue(m_color);
   UINT8 b = GetBValue(m_color);
   UINT8 a = m_alpha;

   for (i = 0; i < n; i++) {
      double x = vx[i];
      double y = fabs(vy[i]);
      if (x < minx) minx = x;
      if (x > maxx) maxx = x;
      if (y > prec) prec = y;
   }
   minx = floor(minx);
   maxx = floor(maxx);
   prec = floor(pow(2,19) / prec);

   float* const list = (float*)malloc(MAX_GRAPHICS_POLYSIZE * sizeof(float));
   if (list == nullptr)
      return;

   yi = 0;
   y0 = floor(vy[n - 1] * prec) / prec;
   y1 = floor(vy[0] * prec) / prec;
   for (i = 1; i <= n; i++) {
      if (yi > MAX_GRAPHICS_POLYSIZE - 4) {
         free(list);
         return;
      }
      double y2 = floor(vy[i % n] * prec) / prec;
      if (((y1 < y2) - (y1 > y2)) == ((y0 < y1) - (y0 > y1))) {
         list[yi++] = -100002.0f;
         list[yi++] = (float)y1;
         list[yi++] = -100002.0f;
         list[yi++] = (float)y1;
      }
      else {
         if (y0 != y1) {
            list[yi++] = (float)((y1 < y0) - (y1 > y0)) - 100002.0f;
            list[yi++] = (float)y1;
         }
         if (y1 != y2) {
            list[yi++] = (float)((y1 < y2) - (y1 > y2)) - 100002.0f;
            list[yi++] = (float)y1;
         }
      }
      y0 = y1;
      y1 = y2;
   }
   xi = yi;

   qsort (list, yi / 2, sizeof(float) * 2, GraphicsCompareFloat);

   for (i = 1; i <= n; i++) {
      double x, y;
      double d = 0.5 / prec;

      double x1 = vx[i - 1];
      y1 = floor(vy[i - 1] * prec) / prec;
      double x2 = vx[i % n];
      double y2 = floor(vy[i % n] * prec) / prec;

      if (y2 < y1) {
         double tmp1 = x1; x1 = x2; x2 = tmp1;
         double tmp2 = y1; y1 = y2; y2 = tmp2;
      }
      if (y2 != y1)
         y0 = (x2 - x1) / (y2 - y1);

      for (int j = 1; j < xi; j += 4) {
         y = list[j];
         if (((y + d) <= y1) || (y == list[j + 4]))
            continue;
         if ((y -= d) >= y2)
            break;
         if (yi > MAX_GRAPHICS_POLYSIZE - 4) {
            free(list);
            return;
         }
         if (y > y1) {
            list[yi++] = (float)(x1 + y0 * (y - y1));
            list[yi++] = (float)y;
         }
         y += d * 2.0;
         if (y < y2) {
            list[yi++] = (float)(x1 + y0 * (y - y1));
            list[yi++] = (float)y;
         }
      }

      y = floor(y1) + 1.0;
      while (y <= y2) {
         x = x1 + y0 * (y - y1);
         if (yi > MAX_GRAPHICS_POLYSIZE - 2) {
            free(list);
            return;
         }
         list[yi++] = (float)x;
         list[yi++] = (float)y;
         y += 1.0;
      }
   }

   qsort (list, yi / 2, sizeof(float) * 2, GraphicsCompareFloat);

   float* const strip = (float*)malloc(((size_t)(maxx - minx) + 2) * sizeof(float));
   if (strip == nullptr) {
      free(list);
      return;
   }
   memset(strip, 0, ((size_t)(maxx - minx) + 2) * sizeof(float));
   n = yi;
   yi = (int)list[1];
   int j = 0;
   for (i = 0; i < n - 7; i += 4) {
      float x1 = list[i + 0];
      float y1 = list[i + 1];
      float x3 = list[i + 2];
      float x2 = list[i + j + 0];
      float y2 = list[i + j + 1];
      float x4 = list[i + j + 2];

      if (x1 + x3 == -200002.0f)
         j += 4;
      else if (x1 + x3 == -200006.0f)
         j -= 4;
      else if ((x1 >= minx) && (x2 >= minx)) {
         if (x1 > x2) { float tmp = x1; x1 = x2; x2 = tmp; }
         if (x3 > x4) { float tmp = x3; x3 = x4; x4 = tmp; }

         for ( xi = (int)(x1 - minx); xi <= (int)(x4 - minx); xi++ ) {
            float u, v;
            float x = (float)(minx + xi);
            if (x < x2)  u = (x - x1 + 1.f) / (x2 - x1 + 1.f); else u = 1.0f;
            if (x >= x3 - 1.f) v = (x4 - x) / (x4 - x3 + 1.f); else v = 1.0f;
            if ((u > 0.0f) && (v > 0.0f))
               strip[xi] += (y2 - y1) * (u + v - 1.0f);
         }
      }

      if ((yi == (int)(list[i + 5] - 1.0f)) || (i == n - 8)) {
         for (xi = 0; xi <= maxx - minx; xi++) {
            if (strip[xi] != 0.0f) {
               if (strip[xi] >= 0.996f) {
                  int x0 = xi;
                  while (strip[++xi] >= 0.996f) ;
                  xi--;
                  SDL_SetRenderDrawColor(m_pRenderer, r, g, b, a);
                  SDL_RenderDrawLine(m_pRenderer, (int)minx + x0, yi, (int)minx + xi, yi);
               }
               else {
                  SDL_SetRenderDrawColor(m_pRenderer, r, g, b, (UINT8)(a * strip[xi]));
                  SDL_RenderDrawPoint(m_pRenderer, (int)minx + xi, yi);
               }
            }
         }
         memset(strip, 0, ((size_t)(maxx - minx) + 2) * sizeof(float));
         yi++;
      }
   }
   free(list);
   free(strip);
}

void Graphics::DrawImage(SDL_Surface* pImage, SDL_Rect* pSrcRect, SDL_Rect* pDstRect)
{
   if (m_pRenderer) {
      SDL_Texture* pTexture = SDL_CreateTextureFromSurface(m_pRenderer, pImage);
      SDL_RenderCopy(m_pRenderer, pTexture, pSrcRect, pDstRect);
      SDL_DestroyTexture(pTexture);
   }
   else if (m_pSurface) {
      SDL_Rect rect = { pDstRect->x + m_translateX, pDstRect->y + m_translateY, pDstRect->w, pDstRect->h };
      SDL_BlitScaled(pImage, pSrcRect, m_pSurface, &rect);
   }
}

void Graphics::DrawTexture(SDL_Texture* pTexture, SDL_Rect* pSrcRect, SDL_Rect* pDstRect)
{
   if (!m_pRenderer || !pTexture)
      return;

   SDL_RenderCopy(m_pRenderer, pTexture, pSrcRect, pDstRect);
}

void Graphics::FillRectangle(const SDL_Rect& rect)
{
   if (m_pRenderer) {
      SDL_SetRenderDrawColor(m_pRenderer, GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), m_alpha);
      SDL_RenderFillRect(m_pRenderer, &rect);
   }
   else if (m_pSurface) {
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
}

void Graphics::SetClip(const SDL_Rect& rect)
{
   if (!m_pSurface)
      return;

   SDL_Rect dstRect = { m_translateX + rect.x, m_translateY + rect.y, rect.w, rect.h };
   SDL_SetClipRect(m_pSurface, &dstRect);
}

void Graphics::ResetClip()
{
   if (!m_pSurface)
      return;

   SDL_SetClipRect(m_pSurface, nullptr);
}

void Graphics::TranslateTransform(int x, int y)
{
   m_translateX += x;
   m_translateY += y;
}

void Graphics::ResetTransform()
{
   m_pModelMatrix->Reset();
}

void Graphics::SetTransform(Matrix* pModelMatrix)
{
   delete m_pModelMatrix;
   m_pModelMatrix = pModelMatrix->Clone();
}

}
