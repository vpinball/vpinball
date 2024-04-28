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
   PUPLabel(TTF_Font* pFont, int size, LONG color, LONG angle, PUP_LABEL_XALIGN xAlign, PUP_LABEL_YALIGN yAlign, int xPos, int yPos, bool visible);
   ~PUPLabel();

   void SetFont(TTF_Font* pFont) { m_pFont = pFont; }
   void SetSize(int size) { m_size = size; }
   void SetColor(LONG color) { m_color = color; }
   void SetAngle(LONG angle) { m_angle = angle; }
   void SetXAlign(PUP_LABEL_XALIGN xAlign) { m_xAlign = xAlign; }
   void SetYAlign(PUP_LABEL_YALIGN yAlign) { m_yAlign = yAlign; }
   void SetXPos(int xPos) { m_xPos = xPos; }
   void SetYPos(int yPos) { m_yPos = yPos; }
   void SetVisible(bool visible) { m_visible = visible; }
   const string& GetText() const { return m_szText; }
   void SetText(const string& szText) { m_szText = szText; }

   void Render(SDL_Renderer* renderer, SDL_Rect& rect);

private:
   TTF_Font* m_pFont;
   int m_size;
   LONG m_color;
   LONG m_angle;
   PUP_LABEL_XALIGN m_xAlign;
   PUP_LABEL_YALIGN m_yAlign;
   int m_xPos;
   int m_yPos;
   bool m_visible;

   string m_szText;
   SDL_Texture* m_pTexture;
};