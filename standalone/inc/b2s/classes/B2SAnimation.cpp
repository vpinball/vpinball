#include "stdafx.h"

#include "B2SAnimation.h"
#include "B2SData.h"

B2SAnimation::B2SAnimation()
{
}

B2SAnimation::~B2SAnimation()
{
   for (auto& [key, pPictimer] : m_pictureBoxAnimations)
      delete pPictimer;
}

void B2SAnimation::AddAnimation(
   string szName,
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
   vector<PictureBoxAnimationEntry*> entries)
{
   if (!m_pictureBoxAnimations.contains(szName))
      m_pictureBoxAnimations[szName] = new PictureBoxAnimation(pForm, pFormDMD, szName, dualMode, interval, loops, startTimerAtVPActivate, lightsStateAtAnimationStart, lightsStateAtAnimationEnd, animationStopBehaviour, lockInvolvedLamps, hideScoreDisplays, bringToFront, randomStart, randomQuality, entries);
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
   if (m_pictureBoxAnimations.contains(szName) && !m_pictureBoxAnimations[szName]->IsEnabled()) {
      m_pictureBoxAnimations[szName]->SetPlayReverse(playReverse);
      m_pictureBoxAnimations[szName]->Start();
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
   if (m_pictureBoxAnimations.contains(szName))
      m_pictureBoxAnimations[szName]->Stop();
}

void B2SAnimation::StopAllAnimations()
{
   for (auto& [key, pTimer] : m_pictureBoxAnimations)
      pTimer->Stop();
}

int B2SAnimation::GetAnimationSlowDown(const string& szName)
{
   if (m_pictureBoxAnimations.contains(szName))
      return m_pictureBoxAnimations[szName]->GetSlowDown();
   return 1;
}

void B2SAnimation::SetAnimationSlowDown(const string& szName, int value)
{
   if (m_pictureBoxAnimations.contains(szName))
      m_pictureBoxAnimations[szName]->SetSlowDown(value);
}

bool B2SAnimation::IsAnimationRunning(const string& szName)
{
   if (m_pictureBoxAnimations.contains(szName))
      return m_pictureBoxAnimations[szName]->IsEnabled();
   return false;
}