#pragma once

/*
 * Portions of this code was derived from SDL_gfx and BBCSDL:
 *
 * https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx
 * https://github.com/rtrussell/BBCSDL/blob/master/src/SDL2_gfxPrimitives.c
 */

#include "Matrix.h"
#include "GraphicsPath.h"

namespace VP {

class RendererGraphics final {
public:
   RendererGraphics(SDL_Renderer* pRenderer);
   ~RendererGraphics();

   SDL_Renderer* GetRenderer() const { return m_pRenderer; }

   void Clear();
   void SetColor(OLE_COLOR color, UINT8 alpha = 255);
   SDL_BlendMode GetBlendMode();
   void SetBlendMode(SDL_BlendMode blendMode);
   void DrawPath(GraphicsPath* pPath);
   void FillPath(GraphicsPath* pPath);
   void DrawImage(SDL_Surface* pImage, SDL_Rect* pSrcRect, SDL_Rect* pDstRect);
   void DrawTexture(SDL_Texture* pTexture, SDL_Rect* pSrcRect, SDL_Rect* pDstRect);
   void FillRectangle(const SDL_Rect& rect);
   void TranslateTransform(int x, int y);
   void ResetTransform();
   Matrix* GetTransform() const { return m_pModelMatrix; }
   void SetTransform(Matrix* pModelMatrix);
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }

private:
   SDL_Renderer* m_pRenderer;

   int m_width;
   int m_height;
   int m_translateX;
   int m_translateY;
   VP::Matrix* m_pModelMatrix;
   OLE_COLOR m_color;
   UINT8 m_alpha;
};

}
