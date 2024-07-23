#pragma once

#include <mutex>

namespace VP {

class Window;

class WindowManager final
{
public:
   enum RenderMode {
      Default = 0,
      Threaded = 1,
   };

   ~WindowManager();

   static WindowManager* GetInstance();

   void RegisterWindow(Window* pWindow);
   void UnregisterWindow(Window* pWindow);
   void Start();
   void ProcessEvent(const SDL_Event* event);
   void ProcessUpdates();
   void Render();

   RenderMode m_renderMode;

private:
   WindowManager();

   void ThreadedRender();

   static WindowManager* m_pInstance;
   vector<Window*> m_windows;
   std::mutex m_mutex;
   bool m_startup;
   bool m_updateLock;
   Uint64 m_lastEventTime;
   bool m_running;
   std::thread* m_pThread;
   Uint64 m_lastRenderTime;
};

}
