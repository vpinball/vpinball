#pragma once

#include "Action.h"

class WaitAction : public Action
{
public:
   WaitAction(float secondsToWait);
   ~WaitAction();

   bool Update(float secondsElapsed) override;

private:
   float m_secondsToWait;
   float m_time;
};
