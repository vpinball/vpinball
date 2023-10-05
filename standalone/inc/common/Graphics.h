#pragma once

/*
 * Portions of this code was derived from Mono:
 *
 * https://github.com/mono/sysdrawing-coregraphics/blob/main/System.Drawing/Graphics.cs
 */

#include "Matrix.h"
#include "GraphicsPath.h"

namespace VP {

class Graphics {
public:
   Graphics(int x, int y, bool alpha = false);
   ~Graphics();

   void TranslateTransform(int x, int y);
   void ResetTransform();
   Matrix* GetTransform() const { return m_pModelMatrix; }
   void SetTransform(Matrix* pModelMatrix);
   void SetClip(int x, int y, int width, int height);
   void ResetClip();
   void SetColor(OLE_COLOR color, UINT8 alpha = 0xFF);
   void DrawPath(GraphicsPath* pGraphicsPath);
   void FillPath(GraphicsPath* pPath);
   void Clear();
   void DrawImage(SDL_Surface* pImage, SDL_Rect* pSrcRect, SDL_Rect* pDstRect);
   void DrawImage(SDL_Surface* pImage, int x, int y, int width, int height);
   void DrawImage(SDL_Surface* pImage, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
   void FillRectangle(int x, int y, int width, int height);
   void FillRectangleAlpha(OLE_COLOR color, int alpha, int x, int y, int width, int height);
   SDL_Surface* GetSurface() { return m_pSurface; }
   UINT8* GetPixels() { return (UINT8*)m_pSurface->pixels; }
   SDL_Texture* ToTexture(SDL_Renderer* pRenderer);
   void UpdateTexture(SDL_Texture* pTexture);
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }

private:
   void MoveTo(float x, float y);
   void LineTo(float x, float y);
   void PlotPath(GraphicsPath* pPath);

   int m_width;
   int m_height;
   float m_cx;
   float m_cy;
   int m_translateX;
   int m_translateY;
   VP::Matrix* m_pModelMatrix;
   SDL_Surface* m_pSurface;
   UINT32 m_color;
};

}