#pragma once

#include "Action.h"
#include "../actors/Group.h"

class AddToAction : public Action
{
public:
   AddToAction(Actor* pTarget, Group* pParent, bool add);
   ~AddToAction();

   virtual bool Update(float secondsElapsed);

private:
   Actor* m_pTarget;
   Group* m_pParent;
   bool m_add;
};
