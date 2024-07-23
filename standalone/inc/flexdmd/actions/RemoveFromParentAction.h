#pragma once

#include "Action.h"
#include "../actors/Actor.h"

class RemoveFromParentAction : public Action
{
public:
   RemoveFromParentAction(Actor* pTarget);
   ~RemoveFromParentAction();

   bool Update(float secondsElapsed) override;

private:
   Actor* m_pTarget;
};
