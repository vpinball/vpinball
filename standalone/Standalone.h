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
   PUPManager* GetPUPManager() { return m_pPUPManager; }

   void PreStartup();
   void PostStartup();
   void Render();
   void Shutdown();

private:
   Standalone();

   static Standalone* m_pInstance;
   PUPManager* m_pPUPManager;
   VP::WindowManager* m_pWindowManager;
};