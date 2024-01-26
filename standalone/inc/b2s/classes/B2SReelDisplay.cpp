#include "stdafx.h"

#include "B2SReelDisplay.h"
#include "../controls/B2SReelBox.h"

B2SReelDisplay::B2SReelDisplay()
{
   m_startDigit = 0;
   m_digits = 0;
   m_score = -1;
   m_nextScore = -1;
   m_currentindex = 0;
   m_currentnewvalue = 0;
   m_currentscore = 0;
   m_currentrestartat = 0;

   m_pTimerRR = new VP::Timer(17, std::bind(&B2SReelDisplay::TimerRRTick, this, std::placeholders::_1));
   m_pTimerIA = new VP::Timer(17, std::bind(&B2SReelDisplay::TimerIATick, this, std::placeholders::_1));
}

B2SReelDisplay::~B2SReelDisplay()
{
   delete m_pTimerRR;
   delete m_pTimerIA;
}

bool B2SReelDisplay::IsInAction()
{
   bool ret = false;
   for (auto& [key, pReelbox] : m_reels) {
      if (pReelbox->IsInAction()) {
         ret = true;
         break;
      }
   }
   return ret;
}

void B2SReelDisplay::SetScore(int score)
{
   if (IsInAction()) {
      if (m_score != score)
         m_nextScore = score;
   }
   else {
      m_score = score;
      SetScore_(score);
   }
}

void B2SReelDisplay::SetScore_(int score, int startAtIndex)
{
   if (m_reels.size() > 0) {
      m_pTimerIA->Start();

      std::ostringstream oss;
      oss << std::setw(m_digits) << std::setfill('0') << score;
      string scoreAsStringX = oss.str();

      int j = 1;
      for (int i = m_startDigit + m_digits - startAtIndex - 1; i >= m_startDigit; i--) {
         if (m_reels.contains(i)) {
            B2SReelBox* pReelbox = m_reels[i];
            int value = pReelbox->GetCurrentText();
            int newvalue = std::stoi(scoreAsStringX.substr(i - m_startDigit, 1));
            bool nextReelShouldWait = (value > newvalue && score > 0);
            m_reels[i]->SetText(std::stoi(scoreAsStringX.substr(i - m_startDigit, 1)), true);
            // maybe get out here since the current reel is rolling over '9'
            if (nextReelShouldWait) {
               StartTimer(i, newvalue, score, j);
               break;
            }
         }
         j++;
      }
   }
}

void B2SReelDisplay::StartTimer(int index, int newvalue, int score, int restartfromright)
{
   m_currentindex = index;
   m_currentnewvalue = newvalue;
   m_currentscore = score;
   m_currentrestartat = restartfromright;
   m_pTimerRR->Start();
}

void B2SReelDisplay::TimerRRTick(VP::Timer* pTimer)
{
   if (m_currentrestartat == 0 || m_reels[m_currentindex]->GetCurrentText() <= m_currentnewvalue
      || (m_reels[m_currentindex]->GetCurrentText() >= 9 && !m_reels[m_currentindex]->IsInReelRolling())) {
      m_pTimerRR->Stop();

      int restartfromright = m_currentrestartat;
      int score = m_currentscore;
      m_currentindex = 0;
      m_currentnewvalue = 0;
      m_currentscore = 0;
      m_currentrestartat = 0;
      SetScore_(score, restartfromright);
   }
}

void B2SReelDisplay::TimerIATick(VP::Timer* pTimer)
{
   if (!IsInAction()) {
      m_pTimerIA->Stop();

      if (m_nextScore > 0) {
         int nextscore = m_nextScore;
         m_nextScore = 0;
         StartTimer(0, 0, nextscore, 0);
      }
   }
}