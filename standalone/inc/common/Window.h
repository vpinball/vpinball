#pragma once

#include "Graphics.h"
#include "Timer.h"

namespace VP {

class Window
{
public:
   Window(const string& name, int x, int y, int w, int h, int frameSkip = 0);
   ~Window();

   void Enable(bool enable);
   bool ShouldRender();
   void WindowHit(const SDL_Point* pArea);

   VP::Graphics* GetGraphics() { return m_pGraphics; }

   void TimerElapsed(VP::Timer* pTimer);

private:
   string m_szName;
   SDL_Window* m_pWindow;
   SDL_Renderer* m_pRenderer;
   bool m_enabled;
   bool m_update;
   int m_frameSkip;
   int m_currentFrame;
   VP::Graphics* m_pGraphics;
   int m_x;
   int m_y;
   VP::Timer* m_pTimer;
};

}
