#pragma once

namespace VP {

class Window;

typedef struct {
   Window* m_pWindow;
   int z;
} WindowData;

class WindowManager final
{
public:
   static WindowManager* GetInstance();

   void RegisterWindow(Window* pWindow, int z);
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
   Uint32 m_lastEventTime;
};

}
