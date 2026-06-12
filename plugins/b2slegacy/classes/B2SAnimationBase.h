#pragma once

#include "../collections/RunningAnimationsCollection.h"
#include "../utils/Timer.h"

#include <map>

namespace B2SLegacy {

class B2SData;
class B2SAnimation;

class B2SAnimationBase : public Timer
{
public:
   B2SAnimationBase(
      B2SData* pB2SData,
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
   ~B2SAnimationBase() override;

   B2SAnimation* GetB2SAnimation() const { return m_pB2SAnimation; }
   const string& GetName() const { return m_szName; }
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
   void SetFinishedListener(FinishedListener listener) { m_finishedListener = listener; }

protected:
   void SetSwitch(int switchid);
   void SwitchTimerElapsed(Timer* pTimer);

   B2SData* m_pB2SData = nullptr;

private:
   B2SAnimation* m_pB2SAnimation = nullptr;
   string m_szName;
   eDualMode m_dualMode = eDualMode_Both;
   eType m_type = eType_Undefined;
   int m_loops = 0;
   bool m_playReverse = false;
   bool m_startMeAtVPActivate = false;
   eLightsStateAtAnimationStart m_lightsStateAtAnimationStart = eLightsStateAtAnimationStart_Undefined;
   eLightsStateAtAnimationEnd m_lightsStateAtAnimationEnd = eLightsStateAtAnimationEnd_Undefined;
   eAnimationStopBehaviour m_animationStopBehaviour = eAnimationStopBehaviour_Undefined;
   bool m_lockInvolvedLamps = false;
   bool m_hideScoreDisplays = false;
   bool m_bringToFront = false;
   bool m_randomStart = false;
   int m_randomQuality = 0;
   int m_baseInterval = 0;
   int m_slowDown = 1;
   bool m_wouldBeStarted = false;
   bool m_stopMeLater = false;
   RunningAnimationsCollection* m_pRunningAnimations = nullptr;
   FinishedListener m_finishedListener;
   std::map<int, bool> m_switches;
   Timer* m_pSwitchTimer = nullptr;
};

}
