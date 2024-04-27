#pragma once

#include <SDL2/SDL_ttf.h>

class PUPLabel
{
public:
   PUPLabel(TTF_Font* face, float h, LONG c, bool vis);
   ~PUPLabel();

   void Render(SDL_Renderer* renderer, SDL_Rect& rect);

   TTF_Font* m_pFont;
   float m_height;
   LONG m_color;
   bool m_visible;
   float m_x;
   float m_y;
   int m_xalign;
   int m_yalign;
   string m_szText;

private:
   SDL_Texture* m_pTexture;
};