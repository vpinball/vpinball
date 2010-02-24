
class Hitable
{

public:
	Hitable ( void );
	~Hitable ( void );

	virtual void GetHitShapes(Vector<HitObject> *pvho) = 0;
	virtual void GetHitShapesDebug(Vector<HitObject> *pvho) = 0;
	virtual void GetTimers(Vector<HitTimer> *pvht) = 0;
	virtual EventProxyBase *GetEventProxyBase() = 0;
	virtual void EndPlay() = 0;
	virtual void RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	virtual void PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	virtual void RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice) = 0;
	virtual void RenderMoversFromCache(Pin3D *ppin3d) = 0;

	// Support for real-time 3D rendering.
    virtual void UpdateRealTime ( void ) = 0;								
	virtual void RenderRealTime ( void ) = 0;								

	PinImage *m_pTextureImage;
	PinModel *m_pVisualModel;
	char m_szTextureImage[MAXTOKEN];
	char m_szVisualModel[MAXTOKEN];

};
