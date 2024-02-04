#pragma once

#include "Action.h"

class SequenceAction : public Action
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   SequenceAction();
   ~SequenceAction();

   STDMETHOD(Add)(IUnknown *action, ICompositeAction **pRetVal);

   bool Update(float secondsElapsed) override;

private:
   vector<Action*> m_actions;
   int m_pos;
};
