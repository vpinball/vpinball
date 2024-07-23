/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 */

#include "stdafx.h"

#include "PUPWindow.h"
#include "PUPScreen.h"

class PUPScreen;

PUPWindow::PUPWindow(PUPScreen* pScreen, const string& szTitle, int x, int y, int w, int h, int z, int rotation)
    : VP::Window(szTitle, x, y, w, h, z, rotation)
{
   m_pScreen = pScreen;
   m_pScreen->SetSize(w, h);
}

PUPWindow::~PUPWindow()
{
}

bool PUPWindow::Init()
{
   if (!VP::Window::Init())
      return false;

   if (m_pScreen)
      m_pScreen->Init(m_pRenderer);

   return true;
}

void PUPWindow::Render()
{
   if (!m_visible)
      return;

   SDL_SetRenderDrawColor(m_pRenderer, 0, 0, 0, 255);
   SDL_RenderClear(m_pRenderer);

   if (m_pScreen)
      m_pScreen->Render();

   SDL_RenderPresent(m_pRenderer);
}
