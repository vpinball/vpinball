#pragma once

#include "../collections/ControlCollection.h"
#include "../collections/ReelDisplayReelBoxCollection.h"
#include "../utils/Timer.h"

namespace B2SLegacy {

class B2SReelDisplay
{
public:
   B2SReelDisplay();
   virtual ~B2SReelDisplay();

   ReelDisplayReelBoxCollection* GetReels() { return &m_reels; }
   int GetStartDigit() const { return m_startDigit; }
   void SetStartDigit(int startDigit) { m_startDigit = startDigit; }
   int GetDigits() const { return m_digits; }
   void SetDigits(int digits) { m_digits = digits; }
   bool IsInAction();
   int GetScore() const { return m_score; }
   void SetScore(int score);
   void SetScore_(int score, int startIndex = 0);

private:
   void StartTimer(int index, int newvalue, int score, int restartfromright);
   void TimerRRTick(Timer* pTimer);
   void TimerIATick(Timer* pTimer);

   Timer* m_pTimerRR;
   Timer* m_pTimerIA;
   ReelDisplayReelBoxCollection m_reels;
   int m_startDigit;
   int m_digits;
   int m_score;
   int m_nextScore;
   int m_currentindex;
   int m_currentnewvalue;
   int m_currentscore;
   int m_currentrestartat;
};

}
