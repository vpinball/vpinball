#pragma once

class PUPManager;

namespace VP {
   class WindowManager;
}

class Standalone final
{
public:
   ~Standalone();

   static Standalone* GetInstance();

   void PreStartup();
   void PostStartup();
   void ProcessEvent(const SDL_Event* pEvent);
   void ProcessUpdates();
   void Render();

private:
   Standalone();

   static Standalone* m_pInstance;
   PUPManager* m_pPUPManager;
   VP::WindowManager* m_pWindowManager;
};