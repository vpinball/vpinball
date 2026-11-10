#include "Scene.h"
#include "actors/FadeOverlay.h"
#include "common.h"

// Animation started on a target that is still animated (e.g. scroll in and out on X with a short pause):
// 1 (default) = original C# FlexDMD (Glide 'overwrite'): the new animation cancels the running one
// 0 = earlier versions of this plugin: both run, the newest is applied last each frame
#ifndef FLEXDMD_TWEEN_OVERWRITE
 #define FLEXDMD_TWEEN_OVERWRITE 1
#endif

namespace Flex {

Scene::Scene(FlexDMD* pFlexDMD, AnimationType animateIn, float pauseS, AnimationType animateOut, const string& id) : Group(pFlexDMD, id)
{
   SetVisible(true);
   m_active = false;
   m_time = 0;
   m_animateIn = animateIn;
   m_animateOut = animateOut;
   m_pause = pauseS;
}

Scene::~Scene()
{
   m_tweens.clear();
}

void Scene::OnStageStateChanged()
{
   Group::OnStageStateChanged();
   UpdateState();
}

void Scene::UpdateState()
{
   bool shouldBeActive = GetVisible() && GetOnStage();
   if (shouldBeActive && !m_active) {
      m_active = true;
      SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());
      m_inAnimLength = StartAnimation(m_animateIn);
      m_outAnimLength = -1;
      Begin();
   }
   else if (!shouldBeActive && m_active) {
      m_active = false;
      m_tweens.clear();
   }
}

float Scene::StartAnimation(AnimationType animation)
{
   float alphaLength = 0.5f;
   float scrollWLength = 0.5f;
   float scrollHLength = scrollWLength * (float)GetHeight() / (float)GetWidth();
   // TODO Missing animations: ZoomIn = 2, ZoomOut = 3
   switch (animation) {
      case AnimationType_FadeIn:
      {
         FadeOverlay* pFade = new FadeOverlay(GetFlexDMD(), string());
         AddActor(pFade);
         pFade->SetAlpha(1.0f);
         pFade->SetColor(RGB(0, 0, 0));
         AddTween(pFade, 0.f, alphaLength);
         return alphaLength;
      }
      case AnimationType_FadeOut:
      {
         FadeOverlay* pFade = new FadeOverlay(GetFlexDMD(), string());
         AddActor(pFade);
         pFade->SetAlpha(0.0f);
         pFade->SetColor(RGB(0, 0, 0));
         AddTween(pFade, 1.0f, alphaLength);
         return alphaLength;
      }
      case AnimationType_ScrollOffLeft:
      {
         SetX(0);
         AddTween(this, TweenProperty::X, (float)-GetWidth(), scrollWLength);
         return scrollWLength;
      }
      case AnimationType_ScrollOffRight:
      {
         SetX(0);
         AddTween(this, TweenProperty::X, (float)GetWidth(), scrollWLength);
         return scrollWLength;
      }
      case AnimationType_ScrollOnLeft:
      {
         SetX((float)GetWidth());
         AddTween(this, TweenProperty::X, 0.0f, scrollWLength);
         return scrollWLength;
      }
      case AnimationType_ScrollOnRight:
      {
         SetX((float)-GetWidth());
         AddTween(this, TweenProperty::X, 0.0f, scrollWLength);
         return scrollWLength;
      }
      case AnimationType_ScrollOffUp:
      {
         SetY(0);
         AddTween(this, TweenProperty::Y, (float)-GetHeight(), scrollHLength);
         return scrollHLength;
      }
      case AnimationType_ScrollOffDown:
      {
         SetY(0);
         AddTween(this, TweenProperty::Y, (float)GetHeight(), scrollHLength);
         return scrollHLength;
      }
      case AnimationType_ScrollOnUp:
      {
         SetY((float)GetHeight());
         AddTween(this, TweenProperty::Y, 0.0f, scrollHLength);
         return scrollHLength;
      }
      case AnimationType_ScrollOnDown:
      {
         SetY((float)-GetHeight());
         AddTween(this, TweenProperty::Y, 0.0f, scrollHLength);
         return scrollHLength;
      }
      case AnimationType_FillFadeIn:
      {
         FadeOverlay* pFade = new FadeOverlay(GetFlexDMD(), string());
         pFade->SetAlpha(1.0f);
         pFade->SetColor(RGB(0, 0, 0));
         AddActor(pFade);

         pFade = new FadeOverlay(GetFlexDMD(), string());
         pFade->SetAlpha(0.0f);
         pFade->SetColor(RGB(255, 255, 255));
         AddActor(pFade);
         AddTween(pFade, 1.0f, alphaLength);
         return alphaLength;
      }
      case AnimationType_FillFadeOut:
      {
         FadeOverlay* pFade = new FadeOverlay(GetFlexDMD(), string());
         pFade->SetAlpha(1.0f);
         pFade->SetColor(RGB(255, 255, 255));
         AddActor(pFade);
         pFade = new FadeOverlay(GetFlexDMD(), string());
         pFade->SetAlpha(0.0f);
         pFade->SetColor(RGB(0, 0, 0));
         AddActor(pFade);
         AddTween(pFade, 1.0f, alphaLength);
        return alphaLength;
      }
      case AnimationType_None:
         return 0.0f;
      default:
         LOGE("Unsupported animation in scene '" + GetName() + "': " + std::to_string(animation));
         return 0.0f;
   }
}

void Scene::Update(float secondsElapsed)
{
   Group::Update(secondsElapsed);
   m_time += secondsElapsed;
   if (m_pause >= 0.0f && m_outAnimLength < 0.f && m_time >= m_inAnimLength + m_pause)
      m_outAnimLength = StartAnimation(m_animateOut);

   for (auto it = m_tweens.begin(); it != m_tweens.end();)
   {
      if (it->tween.Update(secondsElapsed))
      {
         const float value = it->tween.Lerp(it->from, it->to);
         switch (it->target)
         {
         case TweenTarget::X: it->pActor->SetX(value); break;
         case TweenTarget::Y: it->pActor->SetY(value); break;
         case TweenTarget::Alpha: static_cast<FadeOverlay *>(it->pActor)->SetAlpha(value); break;
         case TweenTarget::Value: *it->pValue = value; break;
         }
      }
      if (it->tween.IsDone())
         it = m_tweens.erase(it);
      else
         ++it;
   }
}

bool Scene::IsFinished() const
{
   return m_pause >= 0.0f && m_outAnimLength >= 0.f && m_time >= m_inAnimLength + m_pause + m_outAnimLength;
}

void Scene::AddTween(Actor *pActor, TweenProperty property, float to, float duration)
{
   if (property == TweenProperty::X)
      AddTween(TweenTarget::X, pActor, nullptr, pActor->GetX(), to, duration);
   else
      AddTween(TweenTarget::Y, pActor, nullptr, pActor->GetY(), to, duration);
}

void Scene::AddTween(FadeOverlay *pFade, float to, float duration)
{
   AddTween(TweenTarget::Alpha, pFade, nullptr, pFade->GetAlpha(), to, duration);
}

void Scene::AddTween(float *pValue, float to, float duration)
{
   AddTween(TweenTarget::Value, nullptr, pValue, *pValue, to, duration);
}

void Scene::AddTween(TweenTarget target, Actor *pActor, float *pValue, float from, float to, float duration)
{
#if FLEXDMD_TWEEN_OVERWRITE
   // Glide 'overwrite': replace running tweens on the same target
   std::erase_if(m_tweens, [target, pActor, pValue](const TargetTween &t) { return t.target == target && t.pActor == pActor && t.pValue == pValue; });
#endif
   m_tweens.push_back({ Tween(duration), target, pActor, pValue, from, to });
}

}
