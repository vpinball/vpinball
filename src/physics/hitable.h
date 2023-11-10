#pragma once

class RenderDevice;

class Hitable
{
public:
   virtual ItemTypeEnum HitableGetItemType() const = 0;

   virtual void GetTimers(vector<HitTimer*> &pvht) = 0;
   virtual EventProxyBase *GetEventProxyBase() = 0;

   // Physics support
   virtual void GetHitShapes(vector<HitObject*> &pvho) = 0;
   virtual void GetHitShapesDebug(vector<HitObject*> &pvho) = 0;
   virtual void EndPlay() = 0;

   // Rendering support
   virtual void RenderSetup(RenderDevice *device) = 0;
   virtual void UpdateAnimation(const float diff_time_msec) = 0;
   virtual void Render(const unsigned int renderMask) = 0;
   virtual void RenderRelease() = 0;
   virtual float GetDepth(const Vertex3Ds& viewDir) const { return 0.0f; }
};
