#pragma once

#include "../common/Window.h"

class DMDUtil;

class DMDWindow
{
public:
   static DMDWindow* GetInstance();

   void Init();
   void Shutdown();
   void Render();

   void Enable(bool enable);

private:
   DMDWindow();
   ~DMDWindow();

   VP::Window* m_pWindow;

   DMDUtil* m_pDMDUtil;

   static DMDWindow* m_pInstance;
};
