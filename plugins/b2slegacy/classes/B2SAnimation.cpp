#include "../common.h"

#include "B2SAnimation.h"
#include "B2SData.h"

namespace B2SLegacy {

B2SAnimation::B2SAnimation()
{
}

B2SAnimation::~B2SAnimation()
{
   for (auto& [key, pPictimer] : m_pictureBoxAnimations)
      delete pPictimer;
}

void B2SAnimation::AddAnimation(
   B2SData* pB2SData,
   const string& szName,
   Form* pForm,
   Form* pFormDMD,
   eDualMode dualMode,
   int interval,
   int loops,
   bool startTimerAtVPActivate,
   eLightsStateAtAnimationStart lightsStateAtAnimationStart,
   eLightsStateAtAnimationEnd lightsStateAtAnimationEnd,
   eAnimationStopBehaviour animationStopBehaviour,
   bool lockInvolvedLamps,
   bool hideScoreDisplays,
   bool bringToFront,
   bool randomStart,
   int randomQuality,
   const vector<PictureBoxAnimationEntry*>& entries)
{
   if (!m_pictureBoxAnimations.contains(szName))
      m_pictureBoxAnimations[szName] = new PictureBoxAnimation(pB2SData, pForm, pFormDMD, szName, dualMode, interval, loops, startTimerAtVPActivate, lightsStateAtAnimationStart, lightsStateAtAnimationEnd, animationStopBehaviour, lockInvolvedLamps, hideScoreDisplays, bringToFront, randomStart, randomQuality, entries);
}

void B2SAnimation::AutoStart()
{
   // start all autostart picture box animations
   for (auto& [key, pPictimer] : m_pictureBoxAnimations) {
      if (pPictimer->IsStartMeAtVPActivate() && !pPictimer->IsEnabled())
         pPictimer->Start();
   }
}

void B2SAnimation::StartAnimation(const string& szName, bool playReverse)
{
   const auto& it = m_pictureBoxAnimations.find(szName);
   if (it != m_pictureBoxAnimations.end() && !it->second->IsEnabled()) {
      it->second->SetPlayReverse(playReverse);
      it->second->Start();
    }
}

void B2SAnimation::RestartAnimations()
{
    for (auto& [key, pTimer] : m_pictureBoxAnimations) {
       if (pTimer->IsWouldBeStarted()) {
          pTimer->Stop();
          pTimer->Start();
       }
       else
          pTimer->Stop();
    }
}

void B2SAnimation::StopAnimation(const string& szName)
{
   const auto& it = m_pictureBoxAnimations.find(szName);
   if (it != m_pictureBoxAnimations.end())
      it->second->Stop();
}

void B2SAnimation::StopAllAnimations()
{
   for (auto& [key, pTimer] : m_pictureBoxAnimations)
      pTimer->Stop();
}

int B2SAnimation::GetAnimationSlowDown(const string& szName)
{
   const auto& it = m_pictureBoxAnimations.find(szName);
   if (it != m_pictureBoxAnimations.end())
      return it->second->GetSlowDown();
   return 1;
}

void B2SAnimation::SetAnimationSlowDown(const string& szName, int value)
{
   const auto& it = m_pictureBoxAnimations.find(szName);
   if (it != m_pictureBoxAnimations.end())
      it->second->SetSlowDown(value);
}

bool B2SAnimation::IsAnimationRunning(const string& szName)
{
   const auto& it = m_pictureBoxAnimations.find(szName);
   if (it != m_pictureBoxAnimations.end())
      return it->second->IsEnabled();
   return false;
}

}
