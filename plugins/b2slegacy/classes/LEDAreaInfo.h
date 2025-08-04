#pragma once


namespace B2SLegacy {

class LEDAreaInfo
{
public:
   LEDAreaInfo(const SDL_Rect& rect, bool isOnDMD);

   const SDL_Rect& GetRect() const { return m_rect; }
   void SetRect(const SDL_Rect& rect) { m_rect = rect; }
   bool IsOnDMD() const { return m_onDMD; }
   void SetOnDMD(bool isOnDMD) { m_onDMD = isOnDMD; }

private:
   SDL_Rect m_rect;
   int m_onDMD;
};

}
