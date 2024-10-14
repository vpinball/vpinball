#include "core/stdafx.h"

#include "Window.h"
#include "WindowManager.h"

namespace VP {

Window::Window(const string& szTitle, int x, int y, int w, int h, int z, int rotation)
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
   m_rotation = rotation;
   m_visible = false;
   m_init = false;

   VP::WindowManager::GetInstance()->RegisterWindow(this);
}

bool Window::Init()
{
   UINT32 flags = SDL_WINDOW_UTILITY | SDL_WINDOW_BORDERLESS | SDL_WINDOW_HIDDEN | SDL_WINDOW_UTILITY | SDL_WINDOW_ALWAYS_ON_TOP;

   if (g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true))
      flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;

   SDL_PropertiesID props = SDL_CreateProperties();
   SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, m_szTitle.c_str());
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, m_x);
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, m_y);
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, m_w);
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, m_h);
   SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);
   m_pWindow = SDL_CreateWindowWithProperties(props);
   SDL_DestroyProperties(props);
   if (m_pWindow) {
      SDL_SetWindowPosition(m_pWindow, m_x, m_y);

      m_pRenderer = SDL_CreateRenderer(m_pWindow, NULL);
      if (m_pRenderer) {
         SDL_SetRenderLogicalPresentation(m_pRenderer, m_w, m_h, SDL_LOGICAL_PRESENTATION_STRETCH);

         m_id = SDL_GetWindowID(m_pWindow);

         const char* pRendererName = SDL_GetRendererName(m_pRenderer);

         if (m_rotation < 0 || m_rotation > 3)
            m_rotation = 0;

         PLOGI.printf("Window initialized: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d, rotation=%d, visible=%d, renderer=%s",
            m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z, m_rotation, m_visible, pRendererName ? pRendererName : "Unavailable");

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

      PLOGI.printf("Window updated: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d, rotation=%d, visible=%d", 
         m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z, m_rotation, m_visible);
   }
}

void Window::Hide()
{
   if (!m_visible)
      return;

   m_visible = false;

   if (m_pWindow) {
      SDL_HideWindow(m_pWindow);

      PLOGI.printf("Window updated: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d, rotation=%d, visible=%d", 
         m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z, m_rotation, m_visible);
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

      PLOGI.printf("Window moved: title=%s, id=%d, size=%dx%d, pos=%d,%d, z=%d, rotation=%d", 
         m_szTitle.c_str(), m_id, m_w, m_h, m_x, m_y, m_z, m_rotation);
   }
}

void Window::OnRender()
{
   if (m_init && m_visible)
      Render();
}

}
