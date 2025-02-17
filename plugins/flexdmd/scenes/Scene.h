#pragma once

#include "actors/Group.h"
#include "AnimationType.h"

#include "tweeny-3.2.0.h"

class Scene : public Group
{
public:
   Scene(FlexDMD* pFlexDMD, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id);
   ~Scene();

   void UpdateState();
   virtual void Begin() { };
   float StartAnimation(AnimationType animation);
   bool IsFinished() const;
   void SetVisible(bool visible) override { Group::SetVisible(visible); UpdateState(); }
   void SetAnimateIn(AnimationType animateIn) { m_animateIn = animateIn; };
   void SetPause(float pauseS) { m_pause = pauseS; };
   float GetTime() const { return m_time; };
   void SetAnimateOut(AnimationType animateOut) { m_animateOut = animateOut; };
   void AddTween(float from, float to, float duration, const std::function<bool (float)>& callback);
   void Update(float secondsElapsed) override;
   void OnStageStateChanged() override;

private:
   float m_inAnimLength;
   float m_outAnimLength;
   AnimationType m_animateIn;
   AnimationType m_animateOut;
   bool m_active;

   float m_time;
   float m_pause;

   vector<tweeny::tween<float>> m_tweens;
};
