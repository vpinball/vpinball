#include "stdafx.h"

#include "Form.h"

Form::Form()
{
   m_pGraphics = NULL;
   m_topMost = false;

   m_pTexture = NULL;
}

Form::~Form()
{
   if (m_pTexture)
      SDL_DestroyTexture(m_pTexture);

   if (m_pGraphics)
      delete m_pGraphics;
}

void Form::Show()
{
   if (m_pGraphics)
      delete m_pGraphics;

   m_pGraphics = new VP::Graphics(GetWidth(), GetHeight(), true);
}

void Form::Hide()
{
   if (m_pGraphics)
      delete m_pGraphics;

   m_pGraphics = NULL;
}

void Form::Render(SDL_Renderer* pRenderer)
{
   if (!IsInvalidated() || !m_pGraphics)
      return;

   OnPaint(m_pGraphics);

   if (!m_pTexture) {
      m_pTexture = m_pGraphics->ToTexture(pRenderer);

      if (!m_pTexture)
         return;
   }
   else
      m_pGraphics->UpdateTexture(m_pTexture);

   SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
   SDL_RenderClear(pRenderer);
   SDL_RenderCopy(pRenderer, m_pTexture, NULL, NULL);
   SDL_RenderPresent(pRenderer);
}