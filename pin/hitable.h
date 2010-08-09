#pragma once
class Hitable
	{
public:
	virtual void GetHitShapes(Vector<HitObject> *pvho) = 0;
	virtual void GetHitShapesDebug(Vector<HitObject> *pvho) = 0;
	virtual void GetTimers(Vector<HitTimer> *pvht) = 0;
	virtual EventProxyBase *GetEventProxyBase() = 0;
	virtual void EndPlay() = 0;
	virtual void RenderStatic(Pin3D *ppin3d) = 0;
	virtual void PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	virtual void RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	virtual void RenderMoversFromCache(Pin3D *ppin3d) = 0;
	};
