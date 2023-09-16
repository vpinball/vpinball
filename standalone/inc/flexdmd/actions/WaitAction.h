#pragma once

#include "Action.h"

class WaitAction : public Action
{
public:
   WaitAction(float secondsToWait);
   ~WaitAction();

   virtual bool Update(float secondsElapsed);

private:
   float m_secondsToWait;
   float m_time;
};
