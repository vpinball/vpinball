#include "stdafx.h"
#include "RepeatAction.h"

RepeatAction::RepeatAction(Action* pAction, int count)
{
   m_pAction = pAction;
   m_count = count;
}

RepeatAction::~RepeatAction()
{
}

bool RepeatAction::Update(float secondsElapsed) 
{
   if (m_pAction->Update(secondsElapsed)) {
      m_n++;
      if (m_n == m_count) {
         // Prepare for restart
         m_n = 0;
         return true;
      }
   }
   return false;
}