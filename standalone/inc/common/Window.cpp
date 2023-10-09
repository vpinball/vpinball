#include "stdafx.h"

#include "Window.h"

namespace VP {
 
Window::Window(const string& name, int x, int y, int w, int h, int frameSkip)
{
   m_pRenderer = NULL;

   string szName = "vpx_" + name;
   UINT32 flags = SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY;
#if defined(__APPLE__) && !TARGET_OS_TV
   if (g_pvp->m_settings.LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true))
      flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

   m_pWindow = SDL_CreateWindow(szName.c_str(), x, y, w, h, flags);
   if (m_pWindow) {
      int x, y, w, h;
      SDL_GetWindowPosition(m_pWindow, &x, &y);
      SDL_GetWindowSize(m_pWindow, &w, &h);
      PLOGI.printf("%s window created: %d, %d (%dx%d)", szName.c_str(), x, y, w, h);

      m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
   }

   m_frameSkip = frameSkip;
   m_currentFrame = 0;
   m_enabled = false;
}

Window::~Window()
{
   if (m_pRenderer)
      SDL_DestroyRenderer(m_pRenderer);

   if (m_pWindow)
      SDL_DestroyWindow(m_pWindow);
}

void Window::Enable(bool enable)
{
   if (m_pWindow && m_pRenderer) {
      m_enabled = enable;
      if (enable)
         SDL_ShowWindow(m_pWindow);
      else
         SDL_HideWindow(m_pWindow);
   }
}

bool Window::ShouldRender()
{
   if (m_pWindow && m_enabled) {
      bool shouldProcess = !m_currentFrame;

      if (m_frameSkip > 0)
         m_currentFrame = (m_currentFrame + 1) % m_frameSkip;

      return shouldProcess;
   }

   return false;
}

}