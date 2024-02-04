#include "stdafx.h"

#include "Window.h"

namespace VP {

Window::Window(const string& szTitle, RenderMode renderMode, int x, int y, int w, int h, int z, bool highDpi)
{
   m_pWindow = nullptr;
   m_pRenderer = nullptr;
   m_visible = false;
   m_id = 0;
   m_running = false;
   m_pThread = nullptr;

   UINT32 flags = SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY | SDL_WINDOW_ALWAYS_ON_TOP;

   if (highDpi)
      flags |= SDL_WINDOW_ALLOW_HIGHDPI;

   SDL_Window* pWindow = SDL_CreateWindow(szTitle.c_str(), x, y, w, h, flags);

   if (pWindow) {
      SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

      if (pRenderer) {
         m_szTitle = szTitle;
         m_pWindow = pWindow;
         m_pRenderer = pRenderer;
         m_id = SDL_GetWindowID(pWindow);
         m_renderMode = renderMode;
         SDL_GetWindowPosition(m_pWindow, &m_x, &m_y);
         SDL_GetWindowSize(m_pWindow, &m_w, &m_h);
         m_z = z;
         PLOGI.printf("Window created: title=%s, size=%dx%d, pos=%d,%d, z=%d", szTitle.c_str(), m_w, m_h, m_x, m_y, m_z);

         SDL_RenderSetLogicalSize(m_pRenderer, m_w, m_h);

         VP::WindowManager::GetInstance()->RegisterWindow(this);

         if (renderMode == RenderMode_Default)
            Run();
      }
      else
         SDL_DestroyWindow(pWindow);
   }

   if (!m_pWindow) {
      PLOGE.printf("Unable to create window: title=%s, size=%dx%d, pos=%d,%d, z=%d", szTitle.c_str(), w, h, x, y, z);
   }
}

Window::~Window()
{
   if (m_pThread) {
      m_running = false;
      m_pThread->join();
      delete m_pThread;
   }

   if (m_pRenderer)
      SDL_DestroyRenderer(m_pRenderer);

   if (m_pWindow) {
      VP::WindowManager::GetInstance()->UnregisterWindow(this);

      SDL_DestroyWindow(m_pWindow);
   }
}

void Window::Run()
{
   if (m_running)
      return;

   m_running = true;

   m_pThread = new std::thread([this]() {
      const Uint64 targetFrameTime = 1000 / 60;

      while (m_running) {
         Uint64 frameStartTime = SDL_GetTicks64();

         Render();

         Uint64 frameEndTime = SDL_GetTicks64();
         Uint64 frameDuration = frameEndTime - frameStartTime;

         if (frameDuration < targetFrameTime)
            SDL_Delay(targetFrameTime - frameDuration);
      }
   });
}

void Window::Show()
{
   m_visible = true;

   SDL_ShowWindow(m_pWindow);
}

void Window::Hide()
{
   m_visible = false;

   SDL_HideWindow(m_pWindow);
}

void Window::HandleUpdate()
{
   SDL_RaiseWindow(m_pWindow);

   int x, y;
   SDL_GetWindowPosition(m_pWindow, &x, &y);

   if (x != m_x || y != m_y) {
      m_x = x;
      m_y = y;

      PLOGI.printf("Window moved: title=%s, size=%dx%d, pos=%d,%d, z=%d", m_szTitle.c_str(), m_w, m_h, m_x, m_y, m_z);
   }
}

}
