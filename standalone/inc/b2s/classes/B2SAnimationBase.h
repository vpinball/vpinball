#pragma once

#include "../b2s_i.h"
#include "../collections/RunningAnimationsCollection.h"
#include "../../common/Timer.h"

class B2SAnimationBase : public VP::Timer
{
public:
   B2SAnimationBase(
      eDualMode dualMode,
      int interval,
      eType type,
      int loops,
      bool playReverse,
      bool startMeAtVPActivate,
      eLightsStateAtAnimationStart lightsStateAtAnimationStart,
      eLightsStateAtAnimationEnd lightsStateAtAnimationEnd,
      eAnimationStopBehaviour animationStopBehaviour,
      bool lockInvolvedLamps,
      bool hideScoreDisplays,
      bool bringToFront,
      bool randomStart,
      int randomQuality);
   ~B2SAnimationBase();

   B2SAnimation* GetB2SAnimation() const { return m_pB2SAnimation; }
   string GetName() const { return m_szName; }
   void SetName(const string& szName) { m_szName = szName; }
   eDualMode GetDualMode() const { return m_dualMode; }
   eType GetType() const { return m_type; }
   int GetLoops() const { return m_loops; }
   void SetLoops(int loops) { m_loops = loops; }
   bool IsPlayReverse() const { return m_playReverse; }
   void SetPlayReverse(bool playReverse) { m_playReverse = playReverse; }
   bool IsStartMeAtVPActivate() const { return m_startMeAtVPActivate; }
   eLightsStateAtAnimationStart GetLightsStateAtAnimationStart() const { return m_lightsStateAtAnimationStart; }
   eLightsStateAtAnimationEnd GetLightsStateAtAnimationEnd() const { return m_lightsStateAtAnimationEnd; }
   eAnimationStopBehaviour GetAnimationStopBehaviour() const { return m_animationStopBehaviour; }
   bool IsLockInvolvedLamps() const { return m_lockInvolvedLamps; }
   bool IsHideScoreDisplays() const { return m_hideScoreDisplays; }
   bool IsBringToFront() const { return m_bringToFront; }
   bool IsRandomStart() const { return m_randomStart; }
   int GetBaseInterval() const { return m_baseInterval; }
   int GetSlowDown() const { return m_slowDown; }
   void SetSlowDown(int slowDown) { m_slowDown = slowDown; }
   bool IsWouldBeStarted() const { return m_wouldBeStarted; }
   void SetWouldBeStarted(bool wouldBeStarted) { m_wouldBeStarted = wouldBeStarted; }
   bool IsStopMeLater() const { return m_stopMeLater; }
   void SetStopMeLater(bool stopMeLater) { m_stopMeLater = stopMeLater; }
   RunningAnimationsCollection* GetRunningAnimations() const { return m_pRunningAnimations; }
   void RaiseFinishedEvent();
   using FinishedListener = std::function<void()>;
   void SetFinishedListener(FinishedListener listener) { m_finishedListener = listener; };

protected:
   void SetSwitch(int switchid);
   void SwitchTimerElapsed(VP::Timer* pTimer);

private:
   B2SAnimation* m_pB2SAnimation;
   string m_szName;
   eDualMode m_dualMode;
   eType m_type;
   int m_loops;
   bool m_playReverse;
   bool m_startMeAtVPActivate;
   eLightsStateAtAnimationStart m_lightsStateAtAnimationStart;
   eLightsStateAtAnimationEnd m_lightsStateAtAnimationEnd;
   eAnimationStopBehaviour m_animationStopBehaviour;
   bool m_lockInvolvedLamps;
   bool m_hideScoreDisplays;
   bool m_bringToFront;
   bool m_randomStart;
   int m_randomQuality;
   int m_baseInterval;
   int m_slowDown;
   bool m_wouldBeStarted;
   bool m_stopMeLater;
   RunningAnimationsCollection* m_pRunningAnimations;
   FinishedListener m_finishedListener;
   std::map<int, bool> m_switches;
   VP::Timer* m_pSwitchTimer;
};