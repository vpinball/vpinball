#include "stdafx.h"
#include "ShowAction.h"

ShowAction::ShowAction(Actor* pTarget, bool visible)
{
   m_pTarget = pTarget;
   m_visible = visible;
}

ShowAction::~ShowAction()
{
}

bool ShowAction::Update(float secondsElapsed) 
{
   m_pTarget->SetVisible(m_visible);
   return true;
}
