/*
 * Portions of this code was derived from SDL_gfx, BBCSDL, and Wine:
 *
 * https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx
 * https://github.com/rtrussell/BBCSDL/blob/master/src/SDL2_gfxPrimitives.c
 * https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/gdiplus/graphics.c
 * https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/gdiplus/gdiplus_private.h
 */

#include "common.h"
#include "VPXGraphics.h"

#include <cmath>
#include <cfloat>
#include <cstring>

#ifndef _COLORREF_DEFINED
   typedef unsigned int COLORREF;
   #define _COLORREF_DEFINED
#endif
#include <utils/color.h>

#define MAX_GRAPHICS_POLYSIZE 16384

namespace B2SLegacy {

static int GraphicsCompareFloat(const void *a, const void *b)
{
   float diff = *((float*)a + 1) - *((float*)b + 1);
   if (diff != 0.f)
      return (diff > 0.f) - (diff < 0.f);
   diff = *(float*)a - *(float*)b;
   return (diff > 0.f) - (diff < 0.f);
}

typedef uint32_t ARGB;
typedef float REAL;
typedef int INT;

static inline INT gdip_round(REAL x)
{
    return (INT) floorf(x + 0.5);
}

static ARGB blend_colors(ARGB start, ARGB end, REAL position)
{
    INT start_a, end_a, final_a;
    INT pos;

    pos = gdip_round(position * 0xff);

    start_a = ((start >> 24) & 0xff) * (pos ^ 0xff);
    end_a = ((end >> 24) & 0xff) * pos;

    final_a = start_a + end_a;

    if (final_a < 0xff) return 0;

    return (final_a / 0xff) << 24 |
        ((((start >> 16) & 0xff) * start_a + (((end >> 16) & 0xff) * end_a)) / final_a) << 16 |
        ((((start >> 8) & 0xff) * start_a + (((end >> 8) & 0xff) * end_a)) / final_a) << 8 |
        (((start & 0xff) * start_a + ((end & 0xff) * end_a)) / final_a);
}

VPXGraphics::VPXGraphics(VPXPluginAPI* vpxApi, int width, int height)
   : m_vpxApi(vpxApi),
     m_width(width),
     m_height(height),
     m_bufferSize(width * height * 4)
{
   m_pixelBuffer = new uint8_t[m_bufferSize];
   memset(m_pixelBuffer, 0, m_bufferSize);
}

VPXGraphics::~VPXGraphics()
{
   if (m_texture)
      m_vpxApi->DeleteTexture(m_texture);
   delete[] m_pixelBuffer;
}

void VPXGraphics::Clear()
{
   memset(m_pixelBuffer, 0, m_bufferSize);
   m_needsTextureUpdate = true;
}

void VPXGraphics::SetColor(uint32_t color, uint8_t alpha)
{
   m_color = color;
   m_alpha = alpha;
}

void VPXGraphics::FillPath(GraphicsPath* pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath->GetPoints();
   if (pPoints->size() < 3)
      return;

   FillPolygon(*pPoints);
   m_needsTextureUpdate = true;
}

void VPXGraphics::FillPath(Brush* pBrush, GraphicsPath* pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath->GetPoints();
   if (pPoints->size() < 3)
      return;

   if (pBrush->GetBrushType() == BrushType_PathGradient) {
      PathGradientBrush* const pGradientBrush = static_cast<PathGradientBrush*>(pBrush);
      if (pGradientBrush->GetPath()->GetPoints()->size() < 3)
         return;
      GradientData gradientData;
      InitGradientData(gradientData, pGradientBrush);
      FillPolygon(*pPoints, &gradientData);
   }
   else {
      SolidBrush* const pSolidBrush = static_cast<SolidBrush*>(pBrush);
      SetColor(pSolidBrush->GetColor(), pSolidBrush->GetAlpha());
      FillPolygon(*pPoints);
   }
   m_needsTextureUpdate = true;
}

void VPXGraphics::InitGradientData(GradientData& gradientData, PathGradientBrush* pBrush)
{
   const std::vector<SDL_FPoint>* const pPoints = pBrush->GetPath()->GetPoints();
   const size_t count = pPoints->size();

   float centerX = 0.0f;
   float centerY = 0.0f;
   for (const auto& point : *pPoints) {
      centerX += point.x;
      centerY += point.y;
   }
   centerX /= (float)count;
   centerY /= (float)count;

   gradientData.outer.reserve(count);
   for (const auto& point : *pPoints) {
      float x = point.x;
      float y = point.y;
      m_pModelMatrix.TransformPoint(x, y);
      gradientData.outer.push_back({ x + (float)m_translateX, y + (float)m_translateY });
   }

   // FocusScales push the solid center color outward: uniform scales give a
   // constant inner boundary, anisotropic ones keep a scaled inner polygon.
   const SDL_FPoint& focusScales = pBrush->GetFocusScales();
   if (focusScales.x == focusScales.y)
      gradientData.uniformInner = 1.0f - clamp(focusScales.x, 0.0f, 0.999f);
   else {
      gradientData.inner.reserve(count);
      for (const auto& point : *pPoints) {
         float x = centerX + (point.x - centerX) * focusScales.x;
         float y = centerY + (point.y - centerY) * focusScales.y;
         m_pModelMatrix.TransformPoint(x, y);
         gradientData.inner.push_back({ x + (float)m_translateX, y + (float)m_translateY });
      }
   }

   m_pModelMatrix.TransformPoint(centerX, centerY);
   gradientData.cx = centerX + (float)m_translateX;
   gradientData.cy = centerY + (float)m_translateY;

   const uint32_t centerColor = pBrush->GetCenterColor();
   gradientData.centerArgb = ((uint32_t)pBrush->GetCenterAlpha() << 24)
      | ((uint32_t)GetRValue(centerColor) << 16) | ((uint32_t)GetGValue(centerColor) << 8) | (uint32_t)GetBValue(centerColor);
   const uint32_t surroundColor = pBrush->GetSurroundColor();
   gradientData.surroundArgb = ((uint32_t)pBrush->GetSurroundAlpha() << 24)
      | ((uint32_t)GetRValue(surroundColor) << 16) | ((uint32_t)GetGValue(surroundColor) << 8) | (uint32_t)GetBValue(surroundColor);
}

void VPXGraphics::GradientColorAt(const GradientData& gradientData, float x, float y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
   // GDI+ fans the path into one triangle per edge; we sample a single pixel
   // (to reuse FillPolygon's anti-aliasing), so first pick its containing
   // triangle, then use Wine's distance math.
   const std::vector<SDL_FPoint>& outer = gradientData.outer;
   const size_t count = outer.size();
   const SDL_FPoint center_point = { gradientData.cx, gradientData.cy };

   float distance = 1.0f;
   float innerDistance = gradientData.uniformInner;
   float bestScore = -FLT_MAX;
   for (size_t i = 0; i < count; i++) {
      const SDL_FPoint start_point = outer[i];
      const SDL_FPoint end_point = outer[(i + 1) == count ? 0 : (i + 1)];

      const float denom = (start_point.y - end_point.y) * (center_point.x - end_point.x) + (end_point.x - start_point.x) * (center_point.y - end_point.y);
      if (fabsf(denom) < 1e-6f)
         continue;
      const float wCenter = ((start_point.y - end_point.y) * (x - end_point.x) + (end_point.x - start_point.x) * (y - end_point.y)) / denom;
      const float wStart = ((end_point.y - center_point.y) * (x - end_point.x) + (center_point.x - end_point.x) * (y - end_point.y)) / denom;
      const float wEnd = 1.0f - wCenter - wStart;
      const float score = std::min(std::min(wCenter, wStart), wEnd);
      if (score <= bestScore)
         continue;
      bestScore = score;

      const float center_distance = (end_point.y - start_point.y) * (start_point.x - center_point.x) +
         (end_point.x - start_point.x) * (center_point.y - start_point.y);
      if (fabsf(center_distance) < 1e-6f) {
         distance = 1.0f;
         innerDistance = gradientData.uniformInner;
         continue;
      }
      distance = (end_point.y - start_point.y) * (start_point.x - x) +
         (end_point.x - start_point.x) * (y - start_point.y);
      distance = clamp(distance / center_distance, 0.0f, 1.0f);

      // FocusScales (the bulb glow) moves the full-center-color boundary outward.
      if (gradientData.inner.empty())
         innerDistance = gradientData.uniformInner;
      else {
         const SDL_FPoint& innerS = gradientData.inner[i];
         const SDL_FPoint& innerE = gradientData.inner[(i + 1) == count ? 0 : (i + 1)];
         const float dInnerS = ((end_point.y - start_point.y) * (start_point.x - innerS.x) + (end_point.x - start_point.x) * (innerS.y - start_point.y)) / center_distance;
         const float dInnerE = ((end_point.y - start_point.y) * (start_point.x - innerE.x) + (end_point.x - start_point.x) * (innerE.y - start_point.y)) / center_distance;
         const float t = (wStart + wEnd > 1e-6f) ? clamp(wEnd / (wStart + wEnd), 0.0f, 1.0f) : 0.0f;
         innerDistance = dInnerS + (dInnerE - dInnerS) * t;
      }
   }

   const float position = (innerDistance > 1e-6f) ? clamp(distance / innerDistance, 0.0f, 1.0f) : 1.0f;

   const ARGB outer_color = gradientData.surroundArgb;
   const ARGB argb = blend_colors(outer_color, gradientData.centerArgb, position);
   a = (uint8_t)((argb >> 24) & 0xff);
   r = (uint8_t)((argb >> 16) & 0xff);
   g = (uint8_t)((argb >> 8) & 0xff);
   b = (uint8_t)(argb & 0xff);
}

void VPXGraphics::DrawPath(GraphicsPath* pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath->GetPoints();
   if (pPoints->size() < 2)
      return;

   DrawPolygonOutline(*pPoints);
   m_needsTextureUpdate = true;
}

void VPXGraphics::FillPolygon(const std::vector<SDL_FPoint>& points, const GradientData* pGradientData)
{
   const int ps = static_cast<int>(points.size());
   if (ps < 3) return;

   double* const __restrict vx = new double[ps];
   double* const __restrict vy = new double[ps];

   // Transform points and store in arrays
   int i = 0;
   for (const auto &point : points) {
      float x = point.x;
      float y = point.y;
      m_pModelMatrix.TransformPoint(x, y);
      vx[i] = static_cast<double>(x) + m_translateX;
      vy[i] = static_cast<double>(y) + m_translateY;
      i++;
   }

   double minx = 99999.0;
   double maxx = -99999.0;
   double prec = 0.00001;
   uint8_t r = GetRValue(m_color);
   uint8_t g = GetGValue(m_color);
   uint8_t b = GetBValue(m_color);
   uint8_t a = m_alpha;

   for (i = 0; i < ps; i++) {
      double x = vx[i];
      double y = std::abs(vy[i]);
      if (x < minx) minx = x;
      if (x > maxx) maxx = x;
      if (y > prec) prec = y;
   }
   minx = std::floor(minx);
   maxx = std::floor(maxx);
   static const/*expr*/ double p219 = std::pow(2,19);
   prec = std::floor(p219 / prec);

   float* const __restrict list = new float[MAX_GRAPHICS_POLYSIZE];

   int yi = 0;
   double y0 = std::floor(vy[ps - 1] * prec) / prec;
   double y1 = std::floor(vy[0] * prec) / prec;
   for (i = 1; i <= ps; i++) {
      if (yi > MAX_GRAPHICS_POLYSIZE - 4) {
         delete[] list;
         delete[] vx;
         delete[] vy;
         return;
      }
      double y2 = std::floor(vy[i == ps ? 0 : i] * prec) / prec;
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
   const int xi = yi;

   qsort (list, yi / 2, sizeof(float) * 2, GraphicsCompareFloat);

   for (i = 1; i <= ps; i++) {
      const double d = 0.5 / prec;

      double x1 = vx[i - 1];
      y1 = floor(vy[i - 1] * prec) / prec;
      double x2 = vx[i == ps ? 0 : i];
      double y2 = std::floor(vy[i == ps ? 0 : i] * prec) / prec;

      if (y2 < y1) {
         double tmp1 = x1; x1 = x2; x2 = tmp1;
         double tmp2 = y1; y1 = y2; y2 = tmp2;
      }
      if (y2 != y1)
         y0 = (x2 - x1) / (y2 - y1);

      for (int j = 1; j < xi; j += 4) {
         double y = list[j];
         if (((y + d) <= y1) || (y == list[j + 4]))
            continue;
         if ((y -= d) >= y2)
            break;
         if (yi > MAX_GRAPHICS_POLYSIZE - 4) {
            delete[] list;
            delete[] vx;
            delete[] vy;
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

      double y = std::floor(y1) + 1.0;
      while (y <= y2) {
         const double x = x1 + y0 * (y - y1);
         if (yi > MAX_GRAPHICS_POLYSIZE - 2) {
            delete[] list;
            delete[] vx;
            delete[] vy;
            return;
         }
         list[yi++] = (float)x;
         list[yi++] = (float)y;
         y += 1.0;
      }
   }

   delete[] vx;
   delete[] vy;

   qsort (list, yi / 2, sizeof(float) * 2, GraphicsCompareFloat);

   float* const __restrict strip = new float[(size_t)(maxx - minx) + 2];
   memset(strip, 0, ((size_t)(maxx - minx) + 2) * sizeof(float));
   const int n = yi;
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

         for (int xi = (int)(x1 - minx); xi <= (int)(x4 - minx); xi++) {
            float u, v;
            float x = (float)(minx + xi);
            if (x < x2)  u = (x - x1 + 1.f) / (x2 - x1 + 1.f); else u = 1.0f;
            if (x >= x3 - 1.f) v = (x4 - x) / (x4 - x3 + 1.f); else v = 1.0f;
            if ((u > 0.0f) && (v > 0.0f))
               strip[xi] += (y2 - y1) * (u + v - 1.0f);
         }
      }

      if ((yi == (int)(list[i + 5] - 1.0f)) || (i == n - 8)) {
         for (int xi = 0; xi <= maxx - minx; xi++) {
            if (strip[xi] != 0.0f) {
               if (strip[xi] >= 0.996f) {
                  // Fill solid pixels
                  int x0 = xi;
                  while (strip[++xi] >= 0.996f) ;
                  xi--;
                  for (int x = x0; x <= xi; x++) {
                     if (pGradientData) {
                        uint8_t gr, gg, gb, ga;
                        GradientColorAt(*pGradientData, (float)minx + (float)x + 0.5f, (float)yi + 0.5f, gr, gg, gb, ga);
                        SetPixelBlended((int)minx + x, yi, gr, gg, gb, ga);
                     }
                     else
                        SetPixel((int)minx + x, yi, r, g, b, a);
                  }
               }
               else {
                  // Anti-aliased pixel
                  if (pGradientData) {
                     uint8_t gr, gg, gb, ga;
                     GradientColorAt(*pGradientData, (float)minx + (float)xi + 0.5f, (float)yi + 0.5f, gr, gg, gb, ga);
                     SetPixelBlended((int)minx + xi, yi, gr, gg, gb, (uint8_t)((float)ga * strip[xi]));
                  }
                  else {
                     uint8_t blendedAlpha = (uint8_t)((float)a * strip[xi]);
                     SetPixelBlended((int)minx + xi, yi, r, g, b, blendedAlpha);
                  }
               }
            }
         }
         memset(strip, 0, ((size_t)(maxx - minx) + 2) * sizeof(float));
         yi++;
      }
   }
   delete[] list;
   delete[] strip;
}

void VPXGraphics::DrawPolygonOutline(const std::vector<SDL_FPoint>& points)
{
   const int ps = static_cast<int>(points.size());
   if (ps < 2)
      return;

   uint8_t r = GetRValue(m_color);
   uint8_t g = GetGValue(m_color);
   uint8_t b = GetBValue(m_color);
   uint8_t a = m_alpha;

   for (int i = 0; i < ps; i++) {
      const int nextIndex = (i + 1) == ps ? 0 : (i+1);

      float x1 = points[i].x;
      float y1 = points[i].y;
      float x2 = points[nextIndex].x;
      float y2 = points[nextIndex].y;

      m_pModelMatrix.TransformPoint(x1, y1);
      m_pModelMatrix.TransformPoint(x2, y2);

      x1 += (float)m_translateX;
      y1 += (float)m_translateY;
      x2 += (float)m_translateX;
      y2 += (float)m_translateY;

      DrawLine((int)x1, (int)y1, (int)x2, (int)y2, r, g, b, a);
   }
}

void VPXGraphics::DrawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   int dx = abs(x2 - x1);
   int dy = abs(y2 - y1);
   int sx = (x1 < x2) ? 1 : -1;
   int sy = (y1 < y2) ? 1 : -1;
   int err = dx - dy;

   while (true) {
      SetPixel(x1, y1, r, g, b, a);

      if (x1 == x2 && y1 == y2)
         break;

      int e2 = 2 * err;
      if (e2 > -dy) {
         err -= dy;
         x1 += sx;
      }
      if (e2 < dx) {
         err += dx;
         y1 += sy;
      }
   }
}

void VPXGraphics::SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   if (/*x < 0 ||*/ (unsigned int)x >= (unsigned int)m_width /*|| y < 0*/ || (unsigned int)y >= (unsigned int)m_height) // unsigned int test incl. <0
      return;

   int offset = (y * m_width + x) * 4;
   m_pixelBuffer[offset + 0] = r;
   m_pixelBuffer[offset + 1] = g;
   m_pixelBuffer[offset + 2] = b;
   m_pixelBuffer[offset + 3] = a;
}

void VPXGraphics::SetPixelBlended(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   if (/*x < 0 ||*/ (unsigned int)x >= (unsigned int)m_width /*|| y < 0*/ || (unsigned int)y >= (unsigned int)m_height) // unsigned int test incl. <0
      return;

   int offset = (y * m_width + x) * 4;

   uint8_t existingR = m_pixelBuffer[offset + 0];
   uint8_t existingG = m_pixelBuffer[offset + 1];
   uint8_t existingB = m_pixelBuffer[offset + 2];
   uint8_t existingA = m_pixelBuffer[offset + 3];

   float alpha = (float)a * (float)(1.0/255.0);
   float invAlpha = 1.0f - alpha;

   m_pixelBuffer[offset + 0] = (uint8_t)((float)r * alpha + (float)existingR * invAlpha);
   m_pixelBuffer[offset + 1] = (uint8_t)((float)g * alpha + (float)existingG * invAlpha);
   m_pixelBuffer[offset + 2] = (uint8_t)((float)b * alpha + (float)existingB * invAlpha);
   m_pixelBuffer[offset + 3] = (uint8_t)((float)a         + (float)existingA * invAlpha);
}

VPXTexture VPXGraphics::GetTexture()
{
   if (m_needsTextureUpdate)
      SyncTexture();

   return m_texture;
}

void VPXGraphics::SyncTexture()
{
   UpdateTexture(&m_texture, m_width, m_height, VPXTEXFMT_sRGBA8, m_pixelBuffer);
   m_needsTextureUpdate = false;
}

void VPXGraphics::DrawToContext(VPXRenderContext2D* ctx, int left, int top)
{
   VPXTexture texture = GetTexture();
   if (texture && ctx->DrawImage) {
      ctx->DrawImage(ctx, texture, 1.0f, 1.0f, 1.0f, 1.0f,
                     0, 0, (float)m_width, (float)m_height, 0, 0, 0,
                     (float)left, (float)top, (float)m_width, (float)m_height);
   }
}

void VPXGraphics::TranslateTransform(int x, int y)
{
   m_translateX += x;
   m_translateY += y;
}

void VPXGraphics::ResetTransform()
{
   m_pModelMatrix.Reset();
   m_translateX = 0;
   m_translateY = 0;
}

void VPXGraphics::SetTransform(const Matrix& pModelMatrix)
{
   m_pModelMatrix = pModelMatrix;
}

void VPXGraphics::FillRectangle(const SDL_Rect& rect)
{
   const uint32_t r = GetRValue(m_color);
   const uint32_t g = GetGValue(m_color);
   const uint32_t b = GetBValue(m_color);
   const uint32_t a = m_alpha;
   const uint32_t col = (a << 24) | (b << 16) | (g << 8) | r;

   int x1 = rect.x + m_translateX;
   int y1 = rect.y + m_translateY;
   int x2 = x1 + rect.w;
   int y2 = y1 + rect.h;

   x1 = std::max(0, x1);
   y1 = std::max(0, y1);
   x2 = std::min(m_width, x2);
   y2 = std::min(m_height, y2);

   uint32_t* const __restrict buf = reinterpret_cast<uint32_t*>(m_pixelBuffer);

   for (int y = y1; y < y2; y++)
   {
      int offset = y * m_width + x1;
      for (int x = x1; x < x2; x++,offset++)
         buf[offset] = col;
   }

   m_needsTextureUpdate = true;
}

void VPXGraphics::UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image)
{
   if (m_vpxApi)
      m_vpxApi->UpdateTexture(texture, width, height, format, image);
}

VPXTexture VPXGraphics::CreateTexture(VPXPluginAPI* vpxApi, SDL_Surface* surf)
{
   VPXTexture texture = nullptr;
   if (vpxApi && surf) {
      SDL_LockSurface(surf);
      vpxApi->UpdateTexture(&texture, surf->w, surf->h, VPXTextureFormat::VPXTEXFMT_sRGBA8, static_cast<uint8_t*>(surf->pixels));
      SDL_UnlockSurface(surf);
   }
   return texture;
}

VPXTexture VPXGraphics::DuplicateTexture(VPXPluginAPI* vpxApi, VPXTexture sourceTexture)
{
   if (!vpxApi || !sourceTexture)
      return nullptr;

   SDL_Surface* tempSurface = VPXTextureToSDLSurface(vpxApi, sourceTexture);
   if (!tempSurface)
      return nullptr;

   VPXTexture duplicateTexture = SDLSurfaceToVPXTexture(vpxApi, tempSurface);
   SDL_DestroySurface(tempSurface);

   return duplicateTexture;
}

SDL_Surface* VPXGraphics::VPXTextureToSDLSurface(VPXPluginAPI* vpxApi, VPXTexture texture)
{
   if (!vpxApi || !texture)
      return nullptr;

   VPXTextureInfo* texInfo = vpxApi->GetTextureInfo(texture);
   if (!texInfo || !texInfo->data)
      return nullptr;

   SDL_PixelFormat sdlFormat;
   switch (texInfo->format) {
      case VPXTEXFMT_sRGBA8:
         sdlFormat = SDL_PIXELFORMAT_RGBA32;
         break;
      case VPXTEXFMT_sRGB8:
         sdlFormat = SDL_PIXELFORMAT_RGB24;
         break;
      case VPXTEXFMT_sRGB565:
         sdlFormat = SDL_PIXELFORMAT_RGB565;
         break;
      default:
         return nullptr;
   }

   int bytesPerPixel;
   switch (texInfo->format) {
      case VPXTEXFMT_sRGBA8: bytesPerPixel = 4; break;
      case VPXTEXFMT_sRGB8: bytesPerPixel = 3; break;
      case VPXTEXFMT_sRGB565: bytesPerPixel = 2; break;
      default: return nullptr;
   }

   int pitch = texInfo->width * bytesPerPixel;
   SDL_Surface* surface = SDL_CreateSurfaceFrom(texInfo->width, texInfo->height, sdlFormat,
                                                texInfo->data, pitch);

   if (surface) {
      SDL_Surface* copy = SDL_DuplicateSurface(surface);
      SDL_DestroySurface(surface);
      return copy;
   }

   return nullptr;
}

VPXTexture VPXGraphics::SDLSurfaceToVPXTexture(VPXPluginAPI* vpxApi, SDL_Surface* surface)
{
   if (!vpxApi || !surface)
      return nullptr;

   SDL_Surface* convertedSurface = nullptr;
   if (surface->format != SDL_PIXELFORMAT_RGBA32) {
      convertedSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
      if (!convertedSurface)
         return nullptr;
   }
   else
      convertedSurface = surface;

   VPXTexture texture = nullptr;
   SDL_LockSurface(convertedSurface);
   vpxApi->UpdateTexture(&texture, convertedSurface->w, convertedSurface->h,
                        VPXTEXFMT_sRGBA8, static_cast<uint8_t*>(convertedSurface->pixels));
   SDL_UnlockSurface(convertedSurface);

   if (convertedSurface != surface)
      SDL_DestroySurface(convertedSurface);

   return texture;
}

void VPXGraphics::DrawImage(VPXPluginAPI* vpxApi, VPXRenderContext2D* ctx, VPXTexture texture, SDL_Rect* srcRect, SDL_Rect* destRect)
{
   if (!ctx || !texture || !ctx->DrawImage)
      return;

   VPXTextureInfo* texInfo = vpxApi->GetTextureInfo(texture);
   if (!texInfo)
      return;

   float tintR = 1.0f, tintG = 1.0f, tintB = 1.0f, alpha = 1.0f;
   float pivotX = 0.0f, pivotY = 0.0f, rotation = 0.0f;

   float texX = srcRect ? (float)srcRect->x : 0.0f;
   float texY = srcRect ? (float)srcRect->y : 0.0f;
   float texW = srcRect ? (float)srcRect->w : (float)texInfo->width;
   float texH = srcRect ? (float)srcRect->h : (float)texInfo->height;

   float srcX = destRect ? (float)destRect->x : 0.0f;
   float srcY = destRect ? (float)destRect->y : 0.0f;
   float srcW = destRect ? (float)destRect->w : ctx->srcWidth;
   float srcH = destRect ? (float)destRect->h : ctx->srcHeight;

   ctx->DrawImage(ctx, texture, tintR, tintG, tintB, alpha,
                  texX, texY, texW, texH, pivotX, pivotY, rotation,
                  srcX, srcY, srcW, srcH);
}

}
