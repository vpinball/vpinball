/*
 * This code was derived from notes at:
 *
 * https://nailbuster.com/pup/PUPDMD_Reference_v13.zip
 */

#include "PUPLabel.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"
#include "PUPManager.h"

#pragma warning(push)
#pragma warning(disable : 4267)
#include "RSJparser/RSJparser.tcc"
#pragma warning(pop)

#include <filesystem>

PUPLabel::PUPLabel(PUPManager* manager, const string& szName, const string& szFont, float size, int color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos,
   float yPos, int pagenum, bool visible)
   : m_pManager(manager)
   , m_szName(szName)
{
   if (!szFont.empty())
   {
      m_pFont = manager->GetFont(szFont);
      if (!m_pFont)
         LOGE("Font not found: label=%s, font=%s", szName.c_str(), szFont.c_str());
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
}

PUPLabel::~PUPLabel()
{
}

void PUPLabel::SetCaption(const string& szCaption)
{
   if (szCaption.empty() && m_type != PUP_LABEL_TYPE_TEXT)
      return;

   if (szCaption == "`u`")
      return;

   string szText = szCaption;
   std::ranges::replace(szText.begin(), szText.end(), '~', '\n');
   szText = string_replace_all(szText, "\\r"s, "\n"s);

   {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (m_szCaption != szText)
      {
         m_type = PUP_LABEL_TYPE_TEXT;
         m_szPath.clear();

         const string szExt = extension_from_path(szText);
         if (szExt == "gif" || szExt == "png" || szExt == "apng" || szExt == "bmp" || szExt == "jpg")
         {
            std::filesystem::path fs_path(normalize_path_separators(szText));
            string playlistFolder = fs_path.parent_path().string();
            PUPPlaylist* pPlaylist = m_pScreen->GetPlaylist(playlistFolder);
            if (pPlaylist)
            {
               string szPath = pPlaylist->GetPlayFilePath(fs_path.filename().string());
               if (!szPath.empty())
               {
                  m_szPath = szPath;
                  m_type = (szExt == "gif") ? PUP_LABEL_TYPE_GIF : PUP_LABEL_TYPE_IMAGE;
               }
               else
               {
                  LOGE("Image not found: screen=%d, label=%s, path=%s", m_pScreen->GetScreenNum(), m_szName.c_str(), szText.c_str());
                  // we need to set a path otherwise the caption will be used as text
                  m_szPath = szText;
               }
            }
            else
            {
               LOGE("Image playlist not found: screen=%d, label=%s, path=%s, playlist=%s", m_pScreen->GetScreenNum(), m_szName.c_str(), szText.c_str(), playlistFolder.c_str());
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
   LOGD("PUPLabel::SetSpecial: name=%s, caption=%s, json=%s", m_szName.c_str(), m_szCaption.c_str(), szSpecial.c_str());

   RSJresource json(szSpecial);
   switch (json["mt"s].as<int>(0))
   {
   case 2:
   {
      if (json["zback"s].exists() && json["zback"s].as<int>() == 1)
         m_pScreen->SendLabelToBack(this);

      if (json["ztop"s].exists() && json["ztop"s].as<int>() == 1)
         m_pScreen->SendLabelToFront(this);

      {
         std::lock_guard<std::mutex> lock(m_mutex);

         if (json["size"s].exists())
         {
            m_size = std::stof(json["size"s].as_str());
            m_dirty = true;
         }

         if (json["xpos"s].exists())
         {
            m_xPos = std::stof(json["xpos"s].as_str());
            m_dirty = true;
         }

         if (json["ypos"s].exists())
         {
            m_yPos = std::stof(json["ypos"s].as_str());
            m_dirty = true;
         }

         if (json["fname"s].exists())
         {
            string szFont = json["fname"s].as_str();
            m_pFont = m_pManager->GetFont(szFont);
            if (!m_pFont)
               LOGE("Label font not found: name=%s, font=%s", m_szName.c_str(), szFont.c_str());
            m_dirty = true;
         }

         if (json["fonth"s].exists())
         {
            m_size = std::stof(json["fonth"s].as_str());
            m_dirty = true;
         }

         if (json["color"s].exists())
         {
            m_color = json["color"s].as<int>();
            m_dirty = true;
         }

         if (json["xalign"s].exists())
         {
            m_xAlign = (PUP_LABEL_XALIGN)json["xalign"s].as<int>();
            m_dirty = true;
         }

         if (json["yalign"s].exists())
         {
            m_yAlign = (PUP_LABEL_YALIGN)json["yalign"s].as<int>();
            m_dirty = true;
         }

         if (json["pagenum"s].exists())
         {
            m_pagenum = json["pagenum"s].as<int>();
            m_dirty = true;
         }

         if (json["stopani"s].exists())
         {
            // stop any pup animations on label/image (zoom/flash/pulse).  this is not about animated gifs
            LOGE("stopani not implemented");
            m_dirty = true;
         }

         if (json["rotate"s].exists())
         {
            // in tenths.  so 900 is 90 degrees. rotate support for images too.  note images must be aligned center to rotate properly(default)
            m_angle = std::stof(json["rotate"s].as_str());
            m_dirty = true;
         }

         if (json["zoom"s].exists())
         {
            // 120 for 120% of current height, 80% etc...
            LOGE("zoom not implemented");
            m_dirty = true;
         }

         if (json["alpha"s].exists())
         {
            // '0-255  255=full, 0=blank
            LOGE("alpha not implemented");
            m_dirty = true;
         }

         if (json["gradstate"s].exists() && json["gradcolor"s].exists())
         {
            // color=gradcolor, gradstate = 0 (gradstate is percent)
            LOGE("gradstate/gradcolor not implemented");
            m_dirty = true;
         }

         if (json["grayscale"s].exists())
         {
            // only on image objects.  will show as grayscale.  1=gray filter on 0=off normal mode
            LOGE("filter not implemented");
            m_dirty = true;
         }

         if (json["filter"s].exists())
         {
            // fmode 1-5 (invertRGB, invert,grayscale,invertalpha,clear),blur)
            LOGE("filter not implemented");
            m_dirty = true;
         }

         if (json["shadowcolor"s].exists())
         {
            m_shadowColor = json["shadowcolor"s].as<int>();
            m_dirty = true;
         }

         if (json["shadowtype"s].exists())
         {
            // ST = 1 (Shadow), ST = 2 (Border)
            LOGE("shadowtype not implemented");
            m_dirty = true;
         }

         if (json["xoffset"s].exists())
         {
            m_xoffset = std::stof(json["xoffset"s].as_str());
            m_dirty = true;
         }

         if (json["yoffset"s].exists())
         {
            m_yoffset = std::stof(json["yoffset"s].as_str());
            m_dirty = true;
         }

         if (json["anigif"s].exists())
         {
            m_anigif = json["anigif"s].as<int>();
            m_dirty = true;
         }

         if (json["width"s].exists())
         {
            m_imageWidth = std::stof(json["width"s].as_str());
            m_dirty = true;
         }

         if (json["height"s].exists())
         {
            m_imageHeight = std::stof(json["height"s].as_str());
            m_dirty = true;
         }

         if (json["autow"s].exists())
         {
            LOGE("autow not implemented");
            m_dirty = true;
         }

         if (json["autoh"s].exists())
         {
            LOGE("autoh not implemented");
            m_dirty = true;
         }

         if (json["shadowstate"s].exists())
         {
            m_shadowState = json["shadowstate"s].as<int>();
            m_dirty = true;
         }

         if (json["outline"s].exists())
         {
            m_outline = (json["outline"s].as<int>() == 1);
            m_dirty = true;
         }
      }
   }
   break;

   case 1:
   {
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

void PUPLabel::Render(VPXRenderContext2D* const ctx, SDL_Rect& rect, int pagenum)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   if (!m_visible || pagenum != m_pagenum || m_szCaption.empty())
      return;

   if (m_pendingTextureUpdate.valid())
   {
      if (m_pendingTextureUpdate.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      {
         m_renderState = m_pendingTextureUpdate.get();
         m_animationFrame = -1;
         m_animationStart = SDL_GetTicks();
      }
   }
   else if (m_dirty || (m_szPath.empty() && rect.h != m_renderState.m_prerenderedHeight && m_szPath.empty()))
   {
      m_dirty = false;
      if (m_szPath.empty())
         m_pendingTextureUpdate = std::async(std::launch::async, [this, rect]() { return UpdateLabelTexture(rect.h, m_pFont, m_szCaption, m_size, m_color, m_shadowState, m_shadowColor, { m_xoffset, m_yoffset} ); });
      else
         m_pendingTextureUpdate = std::async(std::launch::async, [this]() { return UpdateImageTexture(m_type, m_szPath); });
   }

   if (!m_renderState.m_pTexture)
      return;

   if (m_renderState.m_pAnimation)
   {
      int expectedFrame = static_cast<int>(static_cast<float>(SDL_GetTicks() - m_animationStart) * (float)(60. / 1000.)) % m_renderState.m_pAnimation->count;
      if (expectedFrame != m_animationFrame)
      {
         m_animationFrame = expectedFrame;
         SDL_Surface* surf = m_renderState.m_pAnimation->frames[m_animationFrame];
         SDL_LockSurface(surf);
         UpdateTexture(&m_renderState.m_pTexture, surf->w, surf->h, VPXTextureFormat::VPXTEXFMT_sRGBA, static_cast<uint8_t*>(surf->pixels));
         SDL_UnlockSurface(surf);
      }
   }

   float width;
   float height;
   if (m_type == PUP_LABEL_TYPE_TEXT)
   {
      width = m_renderState.m_width;
      height = m_renderState.m_height;
   }
   else
   {
      width = (m_imageWidth / 100.0f) * static_cast<float>(rect.w);
      height = (m_imageHeight / 100.0f) * static_cast<float>(rect.h);
   }

   SDL_FRect dest = { static_cast<float>(rect.x), static_cast<float>(rect.y), width, height };

   if (m_xPos > 0.f)
   {
      dest.x += ((m_xPos / 100.0f) * static_cast<float>(rect.w));
      if (m_xAlign == PUP_LABEL_XALIGN_CENTER)
         dest.x -= (width / 2.f);
      else if (m_xAlign == PUP_LABEL_XALIGN_RIGHT)
         dest.x -= width;
   }
   else
   {
      if (m_xAlign == PUP_LABEL_XALIGN_CENTER)
         dest.x += ((static_cast<float>(rect.w) - width) / 2.f);
      else if (m_xAlign == PUP_LABEL_XALIGN_RIGHT)
         dest.x += (static_cast<float>(rect.w) - width);
   }

   if (m_yPos > 0.f)
   {
      dest.y += ((m_yPos / 100.0f) * static_cast<float>(rect.h));
      if (m_yAlign == PUP_LABEL_YALIGN_CENTER)
         dest.y -= (height / 2.f);
      else if (m_yAlign == PUP_LABEL_YALIGN_BOTTOM)
         dest.y -= height;
   }
   else
   {
      if (m_yAlign == PUP_LABEL_YALIGN_CENTER)
         dest.y += ((static_cast<float>(rect.h) - height) / 2.f);
      else if (m_yAlign == PUP_LABEL_YALIGN_BOTTOM)
         dest.y += (static_cast<float>(rect.h) - height);
   }

   //SDL_FPoint center = { height / 2.0f, 0 };

   int texWidth, texHeight;
   GetTextureInfo(m_renderState.m_pTexture, &texWidth, &texHeight);
   ctx->DrawImage(ctx, m_renderState.m_pTexture, 1.f, 1.f, 1.f, 1.f, 0.f, 0.f, static_cast<float>(texWidth), static_cast<float>(texHeight), dest.x, dest.y, dest.w, dest.h);
   // FIXME port SDL_RenderTextureRotated(m_pTexture, NULL, &dest, -m_angle / 10.0, &center, SDL_FLIP_NONE);
}

PUPLabel::RenderState PUPLabel::UpdateImageTexture(PUP_LABEL_TYPE type, const string& szPath)
{
   SetThreadName("PUPLabel.Upd." + m_szName);

   RenderState rs;

   // Handle Image 'labels'
   if (type == PUP_LABEL_TYPE_IMAGE)
   {
      SDL_Surface* image = IMG_Load(szPath.c_str());
      if (image)
      {
         rs.m_pTexture = CreateTexture(image);
         SDL_DestroySurface(image);
      }
      else
      {
         LOGE("Unable to load image: %s", szPath.c_str());
      }
   }
   else if (type == PUP_LABEL_TYPE_GIF)
   {
      rs.m_pAnimation = IMG_LoadAnimation(szPath.c_str());
      if (rs.m_pAnimation)
         rs.m_pTexture = CreateTexture(rs.m_pAnimation->frames[0]);
      else
         LOGE("Unable to load animation: %s", szPath.c_str());
   }
   return rs;
}

PUPLabel::RenderState PUPLabel::UpdateLabelTexture(int outHeight, TTF_Font* pFont, const string& szCaption, float size, int color, int shadowstate, int shadowcolor, SDL_FPoint offset)
{
   SetThreadName("PUPLabel.Upd." + m_szName);

   // TTF_Font may not be accessed simultaneously from multiple thread so serialize updates using a mutex
   static std::mutex fontMutex;
   std::lock_guard<std::mutex> lock(fontMutex);

   RenderState rs;
   rs.m_prerenderedHeight = outHeight;

   float height = (size / 100.0f) * static_cast<float>(outHeight);
   TTF_SetFontSize(pFont, height);
   TTF_SetFontOutline(pFont, 0);

   SDL_Color textColor = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color) };
   SDL_Surface* pTextSurface = TTF_RenderText_Blended_Wrapped(pFont, szCaption.c_str(), szCaption.length(), textColor, 0);
   if (!pTextSurface)
   {
      LOGE("Unable to render text: label=%s, error=%s", m_szName.c_str(), SDL_GetError());
      return rs;
   }

   SDL_Surface* pMergedSurface = nullptr;

   if (shadowstate)
   {
      SDL_Color shadowColor = { GetRValue(shadowcolor), GetGValue(shadowcolor), GetBValue(shadowcolor) };
      SDL_Surface* pShadowSurface = TTF_RenderText_Blended_Wrapped(pFont, szCaption.c_str(), szCaption.length(), shadowColor, 0);
      if (!pShadowSurface)
      {
         LOGE("Unable to render text: label=%s, error=%s", m_szName.c_str(), SDL_GetError());
         delete pTextSurface;
         return rs;
      }

      int xoffset = (int)(((abs(offset.x) / 100.0f) * height) / 2.f);
      int yoffset = (int)(((abs(offset.y) / 100.0f) * height) / 2.f);

      pMergedSurface = SDL_CreateSurface(pTextSurface->w + xoffset, pTextSurface->h + yoffset, SDL_PIXELFORMAT_RGBA32);
      if (pMergedSurface)
      {
         //SDL_FillSurfaceRect(pMergedSurface, NULL, SDL_MapRGBA(pMergedSurface->format, 255, 255, 0, 255));
         if (!m_outline)
         {
            SDL_Rect shadowRect = { (xoffset < 0) ? 0 : xoffset, (yoffset < 0) ? 0 : yoffset, pShadowSurface->w, pShadowSurface->h };
            SDL_BlitSurface(pShadowSurface, NULL, pMergedSurface, &shadowRect);

            SDL_Rect textRect = { (xoffset > 0) ? 0 : xoffset, (yoffset > 0) ? 0 : yoffset, pTextSurface->w, pTextSurface->h };
            SDL_BlitSurface(pTextSurface, NULL, pMergedSurface, &textRect);
         }
         else
         {
            SDL_Rect shadowRects[8]
               = { { 0, 0, pShadowSurface->w, pShadowSurface->h }, { xoffset / 2, 0, pShadowSurface->w, pShadowSurface->h }, { xoffset, 0, pShadowSurface->w, pShadowSurface->h },

                    { 0, yoffset / 2, pShadowSurface->w, pShadowSurface->h }, { xoffset, yoffset / 2, pShadowSurface->w, pShadowSurface->h },

                    { 0, yoffset, pShadowSurface->w, pShadowSurface->h }, { xoffset / 2, yoffset, pShadowSurface->w, pShadowSurface->h },
                    { xoffset, yoffset, pShadowSurface->w, pShadowSurface->h } };

            for (int i = 0; i < 8; ++i)
               SDL_BlitSurface(pShadowSurface, NULL, pMergedSurface, &shadowRects[i]);

            SDL_Rect textRect = { xoffset / 2, yoffset / 2, pTextSurface->w, pTextSurface->h };
            SDL_BlitSurface(pTextSurface, NULL, pMergedSurface, &textRect);
         }
      }

      SDL_DestroySurface(pShadowSurface);
   }
   else
   {
      pMergedSurface = SDL_CreateSurface(pTextSurface->w, pTextSurface->h, SDL_PIXELFORMAT_RGBA32);
      if (pMergedSurface)
      {
         //SDL_FillSurfaceRect(pMergedSurface, NULL, SDL_MapRGBA(pMergedSurface->format, 255, 255, 0, 255));
         SDL_BlitSurface(pTextSurface, NULL, pMergedSurface, NULL);
      }
   }

   if (pMergedSurface)
   {
      rs.m_pTexture = CreateTexture(pMergedSurface);
      if (rs.m_pTexture)
      {
         rs.m_width = static_cast<float>(pMergedSurface->w);
         rs.m_height = static_cast<float>(pMergedSurface->h);
      }
      SDL_DestroySurface(pMergedSurface);
   }

   SDL_DestroySurface(pTextSurface);

   return rs;
}

string PUPLabel::ToString() const
{
   return "name=" + m_szName + ", caption=" + m_szCaption + ", visible=" + (m_visible ? "true" : "false") + ", size=" + std::to_string(m_size) + ", color=" + std::to_string(m_color)
      + ", angle=" + std::to_string(m_angle) + ", xAlign="
      + (m_xAlign == PUP_LABEL_XALIGN_LEFT        ? "LEFT"
            : m_xAlign == PUP_LABEL_XALIGN_CENTER ? "CENTER"
                                                  : "RIGHT")
      + ", yAlign="
      + (m_yAlign == PUP_LABEL_YALIGN_TOP         ? "TOP"
            : m_yAlign == PUP_LABEL_YALIGN_CENTER ? "CENTER"
                                                  : "BOTTOM")
      + ", xPos=" + std::to_string(m_xPos) + ", yPos=" + std::to_string(m_yPos) + ", pagenum=" + std::to_string(m_pagenum) + ", szPath=" + m_szPath;
}
