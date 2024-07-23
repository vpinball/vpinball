#include "stdafx.h"

#include "FadeOverlay.h"
#include "Group.h"

FadeOverlay::FadeOverlay(FlexDMD* pFlexDMD, const string& name) : Actor(pFlexDMD, name)
{
   m_alpha = 1.0f;
   m_color = RGB(0, 0, 0);
}

FadeOverlay::~FadeOverlay()
{
}

void FadeOverlay::Draw(VP::SurfaceGraphics* pGraphics)
{
   Actor::Draw(pGraphics);

   if (GetParent()) {
      SetX(0);
      SetY(0);
      SetWidth(GetParent()->GetWidth());
      SetHeight(GetParent()->GetHeight());
      pGraphics->SetColor(m_color, SDL_min((m_alpha * 255.f), 255.f));
      pGraphics->FillRectangle({ (int)GetX(), (int)GetY(), (int)GetWidth(), (int)GetHeight() });
   }
}