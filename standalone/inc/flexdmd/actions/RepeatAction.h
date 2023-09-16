#pragma once

#include "Action.h"

class RepeatAction : public Action
{
public:
   RepeatAction(Action* pAction, int count);
   ~RepeatAction();

   virtual bool Update(float secondsElapsed);

private:
   Action* m_pAction;
   int m_count;
   int m_n;
};
