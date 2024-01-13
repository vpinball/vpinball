#pragma once

#include "Graphics.h"
#include "Timer.h"

#include "DMDUtil/DMDUtil.h"

namespace VP {

class Window final
{
public:
   ~Window();

   static Window* Create(const string& szTitle, int x, int y, int w, int h);
   void WindowHit(const SDL_Point* pArea);
   void Render(DMDUtil::DMD* pDmd);
   void Show();
   void Hide();
   VP::Graphics* GetGraphics() { return m_pGraphics; }

private:
   Window(const string& szTitle, SDL_Window* pWindow, SDL_Renderer* pRenderer);
   void TimerElapsed(VP::Timer* pTimer);

   string m_szTitle;
   SDL_Window* m_pWindow;
   SDL_Renderer* m_pRenderer;
   VP::Graphics* m_pGraphics;
   SDL_Texture* m_pTexture;
   int m_x;
   int m_y;
   VP::Timer* m_pTimer;
};

}
