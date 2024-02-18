#include "core/stdafx.h"

#include "Form.h"
#include "FormWindow.h"

Form::Form()
{
   m_topMost = false;
   m_pWindow = nullptr;
}

Form::~Form()
{
   delete m_pWindow;
}

void Form::Show()
{
   if (m_pWindow)
      m_pWindow->Show();
}

void Form::Hide()
{
   if (m_pWindow)
      m_pWindow->Hide();
}

void Form::Render(VP::RendererGraphics* pGraphics)
{
   if (!IsInvalidated())
      return;

   pGraphics->SetColor(RGB(0, 0, 0));
   pGraphics->Clear();

   pGraphics->SetBlendMode(SDL_BLENDMODE_BLEND);

   OnPaint(pGraphics);

   pGraphics->Present();
}