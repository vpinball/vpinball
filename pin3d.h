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
	void Multiply(const Matrix3D &mult, Matrix3D &result) const;
	void RotateXMatrix(const GPINFLOAT x);
	void RotateYMatrix(const GPINFLOAT y);
	void RotateZMatrix(const GPINFLOAT z);
	void SetIdentity();
	void Scale(const float x, const float y, const float z );
	void Invert();
	void MultiplyVector(const float x, const float y, const float z, Vertex3D * const pv3DOut) const;
	void MultiplyVector(const float x, const float y, const float z, Vertex3Ds * const pv3DOut) const;
	};

class PinProjection
	{
public:
	void Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z);
	void Translate(const float x, const float y, const float z);
	void FitCameraToVertices(Vector<Vertex3D> * const pvvertex3D, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV);
	void CacheTransform();
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout) const;
	void SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar);

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

	void InitLayout(const float left, const float top, const float right, const float bottom, const float inclination, const float FOV, const float rotation, const float scalex, const float scaley, const float xlatex, const float xlatey);
	void SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar);
	void Identity();
	void Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z);
	void Scale(const float x, const float y, const float z);
	void Translate(const float x, const float y, const float z);
	void FitCameraToVertices(Vector<Vertex3D> * const pvvertex3D, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV);
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout);
	void CacheTransform();
	LPDIRECTDRAWSURFACE7 CreateShadow(const float height);

	void CreateBallShadow();

	void SetUpdatePos(const int left, const int top);
	void Flip(const int offsetx, const int offsety);

	void SetRenderTarget(LPDIRECTDRAWSURFACE7 pddsSurface, LPDIRECTDRAWSURFACE7 pddsZ);
	void SetTextureFilter(const int TextureNum, const int Mode);
	
	void SetTexture(LPDIRECTDRAWSURFACE7 pddsTexture);
	void EnableLightMap(const BOOL fEnable, const float z);

	void SetMaterial(const float r, const float g, const float b, const float a);
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

	void ClearExtents(RECT * const prc, float * const pznear, float * const pzfar);
	void ExpandExtents(RECT * const prc, Vertex3D* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed);
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
