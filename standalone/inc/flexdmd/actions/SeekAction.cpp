#include "stdafx.h"
#include "SeekAction.h"

SeekAction::SeekAction(AnimatedActor* pTarget, float position)
{
   m_pTarget = pTarget;
   m_position = position;
}

SeekAction::~SeekAction()
{
}

bool SeekAction::Update(float secondsElapsed) 
{
   m_pTarget->Seek(m_position);
   return true;
}