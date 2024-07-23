#include "stdafx.h"
#include "ParallelAction.h"

ParallelAction::ParallelAction()
{
}

ParallelAction::~ParallelAction()
{
}

STDMETHODIMP ParallelAction::Add(IUnknown *action, ICompositeAction **pRetVal)
{
   Action* pAction = dynamic_cast<Action*>(action);
   m_actions.push_back(pAction);
   m_runMask.push_back(false);

   return this->QueryInterface(IID_ICompositeAction, (void**)pRetVal);
}

bool ParallelAction::Update(float secondsElapsed) 
{
   bool alive = false;

   for (int i = 0; i < (int)m_actions.size(); i++) {
      if (m_runMask[i]) {
        if (m_actions[i]->Update(secondsElapsed))
           m_runMask[i] = false;
        else 
           alive = true;
      }
   }
   if (!alive) {
      for (int i = 0; i < (int)m_actions.size(); i++)
         m_runMask[i] = true;
   }
   return false;
}