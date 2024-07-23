#pragma once

#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

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
   PUPLabel(const string& szName, const string& szFont, float size, LONG color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos, float yPos, int pagenum, bool visible);
   ~PUPLabel();

   const string& GetCaption() const { return m_szCaption; }
   void SetCaption(const string& szCaption);
   void SetVisible(bool visible);
   void SetSpecial(const string& szSpecial);
   void Render(SDL_Renderer* renderer, SDL_Rect& rect, int pagenum);
   const string& GetName() const { return m_szName; }
   void SetScreen(PUPScreen* pScreen) { m_pScreen = pScreen; }
   string ToString() const;

private:
   void UpdateLabelTexture(SDL_Renderer* pRenderer, SDL_Rect& rect);

   TTF_Font* m_pFont;
   float m_size;
   LONG m_color;
   float m_angle;
   PUP_LABEL_XALIGN m_xAlign;
   PUP_LABEL_YALIGN m_yAlign;
   float m_xPos;
   float m_yPos;
   bool m_visible;
   string m_szCaption;
   int m_pagenum;
   LONG m_shadowColor;
   int m_shadowState;
   float m_xoffset;
   float m_yoffset;
   bool m_outline;
   bool m_dirty;
   PUPScreen* m_pScreen;
   SDL_Texture* m_pTexture;
   float m_width;
   float m_height;
   string m_szName;
   int m_anigif;
   PUP_LABEL_TYPE m_type;
   IMG_Animation* m_pAnimation;
   string m_szPath;
   int m_frame;
   std::mutex m_mutex;
};