#pragma once

#include "inc/common/WindowManager.h"

class Standalone final
{
public:
   ~Standalone();

   static Standalone* GetInstance();

   void Start();
   void StartupDone();
   void ProcessEvent(const SDL_Event* pEvent);
   void ProcessUpdates();
   void Render();

private:
   Standalone();

   static Standalone* m_pInstance;
   VP::WindowManager* m_pWindowManager;
};