#pragma once
class Hitable
	{
public:
	virtual void GetHitShapes(Vector<HitObject> * const pvho) = 0;
	virtual void GetHitShapesDebug(Vector<HitObject> * const pvho) = 0;
	virtual void GetTimers(Vector<HitTimer> * const pvht) = 0;
	virtual EventProxyBase *GetEventProxyBase() = 0;
	virtual void EndPlay() = 0;
	virtual void RenderStatic( const RenderDevice* pd3dDevice) = 0;
	virtual void PostRenderStatic( const RenderDevice* pd3dDevice) = 0;
	virtual void RenderMovers( const RenderDevice* pd3dDevice) = 0;
   virtual void RenderSetup( const RenderDevice* _pd3dDevice) = 0;
	};
