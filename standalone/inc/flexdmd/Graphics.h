#pragma once

class Graphics {
public:
   Graphics(int width, int height);
   ~Graphics();

   void TranslateTransform(int x, int y);
   void Clear(OLE_COLOR color);
   void SetClip(int x, int y, int width, int height);
   void ResetClip();
   void DrawImage(SDL_Surface* pSurface, int x, int y, int width, int height);
   void DrawImage(SDL_Surface* pImage, int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh);
   void FillRectangle(OLE_COLOR color, int x, int y, int width, int height);
   void FillRectangleAlpha(OLE_COLOR color, int alpha, int x, int y, int width, int height);
   void* GetPixels() { return m_pSurface->pixels; }

private:
   SDL_Surface* m_pSurface;

   int m_translateX;
   int m_translateY;
};