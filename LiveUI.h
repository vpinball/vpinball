#pragma once

#include "stdafx.h"

class LiveUI
{
public:
   LiveUI(RenderDevice* rd);
   ~LiveUI();
   void Update();
   void Render();

   void OpenMainUI() { m_ShowUI = true; }

   bool HasKeyboardCapture() const;
   bool HasMouseCapture() const;

private:
   void UpdateMainUI();
   void UpdateCameraModeUI();

   const RenderDevice* m_rd;
   bool m_ShowUI = false;
   int m_rotate = 0;
   U64 m_StartTime_usec = 0; // Used for timed splash overlays
   bool m_disable_esc = false; // Option for keyboard shortcuts
};
