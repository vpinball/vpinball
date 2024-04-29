/*
 * This code was derived from notes at:
 *
 * https://nailbuster.com/pup/PUPDMD_Reference_v13.zip
 */

#include "core/stdafx.h"

#include "PUPLabel.h"

PUPLabel::PUPLabel(TTF_Font* pFont, float size, LONG color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos, float yPos, bool visible, int pagenum)
{
   m_pFont = pFont;
   m_size = size;
   m_color = color;
   m_angle = angle;
   m_xAlign = xAlign;
   m_yAlign = yAlign;
   m_xPos = xPos;
   m_yPos = yPos;
   m_visible = visible;
   m_pagenum = pagenum;
   m_pTexture = NULL;
}

PUPLabel::~PUPLabel()
{
   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);
}

void PUPLabel::Render(SDL_Renderer* pRenderer, SDL_Rect& rect, int pagenum)
{
   if (!m_visible || pagenum != m_pagenum || m_szText.empty())
      return;

   if (m_pTexture) {
      SDL_DestroyTexture(m_pTexture);
      m_pTexture = NULL;
   }

   int height = (int)((m_size / 100.0) * rect.h);
   TTF_SetFontSize(m_pFont, height);

   SDL_Color color = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color) };
   SDL_Surface* pTextSurface = TTF_RenderUTF8_Blended(m_pFont, m_szText.c_str(), color);

   if (pTextSurface) {
      m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pTextSurface);
      if (m_pTexture) {
         int x = rect.x;
         int xPos = (int)((m_xPos / 100.0) * rect.w);
         switch (m_xAlign) {
            case PUP_LABEL_XALIGN_LEFT:
                x += xPos;
                break;
            case PUP_LABEL_XALIGN_CENTER:
                x += ((rect.w - pTextSurface->w) / 2);
                break;
            case PUP_LABEL_XALIGN_RIGHT:
                x += (rect.w - pTextSurface->w - xPos);
                break;
         }
         int y = rect.y;
         int yPos = (int)((m_yPos / 100.0) * rect.h);
         switch (m_yAlign) {
            case PUP_LABEL_YALIGN_TOP:
                y += yPos;
                break;
            case PUP_LABEL_YALIGN_CENTER:
                y += ((rect.h - pTextSurface->h) / 2);
                break;
            case PUP_LABEL_YALIGN_BOTTOM:
                y += (rect.h - pTextSurface->h - yPos);
                break;
         }
         SDL_Rect dest = { x, y, pTextSurface->w, pTextSurface->h };
         SDL_RenderCopy(pRenderer, m_pTexture, NULL, &dest);
      }
   }
   else {
      PLOGW.printf("Unable to render label: %s", m_szText.c_str());
   }
}
