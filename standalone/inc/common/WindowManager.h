#pragma once

#include "Window.h"

namespace VP {

typedef struct {
   Window* m_pWindow;
   Window::RenderMode renderMode;
   int z;
} WindowData;

class WindowManager final
{
public:
   static WindowManager* GetInstance();

   void RegisterWindow(Window* pWindow);
   void UnregisterWindow(Window* pWindow);

   void ProcessEvent(const SDL_Event* event);
   void HandleUpdates();
   void Update();

   void Render();

private:
   WindowManager();

   vector<WindowData*> m_windows;
   static WindowManager* m_pInstance;

   bool m_updateLock;
   Uint64 m_lastEventTime;
};

}
