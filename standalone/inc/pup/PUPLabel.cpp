#include "core/stdafx.h"

#include "PUPLabel.h"

PUPLabel::PUPLabel(TTF_Font* face, float h, LONG c, bool vis)
{
   m_pFont = face;
   m_height = h;
   m_color = c;
   m_visible = vis;

   m_szText = "";
   m_x = 0;
   m_y = 0;
   m_xalign = 0;
   m_yalign = 0;
   m_pTexture = NULL;
}

PUPLabel::~PUPLabel()
{
   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);
}

void PUPLabel::Render(SDL_Renderer* pRenderer, SDL_Rect& rect)
{
   if (m_pTexture) {
      SDL_DestroyTexture(m_pTexture);
      m_pTexture = NULL;
   }

   if (!m_visible || m_szText.empty())
      return;

   TTF_SetFontSize(m_pFont, m_height);

   SDL_Color color = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color) };
   SDL_Surface* pTextSurface = TTF_RenderUTF8_Blended(m_pFont, m_szText.c_str(), color);
   if (pTextSurface) {
      m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pTextSurface);
      if (m_pTexture) {
         int w, h;
         SDL_QueryTexture(m_pTexture, NULL, NULL, &w, &h);

         SDL_Rect dest = { rect.x + (int)m_x, rect.y + (int)m_y, w, h };
         SDL_RenderCopy(pRenderer, m_pTexture, NULL, &dest);

      }
   }
   else {
      PLOGW.printf("Unable to render label: %s", m_szText.c_str());
   }
}
