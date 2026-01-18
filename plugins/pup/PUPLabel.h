// license:GPLv3+

#pragma once

#include "common.h"

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
   void SetVisible(bool visible);
   void SetSpecial(const string& szSpecial);
   void Render(VPXRenderContext2D* const ctx, const SDL_Rect& rect, int pagenum);
   const string& GetName() const { return m_szName; }
   void SetScreen(PUPScreen* pScreen) { m_pScreen = pScreen; }
   string ToString() const;

private:
   class PUPManager* const m_pManager;
   const string m_szName;

   TTF_Font* m_pFont;
   float m_size;
   int m_color;
   float m_angle;
   PUP_LABEL_XALIGN m_xAlign;
   PUP_LABEL_YALIGN m_yAlign;
   float m_xPos;
   float m_yPos;
   bool m_visible;
   string m_szCaption;
   int m_pagenum;
   int m_shadowColor = 0;
   int m_shadowState = 0;
   float m_xoffset = 0;
   float m_yoffset = 0;
   bool m_outline = false;
   PUPScreen* m_pScreen = nullptr;
   int m_anigif = 0;

   PUP_LABEL_TYPE m_type = PUP_LABEL_TYPE_TEXT;
   std::filesystem::path m_szPath;
   float m_imageWidth = 0; // Width of image (unused for text)
   float m_imageHeight = 0; // height of image (unused for text)

   std::mutex m_mutex;

   class RenderState final
   {
   public:
      RenderState() { }

      ~RenderState()
      {
         if (m_pTexture)
            DeleteTexture(m_pTexture);
         if (m_pAnimation)
            IMG_FreeAnimation(m_pAnimation);
      }

      RenderState(RenderState&& other) noexcept
         : m_pTexture(other.m_pTexture)
         , m_prerenderedHeight(other.m_prerenderedHeight)
         , m_prerenderedColor(other.m_prerenderedColor)
         , m_width(other.m_width)
         , m_height(other.m_height)
         , m_pAnimation(other.m_pAnimation)
      {
         other.m_pTexture = nullptr;
         other.m_pAnimation = nullptr;
      }

      RenderState& operator=(RenderState&& other) noexcept
      {
         if (this != &other)
         {
            if (m_pTexture)
               DeleteTexture(m_pTexture);
            if (m_pAnimation)
               IMG_FreeAnimation(m_pAnimation);

            m_prerenderedHeight = other.m_prerenderedHeight;
            m_prerenderedColor = other.m_prerenderedColor;
            m_pTexture = other.m_pTexture;
            m_pAnimation = other.m_pAnimation;
            m_width = other.m_width;
            m_height = other.m_height;

            other.m_pTexture = nullptr;
            other.m_pAnimation = nullptr;
         }
         return *this;
      }

      VPXTexture m_pTexture = nullptr;
      int m_prerenderedHeight = 0; // Height used to evaluate text rendering
      int m_prerenderedColor = 0; // Color used for prerendering the text
      float m_width = 0; // Width of rendered text (unused for images)
      float m_height = 0; // height of rendered text (unused for images)
      IMG_Animation* m_pAnimation = nullptr;
   };
   RenderState UpdateImageTexture(PUP_LABEL_TYPE type, const std::filesystem::path& szPath);
   RenderState UpdateLabelTexture(int outHeight, TTF_Font* pFont, const string& szCaption, float size, int color, int shadowstate, int shadowcolor, SDL_FPoint offset);

   class Animation {
   public:
      Animation(PUPLabel* label, int lengthMs, int foregroundColor, int flashingFreq);
      Animation(PUPLabel* label, int lengthMs, int foregroundColor, int xps, int xpe, int yps, int ype, int motionLen, int motionTween, int motionColor);

      bool Update(const SDL_Rect& screenRect, const SDL_FRect& labelRect);

      bool m_visible = true;
      float m_xOffset = 0.f;
      float m_yOffset = 0.f;
      int m_color;

   private:
      const PUPLabel* m_label;
      const int m_lengthMs;
      const int m_foregroundColor;
      const uint64_t m_startTimestamp;

      int m_flashingPeriod = 0;

      int m_xps = 0;
      int m_xpe = 0;
      int m_yps = 0;
      int m_ype = 0;
      int m_motionLen = 0;
      int m_motionTween = 0;
      int m_motionColor = 0;
   };

   bool m_dirty = true;
   std::unique_ptr<Animation> m_animation;
   RenderState m_renderState;
   int m_animationFrame = 0;
   uint64_t m_animationStart = 0;
   std::future<RenderState> m_pendingTextureUpdate;
};

}
