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
   m_updateLock = false;
   m_lastEventTime = 0;
   
   m_windows.clear();
}

void WindowManager::RegisterWindow(Window* pWindow, int z)
{
   if (!pWindow)
      return;

   WindowData* pWindowData = new WindowData();
   pWindowData->m_pWindow = pWindow;
   pWindowData->z = z;
   m_windows.push_back(pWindowData);

   std::sort(m_windows.begin(), m_windows.end(), [](const WindowData* pWindowData1, const WindowData* pWindowData2) {
      return pWindowData1->z < pWindowData2->z;
   });

   SDL_SetWindowHitTest(SDL_GetWindowFromID(pWindow->GetId()), WindowHitTest, NULL);

   Update();
   SDL_RaiseWindow(g_pplayer->m_sdl_playfieldHwnd);
}

void WindowManager::UnregisterWindow(Window* pWindow)
{
   auto newEnd = std::remove_if(m_windows.begin(), m_windows.end(),
      [pWindow](const WindowData* pWindowData) {
         return pWindowData->m_pWindow == pWindow;
      });

   m_windows.erase(newEnd, m_windows.end());
}

void WindowManager::ProcessEvent(const SDL_Event* event)
{
   if (m_updateLock)
      return;

   if (event->type == SDL_WINDOWEVENT &&
      (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED || event->window.event == SDL_WINDOWEVENT_MOVED))
      m_lastEventTime = SDL_GetTicks();
}

void WindowManager::HandleUpdates()
{
   if (m_lastEventTime == 0)
      return;

   Uint32 now = SDL_GetTicks();
   if (now - m_lastEventTime > 250) {
      if (now - m_lastEventTime > 500) {
         m_updateLock = false;
         m_lastEventTime = 0;
      }
      else if (!m_updateLock) {
         m_updateLock = true;
         Update();
      }
   }
}

void WindowManager::Update()
{
   for (WindowData* pWindowData : m_windows)
      pWindowData->m_pWindow->HandleUpdate();
}

void WindowManager::Render()
{
   for (WindowData* pWindowData : m_windows)
      pWindowData->m_pWindow->Render();
}

}