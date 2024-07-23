#include "stdafx.h"
#include "DelayedAction.h"

DelayedAction::DelayedAction(float secondsToWait, Action* pAction)
{
   m_secondsToWait = secondsToWait;
   m_pAction = pAction;

   m_time = 0.0f;
}

DelayedAction::~DelayedAction()
{
}

bool DelayedAction::Update(float secondsElapsed) 
{
   m_time += m_secondsToWait;
   if (m_time >= m_secondsToWait && m_pAction->Update((secondsElapsed))) {
      // Prepare for restart
      m_time = 0.0f;
      return true;
   }
   return false;
}