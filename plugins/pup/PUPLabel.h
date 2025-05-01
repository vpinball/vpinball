#pragma once

#include "common.h"

#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

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

class PUPLabel
{
public:
   PUPLabel(class PUPManager* manager, const string& szName, const string& szFont, float size, int color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos, float yPos, int pagenum, bool visible);
   ~PUPLabel();

   const string& GetCaption() const { return m_szCaption; }
   void SetCaption(const string& szCaption);
   void SetVisible(bool visible);
   void SetSpecial(const string& szSpecial);
   void Render(VPXRenderContext2D* const ctx, SDL_Rect& rect, int pagenum);
   const string& GetName() const { return m_szName; }
   void SetScreen(PUPScreen* pScreen) { m_pScreen = pScreen; }
   string ToString() const;

private:
   void UpdateLabelTexture(SDL_Rect& rect);

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
   bool m_dirty = true;
   PUPScreen* m_pScreen = nullptr;
   VPXTexture m_pTexture = nullptr;
   float m_width = 0;
   float m_height = 0;
   int m_anigif = 0;
   PUP_LABEL_TYPE m_type = PUP_LABEL_TYPE_TEXT;
   IMG_Animation* m_pAnimation = nullptr;
   string m_szPath;
   int m_frame = 0;
   std::mutex m_mutex;
};