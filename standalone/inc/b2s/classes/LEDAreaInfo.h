#pragma once

class LEDAreaInfo
{
public:
   LEDAreaInfo(const SDL_Rect& rect, bool isOnDMD);

   SDL_Rect& GetRect() { return m_rect; }
   void SetRect(const SDL_Rect& rect) { m_rect = rect; }
   bool IsOnDMD() { return m_onDMD; }
   void SetOnDMD(bool isOnDMD) { m_onDMD = isOnDMD; }

private:
   SDL_Rect m_rect;
   int m_onDMD;
};