#pragma once

#include "Window.h"

namespace VP {

class WindowManager final
{
public:
   static WindowManager* GetInstance();

   void RegisterWindow(Window* pWindow);
   void UnregisterWindow(Window* pWindow);
   void Startup();
   void ProcessEvent(const SDL_Event* event);
   void ProcessUpdates();
   void Render();

private:
   WindowManager();

   static WindowManager* m_pInstance;
   vector<Window*> m_windows;
   bool m_init;
   bool m_updateLock;
   Uint64 m_lastEventTime;
};

}
