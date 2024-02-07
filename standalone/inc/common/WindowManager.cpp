#include "stdafx.h"

#include "WindowManager.h"
#include "Window.h"

namespace VP {

static SDL_HitTestResult WindowHitTest(SDL_Window* pWindow, const SDL_Point* pArea, void* pData)
{
   int width, height;
   SDL_GetWindowSize(pWindow, &width, &height);

   if (pArea->x >= 0 && pArea->x < width && pArea->y >= 0 && pArea->y < height)
      return SDL_HITTEST_DRAGGABLE;

   return SDL_HITTEST_NORMAL;
}

WindowManager* WindowManager::m_pInstance = NULL;

WindowManager* WindowManager::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new WindowManager();

   return m_pInstance;
}

WindowManager::WindowManager()
{
   m_init = false;
   m_updateLock = false;
   m_lastEventTime = 0;
   
   m_windows.clear();
}

void WindowManager::RegisterWindow(Window* pWindow)
{
   if (!pWindow)
      return;

   m_windows.push_back(pWindow);

   std::sort(m_windows.begin(), m_windows.end(), [](Window* pWindow1, Window* pWindow2) {
      return pWindow1->GetZ() < pWindow2->GetZ();
   });
}

void WindowManager::UnregisterWindow(Window* pWindow)
{
   auto newEnd = std::remove_if(m_windows.begin(), m_windows.end(),
      [pWindow](Window* pCurrentWindow) {
         return pCurrentWindow == pWindow;
   });

   m_windows.erase(newEnd, m_windows.end());
}

void WindowManager::Startup()
{
   for (Window* pWindow : m_windows) {
      if (pWindow->Init()) {
         SDL_Window* pSDLWindow = SDL_GetWindowFromID(pWindow->GetId());
         SDL_SetWindowHitTest(pSDLWindow, WindowHitTest, NULL);
      }
   }

   SDL_GL_MakeCurrent(g_pplayer->m_sdl_playfieldHwnd, g_pplayer->m_renderer->m_pd3dPrimaryDevice->m_sdl_context);

   m_init = true;
}

void WindowManager::ProcessEvent(const SDL_Event* event)
{
   if (!m_init || m_updateLock)
      return;

   if (event->type == SDL_WINDOWEVENT &&
      (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED || event->window.event == SDL_WINDOWEVENT_MOVED))
      m_lastEventTime = SDL_GetTicks64();
}

void WindowManager::ProcessUpdates()
{
   if (!m_init || m_lastEventTime == 0)
      return;

   Uint64 now = SDL_GetTicks64();
   if (now - m_lastEventTime > 250) {
      if (now - m_lastEventTime > 500) {
         m_updateLock = false;
         m_lastEventTime = 0;
      }
      else if (!m_updateLock) {
         m_updateLock = true;

         for (Window* pWindow : m_windows)
            pWindow->OnUpdate();
      }
   }
}

void WindowManager::Render()
{
   if (!m_init)
      return;

   for (Window* pWindow: m_windows)
      pWindow->OnRender();

   SDL_GL_MakeCurrent(g_pplayer->m_sdl_playfieldHwnd, g_pplayer->m_renderer->m_pd3dPrimaryDevice->m_sdl_context);
}

}
