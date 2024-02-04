#pragma once

#include "../b2s_i.h"

#include "B2SBaseBox.h"
#include "../classes/Sound.h"
#include "../../common/Timer.h"

class B2SReelBox : public B2SBaseBox
{
public:
   B2SReelBox();
   virtual ~B2SReelBox();

   void OnPaint(VP::RendererGraphics* pGraphics) override;

   int GetSetID() const { return m_setID; }
   void SetSetID(int setID) { m_setID = setID; }
   string GetReelType() const { return m_szReelType; }
   void SetReelType(string reelType);
   string GetSoundName() const { return m_szSoundName; }
   void SetSoundName(string soundName) { m_szSoundName = soundName; }
   Sound* GetSound() const { return m_pSound; }
   void SetSound(Sound* pSound) { m_pSound = pSound; }
   eScoreType GetScoreType() const { return m_scoreType; }
   void SetScoreType(eScoreType scoreType) { m_scoreType = scoreType; }
   string GetGroupName() const { return m_szGroupName; }
   void SetGroupName(string groupName) { m_szGroupName = groupName; }
   bool IsIlluminated() const { return m_illuminated; }
   void SetIlluminated(bool illuminated);
   int GetValue() const { return m_value; }
   void SetValue(int value, bool refresh = false);
   void SetText(int text, bool animateReelChange = true);
   int GetCurrentText() const { return m_currentText; }
   int GetRollingInterval() const { return m_rollingInterval; }
   void SetRollingInterval(int rollingInterval);
   bool IsInReelRolling() { return m_intermediates2go <= 0; }
   bool IsInAction() const { return m_pTimer->IsEnabled(); }

private:
   void ReelAnimationTimerTick(VP::Timer* pTimer);
   string ConvertValue(int value);
   string ConvertText(int text);

   VP::Timer* m_pTimer;
   const int cTimerInterval = 101;
   bool m_led;
   int m_length;
   string m_initValue;
   string m_szReelIndex;
   int m_intermediates;
   int m_intermediates2go;
   int m_setID;
   string m_szReelType;
   string m_szSoundName;
   Sound* m_pSound;
   eScoreType m_scoreType;
   string m_szGroupName;
   bool m_illuminated;
   int m_value;
   int m_currentText;
   int m_text;
   int m_rollingInterval;

   int m_firstintermediatecount;
   B2SData* m_pB2SData;
};
