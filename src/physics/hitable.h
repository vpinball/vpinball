// license:GPLv3+

#pragma once

#include "renderer/Renderable.h"

class Hitable : public Renderable
{
public:
   virtual ItemTypeEnum HitableGetItemType() const = 0;

   virtual void PhysicSetup(class PhysicsEngine* physics, const bool isUI) = 0;
   virtual bool PhysicUpdate(class PhysicsEngine* physics, const bool isUI) { return false; } // Return false if update was not performed (object will be released and setup from scratch)
   virtual void PhysicRelease(class PhysicsEngine* physics, const bool isUI) = 0;

   virtual void TimerSetup(vector<HitTimer*>& pvht) = 0;
   virtual void TimerRelease() = 0;

   virtual EventProxyBase* GetEventProxyBase() = 0;
};
