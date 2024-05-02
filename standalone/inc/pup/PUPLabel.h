#pragma once

#include <SDL2/SDL_ttf.h>

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

class PUPLabel
{
public:
   PUPLabel(TTF_Font* pFont, float size, LONG color, float angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, float xPos, float yPos, bool visible, int pagenum);
   ~PUPLabel();

   void SetFont(TTF_Font* pFont) { m_pFont = pFont; m_dirty = true; }
   void SetSize(float size) { m_size = size; m_dirty = true; }
   void SetColor(LONG color) { m_color = color; m_dirty = true; }
   void SetAngle(float angle) { m_angle = angle; m_dirty = true; }
   void SetXAlign(PUP_LABEL_XALIGN xAlign) { m_xAlign = xAlign; m_dirty = true; }
   void SetYAlign(PUP_LABEL_YALIGN yAlign) { m_yAlign = yAlign; m_dirty = true; }
   void SetXPos(float xPos) { m_xPos = xPos; m_dirty = true; }
   void SetYPos(float yPos) { m_yPos = yPos; m_dirty = true; }
   void SetVisible(bool visible) { m_visible = visible; }
   const string& GetCaption() const { return m_szCaption; }
   void SetCaption(const string& szCaption);
   void SetPageNum(int pagenum) { m_pagenum = pagenum; }
   void SetShadowColor(LONG shadowColor) { m_shadowColor = shadowColor; m_dirty = true; }
   void SetShadowState(int shadowState) { m_shadowState = shadowState; m_dirty = true; }
   void SetXOffset(float xoffset) { m_xoffset = xoffset; m_dirty = true; }
   void SetYOffset(float yoffset) { m_yoffset = yoffset; m_dirty = true; }
   void SetOutline(bool outline) { m_outline = outline; m_dirty = true; }
   void Render(SDL_Renderer* renderer, SDL_Rect& rect, int pagenum);

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

   SDL_Texture* m_pTexture;
   int m_textureWidth;
   int m_textureHeight;
};