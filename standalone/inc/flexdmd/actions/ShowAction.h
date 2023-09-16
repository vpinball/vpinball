#pragma once

#include "Action.h"
#include "../actors/Actor.h"

class ShowAction : public Action
{
public:
   ShowAction(Actor* pTarget, bool visible);
   ~ShowAction();

   virtual bool Update(float secondsElapsed);

private:
   Actor* m_pTarget;
   bool m_visible; 
};
