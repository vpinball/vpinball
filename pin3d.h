enum
	{
	eLightProject1 = 1,
	ePictureTexture = 0,
	};

class Matrix3D : public D3DMATRIX
	{
public:
	void Multiply(Matrix3D &mult, Matrix3D &result);
	void RotateXMatrix(double x);
	void RotateYMatrix(double y);
	void RotateZMatrix(double z);
	void SetIdentity();
	void Invert();
	void MultiplyVector(float x, float y, float z, Vertex3D *pv3DOut);
	};

class PinProjection
	{
public:
	void Rotate(double x, double y, double z);
	void Translate(double x, double y, double z);
	void FitCameraToVertices(Vector<Vertex3D> *pvvertex3D, int cvert, double aspect, double rotation, double inclination, double FOV);
	void CacheTransform();
	void TransformVertices(Vertex3D* rgv, WORD *rgi, int count, Vertex3D *rgvout);
	void SetFieldOfView(double rFOV, double raspect, double rznear, double rzfar);

	Matrix3D m_matWorld;
	Matrix3D m_matView;
	Matrix3D m_matProj;
	Matrix3D m_matrixTotal;

	RECT m_rcviewport;

	double m_rznear, m_rzfar;
	Vertex3D m_vertexcamera;
	};

class Pin3D
	{
public:
	Pin3D();
	~Pin3D();

	HRESULT InitDD(HWND hwnd, BOOL fFullScreen, int screenwidth, int screenheight, int colordepth, int refreshrate);
	HRESULT Create3DDevice(GUID* pDeviceGUID);
	HRESULT CreateZBuffer(GUID* pDeviceGUID);

	void DrawBackground();

	void InitRenderState();

	void InitBackGraphics();

	void InitLayout(float left, float top, float right, float bottom, float inclination, float FOV);
	void SetFieldOfView(double rFOV, double raspect, double rznear, double rzfar);
	void Rotate(double x, double y, double z);
	void Translate(double x, double y, double z);
	void FitCameraToVertices(Vector<Vertex3D> *pvvertex3D, int cvert, double aspect, double rotation, double inclination, double FOV);
	void TransformVertices(Vertex3D* rgv, WORD *rgi, int count, Vertex3D *rgvout);
	void CacheTransform();
	LPDIRECTDRAWSURFACE7 CreateShadow(float height);

	void CreateBallShadow();

	void SetUpdatePos(int left, int top);
	void Flip(int offsetx, int offsety);

	void SetRenderTarget(LPDIRECTDRAWSURFACE7 pddsSurface, LPDIRECTDRAWSURFACE7 pddsZ);
	
	void SetTexture(LPDIRECTDRAWSURFACE7 pddsTexture);
	void EnableLightMap(BOOL fEnable, float z);
	
	void SetMaterial(float r, float g, float b, float a);
	void SetColorKeyEnabled(BOOL fColorKey);
	void SetAlphaEnabled(BOOL fAlpha);
	void SetFiltersLinear();

	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE d3dptPrimitiveType, DWORD  dwVertexTypeDesc,
												  LPVOID lpvVertices, DWORD dwVertexCount,
												  LPWORD lpwIndices, DWORD dwIndexCount);                     

	BOOL GetCacheFileName(char *szFileName);
	BOOL OpenCacheFileForRead();
	void OpenCacheFileForWrite();
	void WriteSurfaceToCacheFile(LPDIRECTDRAWSURFACE7 pdds);
	void WriteObjFrameToCacheFile(ObjFrame *pobjframe);
	void WriteAnimObjectToCacheFile(AnimObject *panimobj, Vector<ObjFrame> *pvobjframe);
	void WriteAnimObjectToCacheFile(AnimObject *panimobj, ObjFrame **rgobjframe, int count);
	void ReadSurfaceFromCacheFile(LPDIRECTDRAWSURFACE7 pdds);
	void ReadObjFrameFromCacheFile(ObjFrame *pobjframe);
	void ReadAnimObjectFromCacheFile(AnimObject *panimobj, Vector<ObjFrame> *pvobjframe);
	void ReadAnimObjectFromCacheFile(AnimObject *panimobj, ObjFrame **rgpobjframe, int count);
	void CloseCacheFile();
	
	// Handy functions for creating obj frames

	void ClearExtents(RECT *prc, float *pznear, float *pzfar);
	void ExpandExtents(RECT *prc, Vertex3D* rgv, float *pznear, float *pzfar, int count, BOOL fTransformed);
	void ExpandRectByRect(RECT *prc, RECT *prcNew);

	void ClipRectToVisibleArea(RECT *prc);
	void EnsureDebugTextures();

	LPDIRECTDRAWSURFACE7 CreateOffscreen(int width, int height);
	LPDIRECTDRAWSURFACE7 CreateOffscreenWithCustomTransparency(int width, int height, int color);
	LPDIRECTDRAWSURFACE7 CreateZBufferOffscreen(int width, int height);

	LPDIRECTDRAW7 m_pDD;
	LPDIRECTDRAWSURFACE7 m_pddsFrontBuffer;
	LPDIRECTDRAWSURFACE7 m_pddsBackBuffer;
	LPDIRECTDRAWSURFACE7 m_pddsZBuffer;
	LPDIRECT3D7 m_pD3D;
	LPDIRECT3DDEVICE7 m_pd3dDevice;

	LPDIRECTDRAWSURFACE7 m_pddsStatic;
	LPDIRECTDRAWSURFACE7 m_pddsStaticZ;

	LPDIRECTDRAWSURFACE7 m_pddsBallTexture;
	LPDIRECTDRAWSURFACE7 m_pddsShadowTexture;
	LPDIRECTDRAWSURFACE7 m_pddsLightTexture;
	LPDIRECTDRAWSURFACE7 m_pddsTargetTexture;
	//LPDIRECTDRAWSURFACE7 m_pddsPlayfieldTexture;
	//LPDIRECTDRAWSURFACE7 m_pddsLightProjectTexture;
	LPDIRECTDRAWSURFACE7 m_pddsLightWhite;

	ExVector<void> m_xvShadowMap;

	Matrix3D m_matrixTotal;

	RECT m_rcScreen;
	int m_dwRenderWidth;
	int m_dwRenderHeight;
	int m_dwViewPortWidth;
	int m_dwViewPortHeight;

	RECT m_rcUpdate;

	int m_width;
	int m_height;
	HWND m_hwnd;
	float m_maxtu;
	float m_maxtv;

	float m_rotation, m_inclination;

	double m_rznear, m_rzfar;
	Vertex3D m_vertexcamera;

	LightProjected m_lightproject;

	BOOL m_fSoftwareOnly;
	
	HANDLE m_hFileCache;
	BOOL m_fReadingFromCache;
	BOOL m_fWritingToCache;
	};