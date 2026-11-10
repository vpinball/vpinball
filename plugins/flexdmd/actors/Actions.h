#pragma once

#include "Actor.h"
#include "AnimatedActor.h"
#include "Group.h"
#include "plugins/ScriptablePlugin.h"

#include "Tween.h"

#include <optional>

namespace Flex {

class Action
{
public:
   virtual ~Action() { assert(m_refCount == 0); }

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
      m_pChild->AddRef();
   }
   ~AddChildAction() override
   {
      m_pChild->Release();
   }

   bool Update(float secondsElapsed) override
   {
      if (m_add)
         m_pTarget->AddActor(m_pChild);
      else
         m_pTarget->RemoveActor(m_pChild);

      return true;
   }

private:
   Group *m_pTarget; // Not reference counted to avoid circular references
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
   Actor *m_pTarget; // Not reference counted to avoid circular references
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
      m_pAction->AddRef();
   }
   ~DelayedAction() override
   {
      m_pAction->Release();
   }

   bool Update(float secondsElapsed) override
   {
      m_time += secondsElapsed;
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

// Moves X and Y together with one tween, as in the original C# FlexDMD
class MoveToAction final : public Action
{
public:
   MoveToAction(Actor *pTarget, float x, float y, float duration)
      : m_pTarget(pTarget)
      , m_x(x)
      , m_y(y)
      , m_duration(duration)
   {
   }

   Interpolation GetEase() const { return m_ease; }
   void SetEase(Interpolation ease) { m_ease = ease; }

   bool Update(float secondsElapsed) override
   {
      if (!m_tween)
      {
         m_fromX = m_pTarget->GetX();
         m_fromY = m_pTarget->GetY();
         // Indexed by Interpolation. Unknown values stay linear, as in C# FlexDMD
         static constexpr Ease::Func easings[] = { Ease::Linear, Ease::ElasticIn, Ease::ElasticOut, Ease::ElasticInOut, //
            Ease::QuadIn, Ease::QuadOut, Ease::QuadInOut, Ease::CubeIn, Ease::CubeOut, Ease::CubeInOut, //
            Ease::QuartIn, Ease::QuartOut, Ease::QuartInOut, Ease::QuintIn, Ease::QuintOut, Ease::QuintInOut, //
            Ease::SineIn, Ease::SineOut, Ease::SineInOut, Ease::BounceIn, Ease::BounceOut, Ease::BounceInOut, //
            Ease::CircIn, Ease::CircOut, Ease::CircInOut, Ease::ExpoIn, Ease::ExpoOut, Ease::ExpoInOut, //
            Ease::BackIn, Ease::BackOut, Ease::BackInOut };
         static_assert(std::size(easings) == Interpolation_BackInOut + 1, "Easing table must match the Interpolation enum");
         m_tween.emplace(m_duration, (m_ease >= 0 && m_ease < static_cast<int>(std::size(easings))) ? easings[m_ease] : Ease::Linear);
      }
      if (m_tween->Update(secondsElapsed))
      {
         m_pTarget->SetX(m_tween->Lerp(m_fromX, m_x));
         m_pTarget->SetY(m_tween->Lerp(m_fromY, m_y));
      }
      if (m_tween->IsDone())
      {
         // Prepare for restart
         m_tween.reset();
         return true;
      }
      return false;
   }

private:
   Actor *m_pTarget; // Not reference counted to avoid circular references
   float m_x;
   float m_y;
   float m_duration;
   Interpolation m_ease = Interpolation_Linear;

   std::optional<Tween> m_tween;
   float m_fromX = 0.f;
   float m_fromY = 0.f;
};


class ParallelAction final : public Action
{
public:
   ParallelAction() { }
   ~ParallelAction() override
   {
      for (auto action : m_actions)
         action->Release();
   }

   ParallelAction* Add(Action* action)
   {
      action->AddRef();
      m_actions.push_back(action);
      m_runMask.push_back(false);
      return this;
   }

   bool Update(float secondsElapsed) override
   {
      bool alive = false;
      const size_t a = m_actions.size();
      for (size_t i = 0; i < a; i++)
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
         for (size_t i = 0; i < a; i++)
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

   bool Update(float secondsElapsed) override
   {
      m_pTarget->Remove();
      return true;
   }

private:
   Actor *m_pTarget; // Not reference counted to avoid circular references
};


class RepeatAction final : public Action
{
public:
   RepeatAction(Action *pAction, int count)
      : m_pAction(pAction)
      , m_count(count)
      , m_n(0)
   {
      m_pAction->AddRef();
   }
   ~RepeatAction() override
   {
      m_pAction->Release();
   }

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

   bool Update(float secondsElapsed) override
   {
      m_pTarget->Seek(m_position);
      return true;
   }

private:
   AnimatedActor *m_pTarget; // Not reference counted to avoid circular references
   float m_position;
};


class SequenceAction final : public Action
{
public:
   SequenceAction()
      : m_pos(0)
   {
   }
   ~SequenceAction() override
   {
      for (auto action : m_actions)
         action->Release();
   }

   SequenceAction *Add(Action *action)
   {
      action->AddRef();
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

   bool Update(float secondsElapsed) override
   {
      m_pTarget->SetVisible(m_visible);
      return true;
   }

private:
   Actor *m_pTarget; // Not reference counted to avoid circular references
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
   ~ActionFactory()
   {
      assert(m_refCount == 0);
   }

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
   Actor *m_pTarget; // Not reference counted to avoid circular references
};

}
