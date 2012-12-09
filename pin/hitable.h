#pragma once
class Hitable
	{
public:
	virtual void GetHitShapes(Vector<HitObject> * const pvho) = 0;
	virtual void GetHitShapesDebug(Vector<HitObject> * const pvho) = 0;
	virtual void GetTimers(Vector<HitTimer> * const pvht) = 0;
	virtual EventProxyBase *GetEventProxyBase() = 0;
	virtual void EndPlay() = 0;
	virtual void RenderStatic(const LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	virtual void PostRenderStatic(const LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	virtual void RenderMovers(const LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	};
