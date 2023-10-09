#pragma once

#include "../common/Window.h"

class B2SScreen;

class B2SWindows
{
public:
   static B2SWindows* GetInstance();

   void Init();
   void Shutdown();
   void Render();
   
   void EnableBackglass(bool enable);
   void EnableDMD(bool enable);

   void SetB2SScreen(B2SScreen* pB2SScreen) { m_pB2SScreen = pB2SScreen; }

private:
   B2SWindows();
   ~B2SWindows();

   VP::Window* m_pBackglassWindow;
   VP::Window* m_pDMDWindow;
   
   B2SScreen* m_pB2SScreen;

   static B2SWindows* m_pInstance;
};
