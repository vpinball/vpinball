#include "stdafx.h"

#include "FlexDMDWindow.h"

FlexDMDWindow::FlexDMDWindow(const std::string& szTitle, VP::Window::RenderMode renderMode, int x, int y, int w, int h, int z, bool highDpi)
    : VP::Window(szTitle, renderMode, x, y, w, h, z, highDpi)
{
   m_pDMD = nullptr;
   m_pVirtualDMD = nullptr;
   m_pitch = 0;
   m_pTexture = NULL;
   m_startTime = 0;
}

FlexDMDWindow::~FlexDMDWindow()
{
   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);
}

FlexDMDWindow* FlexDMDWindow::Create()
{
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindow"s, true)) {
      PLOGI.printf("FlexDMD window disabled");
      return nullptr;
   }

   FlexDMDWindow* pWindow = new FlexDMDWindow("FlexDMD", 
      (VP::Window::RenderMode)pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowRenderMode"s, VP::Window::RenderMode_Default),
      pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowX"s, FLEXDMD_SETTINGS_WINDOW_X),
      pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowY"s, FLEXDMD_SETTINGS_WINDOW_Y),
      pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowWidth"s, FLEXDMD_SETTINGS_WINDOW_WIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "FlexDMDWindowHeight"s, FLEXDMD_SETTINGS_WINDOW_HEIGHT),
      FLEXDMD_ZORDER,
      pSettings->LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true));

   return pWindow;
}

void FlexDMDWindow::SetDMD(DMDUtil::DMD* pDMD)
{
   if (m_pDMD) {
      if (m_pVirtualDMD) {
         m_pDMD->DestroyVirtualDMD(m_pVirtualDMD);
         m_pVirtualDMD = nullptr;
      }
      if (m_pTexture) {
         SDL_DestroyTexture(m_pTexture);
         m_pTexture = NULL;
      }
   }

   if (pDMD) {
      m_pVirtualDMD = pDMD->CreateVirtualDMD();
      m_pitch = m_pVirtualDMD->GetPitch();
   }

   m_pDMD = pDMD;
   m_startTime = SDL_GetTicks64();
}

void FlexDMDWindow::Render()
{
   if (SDL_GetTicks64() - m_startTime < 3000)
      return;

   const UINT8* pRGB24Data = m_pVirtualDMD ? m_pVirtualDMD->GetRGB24Data() : nullptr;

   if (pRGB24Data) {
      if (!m_pTexture) {
         m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, m_pVirtualDMD->GetWidth(), m_pVirtualDMD->GetHeight());
         if (!m_pTexture)
            return;
      }
      if (!SDL_UpdateTexture(m_pTexture, NULL, pRGB24Data, m_pitch)) {
         SDL_RenderCopy(m_pRenderer, m_pTexture, NULL, NULL);
         SDL_RenderPresent(m_pRenderer);
      }
   }
}