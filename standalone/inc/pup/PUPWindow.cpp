/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 */

#include "core/stdafx.h"

#include "PUPWindow.h"
#include "PUPScreen.h"

class PUPScreen;

PUPWindow::PUPWindow(PUPScreen* pScreen, const string& szTitle, int z, int x, int y, int w, int h)
    : VP::Window(szTitle, z, x, y, w, h)
{
   m_pScreen = pScreen;
   m_pScreen->SetSize(w, h);
   m_pSDLTexture = nullptr;
}

PUPWindow::~PUPWindow()
{
}

bool PUPWindow::Init()
{
   if (!VP::Window::Init())
      return false;

   m_pScreen->Init(this);

   return true;
}

void PUPWindow::Render()
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

   m_pScreen->Render();

   SDL_Surface* pSurface = SDL_RenderReadPixels(m_pRenderer, nullptr);
   if (pSurface) {
      memcpy(m_pTexture->data(), pSurface->pixels, pSurface->pitch * pSurface->h);
      g_pplayer->m_renderer->m_renderDevice->m_texMan.SetDirty(m_pTexture);
      SDL_DestroySurface(pSurface);
   }

   SDL_SetRenderTarget(m_pRenderer, NULL);

   Window::Render();
 }
