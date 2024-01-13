#include "stdafx.h"

#include "Form.h"

Form::Form()
{
   m_topMost = false;
   m_pWindow = nullptr;
}

Form::~Form()
{
   if (m_pWindow)
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

void Form::Render()
{
   if (!m_pWindow || !m_pWindow->GetGraphics() || !IsInvalidated())
      return;

   VP::Graphics* pGraphics = m_pWindow->GetGraphics();

   pGraphics->SetColor(RGB(0, 0, 0));
   pGraphics->Clear();

   pGraphics->SetBlendMode(SDL_BLENDMODE_BLEND);

   OnPaint(pGraphics);

   pGraphics->Present();
}