#include "stdafx.h"

#include "PictureBoxAnimation.h"
#include "PictureBoxAnimationEntry.h"
#include "../controls/B2SPictureBox.h"
#include "../controls/B2SLEDBox.h"
#include "../dream7/Dream7Display.h"
#include "../forms/Form.h"
#include "../forms/FormBackglass.h"
#include "B2SData.h"
#include "B2SSettings.h"
#include "B2SAnimation.h"

PictureBoxAnimation::PictureBoxAnimation(
   Form* pForm,
   Form* pFormDMD,
   const string& szName,
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
   vector<PictureBoxAnimationEntry*> entries) : B2SAnimationBase(dualMode, interval, eType_ImageCollectionAtForm, loops, false,
      startTimerAtVPActivate, lightsStateAtAnimationStart, lightsStateAtAnimationEnd, animationStopBehaviour,
      lockInvolvedLamps, hideScoreDisplays, bringToFront, randomStart, randomQuality)
{
   //Me.SynchronizingObject = _Form
   m_pForm = pForm;
   m_pFormDMD = pFormDMD;
   SetName(szName);

   m_loopticker = 0;
   m_ticker = 0;
   m_reachedThe0Point = false;
   m_pMainFormBackgroundImage = NULL;
   SetElapsedListener(std::bind(&PictureBoxAnimation::PictureBoxAnimationTick, this, std::placeholders::_1));

   // collect all the entries in a nice collection
   for (auto& pEntry : entries)
   {
      bool isOn1Valid = (!pEntry->GetOn1()[0].empty() || pEntry->GetWaitAfterOn1() > 0);
      bool isOff1Valid = (!pEntry->GetOff1()[0].empty() || pEntry->GetWaitAfterOff1() > 0);
      bool isOn2Valid = (!pEntry->GetOn2()[0].empty() || pEntry->GetWaitAfterOn2() > 0);
      bool isOff2Valid = (!pEntry->GetOff2()[0].empty() || pEntry->GetWaitAfterOff2() > 0);
      int pulseswitch = pEntry->GetPulseSwitch();
      if (isOn1Valid) {
         m_entries[m_entries.size() + 1] = new EntryAction(*pEntry->GetOn1(), pEntry->GetWaitAfterOn1(), true, isOff1Valid ? 1 : 0, pulseswitch);
         pulseswitch = 0;
      }
      if (isOff1Valid) {
         m_entries[m_entries.size() + 1] = new EntryAction(*pEntry->GetOff1(), pEntry->GetWaitAfterOff1(), false, isOn1Valid ? -1 : 0, pulseswitch);
         pulseswitch = 0;
      }
      if (isOn2Valid) {
         m_entries[m_entries.size() + 1] = new EntryAction(*pEntry->GetOn2(), pEntry->GetWaitAfterOn2(), true, isOff2Valid ? 1 : 0, pulseswitch);
         pulseswitch = 0;
      }
      if (isOff2Valid) {
         m_entries[m_entries.size() + 1] = new EntryAction(*pEntry->GetOff2(), pEntry->GetWaitAfterOff2(), false, isOn2Valid ? -1 : 0, pulseswitch);
         pulseswitch = 0;
      }
      if (pulseswitch > 0) {
         m_entries[m_entries.size() + 1] =  new EntryAction(std::vector<std::string>{""}, 0, true, 0, pulseswitch);
         pulseswitch = 0;
      }
   }

   // pick up all involved lights
   m_lightsInvolved.clear();
   for (auto& [key, pEntry] : m_entries) {
      for (auto& bulb : *pEntry->GetBulbs()) {
         if (!bulb.empty() && std::find(m_lightsInvolved.begin(), m_lightsInvolved.end(), bulb) == m_lightsInvolved.end())
            m_lightsInvolved.push_back(bulb);
      }
   }
}

PictureBoxAnimation::~PictureBoxAnimation() {
   Stop();
}

void PictureBoxAnimation::Start()
{
   // set the start flag
   SetWouldBeStarted(true);
   SetStopMeLater(false);

   B2SSettings* pB2SSettings = B2SSettings::GetInstance();

   // maybe get out here because animation is not allowed to start
   if (pB2SSettings->GetAllAnimationSlowDown() == 0 || GetSlowDown() == 0)
      return;

   B2SData* pB2SData = B2SData::GetInstance();

   // maybe get out here because of not matching dual mode
   if (pB2SData->IsDualBackglass()) {
      if (!(GetDualMode() == eDualMode_Both || GetDualMode() == pB2SSettings->GetCurrentDualMode()))
         return;
   }

   // maybe switch off all lights but pick them up
   m_lightsStateAtStartup.clear();
   for (int i = 1; i <= 2; i++) {
      Form* pCurrentForm = (i == 2) ? m_pFormDMD : m_pForm;
      if (pCurrentForm) {
         for (auto& pControl : *pCurrentForm->GetControls()) {
            B2SPictureBox* pPicbox = dynamic_cast<B2SPictureBox*>(pControl);
            if (pPicbox) {
               if (GetLightsStateAtAnimationEnd() == eLightsStateAtAnimationEnd_LightsReseted && m_lightsStateAtStartup.find(pPicbox->GetName()) == m_lightsStateAtStartup.end())
                  m_lightsStateAtStartup[pPicbox->GetName()] = pPicbox->IsVisible();
               if (GetLightsStateAtAnimationStart() == eLightsStateAtAnimationStart_LightsOff)
                  pPicbox->SetVisible(false);
            }
         }
         if (GetLightsStateAtAnimationStart() == eLightsStateAtAnimationStart_LightsOff) {
            FormBackglass* pFormBackglass = dynamic_cast<FormBackglass*>(pCurrentForm);
            if (pFormBackglass) {
               m_pMainFormBackgroundImage = pFormBackglass->GetBackgroundImage();
               pFormBackglass->SetBackgroundImage(pFormBackglass->GetDarkImage());
               pFormBackglass->Refresh();
            }
            else
               pCurrentForm->Refresh();
         }
      }
   }

   // maybe switch on or off all involved lights
   if (GetLightsStateAtAnimationStart() == eLightsStateAtAnimationStart_InvolvedLightsOff || GetLightsStateAtAnimationStart() == eLightsStateAtAnimationStart_InvolvedLightsOn) {
      for (auto& groupname : m_lightsInvolved)
         LightGroup(groupname, (GetLightsStateAtAnimationStart() == eLightsStateAtAnimationStart_InvolvedLightsOn));
   }

   // maybe lock some illu
   if (IsLockInvolvedLamps()) {
      for (auto& groupname : m_lightsInvolved) {
         if (pB2SData->GetIlluminationLocks()->contains(groupname))
            (*pB2SData->GetIlluminationLocks())[groupname]++;
         else
            (*pB2SData->GetIlluminationLocks())[groupname] = 1;
         pB2SData->SetUseIlluminationLocks(true);
      }
   }

   // maybe hide score displays
   if (IsHideScoreDisplays()) {
      eLEDTypes selectedLEDType = GetLEDType();
      if (selectedLEDType == eLEDTypes_Dream7) {
         for (auto& [key, pLedDisplay] : *pB2SData->GetLEDDisplays())
            pLedDisplay->SetVisible(false);
      }
      else if (selectedLEDType == eLEDTypes_Rendered) {
         for (auto& [key, pLED] : *pB2SData->GetLEDs())
            pLED->SetVisible(false);
      }
   }

   // maybe set a new timer interval
   SetInterval(GetBaseInterval() * (pB2SSettings->GetAllAnimationSlowDown() > 1 ? pB2SSettings->GetAllAnimationSlowDown() : (GetSlowDown() > 1 ? GetSlowDown() : 1)));

   // start the base timer
   m_loopticker = 0;
   m_ticker = 0;
   VP::Timer::Start();
   GetRunningAnimations()->Add(GetName());
}

void PictureBoxAnimation::Stop()
{
   // remove the start flag
   SetWouldBeStarted(false);

   // maybe stop the animation a bit later to do the last animation steps
   if (GetAnimationStopBehaviour() == eAnimationStopBehaviour_RunAnimationTillEnd || GetAnimationStopBehaviour() == eAnimationStopBehaviour_RunAnimationToFirstStep) {
      if (!IsStopMeLater())
         m_reachedThe0Point = false;
      SetStopMeLater(true);
      return;
   }

   // stop the base timer
   GetRunningAnimations()->Remove(GetName());
   VP::Timer::Stop();

   B2SData* pB2SData = B2SData::GetInstance();

   // maybe show score displays
   if (IsHideScoreDisplays()) {
      eLEDTypes selectedLEDType = GetLEDType();
      if (selectedLEDType == eLEDTypes_Dream7) {
         for (auto& [key, pLedDisplay] : *pB2SData->GetLEDDisplays())
            pLedDisplay->SetVisible(true);
      }
      else if (selectedLEDType == eLEDTypes_Rendered) {
         for (auto& [key, pLED] : *pB2SData->GetLEDs())
            pLED->SetVisible(true);
      }
   }

   // maybe unlock all illu
   if (IsLockInvolvedLamps()) {
      for (auto& groupname : m_lightsInvolved) {
         if (pB2SData->GetIlluminationLocks()->contains(groupname)) {
            if ((*pB2SData->GetIlluminationLocks())[groupname] > 1)
               (*pB2SData->GetIlluminationLocks())[groupname]--;
            else
               pB2SData->GetIlluminationLocks()->erase(groupname);
         }
      }
      pB2SData->SetUseIlluminationLocks(pB2SData->GetIlluminationLocks()->size() > 0);
   }

   // maybe switch all involved lights on/off or set some lights to initial state
   if (GetLightsStateAtAnimationEnd() == eLightsStateAtAnimationEnd_InvolvedLightsOff || GetLightsStateAtAnimationEnd() == eLightsStateAtAnimationEnd_InvolvedLightsOn) {
      for (auto& groupname : m_lightsInvolved)
         LightGroup(groupname, (GetLightsStateAtAnimationEnd() == eLightsStateAtAnimationEnd_InvolvedLightsOn));
   }
   else if (GetLightsStateAtAnimationEnd() == eLightsStateAtAnimationEnd_LightsReseted) {
      for (auto& [key, value] : m_lightsStateAtStartup)
         LightBulb(key, value);
      if (m_pMainFormBackgroundImage) {
         FormBackglass* pFormBackglass = dynamic_cast<FormBackglass*>(m_pForm);
         if (pFormBackglass)
            pFormBackglass->SetBackgroundImage(m_pMainFormBackgroundImage);
      }
   }
}

void PictureBoxAnimation::PictureBoxAnimationTick(VP::Timer* pTimer)
{
   // first of all stop the timer
   VP::Timer::Stop();

   // maybe get out since the timer is already stopped
   if (!GetRunningAnimations()->Contains(GetName()))
      return;

   B2SSettings* pB2SSettings = B2SSettings::GetInstance();

   // show animation stuff
   if (!m_entries.empty()) {
      while (true) {
         int index = !IsPlayReverse() ? m_ticker + 1 : m_entries.size() - m_ticker;
         if (m_entries.find(index) != m_entries.end()) {
            EntryAction* pCurrentEntryAction = m_entries[index];
            if (pCurrentEntryAction->GetCorrector() != 0 && IsPlayReverse())
               pCurrentEntryAction = m_entries[index + pCurrentEntryAction->GetCorrector()];
            // light or unlight bulbs
            for (auto& groupname : *pCurrentEntryAction->GetBulbs()) {
               if (!groupname.empty())
                  LightGroup(groupname, pCurrentEntryAction->IsVisible());
            }
            // maybe set switch
            if (pCurrentEntryAction->GetPulseSwitch() > 0)
               SetSwitch(pCurrentEntryAction->GetPulseSwitch());
            // set next interval
            if (pCurrentEntryAction->GetIntervalMultiplier() > 0) {
               SetInterval(pCurrentEntryAction->GetIntervalMultiplier() * GetBaseInterval() * (pB2SSettings->GetAllAnimationSlowDown() > 1 ? pB2SSettings->GetAllAnimationSlowDown() : (GetSlowDown() > 1 ? GetSlowDown() : 1)));
               break;
            }
            else {
               m_ticker++;
               if (m_ticker >= m_entries.size())
                  break;
            }
         }
         else
            break;
      }
   }

   // count on and maybe restart the timer
   bool restart = true;
   m_ticker++;
   if (m_ticker >= m_entries.size()) {
       m_reachedThe0Point = true;
       m_loopticker++;
       m_ticker = 0;
       if (GetLoops() > 0 && m_loopticker >= GetLoops()) {
           m_loopticker = 0;
           restart = false;
           RaiseFinishedEvent();
       }
   }
   if (restart) {
      if (pB2SSettings->GetAllAnimationSlowDown() == 0 || GetSlowDown() == 0)
         Stop();
      else if (IsStopMeLater() && GetAnimationStopBehaviour() == eAnimationStopBehaviour_RunAnimationTillEnd && m_ticker == 0)
         Stop();
      else if (IsStopMeLater() && GetAnimationStopBehaviour() == eAnimationStopBehaviour_RunAnimationToFirstStep && (m_ticker == 1 || m_ticker == 2) && m_reachedThe0Point)
         Stop();
      else
         VP::Timer::Start();
   }
   else
      Stop();
}

void PictureBoxAnimation::LightGroup(const string& szGroupName, bool visible)
{
   B2SData* pB2SData = B2SData::GetInstance();

   // only do the lightning stuff if the group has a name
   if (!szGroupName.empty() && pB2SData->GetIlluminationGroups()->contains(szGroupName)) {
      // get all matching picture boxes
      for (auto& pPicbox : (*pB2SData->GetIlluminationGroups())[szGroupName]) {
         pPicbox->SetVisible(visible);
         if (visible && IsBringToFront())
            pPicbox->BringToFront();
      }
   }
}

void PictureBoxAnimation::LightBulb(const string& szBulb, bool visible)
{
   // only do the lightning stuff if the bulb has a name
   if (!szBulb.empty()) {
      Form* pCurrentForm = NULL;
      if (m_pForm->GetControl(szBulb))
         pCurrentForm = m_pForm;
      else if (m_pFormDMD->GetControl(szBulb))
         pCurrentForm = m_pFormDMD;
      if (pCurrentForm) {
         pCurrentForm->GetControl(szBulb)->SetVisible(visible);
         if (visible && IsBringToFront())
            pCurrentForm->GetControl(szBulb)->BringToFront();
      }
   }
}

eLEDTypes PictureBoxAnimation::GetLEDType()
{
   B2SData* pB2SData = B2SData::GetInstance();

   eLEDTypes ret = eLEDTypes_Undefined;
   if (pB2SData->GetLEDDisplays()->size() > 0) {
      for (const auto& [key, pDisplay] : *pB2SData->GetLEDDisplays()) {
         if (pDisplay->IsVisible()) {
            ret = eLEDTypes_Dream7;
            break;
         }
      }
   }
   else if (pB2SData->GetLEDs()->size() > 0) {
      for (const auto& [key, pLED] : *pB2SData->GetLEDs()) {
         if (pLED->IsVisible()) {
            ret = eLEDTypes_Rendered;
            break;
         }
      }
   }
   return ret;
}