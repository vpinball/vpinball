#include "stdafx.h"
#include "WaitAction.h"

WaitAction::WaitAction(float secondsToWait)
{
   m_secondsToWait = secondsToWait;
   m_time = 0.0f;
}

WaitAction::~WaitAction()
{
}

bool WaitAction::Update(float secondsElapsed) 
{
   m_time += secondsElapsed;
   if (m_time >= m_secondsToWait) {
      // Prepare for restart
      m_time = 0.0f;
      return true;
   }
   return false;
}