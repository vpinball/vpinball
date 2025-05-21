#pragma once

class PUPManager;

class Standalone final
{
public:
   ~Standalone();

   static Standalone* GetInstance();

   PUPManager* GetPUPManager() { return m_pPUPManager; }
   void PreStartup();
   void PostStartup();
   void Shutdown();

private:
   Standalone();

   static Standalone* m_pInstance;
   PUPManager* m_pPUPManager;
};