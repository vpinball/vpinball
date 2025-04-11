#pragma once

#include <mutex>

namespace VP {

class Window;

class WindowManager final
{
public:
   ~WindowManager();

   static WindowManager* GetInstance();

   void RegisterWindow(Window* pWindow);
   void UnregisterWindow(Window* pWindow);
   vector<VPX::Window*> GetVPXWindows() const;
   void Start();
   void Render();
   void Stop();


private:
   WindowManager();

   static WindowManager* m_pInstance;
   vector<Window*> m_windows;
   std::mutex m_mutex;
   bool m_running;
};

}
