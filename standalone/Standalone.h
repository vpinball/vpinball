#pragma once

class Standalone final
{
public:
   ~Standalone();

   static Standalone* GetInstance();

   void Startup();

private:
   Standalone();

   static Standalone* m_pInstance;
};