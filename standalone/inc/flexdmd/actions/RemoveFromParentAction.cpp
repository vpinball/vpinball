#include "stdafx.h"
#include "RemoveFromParentAction.h"

RemoveFromParentAction::RemoveFromParentAction(Actor* pTarget)
{
   m_pTarget = pTarget;
}

RemoveFromParentAction::~RemoveFromParentAction()
{
}

bool RemoveFromParentAction::Update(float secondsElapsed) 
{
   m_pTarget->Remove();
   return true;
}