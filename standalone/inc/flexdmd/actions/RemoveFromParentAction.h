#pragma once

#include "Action.h"
#include "../actors/Actor.h"

class RemoveFromParentAction : public Action
{
public:
   RemoveFromParentAction(Actor* pTarget);
   ~RemoveFromParentAction();

   virtual bool Update(float secondsElapsed);

private:
   Actor* m_pTarget;
};
