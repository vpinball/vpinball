#pragma once

class RenderDevice;

class Hitable
{
public:
   virtual void GetHitShapes(vector<HitObject*> &pvho) = 0;
   virtual void GetHitShapesDebug(vector<HitObject*> &pvho) = 0;
   virtual void GetTimers(vector<HitTimer*> &pvht) = 0;
   virtual EventProxyBase *GetEventProxyBase() = 0;

   virtual void EndPlay() = 0;

   virtual ItemTypeEnum HitableGetItemType() const = 0;

   // New rendering API (implementation in progress, the aim being to split its lifecycle from the other parts, for example for LiveUI update)
   virtual void RenderSetup(RenderDevice *device) { RenderSetup(); }
   virtual void Render(const unsigned int renderMask) { if ((renderMask & 1) == 0) RenderDynamic(); if ((renderMask & 2) == 0) RenderStatic();}
   virtual void RenderRelease() { }
   // Previous API (called by the new API while the transition is in progress)
   virtual void RenderSetup() = 0;
   virtual void UpdateAnimation(const float diff_time_msec) = 0;
   virtual void RenderStatic() = 0;
   virtual void RenderDynamic() = 0;
   virtual bool IsTransparent() const                     { return false; }
   virtual float GetDepth(const Vertex3Ds& viewDir) const { return 0.0f; }
};
