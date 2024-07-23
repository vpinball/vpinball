#pragma once

#include "Action.h"
#include "../actors/Actor.h"

#include "../inc/tweeny-3.2.0.h"

class TweenAction : public Action
{
public:
   STDMETHOD(GetIDsOfNames)(REFIID /*riid*/, LPOLESTR* rgszNames, UINT cNames, LCID lcid,DISPID* rgDispId);
   STDMETHOD(Invoke)(DISPID dispIdMember, REFIID /*riid*/, LCID lcid, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult, EXCEPINFO* pExcepInfo, UINT* puArgErr);

public:
   TweenAction(Actor* pTarget, float duration);
   ~TweenAction();

   STDMETHOD(get_Ease)(Interpolation *pRetVal);
   STDMETHOD(put_Ease)(Interpolation pRetVal);

   bool Update(float secondsElapsed) override;

   virtual void Begin() = 0;
   virtual void End() { };

   Actor* GetTarget() { return m_pTarget; }
   float GetDuration() { return m_duration; }

   tweeny::tween<float>& AddTween(float from, float to, float duration, std::function<bool (float)> callback);

private:
   Actor* m_pTarget;
   float m_duration;
   Interpolation m_ease;

   vector<tweeny::tween<float>> m_tweens;
};
