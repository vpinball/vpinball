#include "stdafx.h"

#include "Window.h"

namespace VP {

static SDL_HitTestResult WindowHitTest(SDL_Window* pWindow, const SDL_Point* pArea, void* pData)
{
   static_cast<Window*>(pData)->WindowHit(pArea);
   return SDL_HITTEST_DRAGGABLE;
}

Window* Window::Create(const string& szTitle, int x, int y, int w, int h)
{
   Window* pWindow = nullptr;

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   UINT32 flags = SDL_WINDOW_SKIP_TASKBAR | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY | SDL_WINDOW_ALWAYS_ON_TOP;

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true))
      flags |= SDL_WINDOW_ALLOW_HIGHDPI;

   SDL_Window* pSDLWindow = SDL_CreateWindow(szTitle.c_str(), x, y, w, h, flags);

   if (pSDLWindow) {
      SDL_Renderer* pSDLRenderer = SDL_CreateRenderer(pSDLWindow, -1, SDL_RENDERER_ACCELERATED);

      if (pSDLRenderer)
         pWindow = new Window(szTitle, pSDLWindow, pSDLRenderer);
      else
         SDL_DestroyWindow(pSDLWindow);
   }

   if (!pWindow) {
      PLOGE.printf("Unable to create window: title=%s", szTitle.c_str());
   }

   SDL_GL_MakeCurrent(g_pplayer->m_sdl_playfieldHwnd, g_pplayer->m_renderer->m_pd3dPrimaryDevice->m_sdl_context);
   SDL_RaiseWindow(g_pplayer->m_sdl_playfieldHwnd);

   return pWindow;
}

Window::Window(const string& szTitle, SDL_Window* pWindow, SDL_Renderer* pRenderer)
{
   m_szTitle = szTitle;
   m_pWindow = pWindow;
   m_pRenderer = pRenderer;
   m_pGraphics = new VP::Graphics(m_pRenderer);
   m_pTexture = nullptr;

   int w2, h2;
   SDL_GetWindowPosition(m_pWindow, &m_x, &m_y);
   SDL_GetWindowSize(m_pWindow, &w2, &h2);
   PLOGI.printf("Window created: title=%s, size=%dx%d, pos=%d,%d", m_szTitle.c_str(), w2, h2, m_x, m_y);

   SDL_RenderSetLogicalSize(m_pRenderer, w2, h2);

   m_pTimer = new VP::Timer();
   m_pTimer->SetInterval(200);
   m_pTimer->SetElapsedListener(std::bind(&Window::TimerElapsed, this, std::placeholders::_1));

   SDL_SetWindowHitTest(m_pWindow, WindowHitTest, this);
}

Window::~Window()
{
   delete m_pTimer;

   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);

   if (m_pGraphics)
      delete m_pGraphics;

   if (m_pRenderer)
      SDL_DestroyRenderer(m_pRenderer);

   if (m_pWindow)
      SDL_DestroyWindow(m_pWindow);
}

void Window::Render(DMDUtil::DMD* pDmd)
{
   SDL_RenderClear(m_pRenderer);

   if (pDmd) {
      if (!m_pTexture)
         m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, pDmd->GetWidth(), pDmd->GetHeight());

      if (m_pTexture) {
         SDL_UpdateTexture(m_pTexture, NULL, pDmd->GetRGB32Data(), pDmd->GetWidth() * sizeof(UINT32));
         SDL_RenderCopy(m_pRenderer, m_pTexture, NULL, NULL);
      }
   }

   SDL_RenderPresent(m_pRenderer);
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

      PLOGI.printf("Window moved: title=%s, pos=%d,%d", m_szTitle.c_str(), m_x, m_y);
   }
}

void Window::Show()
{
   SDL_ShowWindow(m_pWindow);
}

void Window::Hide()
{
   SDL_HideWindow(m_pWindow);
}

}
