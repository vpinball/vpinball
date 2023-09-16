#pragma once

#include "Action.h"
#include "../actors/Actor.h"

class BlinkAction : public Action
{
public:
   BlinkAction(Actor* pTarget, float secondsShow, float secondsHide, int repeat);
   ~BlinkAction();

   virtual bool Update(float secondsElapsed);

private:
   Actor* m_pTarget;
   float m_secondsShow;
   float m_secondsHide;
   int m_repeat;

   int m_n;
   float m_time;
};
