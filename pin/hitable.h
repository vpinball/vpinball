#pragma once

class Hitable
{
public:
   virtual void GetHitShapes(vector<HitObject*> &pvho) = 0;
   virtual void GetHitShapesDebug(vector<HitObject*> &pvho) = 0;
   virtual void GetTimers(vector<HitTimer*> &pvht) = 0;
   virtual EventProxyBase *GetEventProxyBase() = 0;

   virtual void RenderSetup() = 0;
   virtual void UpdateAnimation(float diff_time_msec) = 0;
   virtual void RenderStatic() = 0;
   virtual void RenderDynamic() = 0;
   virtual void EndPlay() = 0;

   virtual ItemTypeEnum HitableGetItemType() const = 0;

   virtual bool IsTransparent() const                     { return false; }
   virtual float GetDepth(const Vertex3Ds& viewDir) const { return 0.0f; }
   virtual unsigned long long GetMaterialID() const       { return 0; }
   virtual unsigned long long GetImageID() const          { return 0; }
   virtual bool RenderToLightBuffer() const               { return false; }
   virtual bool IsDMD() const                             { return false; }
};
