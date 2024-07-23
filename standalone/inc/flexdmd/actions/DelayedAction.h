#pragma once

#include "Action.h"

class DelayedAction : public Action
{
public:
   DelayedAction(float secondsToWait, Action* pAction);
   ~DelayedAction();

   bool Update(float secondsElapsed) override;

private:
   float m_secondsToWait;
   Action* m_pAction;
   float m_time;
};
