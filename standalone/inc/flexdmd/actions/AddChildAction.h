#pragma once

#include "Action.h"
#include "../actors/Group.h"

class AddChildAction : public Action
{
public:
   AddChildAction(Group* pTarget, Actor* pChild, bool add);
   ~AddChildAction();

   bool Update(float secondsElapsed) override;

private:
   Group* m_pTarget;
   Actor* m_pChild;
   bool m_add;
};
