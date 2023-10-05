#include "stdafx.h"
#include "AddToAction.h"

AddToAction::AddToAction(Actor* pTarget, Group* pParent, bool add)
{
   m_pTarget = pTarget;
   m_pParent = pParent;
   m_add = add;
}

AddToAction::~AddToAction()
{
}

bool AddToAction::Update(float secondsElapsed) 
{
   if (m_add)
      m_pParent->AddActor(m_pTarget);
   else
      m_pParent->RemoveActor(m_pTarget);

   return true;
}
