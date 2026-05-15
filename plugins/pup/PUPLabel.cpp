// license:GPLv3+

/*
 * This code was derived from notes at:
 *
 * https://www.nailbuster.com/wikipinup/doku.php?id=pup_dmd
 * https://nailbuster.com/pupupdatesv14/PUPDMDFramework_Starter_v1_1.zip
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

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

// FIXME rotation is done via CPU (90/270 degress render blank via GPU?)
static SDL_Surface* RotateSurface(SDL_Surface* const src, float angleDeg)
{
   if (!src || angleDeg == 0.f)
      return nullptr;

   const float rad = -angleDeg * (float)(M_PI / 180.0);
   const float c = cosf(rad);
   const float s = sinf(rad);
   const float absC = fabsf(c);
   const float absS = fabsf(s);

   const unsigned int srcW = src->w;
   const unsigned int srcH = src->h;
   const int dstW = (int)ceilf(srcW * absC + srcH * absS);
   const int dstH = (int)ceilf(srcW * absS + srcH * absC);

   SDL_Surface* const dst = SDL_CreateSurface(dstW, dstH, SDL_PIXELFORMAT_RGBA32);
   if (!dst)
      return nullptr;

   SDL_LockSurface(src);
   SDL_LockSurface(dst);

   const uint32_t* const __restrict srcPixels = (const uint32_t*)src->pixels;
   uint32_t* const __restrict dstPixels = (uint32_t*)dst->pixels;
   const int srcPitch = src->pitch / 4;
   const int dstPitch = dst->pitch / 4;

   const float srcCx = srcW * 0.5f;
   const float srcCy = srcH * 0.5f;
   const float dstCx = dstW * 0.5f;
   const float dstCy = dstH * 0.5f;

   for (int dy = 0; dy < dstH; dy++)
   {
      for (int dx = 0; dx < dstW; dx++)
      {
         const float rx = (dx - dstCx) * c + (dy - dstCy) * s + srcCx;
         const float ry = -(dx - dstCx) * s + (dy - dstCy) * c + srcCy;
         const int sx = (int)rx;
         const int sy = (int)ry;
         if (/*sx >= 0 &&*/ (unsigned int)sx < srcW && /*sy >= 0 &&*/ (unsigned int)sy < srcH)
            dstPixels[dy * dstPitch + dx] = srcPixels[sy * srcPitch + sx];
         else
            dstPixels[dy * dstPitch + dx] = 0;
      }
   }

   SDL_UnlockSurface(dst);
   SDL_UnlockSurface(src);
   return dst;
}

// See pDMDLabelSetFilter — apply pixel filter to an RGBA32 surface
// fmode: 1=invertRGB, 2=invert(+alpha), 3=grayscale, 4=invertalpha, 5=clear
static void ApplyFilter(SDL_Surface* surf, int filterMode)
{
   if (!surf || filterMode <= 0 || filterMode > 5)
      return;

   SDL_LockSurface(surf);
   uint32_t* const pixels = (uint32_t*)surf->pixels;
   const int pitch = surf->pitch / 4;
   const int w = surf->w;
   const int h = surf->h;

   for (int y = 0; y < h; y++)
   {
      for (int x = 0; x < w; x++)
      {
         uint32_t& px = pixels[y * pitch + x];
         const uint8_t r = px & 0xFF;
         const uint8_t g = (px >> 8) & 0xFF;
         const uint8_t b = (px >> 16) & 0xFF;
         const uint8_t a = (px >> 24) /*& 0xFF*/;

         switch (filterMode)
         {
         case 1: // invertRGB
            px = (a << 24) | ((255 - b) << 16) | ((255 - g) << 8) | (255 - r);
            break;
         case 2: // invert (RGB + alpha)
            px = ((255 - a) << 24) | ((255 - b) << 16) | ((255 - g) << 8) | (255 - r);
            break;
         case 3: // grayscale
         {
            const uint8_t gray = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
            px = (a << 24) | (gray << 16) | (gray << 8) | gray;
            break;
         }
         case 4: // invertalpha
            px = ((255 - a) << 24) | (b << 16) | (g << 8) | r;
            break;
         case 5: // clear
            px = 0;
            break;
         }
      }
   }
   SDL_UnlockSurface(surf);
}

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
      LOGE("Font not found: label=" + szName + ", font=" + szFont);
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
         const bool wasImage = (m_type == PUP_LABEL_TYPE_IMAGE || m_type == PUP_LABEL_TYPE_GIF);
         m_type = PUP_LABEL_TYPE_TEXT;
         m_szPath.clear();

         const string szExt = extension_from_path(szText);
         if (szExt == "gif" || szExt == "png" || szExt == "apng" || szExt == "bmp" || szExt == "jpg" || szExt == "jpeg")
         {
            std::filesystem::path fs_path(normalize_path_separators(szText));
            string playlistFolder = fs_path.parent_path().string();
            PUPPlaylist* pPlaylist = m_pScreen->GetPlaylist(playlistFolder);
            std::filesystem::path szPath;
            if (pPlaylist)
               szPath = pPlaylist->GetPlayFilePath(fs_path.filename());
            if (!szPath.empty())
            {
               m_szPath = szPath;
               m_type = (szExt == "gif") ? PUP_LABEL_TYPE_GIF : PUP_LABEL_TYPE_IMAGE;
            }
            else
            {
               // Many tables use clear.png / clear1.png as a "clear this label" caption without
               // ever shipping the file in the pup pack. Drop the caption so nothing renders, and
               // if we had previously loaded an image, hide the label so a later valid image
               // doesn't silently reappear.
               LOGE(std::format("Image not found, hiding label: screen={}, label={}, path={}",
                  m_pScreen->GetScreenNum(), m_szName, szText));
               if (wasImage)
                  m_visible = false;
               m_szCaption.clear();
               m_dirty = true;
               return;
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
   LOGD(std::format("PUPLabel::SetSpecial: name={}, caption={}, json={}", m_szName, m_szCaption, szSpecial));
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
            // See pDMDLabelFlash — fq is the toggle interval in milliseconds, fc is optional
            if (json["len"s].exists() && json["fq"s].exists() && json["fq"s].as<int>() > 0)
               m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(), json["fc"s].as<int>(m_color), json["fq"s].as<int>());
            else
            {
               LOGE("Invalid label animation specified: {" + szSpecial + '}');
            }
            break;
            
         case 2:
            // See pDMDLabelMoveHorz / pDMDLabelMoveVert / pDMDLabelMoveTO
            // See pDMDLabelMoveHorzFade / pDMDLabelMoveVertFade — af = alpha fade start time (ms before end)
            if (json["len"s].exists())
            {
               int len = json["len"s].as<int>();
               int mlen = json["mlen"s].as<int>(0);
               if (mlen == 0) mlen = len;
               m_animation = std::make_unique<Animation>(this, len, json["fc"s].as<int>(m_color),
                  json["xps"s].as<int>(0), json["xpe"s].as<int>(0), json["yps"s].as<int>(0), json["ype"s].as<int>(0),
                  mlen, json["tt"s].as<int>(0), json["mColor"s].as<int>(m_color));
               m_animation->m_alphaFade = json["af"s].as<int>(0);
            }
            else
            {
               LOGE("Invalid label animation specified: {" + szSpecial + '}');
            }
            break;

         case 3:
         {
            // See pDMDZoomBig — zoom from hstart% to hend% over len ms
            float hstart = static_cast<float>(json["hstart"s].as<double>(100));  // default 100% (normal size)
            float hend = static_cast<float>(json["hend"s].as<double>(200));    // default 200% (double size)
            m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(1000), json["fc"s].as<int>(m_color), hstart, hend, 0);  // 1000ms fallback
            break;
         }

         case 4:
         {
            // See pDMDLabelPulseText / pDMDLabelPulseImage — bounce zoom between hstart% and hend%
            // See pDMDLabelPulseNumber — numstart/numend count up during animation.
            // pspeed=0 means use a default bounce speed derived from the zoom range.
            float hstart = static_cast<float>(json["hstart"s].as<double>(80));   // default 80% from pDMDLabelPulseText
            float hend = static_cast<float>(json["hend"s].as<double>(120));      // default 120% from pDMDLabelPulseText
            int pspeed = json["pspeed"s].as<int>(0);
            if (pspeed == 0)
               pspeed = std::max(1, static_cast<int>(hend - hstart));
            m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(3000), json["fc"s].as<int>(m_color), hstart, hend, pspeed);  // 3000ms fallback if len missing
            m_animation->m_numStart = json["numstart"s].as<int>(INT_MIN);   // INT_MIN = no number counting
            m_animation->m_numEnd = json["numend"s].as<int>(INT_MIN);
            m_animation->m_numFormat = json["numformat"s].as<int>(0);       // 0=plain, 1=thousands separators
            break;
         }

         case 5:
         {
            // See pDMDLabelFadeOut / pDMDLabelFadeIn — alpha ease from astart to aend over len ms
            int astart = json["astart"s].as<int>(255);   // default 255 (fully visible) from pDMDLabelFadeOut
            int aend = json["aend"s].as<int>(0);          // default 0 (invisible) from pDMDLabelFadeOut
            m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(1000), json["fc"s].as<int>(m_color), astart, aend, 0, false);  // 1000ms fallback
            break;
         }

         case 6:
         {
            // See pDMDLabelFadePulse — alpha oscillates between astart and aend
            int astart = json["astart"s].as<int>(70);    // default 70 (dim) from pDMDLabelFadePulse
            int aend = json["aend"s].as<int>(255);        // default 255 (full) from pDMDLabelFadePulse
            int pspeed = json["pspeed"s].as<int>(40);     // default 40 (alpha units per tick) from pDMDLabelFadePulse
            m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(3000), json["fc"s].as<int>(m_color), astart, aend, pspeed, false);  // 3000ms fallback
            break;
         }

         case 7:
         {
            // See pDMDScreenFadeOut / pDMDScreenFadeIn — same as at=5 but for parent screen
            int astart = json["astart"s].as<int>(255);   // default 255 from pDMDScreenFadeOut
            int aend = json["aend"s].as<int>(0);          // default 0 from pDMDScreenFadeOut
            m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(1000), json["fc"s].as<int>(m_color), astart, aend, 0, true);  // 1000ms fallback
            break;
         }

         case 8:
         {
            // See pDMDLabelWiggleText / pDMDLabelWiggleImage — rotation bounces between rstart and rend
            // Uses GPU rotation since wiggle angles are small (never hits 90°/270°).
            float rstart = static_cast<float>(json["rstart"s].as<double>(-45)) / 10.0f;  // default -4.5 deg from pDMDLabelWiggleText
            float rend = static_cast<float>(json["rend"s].as<double>(45)) / 10.0f;       // default +4.5 deg from pDMDLabelWiggleText
            int rspeed = json["rspeed"s].as<int>(5);   // default 5 from pDMDLabelWiggleText, 0 = use range as speed
            if (rspeed == 0)
               rspeed = std::max(1, static_cast<int>(fabsf(rend - rstart)));
            m_animation = std::make_unique<Animation>(this, json["len"s].as<int>(3000), json["fc"s].as<int>(m_color), rstart, rend, rspeed, 0);  // 3000ms fallback
            break;
         }

         case 10:
            // See pDMDLabelClone / pDMDLabelCloneDelay — delayed show.
            // True cloning (creating a duplicate label) is not yet implemented.
            NOT_IMPLEMENTED("at=10 clone not implemented"s);
            break;

         default:
            LOGE("Unsupported Label.SetSpecial animation type: " + std::to_string(json["at"s].as<int>(0)));
            break;
         }
      }
      break;

   case 2:
   {
      std::lock_guard lock(m_mutex);
      for (auto& [key, value] : json.as_object())
      {
         if (key == "mt")
         {

         }
         else if (key == "zback")
         {
            // See pDMDLabelSendToBack
            if (value.as<int>() == 1)
               m_pScreen->SendLabelToBack(this);
         }
         else if (key == "ztop")
         {
            // See pDMDLabelSendToFront
            if (value.as<int>() == 1)
               m_pScreen->SendLabelToFront(this);
         }
         else if (key == "size")
         {
            // See pDMDLabelSetSizeText — height as percent of display height
            const float newSize = static_cast<float>(json["size"s].as<double>());
            if (m_size != newSize)
            {
               m_size = newSize;
               m_dirty = true;
            }
         }
         else if (key == "xpos")
         {
            // See pDMDLabelSetPos — position as percent of display width/height
            m_xPos = static_cast<float>(value.as<double>());
         }
         else if (key == "ypos")
         {
            m_yPos = static_cast<float>(value.as<double>());
         }
         else if (key == "fname")
         {
            string szFont = value.as_str();
            PUPFont* newFont = m_pManager->GetFont(szFont);
            if (!newFont)
               LOGE("Label font not found: name=" + m_szName + ", font=" + szFont);
            if (newFont != m_pFont)
            {
               m_pFont = newFont;
               m_dirty = true;
            }
         }
         else if (key == "fonth")
         {
            const float newSize = static_cast<float>(value.as<double>());
            if (m_size != newSize)
            {
               m_size = newSize;
               m_dirty = true;
            }
         }
         else if (key == "color")
         {
            // See pDMDLabelSetColor
            const int newColor = value.as<int>();
            if (m_color != newColor)
            {
               m_color = newColor;
               m_dirty = true;
            }
         }
         else if (key == "xalign")
         {
            // See PDMDLabelSetAlign — 0=left 1=center 2=right, 0=top 1=center 2=bottom
            // Dirty because TTF_SetFontWrapAlignment uses xAlign for multi-line text justification
            const PUP_LABEL_XALIGN newXAlign = (PUP_LABEL_XALIGN)value.as<int>();
            if (m_xAlign != newXAlign)
            {
               m_xAlign = newXAlign;
               m_dirty = true;
            }
         }
         else if (key == "yalign")
         {
            m_yAlign = (PUP_LABEL_YALIGN)value.as<int>();
         }
         else if (key == "pagenum")
         {
            m_pagenum = value.as<int>();
         }
         else if (key == "stopani")
         {
            // See pDMDLabelStopAnis — stop any pup animations on label/image (zoom/flash/pulse), not animated gifs
            m_animation = nullptr;
            m_alpha = 1.0f;
            m_zoom = 100.0f;
            m_gradState = 0;
            m_filterMode = 0;
            m_autoFitWidth = 0;
            m_autoFitHeight = 0;
            m_dirty = true;
         }
         else if (key == "rotate")
         {
            // See pDMDLabelSetRotate — in tenths, so 900 is 90 degrees. rotate support for images too.
            // note images must be aligned center to rotate properly(default)
            const float newAngle = static_cast<float>(value.as<double>() / 10.0);
            if (m_angle != newAngle)
            {
               m_angle = newAngle;
               m_dirty = true;
            }
         }
         else if (key == "zoom")
         {
            // See pDMDLabelSetZoom — 120 for 120% of current height, 80% etc...
            m_zoom = static_cast<float>(value.as<double>());
         }
         else if (key == "alpha")
         {
            // See pDMDLabelSetAlpha — 0-255  255=full, 0=blank
            m_alpha = saturate((float)value.as<int>() / 255.0f);
         }
         else if (key == "gradstate")
         {
            // See pDMDLabelSetColorGradient — gradstate controls gradient blend (0=off, 1+=on)
            const int newGradState = value.as<int>();
            if (m_gradState != newGradState)
            {
               m_gradState = newGradState;
               m_dirty = true;
            }
         }
         else if (key == "gradcolor")
         {
            // See pDMDLabelSetColorGradientPercent — end color of the vertical gradient
            const int newGradColor = value.as<int>();
            if (m_gradColor != newGradColor)
            {
               m_gradColor = newGradColor;
               m_dirty = true;
            }
         }
         else if (key == "grayscale")
         {
            // See pDMDLabelSetGrayScale — shortcut for filter mode 3 on images. 1=on, 0=off
            const int newFilterMode = (value.as<int>() == 1) ? 3 : 0;
            if (m_filterMode != newFilterMode)
            {
               m_filterMode = newFilterMode;
               m_dirty = true;
            }
         }
         else if (key == "filter")
         {
            // See pDMDLabelSetFilter — fmode 1=invertRGB, 2=invert, 3=grayscale, 4=invertalpha, 5=clear/blur
            const int newFilterMode = value.as<int>();
            if (m_filterMode != newFilterMode)
            {
               m_filterMode = newFilterMode;
               m_dirty = true;
            }
         }
         else if (key == "shadowstate")
         {
            // See pDMDLabelSetShadow / pDMDLabelSetBorder / pDMDLabelSetOutShadow
            // shadowstate is a simple on/off: 0 disables shadow, non-zero enables drop shadow (type 1)
            const int newShadowType = (value.as<int>() != 0) ? 1 : 0;
            if (m_shadowType != newShadowType)
            {
               m_shadowType = newShadowType;
               m_dirty = true;
            }
         }
         else if (key == "shadowcolor")
         {
            const int newShadowColor = value.as<int>();
            if (m_shadowColor != newShadowColor)
            {
               m_shadowColor = newShadowColor;
               m_dirty = true;
            }
         }
         else if (key == "shadowtype")
         {
            // shadowtype sets the rendering mode directly: 0=none, 1=drop shadow, 2=border
            const int newShadowType = value.as<int>();
            if (m_shadowType != newShadowType)
            {
               m_shadowType = newShadowType;
               m_dirty = true;
            }
         }
         else if (key == "xoffset")
         {
            // See pDMDLabelSetShadow / pDMDLabelSetBorder — shadow displacement as % of font height
            const float newXOffset = static_cast<float>(value.as<double>());
            if (m_xoffset != newXOffset)
            {
               m_xoffset = newXOffset;
               m_dirty = true;
            }
         }
         else if (key == "yoffset")
         {
            const float newYOffset = static_cast<float>(value.as<double>());
            if (m_yoffset != newYOffset)
            {
               m_yoffset = newYOffset;
               m_dirty = true;
            }
         }
         else if (key == "anigif")
         {
            // See pDMDPNGAnimate — speed is frame timer, 100 is ~30fps
            m_anigif = value.as<int>();
            m_animating = m_anigif > 0;
            m_dirty = true;
         }
         else if (key == "animate")
         {
            // See pDMDPNGAnimate / pDMDPNGAnimateOnce — animation interval in ms, 0 stops
            int interval = value.as<int>();
            if (interval < 1)
            {
               m_animating = false;
            }
            else
            {
               m_visible = true;
               m_anigif = interval;
               m_animating = true;
               m_dirty = true;
            }
         }
         else if (key == "anistart")
         {
            int interval = value.as<int>();
            m_anigif = interval;
            m_animating = interval > 0;
            m_dirty = true;
         }
         else if (key == "gifloop")
         {
            // See pDMDPNGAnimateEx — 1=loop forever (default), 0=play once
            m_gifLoop = (value.as<int>() == 1);
         }
         else if (key == "gifstart")
         {
            // See pDMDPNGAnimateEx / pDMDPNGShowFrame — start frame index, -1 resets to full range
            m_gifStart = value.as<int>();
            if (m_gifStart < 0)
            {
               m_gifStart = 0;
               m_gifEnd = -1;
            }
            m_animationFrame = -1;
            m_animationStart = SDL_GetTicks();
         }
         else if (key == "gifend")
         {
            // See pDMDPNGAnimateEx / pDMDPNGShowFrame — end frame index, start==end shows single frame
            m_gifEnd = value.as<int>();
            if (m_gifStart == m_gifEnd && m_gifEnd >= 0)
               m_dirty = true;
         }
         else if (key == "aniendhide")
         {
            // See pDMDPNGAnimateOnce — 1=hide label when animation completes
            m_hideOnAnimEnd = (value.as<int>() == 1);
         }
         else if (key == "anidispose")
         {
            // See pDMDPNGAnimateOnceAndDispose — marks for cleanup after play
         }
         else if (key == "width")
         {
            // See pDMDLabelSetSizeImage — width/height as percent of display dimensions
            const float newWidth = static_cast<float>(value.as<double>());
            if (m_imageWidth != newWidth)
            {
               m_imageWidth = newWidth;
               m_dirty = true;
            }
         }
         else if (key == "height")
         {
            const float newHeight = static_cast<float>(value.as<double>());
            if (m_imageHeight != newHeight)
            {
               m_imageHeight = newHeight;
               m_dirty = true;
            }
         }
         else if (key == "autow")
         {
            // See pDMDLabelSetAutoSize — auto-shrink font to fit within specified dimensions
            const float newAutoW = static_cast<float>(value.as<double>());
            if (m_autoFitWidth != newAutoW)
            {
               m_autoFitWidth = newAutoW;
               m_dirty = true;
            }
         }
         else if (key == "autoh")
         {
            const float newAutoH = static_cast<float>(value.as<double>());
            if (m_autoFitHeight != newAutoH)
            {
               m_autoFitHeight = newAutoH;
               m_dirty = true;
            }
         }
         else if (key == "outline")
         {
            // See pDMDLabelSetBorder / pDMDLabelSetOutShadow — outline=1 upgrades shadow to border mode (type 2)
            const int newOutline = value.as<int>();
            const int newShadowType = (newOutline == 1) ? 2 : m_shadowType;
            if (m_outline != newOutline || m_shadowType != newShadowType)
            {
               m_outline = newOutline;
               m_shadowType = newShadowType;
               m_dirty = true;
            }
         }
         else if (key == "bold")
         {
            const bool newBold = (value.as<int>() == 1);
            if (m_bold != newBold)
            {
               m_bold = newBold;
               m_dirty = true;
            }
         }
         else if (key == "italic")
         {
            const bool newItalic = (value.as<int>() == 1);
            if (m_italic != newItalic)
            {
               m_italic = newItalic;
               m_dirty = true;
            }
         }
         else if (key == "v2")
         {
            // See pupCreateLabel / pupCreateLabelImage
            // v2 enables absolute positioning: xpos/ypos=0 means literally 0%,
            // disabling the default behavior where 0 maps to center/right/bottom edge
            if (!m_useAbsolutePos)
            {
               m_useAbsolutePos = true;
               m_dirty = true;
            }
         }
         else
         {
            NOT_IMPLEMENTED(key + " not implemented");
         }
      }
   }
      break;

   case 0:
      // mt=0 or missing mt
      // Batman66 sends mt=0 which spams the log
      break;

   default:
      LOGE("Unsupported Label.SetSpecial mt mode: " + std::to_string(json["mt"s].as<int>(0)));
      break;
   }
}

void PUPLabel::Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect, int pagenum, float screenAlpha)
{
   std::lock_guard lock(m_mutex);

   if ((!m_visible && !m_animation) || pagenum != m_pagenum || m_szCaption.empty())
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
      {
         m_pendingTextureUpdate = std::async(std::launch::async, [this]() {
            std::lock_guard lock(m_mutex);
            return UpdateImageTexture(m_type, m_szPath);
         });
      }
      else if (m_pFont)
         m_pendingTextureUpdate = std::async(std::launch::async, [this, rect, fontColor]() {
            std::lock_guard lock(m_mutex);
            return UpdateLabelTexture(rect.h, m_pFont, m_szCaption, m_size, fontColor, m_shadowType, m_shadowColor, { m_xoffset, m_yoffset} );
         });
   }

   if (!m_renderState.m_pTexture)
      return;

   if (m_renderState.m_pAnimation)
   {
      const int frameCount = m_renderState.m_pAnimation->count;
      const int startFrame = (m_gifStart >= 0 && m_gifStart < frameCount) ? m_gifStart : 0;
      const int endFrame = (m_gifEnd >= 0 && m_gifEnd < frameCount) ? m_gifEnd : frameCount - 1;

      int expectedFrame = startFrame;
      if (startFrame != endFrame)
      {
         int elapsed = static_cast<int>(SDL_GetTicks() - m_animationStart);
         float speed = m_anigif > 0 ? m_anigif / 100.f : 1.f;
         elapsed = static_cast<int>(elapsed * speed);
         const int startOffset = startFrame > 0 ? m_renderState.m_accumulatedDelays[startFrame - 1] : 0;
         const int rangeDuration = m_renderState.m_accumulatedDelays[endFrame] - startOffset;
         if (rangeDuration > 0)
         {
            if (m_gifLoop)
            {
               elapsed = elapsed % rangeDuration;
            }
            else if (elapsed >= rangeDuration)
            {
               elapsed = rangeDuration - 1;
               if (m_animating)
               {
                  m_animating = false;
                  if (m_hideOnAnimEnd)
                     m_visible = false;
               }
            }
         }
         const auto& delays = m_renderState.m_accumulatedDelays;
         expectedFrame = static_cast<int>(std::upper_bound(delays.begin() + startFrame, delays.begin() + endFrame + 1, startOffset + elapsed) - delays.begin());
         if (expectedFrame > endFrame)
            expectedFrame = endFrame;
         if (expectedFrame < startFrame)
            expectedFrame = startFrame;
      }
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

   float xposPercent = m_xPos / 100.0f;
   if (m_xPos == 0.f && !m_useAbsolutePos)
   {
      if (m_xAlign == PUP_LABEL_XALIGN_CENTER)
         xposPercent = 0.5f;
      else if (m_xAlign == PUP_LABEL_XALIGN_RIGHT)
         xposPercent = 1.0f;
   }

   dest.x += static_cast<float>(rect.w) * xposPercent;

   if (m_xAlign == PUP_LABEL_XALIGN_CENTER)
      dest.x -= (width / 2.f);
   else if (m_xAlign == PUP_LABEL_XALIGN_RIGHT)
      dest.x -= width;

   float yposPercent = m_yPos / 100.0f;
   if (m_yPos == 0.f && !m_useAbsolutePos)
   {
      if (m_yAlign == PUP_LABEL_YALIGN_CENTER)
         yposPercent = 0.5f;
      else if (m_yAlign == PUP_LABEL_YALIGN_BOTTOM)
         yposPercent = 1.0f;
   }

   const float yPosOffset = static_cast<float>(rect.h) * yposPercent;
   dest.y += yPosOffset;

   const float alignHeight = (m_type == PUP_LABEL_TYPE_TEXT && m_renderState.m_logicalHeight > 0) ? m_renderState.m_logicalHeight : height;
   float alignOffset = 0.f;
   if (m_yAlign == PUP_LABEL_YALIGN_CENTER)
      alignOffset = -(alignHeight / 2.f);
   else if (m_yAlign == PUP_LABEL_YALIGN_BOTTOM)
      alignOffset = -alignHeight;
   dest.y += alignOffset;

   if (m_type == PUP_LABEL_TYPE_TEXT)
      dest.y += m_renderState.m_baselineOffset;

   if (m_shadowType == 2)
   {
      const float fontHeight = (m_size / 100.0f) * static_cast<float>(rect.h);
      dest.x += static_cast<float>(static_cast<int>(fontHeight * (m_xoffset / 100.0f))) * -2.f;
   }

   bool visible = true;
   int color = 0xFFFFFFFF;
   if (m_animation)
   {
      if (m_animation->Update(rect, dest))
      {
         if (m_animation->IsScreenFade() && m_pScreen)
            m_pScreen->m_screenAlpha = m_animation->m_alpha;
         else if (m_animation->IsFade())
            m_alpha = m_animation->m_alpha;
         if (m_animation->IsZoom())
            m_zoom = m_animation->m_zoom;
         if (m_animation->m_alphaFade > 0)
            m_alpha = 0.f;
         // Persist final motion position so label stays where the animation left it
         m_xPos += (m_animation->m_xOffset / static_cast<float>(rect.w)) * 100.f;
         m_yPos += (m_animation->m_yOffset / static_cast<float>(rect.h)) * 100.f;
         m_animation = nullptr;
         return;
      }
      else
      {
         // See pDMDScreenFadeOut / pDMDScreenFadeIn — at=7 fades the entire screen
         if (m_animation->IsScreenFade() && m_pScreen)
            m_pScreen->m_screenAlpha = m_animation->m_alpha;

         dest.x += m_animation->m_xOffset;
         dest.y += m_animation->m_yOffset;
         visible = m_animation->m_visible;
         color = m_animation->m_color;
      }
   }

   float alpha = (m_animation ? m_animation->m_alpha : m_alpha) * screenAlpha;

   float zoom = m_animation ? m_animation->m_zoom : m_zoom;
   if (zoom != 100.0f)
   {
      const float scale = zoom / 100.0f;
      const float cx = dest.x + dest.w * 0.5f;
      const float cy = dest.y + dest.h * 0.5f;
      dest.w *= scale;
      dest.h *= scale;
      dest.x = cx - dest.w * 0.5f;
      dest.y = cy - dest.h * 0.5f;
   }

   if (visible && alpha > 0.f)
   {
      // FIXME implement color (as the animation may animate it)
      // FIXME rotation is done via CPU (90/270 degress render blank via GPU?)
      // Wiggle animation uses GPU rotation since the angles are small
      const VPXTextureInfo* texInfo = GetTextureInfo(m_renderState.m_pTexture);
      const float drawAngle = (m_animation && m_animation->IsWiggle()) ? m_animation->m_wiggle : 0.f;
      const float pivotX = (drawAngle != 0.f) ? static_cast<float>(texInfo->width) * 0.5f : 0.f;
      const float pivotY = (drawAngle != 0.f) ? static_cast<float>(texInfo->height) * 0.5f : 0.f;
      ClipDrawImage(ctx, m_renderState.m_pTexture, 1.f, 1.f, 1.f, alpha,
         0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height),
         pivotX, pivotY, drawAngle, dest, rect);
   }
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
         if (m_filterMode > 0)
            ApplyFilter(image, m_filterMode);
         rs.m_pTexture = CreateTexture(image);
         SDL_DestroySurface(image);
      }
      else
      {
         LOGE("Unable to load image: " + szPath.string());
      }
   }
   else if (type == PUP_LABEL_TYPE_GIF)
   {
      rs.m_pAnimation = std::shared_ptr<IMG_Animation>(IMG_LoadAnimation(szPath.string().c_str()), IMG_FreeAnimation);
      if (rs.m_pAnimation) {
         rs.m_accumulatedDelays.resize(rs.m_pAnimation->count);
         int accum = 0;
         for (int i = 0; i < rs.m_pAnimation->count; i++)
         {
            accum += rs.m_pAnimation->delays[i];
            rs.m_accumulatedDelays[i] = accum;
         }
         rs.m_totalDuration = accum;
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
         LOGE("Unable to load animation: " + szPath.string());
   }
   return rs;
}

PUPLabel::RenderState PUPLabel::UpdateLabelTexture(int outHeight, PUPFont* pFont, const string& szCaption, float size, int color, int shadowstate, int shadowcolor, SDL_FPoint offset)
{
   SetThreadName("PUPLabel.Upd." + m_szName);

   // TTF_Font may not be accessed simultaneously from multiple thread so serialize updates using a global static mutex
   std::lock_guard lock(m_fontMutex);

   RenderState rs;
   rs.m_prerenderedHeight = outHeight;
   rs.m_prerenderedColor = color;

   const float fontHeight = (size / 100.0f) * static_cast<float>(outHeight);
   TTF_FontStyleFlags style = TTF_STYLE_NORMAL;
   // FIXME TTF_STYLE_BOLD gives bad results (test with Blood Machines, during Bonus sequence)
   // style |= m_bold ? TTF_STYLE_BOLD : 0;
   if (m_italic) style |= TTF_STYLE_ITALIC;

   const PUPFont::SizeMetrics metrics = pFont->Apply(fontHeight);
   TTF_Font* const pTTFFont = pFont->GetTTFFont();

   const float shadowTop = (m_shadowType == 2) ? static_cast<float>(SDL_max(1, static_cast<int>(SDL_max(std::abs(fontHeight * (m_xoffset / 100.0f)), std::abs(fontHeight * (m_yoffset / 100.0f)))) - 2)) : 0.f;
   rs.m_baselineOffset = metrics.winAscent - metrics.sdlAscent - shadowTop;

   // FIXME TTF_SetFontOutline does not gives clean outlines
   // TTF_SetFontOutline(pTTFFont, m_outline);
   TTF_SetFontOutline(pTTFFont, 0);
   TTF_SetFontStyle(pTTFFont, style);
   TTF_SetFontHinting(pTTFFont, TTF_HINTING_NORMAL);
   TTF_SetFontWrapAlignment(pTTFFont,
      m_xAlign == PUP_LABEL_XALIGN_CENTER ? TTF_HORIZONTAL_ALIGN_CENTER :
      m_xAlign == PUP_LABEL_XALIGN_RIGHT  ? TTF_HORIZONTAL_ALIGN_RIGHT :
                                             TTF_HORIZONTAL_ALIGN_LEFT);

   string text = szCaption;
   if (m_pScreen && m_pScreen->m_padTextAlways)
   {
      // FN=46 / pDMDAlwaysPAD: ensure caption and each newline are bracketed by spaces.
      if (text.empty() || text.front() != ' ')
         text.insert(0, " ");
      if (text.back() != ' ')
         text += ' ';
      for (size_t pos = text.find('\n'); pos != string::npos; pos = text.find('\n', pos + 2))
      {
         if (pos > 0 && text[pos - 1] != ' ')
         {
            text.insert(pos, " ");
            pos++;
         }
         if (pos + 1 >= text.size() || text[pos + 1] != ' ')
            text.insert(pos + 1, " ");
      }
   }
   std::replace_if(text.begin(), text.end(), [pTTFFont](char c) { return c != '\n' && !TTF_FontHasGlyph(pTTFFont, c); }, ' ');

   const int lineCount = 1 + static_cast<int>(std::count(text.begin(), text.end(), '\n'));
   rs.m_logicalHeight = fontHeight * static_cast<float>(lineCount);

   // See pDMDLabelSetAutoSize — shrink font until text fits within autow/autoh dimensions
   const float maxW = (m_autoFitWidth > 0) ? (m_autoFitWidth / 100.0f) * static_cast<float>(outHeight) * (16.0f / 9.0f) : 0;
   const float maxH = (m_autoFitHeight > 0) ? (m_autoFitHeight / 100.0f) * static_cast<float>(outHeight) : 0;
   float adjustedHeight = fontHeight;

   SDL_Color textColor = { GetRValue(m_color), GetGValue(m_color), GetBValue(m_color), 255 };
   SDL_Surface* pTextSurface = TTF_RenderText_Blended_Wrapped(pTTFFont, text.c_str(), text.length(), textColor, 0);
   if (!pTextSurface)
   {
      LOGE("Unable to render text: label=" + m_szName + ", error=" + SDL_GetError());
      return rs;
   }

   while (pTextSurface && (maxW > 0 || maxH > 0))
   {
      bool fits = true;
      if (maxW > 0 && pTextSurface->w > maxW) fits = false;
      if (maxH > 0 && pTextSurface->h > maxH) fits = false;
      if (fits || adjustedHeight < 4.f) break;
      adjustedHeight -= 2.f;
      pFont->Apply(adjustedHeight);
      SDL_DestroySurface(pTextSurface);
      pTextSurface = TTF_RenderText_Blended_Wrapped(pTTFFont, text.c_str(), text.length(), textColor, 0);
   }

   {
      SDL_Surface* pConverted = SDL_ConvertSurface(pTextSurface, SDL_PIXELFORMAT_RGBA32);
      if (pConverted)
      {
         SDL_DestroySurface(pTextSurface);
         pTextSurface = pConverted;
      }
   }

   if (m_gradState > 0)
   {
      SDL_LockSurface(pTextSurface);
      uint32_t* const pixels = (uint32_t*)pTextSurface->pixels;
      const int pitch = pTextSurface->pitch / 4;
      const int h = pTextSurface->h;
      for (int y = 0; y < h; y++)
      {
         const float t = (float)y / (float)(h > 1 ? h - 1 : 1);
         const uint8_t gr = GetRValue(m_color) + static_cast<uint8_t>(t * (GetRValue(m_gradColor) - GetRValue(m_color)));
         const uint8_t gg = GetGValue(m_color) + static_cast<uint8_t>(t * (GetGValue(m_gradColor) - GetGValue(m_color)));
         const uint8_t gb = GetBValue(m_color) + static_cast<uint8_t>(t * (GetBValue(m_gradColor) - GetBValue(m_color)));
         for (int x = 0; x < pTextSurface->w; x++)
         {
            uint32_t& px = pixels[y * pitch + x];
            const uint8_t a = (px >> 24) /*& 0xFF*/;
            if (a > 0)
               px = (a << 24) | (gb << 16) | (gg << 8) | gr;
         }
      }
      SDL_UnlockSurface(pTextSurface);
   }

   const int xoffset = static_cast<int>(fontHeight * (offset.x / 100.0f));
   const int yoffset = static_cast<int>(fontHeight * (offset.y / 100.0f));
   const int absxoff = abs(xoffset);
   const int absyoff = abs(yoffset);

   if (shadowstate == 2 && (absxoff != 0 || absyoff != 0))
   {
      // See pDMDLabelSetBorder — render foreground text inside a true outline glyph
      // produced by TTF_SetFontOutline.
      const int outline = SDL_max(1, SDL_max(absxoff, absyoff) - 2);
      const SDL_Color shadowColor = { GetRValue(shadowcolor), GetGValue(shadowcolor), GetBValue(shadowcolor), 255 };
      TTF_SetFontOutline(pTTFFont, outline);
      SDL_Surface* pOutlineSurface = TTF_RenderText_Blended_Wrapped(pTTFFont, text.c_str(), text.length(), shadowColor, 0);
      TTF_SetFontOutline(pTTFFont, 0);

      if (!pOutlineSurface)
      {
         LOGE("Unable to render outline: label=" + m_szName + ", error=" + SDL_GetError());
         SDL_DestroySurface(pTextSurface);
         return rs;
      }

      SDL_Surface* pMergedSurface = SDL_ConvertSurface(pOutlineSurface, SDL_PIXELFORMAT_RGBA32);
      SDL_DestroySurface(pOutlineSurface);
      if (!pMergedSurface)
      {
         LOGE("Unable to convert outline: label=" + m_szName + ", error=" + SDL_GetError());
         SDL_DestroySurface(pTextSurface);
         return rs;
      }

      SDL_Rect textRect = { outline, outline, pTextSurface->w, pTextSurface->h };
      SDL_BlitSurface(pTextSurface, nullptr, pMergedSurface, &textRect);
      SDL_DestroySurface(pTextSurface);

      if (m_filterMode > 0)
         ApplyFilter(pMergedSurface, m_filterMode);

      if (m_angle != 0.f)
      {
         SDL_Surface* pRotated = RotateSurface(pMergedSurface, m_angle);
         if (pRotated)
         {
            SDL_DestroySurface(pMergedSurface);
            pMergedSurface = pRotated;
         }
      }

      rs.m_pTexture = CreateTexture(pMergedSurface);
      if (rs.m_pTexture)
      {
         rs.m_width = static_cast<float>(pMergedSurface->w);
         rs.m_height = static_cast<float>(pMergedSurface->h);
      }
      SDL_DestroySurface(pMergedSurface);
      return rs;
   }

   SDL_Surface* pMergedSurface = SDL_CreateSurface(pTextSurface->w + absxoff, pTextSurface->h + absyoff, SDL_PIXELFORMAT_RGBA32);
   if (!pMergedSurface)
   {
      LOGE("Unable to render text: label=" + m_szName + ", error=" + SDL_GetError());
      SDL_DestroySurface(pTextSurface);
      return rs;
   }

   if (shadowstate && (xoffset != 0 || yoffset != 0))
   {
      // See pDMDLabelSetShadow — drop shadow: render shadow text at offset, foreground at origin
      const SDL_Color shadowColor = { GetRValue(shadowcolor), GetGValue(shadowcolor), GetBValue(shadowcolor), 255 };
      SDL_Surface* pShadowSurface = TTF_RenderText_Blended_Wrapped(pTTFFont, text.c_str(), text.length(), shadowColor, 0);
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
         LOGE("Failed to render shadow: label=" + m_szName + ", error=" + SDL_GetError());
         SDL_Rect textRect = { (xoffset < 0) ? -xoffset : 0, (yoffset < 0) ? -yoffset : 0, pTextSurface->w, pTextSurface->h };
         SDL_BlitSurface(pTextSurface, nullptr, pMergedSurface, &textRect);
      }
   }
   else
   {
      SDL_BlitSurface(pTextSurface, nullptr, pMergedSurface, nullptr);
   }
   SDL_DestroySurface(pTextSurface);

   if (m_filterMode > 0)
      ApplyFilter(pMergedSurface, m_filterMode);

   if (m_angle != 0.f)
   {
      SDL_Surface* pRotated = RotateSurface(pMergedSurface, m_angle);
      if (pRotated)
      {
         SDL_DestroySurface(pMergedSurface);
         pMergedSurface = pRotated;
      }
   }

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

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, int flashingPeriod)
   : m_color(label->m_color)
   , m_label(label)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_flashingPeriod(flashingPeriod)
{
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, int xps, int xpe, int yps, int ype, int motionLen, int motionTween, int motionColor)
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

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, int alphaStart, int alphaEnd, int pulseSpeed, bool screenFade)
   : m_label(label)
   , m_color(label->m_color)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_alphaStart(alphaStart)
   , m_alphaEnd(alphaEnd)
   , m_pulseSpeed(pulseSpeed)
   , m_screenFade(screenFade)
{
   m_alpha = static_cast<float>(m_alphaStart) / 255.0f;
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float zoomStart, float zoomEnd, int zoomPulseSpeed)
   : m_label(label)
   , m_color(label->m_color)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_zoomStart(zoomStart)
   , m_zoomEnd(zoomEnd)
   , m_zoomPulseSpeed(zoomPulseSpeed)
{
   m_zoom = m_zoomStart;
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float wiggleStart, float wiggleEnd, int wiggleSpeed, int dummy)
   : m_label(label)
   , m_color(label->m_color)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_wiggleStart(wiggleStart)
   , m_wiggleEnd(wiggleEnd)
   , m_wiggleSpeed(wiggleSpeed)
{
   (void)dummy;
   m_wiggle = m_wiggleStart;
}

bool PUPLabel::Animation::Update(const SDL_Rect& screenRect, const SDL_FRect& labelRect)
{
   uint64_t elapsed = SDL_GetTicks() - m_startTimestamp;

   if (elapsed >= m_lengthMs)
   {
      m_color = m_label->m_color;
      m_xOffset = 0.f;
      m_yOffset = 0.f;
      m_alpha = static_cast<float>(m_alphaEnd) / 255.0f;
      m_zoom = m_zoomEnd;
      if (m_numStart != INT_MIN)
      {
         string numStr = std::to_string(m_numEnd);
         if (m_numFormat == 1)
         {
            int insertPos = static_cast<int>(numStr.length()) - 3;
            while (insertPos > 0) { numStr.insert(insertPos, ","); insertPos -= 3; }
         }
         PUPLabel* label = const_cast<PUPLabel*>(m_label);
         label->m_szCaption = numStr;
         label->m_dirty = true;
      }
      return true;
   }

   if (m_flashingPeriod)
   {
      // See pDMDLabelFlash — toggle visibility every fq milliseconds
      m_visible = ((elapsed / m_flashingPeriod) & 1) == 0;
      m_color = m_foregroundColor;
   }

   if (m_delayShow > 0)
   {
      // See pDMDLabelClone — hide until delay elapsed, then show
      m_visible = (static_cast<int>(elapsed) >= m_delayShow);
   }

   if (m_motionLen)
   {
      float pos = saturate(static_cast<float>(elapsed) / static_cast<float>(m_motionLen));
      switch (m_motionTween)
      {
      default:
      // Robert Penner easing functions (see https://easings.net)
      case 0: break;
      case 1: pos = pos * pos; break;
      case 2: pos = pos * (2.f - pos); break;
      case 3: pos = pos < 0.5f ? 2.f * pos * pos : -1.f + (4.f - 2.f * pos) * pos; break;
      case 4: pos = pos * pos * pos; break;
      case 5: { float t = pos - 1.f; pos = t * t * t + 1.f; } break;
      case 6: pos = pos < 0.5f ? 4.f * pos * pos * pos : (pos - 1.f) * (2.f * pos - 2.f) * (2.f * pos - 2.f) + 1.f; break;
      case 7: pos = pos * pos * pos * pos; break;
      case 8: { float t = pos - 1.f; pos = -(t * t * t * t) + 1.f; } break;
      case 9: pos = pos < 0.5f ? 8.f * pos * pos * pos * pos : -8.f * (pos - 1.f) * (pos - 1.f) * (pos - 1.f) * (pos - 1.f) + 1.f; break;
      case 10: pos = pos * pos * pos * pos * pos; break;
      }
      const float xBase = labelRect.x;
      const float xLeft = static_cast<float>(screenRect.x) - labelRect.w;
      const float xRight = static_cast<float>(screenRect.x) + static_cast<float>(screenRect.w);
      const float yBase = labelRect.y;
      const float yTop = static_cast<float>(screenRect.y) - labelRect.h;
      const float yBottom = static_cast<float>(screenRect.y) + static_cast<float>(screenRect.h);
      // -1 = off-screen left/top, 0 = current position, 1 = off-screen right/bottom,
      // other values = percentage position (see pDMDLabelMoveTO)
      const float screenW = static_cast<float>(screenRect.w);
      const float screenH = static_cast<float>(screenRect.h);
      // TODO: position interpretation when only end is specified (no start) is uncertain.
      // GOTG bumper pops use ype:150 with no yps — video shows upward movement.
      // Current: treat as pixel offset upward from current position when start is absent.
      const float xs = m_xps == 0 ? xBase : m_xps == 1 ? xRight : m_xps == -1 ? xLeft : static_cast<float>(screenRect.x) + screenW * (float)m_xps / 100.f;
      const float xe = m_xpe == 0 ? xBase : m_xpe == 1 ? xRight : m_xpe == -1 ? xLeft
         : (m_xps == 0 ? xBase - static_cast<float>(m_xpe) : static_cast<float>(screenRect.x) + screenW * (float)m_xpe / 100.f);
      const float ys = m_yps == 0 ? yBase : m_yps == 1 ? yBottom : m_yps == -1 ? yTop : static_cast<float>(screenRect.y) + screenH * (float)m_yps / 100.f;
      const float ye = m_ype == 0 ? yBase : m_ype == 1 ? yBottom : m_ype == -1 ? yTop
         : (m_yps == 0 ? yBase - static_cast<float>(m_ype) : static_cast<float>(screenRect.y) + screenH * (float)m_ype / 100.f);
      m_color = pos < 1.0f ? m_motionColor : m_foregroundColor;
      m_xOffset = lerp(xs, xe, pos) - xBase;
      m_yOffset = lerp(ys, ye, pos) - yBase;

      if (m_alphaFade > 0)
      {
         const int fadeStart = static_cast<int>(m_lengthMs) - m_alphaFade;
         if (static_cast<int>(elapsed) > fadeStart)
            m_alpha = 1.0f - saturate(static_cast<float>(elapsed - fadeStart) / static_cast<float>(m_alphaFade));
      }
   }

   if (m_alphaStart != m_alphaEnd)
   {
      const float as = static_cast<float>(m_alphaStart) / 255.0f;
      const float ae = static_cast<float>(m_alphaEnd) / 255.0f;
      if (m_pulseSpeed > 0)
      {
         // See pDMDLabelFadePulse — bounce alpha between astart and aend
         // Approximate one full cycle takes range/speed*2 ticks * 33ms
         const float range = static_cast<float>(m_alphaEnd - m_alphaStart);
         const float periodMs = (range / static_cast<float>(m_pulseSpeed)) * (float)(2.0 * 33.0);
         const float cyclePos = fmodf(static_cast<float>(elapsed), periodMs) / periodMs;
         const float t = cyclePos < 0.5f ? cyclePos * 2.0f : 2.0f - cyclePos * 2.0f;
         m_alpha = as + t * (ae - as);
      }
      else
      {
         // See pDMDLabelFadeOut / pDMDLabelFadeIn / pDMDScreenFadeOut / pDMDScreenFadeIn
         float pos = saturate(static_cast<float>(elapsed) / static_cast<float>(m_lengthMs));
         m_alpha = as + pos * (ae - as);
      }
      m_color = m_foregroundColor;
   }

   if (m_zoomStart != m_zoomEnd)
   {
      if (m_zoomPulseSpeed != 0)
      {
         // See pDMDLabelPulseText / pDMDLabelPulseImage — bounce zoom between hstart and hend
         // Approximate one full cycle (start->end->start) every 500ms
         const float periodMs = 500.0f;
         const float cyclePos = fmodf(static_cast<float>(elapsed), periodMs) / periodMs;
         const float t = cyclePos < 0.5f ? cyclePos * 2.0f : 2.0f - cyclePos * 2.0f;
         m_zoom = m_zoomStart + t * (m_zoomEnd - m_zoomStart);
      }
      else
      {
         // See pDMDZoomBig — zoom from hstart to hend over len ms
         float pos = saturate(static_cast<float>(elapsed) / static_cast<float>(m_lengthMs));
         m_zoom = m_zoomStart + pos * (m_zoomEnd - m_zoomStart);
      }
      m_color = m_foregroundColor;
   }

   if (m_wiggleStart != m_wiggleEnd)
   {
      // See pDMDLabelWiggleText / pDMDLabelWiggleImage — bounce rotation between rstart and rend.
      const float periodMs = 300.0f;
      const float cyclePos = fmodf(static_cast<float>(elapsed), periodMs) / periodMs;
      const float t = cyclePos < 0.5f ? cyclePos * 2.0f : 2.0f - cyclePos * 2.0f;
      m_wiggle = m_wiggleStart + t * (m_wiggleEnd - m_wiggleStart);
      m_color = m_foregroundColor;
   }

   // See pDMDLabelPulseNumber — interpolate number and update label caption
   if (m_numStart != INT_MIN)
   {
      float pos = saturate(static_cast<float>(elapsed) / static_cast<float>(m_lengthMs));
      int num = (pos >= 1.f) ? m_numEnd : m_numStart + static_cast<int>(pos * static_cast<float>(m_numEnd - m_numStart));
      string numStr;
      if (m_numFormat == 1)
      {
         numStr = std::to_string(num);
         int insertPos = static_cast<int>(numStr.length()) - 3;
         while (insertPos > 0)
         {
            numStr.insert(insertPos, ",");
            insertPos -= 3;
         }
      }
      else
         numStr = std::to_string(num);

      if (numStr != m_numText)
      {
         m_numText = numStr;
         PUPLabel* label = const_cast<PUPLabel*>(m_label);
         label->m_szCaption = numStr;
         label->m_dirty = true;
      }
   }

   return false;
}


}
