// license:GPLv3+

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

#define LOG_PUPLABEL 0

namespace PUP {

#ifndef GetRValue
#define GetRValue(rgba32) static_cast<uint8_t>(rgba32)
#define GetGValue(rgba32) static_cast<uint8_t>((rgba32) >> 8)
#define GetBValue(rgba32) static_cast<uint8_t>((rgba32) >> 16)
#endif

PUPLabel::PUPLabel(PUPManager* manager, const string& szName, const string& szFont, float size, int color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos,
   float yPos, int pagenum, bool visible)
   : m_pManager(manager)
   , m_szName(szName)
   , m_pFont(szFont.empty() ? nullptr : manager->GetFont(szFont))
   , m_size(size)
   , m_color(color)
   , m_angle(angle)
   , m_xAlign(xAlign)
   , m_yAlign(yAlign)
   , m_xPos(xPos)
   , m_yPos(yPos)
   , m_visible(visible)
   , m_pagenum(pagenum)
{
   if (!szFont.empty() && !m_pFont)
   {
      LOGE("Font not found: label=%s, font=%s", szName.c_str(), szFont.c_str());
   }
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
   szText = string_replace_all(szText, "\\r"s, '\n');

   {
      std::lock_guard lock(m_mutex);
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
               std::filesystem::path szPath = pPlaylist->GetPlayFilePath(fs_path.filename().string());
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
   std::lock_guard lock(m_mutex);
   m_visible = visible;
}

void PUPLabel::SetSpecial(const string& szSpecial)
{
   #if LOG_PUPLABEL
   LOGD("PUPLabel::SetSpecial: name=%s, caption=%s, json=%s", m_szName.c_str(), m_szCaption.c_str(), szSpecial.c_str());
   #endif

   string str = trim_string(szSpecial);
   if (str.empty())
      return;

   RSJresource json(str);
   switch (json["mt"s].as<int>(0))
   {
   case 1:
      /* Animate
            at = animate type
            len = length in ms of animation
            fc = foreground color of text during animation
            at = 1: flashing
               fq = frequency of flashing
            at = 2: motion
               yps = y position start ( 0=no y movement, 1= slide from bottom, -1=slide from top)
               xps = x position start ( 0=no x movement, 1= slide from right, -1=slide from left)
               ype = y position end (see yps)
               xpe = x position end (see xps)
               mlen = length ms of AniTween of movement.
               tt = tween index....yup supports 0-10 different tweens of movement between start and end.
               mColor = ?
         */
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         switch (json["at"s].as<int>(0))
         {
         case 1:
            if (json["len"s].exists() && json["fc"s].exists() && json["fq"s].exists() && json["fq"s].as<int>() > 0)
               m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(), json["fc"s].as<int>(), 1000 / json["fq"s].as<int>());
            else
            {
               LOGE("Invalid label animation specified: {%s}", szSpecial.c_str());
            }
            break;
            
         case 2:
            if (json["len"s].exists() && json["fc"s].exists())
               m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(), json["fc"s].as<int>(), 
                  json["xps"s].as<int>(0), json["xpe"s].as<int>(0), json["yps"s].as<int>(0), json["ype"s].as<int>(0),
                  json["mlen"s].as<int>(0), json["tt"s].as<int>(0), json["mColor"s].as<int>(0));
            else
            {
               LOGE("Invalid label animation specified: {%s}", szSpecial.c_str());
            }
            break;

         default:
            LOGE("Unsupported Label.SetSpecial animation type: %d", json["at"s].as<int>(0));
            break;
         }
      }
      break;
   
   case 2:
      if (json["zback"s].exists() && json["zback"s].as<int>() == 1)
         m_pScreen->SendLabelToBack(this);

      if (json["ztop"s].exists() && json["ztop"s].as<int>() == 1)
         m_pScreen->SendLabelToFront(this);

      {
         std::lock_guard lock(m_mutex);

         if (json["size"s].exists())
         {
            m_size = static_cast<float>(json["size"s].as<double>());
            m_dirty = true;
         }

         if (json["xpos"s].exists())
         {
            m_xPos = static_cast<float>(json["xpos"s].as<double>());
            m_dirty = true;
         }

         if (json["ypos"s].exists())
         {
            m_yPos = static_cast<float>(json["ypos"s].as<double>());
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
            m_size = static_cast<float>(json["fonth"s].as<double>());
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
            m_animation = nullptr;
            m_dirty = true;
         }

         if (json["rotate"s].exists())
         {
            // in tenths.  so 900 is 90 degrees. rotate support for images too.  note images must be aligned center to rotate properly(default)
            m_angle = static_cast<float>(json["rotate"s].as<double>() / 10.0);
            m_dirty = true;
         }

         if (json["zoom"s].exists())
         {
            // 120 for 120% of current height, 80% etc...
            NOT_IMPLEMENTED("zoom not implemented");
            m_dirty = true;
         }

         if (json["alpha"s].exists())
         {
            // '0-255  255=full, 0=blank
            NOT_IMPLEMENTED("alpha not implemented");
            m_dirty = true;
         }

         if (json["gradstate"s].exists() && json["gradcolor"s].exists())
         {
            // color=gradcolor, gradstate = 0 (gradstate is percent)
            NOT_IMPLEMENTED("gradstate/gradcolor not implemented");
            m_dirty = true;
         }

         if (json["grayscale"s].exists())
         {
            // only on image objects.  will show as grayscale.  1=gray filter on 0=off normal mode
            NOT_IMPLEMENTED("filter not implemented");
            m_dirty = true;
         }

         if (json["filter"s].exists())
         {
            // fmode 1-5 (invertRGB, invert,grayscale,invertalpha,clear),blur)
            NOT_IMPLEMENTED("filter not implemented");
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
            NOT_IMPLEMENTED("shadowtype not implemented");
            m_dirty = true;
         }

         if (json["xoffset"s].exists())
         {
            m_xoffset = static_cast<float>(json["xoffset"s].as<double>());
            m_dirty = true;
         }

         if (json["yoffset"s].exists())
         {
            m_yoffset = static_cast<float>(json["yoffset"s].as<double>());
            m_dirty = true;
         }

         if (json["anigif"s].exists())
         {
            m_anigif = json["anigif"s].as<int>();
            m_dirty = true;
         }

         if (json["width"s].exists())
         {
            m_imageWidth = static_cast<float>(json["width"s].as<double>());
            m_dirty = true;
         }

         if (json["height"s].exists())
         {
            m_imageHeight = static_cast<float>(json["height"s].as<double>());
            m_dirty = true;
         }

         if (json["autow"s].exists())
         {
            NOT_IMPLEMENTED("autow not implemented");
            m_dirty = true;
         }

         if (json["autoh"s].exists())
         {
            NOT_IMPLEMENTED("autoh not implemented");
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
      break;

   default:
      LOGE("Unsupported Label.SetSpecial mt mode: %d", json["mt"s].as<int>(0));
      break;
   }
}

void PUPLabel::Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect, int pagenum)
{
   std::lock_guard lock(m_mutex);

   if (!m_visible || pagenum != m_pagenum || m_szCaption.empty() || (m_animation && !m_animation->m_visible))
      return;
   
   int fontColor = m_animation ? m_animation->m_color : m_color;
   
   if (m_pendingTextureUpdate.valid())
   {
      if (m_pendingTextureUpdate.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      {
         m_renderState = m_pendingTextureUpdate.get();
         m_animationFrame = -1;
         m_animationStart = SDL_GetTicks();
      }
   }
   else if (m_dirty
      || (m_szPath.empty() && rect.h != m_renderState.m_prerenderedHeight)
      || (m_szPath.empty() && fontColor != m_renderState.m_prerenderedColor))
   {
      m_dirty = false;
      if (!m_szPath.empty())
         m_pendingTextureUpdate = std::async(std::launch::async, [this]() {
            std::lock_guard lock(m_mutex);
            return UpdateImageTexture(m_type, m_szPath);
         });
      else if (m_pFont)
         m_pendingTextureUpdate = std::async(std::launch::async, [this, rect, fontColor]() {
            std::lock_guard lock(m_mutex);
            return UpdateLabelTexture(rect.h, m_pFont, m_szCaption, m_size, fontColor, m_shadowState, m_shadowColor, { m_xoffset, m_yoffset} );
         });
   }

   if (!m_renderState.m_pTexture)
      return;

   if (m_renderState.m_pAnimation)
   {
      int expectedFrame = static_cast<int>(static_cast<float>(SDL_GetTicks() - m_animationStart) * (float)(60. / 1000.)) % m_renderState.m_pAnimation->count;
      if (expectedFrame != m_animationFrame)
      {
         m_animationFrame = expectedFrame;
         // TODO perform conversion on the ancillary thread and not at render time
         SDL_Surface* surf = m_renderState.m_pAnimation->frames[m_animationFrame];
         if (surf->format != SDL_PIXELFORMAT_RGBA32)
            surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
         SDL_LockSurface(surf);
         // TODO handle situations where width != pitch
         UpdateTexture(&m_renderState.m_pTexture, surf->w, surf->h, VPXTextureFormat::VPXTEXFMT_sRGBA8, surf->pixels);
         SDL_UnlockSurface(surf);
         if (surf != m_renderState.m_pAnimation->frames[m_animationFrame])
            SDL_DestroySurface(surf);
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

   dest.x += static_cast<float>(rect.w) * (m_xPos == 0.f ? 0.5f : (m_xPos / 100.0f));
   if (m_xAlign == PUP_LABEL_XALIGN_CENTER)
      dest.x -= (width / 2.f);
   else if (m_xAlign == PUP_LABEL_XALIGN_RIGHT)
      dest.x -= width;

   dest.y += static_cast<float>(rect.h) * (m_yPos == 0.f ? 0.5f : (m_yPos / 100.0f));
   if (m_yAlign == PUP_LABEL_YALIGN_CENTER)
      dest.y -= (height / 2.f);
   else if (m_yAlign == PUP_LABEL_YALIGN_BOTTOM)
      dest.y -= height;

   if (m_animation)
   {
      if (m_animation->Update(rect, dest))
         m_animation = nullptr;

      dest.x += m_animation->m_xOffset;
      dest.y += m_animation->m_yOffset;
   }

   VPXTextureInfo* texInfo = GetTextureInfo(m_renderState.m_pTexture);
   ctx->DrawImage(ctx, m_renderState.m_pTexture, 1.f, 1.f, 1.f, 1.f,
      0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), 
      0.f, 0.f, -m_angle, // FIXME compute center (used to be SDL_FPoint center = { height / 2.0f, 0 };)
      dest.x, dest.y, dest.w, dest.h);
}

PUPLabel::RenderState PUPLabel::UpdateImageTexture(PUP_LABEL_TYPE type, const std::filesystem::path& szPath)
{
   SetThreadName("PUPLabel.Upd." + m_szName);

   RenderState rs;

   // Handle Image 'labels'
   if (type == PUP_LABEL_TYPE_IMAGE)
   {
      SDL_Surface* image = IMG_Load(szPath.string().c_str());
      if (image && image->format != SDL_PIXELFORMAT_RGBA32) {
         SDL_Surface* newImage = SDL_ConvertSurface(image, SDL_PIXELFORMAT_RGBA32);
         SDL_DestroySurface(image);
         image = newImage;
      }
      if (image) {
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
      rs.m_pAnimation = IMG_LoadAnimation(szPath.string().c_str());
      if (rs.m_pAnimation) {
         SDL_Surface* image = rs.m_pAnimation->frames[0];
         if (image) {
            if (image->format == SDL_PIXELFORMAT_RGBA32)
               rs.m_pTexture = CreateTexture(image);
            else {
               SDL_Surface* newImage = SDL_ConvertSurface(image, SDL_PIXELFORMAT_RGBA32);
               if (newImage) {
                  rs.m_pTexture = CreateTexture(newImage);
                  SDL_DestroySurface(newImage);
               }
            }
         }
      }
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
   rs.m_prerenderedColor = color;

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
      + ", xPos=" + std::to_string(m_xPos) + ", yPos=" + std::to_string(m_yPos) + ", pagenum=" + std::to_string(m_pagenum) + ", szPath=" + m_szPath.string();
}

PUPLabel::Animation::Animation(PUPLabel* label, int lengthMs, int foregroundColor, int flashingPeriod)
   : m_label(label)
   , m_color(label->m_color)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_flashingPeriod(flashingPeriod)
{
}

PUPLabel::Animation::Animation(PUPLabel* label, int lengthMs, int foregroundColor, int xps, int xpe, int yps, int ype, int motionLen, int motionTween, int motionColor)
   : m_label(label)
   , m_color(label->m_color)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_xps(xps)
   , m_xpe(xpe)
   , m_yps(yps)
   , m_ype(ype)
   , m_motionLen(motionLen)
   , m_motionTween(motionTween)
   , m_motionColor(motionColor)
{
}
   
bool PUPLabel::Animation::Update(const SDL_Rect& screenRect, const SDL_FRect& labelRect)
{
   uint64_t elapsed = SDL_GetTicks() - m_startTimestamp;

   if (elapsed >= m_lengthMs)
   {
      m_color = m_label->m_color;
      m_xOffset = 0.f;
      m_yOffset = 0.f;
      m_visible = true;
      return true;
   }

   if (m_flashingPeriod)
   {
      m_visible = ((elapsed / m_flashingPeriod) & 1) != 0;
      m_color = m_foregroundColor;
   }

   if (m_motionLen)
   {
      // TODO implement tweening
      float pos = clamp(static_cast<float>(elapsed) / static_cast<float>(m_motionLen), 0.f, 1.f);
      const float xBase = labelRect.x;
      const float xLeft = static_cast<float>(screenRect.x) - labelRect.w;
      const float xRight = static_cast<float>(screenRect.x) + static_cast<float>(screenRect.w);
      const float yBase = labelRect.y;
      const float yTop = static_cast<float>(screenRect.y) - labelRect.h;
      const float yBottom = static_cast<float>(screenRect.y) + static_cast<float>(screenRect.h);
      const float xs = m_xps == 0 ? xBase : m_xps == 1 ? xRight : xLeft;
      const float xe = m_xpe == 0 ? xBase : m_xpe == 1 ? xRight : xLeft;
      const float ys = m_yps == 0 ? yBase : m_yps == 1 ? yBottom : yTop;
      const float ye = m_ype == 0 ? yBase : m_ype == 1 ? yBottom : yTop;
      m_color = pos < 1.0f ? m_motionColor : m_foregroundColor;
      m_xOffset = lerp(pos, xs, xe) - xBase;
      m_yOffset = lerp(pos, ys, ye) - yBase;
   }

   return false;
}


}
