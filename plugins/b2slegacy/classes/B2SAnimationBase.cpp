#include "../common.h"

#include "B2SAnimationBase.h"
#include "B2SData.h"

namespace B2SLegacy {

B2SAnimationBase::B2SAnimationBase(
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
   int randomQuality) : Timer()
{
   m_pB2SData = pB2SData;
   m_dualMode = dualMode;
   SetInterval(interval);
   m_type = type;
   m_loops = loops;
   m_playReverse = playReverse;
   m_baseInterval = interval;
   m_startMeAtVPActivate = startMeAtVPActivate;
   m_lightsStateAtAnimationStart = lightsStateAtAnimationStart;
   if (m_lightsStateAtAnimationStart == eLightsStateAtAnimationStart_Undefined)
      m_lightsStateAtAnimationStart = eLightsStateAtAnimationStart_NoChange;
   m_lightsStateAtAnimationEnd = lightsStateAtAnimationEnd;
   if (m_lightsStateAtAnimationEnd == eLightsStateAtAnimationEnd_Undefined)
      m_lightsStateAtAnimationEnd = eLightsStateAtAnimationEnd_InvolvedLightsOff;
   m_animationStopBehaviour = animationStopBehaviour;
   if (m_animationStopBehaviour == eAnimationStopBehaviour_Undefined)
      m_animationStopBehaviour = eAnimationStopBehaviour_StopImmediatelly;
   m_lockInvolvedLamps = lockInvolvedLamps;
   m_hideScoreDisplays = hideScoreDisplays;
   m_bringToFront = bringToFront;
   m_randomStart = randomStart;
   m_randomQuality = randomQuality;

   m_slowDown = 1;
   m_wouldBeStarted = false;
   m_stopMeLater = false;

   m_pRunningAnimations = RunningAnimationsCollection::GetInstance();

   m_finishedListener = NULL;

   m_pSwitchTimer = NULL;
}

B2SAnimationBase::~B2SAnimationBase()
{
   delete m_pSwitchTimer;
}

void B2SAnimationBase::RaiseFinishedEvent()
{
   if (m_finishedListener) {
      m_finishedListener();
      m_finishedListener = NULL;
   }
}

void B2SAnimationBase::SetSwitch(int switchid)
{
   // TODO: m_pB2SData->GetVPinMAME()->put_Switch(switchid, VARIANT_TRUE);

   if (!m_switches.contains(switchid))
      m_switches[switchid] = true;

   if (!m_pSwitchTimer) {
      m_pSwitchTimer = new Timer();
      m_pSwitchTimer->SetInterval(200);
      m_pSwitchTimer->SetElapsedListener(std::bind(&B2SAnimationBase::SwitchTimerElapsed, this, std::placeholders::_1));
   }

   m_pSwitchTimer->Stop();
   m_pSwitchTimer->Start();
}

void B2SAnimationBase::SwitchTimerElapsed(Timer* pTimer)
{
   m_pSwitchTimer->Stop();

   // TODO: for (auto& [switchid, value] : m_switches)
   //   m_pB2SData->GetVPinMAME()->put_Switch(switchid, VARIANT_FALSE);

   m_switches.clear();
}

}
