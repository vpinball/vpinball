/*
 * Portions of this code was derived from Wine:
 *
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

namespace B2SLegacy {

typedef uint32_t ARGB;
typedef int INT;

// Blend surround->center at an already-quantized position (0..255)
// (if necessary, could use similar trick/approximation(s) as blend_into below)
static inline ARGB blend_colors_pos(ARGB start, ARGB end, INT pos)
{
    const INT start_a = ((start >> 24) & 0xff) * (pos ^ 0xff);
    const INT end_a = ((end >> 24) & 0xff) * pos;
    const INT final_a = start_a + end_a;

    if (final_a < 0xff) return 0;

    return (final_a / 0xff) << 24 |
        ((((start >> 16) & 0xff) * start_a + (((end >> 16) & 0xff) * end_a)) / final_a) << 16 |
        ((((start >> 8) & 0xff) * start_a + (((end >> 8) & 0xff) * end_a)) / final_a) << 8 |
        (((start & 0xff) * start_a + ((end & 0xff) * end_a)) / final_a);
}

// Source-over blend of (r,g,b,a) onto an already-stored buffer pixel, in place.
// Two-lane (R/B, G/A) integer blend, /255 is approximated by >>8 (<=1 LSB low; a==255 path stays exact) though
static inline void blend_into(uint32_t& pixel, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   if (a == 0)
      return;
   const uint32_t src = (uint32_t)r | ((uint32_t)g << 8) | ((uint32_t)b << 16) | 0xff000000u;
   if (a == 255) {
      pixel = src;
      return;
   }
   const uint32_t dst = pixel;
   const uint32_t alpha = a;
   const uint32_t invAlpha = 255u - a;
   const uint32_t rb = ( src       & 0x00FF00FFu) * alpha + ( dst       & 0x00FF00FFu) * invAlpha;
   const uint32_t ga = ((src >> 8) & 0x00FF00FFu) * alpha + ((dst >> 8) & 0x00FF00FFu) * invAlpha;
   pixel = ((rb >> 8) & 0x00FF00FFu) | (ga & 0xFF00FF00u);
}

VPXGraphics::VPXGraphics(VPXPluginAPI* vpxApi, int width, int height)
   : m_vpxApi(vpxApi),
     m_pixelBuffer((size_t)width * height, 0),
     m_width(width),
     m_height(height)
{
}

VPXGraphics::~VPXGraphics()
{
   if (m_texture)
      m_vpxApi->DeleteTexture(m_texture);
}

void VPXGraphics::Clear()
{
   std::fill(m_pixelBuffer.begin(), m_pixelBuffer.end(), 0u);
   m_needsTextureUpdate = true;
}

void VPXGraphics::SetColor(uint32_t color, uint8_t alpha)
{
   m_color = color;
   m_alpha = alpha;
}

void VPXGraphics::FillPath(const GraphicsPath& pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath.GetPoints();
   if (pPoints->size() < 3)
      return;

   FillPolygon<false>(pPoints, nullptr);
   m_needsTextureUpdate = true;
}

void VPXGraphics::FillPath(const Brush& pBrush, const GraphicsPath& pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath.GetPoints();
   if (pPoints->size() < 3)
      return;

   if (pBrush.GetBrushType() == BrushType_PathGradient) {
      const PathGradientBrush* const pGradientBrush = static_cast<const PathGradientBrush*>(&pBrush);
      if (pGradientBrush->GetPath()->GetPoints()->size() < 3)
         return;
      //!! Note: if the brush's gradient path equals the fill path, the vertices get transformed twice:
      // once here for the gradient geometry, and once in FillPolygon for the fill, BUT the API allows the two paths to differ
      InitGradientData(m_gradientData, pGradientBrush);
      FillPolygon<true>(pPoints, &m_gradientData);
   }
   else {
      const SolidBrush* const pSolidBrush = static_cast<const SolidBrush*>(&pBrush);
      SetColor(pSolidBrush->GetColor(), pSolidBrush->GetAlpha());
      FillPolygon<false>(pPoints, nullptr);
   }
   m_needsTextureUpdate = true;
}

void VPXGraphics::InitGradientData(GradientData& gradientData, const PathGradientBrush* const __restrict pBrush)
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

   const Matrix xform = EffectiveTransform();
   std::vector<SDL_FPoint> outer(count);
   for (size_t i = 0; i < count; i++) {
      const auto& point = (*pPoints)[i];
      float x = point.x;
      float y = point.y;
      xform.TransformPoint(x, y);
      outer[i] = { x, y };
   }

   // FocusScales push the solid center color outward: uniform scales give a
   // constant inner boundary, anisotropic ones keep a scaled inner polygon
   // (gradientData is reused across fills, so reset uniformInner unconditionally)
   const SDL_FPoint& focusScales = pBrush->GetFocusScales();
   std::vector<SDL_FPoint> inner((focusScales.x == focusScales.y) ? 0 : count);
   gradientData.uniformInner = 1.0f;
   if (focusScales.x == focusScales.y)
      gradientData.uniformInner -= clamp(focusScales.x, 0.0f, 0.999f);
   else {
      for (size_t i = 0; i < count; i++) {
         const auto& point = (*pPoints)[i];
         float x = centerX + (point.x - centerX) * focusScales.x;
         float y = centerY + (point.y - centerY) * focusScales.y;
         xform.TransformPoint(x, y);
         inner[i] = { x, y };
      }
   }
   gradientData.hasInner = !inner.empty();

   xform.TransformPoint(centerX, centerY);
   const float cx = centerX;
   const float cy = centerY;

   const uint32_t centerArgb   = (pBrush->GetCenterColor()   & 0x00FFFFFFu) | ((uint32_t)pBrush->GetCenterAlpha()   << 24);
   const uint32_t surroundArgb = (pBrush->GetSurroundColor() & 0x00FFFFFFu) | ((uint32_t)pBrush->GetSurroundAlpha() << 24);

   // Per-pixel blend_colors LUT: tabulate the surround->center blend once. The table
   // depends only on the two colors, so reuse it across fills until these change
   if (!gradientData.lutValid || surroundArgb != gradientData.lutSurround || centerArgb != gradientData.lutCenter) {
      for (int k = 0; k < 256; k++)
         gradientData.blendLut[k] = blend_colors_pos(surroundArgb, centerArgb, k);
      gradientData.lutSurround = surroundArgb;
      gradientData.lutCenter = centerArgb;
      gradientData.lutValid = true;
   }

   // Precompute each fan triangle's affine coefficients (see GradientEdge)
   gradientData.edges.resize(count);
   for (size_t i = 0; i < count; i++) {
      const size_t j = (i + 1) == count ? 0 : (i + 1);
      const float sx = outer[i].x, sy = outer[i].y;
      const float ex = outer[j].x, ey = outer[j].y;
      GradientEdge& e = gradientData.edges[i];

      const float denom = (sy - ey) * (cx - ex) + (ex - sx) * (cy - ey);
      e.denomOk = fabsf(denom) >= 1e-6f;
      if (e.denomOk) {
         const float inv = 1.0f / denom;
         e.wcA = (sy - ey) * inv;
         e.wcB = (ex - sx) * inv;
         e.wcC = (-(sy - ey) * ex - (ex - sx) * ey) * inv;
         e.wsA = (ey - cy) * inv;
         e.wsB = (cx - ex) * inv;
         e.wsC = (-(ey - cy) * ex - (cx - ex) * ey) * inv;
      }
      else {
         e.wcA = e.wcB = e.wcC = e.wsA = e.wsB = e.wsC = 0.0f;
      }

      const float cd = (ey - sy) * (sx - cx) + (ex - sx) * (cy - sy);
      e.cdOk = fabsf(cd) >= 1e-6f;
      e.dA = e.dB = e.dC = 0.0f;
      e.dInnerS = e.dInnerE = 0.0f;
      if (e.cdOk) {
         const float cinv = 1.0f / cd;
         e.dA = -(ey - sy) * cinv;
         e.dB =  (ex - sx) * cinv;
         e.dC =  ((ey - sy) * sx - (ex - sx) * sy) * cinv;
         if (gradientData.hasInner) {
            const float isx = inner[i].x, isy = inner[i].y;
            const float iex = inner[j].x, iey = inner[j].y;
            e.dInnerS = ((ey - sy) * (sx - isx) + (ex - sx) * (isy - sy)) * cinv;
            e.dInnerE = ((ey - sy) * (sx - iex) + (ex - sx) * (iey - sy)) * cinv;
         }
      }
   }
}

void VPXGraphics::GradientColorAt(const GradientData& gradientData, float x, float y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a)
{
   // Pick the containing fan triangle (highest min-barycentric score) and read
   // its radial distance, all from the precomputed affine coefficients
   float distance = 1.0f;
   float innerDistance = gradientData.uniformInner;
   float bestScore = -FLT_MAX;
   const size_t count = gradientData.edges.size();
   for (size_t i = 0; i < count; i++) {
      const GradientEdge& e = gradientData.edges[i];
      if (!e.denomOk)
         continue;
      const float wCenter = e.wcA * x + e.wcB * y + e.wcC;
      const float wStart  = e.wsA * x + e.wsB * y + e.wsC;
      const float wEnd = 1.0f - wCenter - wStart;
      const float score = std::min(std::min(wCenter, wStart), wEnd);
      if (score <= bestScore)
         continue;
      bestScore = score;

      if (!e.cdOk) {
         distance = 1.0f;
         innerDistance = gradientData.uniformInner;
         continue;
      }
      distance = clamp(e.dA * x + e.dB * y + e.dC, 0.0f, 1.0f);

      // FocusScales (the bulb glow) moves the full-center-color boundary outward
      if (!gradientData.hasInner)
         innerDistance = gradientData.uniformInner;
      else {
         innerDistance = e.dInnerS;
         if (wStart + wEnd > 1e-6f)
            innerDistance += (e.dInnerE - e.dInnerS) * clamp(wEnd / (wStart + wEnd), 0.0f, 1.0f);
      }
   }

   const int idx = (innerDistance > 1e-6f) ? static_cast<int>(clamp((distance / innerDistance) * 255.0f + 0.5f, 0.0f, 255.0f)) : 255;
   const ARGB argb = gradientData.blendLut[idx];
   r = (uint8_t)(argb & 0xff);
   g = (uint8_t)((argb >> 8) & 0xff);
   b = (uint8_t)((argb >> 16) & 0xff);
   a = (uint8_t)((argb >> 24) /*& 0xff*/);
}

void VPXGraphics::DrawPath(const GraphicsPath& pPath)
{
   const std::vector<SDL_FPoint>* const pPoints = pPath.GetPoints();
   if (pPoints->size() < 2)
      return;

   DrawPolygonOutline(*pPoints);
   m_needsTextureUpdate = true;
}

// Accumulate one edge sub-segment (clipped to a single scanline) into a row of
// signed-area deltas. (xa, xb) are the edge's x at the top and bottom of the
// scanline slice and 'd' is the signed vertical coverage of that slice
// (height * winding direction). After a left-to-right prefix sum of the row,
// cell C holds the signed coverage of pixel C: the convention is that the area
// to the *right* of the edge is "inside", so the per-cell partial plus a carry
// into the next cell sum to 'd' and the prefix sum propagates full coverage
// rightward. 'acc' must have (w + 1) cells; column w is the carry sink
static inline void AccumulateSpan(float* const __restrict acc, const int w, float xa, float xb, const float d)
{
   float xL = xa < xb ? xa : xb;
   const float xR = xa < xb ? xb : xa;
   const float origDx = xR - xL;

   // Vertical (or sub-cell-wide) edge: all coverage lands in one column.
   if (origDx <= 1e-6f) {
      if (xL < 0.0f) { acc[0] += d; return; } // edge left of view: fully inside
      if (xL >= (float)w) return;             // edge right of view: nothing visible
      const float fc = floorf(xL);            // assume xL >= 0
      const int c = (int)fc;
      const float f = xL - fc;                // sub-cell position
      acc[c]     += d * (1.0f - f);
      acc[c + 1] += d * f;
      return;
   }

   const float invDx = 1.0f / origDx;

   // Portion left of the viewport contributes full coverage from column 0
   if (xL < 0.0f) {
      const float seg = std::min(xR, 0.0f) - xL;
      acc[0] += d * (seg * invDx);
      if (xR <= 0.0f) return;
      xL = 0.0f;
   }
   if (xL >= (float)w) return;
   const float xRc = std::min(xR, (float)w); // portion right of view covers no visible cell

   // Walk the integer columns the (clipped) edge spans, splitting 'd' in proportion to each piece's x-length and depositing its trapezoidal area
   for (int c = (int)xL; (float)c < xRc; c++) {
      const float pa = std::max(xL, (float)c);
      const float pb = std::min(xRc, (float)c + 1.f);
      const float len = pb - pa;
      if (len <= 0.0f)
         continue;
      const float dp = d * (len * invDx);
      const float f = 0.5f * (pa + pb) - (float)c; // mean sub-cell position, in [0,1]
      acc[c]     += dp * (1.0f - f);
      acc[c + 1] += dp * f;
   }
}

template <bool Gradient>
void VPXGraphics::FillPolygon(const std::vector<SDL_FPoint>* const __restrict points, [[maybe_unused]] const GradientData* const __restrict pGradientData)
{
   const int ps = static_cast<int>(points->size());
   if (ps < 3)
      return;

   // Transform vertices into surface space and track the bounding box
   const Matrix xform = EffectiveTransform();
   m_rasterVerts.clear();
   m_rasterVerts.reserve(ps);
   float minXs = FLT_MAX, minYs = FLT_MAX, maxXs = -FLT_MAX, maxYs = -FLT_MAX;
   for (const auto& point : *points) {
      float x = point.x;
      float y = point.y;
      xform.TransformPoint(x, y);
      m_rasterVerts.push_back({ x, y });
      minXs = std::min(minXs, x); maxXs = std::max(maxXs, x);
      minYs = std::min(minYs, y); maxYs = std::max(maxYs, y);
   }

   // Integer bounding box clipped to the surface; everything below is local to it
   const int ix0 = std::max(0, (int)std::floor(minXs));
   const int iy0 = std::max(0, (int)std::floor(minYs));
   const int ix1 = std::min(m_width,  (int)std::ceil(maxXs));
   const int iy1 = std::min(m_height, (int)std::ceil(maxYs));
   const int w = ix1 - ix0;
   const int h = iy1 - iy0;
   if (w <= 0 || h <= 0)
      return;

   // Build the non-horizontal edge list in bbox-local coordinates
   const float ix0f = (float)ix0, iy0f = (float)iy0;
   m_rasterEdges.clear();
   for (int i = 0; i < ps; i++) {
      const SDL_FPoint& a = m_rasterVerts[i];
      const SDL_FPoint& b = m_rasterVerts[(i + 1 == ps) ? 0 : (i + 1)];
      const float ax = a.x - ix0f, ay = a.y - iy0f;
      const float bx = b.x - ix0f, by = b.y - iy0f;
      if (ay == by) // horizontal edge contributes no coverage
         continue;
      RasterEdge e;
      if (ay < by) { e.dir =  1.0f; e.x0 = ax; e.y0 = ay; e.y1 = by; e.dxdy = (bx - ax) / (by - ay); }
      else         { e.dir = -1.0f; e.x0 = bx; e.y0 = by; e.y1 = ay; e.dxdy = (ax - bx) / (ay - by); }
      m_rasterEdges.push_back(e);
   }
   if (m_rasterEdges.empty())
      return;

   m_scanAccum.assign((size_t)w + 1, 0.0f);
   float* const __restrict acc = m_scanAccum.data();

   // Solid-fill color (unused by the gradient case)
   [[maybe_unused]] const uint8_t r = GetRValue(m_color);
   [[maybe_unused]] const uint8_t g = GetGValue(m_color);
   [[maybe_unused]] const uint8_t b = GetBValue(m_color);
   [[maybe_unused]] const uint8_t a = m_alpha;

   for (int row = 0; row < h; row++) {
      const float rowTop = (float)row;
      const float rowBot = rowTop + 1.f;

      // Deposit each edge's contribution for the part it covers in this scanline
      for (const RasterEdge& e : m_rasterEdges) {
         const float ytop = std::max(rowTop, e.y0);
         const float ybot = std::min(rowBot, e.y1);
         if (ybot <= ytop)
            continue;
         const float xa = e.x0 + (ytop - e.y0) * e.dxdy;
         const float xb = e.x0 + (ybot - e.y0) * e.dxdy;
         AccumulateSpan(acc, w, xa, xb, (ybot - ytop) * e.dir);
      }

      // Prefix sum turns the area deltas into per-pixel coverage; clear cells as consumed, so the row buffer is zeroed for the next scanline.
      // The bbox was already clipped to the surface, so the row is fully visible/in screen bounds
      const int sy = iy0 + row;
      uint32_t* const __restrict rowPix = m_pixelBuffer.data() + (size_t)sy * m_width + ix0;
      float running = 0.0f;
      for (int col = 0; col < w; col++) {
         running += acc[col];
         acc[col] = 0.0f;
         float cov = std::fabs(running);
         if (cov <= 1.0f / 512.0f)   // below ~0.5/255: invisible
            continue;
         if (cov > 1.0f)
            cov = 1.0f;
         if constexpr (Gradient) {
            uint8_t gr, gg, gb, ga;
            GradientColorAt(*pGradientData, (float)(ix0 + col) + 0.5f, (float)sy + 0.5f, gr, gg, gb, ga);
            blend_into(rowPix[col], gr, gg, gb, (uint8_t)((float)ga * cov + 0.5f));
         }
         else
            blend_into (rowPix[col], r, g, b, (cov >= 0.996f) ? a : (uint8_t)((float)a * cov + 0.5f)); //!! magic value, to snap to 1
      }
      acc[w] = 0.0f;   // clear the carry sink
   }
}

void VPXGraphics::DrawPolygonOutline(const std::vector<SDL_FPoint>& points)
{
   const int ps = static_cast<int>(points.size());
   if (ps < 2)
      return;

   const uint32_t rgba = (m_color & 0x00FFFFFFu) | ((uint32_t)m_alpha << 24);
   const Matrix xform = EffectiveTransform();

   for (int i = 0; i < ps; i++) {
      const int nextIndex = (i + 1) == ps ? 0 : (i+1);

      float x1 = points[i].x;
      float y1 = points[i].y;
      float x2 = points[nextIndex].x;
      float y2 = points[nextIndex].y;

      xform.TransformPoint(x1, y1);
      xform.TransformPoint(x2, y2);

      DrawLine((int)x1, (int)y1, (int)x2, (int)y2, rgba);
   }
}

void VPXGraphics::DrawLine(int x1, int y1, int x2, int y2, uint32_t rgba)
{
   int dx = abs(x2 - x1);
   int dy = abs(y2 - y1);
   int sx = (x1 < x2) ? 1 : -1;
   int sy = (y1 < y2) ? 1 : -1;
   int err = dx - dy;

   while (true) {
      SetPixel(x1, y1, rgba);

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

void VPXGraphics::SetPixel(int x, int y, uint32_t rgba)
{
   if (/*x < 0 ||*/ (unsigned int)x >= (unsigned int)m_width /*|| y < 0*/ || (unsigned int)y >= (unsigned int)m_height) // unsigned int test incl. <0
      return;
   m_pixelBuffer[(size_t)y * m_width + x] = rgba;
}

void VPXGraphics::SetPixelBlended(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
   if (/*x < 0 ||*/ (unsigned int)x >= (unsigned int)m_width /*|| y < 0*/ || (unsigned int)y >= (unsigned int)m_height) // unsigned int test incl. <0
      return;
   blend_into(m_pixelBuffer[(size_t)y * m_width + x], r, g, b, a);
}

VPXTexture VPXGraphics::GetTexture()
{
   if (m_needsTextureUpdate)
      SyncTexture();

   return m_texture;
}

void VPXGraphics::SyncTexture()
{
   UpdateTexture(&m_texture, m_width, m_height, VPXTEXFMT_sRGBA8, m_pixelBuffer.data());
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
   const uint32_t col = (m_color & 0x00FFFFFFu) | ((uint32_t)m_alpha << 24);

   int x1 = rect.x + m_translateX;
   int y1 = rect.y + m_translateY;
   int x2 = x1 + rect.w;
   int y2 = y1 + rect.h;

   x1 = std::max(0, x1);
   y1 = std::max(0, y1);
   x2 = std::min(m_width, x2);
   y2 = std::min(m_height, y2);

   uint32_t* const __restrict buf = m_pixelBuffer.data();

   for (int y = y1; y < y2; y++)
   {
      size_t offset = (size_t)y * m_width + x1;
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
