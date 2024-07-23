#include "stdafx.h"
#include "AddChildAction.h"

AddChildAction::AddChildAction(Group* pTarget, Actor* pChild, bool add)
{
   m_pTarget = pTarget;
   m_pChild = pChild;
   m_add = add;
}

AddChildAction::~AddChildAction()
{
}

bool AddChildAction::Update(float secondsElapsed) 
{
   if (m_add)
      m_pTarget->AddActor(m_pChild);
   else
      m_pTarget->RemoveActor(m_pChild);

   return true;
}