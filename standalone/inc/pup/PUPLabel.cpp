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
   m_shadowColor = 0;
   m_shadowState = 0;
   m_xoffset = 0;
   m_yoffset = 0;
   m_outline = false;
   m_dirty = true;

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

   if (m_dirty)
      UpdateLabelTexture(pRenderer, rect);

   if (!m_pTexture) {
      PLOGW.printf("Unable to render label: %s", m_szText.c_str());
      return;
   }

   int x = rect.x;
   int xPos = (int)((m_xPos / 100.0) * rect.w);
   switch (m_xAlign) {
      case PUP_LABEL_XALIGN_LEFT:
         x += xPos;
         break;
      case PUP_LABEL_XALIGN_CENTER:
         x += ((rect.w - m_textureWidth) / 2);
         break;
      case PUP_LABEL_XALIGN_RIGHT:
         x += (xPos == 0 ? (rect.w - m_textureWidth) : (xPos - m_textureWidth));
         break;
   }
   int y = rect.y;
   int yPos = (int)((m_yPos / 100.0) * rect.h);
   switch (m_yAlign) {
      case PUP_LABEL_YALIGN_TOP:
         y += yPos;
         break;
      case PUP_LABEL_YALIGN_CENTER:
         y += ((rect.h - m_textureHeight) / 2);
         break;
      case PUP_LABEL_YALIGN_BOTTOM:
         y += (yPos == 0 ? (rect.h - m_textureHeight) : (yPos - m_textureHeight));
         break;
   }
   SDL_Rect dest = { x, y, m_textureWidth, m_textureHeight };
   SDL_RenderCopy(pRenderer, m_pTexture, NULL, &dest);
}

void PUPLabel::UpdateLabelTexture(SDL_Renderer* pRenderer, SDL_Rect& rect)
{
   if (m_pTexture) {
      SDL_DestroyTexture(m_pTexture);
      m_pTexture = NULL;
   }

   int height = (int)((m_size / 100.0) * rect.h);
   TTF_SetFontSize(m_pFont, height);
   TTF_SetFontOutline(m_pFont, 0);

   SDL_Color textColor = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color) };
   SDL_Surface* pTextSurface = TTF_RenderUTF8_Blended(m_pFont, m_szText.c_str(), textColor);
   if (!pTextSurface)
      return;

   if (m_shadowState) {
      SDL_Color shadowColor = { GetRValue(m_shadowColor), GetGValue(m_shadowColor), GetBValue(m_shadowColor) };
      SDL_Surface* pShadowSurface = TTF_RenderUTF8_Blended(m_pFont, m_szText.c_str(), shadowColor);
      if (!pShadowSurface) {
         delete pTextSurface;
         return;
      }

      int xoffset = (int)abs(m_xoffset);
      int yoffset = (int)abs(m_yoffset);

      SDL_Surface* pMergedSurface = SDL_CreateRGBSurfaceWithFormat(0, pTextSurface->w + xoffset, pTextSurface->h + yoffset, 32, SDL_PIXELFORMAT_RGBA32);
      if (pMergedSurface) {
         if (!m_outline) {
            SDL_Rect shadowRect = { (m_xoffset < 0) ? 0 : xoffset, (m_yoffset < 0) ? 0 : yoffset, pShadowSurface->w, pShadowSurface->h };
            SDL_BlitSurface(pShadowSurface, NULL, pMergedSurface, &shadowRect);

            SDL_Rect textRect = { (m_xoffset > 0) ? 0 : xoffset, (m_yoffset > 0) ? 0 : yoffset, pTextSurface->w, pTextSurface->h };
            SDL_BlitSurface(pTextSurface, NULL, pMergedSurface, &textRect);
         }
         else {
            SDL_Rect shadowRects[8] = {
               { 0,           0,           pShadowSurface->w, pShadowSurface->h },
               { xoffset / 2, 0,           pShadowSurface->w, pShadowSurface->h },
               { xoffset,     0,           pShadowSurface->w, pShadowSurface->h },

               { 0,           yoffset / 2, pShadowSurface->w, pShadowSurface->h },
               { xoffset,     yoffset / 2, pShadowSurface->w, pShadowSurface->h },

               { 0,           yoffset,     pShadowSurface->w, pShadowSurface->h },
               { xoffset / 2, yoffset,     pShadowSurface->w, pShadowSurface->h },
               { xoffset,     yoffset,     pShadowSurface->w, pShadowSurface->h }
            };

            for (int i = 0; i < 8; ++i)
               SDL_BlitSurface(pShadowSurface, NULL, pMergedSurface, &shadowRects[i]);

            SDL_Rect textRect = { xoffset / 2, yoffset / 2, pTextSurface->w, pTextSurface->h };
            SDL_BlitSurface(pTextSurface, NULL, pMergedSurface, &textRect);
         }

         if (pMergedSurface) {
            m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pMergedSurface);
            m_textureWidth = pMergedSurface->w;
            m_textureHeight = pMergedSurface->h;

            SDL_FreeSurface(pMergedSurface);
         }
      }

      SDL_FreeSurface(pShadowSurface);
   }
   else {
      m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pTextSurface);
      m_textureWidth = pTextSurface->w;
      m_textureHeight = pTextSurface->h;
   }

   SDL_FreeSurface(pTextSurface);

   if (m_pTexture)
      m_dirty = false;
}