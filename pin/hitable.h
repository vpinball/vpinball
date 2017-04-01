#pragma once

class Hitable
{
public:
   virtual void GetHitShapes(Vector<HitObject> * const pvho) = 0;
   virtual void GetHitShapesDebug(Vector<HitObject> * const pvho) = 0;
   virtual void GetTimers(Vector<HitTimer> * const pvht) = 0;
   virtual EventProxyBase *GetEventProxyBase() = 0;
   virtual void EndPlay() = 0;
   virtual void PreRenderStatic(RenderDevice* pd3dDevice) { }
   virtual void RenderStatic(RenderDevice* pd3dDevice) = 0;
   virtual void PostRenderStatic(RenderDevice* pd3dDevice) = 0;
   virtual void RenderSetup(RenderDevice* pd3dDevice) = 0;

   virtual bool IsTransparent()                        { return false; }
   virtual float GetDepth(const Vertex3Ds& viewDir)    { return 0.0f; }
   virtual unsigned long long GetMaterialID()          { return 0; }
   virtual unsigned long long GetImageID()             { return 0; }
   virtual bool RenderToLightBuffer()                  { return false; }
   virtual bool IsDMD()                                { return false; }
};
