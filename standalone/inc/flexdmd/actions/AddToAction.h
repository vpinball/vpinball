#pragma once

#include "Action.h"
#include "../actors/Group.h"

class AddToAction : public Action
{
public:
   AddToAction(Actor* pTarget, Group* pParent, bool add);
   ~AddToAction();

   bool Update(float secondsElapsed) override;

private:
   Actor* m_pTarget;
   Group* m_pParent;
   bool m_add;
};
