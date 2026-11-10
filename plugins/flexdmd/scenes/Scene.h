#pragma once

#include "actors/Group.h"
#include "AnimationType.h"

#include "actors/Tween.h"

namespace Flex {

class FadeOverlay;

class Scene : public Group
{
public:
   Scene(FlexDMD* pFlexDMD, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id);
   ~Scene() override;

   void UpdateState();
   virtual void Begin() { }
   float StartAnimation(AnimationType animation);
   bool IsFinished() const;
   void SetVisible(bool visible) override { Group::SetVisible(visible); UpdateState(); }
   void SetAnimateIn(AnimationType animateIn) { m_animateIn = animateIn; }
   void SetPause(float pauseS) { m_pause = pauseS; }
   float GetTime() const { return m_time; }
   void SetAnimateOut(AnimationType animateOut) { m_animateOut = animateOut; }
   void Update(float secondsElapsed) override;
   void OnStageStateChanged() override;

protected:
   // Linear animations, as the Glide tweens of the original C# FlexDMD
   enum class TweenProperty { X, Y };
   void AddTween(Actor *pActor, TweenProperty property, float to, float duration);
   void AddTween(FadeOverlay *pFade, float to, float duration); // Alpha of a fade overlay
   void AddTween(float *pValue, float to, float duration); // Plain value, e.g. a scroll offset

private:
   float m_inAnimLength;
   float m_outAnimLength;
   AnimationType m_animateIn;
   AnimationType m_animateOut;
   bool m_active;

   float m_time;
   float m_pause;

   enum class TweenTarget { X, Y, Alpha, Value };
   struct TargetTween
   {
      Tween tween;
      TweenTarget target;
      Actor *pActor; // X, Y, Alpha (a FadeOverlay)
      float *pValue; // Value
      float from, to;
   };
   void AddTween(TweenTarget target, Actor *pActor, float *pValue, float from, float to, float duration);

   vector<TargetTween> m_tweens;
};

}