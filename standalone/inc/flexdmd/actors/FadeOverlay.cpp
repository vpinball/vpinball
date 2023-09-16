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

void FadeOverlay::Draw(Graphics* graphics)
{
   Actor::Draw(graphics);

   if (GetParent()) {
      SetX(0);
      SetY(0);
      SetWidth(GetParent()->GetWidth());
      SetHeight(GetParent()->GetHeight());
      graphics->FillRectangleAlpha(m_color, (UINT8)SDL_min((m_alpha * 255.f), 255.f), (int)GetX(), (int)GetY(), (int)GetWidth(), (int)GetHeight());
   }
}