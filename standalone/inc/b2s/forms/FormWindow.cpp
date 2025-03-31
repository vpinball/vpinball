#include "core/stdafx.h"

#include "FormWindow.h"
#include "Form.h"

FormWindow::FormWindow(Form* pForm, const string& szTitle, int z, int x, int y, int w, int h)
    : VP::Window(szTitle, z, x, y, w, h)
{
   m_destRect = { 0.0f, 0.0f, (float)w, (float)h };
   m_pForm = pForm;
   m_pGraphics = nullptr;
   m_pSDLTexture = nullptr;
}

bool FormWindow::Init()
{
   if (!VP::Window::Init())
      return false;

   m_pGraphics = new VP::RendererGraphics(m_pRenderer);
   m_pForm->SetGraphics(m_pGraphics);

   return true;
}

FormWindow::~FormWindow()
{
   delete m_pGraphics;

   if (m_pSDLTexture)
      SDL_DestroyTexture(m_pSDLTexture);
}

void FormWindow::Render()
{
   if (!m_pSDLTexture) {
      m_pSDLTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, GetDrawableWidth(), GetDrawableHeight());
      if (!m_pSDLTexture)
         return;
   }

   if (!m_pTexture) {
      m_pTexture = new BaseTexture(GetDrawableWidth(), GetDrawableHeight(), BaseTexture::RGBA);
      if (!m_pTexture)
         return;
   }

   SDL_SetRenderTarget(m_pRenderer, m_pSDLTexture);
   SDL_SetRenderLogicalPresentation(m_pRenderer, GetWidth(), GetHeight(), SDL_LOGICAL_PRESENTATION_STRETCH);

   bool update = m_pForm->Render();
   if (update) {
      SDL_Surface* pSurface = SDL_RenderReadPixels(m_pRenderer, nullptr);
      if (pSurface) {
         memcpy(m_pTexture->data(), pSurface->pixels, pSurface->pitch * pSurface->h);
         g_pplayer->m_renderer->m_renderDevice->m_texMan.SetDirty(m_pTexture);
         SDL_DestroySurface(pSurface);
      }
   }

   SDL_SetRenderTarget(m_pRenderer, NULL);

   Window::Render();
}
