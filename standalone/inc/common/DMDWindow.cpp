#include "core/stdafx.h"

#include "DMDWindow.h"

namespace VP {

DMDWindow::DMDWindow(const std::string& szTitle, int x, int y, int w, int h, int z)
    : VP::Window(szTitle, x, y, w, h, z)
{
   m_pDMD = nullptr;
   m_pVirtualDMD = nullptr;
   m_pitch = 0;
   m_pTexture = NULL;
}

DMDWindow::~DMDWindow()
{
   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);
}

void DMDWindow::SetDMD(DMDUtil::DMD* pDMD)
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

void DMDWindow::Render()
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

}