#pragma once

#include "Action.h"
#include "../actors/AnimatedActor.h"

class SeekAction : public Action
{
public:
   SeekAction(AnimatedActor* pTarget, float position);
   ~SeekAction();

   bool Update(float secondsElapsed) override;

private:
   AnimatedActor* m_pTarget;
   float m_position;
};
