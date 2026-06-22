#pragma once

#include "../collections/ControlCollection.h"
#include "../collections/ReelDisplayReelBoxCollection.h"
#include "../utils/Timer.h"

namespace B2SLegacy {

class B2SReelDisplay final
{
public:
   B2SReelDisplay();
   virtual ~B2SReelDisplay();

   ReelDisplayReelBoxCollection* GetReels() { return &m_reels; }
   int GetStartDigit() const { return m_startDigit; }
   void SetStartDigit(int startDigit) { m_startDigit = startDigit; }
   int GetDigits() const { return m_digits; }
   void SetDigits(int digits) { m_digits = digits; }
   bool IsInAction() const;
   int GetScore() const { return m_score; }
   void SetScore(int score);
   void SetScore_(int score, int startIndex = 0);

private:
   void StartTimer(int index, int newvalue, int score, int restartfromright);
   void TimerRRTick(Timer* pTimer);
   void TimerIATick(Timer* pTimer);

   Timer* m_pTimerRR = nullptr;
   Timer* m_pTimerIA = nullptr;
   ReelDisplayReelBoxCollection m_reels;
   int m_startDigit = 0;
   int m_digits = 0;
   int m_score = -1;
   int m_nextScore = -1;
   int m_currentindex = 0;
   int m_currentnewvalue = 0;
   int m_currentscore = 0;
   int m_currentrestartat = 0;
};

}
