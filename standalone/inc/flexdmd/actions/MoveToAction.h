#pragma once

#include "TweenAction.h"
#include "../actors/Actor.h"

class MoveToAction : public TweenAction
{
public:
   MoveToAction(Actor* pTarget, float x, float y, float duration);
   ~MoveToAction();

   void Begin();

private:
   float m_x;
   float m_y;
};
