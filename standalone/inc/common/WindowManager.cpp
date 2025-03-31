#include "core/stdafx.h"

#include "WindowManager.h"
#include "Window.h"

namespace VP {

WindowManager* WindowManager::m_pInstance = NULL;

WindowManager* WindowManager::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new WindowManager();

   return m_pInstance;
}

WindowManager::WindowManager()
{
   m_startup = false;
   m_windows.clear();
}

WindowManager::~WindowManager()
{
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

   if (m_startup)
      pWindow->Init();
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
      for (Window* pWindow : m_windows)
         pWindow->Init();
   }

   m_startup = true;
}

void WindowManager::Render()
{
   if (!m_startup || !g_pplayer)
      return;

   {
      std::lock_guard<std::mutex> guard(m_mutex);
      for (Window* pWindow: m_windows) {
         if (pWindow->CanRender())
            pWindow->Render();
      }
   }
}

}
