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
#include <format>

#define LOG_PUPLABEL 0

namespace PUP {

#ifndef GetRValue
#define GetRValue(rgba32) static_cast<uint8_t>(rgba32)
#define GetGValue(rgba32) static_cast<uint8_t>((rgba32) >> 8)
#define GetBValue(rgba32) static_cast<uint8_t>((rgba32) >> 16)
#endif

std::mutex PUPLabel::m_fontMutex;

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
   , m_onShowVisible(visible)
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
         std::lock_guard lock(m_mutex);
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
   {
      std::lock_guard lock(m_mutex);
      for (auto& [key, value] : json.as_object())
      {
         if (key == "mt"s)
         {

         }
         else if (key == "zback"s)
         {
            if (value.as<int>() == 1)
               m_pScreen->SendLabelToBack(this);
         }
         else if (key == "ztop"s)
         {
            if (value.as<int>() == 1)
               m_pScreen->SendLabelToFront(this);
         }
         else if (key == "size"s)
         {
            m_size = static_cast<float>(json["size"s].as<double>());
            m_dirty = true;
         }
         else if (key == "xpos"s)
         {
            m_xPos = static_cast<float>(value.as<double>());
            m_dirty = true;
         }
         else if (key == "ypos"s)
         {
            m_yPos = static_cast<float>(value.as<double>());
            m_dirty = true;
         }
         else if (key == "fname"s)
         {
            string szFont = value.as_str();
            m_pFont = m_pManager->GetFont(szFont);
            if (!m_pFont)
               LOGE("Label font not found: name=%s, font=%s", m_szName.c_str(), szFont.c_str());
            m_dirty = true;
         }
         else if (key == "fonth"s)
         {
            m_size = static_cast<float>(value.as<double>());
            m_dirty = true;
         }
         else if (key == "color"s)
         {
            m_color = value.as<int>();
            m_dirty = true;
         }
         else if (key == "xalign"s)
         {
            m_xAlign = (PUP_LABEL_XALIGN)value.as<int>();
            m_dirty = true;
         }
         else if (key == "yalign"s)
         {
            m_yAlign = (PUP_LABEL_YALIGN)value.as<int>();
            m_dirty = true;
         }
         else if (key == "pagenum"s)
         {
            m_pagenum = value.as<int>();
            m_dirty = true;
         }
         else if (key == "stopani"s)
         {
            // stop any pup animations on label/image (zoom/flash/pulse).  this is not about animated gifs
            m_animation = nullptr;
            m_dirty = true;
         }
         else if (key == "rotate"s)
         {
            // in tenths.  so 900 is 90 degrees. rotate support for images too.  note images must be aligned center to rotate properly(default)
            m_angle = static_cast<float>(value.as<double>() / 10.0);
            m_dirty = true;
         }
         else if (key == "zoom"s)
         {
            // 120 for 120% of current height, 80% etc...
            NOT_IMPLEMENTED("zoom not implemented");
            m_dirty = true;
         }
         else if (key == "alpha"s)
         {
            // '0-255  255=full, 0=blank
            NOT_IMPLEMENTED("alpha not implemented");
            m_dirty = true;
         }
         else if (key == "gradstate"s)
         {
            // color=gradcolor, gradstate = 0 (gradstate is percent)
            NOT_IMPLEMENTED("gradstate/gradcolor not implemented");
            m_dirty = true;
         }
         else if (key == "gradcolor"s)
         {
            // color=gradcolor, gradstate = 0 (gradstate is percent)
            NOT_IMPLEMENTED("gradstate/gradcolor not implemented");
            m_dirty = true;
         }
         else if (key == "grayscale"s)
         {
            // only on image objects.  will show as grayscale.  1=gray filter on 0=off normal mode
            NOT_IMPLEMENTED("filter not implemented");
            m_dirty = true;
         }
         else if (key == "filter"s)
         {
            // fmode 1-5 (invertRGB, invert,grayscale,invertalpha,clear),blur)
            NOT_IMPLEMENTED("filter not implemented");
            m_dirty = true;
         }
         else if (key == "shadowstate"s)
         {
            m_shadowState = value.as<int>();
            m_dirty = true;
         }
         else if (key == "shadowcolor"s)
         {
            m_shadowColor = value.as<int>();
            m_dirty = true;
         }
         else if (key == "shadowtype"s)
         {
            // ST = 1 (Shadow), ST = 2 (Border)
            NOT_IMPLEMENTED("shadowtype not implemented");
            m_dirty = true;
         }
         else if (key == "xoffset"s)
         {
            m_xoffset = static_cast<float>(value.as<double>());
            m_dirty = true;
         }
         else if (key == "yoffset"s)
         {
            m_yoffset = static_cast<float>(value.as<double>());
            m_dirty = true;
         }
         else if (key == "anigif"s)
         {
            m_anigif = value.as<int>();
            m_dirty = true;
         }
         else if (key == "width"s)
         {
            m_imageWidth = static_cast<float>(value.as<double>());
            m_dirty = true;
         }
         else if (key == "height"s)
         {
            m_imageHeight = static_cast<float>(value.as<double>());
            m_dirty = true;
         }
         else if (key == "autow"s)
         {
            NOT_IMPLEMENTED("autow not implemented");
            m_dirty = true;
         }
         else if (key == "autoh"s)
         {
            NOT_IMPLEMENTED("autoh not implemented");
            m_dirty = true;
         }
         else if (key == "outline"s)
         {
            m_outline = value.as<int>();
            m_dirty = true;
         }
         else if (key == "bold"s)
         {
            m_bold = (value.as<int>() == 1);
            m_dirty = true;
         }
         else
         {
            NOT_IMPLEMENTED(key.c_str(), " not implemented");
         }
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

   // TTF_Font may not be accessed simultaneously from multiple thread so serialize updates using a global static mutex
   std::lock_guard lock(m_fontMutex);

   RenderState rs;
   rs.m_prerenderedHeight = outHeight;
   rs.m_prerenderedColor = color;

   const float fontHeight = (size / 100.0f) * static_cast<float>(outHeight);
   TTF_FontStyleFlags style = TTF_STYLE_NORMAL;
   // FIXME TTF_STYLE_BOLD gives bad results (test with Blood Machines, during Bonnus sequence)
   // style |= m_bold ? TTF_STYLE_BOLD : 0;
   TTF_SetFontSize(pFont, fontHeight);
   // FIXME TTF_SetFontOutline does not gives clean outlines
   // TTF_SetFontOutline(pFont, m_outline);
   TTF_SetFontOutline(pFont, 0);
   TTF_SetFontStyle(pFont, style);
   TTF_SetFontHinting(pFont, TTF_HINTING_NORMAL);

   string text = szCaption;
   std::replace_if(text.begin(), text.end(), [pFont](char c) { return !TTF_FontHasGlyph(pFont, c); }, ' ');

   SDL_Color textColor = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), 255 };
   SDL_Surface* pTextSurface = TTF_RenderText_Blended_Wrapped(pFont, text.c_str(), text.length(), textColor, 0);
   if (!pTextSurface)
   {
      LOGE("Unable to render text: label=%s, error=%s", m_szName.c_str(), SDL_GetError());
      return rs;
   }

   const auto xoffset = static_cast<int>(fontHeight * (offset.x / 100.0f));
   const auto yoffset = static_cast<int>(fontHeight * (offset.y / 100.0f));
   SDL_Surface* pMergedSurface = SDL_CreateSurface(pTextSurface->w + abs(xoffset), pTextSurface->h + abs(yoffset), SDL_PIXELFORMAT_RGBA32);
   if (!pMergedSurface)
   {
      LOGE("Unable to render text: label=%s, error=%s", m_szName.c_str(), SDL_GetError());
      return rs;
   }
   //SDL_FillSurfaceRect(pMergedSurface, NULL, SDL_MapRGBA(SDL_GetPixelFormatDetails(pMergedSurface->format), nullptr, 255, 255, 0, 255));

   if (shadowstate && (xoffset != 0 || yoffset != 0))
   { // Shadow rendering
      const SDL_Color shadowColor = { GetRValue(shadowcolor), GetGValue(shadowcolor), GetBValue(shadowcolor), 255 };
      SDL_Surface* pShadowSurface = TTF_RenderText_Blended_Wrapped(pFont, text.c_str(), text.length(), shadowColor, 0);
      if (pShadowSurface)
      {
         SDL_Rect shadowRect = { (xoffset < 0) ? 0 : xoffset, (yoffset < 0) ? 0 : yoffset, pShadowSurface->w, pShadowSurface->h };
         SDL_BlitSurface(pShadowSurface, nullptr, pMergedSurface, &shadowRect);

         SDL_Rect textRect = { (xoffset < 0) ? -xoffset : 0, (yoffset < 0) ? -yoffset : 0, pTextSurface->w, pTextSurface->h };
         SDL_BlitSurface(pTextSurface, nullptr, pMergedSurface, &textRect);
         SDL_DestroySurface(pShadowSurface);
      }
      else
      {
         LOGE("Failed to render shadow: label=%s, error=%s", m_szName.c_str(), SDL_GetError());
         SDL_Rect textRect = { (xoffset < 0) ? -xoffset : 0, (yoffset < 0) ? -yoffset : 0, pTextSurface->w, pTextSurface->h };
         SDL_BlitSurface(pTextSurface, nullptr, pMergedSurface, &textRect);
      }
   }
   else
   { // BGRA to RGBA conversion
      SDL_BlitSurface(pTextSurface, nullptr, pMergedSurface, nullptr);
   }
   SDL_DestroySurface(pTextSurface);

   rs.m_pTexture = CreateTexture(pMergedSurface);
   if (rs.m_pTexture)
   {
      rs.m_width = static_cast<float>(pMergedSurface->w);
      rs.m_height = static_cast<float>(pMergedSurface->h);
   }
   SDL_DestroySurface(pMergedSurface);
   return rs;
}

string PUPLabel::ToString() const
{
   return std::format("name={}, caption={}, visible={}, size={}, color={}, angle={}, xAlign={}, yAlign={}, xPos={}, yPos={}, pagenum={}, szPath={}", m_szName, m_szCaption,
      (m_visible ? "true" : "false"), m_size, m_color, m_angle,
      (m_xAlign == PUP_LABEL_XALIGN_LEFT          ? "LEFT"
            : m_xAlign == PUP_LABEL_XALIGN_CENTER ? "CENTER"
                                                  : "RIGHT"),
      (m_yAlign == PUP_LABEL_YALIGN_TOP           ? "TOP"
            : m_yAlign == PUP_LABEL_YALIGN_CENTER ? "CENTER"
                                                  : "BOTTOM"),
      m_xPos, m_yPos, m_pagenum, m_szPath.string());
}

PUPLabel::Animation::Animation(PUPLabel* label, int lengthMs, int foregroundColor, int flashingPeriod)
   : m_color(label->m_color)
   , m_label(label)
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
