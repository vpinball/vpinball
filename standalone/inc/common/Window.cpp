#include "stdafx.h"

#include "Window.h"

namespace VP {

Window::Window(const string& szTitle, int x, int y, int w, int h, int z)
{
   m_pWindow = nullptr;
   m_id = 0;
   m_pRenderer = nullptr;
   m_szTitle = szTitle;
   m_x = x;
   m_y = y;
   m_h = h;
   m_w = w;
   m_z = z;
   m_visible = false;
   m_init = false;

   VP::WindowManager::GetInstance()->RegisterWindow(this);
}

bool Window::Init()
{
   UINT32 flags = SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY | SDL_WINDOW_ALWAYS_ON_TOP;

   if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true))
      flags |= SDL_WINDOW_ALLOW_HIGHDPI;

   m_pWindow = SDL_CreateWindow(m_szTitle.c_str(), m_x, m_y, m_w, m_h, flags);
   if (m_pWindow) {
      m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);

      if (m_pRenderer) {
         m_id = SDL_GetWindowID(m_pWindow);

         PLOGI.printf("Window initialized: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d, visible=%d", m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z, m_visible);

         if (m_visible)
            SDL_ShowWindow(m_pWindow);

         m_init = true;

         return true;
      }
      else {
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
      }
   }

   PLOGE.printf("Failed to initialize window: title=%s", m_szTitle.c_str());

   return false;
}

Window::~Window()
{
   VP::WindowManager::GetInstance()->UnregisterWindow(this);

   if (m_pRenderer)
      SDL_DestroyRenderer(m_pRenderer);

   if (m_pWindow) {
      SDL_DestroyWindow(m_pWindow);
   }
}

void Window::Show()
{
   if (m_visible)
      return;

   m_visible = true;

   if (m_init) {
      SDL_ShowWindow(m_pWindow);

      PLOGI.printf("Window updated: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d, visible=%d", m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z, m_visible);
   }
}

void Window::Hide()
{
   if (!m_visible)
      return;

   m_visible = false;

   if (m_pWindow) {
      SDL_HideWindow(m_pWindow);

      PLOGI.printf("Window updated: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d, visible=%d", m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z, m_visible);
   }
}

void Window::OnUpdate()
{
   if (!m_init || !m_visible)
      return;

   SDL_RaiseWindow(m_pWindow);

   int x, y;
   SDL_GetWindowPosition(m_pWindow, &x, &y);

   if (x != m_x || y != m_y) {
      m_x = x;
      m_y = y;

      PLOGI.printf("Window moved: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d", m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z);
   }
}

void Window::OnRender()
{
   if (m_init && m_visible) {
      Uint64 now = SDL_GetTicks64();
      Uint64 timeSinceLastRender = now - m_lastRenderTime;

      if(timeSinceLastRender < m_frameDuration)
         return;

       Render();

       m_lastRenderTime = now;
   }
}

}
