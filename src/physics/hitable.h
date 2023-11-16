#pragma once

#include "renderer/Renderable.h"

class Hitable : public Renderable
{
public:
   virtual ItemTypeEnum HitableGetItemType() const = 0;

   virtual void GetTimers(vector<HitTimer*> &pvht) = 0;
   virtual EventProxyBase *GetEventProxyBase() = 0;

   // Physics support
   virtual void GetHitShapes(vector<HitObject*> &pvho) = 0;
   virtual void GetHitShapesDebug(vector<HitObject*> &pvho) = 0;
   virtual void EndPlay() = 0;
};
