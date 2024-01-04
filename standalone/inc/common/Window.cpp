#include "stdafx.h"

#include "Window.h"

namespace VP {

static SDL_HitTestResult WindowHitTest(SDL_Window* pWindow, const SDL_Point* pArea, void* pData)
{
   static_cast<Window*>(pData)->WindowHit(pArea);
   return SDL_HITTEST_DRAGGABLE;
}

Window::Window(const string& name, int x, int y, int w, int h, int frameSkip)
{
   m_pRenderer = nullptr;
   m_pGraphics = nullptr;

   m_update = false;

   m_szName = "vpx_" + name;
   UINT32 flags = SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY;

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true))
      flags |= SDL_WINDOW_ALLOW_HIGHDPI;

   m_pWindow = SDL_CreateWindow(m_szName.c_str(), x, y, w, h, flags);
   if (m_pWindow) {
      int w2, h2;
      SDL_GetWindowPosition(m_pWindow, &m_x, &m_y);
      SDL_GetWindowSize(m_pWindow, &w2, &h2);
      PLOGI.printf("%s window created: %d, %d (%dx%d)", m_szName.c_str(), m_x, m_y, w2, h2);

      m_pTimer = new VP::Timer();
      m_pTimer->SetInterval(200);
      m_pTimer->SetElapsedListener(std::bind(&Window::TimerElapsed, this, std::placeholders::_1));

      SDL_SetWindowHitTest(m_pWindow, WindowHitTest, this);

      SDL_RaiseWindow(g_pplayer->m_sdl_playfieldHwnd);

      m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
      if (m_pRenderer) {
         SDL_RenderSetLogicalSize(m_pRenderer, w2, h2);
         m_pGraphics = new VP::Graphics(m_pRenderer);
      }
   }

   m_frameSkip = frameSkip;
   m_currentFrame = 0;
   m_enabled = false;
}

Window::~Window()
{
   delete m_pTimer;

   delete m_pGraphics;

   if (m_pRenderer)
      SDL_DestroyRenderer(m_pRenderer);

   if (m_pWindow)
      SDL_DestroyWindow(m_pWindow);
}

void Window::WindowHit(const SDL_Point* pArea)
{
   if (m_pTimer) {
      m_pTimer->Stop();
      m_pTimer->Start();
   }
}

void Window::TimerElapsed(VP::Timer* pTimer)
{
   m_pTimer->Stop();

   int x, y;
   SDL_GetWindowPosition(m_pWindow, &x, &y);

   if (x != m_x || y != m_y) {
      m_x = x;
      m_y = y;

      PLOGI.printf("%s window moved to: %d, %d", m_szName.c_str(), m_x, m_y);
   }
}

void Window::Enable(bool enable)
{
   if (m_pWindow && m_pRenderer) {
      m_enabled = enable;
      m_update = true;
   }
}

bool Window::ShouldRender()
{
   if (!m_pWindow)
       return false;

   if (m_update) {
      if (m_enabled)
         SDL_ShowWindow(m_pWindow);
      else
         SDL_HideWindow(m_pWindow);

      SDL_RaiseWindow(g_pplayer->m_sdl_playfieldHwnd);
      m_update = false;
   }

   if (!m_enabled)
      return false;

   bool shouldProcess = !m_currentFrame;

   if (m_frameSkip > 0)
      m_currentFrame = (m_currentFrame + 1) % m_frameSkip;

   return shouldProcess;
}

}
