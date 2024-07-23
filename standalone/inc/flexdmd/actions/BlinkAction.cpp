#include "stdafx.h"
#include "BlinkAction.h"

BlinkAction::BlinkAction(Actor* pTarget, float secondsShow, float secondsHide, int repeat)
{
   m_pTarget = pTarget;
   m_secondsShow = secondsShow;
   m_secondsHide = secondsHide;
   m_repeat = repeat;

   m_n = 0;
   m_time = 0;
}

BlinkAction::~BlinkAction()
{
}

bool BlinkAction::Update(float secondsElapsed) 
{
   m_time += secondsElapsed;
   if (m_pTarget->GetVisible() && m_time > m_secondsShow) {
      m_time -= m_secondsShow;
      m_pTarget->SetVisible(false);
      m_n++;
      if (m_repeat >= 0 && m_n > m_repeat)
         return true;
   }
   else if (!m_pTarget->GetVisible() && m_time > m_secondsHide) {
      m_time -= m_secondsHide;
      m_pTarget->SetVisible(true);
   }
   return false;
}