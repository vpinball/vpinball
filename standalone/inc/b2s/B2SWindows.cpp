#include "stdafx.h"

#include "B2SWindows.h"
#include "classes/B2SScreen.h"

B2SWindows* B2SWindows::m_pInstance = NULL;

B2SWindows* B2SWindows::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new B2SWindows();

   return m_pInstance;
}

B2SWindows::B2SWindows()
{
   m_pBackglassWindow = NULL;
   m_pDMDWindow = NULL;

   m_pB2SScreen = NULL;
}

B2SWindows::~B2SWindows()
{
   Shutdown();
}

void B2SWindows::Init()
{
   if (!g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2S"s, false))
      return;

   m_pBackglassWindow = new VP::Window("b2s_backglass",
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SBackglassX"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SBackglassY"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SBackglassWidth"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SBackglassHeight"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SBackglassFrameSkip"s, 0));

   m_pDMDWindow = new VP::Window("b2s_dmd",
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SDMDX"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SDMDY"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SDMDWidth"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SDMDHeight"s, 0),
      g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Standalone, "B2SDMDFrameSkip"s, 0));
}

void B2SWindows::Shutdown()
{
   if (m_pBackglassWindow) {
      delete m_pBackglassWindow;
      m_pBackglassWindow = NULL;
   }

   if (m_pDMDWindow) {
      delete m_pDMDWindow;
      m_pDMDWindow = NULL;
   }

   m_pB2SScreen = NULL;
}

void B2SWindows::EnableBackglass(bool enable)
{
   if (m_pBackglassWindow)
      m_pBackglassWindow->Enable(enable);
}

void B2SWindows::EnableDMD(bool enable)
{
   if (m_pDMDWindow)
      m_pDMDWindow->Enable(enable);
}

void B2SWindows::Render()
{
   if (!m_pB2SScreen)
      return;

   if (m_pBackglassWindow && m_pBackglassWindow->ShouldRender()) 
      m_pB2SScreen->RenderBackglass(m_pBackglassWindow->GetRenderer());

   if (m_pDMDWindow && m_pDMDWindow->ShouldRender()) 
      m_pB2SScreen->RenderDMD(m_pDMDWindow->GetRenderer());
}