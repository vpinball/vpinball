#pragma once

#include "../b2s_i.h"

#include "../collections/RunningAnimationsCollection.h"
#include "PictureBoxAnimation.h"
#include "../forms/Form.h"

class B2SAnimation
{
public:
   B2SAnimation();
   virtual ~B2SAnimation();

   void AddAnimation(
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
      vector<PictureBoxAnimationEntry*> entries);

   void AutoStart();
   void StartAnimation(const string& szName, bool playReverse = false);
   void RestartAnimations();
   void StopAnimation(const string& szName);
   void StopAllAnimations();
   int GetAnimationSlowDown(const string& szName);
   void SetAnimationSlowDown(const string& szName, int value);
   bool IsAnimationRunning(const string& szName);

private:
   RunningAnimationsCollection m_runningAnimations;
   std::map<string, PictureBoxAnimation*> m_pictureBoxAnimations;
};