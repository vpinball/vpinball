#pragma once

/*
 * Portions of this code was derived from SDL_gfx, BBCSDL, and Wine:
 *
 * https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx
 * https://github.com/rtrussell/BBCSDL/blob/master/src/SDL2_gfxPrimitives.c
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
   void FillPath(GraphicsPath* pPath);
   void FillPath(Brush* pBrush, GraphicsPath* pPath);
   void DrawPath(GraphicsPath* pPath);
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
   struct GradientData {
      float cx = 0.0f;
      float cy = 0.0f;
      float uniformInner = 1.0f;
      std::vector<SDL_FPoint> outer;
      std::vector<SDL_FPoint> inner;
      uint32_t centerArgb = 0;
      uint32_t surroundArgb = 0;
   };

   void InitGradientData(GradientData& gradientData, PathGradientBrush* pBrush);
   static void GradientColorAt(const GradientData& gradientData, float x, float y, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);
   void FillPolygon(const std::vector<SDL_FPoint>& points, const GradientData* pGradientData = nullptr);
   void DrawPolygonOutline(const std::vector<SDL_FPoint>& points);
   void DrawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
   void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
   void SetPixelBlended(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
   void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image);

   VPXPluginAPI* m_vpxApi = nullptr;
   VPXTexture m_texture = nullptr;
   uint8_t* m_pixelBuffer = nullptr;
   int m_width = 0;
   int m_height = 0;
   int m_bufferSize = 0;
   int m_translateX = 0;
   int m_translateY = 0;
   Matrix m_pModelMatrix;
   uint32_t m_color = RGB(0, 0, 0);
   uint8_t m_alpha = 255;
   bool m_needsTextureUpdate = true;
};

}
