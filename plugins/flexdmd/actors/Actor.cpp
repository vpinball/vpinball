#include "Actor.h"
#include "Actions.h"
#include "FlexDMD.h"

#include <cassert>
#include <SDL3/SDL_surface.h>

namespace Flex {

Actor::Actor(const FlexDMD *pFlexDMD, const string& name)
   : m_pFlexDMD(pFlexDMD)
   , m_name(name)
{
   m_pActionFactory = new ActionFactory(this);
}

Actor::~Actor()
{
   assert(m_refCount == 0);
   ClearActions();
   m_pActionFactory->Release();
}

ActionFactory* Actor::GetActionFactory() const
{
   m_pActionFactory->AddRef();
   return m_pActionFactory;
}

void Actor::AddAction(Action* action)
{
   action->AddRef();
   m_actions.push_back(action);
}

void Actor::ClearActions()
{
   for (auto action : m_actions)
      action->Release();
   m_actions.clear();
}

void Actor::Remove()
{
   if (m_parent != nullptr)
      m_parent->RemoveActor(this);
}

void Actor::Update(float secondsElapsed)
{
   if (!m_onStage)
      assert("Update was called on an actor which is not on stage.");
   for (auto it = m_actions.begin(); it != m_actions.end();)
   {
      if ((*it)->Update(secondsElapsed))
      {
         (*it)->Release();
         it = m_actions.erase(it);
      }
      else
         ++it;
   }
   if (m_fillParent && m_parent != nullptr)
      SetBounds(0, 0, m_parent->m_width, m_parent->m_height);
}

void Actor::Draw(Flex::SurfaceGraphics* pGraphics)
{
   if (!m_onStage)
      assert("Update was called on an actor which is not on stage.");
   if (m_visible && m_clearBackground)
   {
      pGraphics->SetColor(RGB(0, 0, 0));
      pGraphics->FillRectangle({ (int)m_x, (int)m_y, (int)m_width, (int)m_height });
   }
}

}