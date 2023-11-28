#include "stdafx.h"

#include "Form.h"

Form::Form()
{
   m_pGraphics = NULL;
   m_topMost = false;
}

Form::~Form()
{
}

void Form::Show()
{
}

void Form::Hide()
{
}

void Form::Render(VP::Graphics* pGraphics)
{
   if (!IsInvalidated() || !pGraphics)
      return;

   pGraphics->SetColor(RGB(0, 0, 0));
   pGraphics->Clear();

   pGraphics->SetBlendMode(SDL_BLENDMODE_BLEND);

   OnPaint(pGraphics);

   pGraphics->Present();
}