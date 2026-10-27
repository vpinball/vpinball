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

// Rotation is baked into the texture on the CPU (GPU rotation skews the image).
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

   const float rxoo = srcCx - dstCx * c - dstCy * s;
   const float ryoo = srcCy + dstCx * s - dstCy * c;
   for (int dy = 0; dy < dstH; dy++)
   {
      int ofs = dy * dstPitch;
      const float rxo = rxoo + (float)dy * s;
      const float ryo = ryoo + (float)dy * c;
      for (int dx = 0; dx < dstW; dx++,ofs++)
      {
         const float rx = rxo + (float)dx * c;
         const float ry = ryo - (float)dx * s;
         const int sx = (int)rx;
         const int sy = (int)ry;
         if (/*sx >= 0 &&*/ (unsigned int)sx < srcW && /*sy >= 0 &&*/ (unsigned int)sy < srcH)
            dstPixels[ofs] = srcPixels[sy * srcPitch + sx];
         else
            dstPixels[ofs] = 0;
      }
   }

   SDL_UnlockSurface(dst);
   SDL_UnlockSurface(src);
   return dst;
}

// See pDMDLabelSetFilter - apply pixel filter to an RGBA32 surface
// fmode: 1=invertRGB, 2=invert(+alpha), 3=grayscale, 4=invertalpha, 5=clear, 6=blur
static void ApplyFilter(SDL_Surface* surf, int filterMode)
{
   if (!surf || filterMode <= 0 || filterMode > 6)
      return;

   SDL_LockSurface(surf);
   uint32_t* const __restrict pixels = (uint32_t*)surf->pixels;
   const int pitch = surf->pitch / 4;
   const int w = surf->w;
   const int h = surf->h;

   for (int y = 0; y < h; y++)
   {
      int ofs = y * pitch;
      if (filterMode == 5) // clear
      {
         memset(pixels + ofs, 0, w*sizeof(uint32_t));
         continue;
      }
      for (int x = 0; x < w; x++,ofs++)
      {
         uint32_t& px = pixels[ofs];
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
         //case 5: // clear
         //   px = 0;
         //   break;
         }
      }
   }
   SDL_UnlockSurface(surf);

   if (filterMode == 6)
   {
      // PUP filter 6 is a box blur (radius 5). Keep it simple and separable so the
      // costly row pass is only executed once per frame.
      constexpr int radius = 5;
      SDL_LockSurface(surf);
      uint32_t* const pixels = (uint32_t*)surf->pixels;
      const int pitch = surf->pitch / 4;
      vector<uint32_t> tmp(static_cast<size_t>(surf->w) * surf->h);
      for (int y = 0; y < surf->h; y++)
      {
         for (int x = 0; x < surf->w; x++)
         {
            uint32_t r = 0, g = 0, b = 0, a = 0;
            int count = 0;
            for (int dx = -radius; dx <= radius; dx++)
            {
               const int sx = std::clamp(x + dx, 0, surf->w - 1);
               const uint32_t px = pixels[y * pitch + sx];
               r += px & 0xFF; g += (px >> 8) & 0xFF; b += (px >> 16) & 0xFF; a += (px >> 24) & 0xFF;
               count++;
            }
            tmp[static_cast<size_t>(y) * surf->w + x] = ((a / count) << 24) | ((b / count) << 16) | ((g / count) << 8) | (r / count);
         }
      }
      for (int y = 0; y < surf->h; y++)
      {
         for (int x = 0; x < surf->w; x++)
         {
            uint32_t r = 0, g = 0, b = 0, a = 0;
            int count = 0;
            for (int dy = -radius; dy <= radius; dy++)
            {
               const int sy = std::clamp(y + dy, 0, surf->h - 1);
               const uint32_t px = tmp[static_cast<size_t>(sy) * surf->w + x];
               r += px & 0xFF; g += (px >> 8) & 0xFF; b += (px >> 16) & 0xFF; a += (px >> 24) & 0xFF;
               count++;
            }
            pixels[y * pitch + x] = ((a / count) << 24) | ((b / count) << 16) | ((g / count) << 8) | (r / count);
         }
      }
      SDL_UnlockSurface(surf);
   }
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
   if (m_pendingTextureUpdate.valid())
      m_pendingTextureUpdate.wait();
   if (m_pSourceImage)
      SDL_DestroySurface(m_pSourceImage);
}

void PUPLabel::SetCaption(const string& szCaption)
{
   if (szCaption.empty() && m_type != PUP_LABEL_TYPE_TEXT)
      return;

   if (szCaption == "`u`")
      return;

   // FN 46 pad mode: PinUP turns '~' into two spaces instead of a line break and
   // wraps text captions in a space on each side.
   const bool padText = m_pScreen && m_pScreen->m_padTextAlways;
   string szText = padText ? string_replace_all(szCaption, "~"s, "  "s) : szCaption;
   if (!padText)
      std::ranges::replace(szText.begin(), szText.end(), '~', '\n');
   szText = string_replace_all(szText, "\\r"s, '\n');

   // PUP supports an embedded color suffix: "SOME TEXT|16744448". The color is
   // applied before the caption-change test so a color-only update still redraws.
   int embeddedColor = -1;
   if (const size_t colorPos = szText.find('|'); colorPos != string::npos)
   {
      embeddedColor = string_to_int(szText.substr(colorPos + 1), 16744448);
      szText.resize(colorPos);
   }

   {
      std::lock_guard lock(m_mutex);

      if (embeddedColor >= 0 && m_color != embeddedColor)
      {
         m_color = embeddedColor;
         m_dirty = true;
      }

      if (m_szCaption == szText)
         return;

      const string szExt = extension_from_path(szText);
      const bool isAnimatedImage = (szExt == "gif" || szExt == "apng" || szExt == "agif");
      const bool isImageCaption = (isAnimatedImage || szExt == "png" || szExt == "bmp" || szExt == "jpg" || szExt == "jpeg");

      if (!isImageCaption)
      {
         if (padText && !szText.empty())
            szText = ' ' + szText + ' ';
         // A plain caption (e.g. " " from pDMDLabelShow/pDMDLabelHide) must keep an existing
         // image, not convert the label to text and wipe it.
         if (m_type == PUP_LABEL_TYPE_IMAGE || m_type == PUP_LABEL_TYPE_GIF)
         {
            m_szCaption = szText;
            return;
         }
         m_type = PUP_LABEL_TYPE_TEXT;
         m_szPath.clear();
         m_szCaption = szText;
         m_dirty = true;
         return;
      }

      const bool wasImage = (m_type == PUP_LABEL_TYPE_IMAGE || m_type == PUP_LABEL_TYPE_GIF);
      string szRelPath = normalize_path_separators(szText);
      szRelPath.erase(0, szRelPath.find_first_not_of(PATH_SEPARATOR_CHAR));
      std::filesystem::path fs_path = PluginStrings::PathFromUTF8(szRelPath);
      // The playlist is the first path component, the rest is the file path inside
      // the playlist folder (which may contain subfolders, e.g. playlist\player2\foo.png)
      const std::filesystem::path playlistFolder = *fs_path.begin();
      PUPPlaylist* pPlaylist = m_pScreen->GetPlaylist(PluginStrings::PathToUTF8(playlistFolder));
      std::filesystem::path szPath;
      if (pPlaylist)
         szPath = pPlaylist->GetPlayFilePath(fs_path.lexically_relative(playlistFolder));
      if (szPath.empty())
      {
         // PUP also accepts a caption that is a path relative to the pack root,
         // without requiring a matching playlist entry.
         szPath = find_case_insensitive_file_path(m_pScreen->GetManager()->GetPath() / fs_path);
         if (!szPath.empty() && !std::filesystem::is_regular_file(szPath))
            szPath.clear();
      }
      if (!szPath.empty())
      {
         const bool pathChanged = (m_szPath != szPath);
         m_szPath = szPath;
         m_type = isAnimatedImage ? PUP_LABEL_TYPE_GIF : PUP_LABEL_TYPE_IMAGE;
         m_szCaption = szText;
         m_dirty = true;
         if (pathChanged)
         {
            m_anigif = 0;
            m_animating = (m_type == PUP_LABEL_TYPE_GIF);
            m_animationFrame = -1;
            m_animationStart = SDL_GetTicks();
         }
      }
      else
      {
         // Many tables use clear.png / clear1.png as a "clear this label" caption without
         // ever shipping the file in the pup pack. Drop the caption so nothing renders, and
         // if we had previously loaded an image, hide the label so a later valid image
         // doesn't silently reappear.
         LOGE(std::format("Image not found, hiding label: screen={}, label={}, path={}",
            m_pScreen->GetScreenNum(), m_szName, szText));
         m_type = PUP_LABEL_TYPE_TEXT;
         m_szPath.clear();
         if (wasImage)
            m_visible = false;
         m_szCaption.clear();
         m_dirty = true;
      }
   }
}

void PUPLabel::SetVisible(bool visible)
{
   std::lock_guard lock(m_mutex);
   m_visible = visible;
}

void PUPLabel::ClearAnimation()
{
   std::lock_guard lock(m_mutex);
   ClearAnimationInternal();
}

void PUPLabel::ClearAnimationInternal()
{
   if (m_animation)
   {
      m_animation->Restore();
      m_animation.reset();
   }
}

float PUPLabel::EffectiveSize() const
{
   // PinUP's 'size' key sets TFont.Size in points at 96 dpi, i.e. an em height of 96/72 the
   // pixel value, while LabelNew 'sz' and 'fonth' set the Windows cell height. Our font
   // metrics work in cell heights, so convert the em height back through the font's ratio.
   if (!m_sizeIsPoints || !m_pFont)
      return m_size;
   return m_size * (96.f / 72.f) / m_pFont->GetWinScale();
}

SDL_FPoint PUPLabel::GetShadowOffsetPixels(float fontHeight) const
{
   return { m_xOffsetIsPercent ? fontHeight * m_xoffset / 100.f : m_xoffset, m_yOffsetIsPercent ? fontHeight * m_yoffset / 100.f : m_yoffset };
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
   {
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
      std::lock_guard lock(m_mutex);

      // PUP runs one animation per label unless 'ad' asks for parallel animations.
      // This implementation still supports a single active animation, but at least
      // matches the default behavior of replacing the previous one.
      if (json["ad"s].as<int>(0) == 0)
         ClearAnimationInternal();
      else if (m_animation)
         LOGW("Parallel label animations (ad=1) are not fully supported; replacing the active animation: label=" + m_szName);

      const int foregroundColor = json["fc"s].as<int>(m_color);
      const int alphaFade = json["af"s].as<int>(0);
      const int animType = json["at"s].as<int>(0);
      const int animLength = json["len"s].as<int>(0);
      int motionLength = json["mlen"s].as<int>(0);
      if (motionLength == 0)
         motionLength = animLength;
      const int motionTween = json["tt"s].as<int>(0);

      switch (animType)
      {
      case 1:
         // See pDMDLabelFlash - fq is the toggle interval in milliseconds, fc is optional
         if (json["len"s].exists() && json["fq"s].as<int>(0) > 0)
            m_animation = std::make_unique<Animation>(this, animLength, foregroundColor, json["fq"s].as<int>());
         else
            LOGE("Invalid label animation specified: {" + szSpecial + '}');
         break;

      case 2:
         // See pDMDLabelMoveHorz / pDMDLabelMoveVert / pDMDLabelMoveTO
         // See pDMDLabelMoveHorzFade / pDMDLabelMoveVertFade - af = alpha fade start time (ms before end)
         if (json["len"s].exists())
            m_animation = std::make_unique<Animation>(this, animLength, foregroundColor, static_cast<float>(json["xps"s].as<double>(0)), static_cast<float>(json["xpe"s].as<double>(0)),
               static_cast<float>(json["yps"s].as<double>(0)), static_cast<float>(json["ype"s].as<double>(0)), motionLength, motionTween, json["mColor"s].as<int>(foregroundColor));
         else
            LOGE("Invalid label animation specified: {" + szSpecial + '}');
         break;

      case 3:
      {
         // See pDMDZoomBig - zoom from hstart% to hend% over mlen ms
         // 'ns' makes the percentages relative to the label instead of the display.
         const float hstart = static_cast<float>(json["hstart"s].as<double>(100));
         const float hend = static_cast<float>(json["hend"s].as<double>(200));
         m_animation = std::make_unique<Animation>(this, animLength > 0 ? animLength : 1000, foregroundColor, hstart, hend, 0);
         m_animation->m_zoomIsPulse = false;
         m_animation->m_zoomUseDrawHeight = json["ns"s].exists();
         m_animation->SetMotionTiming(motionLength, motionTween);
         break;
      }

      case 4:
      {
         // See pDMDLabelPulseText / pDMDLabelPulseImage - bounce zoom between hstart% and hend%
         // See pDMDLabelPulseNumber - numstart/numend count up during animation.
         const float hstart = static_cast<float>(json["hstart"s].as<double>(80));
         const float hend = static_cast<float>(json["hend"s].as<double>(120));
         const int pspeed = json["pspeed"s].as<int>(0);
         m_animation = std::make_unique<Animation>(this, animLength > 0 ? animLength : 3000, foregroundColor, hstart, hend, pspeed);
         m_animation->m_zoomIsPulse = true;
         m_animation->SetMotionTiming(motionLength, motionTween);
         if (json["numend"s].exists())
         {
            m_animation->m_numStart = json["numstart"s].as<int>(0);
            m_animation->m_numEnd = json["numend"s].as<int>(0);
            m_animation->m_numFormat = json["numformat"s].as<int>(0);
         }
         break;
      }

      case 5:
      {
         // See pDMDLabelFadeOut / pDMDLabelFadeIn - alpha ease from astart to aend over mlen ms
         const int astart = json["astart"s].as<int>(255);
         const int aend = json["aend"s].as<int>(0);
         m_animation = std::make_unique<Animation>(this, animLength > 0 ? animLength : 1000, foregroundColor, astart, aend, 0, false);
         m_animation->SetMotionTiming(motionLength, motionTween);
         break;
      }

      case 6:
      {
         // See pDMDLabelFadePulse - alpha oscillates between astart and aend
         const int astart = json["astart"s].as<int>(70);
         const int aend = json["aend"s].as<int>(255);
         const int pspeed = json["pspeed"s].as<int>(1);
         m_animation = std::make_unique<Animation>(this, animLength > 0 ? animLength : 3000, foregroundColor, astart, aend, std::max(1, pspeed), false);
         break;
      }

      case 7:
      {
         // See pDMDScreenFadeOut / pDMDScreenFadeIn - same as at=5 but for parent screen
         const int astart = json["astart"s].as<int>(255);
         const int aend = json["aend"s].as<int>(0);
         m_animation = std::make_unique<Animation>(this, animLength > 0 ? animLength : 1000, foregroundColor, astart, aend, 0, true);
         m_animation->SetMotionTiming(motionLength, motionTween);
         break;
      }

      case 8:
      {
         // See pDMDLabelWiggleText / pDMDLabelWiggleImage - rotation bounces between rstart and rend.
         // PUP values are degrees, not tenths of degrees (unlike LabelNew/SetRotate).
         const float rstart = static_cast<float>(json["rstart"s].as<double>(-45));
         const float rend = static_cast<float>(json["rend"s].as<double>(45));
         int rspeed = json["rspeed"s].as<int>(1);
         if (rspeed == 0)
            rspeed = 1;
         m_animation = std::make_unique<Animation>(this, animLength > 0 ? animLength : 3000, foregroundColor, rstart, rend, rspeed, 0);
         m_animation->SetMotionTiming(motionLength, motionTween);
         break;
      }

      case 9:
      {
         // See pDMDLabelFlashFilter - invert filter flashing.
         if (json["len"s].exists() && json["fq"s].as<int>(0) > 0)
         {
            m_animation = std::make_unique<Animation>(this, animLength, foregroundColor, json["fq"s].as<int>());
            m_animation->m_invertFilter = true;
            m_animation->m_filterMode = json["fm"s].as<int>(0);
         }
         else
            LOGE("Invalid label filter animation specified: {" + szSpecial + '}');
         break;
      }

      case 10:
      {
         // See pDMDLabelClone / pDMDLabelCloneDelay - delayed show. PUP draws a
         // temporary copy over the label; showing this label after the delay is
         // visually equivalent (the copy has the same text, font and position).
         const int delay = json["dl"s].as<int>(0);
         m_animation = std::make_unique<Animation>(this, animLength, foregroundColor, 0);
         m_animation->m_delayShow = delay;
         m_animation->m_visible = (delay <= 0);
         break;
      }

      default:
         LOGE("Unsupported Label.SetSpecial animation type: " + std::to_string(animType));
         break;
      }

      if (m_animation)
      {
         m_animation->m_type = animType;
         m_animation->m_alphaFade = alphaFade;
         if (json["xp"s].exists())
         {
            m_animation->m_hasPixelX = true;
            m_animation->m_pixelX = static_cast<float>(json["xp"s].as<int>());
         }
         if (json["yp"s].exists())
         {
            m_animation->m_hasPixelY = true;
            m_animation->m_pixelY = static_cast<float>(json["yp"s].as<int>());
         }
      }
   }
   break;

   case 2:
   {
      std::lock_guard lock(m_mutex);
      static constexpr std::string_view keyOrder[] = { "size", "fonth", "xpos", "ypos", "width", "height", "autow", "autoh", "zoom", "xoffset", "yoffset", "anistart", "anigif",
         "animate", "fname", "color", "xalign", "yalign", "pagenum", "quality", "rotate", "alpha", "xoffpix", "yoffpix", "qual", "bold", "italic", "underline", "normal",
         "shadowcolor", "shadowtype", "shadowstate", "outline", "gradcolor", "gradstate", "grayscale", "filter", "dodefault", "maxlen", "transcolor", "gifstart", "gifend",
         "gifloop", "anidispose", "anihide", "aniendhide", "rescale", "ztop", "zback", "forceshow", "forcehide", "stopani", "showstats", "v2", "dispose" };
      RSJobject& properties = json.as_object();
      vector<std::pair<string, RSJresource*>> ordered;
      for (const std::string_view& name : keyOrder)
         if (auto it = properties.find(string(name)); it != properties.end())
            ordered.emplace_back(it->first, &it->second);
      for (auto& [name, property] : properties)
         if (std::ranges::find(keyOrder, name) == std::end(keyOrder))
            ordered.emplace_back(name, &property);
      for (auto& [key, pValue] : ordered)
      {
         RSJresource& value = *pValue;
         if (key == "mt")
         {
         }
         else if (key == "zback")
         {
            // See pDMDLabelSendToBack. PUP puts -1 behind everything and any other
            // value just in front of the first (background) label.
            if (value.as<int>() == -1)
               m_pScreen->SendLabelToBack(this);
            else
               m_pScreen->SendLabelToBackground(this);
         }
         else if (key == "ztop")
         {
            // See pDMDLabelSendToFront
            m_pScreen->SendLabelToFront(this);
         }
         else if (key == "size")
         {
            // See pDMDLabelSetSizeText - percent of display height, applied by PinUP as a point size
            const float newSize = std::max(1.f, static_cast<float>(json["size"s].as<double>()));
            if (m_size != newSize || !m_sizeIsPoints)
            {
               m_size = newSize;
               m_sizeIsPoints = true;
               m_dirty = true;
            }
         }
         else if (key == "xpos")
         {
            // See pDMDLabelSetPos - position as percent of display width/height
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
            if (m_size != newSize || m_sizeIsPoints)
            {
               m_size = newSize;
               m_sizeIsPoints = false;
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
            // See PDMDLabelSetAlign - 0=left 1=center 2=right, 0=top 1=center 2=bottom
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
            // See pDMDLabelStopAnis - stop any pup animations on label/image (zoom/flash/pulse), not animated gifs.
            // PUP ends the animation, restoring the label's pre-animation state.
            ClearAnimationInternal();
         }
         else if (key == "rotate")
         {
            // See pDMDLabelSetRotate - in tenths, so 900 is 90 degrees. rotate support for images too.
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
            // See pDMDLabelSetZoom - 120 for 120% of current height, 80% etc...
            m_zoom = static_cast<float>(value.as<double>());
         }
         else if (key == "alpha")
         {
            // See pDMDLabelSetAlpha - 0-255  255=full, 0=blank
            m_alpha = saturate((float)value.as<int>() / 255.0f);
         }
         else if (key == "gradstate")
         {
            // See pDMDLabelSetColorGradient - gradstate controls gradient blend (0=off, 1+=on)
            const int newGradState = value.as<int>();
            if (m_gradState != newGradState)
            {
               m_gradState = newGradState;
               m_dirty = true;
            }
         }
         else if (key == "gradcolor")
         {
            // See pDMDLabelSetColorGradientPercent - end color of the vertical gradient
            const int newGradColor = value.as<int>();
            if (m_gradColor != newGradColor)
            {
               m_gradColor = newGradColor;
               m_dirty = true;
            }
         }
         else if (key == "grayscale")
         {
            // See pDMDLabelSetGrayScale - shortcut for filter mode 3 on images. 1=on, 0=off
            const int newFilterMode = (value.as<int>() == 1) ? 3 : 0;
            if (m_filterMode != newFilterMode)
            {
               m_filterMode = newFilterMode;
               m_dirty = true;
            }
         }
         else if (key == "filter")
         {
            // See pDMDLabelSetFilter - fmode 1=invertRGB, 2=invert, 3=grayscale, 4=invertalpha, 5=clear/blur
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
            // See pDMDLabelSetShadow / pDMDLabelSetBorder - shadow displacement as % of font height
            const float newXOffset = static_cast<float>(value.as<double>());
            m_xOffsetIsPercent = true;
            if (m_xoffset != newXOffset)
            {
               m_xoffset = newXOffset;
               m_dirty = true;
            }
         }
         else if (key == "yoffset")
         {
            const float newYOffset = static_cast<float>(value.as<double>());
            m_yOffsetIsPercent = true;
            if (m_yoffset != newYOffset)
            {
               m_yoffset = newYOffset;
               m_dirty = true;
            }
         }
         else if (key == "xoffpix")
         {
            m_xoffset = static_cast<float>(value.as<int>());
            m_xOffsetIsPercent = false;
            m_dirty = true;
         }
         else if (key == "yoffpix")
         {
            m_yoffset = static_cast<float>(value.as<int>());
            m_yOffsetIsPercent = false;
            m_dirty = true;
         }
         else if (key == "anigif")
         {
            // See pDMDPNGAnimate. PUP maps 100 to a 40 ms frame time and lower
            // values to progressively slower frames (0 -> 330 ms).
            const int speed = value.as<int>();
            m_anigif = (speed >= 100) ? 40 : 30 + static_cast<int>(lroundf((100 - speed) / 10.f * 30.f));
            m_animating = true;
            m_animationFrame = -1;
            m_animationStart = SDL_GetTicks();
            m_dirty = true;
         }
         else if (key == "animate")
         {
            // See pDMDPNGAnimate / pDMDPNGAnimateOnce - frame interval in ms, 0 stops
            const int interval = value.as<int>();
            if (interval < 1)
            {
               m_animating = false;
            }
            else
            {
               m_visible = true;
               m_anigif = interval;
               m_animating = true;
               m_animationFrame = -1;
               m_animationStart = SDL_GetTicks();
               m_dirty = true;
            }
         }
         else if (key == "anistart")
         {
            // See pDMDPNGAnimate - frame interval in ms, 0 stops
            const int interval = value.as<int>();
            m_anigif = interval;
            m_animating = interval > 0;
            m_animationFrame = -1;
            m_animationStart = SDL_GetTicks();
            m_dirty = true;
         }
         else if (key == "gifloop")
         {
            // See pDMDPNGAnimateEx - 1=loop forever (default), 0=play once
            m_gifLoop = (value.as<int>() == 1);
         }
         else if (key == "gifstart")
         {
            // See pDMDPNGAnimateEx / pDMDPNGShowFrame - start frame index, -1 resets to full range
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
            // See pDMDPNGAnimateEx / pDMDPNGShowFrame - end frame index, start==end shows single frame
            m_gifEnd = value.as<int>();
            if (m_gifStart == m_gifEnd && m_gifEnd >= 0)
               m_dirty = true;
         }
         else if (key == "aniendhide")
         {
            // See pDMDPNGAnimateOnce - 1=hide label when animation completes
            m_hideOnAnimEnd = (value.as<int>() == 1);
         }
         else if (key == "anidispose")
         {
            // See pDMDPNGAnimateOnceAndDispose - release the image when playback ends
            m_disposeOnAnimEnd = (value.as<int>() == 1);
         }
         else if (key == "width")
         {
            // See pDMDLabelSetSizeImage - width/height as percent of display dimensions
            if (m_type != PUP_LABEL_TYPE_TEXT)
            {
               const float newWidth = static_cast<float>(value.as<double>());
               m_imageWidthSet = true;
               if (m_imageWidth != newWidth)
               {
                  m_imageWidth = newWidth;
                  m_dirty = true;
               }
            }
         }
         else if (key == "height")
         {
            if (m_type != PUP_LABEL_TYPE_TEXT)
            {
               const float newHeight = static_cast<float>(value.as<double>());
               m_imageHeightSet = true;
               if (m_imageHeight != newHeight)
               {
                  m_imageHeight = newHeight;
                  m_dirty = true;
               }
            }
         }
         else if (key == "autow")
         {
            // See pDMDLabelSetAutoSize - auto-shrink font to fit within specified dimensions
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
            // See pDMDLabelSetBorder / pDMDLabelSetOutShadow.
            // PUP keeps a plain drop shadow when outline is 0, and otherwise sets the
            // shadow type to outline when 1 (or to the raw outline value). This also
            // covers the legacy meaning where outline held the shadow type itself.
            const int newOutline = value.as<int>();
            int newShadowType = m_shadowType;
            if (m_shadowType == 1)
            {
               if (newOutline == 1)
                  newShadowType = 2;
            }
            else
               newShadowType = (newOutline == 1) ? 2 : newOutline;
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
         else if (key == "underline")
         {
            const bool newUnderline = (value.as<int>() == 1);
            if (m_underline != newUnderline)
            {
               m_underline = newUnderline;
               m_dirty = true;
            }
         }
         else if (key == "normal")
         {
            if (m_bold || m_italic || m_underline)
            {
               m_bold = m_italic = m_underline = false;
               m_dirty = true;
            }
         }
         else if (key == "dodefault")
         {
            m_doDefault = (value.as<int>() == 1);
         }
         else if (key == "maxlen")
         {
            // See pDMDLabelSetMaxLen - trim text captions to a maximum length
            const int maxLen = value.as<int>();
            if (m_type == PUP_LABEL_TYPE_TEXT && maxLen > 0 && static_cast<int>(m_szCaption.length()) > maxLen)
            {
               m_szCaption.resize(maxLen);
               m_dirty = true;
            }
         }
         else if (key == "dispose")
         {
            // See pDMDLabelDispose - hide and remove the label after this update
            ClearAnimationInternal();
            m_visible = false;
            m_doDispose = true;
         }
         else if (key == "forceshow")
         {
            m_visible = true;
         }
         else if (key == "forcehide")
         {
            m_visible = false;
         }
         else if (key == "anihide")
         {
            // See pDMDLabelHideAni - stop any label animation and hide it
            ClearAnimationInternal();
            m_visible = false;
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

   if (m_disposePending)
   {
      // Wait (without blocking while holding the lock) until a texture update launched
      // before the animation ended has completed before releasing its source surface.
      if (m_pendingTextureUpdate.valid())
      {
         if (m_pendingTextureUpdate.wait_for(std::chrono::seconds(0)) != std::future_status::ready)
            return;
         (void)m_pendingTextureUpdate.get();
      }
      m_szPath.clear();
      m_type = PUP_LABEL_TYPE_TEXT;
      m_szCaption.clear();
      m_renderState = RenderState {};
      if (m_pSourceImage)
      {
         SDL_DestroySurface(m_pSourceImage);
         m_pSourceImage = nullptr;
         m_sourceImagePath.clear();
      }
      m_disposePending = false;
      m_dirty = false;
      return;
   }

   // PUP draws labels with a negative page number on every page (page -1 is used
   // for persistent labels such as pBackground).
   if ((!m_visible && !m_animation) || (m_pagenum >= 0 && pagenum != m_pagenum) || m_szCaption.empty())
      return;

   int fontColor = m_animation ? m_animation->m_color : m_color;

   // Re-bake the texture when the rotation angle moves. PUP's wiggle animation sets an
   // absolute rotation rather than adding an offset to the label angle, and PUP's rotate
   // runs the opposite way from RotateSurface (PUP stores 360 - angle/10 for 'rotate').
   const float wantAngle = (m_animation && m_animation->IsWiggle()) ? -m_animation->m_wiggle : m_angle;
   if (wantAngle != m_renderAngle)
   {
      m_renderAngle = wantAngle;
      m_dirty = true;
   }

   if (m_pendingTextureUpdate.valid())
   {
      if (m_pendingTextureUpdate.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      {
         m_renderState = m_pendingTextureUpdate.get();
         m_renderState.CreateTextureFromSurface();
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
            const float fontHeight = (EffectiveSize() / 100.0f) * static_cast<float>(rect.h);
            return UpdateLabelTexture(rect.w, rect.h, m_pFont, m_szCaption, EffectiveSize(), fontColor, m_shadowType, m_shadowColor, GetShadowOffsetPixels(fontHeight));
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
      if (startFrame != endFrame && m_animating)
      {
         const int elapsed = static_cast<int>(SDL_GetTicks() - m_animationStart);
         bool finished = false;
         if (m_anigif > 0)
         {
            // PUP's anigif/anistart/animate values are per-frame intervals in ms,
            // not a multiplier on the file's own delays.
            const int rangeFrames = endFrame - startFrame + 1;
            const int rangeDuration = m_anigif * rangeFrames;
            if (!m_gifLoop && elapsed >= rangeDuration)
            {
               expectedFrame = endFrame;
               finished = m_animating;
            }
            else
               expectedFrame = startFrame + (elapsed / m_anigif) % rangeFrames;
         }
         else
         {
            const int startOffset = startFrame > 0 ? m_renderState.m_accumulatedDelays[startFrame - 1] : 0;
            const int rangeDuration = m_renderState.m_accumulatedDelays[endFrame] - startOffset;
            int frameTime = elapsed;
            if (rangeDuration > 0)
            {
               if (m_gifLoop)
                  frameTime = elapsed % rangeDuration;
               else if (elapsed >= rangeDuration)
               {
                  frameTime = rangeDuration - 1;
                  finished = m_animating;
               }
            }
            const auto& delays = m_renderState.m_accumulatedDelays;
            expectedFrame = static_cast<int>(std::upper_bound(delays.begin() + startFrame, delays.begin() + endFrame + 1, startOffset + frameTime) - delays.begin());
            if (expectedFrame > endFrame)
               expectedFrame = endFrame;
            if (expectedFrame < startFrame)
               expectedFrame = startFrame;
         }
         if (finished)
         {
            m_animating = false;
            if (m_hideOnAnimEnd)
               m_visible = false;
            if (m_disposeOnAnimEnd)
            {
               // See pDMDPNGAnimateOnceAndDispose: release the animation/image at the
               // next render, once any in-flight texture update has completed.
               m_disposePending = true;
               return;
            }
         }
      }
      else if (!m_animating && m_animationFrame >= startFrame && m_animationFrame <= endFrame)
         expectedFrame = m_animationFrame;
      if (expectedFrame != m_animationFrame)
      {
         m_animationFrame = expectedFrame;
         // TODO perform conversion on the ancillary thread and not at render time
         SDL_Surface* const frame = m_renderState.m_pAnimation->frames[m_animationFrame];
         SDL_Surface* surf = frame;
         if (surf->format != SDL_PIXELFORMAT_RGBA32)
            surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
         if (surf && m_filterMode > 0 && surf == frame)
            surf = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
         if (surf && m_filterMode > 0)
            ApplyFilter(surf, m_filterMode);
         if (surf)
         {
            SDL_LockSurface(surf);
            // TODO handle situations where width != pitch
            UpdateTexture(&m_renderState.m_pTexture, surf->w, surf->h, VPXTextureFormat::VPXTEXFMT_sRGBA8, surf->pixels);
            SDL_UnlockSurface(surf);
         }
         if (surf != frame)
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
      // PUP uses the native image size until width/height are explicitly set,
      // except pBackground which is always stretched to the display size.
      // Scale the dest by m_imageScale* so a rotated (grown) texture keeps its on-screen size.
      const bool isBackground = StrCompareNoCase(m_szName, "pBackground"s);
      const float baseWidth = isBackground ? static_cast<float>(rect.w)
         : m_imageWidthSet ? (m_imageWidth / 100.0f) * static_cast<float>(rect.w)
                           : static_cast<float>(m_renderState.m_sourceWidth);
      const float baseHeight = isBackground ? static_cast<float>(rect.h)
         : m_imageHeightSet ? (m_imageHeight / 100.0f) * static_cast<float>(rect.h)
                            : static_cast<float>(m_renderState.m_sourceHeight);
      width = baseWidth * m_renderState.m_imageScaleX;
      height = baseHeight * m_renderState.m_imageScaleY;
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
      const float fontHeight = (EffectiveSize() / 100.0f) * static_cast<float>(rect.h);
      const SDL_FPoint shadowOffset = GetShadowOffsetPixels(fontHeight);
      dest.x += static_cast<float>(static_cast<int>(shadowOffset.x)) * -2.f;
   }

   bool visible = true;
   if (m_animation)
   {
      if (m_animation->m_type == 1 || m_animation->m_type == 2 || m_animation->m_type == 10)
      {
         if (m_animation->m_hasPixelX)
            dest.x = static_cast<float>(rect.x) + m_animation->m_pixelX;
         if (m_animation->m_hasPixelY)
            dest.y = static_cast<float>(rect.y) + m_animation->m_pixelY;
      }

      if (m_animation->Update(rect, dest))
      {
         // The animation restored the label's pre-animation state itself.
         m_animation.reset();
         return;
      }

      // See pDMDScreenFadeOut / pDMDScreenFadeIn - at=7 fades the label overlay (FN 22 multiplier)
      if (m_animation->IsScreenFade() && m_pScreen)
         m_pScreen->m_overlayAlpha = m_animation->m_alpha;

      dest.x += m_animation->m_xOffset;
      dest.y += m_animation->m_yOffset;
      visible = m_animation->m_visible;
   }

   float alpha = ((m_animation && !m_animation->IsScreenFade()) ? m_animation->m_alpha : m_alpha) * screenAlpha;

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
      // Rotation and animated text color are baked into the texture (see RotateSurface /
      // UpdateLabelTexture), so none are applied here.
      const VPXTextureInfo* texInfo = GetTextureInfo(m_renderState.m_pTexture);
      ClipDrawImage(ctx, m_renderState.m_pTexture, 1.f, 1.f, 1.f, alpha,
         0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height),
         0.f, 0.f, 0.f, dest, rect);
   }
}

PUPLabel::RenderState PUPLabel::UpdateImageTexture(PUP_LABEL_TYPE type, const std::filesystem::path& szPath)
{
   SetThreadName("PUPLabel.Upd." + m_szName);

   RenderState rs;

   // Handle Image 'labels'
   if (type == PUP_LABEL_TYPE_IMAGE)
   {
      // Treat an active wiggle as rotating so the cached source survives the angle crossing 0.
      const bool rotating = (m_renderAngle != 0.f) || (m_animation && m_animation->IsWiggle());
      if (!rotating)
      {
         // No rotation: texture directly without keeping a cached source.
         if (m_pSourceImage) {
            SDL_DestroySurface(m_pSourceImage);
            m_pSourceImage = nullptr;
            m_sourceImagePath.clear();
         }
         SDL_Surface* image = IMG_Load(PluginStrings::PathToUTF8(szPath).c_str());
         if (image && image->format != SDL_PIXELFORMAT_RGBA32) {
            SDL_Surface* newImage = SDL_ConvertSurface(image, SDL_PIXELFORMAT_RGBA32);
            SDL_DestroySurface(image);
            image = newImage;
         }
         if (image) {
            rs.m_sourceWidth = image->w;
            rs.m_sourceHeight = image->h;
            if (m_filterMode > 0)
               ApplyFilter(image, m_filterMode);
            rs.m_pPendingSurface = image;
         }
         else
            LOGE("Unable to load image: " + PluginStrings::PathToUTF8(szPath));
      }
      // Rotated image: cache the decoded source so a spinning image does not re-decode every
      // frame, then bake the rotation into a working copy.
      else
      {
         if (!m_pSourceImage || m_sourceImagePath != szPath)
         {
            if (m_pSourceImage)
               SDL_DestroySurface(m_pSourceImage);
            m_pSourceImage = IMG_Load(PluginStrings::PathToUTF8(szPath).c_str());
            if (m_pSourceImage && m_pSourceImage->format != SDL_PIXELFORMAT_RGBA32) {
               SDL_Surface* newImage = SDL_ConvertSurface(m_pSourceImage, SDL_PIXELFORMAT_RGBA32);
               SDL_DestroySurface(m_pSourceImage);
               m_pSourceImage = newImage;
            }
            m_sourceImagePath = szPath;
         }

         if (m_pSourceImage) {
            SDL_Surface* work = SDL_ConvertSurface(m_pSourceImage, SDL_PIXELFORMAT_RGBA32);
            if (work) {
               if (m_filterMode > 0)
                  ApplyFilter(work, m_filterMode);
               SDL_Surface* rotated = RotateSurface(work, m_renderAngle);
               if (rotated) {
                  SDL_DestroySurface(work);
                  work = rotated;
               }
            }
            SDL_Surface* const tex = work ? work : m_pSourceImage;
            rs.m_sourceWidth = m_pSourceImage->w;
            rs.m_sourceHeight = m_pSourceImage->h;
            rs.m_imageScaleX = static_cast<float>(tex->w) / static_cast<float>(m_pSourceImage->w);
            rs.m_imageScaleY = static_cast<float>(tex->h) / static_cast<float>(m_pSourceImage->h);
            // work is already an owned copy; the cached source is duplicated so it survives
            rs.m_pPendingSurface = work ? work : SDL_DuplicateSurface(tex);
         }
         else
            LOGE("Unable to load image: " + PluginStrings::PathToUTF8(szPath));
      }
   }
   else if (type == PUP_LABEL_TYPE_GIF)
   {
      rs.m_pAnimation = std::shared_ptr<IMG_Animation>(IMG_LoadAnimation(PluginStrings::PathToUTF8(szPath).c_str()), IMG_FreeAnimation);
      if (rs.m_pAnimation) {
         rs.m_accumulatedDelays.resize(rs.m_pAnimation->count);
         int accum = 0;
         for (int i = 0; i < rs.m_pAnimation->count; i++)
         {
            accum += rs.m_pAnimation->delays[i];
            rs.m_accumulatedDelays[i] = accum;
         }
         SDL_Surface* image = rs.m_pAnimation->frames[0];
         if (image) {
            rs.m_sourceWidth = image->w;
            rs.m_sourceHeight = image->h;
            rs.m_pPendingSurface = image->format == SDL_PIXELFORMAT_RGBA32 ? SDL_DuplicateSurface(image) : SDL_ConvertSurface(image, SDL_PIXELFORMAT_RGBA32);
         }
      }
      else
         LOGE("Unable to load animation: " + PluginStrings::PathToUTF8(szPath));
   }
   return rs;
}

PUPLabel::RenderState PUPLabel::UpdateLabelTexture(int outWidth, int outHeight, PUPFont* pFont, const string& szCaption, float size, int color, int shadowstate, int shadowcolor, SDL_FPoint offsetPixels)
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
   if (m_underline) style |= TTF_STYLE_UNDERLINE;

   const PUPFont::SizeMetrics metrics = pFont->Apply(fontHeight);
   TTF_Font* const pTTFFont = pFont->GetTTFFont();

   const float shadowTop = (m_shadowType == 2) ? static_cast<float>(SDL_max(1, static_cast<int>(SDL_max(std::abs(offsetPixels.x), std::abs(offsetPixels.y))) - 2)) : 0.f;
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

   // Replace any codepoint the font cannot render with a space, keeping newlines. This must
   // walk UTF-8 by codepoint: TTF_FontHasGlyph takes a Unicode codepoint, so testing raw bytes
   // would reject every byte of a multibyte character (e.g. the 'é' in "Crédits") and drop it.
   {
      string sanitized;
      sanitized.reserve(text.size());
      const char* p = text.c_str();
      size_t remaining = text.size();
      while (remaining > 0)
      {
         const char* const start = p;
         const uint32_t cp = SDL_StepUTF8(&p, &remaining);
         const size_t bytes = static_cast<size_t>(p - start);
         if (cp == '\n' || (cp != SDL_INVALID_UNICODE_CODEPOINT && TTF_FontHasGlyph(pTTFFont, cp)))
            sanitized.append(start, bytes);
         else
            sanitized += ' ';
      }
      text = std::move(sanitized);
   }

   const int lineCount = 1 + static_cast<int>(std::count(text.begin(), text.end(), '\n'));

   // See pDMDLabelSetAutoSize - shrink font until text fits within autow/autoh dimensions
   const float maxW = (m_autoFitWidth > 0) ? (m_autoFitWidth / 100.0f) * static_cast<float>(outWidth) : 0;
   const float maxH = (m_autoFitHeight > 0) ? (m_autoFitHeight / 100.0f) * static_cast<float>(outHeight) : 0;
   float adjustedHeight = fontHeight;

   SDL_Color textColor = { GetRValue(color), GetGValue(color), GetBValue(color), 255 };
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

   rs.m_logicalHeight = adjustedHeight * static_cast<float>(lineCount);

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
      uint32_t* const __restrict pixels = (uint32_t*)pTextSurface->pixels;
      const int pitch = pTextSurface->pitch / 4;
      const int h = pTextSurface->h;
      const int hm1 = (h > 1 ? h - 1 : 1);
      // gradstate is the percentage of the text kept at the start color before
      // the gradient starts (PUP 1..100).
      const int startLine = static_cast<int>(floorf(hm1 * (std::clamp(m_gradState, 0, 100) / 100.0f)));
      const int gradHeight = std::max(1, hm1 - startLine);
      for (int y = 0; y < h; y++)
      {
         const int gy = std::clamp(y - startLine, 0, gradHeight);
         const int sR = GetRValue(color), sG = GetGValue(color), sB = GetBValue(color);
         const uint32_t gr = sR + gy * ((int)GetRValue(m_gradColor) - sR) / gradHeight;
         const uint32_t gg = sG + gy * ((int)GetGValue(m_gradColor) - sG) / gradHeight;
         const uint32_t gb = sB + gy * ((int)GetBValue(m_gradColor) - sB) / gradHeight;
         const uint32_t g = (gb << 16) | (gg << 8) | gr;
         size_t ofs = y*pitch;
         for (int x = 0; x < pTextSurface->w; x++,ofs++)
         {
            uint32_t& px = pixels[ofs];
            const uint32_t a = px & 0xFF000000u;
            if (a != 0)
               px = a | g;
         }
      }
      SDL_UnlockSurface(pTextSurface);
   }

   const int xoffset = static_cast<int>(offsetPixels.x);
   const int yoffset = static_cast<int>(offsetPixels.y);
   const int absxoff = abs(xoffset);
   const int absyoff = abs(yoffset);

   if (shadowstate == 2 && (absxoff != 0 || absyoff != 0))
   {
      // See pDMDLabelSetBorder - render foreground text inside a true outline glyph
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

      if (m_renderAngle != 0.f)
      {
         SDL_Surface* pRotated = RotateSurface(pMergedSurface, m_renderAngle);
         if (pRotated)
         {
            SDL_DestroySurface(pMergedSurface);
            pMergedSurface = pRotated;
         }
      }

      rs.m_width = static_cast<float>(pMergedSurface->w);
      rs.m_height = static_cast<float>(pMergedSurface->h);
      rs.m_pPendingSurface = pMergedSurface;
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
      // See pDMDLabelSetShadow - drop shadow: render shadow text at offset, foreground at origin
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

   if (m_renderAngle != 0.f)
   {
      SDL_Surface* pRotated = RotateSurface(pMergedSurface, m_renderAngle);
      if (pRotated)
      {
         SDL_DestroySurface(pMergedSurface);
         pMergedSurface = pRotated;
      }
   }

   rs.m_width = static_cast<float>(pMergedSurface->w);
   rs.m_height = static_cast<float>(pMergedSurface->h);
   rs.m_pPendingSurface = pMergedSurface;
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
      m_xPos, m_yPos, m_pagenum, PluginStrings::PathToUTF8(m_szPath));
}

namespace
{
   float ApplyTween(float pos, int tween)
   {
      switch (tween)
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
      return pos;
   }

   string FormatPupNumber(int num, int format)
   {
      string numStr = std::to_string(num);
      if (format == 1)
      {
         const int firstDigit = (num < 0) ? 1 : 0; // No separator after the minus sign
         int insertPos = static_cast<int>(numStr.length()) - 3;
         while (insertPos > firstDigit)
         {
            numStr.insert(insertPos, ",");
            insertPos -= 3;
         }
         // PUP's %2.0n formatting pads to 2 characters and adds spaces around the value
         if (numStr.length() < 2)
            numStr.insert(0, 1, ' ');
         numStr = ' ' + numStr + ' ';
      }
      return numStr;
   }
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, int flashingPeriod)
   : m_color(foregroundColor)
   , m_label(label)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_flashingPeriod(flashingPeriod)
{
   CaptureState();
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float xps, float xpe, float yps, float ype, int motionLen, int motionTween, int motionColor)
   : m_color(foregroundColor)
   , m_label(label)
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
   CaptureState();
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, int alphaStart, int alphaEnd, int pulseSpeed, bool screenFade)
   : m_color(foregroundColor)
   , m_label(label)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_alphaStart(alphaStart)
   , m_alphaEnd(alphaEnd)
   , m_pulseSpeed(pulseSpeed)
   , m_screenFade(screenFade)
{
   m_alpha = static_cast<float>(m_alphaStart) / 255.0f;
   CaptureState();
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float zoomStart, float zoomEnd, int zoomPulseSpeed)
   : m_color(foregroundColor)
   , m_label(label)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_zoomStart(zoomStart)
   , m_zoomEnd(zoomEnd)
   , m_zoomPulseSpeed(zoomPulseSpeed)
{
   m_zoom = m_zoomStart;
   CaptureState();
}

PUPLabel::Animation::Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float wiggleStart, float wiggleEnd, int wiggleSpeed, int dummy)
   : m_color(foregroundColor)
   , m_label(label)
   , m_lengthMs(lengthMs)
   , m_foregroundColor(foregroundColor)
   , m_startTimestamp(SDL_GetTicks())
   , m_wiggleStart(wiggleStart)
   , m_wiggleEnd(wiggleEnd)
   , m_wiggleSpeed(wiggleSpeed)
{
   (void)dummy;
   m_wiggle = m_wiggleStart;
   CaptureState();
}

void PUPLabel::Animation::CaptureState()
{
   m_savedVisible = m_label->m_visible;
   m_savedAlpha = m_label->m_alpha;
   m_savedZoom = m_label->m_zoom;
   m_savedColor = m_label->m_color;
   m_savedFilterMode = m_label->m_filterMode;
}

void PUPLabel::Animation::Restore()
{
   m_label->m_visible = m_savedVisible;
   m_label->m_alpha = m_savedAlpha;
   m_label->m_zoom = m_savedZoom;
   if (m_label->m_color != m_savedColor)
   {
      m_label->m_color = m_savedColor;
      m_label->m_dirty = true;
   }
   if (m_invertFilter && m_label->m_filterMode != m_savedFilterMode)
   {
      m_label->m_filterMode = m_savedFilterMode;
      m_label->m_dirty = true;
   }
}

bool PUPLabel::Animation::Update(const SDL_Rect& screenRect, const SDL_FRect& labelRect)
{
   const uint64_t elapsed = SDL_GetTicks() - m_startTimestamp;

   if (elapsed >= m_lengthMs)
   {
      // Set the final number before restoring the rest of the label state.
      if (m_numStart != INT_MIN)
      {
         const string numStr = FormatPupNumber(m_numEnd, m_numFormat);
         if (numStr != m_numText)
         {
            m_numText = numStr;
            m_label->m_szCaption = numStr;
            m_label->m_dirty = true;
         }
      }
      if (m_screenFade && m_label->m_pScreen)
         m_label->m_pScreen->m_overlayAlpha = static_cast<float>(m_alphaEnd) / 255.f;
      Restore();
      return true;
   }

   if (m_flashingPeriod)
   {
      if (m_invertFilter)
      {
         // See pDMDLabelFlashFilter - toggle the label's filter every fq ms.
         const bool filterOn = ((elapsed / m_flashingPeriod) & 1) != 0;
         const int newFilterMode = filterOn ? m_filterMode : 0;
         if (m_label->m_filterMode != newFilterMode)
         {
            m_label->m_filterMode = newFilterMode;
            m_label->m_dirty = true;
         }
      }
      else
      {
         // See pDMDLabelFlash - start hidden, then toggle visibility every fq milliseconds
         m_visible = ((elapsed / m_flashingPeriod) & 1) != 0;
      }
      m_color = m_foregroundColor;
   }

   if (m_delayShow > 0)
   {
      // See pDMDLabelClone - hide until delay elapsed, then show
      m_visible = (static_cast<int>(elapsed) >= m_delayShow);
   }

   if (m_motionLen)
   {
      float pos = saturate(static_cast<float>(elapsed) / static_cast<float>(m_motionLen));
      pos = ApplyTween(pos, m_motionTween);

      const float xBase = labelRect.x;
      const float xLeft = static_cast<float>(screenRect.x) - labelRect.w;
      const float xRight = static_cast<float>(screenRect.x) + static_cast<float>(screenRect.w);
      const float yBase = labelRect.y;
      const float yTop = static_cast<float>(screenRect.y) - labelRect.h;
      const float yBottom = static_cast<float>(screenRect.y) + static_cast<float>(screenRect.h);
      const float screenW = static_cast<float>(screenRect.w);
      const float screenH = static_cast<float>(screenRect.h);

      // PUP position values: -1 = off-screen left/top, 0 = current position,
      // 1 = off-screen right/bottom, 2 = the 'xp'/'yp' pixel position (end only, already
      // applied to the base position by Render), otherwise a percentage of the display.
      const auto mapX = [&](float value, bool isStart) -> float
      {
         if (value == 0.f)
            return xBase;
         if (value == 1.f)
            return xRight;
         if (value == -1.f)
            return xLeft;
         if (value == 2.f && !isStart)
            return xBase;
         return static_cast<float>(screenRect.x) + screenW * value / 100.f;
      };
      const auto mapY = [&](float value, bool isStart) -> float
      {
         if (value == 0.f)
            return yBase;
         if (value == 1.f)
            return yBottom;
         if (value == -1.f)
            return yTop;
         if (value == 2.f && !isStart)
            return yBase;
         return static_cast<float>(screenRect.y) + screenH * value / 100.f;
      };

      const float xs = mapX(m_xps, true);
      const float xe = mapX(m_xpe, false);
      const float ys = mapY(m_yps, true);
      const float ye = mapY(m_ype, false);
      m_color = pos < 1.0f ? m_motionColor : m_foregroundColor;
      m_xOffset = lerp(xs, xe, pos) - xBase;
      m_yOffset = lerp(ys, ye, pos) - yBase;
   }

   if (m_alphaStart != m_alphaEnd)
   {
      const float as = static_cast<float>(m_alphaStart) / 255.0f;
      const float ae = static_cast<float>(m_alphaEnd) / 255.0f;
      if (m_pulseSpeed > 0)
      {
         // See pDMDLabelFadePulse - bounce alpha between astart and aend
         const float range = fabsf(static_cast<float>(m_alphaEnd - m_alphaStart));
         const float periodMs = std::max(1.f, (range / static_cast<float>(m_pulseSpeed)) * (2.f * 30.f));
         const float cyclePos = fmodf(static_cast<float>(elapsed), periodMs) / periodMs;
         const float t = cyclePos < 0.5f ? cyclePos * 2.0f : 2.0f - cyclePos * 2.0f;
         m_alpha = as + t * (ae - as);
      }
      else
      {
         // See pDMDLabelFadeOut / pDMDLabelFadeIn / pDMDScreenFadeOut / pDMDScreenFadeIn
         const float fadeMs = static_cast<float>(m_motionLen > 0 ? m_motionLen : static_cast<int>(m_lengthMs));
         const float pos = saturate(static_cast<float>(elapsed) / std::max(1.f, fadeMs));
         m_alpha = as + pos * (ae - as);
      }
      m_color = m_foregroundColor;
   }

   if (m_zoomStart != m_zoomEnd)
   {
      if (m_zoomIsPulse)
      {
         // See pDMDLabelPulseText / pDMDLabelPulseImage - bounce zoom between hstart and hend.
         // PUP advances by pspeed pixels every 30 ms; a missing speed defaults to 3% of the label height.
         const float rangePx = fabsf(m_zoomEnd - m_zoomStart) * labelRect.h / 100.f;
         const float speedPx = (m_zoomPulseSpeed > 0) ? static_cast<float>(m_zoomPulseSpeed) : std::max(1.f, labelRect.h * 0.03f);
         const float periodMs = std::max(1.f, (rangePx / speedPx) * (2.f * 30.f));
         const float cyclePos = fmodf(static_cast<float>(elapsed), periodMs) / periodMs;
         const float t = cyclePos < 0.5f ? cyclePos * 2.0f : 2.0f - cyclePos * 2.0f;
         m_zoom = m_zoomStart + t * (m_zoomEnd - m_zoomStart);
      }
      else
      {
         // See pDMDZoomBig - hstart/hend are pixel heights expressed as a
         // percentage of the display, or of the label when 'ns' is set.
         float startScale = m_zoomStart;
         float endScale = m_zoomEnd;
         if (m_zoomUseDrawHeight)
         {
            startScale /= 100.f;
            endScale /= 100.f;
         }
         else if (m_label->m_type == PUP_LABEL_TYPE_TEXT)
         {
            const float labelSize = std::max(1.f, m_label->EffectiveSize());
            startScale /= labelSize;
            endScale /= labelSize;
         }
         else
         {
            // Image labels are zoomed to a pixel height derived from the display height.
            const float currentHeight = std::max(1.f, labelRect.h);
            startScale = (static_cast<float>(screenRect.h) * m_zoomStart / 100.f) / currentHeight;
            endScale = (static_cast<float>(screenRect.h) * m_zoomEnd / 100.f) / currentHeight;
         }
         const float pos = ApplyTween(saturate(static_cast<float>(elapsed) / std::max(1.f, static_cast<float>(m_motionLen))), m_motionTween);
         m_zoom = 100.f * (startScale + pos * (endScale - startScale));
      }
      m_color = m_foregroundColor;
   }

   if (m_wiggleStart != m_wiggleEnd)
   {
      // See pDMDLabelWiggleText / pDMDLabelWiggleImage - bounce rotation between rstart and rend.
      // PUP advances by rspeed degrees every 30 ms.
      const float range = fabsf(m_wiggleEnd - m_wiggleStart);
      const float periodMs = std::max(1.f, (range / static_cast<float>(std::max(1, m_wiggleSpeed))) * (2.f * 30.f));
      const float cyclePos = fmodf(static_cast<float>(elapsed), periodMs) / periodMs;
      const float t = cyclePos < 0.5f ? cyclePos * 2.0f : 2.0f - cyclePos * 2.0f;
      m_wiggle = m_wiggleStart + t * (m_wiggleEnd - m_wiggleStart);
      m_color = m_foregroundColor;
   }

   // See pDMDLabelPulseNumber - interpolate number and update label caption
   if (m_numStart != INT_MIN)
   {
      const float pos = ApplyTween(saturate(static_cast<float>(elapsed) / std::max(1.f, static_cast<float>(m_motionLen))), m_motionTween);
      const int num = (pos >= 1.f) ? m_numEnd : m_numStart + static_cast<int>(lroundf(pos * static_cast<float>(m_numEnd - m_numStart)));
      const string numStr = FormatPupNumber(num, m_numFormat);
      if (numStr != m_numText)
      {
         m_numText = numStr;
         m_label->m_szCaption = numStr;
         m_label->m_dirty = true;
      }
   }

   // See pDMDLabelMoveHorzFade / pDMDLabelMoveVertFade - fade the last 'af' ms.
   // PUP applies this to the animated copy, so only the copy-based types (1-4, 10) see it.
   if (m_alphaFade > 50 && (m_type <= 4 || m_type == 10))
   {
      const int fadeStart = static_cast<int>(m_lengthMs) - m_alphaFade;
      if (static_cast<int>(elapsed) > fadeStart)
         m_alpha = saturate(static_cast<float>(static_cast<int>(m_lengthMs) - static_cast<int>(elapsed)) / static_cast<float>(m_alphaFade));
   }

   return false;
}


}
