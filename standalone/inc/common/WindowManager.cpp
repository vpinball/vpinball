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
#ifdef __APPLE__
   m_renderMode = (RenderMode)g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "WindowRenderMode"s, (int)RenderMode::Default);
#else
   m_renderMode = (RenderMode)g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "WindowRenderMode"s, (int)RenderMode::Threaded);
#endif

   m_startup = false;
   m_updateLock = false;
   m_lastEventTime = 0;
   m_lastRenderTime = 0;
   m_running = false;
   m_pThread = nullptr;
   m_windows.clear();
}

WindowManager::~WindowManager()
{
   m_running = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
   }

   for (Window* pWindow : m_windows)
      delete pWindow;
}

void WindowManager::RegisterWindow(Window* pWindow)
{
   if (!pWindow)
      return;

   PLOGI.printf("Register window: %s", pWindow->GetTitle().c_str());

   {
      std::lock_guard<std::mutex> guard(m_mutex);
      m_windows.push_back(pWindow);
      std::sort(m_windows.begin(), m_windows.end(), [](Window* pWindow1, Window* pWindow2) {
         return pWindow1->GetZ() < pWindow2->GetZ();
      });
   }

   if (m_startup) {
      if (pWindow->Init()) {
         SDL_Window* pSDLWindow = SDL_GetWindowFromID(pWindow->GetId());
         SDL_SetWindowHitTest(pSDLWindow, WindowHitTest, NULL);
      }

      SDL_GL_MakeCurrent(g_pplayer->m_sdl_playfieldHwnd, g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_sdl_context);
   }
}

void WindowManager::UnregisterWindow(Window* pWindow)
{
   if (!pWindow)
      return;

   PLOGI.printf("Unregister window: %s", pWindow->GetTitle().c_str());

   {
      std::lock_guard<std::mutex> guard(m_mutex);
      auto newEnd = std::remove_if(m_windows.begin(), m_windows.end(),
         [pWindow](Window* pCurrentWindow) {
            return pCurrentWindow == pWindow;
      });
      m_windows.erase(newEnd, m_windows.end());
   }
}

void WindowManager::Start()
{
   PLOGI.printf("Window manager start");

   {
      std::lock_guard<std::mutex> guard(m_mutex);
      for (Window* pWindow : m_windows) {
         if (pWindow->Init()) {
            SDL_Window* pSDLWindow = SDL_GetWindowFromID(pWindow->GetId());
            SDL_SetWindowHitTest(pSDLWindow, WindowHitTest, NULL);
         }
      }
   }

   SDL_GL_MakeCurrent(g_pplayer->m_sdl_playfieldHwnd, g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_sdl_context);

   if (m_renderMode == RenderMode::Threaded)
      ThreadedRender();

   m_startup = true;
}

void WindowManager::ProcessEvent(const SDL_Event* event)
{
   if (!m_startup || m_updateLock)
      return;

   if (event->type == SDL_WINDOWEVENT &&
      (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED || event->window.event == SDL_WINDOWEVENT_MOVED))
      m_lastEventTime = SDL_GetTicks64();
}

void WindowManager::ProcessUpdates()
{
   if (!m_startup || m_lastEventTime == 0)
      return;

   Uint64 now = SDL_GetTicks64();
   if (now - m_lastEventTime > 250) {
      if (now - m_lastEventTime > 500) {
         m_updateLock = false;
         m_lastEventTime = 0;
      }
      else if (!m_updateLock) {
         m_updateLock = true;

         {
            std::lock_guard<std::mutex> guard(m_mutex);
            for (Window* pWindow: m_windows)
               pWindow->OnUpdate();
         }
      }
   }
}

void WindowManager::ThreadedRender()
{
   if (m_running)
      return;

   m_running = true;

   PLOGI.printf("Starting render thread");

   m_pThread = new std::thread([this]() {
      while (m_running) {
         Uint64 startTime = SDL_GetTicks64();

         {
            std::lock_guard<std::mutex> guard(m_mutex);
            for (Window* pWindow: m_windows)
               pWindow->OnRender();
         }

         double renderingDuration = SDL_GetTicks64() - startTime;

         int sleepMs = (1000 / 60) - (int)renderingDuration;

         if (sleepMs > 1)
            SDL_Delay(sleepMs);
      }

      PLOGI.printf("Render thread finished");
   });
}

void WindowManager::Render()
{
   if (!m_startup || !g_pplayer)
      return;

   Uint64 startTime = SDL_GetTicks64();

   if (startTime - m_lastRenderTime < (1000 / 60))
      return;

   {
      std::lock_guard<std::mutex> guard(m_mutex);
      for (Window* pWindow: m_windows)
         pWindow->OnRender();
   }

   SDL_GL_MakeCurrent(g_pplayer->m_sdl_playfieldHwnd, g_pplayer->m_pin3d.m_pd3dPrimaryDevice->m_sdl_context);

   m_lastRenderTime = startTime;
}

}