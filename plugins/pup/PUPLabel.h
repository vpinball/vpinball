// license:GPLv3+

#pragma once

#include "common.h"
#include "PUPFont.h"

#include <climits>
#include <future>

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

namespace PUP {

typedef enum
{
   PUP_LABEL_XALIGN_LEFT,
   PUP_LABEL_XALIGN_CENTER,
   PUP_LABEL_XALIGN_RIGHT
} PUP_LABEL_XALIGN;

typedef enum
{
   PUP_LABEL_YALIGN_TOP,
   PUP_LABEL_YALIGN_CENTER,
   PUP_LABEL_YALIGN_BOTTOM
} PUP_LABEL_YALIGN;

typedef enum
{
   PUP_LABEL_TYPE_TEXT,
   PUP_LABEL_TYPE_IMAGE,
   PUP_LABEL_TYPE_GIF
} PUP_LABEL_TYPE;

class PUPScreen;

class PUPLabel final
{
public:
   PUPLabel(class PUPManager* manager, const string& szName, const string& szFont, float size, int color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos, float yPos, int pagenum, bool visible);
   ~PUPLabel();

   const string& GetCaption() const { return m_szCaption; }
   void SetCaption(const string& szCaption);
   bool GetOnShowVisible() const { return m_onShowVisible; }
   bool IsVisible() const { return m_visible; }
   void SetVisible(bool visible);
   void SetSpecial(const string& szSpecial);
   void Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect, int pagenum, float screenAlpha = 1.f);
   const string& GetName() const { return m_szName; }
   int GetPagenum() const { return m_pagenum; }
   bool GetDoDefault() const { return m_doDefault; }
   bool GetDoDispose() const { return m_doDispose; }
   bool IsAnimating() const { return m_animation != nullptr; }
   void ClearAnimation();
   void SetScreen(PUPScreen* pScreen) { m_pScreen = pScreen; }
   string ToString() const;

private:
   class PUPManager* const m_pManager;
   const string m_szName;

   PUPFont* m_pFont;
   float m_size;
   bool m_sizeIsPoints = false; // 'size' key: PinUP applies it as TFont.Size (points), 'sz'/'fonth' are cell heights
   int m_color;
   float m_angle;
   PUP_LABEL_XALIGN m_xAlign;
   PUP_LABEL_YALIGN m_yAlign;
   float m_xPos;
   float m_yPos;
   bool m_onShowVisible;
   bool m_visible;
   string m_szCaption;
   int m_pagenum;
   int m_shadowColor = 0x808080; // PUP's default shadow color (gray)
   int m_shadowType = 0;
   float m_xoffset = 3.f; // PUP defaults to a 3 pixel drop shadow until xoffset/yoffset are set
   float m_yoffset = 3.f;
   bool m_xOffsetIsPercent = false;
   bool m_yOffsetIsPercent = false;
   int m_outline = 0;
   bool m_bold = false;
   bool m_italic = false;
   bool m_underline = false;
   float m_alpha = 1.0f;
   bool m_useAbsolutePos = false;
   bool m_doDefault = true;
   bool m_doDispose = false;
   bool m_disposeOnAnimEnd = false;
   float m_zoom = 100.0f;
   int m_gradColor = 0;
   int m_gradState = 0;
   int m_filterMode = 0;
   float m_autoFitWidth = 0;
   float m_autoFitHeight = 0;
   PUPScreen* m_pScreen = nullptr;
   int m_anigif = 0; // frame interval in milliseconds, 0 to use the image's own frame delays
   bool m_gifLoop = true;
   int m_gifStart = -1;
   int m_gifEnd = -1;
   bool m_hideOnAnimEnd = false;
   bool m_animating = false;
   bool m_disposePending = false;
   bool m_imageWidthSet = false;
   bool m_imageHeightSet = false;

   PUP_LABEL_TYPE m_type = PUP_LABEL_TYPE_TEXT;
   std::filesystem::path m_szPath;
   float m_imageWidth = 0; // Width of image (unused for text)
   float m_imageHeight = 0; // height of image (unused for text)
   float m_renderAngle = 0.f; // Angle baked into the current texture (static m_angle + active wiggle)
   SDL_Surface* m_pSourceImage = nullptr; // Cached decoded image, so rotation does not re-decode each frame
   std::filesystem::path m_sourceImagePath; // Path m_pSourceImage was decoded from

   std::mutex m_mutex;
   static std::mutex m_fontMutex;

   class RenderState final
   {
   public:
      RenderState() { }

      ~RenderState()
      {
         if (m_pTexture)
            DeleteTexture(m_pTexture);
         if (m_pPendingSurface)
            SDL_DestroySurface(m_pPendingSurface);
      }

      RenderState(RenderState&& other) noexcept
         : m_pTexture(other.m_pTexture)
         , m_pPendingSurface(other.m_pPendingSurface)
         , m_prerenderedHeight(other.m_prerenderedHeight)
         , m_prerenderedColor(other.m_prerenderedColor)
         , m_sourceWidth(other.m_sourceWidth)
         , m_sourceHeight(other.m_sourceHeight)
         , m_width(other.m_width)
         , m_height(other.m_height)
         , m_logicalHeight(other.m_logicalHeight)
         , m_baselineOffset(other.m_baselineOffset)
         , m_imageScaleX(other.m_imageScaleX)
         , m_imageScaleY(other.m_imageScaleY)
         , m_pAnimation(std::move(other.m_pAnimation))
         , m_accumulatedDelays(std::move(other.m_accumulatedDelays))
      {
         other.m_pTexture = nullptr;
         other.m_pPendingSurface = nullptr;
      }

      RenderState& operator=(RenderState&& other) noexcept
      {
         if (this != &other)
         {
            if (m_pTexture)
               DeleteTexture(m_pTexture);
            if (m_pPendingSurface)
               SDL_DestroySurface(m_pPendingSurface);

            m_prerenderedHeight = other.m_prerenderedHeight;
            m_prerenderedColor = other.m_prerenderedColor;
            m_sourceWidth = other.m_sourceWidth;
            m_sourceHeight = other.m_sourceHeight;
            m_pTexture = other.m_pTexture;
            m_pPendingSurface = other.m_pPendingSurface;
            m_pAnimation = std::move(other.m_pAnimation);
            m_accumulatedDelays = std::move(other.m_accumulatedDelays);
            m_width = other.m_width;
            m_height = other.m_height;
            m_logicalHeight = other.m_logicalHeight;
            m_baselineOffset = other.m_baselineOffset;
            m_imageScaleX = other.m_imageScaleX;
            m_imageScaleY = other.m_imageScaleY;

            other.m_pTexture = nullptr;
            other.m_pPendingSurface = nullptr;
         }
         return *this;
      }

      // The VPX texture API is not thread safe, so async updates rasterize to
      // m_pPendingSurface and the texture is created once the render state is
      // collected on the API thread (see Render).
      void CreateTextureFromSurface()
      {
         if (m_pPendingSurface == nullptr)
            return;
         m_pTexture = CreateTexture(m_pPendingSurface);
         SDL_DestroySurface(m_pPendingSurface);
         m_pPendingSurface = nullptr;
      }

      VPXTexture m_pTexture = nullptr;
      SDL_Surface* m_pPendingSurface = nullptr;
      int m_prerenderedHeight = 0; // Height used to evaluate text rendering
      int m_prerenderedColor = 0; // Color used for prerendering the text
      int m_sourceWidth = 0; // Native width of the loaded image/animation
      int m_sourceHeight = 0; // Native height of the loaded image/animation
      float m_width = 0; // Width of rendered text (unused for images)
      float m_height = 0; // Height of rendered text (unused for images)
      float m_logicalHeight = 0; // Windows-equivalent text-box height for alignment
      float m_baselineOffset = 0; // Y shift to align SDL_ttf baseline with Windows GDI baseline
      float m_imageScaleX = 1.f; // CPU rotation grows the texture to its bounding box; scale dest to compensate
      float m_imageScaleY = 1.f;
      std::shared_ptr<IMG_Animation> m_pAnimation;
      vector<int> m_accumulatedDelays;
   };
   RenderState UpdateImageTexture(PUP_LABEL_TYPE type, const std::filesystem::path& szPath);
   RenderState UpdateLabelTexture(int outWidth, int outHeight, PUPFont* pFont, const string& szCaption, float size, int color, int shadowstate, int shadowcolor, SDL_FPoint offsetPixels);
   void ClearAnimationInternal();
   float EffectiveSize() const;
   SDL_FPoint GetShadowOffsetPixels(float fontHeight) const;

   class Animation {
   public:
      Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, int flashingPeriod);
      Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float xps, float xpe, float yps, float ype, int motionLen, int motionTween, int motionColor);
      Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, int alphaStart, int alphaEnd, int pulseSpeed, bool screenFade);
      Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float zoomStart, float zoomEnd, int zoomPulseSpeed);
      Animation(PUPLabel* label, unsigned int lengthMs, int foregroundColor, float wiggleStart, float wiggleEnd, int wiggleSpeed, int dummy);

      bool Update(const SDL_Rect& screenRect, const SDL_FRect& labelRect);
      void Restore();
      void SetMotionTiming(int motionLen, int motionTween) { m_motionLen = motionLen; m_motionTween = motionTween; }
      bool IsFade() const { return m_alphaStart != m_alphaEnd; }
      bool IsScreenFade() const { return m_screenFade; }
      bool IsZoom() const { return m_zoomStart != m_zoomEnd; }
      bool IsWiggle() const { return m_wiggleStart != m_wiggleEnd; }

      bool m_visible = true;
      float m_xOffset = 0.f;
      float m_yOffset = 0.f;
      float m_alpha = 1.f;
      float m_zoom = 100.f;
      float m_wiggle = 0.f;
      int m_color;

      // PUP animation nuances
      int m_type = 0; // 'at'
      bool m_hasPixelX = false; // 'xp' / 'yp': pixel position of the animated copy
      bool m_hasPixelY = false;
      float m_pixelX = 0.f;
      float m_pixelY = 0.f;
      bool m_zoomIsPulse = false; // at=4 (bounce) vs at=3 (single zoom)
      bool m_zoomUseDrawHeight = false; // at=3 'ns' flag: percentages of the label instead of the display
      bool m_invertFilter = false; // at=9
      int m_filterMode = 0; // at=9 filter to toggle

   private:
      void CaptureState();

      PUPLabel* m_label;
      const unsigned int m_lengthMs;
      const int m_foregroundColor;
      const uint64_t m_startTimestamp;

      // State to restore when the animation ends or is stopped
      bool m_savedVisible = true;
      float m_savedAlpha = 1.f;
      float m_savedZoom = 100.f;
      int m_savedColor = 0;
      int m_savedFilterMode = 0;

      int m_flashingPeriod = 0;

      float m_xps = 0.f;
      float m_xpe = 0.f;
      float m_yps = 0.f;
      float m_ype = 0.f;
      int m_motionLen = 0;
      int m_motionTween = 0;
      int m_motionColor = 0;
   public:
      int m_alphaFade = 0;
   private:

      int m_alphaStart = 255;
      int m_alphaEnd = 255;
      int m_pulseSpeed = 0;
      bool m_screenFade = false;

      float m_zoomStart = 100.f;
      float m_zoomEnd = 100.f;
      int m_zoomPulseSpeed = 0;

      float m_wiggleStart = 0.f;
      float m_wiggleEnd = 0.f;
      int m_wiggleSpeed = 0;

   public:
      int m_delayShow = 0;
      int m_numStart = INT_MIN;
      int m_numEnd = INT_MIN;
      int m_numFormat = 0;
      string m_numText;
   };

   bool m_dirty = true;
   std::unique_ptr<Animation> m_animation;
   RenderState m_renderState;
   int m_animationFrame = 0;
   uint64_t m_animationStart = 0;
   std::future<RenderState> m_pendingTextureUpdate;
};

}
