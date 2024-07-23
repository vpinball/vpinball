#include "stdafx.h"
#include "MoveToAction.h"

MoveToAction::MoveToAction(Actor* pTarget, float x, float y, float duration) : TweenAction::TweenAction(pTarget, duration)
{
   m_x = x;
   m_y = y;
}

MoveToAction::~MoveToAction()
{
}

void MoveToAction::Begin()
{
   Actor* actor = this->GetTarget();

   if (GetDuration() == 0) {
      actor->SetX(m_x);
      actor->SetY(m_y);

      return;
   }

   AddTween(actor->GetX(), m_x, GetDuration(), [actor](float newValue) {
      actor->SetX(newValue);
      return false;
   });

   AddTween(actor->GetY(), m_y, GetDuration(), [actor](float newValue) {
      actor->SetY(newValue);
      return false;
   });
}