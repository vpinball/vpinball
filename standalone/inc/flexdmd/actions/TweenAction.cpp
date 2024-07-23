#include "stdafx.h"
#include "TweenAction.h"

TweenAction::TweenAction(Actor* pTarget, float duration)
{
   m_pTarget = pTarget;
   m_duration = duration;

   m_ease = Interpolation_Linear;
}

TweenAction::~TweenAction()
{
}

STDMETHODIMP TweenAction::get_Ease(Interpolation *pRetVal)
{
   *pRetVal = m_ease;

   return S_OK;
}

STDMETHODIMP TweenAction::put_Ease(Interpolation pRetVal)
{
   m_ease = pRetVal;

   return S_OK;
}

bool TweenAction::Update(float secondsElapsed) 
{
   if (m_tweens.size() == 0) {
      Begin();
      switch (m_ease) {
         case Interpolation_Linear:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::linear);
            break;
         case Interpolation_ElasticIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::elasticIn);
            break;
         case Interpolation_ElasticOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::elasticOut);
            break;
         case Interpolation_ElasticInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::elasticInOut);
            break;
         case Interpolation_QuadIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quadraticIn);
            break;
         case Interpolation_QuadOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quadraticOut);
            break;
         case Interpolation_QuadInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quadraticInOut);
            break;
         case Interpolation_CubeIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::cubicIn);
            break;
         case Interpolation_CubeOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::cubicOut);
            break;
         case Interpolation_CubeInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::cubicInOut);
            break;
         case Interpolation_QuartIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quarticIn);
            break;
         case Interpolation_QuartOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quarticOut);
            break;
         case Interpolation_QuartInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quarticInOut);
            break;
         case Interpolation_QuintIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quinticIn);
            break;
         case Interpolation_QuintOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quinticOut);
            break;
         case Interpolation_QuintInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::quinticInOut);
            break;
         case Interpolation_SineIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::sinusoidalIn);
            break;
         case Interpolation_SineOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::sinusoidalOut);
            break;
         case Interpolation_SineInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::sinusoidalInOut);
            break;
         case Interpolation_BounceIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::bounceIn);
            break;
         case Interpolation_BounceOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::bounceOut);
            break;
         case Interpolation_BounceInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::bounceInOut);
            break;
         case Interpolation_CircIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::circularIn);
            break;
         case Interpolation_CircOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::circularOut);
            break;
         case Interpolation_CircInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::circularInOut);
            break;
         case Interpolation_ExpoIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::exponentialIn);
            break;
         case Interpolation_ExpoOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::exponentialOut);
            break;
         case Interpolation_ExpoInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::exponentialInOut);
            break;
         case Interpolation_BackIn:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::backIn);
            break;
         case Interpolation_BackOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::backOut);
            break;
         case Interpolation_BackInOut:
            for (auto &tween : m_tweens)
               tween.via(tweeny::easing::backInOut);
            break;
      }
   }
   bool done = true;
   for (auto &tween : m_tweens) {
      tween.step((int)(secondsElapsed * 1000.0f));
      if (tween.progress() < 1.0f) {
         done = false;
         break;
      }
   }
   if (done) {
      End();
      // Prepare for restart
      m_tweens.clear();
      return true;
   }
   return false;
}

tweeny::tween<float>& TweenAction::AddTween(float from, float to, float duration, std::function<bool (float)> callback)
{
   m_tweens.emplace_back(tweeny::tween<float>(tweeny::from(from).to(to).during((int)(duration * 1000.0f)).onStep(callback)));
   return m_tweens.back();
}