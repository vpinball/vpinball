#pragma once

class Standalone final
{
public:
   ~Standalone();

   static Standalone* GetInstance();

   void PreStartup();
   void PostStartup();
   void Shutdown();

private:
   Standalone();

   static Standalone* m_pInstance;
};