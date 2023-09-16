#pragma once

#include "Action.h"
#include "../actors/AnimatedActor.h"

class SeekAction : public Action
{
public:
   SeekAction(AnimatedActor* pTarget, float position);
   ~SeekAction();

   virtual bool Update(float secondsElapsed);

private:
   AnimatedActor* m_pTarget;
   float m_position;
};
