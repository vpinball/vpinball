#include "stdafx.h"

#include "FormWindow.h"
#include "Form.h"

FormWindow::FormWindow(Form* pForm, const string& szTitle, int x, int y, int w, int h, int z, int rotation)
    : VP::Window(szTitle, x, y, w, h, z, rotation)
{
   m_destRect = { 0.0f, 0.0f, (float)w, (float)h };
   m_angle = 0;
   m_pForm = pForm;
   m_pGraphics = nullptr;
   m_pTexture = nullptr;
}

bool FormWindow::Init()
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

   m_pGraphics = new VP::RendererGraphics(m_pRenderer);
   m_pForm->SetGraphics(m_pGraphics);

   return true;
}

FormWindow::~FormWindow()
{
   delete m_pGraphics;

   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);
}

void FormWindow::Render()
{
   if (!m_pTexture) {
      m_pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, GetWidth(), GetHeight());
      if (!m_pTexture)
         return;
   }

   SDL_SetRenderTarget(m_pRenderer, m_pTexture);
   bool update = m_pForm->Render();
   SDL_SetRenderTarget(m_pRenderer, NULL);

   if (update) {
      SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
      SDL_RenderClear(m_pRenderer);
      SDL_RenderCopyExF(m_pRenderer, m_pTexture, NULL, &m_destRect, m_angle, NULL, SDL_FLIP_NONE);
      SDL_RenderPresent(m_pRenderer);
   }
}