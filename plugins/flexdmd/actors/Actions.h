#pragma once

#include "Actor.h"
#include "AnimatedActor.h"
#include "Group.h"
#include "ScriptablePlugin.h"

#include "tweeny-3.2.0.h"

class Action
{
public:
   virtual ~Action() { assert(m_refCount == 0); };

   PSC_IMPLEMENT_REFCOUNT()

   virtual bool Update(float secondsElapsed) = 0;
};


class AddChildAction final : public Action
{
public:
   AddChildAction(Group *pTarget, Actor *pChild, bool add)
      : m_pTarget(pTarget)
      , m_pChild(pChild)
      , m_add(add)
   {
   }
   ~AddChildAction() override = default;

   bool Update(float secondsElapsed) override
   {
      if (m_add)
         m_pTarget->AddActor(m_pChild);
      else
         m_pTarget->RemoveActor(m_pChild);

      return true;
   }

private:
   Group *m_pTarget;
   Actor *m_pChild;
   bool m_add;
};


class BlinkAction final : public Action
{
public:
   BlinkAction(Actor *pTarget, float secondsShow, float secondsHide, int repeat)
      : m_pTarget(pTarget)
      , m_secondsShow(secondsShow)
      , m_secondsHide(secondsHide)
      , m_repeat(repeat)
      , m_n(0)
      , m_time(0.f)
   {
   }
   ~BlinkAction() override = default;

   bool Update(float secondsElapsed) override
   {
      m_time += secondsElapsed;
      if (m_pTarget->GetVisible() && m_time > m_secondsShow)
      {
         m_time -= m_secondsShow;
         m_pTarget->SetVisible(false);
         m_n++;
         if (m_repeat >= 0 && m_n > m_repeat)
            return true;
      }
      else if (!m_pTarget->GetVisible() && m_time > m_secondsHide)
      {
         m_time -= m_secondsHide;
         m_pTarget->SetVisible(true);
      }
      return false;
   }

private:
   Actor *m_pTarget;
   float m_secondsShow;
   float m_secondsHide;
   int m_repeat;

   int m_n;
   float m_time;
};


class DelayedAction final : public Action
{
public:
   DelayedAction(float secondsToWait, Action *pAction)
      : m_secondsToWait(secondsToWait)
      , m_pAction(pAction)
      , m_time(0.0f)
   {
   }
   ~DelayedAction() override = default;

   bool Update(float secondsElapsed) override
   {
      m_time += m_secondsToWait;
      if (m_time >= m_secondsToWait && m_pAction->Update(secondsElapsed))
      {
         // Prepare for restart
         m_time = 0.0f;
         return true;
      }
      return false;
   }

private:
   float m_secondsToWait;
   Action *m_pAction;
   float m_time;
};


enum Interpolation
{
   Interpolation_Linear = 0,
   Interpolation_ElasticIn = 1,
   Interpolation_ElasticOut = 2,
   Interpolation_ElasticInOut = 3,
   Interpolation_QuadIn = 4,
   Interpolation_QuadOut = 5,
   Interpolation_QuadInOut = 6,
   Interpolation_CubeIn = 7,
   Interpolation_CubeOut = 8,
   Interpolation_CubeInOut = 9,
   Interpolation_QuartIn = 10,
   Interpolation_QuartOut = 11,
   Interpolation_QuartInOut = 12,
   Interpolation_QuintIn = 13,
   Interpolation_QuintOut = 14,
   Interpolation_QuintInOut = 15,
   Interpolation_SineIn = 16,
   Interpolation_SineOut = 17,
   Interpolation_SineInOut = 18,
   Interpolation_BounceIn = 19,
   Interpolation_BounceOut = 20,
   Interpolation_BounceInOut = 21,
   Interpolation_CircIn = 22,
   Interpolation_CircOut = 23,
   Interpolation_CircInOut = 24,
   Interpolation_ExpoIn = 25,
   Interpolation_ExpoOut = 26,
   Interpolation_ExpoInOut = 27,
   Interpolation_BackIn = 28,
   Interpolation_BackOut = 29,
   Interpolation_BackInOut = 30
};

class TweenAction : public Action
{
public:
   TweenAction(Actor *pTarget, float duration)
      : m_pTarget(pTarget)
      , m_duration(duration)
      , m_ease(Interpolation_Linear)
   {
   }
   ~TweenAction() override = default;

   Interpolation GetEase() const { return m_ease; }
   void SetEase(Interpolation ease) { m_ease = ease; }

   bool Update(float secondsElapsed) override
   {
      if (m_tweens.empty())
      {
         Begin();
         switch (m_ease)
         {
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
      for (auto &tween : m_tweens)
      {
         tween.step((int)(secondsElapsed * 1000.0f));
         if (tween.progress() < 1.0f)
         {
            done = false;
            break;
         }
      }
      if (done)
      {
         End();
         // Prepare for restart
         m_tweens.clear();
         return true;
      }
      return false;
   }

   virtual void Begin() = 0;
   virtual void End() {}

   Actor *GetTarget() const { return m_pTarget; }
   float GetDuration() const { return m_duration; }

   void AddTween(float from, float to, float duration, const std::function<bool(float)>& callback)
   {
      m_tweens.emplace_back(tweeny::tween<float>(tweeny::from(from).to(to).during(duration * 1000.0f).onStep(callback)));
   }

private:
   Actor *m_pTarget;
   float m_duration;
   Interpolation m_ease;

   vector<tweeny::tween<float>> m_tweens;
};


class MoveToAction final : public TweenAction
{
public:
   MoveToAction(Actor *pTarget, float x, float y, float duration)
      : TweenAction(pTarget, duration)
      , m_x(x)
      , m_y(y)
   {
   }
   ~MoveToAction() override = default;

   void Begin() override
   {
      Actor *actor = this->GetTarget();
      if (GetDuration() == 0.f)
      {
         actor->SetX(m_x);
         actor->SetY(m_y);
         return;
      }
      AddTween(actor->GetX(), m_x, GetDuration(),
         [actor](float newValue)
         {
            actor->SetX(newValue);
            return false;
         });
      AddTween(actor->GetY(), m_y, GetDuration(),
         [actor](float newValue)
         {
            actor->SetY(newValue);
            return false;
         });
   }

   bool Update(float secondsElapsed) override
   {
      return TweenAction::Update(secondsElapsed);
   }

private:
   float m_x;
   float m_y;
};


class ParallelAction final : public Action
{
public:
   ParallelAction() { }
   ~ParallelAction() override = default;

   ParallelAction* Add(Action* action)
   {
      m_actions.push_back(action);
      m_runMask.push_back(false);
      return this;
   }

   bool Update(float secondsElapsed) override
   {
      bool alive = false;
      const int a = (int)m_actions.size();
      for (int i = 0; i < a; i++)
      {
         if (m_runMask[i])
         {
            if (m_actions[i]->Update(secondsElapsed))
               m_runMask[i] = false;
            else
               alive = true;
         }
      }
      if (!alive)
      {
         for (int i = 0; i < a; i++)
            m_runMask[i] = true;
      }
      return false;
   }

private:
   vector<Action *> m_actions;
   vector<bool> m_runMask;
};


class RemoveFromParentAction final : public Action
{
public:
   RemoveFromParentAction(Actor *pTarget)
      : m_pTarget(pTarget)
   {
   }
   ~RemoveFromParentAction() override = default;

   bool Update(float secondsElapsed) override
   {
      m_pTarget->Remove();
      return true;
   }

private:
   Actor *m_pTarget;
};


class RepeatAction final : public Action
{
public:
   RepeatAction(Action *pAction, int count)
      : m_pAction(pAction)
      , m_count(count)
      , m_n(0)
   {
   }
   ~RepeatAction() override = default;

   bool Update(float secondsElapsed) override
   {
      if (m_pAction->Update(secondsElapsed))
      {
         m_n++;
         if (m_n == m_count)
         {
            // Prepare for restart
            m_n = 0;
            return true;
         }
      }
      return false;
   }

private:
   Action *m_pAction;
   int m_count;
   int m_n;
};


class SeekAction final : public Action
{
public:
   SeekAction(AnimatedActor *pTarget, float position)
      : m_pTarget(pTarget)
      , m_position(position)
   {
   }
   ~SeekAction() override = default;

   bool Update(float secondsElapsed) override
   {
      m_pTarget->Seek(m_position);
      return true;
   }

private:
   AnimatedActor *m_pTarget;
   float m_position;
};


class SequenceAction final : public Action
{
public:
   SequenceAction()
      : m_pos(0)
   {
   }
   ~SequenceAction() override = default;

   SequenceAction *Add(Action *action)
   {
      m_actions.push_back(action);
      return this;
   }

   bool Update(float secondsElapsed) override
   {
      const int a = (int)m_actions.size();
      if (m_pos >= a)
      {
         m_pos = 0;
         return true;
      }
      while (m_actions[m_pos]->Update(secondsElapsed))
      {
         m_pos++;
         if (m_pos >= a)
         {
            m_pos = 0;
            return true;
         }
      }
      return false;
   }

private:
   vector<Action *> m_actions;
   int m_pos;
};


class ShowAction final : public Action
{
public:
   ShowAction(Actor *pTarget, bool visible)
      : m_pTarget(pTarget)
      , m_visible(visible)
   {
   }
   ~ShowAction() override = default;

   bool Update(float secondsElapsed) override
   {
      m_pTarget->SetVisible(m_visible);
      return true;
   }

private:
   Actor *m_pTarget;
   bool m_visible;
};


class WaitAction final : public Action
{
public:
   WaitAction(float secondsToWait)
      : m_secondsToWait(secondsToWait)
      , m_time(0.0f)
   {
   }
   ~WaitAction() override = default;

   bool Update(float secondsElapsed) override
   {
      m_time += secondsElapsed;
      if (m_time >= m_secondsToWait)
      {
         // Prepare for restart
         m_time = 0.0f;
         return true;
      }
      return false;
   }

private:
   float m_secondsToWait;
   float m_time;
};


class ActionFactory final
{
public:
   ActionFactory(Actor *pTarget)
      : m_pTarget(pTarget)
   {
   }
   ~ActionFactory() = default;

   PSC_IMPLEMENT_REFCOUNT()

   WaitAction* Wait(float secondsToWait) const { return new WaitAction(secondsToWait); }
   DelayedAction* Delayed(float secondsToWait, Action* action) const { return new DelayedAction(secondsToWait, action); }
   ParallelAction* Parallel() const { return new ParallelAction(); }
   SequenceAction* Sequence() const { return new SequenceAction(); }
   RepeatAction* Repeat(Action* action, int count) const { return new RepeatAction(action, count); }
   BlinkAction* Blink(float secondsShow, float secondsHide, int repeat) { return new BlinkAction(m_pTarget, secondsShow, secondsHide, repeat); }
   ShowAction* Show(bool visible) const { return new ShowAction(m_pTarget, visible); }
   AddChildAction* AddTo(Group* parent) const { return new AddChildAction(parent, m_pTarget, true); }
   RemoveFromParentAction* RemoveFromParent() const { return new RemoveFromParentAction(m_pTarget); }
   AddChildAction* AddChild(Actor* child) const { return new AddChildAction(dynamic_cast<Group *>(m_pTarget), child, true); }
   AddChildAction *RemoveChild(Actor *child) const { return new AddChildAction(dynamic_cast<Group *>(m_pTarget), child, false); }
   SeekAction *Seek(float pos) const { return new SeekAction(dynamic_cast<AnimatedActor *>(m_pTarget), pos); }
   MoveToAction* MoveTo(float x, float y, float duration) { return new MoveToAction(m_pTarget, x, y, duration); }

private:
   Actor *m_pTarget;
};
