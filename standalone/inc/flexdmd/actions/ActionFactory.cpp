#include "stdafx.h"

#include "ActionFactory.h"
#include "WaitAction.h"
#include "DelayedAction.h"
#include "ParallelAction.h"
#include "SequenceAction.h"
#include "RepeatAction.h"
#include "BlinkAction.h"
#include "ShowAction.h"
#include "AddToAction.h"
#include "RemoveFromParentAction.h"
#include "AddChildAction.h"
#include "SeekAction.h"
#include "MoveToAction.h"

ActionFactory::ActionFactory(Actor* pTarget)
{
   m_pTarget = pTarget;
}

ActionFactory::~ActionFactory()
{
}

STDMETHODIMP ActionFactory::Wait(single secondsToWait, IUnknown **pRetVal)
{
   WaitAction* obj = new WaitAction(secondsToWait);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::Delayed(single secondsToWait, IUnknown *action, IUnknown **pRetVal)
{  
   DelayedAction* obj = new DelayedAction(secondsToWait, dynamic_cast<Action*>(action));
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::Parallel(ICompositeAction **pRetVal)
{
   ParallelAction* obj = new ParallelAction();
   return obj->QueryInterface(IID_ICompositeAction, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::Sequence(ICompositeAction **pRetVal)
{
   SequenceAction* obj = new SequenceAction();
   return obj->QueryInterface(IID_ICompositeAction, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::Repeat(IUnknown *action, LONG count, IUnknown **pRetVal)
{
   RepeatAction* obj = new RepeatAction(dynamic_cast<Action*>(action), count); 
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::Blink(single secondsShow, single secondsHide, LONG Repeat, IUnknown **pRetVal)
{
   BlinkAction* obj = new BlinkAction(m_pTarget, secondsShow, secondsHide, Repeat);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::Show(VARIANT_BOOL visible, IUnknown **pRetVal)
{
   ShowAction* obj = new ShowAction(m_pTarget, visible == VARIANT_TRUE);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::AddTo(IGroupActor *parent, IUnknown **pRetVal)
{
   AddToAction* obj = new AddToAction(m_pTarget, dynamic_cast<Group*>(parent), true);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::RemoveFromParent(IUnknown **pRetVal)
{
   RemoveFromParentAction* obj = new RemoveFromParentAction(m_pTarget);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::AddChild(IUnknown *child, IUnknown **pRetVal)
{
   AddChildAction* obj = new AddChildAction((Group*)m_pTarget, dynamic_cast<Actor*>(child), true);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::RemoveChild(IUnknown *child, IUnknown **pRetVal)
{
   AddChildAction* obj = new AddChildAction((Group*)m_pTarget, dynamic_cast<Actor*>(child), false);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::Seek(single pos, IUnknown **pRetVal)
{
   SeekAction* obj = new SeekAction((AnimatedActor*)m_pTarget, pos);
   return obj->QueryInterface(IID_IUnknown, (void**)pRetVal);
}

STDMETHODIMP ActionFactory::MoveTo(single x, single y, single duration, ITweenAction **pRetVal)
{
   MoveToAction* obj = new MoveToAction(m_pTarget, x, y, duration);
   return obj->QueryInterface(IID_ITweenAction, (void**)pRetVal);
}
