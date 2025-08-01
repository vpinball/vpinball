#pragma once

/*
 * Portions of this code was derived from SDL_gfx and BBCSDL:
 *
 * https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx
 * https://github.com/rtrussell/BBCSDL/blob/master/src/SDL2_gfxPrimitives.c
 */

#include "common.h"
#include "Matrix.h"
#include "GraphicsPath.h"

namespace B2SLegacy {

class VPXGraphics final {
public:
   VPXGraphics(VPXPluginAPI* vpxApi, int width, int height);
   ~VPXGraphics();

   void Clear();
   void SetColor(uint32_t color, uint8_t alpha = 255);
   void FillPath(GraphicsPath* pPath);
   void DrawPath(GraphicsPath* pPath);
   void FillRectangle(const SDL_Rect& rect);
   void TranslateTransform(int x, int y);
   void ResetTransform();
   Matrix* GetTransform() const { return m_pModelMatrix; }
   void SetTransform(Matrix* pModelMatrix);
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
   void FillPolygon(const std::vector<SDL_FPoint>& points);
   void DrawPolygonOutline(const std::vector<SDL_FPoint>& points);
   void DrawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
   void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
   void SetPixelBlended(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
   void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const uint8_t* image);

   VPXPluginAPI* m_vpxApi;
   VPXTexture m_texture;
   uint8_t* m_pixelBuffer;
   int m_width;
   int m_height;
   int m_bufferSize;
   int m_translateX;
   int m_translateY;
   Matrix* m_pModelMatrix;
   uint32_t m_color;
   uint8_t m_alpha;
   bool m_needsTextureUpdate;
};

}