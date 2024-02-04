#include "stdafx.h"

#include "VPinMAMEWindow.h"

VPinMAMEWindow::VPinMAMEWindow(const std::string& szTitle, int x, int y, int w, int h, int z, bool highDpi)
    : VP::Window(szTitle, x, y, w, h, z, highDpi)
{
   m_pDMD = nullptr;
   m_pVirtualDMD = nullptr;
   m_pitch = 0;
   m_pTexture = NULL;
}

VPinMAMEWindow::~VPinMAMEWindow()
{
   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);
}

VPinMAMEWindow* VPinMAMEWindow::Create()
{
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindow"s, true)) {
      PLOGI.printf("PinMAME window disabled");
      return nullptr;
   }

   VPinMAMEWindow* pWindow = new VPinMAMEWindow("PinMAME", 
      pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowX"s, PINMAME_SETTINGS_WINDOW_X),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowY"s, PINMAME_SETTINGS_WINDOW_Y),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowWidth"s, PINMAME_SETTINGS_WINDOW_WIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PinMAMEWindowHeight"s, PINMAME_SETTINGS_WINDOW_HEIGHT),
      PINMAME_ZORDER,
      pSettings->LoadValueWithDefault(Settings::Standalone, "HighDPI"s, true));

   return pWindow;
}

void VPinMAMEWindow::SetDMD(DMDUtil::DMD* pDMD)
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
}

void VPinMAMEWindow::Render()
{
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