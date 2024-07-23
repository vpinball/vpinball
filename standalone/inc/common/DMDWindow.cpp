#include "stdafx.h"

#include "DMDWindow.h"

namespace VP {

DMDWindow::DMDWindow(const string& szTitle, int x, int y, int w, int h, int z, int rotation)
    : VP::Window(szTitle, x, y, w, h, z, rotation)
{
   m_destRect = { 0.0f, 0.0f, (float)w, (float)h };
   m_angle = 0;
   m_pDMD = nullptr;
   m_pRGB24DMD = nullptr;
   m_pitch = 0;
   m_pTexture = NULL;
   m_attached = false;
}

DMDWindow::~DMDWindow()
{
   if (m_pDMD) {
      PLOGE.printf("Destructor called without first detaching DMD.");
   }
}

bool DMDWindow::Init()
{
   if (!VP::Window::Init())
      return false;

   int rotation = GetRotation();

   if (rotation == 0 || rotation == 2) {
      SDL_RenderSetLogicalSize(m_pRenderer, GetWidth(), GetHeight());
      m_angle = (rotation == 0) ? 0 : 180;
   }
   else if (rotation == 1 || rotation == 3) {
      SDL_RenderSetLogicalSize(m_pRenderer, GetHeight(), GetWidth());
      m_angle = (rotation == 1) ? 90 : 270;
      float xRotated = GetHeight() - m_destRect.y - (m_destRect.w + m_destRect.h) / 2.0f;
      float yRotated = m_destRect.x + (m_destRect.w - m_destRect.h) / 2.0f;
      m_destRect.x = xRotated;
      m_destRect.y = yRotated;
   }

   return true;
}

void DMDWindow::AttachDMD(DMDUtil::DMD* pDMD, int width, int height)
{
   if (m_pDMD) {
      PLOGE.printf("Unable to attach DMD: message=Detach existing DMD first.");
      return;
   }

   if (!pDMD) {
      PLOGE.printf("Unable to attach DMD: message=DMD is null.");
      return;
   }

   PLOGI.printf("Attaching DMD: width=%d, height=%d", width, height);

   m_pRGB24DMD = pDMD->CreateRGB24DMD(width, height);

   if (m_pRGB24DMD) {
      m_pitch = m_pRGB24DMD->GetPitch();
      m_pDMD = pDMD;
      m_attached = true;
   }
   else {
      PLOGE.printf("Failed to attach DMD: message=Failed to create RGB24DMD.");
   }
}

void DMDWindow::DetachDMD()
{
   if (!m_pDMD) {
      PLOGE.printf("Unable to detach DMD: message=No DMD attached.");
      return;
   }

   m_attached = false;

   if (m_pRGB24DMD) {
      PLOGI.printf("Detaching DMD");
      m_pDMD->DestroyRGB24DMD(m_pRGB24DMD);
      m_pRGB24DMD = nullptr;
   }

   if (m_pTexture) {
      SDL_DestroyTexture(m_pTexture);
      m_pTexture = NULL;
   }

   m_pDMD = nullptr;
}

void DMDWindow::Render()
{
   if (!m_attached)
      return;

   const UINT8* pRGB24Data = m_pRGB24DMD->GetData();
   if (pRGB24Data) {
      if (!m_pTexture) {
         m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, m_pRGB24DMD->GetWidth(), m_pRGB24DMD->GetHeight());
         if (!m_pTexture)
            return;
      }
      if (SDL_UpdateTexture(m_pTexture, NULL, pRGB24Data, m_pitch))
         return;
      SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
      SDL_RenderClear(m_pRenderer);
      SDL_RenderCopyExF(m_pRenderer, m_pTexture, NULL, &m_destRect, m_angle, NULL, SDL_FLIP_NONE);
      SDL_RenderPresent(m_pRenderer);
   }
}

}