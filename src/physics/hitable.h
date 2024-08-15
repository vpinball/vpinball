// license:GPLv3+

#pragma once

#include "renderer/Renderable.h"

class Hitable : public Renderable
{
public:
   virtual ItemTypeEnum HitableGetItemType() const = 0;

   virtual void BeginPlay(vector<HitTimer*> &pvht) = 0;
   virtual void EndPlay() = 0;

   virtual EventProxyBase *GetEventProxyBase() = 0;

   virtual void PhysicSetup(class PhysicsEngine* physics, const bool isUI) = 0;
   virtual void PhysicRelease(class PhysicsEngine* physics, const bool isUI) = 0;
};
