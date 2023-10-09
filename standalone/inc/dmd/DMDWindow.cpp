#include "stdafx.h"

#include "DMDWindow.h"
#include "DMDUtil.h"

DMDWindow* DMDWindow::m_pInstance = NULL;

DMDWindow* DMDWindow::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new DMDWindow();

   return m_pInstance;
}

DMDWindow::DMDWindow()
{
   m_pWindow = NULL;

   m_pDMDUtil = DMDUtil::GetInstance();
}

DMDWindow::~DMDWindow()
{
   Shutdown();
}

void DMDWindow::Init()
{
   if (!g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DMD"s, false))
      return;

   m_pWindow = new VP::Window("dmd",
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DMDX"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DMDY"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DMDWidth"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DMDHeight"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "DMDFrameSkip"s, 0));
}

void DMDWindow::Shutdown()
{
   if (m_pWindow) {
      delete m_pWindow;
      m_pWindow = NULL;
   }
}

void DMDWindow::Enable(bool enable)
{
   if (m_pWindow)
      m_pWindow->Enable(enable);
}

void DMDWindow::Render()
{
   if (m_pWindow && m_pWindow->ShouldRender()) 
      m_pDMDUtil->Render(m_pWindow->GetRenderer());
}