#pragma once

/*
 * Portions of this code was derived from Wine:
 *
 * https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/gdiplus/graphics.c
 * https://gitlab.winehq.org/wine/wine/-/blob/master/dlls/gdiplus/gdiplus_private.h
 */

#include "common.h"
#include "Matrix.h"
#include "GraphicsPath.h"
#include "Brush.h"

namespace B2SLegacy {

class VPXGraphics final {
public:
   VPXGraphics(VPXPluginAPI* vpxApi, int width, int height);
   ~VPXGraphics();

   void Clear();
   void SetColor(uint32_t color, uint8_t alpha = 255);
   void FillPath(const GraphicsPath& pPath);
   void FillPath(const Brush& pBrush, const GraphicsPath& pPath);
   void DrawPath(const GraphicsPath& pPath);
   void FillRectangle(const SDL_Rect& rect);
   void TranslateTransform(int x, int y);
   void ResetTransform();
   const Matrix& GetTransform() const { return m_pModelMatrix; }
   void SetTransform(const Matrix& pModelMatrix);
   VPXTexture GetTexture();
   void SyncTexture();
   void DrawToContext(VPXRenderContext2D* ctx, int left, int top);
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }

   static VPXTexture CreateTexture(VPXPluginAPI* vpxApi, SDL_Surface* surf);
   static VPXTexture DuplicateTexture(VPXPluginAPI* vpxApi, VPXTexture sourceTexture);
   static SDL_Surface* VPXTextureToSDLSurface(VPXPluginAPI* vpxApi, VPXTexture texture);
   static VPXTexture SDLSurfaceToVPXTexture(VPXPluginAPI* vpxApi, SDL_Surface* surface);
   static void DrawImage(VPXPluginAPI* vpxApi, VPXRenderContext2D* ctx, VPXTexture texture, SDL_Rect* srcRect, SDL_Rect* destRect);

private:
   // GDI+ fans a path gradient into one triangle per edge (start, end, center).
   // For each edge the per-pixel barycentric weights and the radial distance are
   // affine in (x, y), so we precompute their coefficients once per fill instead
   // of recomputing them for every pixel
   struct GradientEdge {
      float wcA, wcB, wcC;   // wCenter = wcA*x + wcB*y + wcC
      float wsA, wsB, wsC;   // wStart  = wsA*x + wsB*y + wsC
      float dA,  dB,  dC;    // raw radial distance = dA*x + dB*y + dC
      float dInnerS, dInnerE;// inner-boundary distances at the edge endpoints
      bool  denomOk;         // false if the triangle is degenerate (skip)
      bool  cdOk;            // false if center distance is ~0 (use defaults)
   };

   struct GradientData {
      float uniformInner = 1.0f;
      bool  hasInner = false;
      std::vector<GradientEdge> edges;
      uint32_t blendLut[256] = {}; // surround->center blend, indexed by position*255
      //!! The 256-entry build only pays off past ~256 covered pixels, so cache it across fills,
      // rebuild only when the brush colors actually change
      uint32_t lutSurround = 0;
      uint32_t lutCenter = 0;
      bool lutValid = false;
   };

   // A non-horizontal polygon edge in bbox-local coordinates, normalized so that y0 < y1.
   // 'dir' carries the original winding direction (+1 if the edge ran downward, -1 if upward) for the non-zero fill rule
   struct RasterEdge {
      float x0;   // x at y0 (the top vertex)
      float y0;   // top y
      float y1;   // bottom y
      float dxdy; // dx/dy slope
      float dir;  // +1 downward, -1 upward
   };

   // The model transform with m_translateX/Y folded in, so callers transform points in one step instead of transform + separate add
   Matrix EffectiveTransform() const
   {
      Matrix m = m_pModelMatrix;
      m.TranslatePost((float)m_translateX, (float)m_translateY);
      return m;
   }

   void InitGradientData(GradientData& gradientData, const PathGradientBrush* const __restrict pBrush);
   static void GradientColorAt(const GradientData& gradientData, float x, float y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
   template <bool Gradient>
   void FillPolygon(const std::vector<SDL_FPoint>* const __restrict points, const GradientData* const __restrict pGradientData);
   void DrawPolygonOutline(const std::vector<SDL_FPoint>& points);
   void DrawLine(int x1, int y1, int x2, int y2, uint32_t rgba);
   void SetPixel(int x, int y, uint32_t rgba);
   void SetPixelBlended(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
   void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image);

   VPXPluginAPI* m_vpxApi = nullptr;
   VPXTexture m_texture = nullptr;
   std::vector<uint32_t> m_pixelBuffer; // one RGBA8 pixel per entry (R | G<<8 | B<<16 | A<<24)
   int m_width = 0;
   int m_height = 0;
   int m_translateX = 0;
   int m_translateY = 0;
   Matrix m_pModelMatrix;
   uint32_t m_color = RGB(0, 0, 0);
   uint8_t m_alpha = 255;
   bool m_needsTextureUpdate = true;

   // Reusable scratch for FillPolygon/gradients, grown on demand, never freed per call
   std::vector<SDL_FPoint> m_rasterVerts; // transformed vertices (surface space)
   std::vector<RasterEdge> m_rasterEdges; // active non-horizontal edges
   std::vector<float> m_scanAccum;        // one scanline of signed-area deltas (w + 1)
   GradientData m_gradientData;           // per-fill gradient state (reused; edge buffer kept)
};

}
