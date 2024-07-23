/*
 * This code was derived from notes at:
 *
 * https://nailbuster.com/pup/PUPDMD_Reference_v13.zip
 */

#include "stdafx.h"

#include "PUPLabel.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"
#include "PUPManager.h"

#include "RSJparser/RSJparser.tcc"

PUPLabel::PUPLabel(const string& szName, const string& szFont, float size, LONG color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos, float yPos, int pagenum, bool visible)
{
   m_szName = szName;

   if (!szFont.empty()) {
      TTF_Font* pFont = PUPManager::GetInstance()->GetFont(szFont);
      if (!pFont) {
         PLOGE.printf("Font not found: label=%s, font=%s", szName.c_str(), szFont.c_str());
      }
      m_pFont = pFont;
   }
   else
      m_pFont = nullptr;

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
   m_pScreen = nullptr;
   m_pTexture = NULL;
   m_width = 0;
   m_height = 0;
   m_anigif = 0;
   m_pAnimation = NULL;
}

PUPLabel::~PUPLabel()
{
   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);

   if (m_pAnimation)
      IMG_FreeAnimation(m_pAnimation);
}

void PUPLabel::SetCaption(const string& szCaption)
{
   if (szCaption.empty() && m_type != PUP_LABEL_TYPE_TEXT)
      return;

   if (szCaption == "`u`")
      return;

   string szText = szCaption;
   szText = string_replace_all(szText, "~", "\n");
   szText = string_replace_all(szText, "\\r", "\n");

   {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_szCaption != szText) {
         m_type = PUP_LABEL_TYPE_TEXT;
         m_szPath.clear();

         const string szExt = extension_from_path(szText);
         if (szExt == "gif" || szExt == "png" || szExt == "apng" || szExt == "bmp" || szExt == "jpg") {
            std::filesystem::path fs_path(normalize_path_separators(szText));
            PUPPlaylist* pPlaylist = m_pScreen->GetPlaylist(fs_path.parent_path());
            if (pPlaylist) {
               string szPath = pPlaylist->GetPlayFilePath(fs_path.filename().string());
               if (!szPath.empty()) {
                  m_szPath = szPath;
                  m_type = (szExt == "gif") ? PUP_LABEL_TYPE_GIF : PUP_LABEL_TYPE_IMAGE;
               }
            }
         }

         m_szCaption = szText;
         m_dirty = true;
      }
   }
}

void PUPLabel::SetVisible(bool visible)
{
   std::lock_guard<std::mutex> lock(m_mutex);
   m_visible = visible;
}

void PUPLabel::SetSpecial(const string& szSpecial)
{
   PLOGD.printf("PUPLabel::SetSpecial: name=%s, caption=%s, json=%s", m_szName.c_str(), m_szCaption.c_str(), szSpecial.c_str());

   RSJresource json(szSpecial);
   switch (json["mt"s].as<int>(0)) {
      case 2: {
         if (json["zback"s].exists() && json["zback"s].as<int>() == 1)
            m_pScreen->SendLabelToBack(this);

         if (json["ztop"s].exists() && json["ztop"s].as<int>() == 1)
            m_pScreen->SendLabelToFront(this);

         {
            std::lock_guard<std::mutex> lock(m_mutex);

            if (json["size"].exists()) {
               m_size = std::stof(json["size"].as_str());
               m_dirty = true;
            }

            if (json["xpos"].exists()) {
               m_xPos = std::stof(json["xpos"].as_str());
               m_dirty = true;
            }

            if (json["ypos"].exists()) {
               m_yPos = std::stof(json["ypos"].as_str());
               m_dirty = true;
            }

            if (json["fname"].exists()) {
               string szFont = json["fname"].as_str();
               TTF_Font* pFont = PUPManager::GetInstance()->GetFont(szFont);
               if (!pFont) {
                  PLOGE.printf("Label font not found: name=%s, font=%s", m_szName.c_str(), szFont.c_str());
               }
               m_pFont = pFont;
               m_dirty = true;
            }

            if (json["fonth"].exists()) {
               m_size = std::stof(json["fonth"].as_str());
               m_dirty = true;
            }

            if (json["color"].exists()) {
               m_color = json["color"].as<int>();
               m_dirty = true;
            }

            if (json["xalign"].exists()) {
               m_xAlign = (PUP_LABEL_XALIGN)json["xalign"].as<int>();
               m_dirty = true;
            }

            if (json["yalign"].exists()) {
               m_yAlign = (PUP_LABEL_YALIGN)json["yalign"].as<int>();
               m_dirty = true;
            }

            if (json["pagenum"].exists()) {
               m_pagenum = json["pagenum"].as<int>();
               m_dirty = true;
            }

            if (json["stopani"].exists()) {
               // stop any pup animations on label/image (zoom/flash/pulse).  this is not about animated gifs
               PLOGW.printf("stopani not implemented");
               m_dirty = true;
            }

            if (json["rotate"].exists()) {
               // in tenths.  so 900 is 90 degrees. rotate support for images too.  note images must be aligned center to rotate properly(default)
               m_angle = std::stof(json["rotate"].as_str());
               m_dirty = true;
            }

            if (json["zoom"].exists()) {
               // 120 for 120% of current height, 80% etc...
               PLOGW.printf("zoom not implemented");
               m_dirty = true;
            }

            if (json["alpha"].exists()) {
               // '0-255  255=full, 0=blank
               PLOGW.printf("alpha not implemented");
               m_dirty = true;
            }

            if (json["gradstate"].exists() && json["gradcolor"].exists()) {
               // color=gradcolor, gradstate = 0 (gradstate is percent)
               PLOGW.printf("gradstate/gradcolor not implemented");
               m_dirty = true;
            }

            if (json["grayscale"].exists()) {
               // only on image objects.  will show as grayscale.  1=gray filter on 0=off normal mode
               PLOGW.printf("filter not implemented");
               m_dirty = true;
            }

            if (json["filter"].exists()) {
               // fmode 1-5 (invertRGB, invert,grayscale,invertalpha,clear),blur)
               PLOGW.printf("filter not implemented");
               m_dirty = true;
            }

            if (json["shadowcolor"].exists()) {
               m_shadowColor = json["shadowcolor"].as<int>();
               m_dirty = true;
            }

            if (json["shadowtype"].exists()) {
               // ST = 1 (Shadow), ST = 2 (Border)
               PLOGW.printf("shadowtype not implemented");
               m_dirty = true;
            }

            if (json["xoffset"].exists()) {
               m_xoffset = std::stof(json["xoffset"].as_str());
               m_dirty = true;
            }

            if (json["yoffset"].exists()) {
               m_yoffset = std::stof(json["yoffset"].as_str());
               m_dirty = true;
            }

            if (json["anigif"].exists()) {
               m_anigif = json["anigif"].as<int>();
               m_dirty = true;
            }

            if (json["width"].exists()) {
               m_width = std::stof(json["width"].as_str());
               m_dirty = true;
            }

            if (json["height"].exists()) {
               m_height = std::stof(json["height"].as_str());
               m_dirty = true;
            }

            if (json["autow"].exists()) {
               PLOGW.printf("autow not implemented");
               m_dirty = true;
            }

            if (json["autoh"].exists()) {
               PLOGW.printf("autoh not implemented");
               m_dirty = true;
            }

            if (json["shadowstate"].exists()) {
               m_shadowState = json["shadowstate"].as<int>();
               m_dirty = true;
            }

            if (json["outline"].exists()) {
               m_outline = (json["outline"].as<int>() == 1);
               m_dirty = true;
            }
         }
      }
      break;

      case 1: {
         /*
            Animate
            at = animate type (1=flashing, 2=motion)
            fq = when flashing its the frequency of flashing
            len = length in ms of animation
            fc = foreground color of text during animation
            PLOGW << "Label animation not implemented";
         */
      }
      break;

      default: break;
   }
}

void PUPLabel::Render(SDL_Renderer* pRenderer, SDL_Rect& rect, int pagenum)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   if (!m_visible || pagenum != m_pagenum || m_szCaption.empty())
      return;

   if (m_dirty) {
      if (!m_szPath.empty()) {
         if (m_pTexture) {
            SDL_DestroyTexture(m_pTexture);
            m_pTexture = NULL;
         }
         if (m_pAnimation) {
            IMG_FreeAnimation(m_pAnimation);
            m_pAnimation = nullptr;
         }
         if (m_type == PUP_LABEL_TYPE_IMAGE) {
            m_pTexture = IMG_LoadTexture(pRenderer, m_szPath.c_str());
            if (m_pTexture)
               m_dirty = false;
            else {
               PLOGE.printf("Unable to load image: %s", m_szPath.c_str());
            }
         }
         else if (m_type == PUP_LABEL_TYPE_GIF) {
            m_pAnimation = IMG_LoadAnimation(m_szPath.c_str());
            if (m_pAnimation) {
               m_frame = 0;
               m_pTexture = SDL_CreateTextureFromSurface(pRenderer, m_pAnimation->frames[m_frame]);
               m_dirty = false;
            }
            else {
               PLOGE.printf("Unable to load animation: %s", m_szPath.c_str());
            }
         }
      }
      else {
         if (m_pFont)
            UpdateLabelTexture(pRenderer, rect);
      }
   }

   if (!m_pTexture)
      return;

   if (m_pAnimation) {
      if (++m_frame >= m_pAnimation->count)
         m_frame = 0;
      SDL_UpdateTexture(m_pTexture, nullptr, m_pAnimation->frames[m_frame]->pixels, m_pAnimation->frames[m_frame]->pitch);
   }

   float width;
   float height;

   if (m_type == PUP_LABEL_TYPE_TEXT) {
      width = m_width;
      height = m_height;
   }
   else {
      width = (m_width / 100.0) * rect.w;
      height = (m_height / 100.0) * rect.h;
   }

   SDL_FRect dest = { (float)rect.x, (float)rect.y, width, height };

   if (m_xPos > 0) {
      dest.x += ((m_xPos / 100.0) * rect.w);
      if (m_xAlign == PUP_LABEL_XALIGN_CENTER)
         dest.x -= (width / 2);
      else if (m_xAlign == PUP_LABEL_XALIGN_RIGHT)
         dest.x -= width;
   }
   else {
      if (m_xAlign == PUP_LABEL_XALIGN_CENTER)
         dest.x += ((rect.w - width) / 2);
      else if (m_xAlign == PUP_LABEL_XALIGN_RIGHT)
         dest.x += (rect.w - width);
   }

   if (m_yPos > 0) {
      dest.y += ((m_yPos / 100.0) * rect.h);
      if (m_yAlign == PUP_LABEL_YALIGN_CENTER)
         dest.y -= (height / 2);
      else if (m_yAlign == PUP_LABEL_YALIGN_BOTTOM)
         dest.y -= height;
   }
   else {
      if (m_yAlign == PUP_LABEL_YALIGN_CENTER)
         dest.y += ((rect.h - height) / 2.0);
      else if (m_yAlign == PUP_LABEL_YALIGN_BOTTOM)
         dest.y += (rect.h - height);
   }

   SDL_FPoint center = { height / 2.0f, 0 };
   SDL_RenderCopyExF(pRenderer, m_pTexture, NULL, &dest, -m_angle / 10.0, &center, SDL_FLIP_NONE);
}

void PUPLabel::UpdateLabelTexture(SDL_Renderer* pRenderer, SDL_Rect& rect)
{
   static robin_hood::unordered_map<string, robin_hood::unordered_set<string>> warnedLabels;

   if (m_pTexture) {
      SDL_DestroyTexture(m_pTexture);
      m_pTexture = NULL;
   }

   int height = (int)((m_size / 100.0) * rect.h);
   TTF_SetFontSize(m_pFont, height);
   TTF_SetFontOutline(m_pFont, 0);

   SDL_Color textColor = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color) };
   SDL_Surface* pTextSurface = TTF_RenderUTF8_Blended_Wrapped(m_pFont, m_szCaption.c_str(), textColor, 0);
   if (!pTextSurface) {
      string szError = TTF_GetError();
      if (warnedLabels[szError].find(m_szName) == warnedLabels[szError].end()) {
         PLOGW.printf("Unable to render text: label=%s, error=%s", m_szName.c_str(), szError.c_str());
         warnedLabels[szError].insert(m_szName);
      }
      return;
   }

   SDL_Surface* pMergedSurface = NULL;

   if (m_shadowState) {
      SDL_Color shadowColor = { GetRValue(m_shadowColor), GetGValue(m_shadowColor), GetBValue(m_shadowColor) };
      SDL_Surface* pShadowSurface = TTF_RenderUTF8_Blended_Wrapped(m_pFont, m_szCaption.c_str(), shadowColor, 0);
      if (!pShadowSurface) {
         string szError = TTF_GetError();
         if (warnedLabels[szError].find(m_szName) == warnedLabels[szError].end()) {
            PLOGW.printf("Unable to render text: label=%s, error=%s", m_szName.c_str(), szError.c_str());
            warnedLabels[szError].insert(m_szName);
         }
         delete pTextSurface;
         return;
      }

      int xoffset = (int) (((abs(m_xoffset) / 100.0) * height) / 2);
      int yoffset = (int) (((abs(m_yoffset) / 100.0) * height) / 2);

      pMergedSurface = SDL_CreateRGBSurfaceWithFormat(0, pTextSurface->w + xoffset, pTextSurface->h + yoffset, 32, SDL_PIXELFORMAT_RGBA32);
      if (pMergedSurface) {
         //SDL_FillRect(pMergedSurface, NULL, SDL_MapRGBA(pMergedSurface->format, 255, 255, 0, 255));
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
      }

      SDL_FreeSurface(pShadowSurface);
   }
   else {
      pMergedSurface = SDL_CreateRGBSurfaceWithFormat(0, pTextSurface->w, pTextSurface->h, 32, SDL_PIXELFORMAT_RGBA32);
      if (pMergedSurface) {
         //SDL_FillRect(pMergedSurface, NULL, SDL_MapRGBA(pMergedSurface->format, 255, 255, 0, 255));
         SDL_BlitSurface(pTextSurface, NULL, pMergedSurface, NULL);
      }
   }

   if (pMergedSurface) {
      m_pTexture = SDL_CreateTextureFromSurface(pRenderer, pMergedSurface);
      if (m_pTexture) {
         m_width = pMergedSurface->w;
         m_height = pMergedSurface->h;

         m_dirty = true;
      }

      SDL_FreeSurface(pMergedSurface);
   }

   SDL_FreeSurface(pTextSurface);
}

string PUPLabel::ToString() const
{
   return "name=" + m_szName +
      ", caption=" + m_szCaption +
      ", visible=" + (m_visible ? "true" : "false") +
      ", size=" + std::to_string(m_size) +
      ", color=" + std::to_string(m_color) +
      ", angle=" + std::to_string(m_angle) +
      ", xAlign=" + (m_xAlign == PUP_LABEL_XALIGN_LEFT  ? "LEFT" : m_xAlign == PUP_LABEL_XALIGN_CENTER ? "CENTER" : "RIGHT") +
      ", yAlign=" + (m_yAlign == PUP_LABEL_YALIGN_TOP ? "TOP" : m_yAlign == PUP_LABEL_YALIGN_CENTER ? "CENTER" : "BOTTOM") +
      ", xPos=" + std::to_string(m_xPos) +
      ", yPos=" + std::to_string(m_yPos) +
      ", pagenum=" + std::to_string(m_pagenum) +
      ", szPath=" + m_szPath;
}
