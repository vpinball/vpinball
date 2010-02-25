#pragma once
enum
{
	eLightProject1 = 1,
	ePictureTexture = 0,
};

enum
{
	TEXTURE_MODE_POINT,				// Point sampled (aka no) texture filtering.
	TEXTURE_MODE_BILINEAR,			// Bilinar texture filtering. 
	TEXTURE_MODE_TRILINEAR,			// Trilinar texture filtering. 
	TEXTURE_MODE_ANISOTROPIC		// Anisotropic texture filtering. 
};


class Matrix3D : public D3DMATRIX
	{
public:
	void Multiply(Matrix3D &mult, Matrix3D &result);
	void RotateXMatrix(GPINFLOAT x);
	void RotateYMatrix(GPINFLOAT y);
	void RotateZMatrix(GPINFLOAT z);
	void SetIdentity();
	void Scale( float x, float y, float z );
	void Invert();
	void MultiplyVector(float x, float y, float z, Vertex3D *pv3DOut);
	};

class PinProjection
	{
public:
	void Rotate(GPINFLOAT x, GPINFLOAT y, GPINFLOAT z);
	void Translate(GPINFLOAT x, GPINFLOAT y, GPINFLOAT z);
	void FitCameraToVertices(Vector<Vertex3D> *pvvertex3D, int cvert, GPINFLOAT aspect, GPINFLOAT rotation, GPINFLOAT inclination, GPINFLOAT FOV);
	void CacheTransform();
	void TransformVertices(Vertex3D* rgv, WORD *rgi, int count, Vertex3D *rgvout);
	void SetFieldOfView(GPINFLOAT rFOV, GPINFLOAT raspect, GPINFLOAT rznear, GPINFLOAT rzfar);

	Matrix3D m_matWorld;
	Matrix3D m_matView;
	Matrix3D m_matProj;
	Matrix3D m_matrixTotal;

	RECT m_rcviewport;

	GPINFLOAT m_rznear, m_rzfar;
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

	void InitLayout(float left, float top, float right, float bottom, float inclination, float FOV, float rotation, float scalex, float scaley, float xlatex, float xlatey);
	void SetFieldOfView(GPINFLOAT rFOV, GPINFLOAT raspect, GPINFLOAT rznear, GPINFLOAT rzfar);
	void Identity(void);
	void Rotate(GPINFLOAT x, GPINFLOAT y, GPINFLOAT z);
	void Scale( float x, float y, float z );
	void Translate(GPINFLOAT x, GPINFLOAT y, GPINFLOAT z);
	void FitCameraToVertices(Vector<Vertex3D> *pvvertex3D, int cvert, GPINFLOAT aspect, GPINFLOAT rotation, GPINFLOAT inclination, GPINFLOAT FOV);
	void TransformVertices(Vertex3D* rgv, WORD *rgi, int count, Vertex3D *rgvout);
	void CacheTransform();
	LPDIRECTDRAWSURFACE7 CreateShadow(float height);

	void CreateBallShadow();

	void SetUpdatePos(int left, int top);
	void Flip(int offsetx, int offsety);

	void SetRenderTarget(LPDIRECTDRAWSURFACE7 pddsSurface, LPDIRECTDRAWSURFACE7 pddsZ);
	void SetTextureFilter( int TextureNum, int Mode );
	
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
	void DeleteCacheFileName();
	
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

	LPDIRECTDRAWSURFACE7 m_pddsBackTextureBuffer;		
	LPDIRECTDRAWSURFACE7 m_pddsZTextureBuffer;

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

	BOOL m_fFlatRot;
	float m_scalex, m_scaley;
	float m_xlatex, m_xlatey;

	GPINFLOAT m_rznear, m_rzfar;
	Vertex3D m_vertexcamera;

	LightProjected m_lightproject;

	HANDLE m_hFileCache;
	BOOL m_fReadingFromCache;
	BOOL m_fWritingToCache;
	};
