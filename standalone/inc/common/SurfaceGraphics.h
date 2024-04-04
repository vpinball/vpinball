#pragma once

namespace VP {

class SurfaceGraphics final {
public:
   SurfaceGraphics(SDL_Surface* pSurface);
   ~SurfaceGraphics();

   void Clear();
   void SetColor(OLE_COLOR color, UINT8 alpha = 255);
   void DrawImage(SDL_Surface* pImage, SDL_Rect* pSrcRect, SDL_Rect* pDstRect);
   void FillRectangle(const SDL_Rect& rect);
   void SetClip(const SDL_Rect& rect);
   void ResetClip();
   void TranslateTransform(int x, int y);
   void ResetTransform();
   int GetWidth() const { return m_width; }
   int GetHeight() const { return m_height; }

private:
   SDL_Surface* m_pSurface;

   int m_width;
   int m_height;
   int m_translateX;
   int m_translateY;
   OLE_COLOR m_color;
   UINT8 m_alpha;
};

}
