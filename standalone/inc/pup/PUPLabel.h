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

   void SetFont(TTF_Font* pFont) { m_pFont = pFont; }
   void SetSize(float size) { m_size = size; }
   void SetColor(LONG color) { m_color = color; }
   void SetAngle(float angle) { m_angle = angle; }
   void SetXAlign(PUP_LABEL_XALIGN xAlign) { m_xAlign = xAlign; }
   void SetYAlign(PUP_LABEL_YALIGN yAlign) { m_yAlign = yAlign; }
   void SetXPos(float xPos) { m_xPos = xPos; }
   void SetYPos(float yPos) { m_yPos = yPos; }
   void SetVisible(bool visible) { m_visible = visible; }
   const string& GetText() const { return m_szText; }
   void SetText(const string& szText) { m_szText = szText; }
   void SetPageNum(int pagenum) { m_pagenum = pagenum; }
   void Render(SDL_Renderer* renderer, SDL_Rect& rect, int pagenum);

private:
   TTF_Font* m_pFont;
   float m_size;
   LONG m_color;
   float m_angle;
   PUP_LABEL_XALIGN m_xAlign;
   PUP_LABEL_YALIGN m_yAlign;
   float m_xPos;
   float m_yPos;
   bool m_visible;
   string m_szText;
   int m_pagenum;
   SDL_Texture* m_pTexture;
};