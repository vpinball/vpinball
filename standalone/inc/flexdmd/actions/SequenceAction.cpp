#include "stdafx.h"
#include "SequenceAction.h"

SequenceAction::SequenceAction()
{
   m_pos = 0;
}

SequenceAction::~SequenceAction()
{
}

STDMETHODIMP SequenceAction::Add(IUnknown *action, ICompositeAction **pRetVal)
{
   Action* pAction = dynamic_cast<Action*>(action);
   if (pAction == NULL) {
      return E_INVALIDARG;
   }

   m_actions.push_back(pAction);

   return this->QueryInterface(IID_ICompositeAction, (void**)pRetVal);
}

bool SequenceAction::Update(float secondsElapsed) 
{
   if (m_pos >= (int)m_actions.size()) {
      m_pos = 0;
      return true;
   }
   while (m_actions[m_pos]->Update(secondsElapsed)) {
      m_pos++;
      if (m_pos >= (int)m_actions.size()) {
         m_pos = 0;
         return true;
      }
   }
   return false;
}