#pragma once

#include "stdafx.h"

class LiveUI
{
public:
   LiveUI(RenderDevice* rd);
   ~LiveUI();
   void Update();
   void Render();

   void OpenMainUI() { m_inMainUI = true; }

private:
   void UpdateMainUI();
   void UpdateCameraModeUI();

   const RenderDevice* m_rd;
   bool m_inMainUI = false;
   int m_rotate = 0;
   U64 m_StartTime_usec;
};
