#pragma once

extern int NumVideoBytes;

enum
{
	eLightProject1 = 1,
	ePictureTexture = 0
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
	inline void Multiply(const Matrix3D &mult, Matrix3D &result) const
	{
	Matrix3D matrixT;
	for (int i=0; i<4; ++i)
		{
		for (int l=0; l<4; ++l)
			{
			matrixT.m[i][l] = (m[0][l] * mult.m[i][0]) + (m[1][l] * mult.m[i][1]) +
						      (m[2][l] * mult.m[i][2]) + (m[3][l] * mult.m[i][3]);
			}
		}
	result = matrixT;
	}
	inline void RotateXMatrix(const GPINFLOAT x)
	{
	SetIdentity();
	_22 = _33 = (float)cos(x);
	_23 = (float)sin(x);
	_32 = -_23;
	}
	inline void RotateYMatrix(const GPINFLOAT y)
	{
	SetIdentity();
	_11 = _33 = (float)cos(y);
	_31 = (float)sin(y);
	_13 = -_31;
	}
	inline void RotateZMatrix(const GPINFLOAT z)
	{
	SetIdentity();
	_11 = _22 = (float)cos(z);
	_12 = (float)sin(z);
	_21 = -_12;
	}
	inline void SetIdentity()
	{
	_11 = _22 = _33 = _44 = 1.0f;
	_12 = _13 = _14 = _41 =
	_21 = _23 = _24 = _42 =
	_31 = _32 = _34 = _43 = 0.0f;
	}
	inline void Scale(const float x, const float y, const float z)
	{
	_11 *= x;
	_12 *= x;
	_13 *= x;
	_21 *= y;
	_22 *= y;
	_23 *= y;
	_31 *= z;
	_32 *= z;
	_33 *= z;
	}
	inline void MultiplyVector(const float x, const float y, const float z, Vertex3D * const pv3DOut) const
	{
	// Transform it through the current matrix set
	const float xp = _11*x + _21*y + _31*z + _41;
	const float yp = _12*x + _22*y + _32*z + _42;
	const float wp = _14*x + _24*y + _34*z + _44;

	const float zp = _13*x + _23*y + _33*z + _43;

	const float inv_wp = 1.0f/wp;
	pv3DOut->x = xp*inv_wp;
	pv3DOut->y = yp*inv_wp;
	pv3DOut->z = zp*inv_wp;
	}
	inline void MultiplyVector(const float x, const float y, const float z, Vertex3Ds * const pv3DOut) const
	{
	// Transform it through the current matrix set
	const float xp = _11*x + _21*y + _31*z + _41;
	const float yp = _12*x + _22*y + _32*z + _42;
	const float wp = _14*x + _24*y + _34*z + _44;

	const float zp = _13*x + _23*y + _33*z + _43;

	const float inv_wp = 1.0f/wp;
	pv3DOut->x = xp*inv_wp;
	pv3DOut->y = yp*inv_wp;
	pv3DOut->z = zp*inv_wp;
	}

	void Invert();
	};

class PinProjection
	{
public:
	void Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z);
	void Translate(const float x, const float y, const float z);
	void FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV);
	void CacheTransform();
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout) const;
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;
	void SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar);

	Matrix3D m_matWorld;
	Matrix3D m_matView;
	Matrix3D m_matProj;
	Matrix3D m_matrixTotal;

	RECT m_rcviewport;

	GPINFLOAT m_rznear, m_rzfar;
	Vertex3Ds m_vertexcamera;
	};

class Pin3D
	{
public:
	Pin3D();
	~Pin3D();

	HRESULT InitDD(const HWND hwnd, const bool fFullScreen, const int screenwidth, const int screenheight, const int colordepth, int &refreshrate, const bool stereo3D);
	HRESULT Create3DDevice(const GUID * const pDeviceGUID);
	HRESULT CreateZBuffer(const GUID * const pDeviceGUID);

	void DrawBackground();

	void InitRenderState() const;

	void InitBackGraphics();

	void InitLayout(const float left, const float top, const float right, const float bottom, const float inclination, const float FOV, const float rotation, const float scalex, const float scaley, const float xlatex, const float xlatey, const float layback, const float maxSeparation, const float ZPD);
	void SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar);
	void Identity();
	void Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z);
	void Scale(const float x, const float y, const float z);
	void Translate(const float x, const float y, const float z);
	void FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV, const GPINFLOAT skew);
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout) const;
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;
	void CacheTransform();
	LPDIRECTDRAWSURFACE7 CreateShadow(const float height);

	void CreateBallShadow();

	void SetUpdatePos(const int left, const int top);
	void Flip(const int offsetx, const int offsety, const BOOL vsync);

	void SetRenderTarget(const LPDIRECTDRAWSURFACE7 pddsSurface, const LPDIRECTDRAWSURFACE7 pddsZ) const;
	void SetTextureFilter(const int TextureNum, const int Mode) const;
	
	void SetTexture(LPDIRECTDRAWSURFACE7 pddsTexture);
	void EnableLightMap(const BOOL fEnable, const float z);

	void SetMaterial(const float r, const float g, const float b, const float a);
	void SetColorKeyEnabled(const BOOL fColorKey) const;
	void SetAlphaEnabled(const BOOL fAlpha) const;
	void SetFiltersLinear() const;

	// Handy functions for creating obj frames

	void ClearExtents(RECT * const prc, float * const pznear, float * const pzfar);
	void ExpandExtents(RECT * const prc, Vertex3D* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed);
	void ExpandExtentsPlus(RECT * const prc, Vertex3D* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed);
	void ExpandRectByRect(RECT * const prc, const RECT * const prcNew) const;

	void ClipRectToVisibleArea(RECT * const prc) const;
	void EnsureDebugTextures();

	LPDIRECTDRAWSURFACE7 CreateOffscreen(const int width, const int height) const;
	LPDIRECTDRAWSURFACE7 CreateOffscreenWithCustomTransparency(const int width, const int height, const int color) const;
	LPDIRECTDRAWSURFACE7 CreateZBufferOffscreen(const int width, const int height) const;

	LPDIRECTDRAW7 m_pDD;
	LPDIRECTDRAWSURFACE7 m_pddsFrontBuffer;
	LPDIRECTDRAWSURFACE7 m_pddsBackBuffer;

	LPDIRECTDRAWSURFACE7 m_pdds3DBackBuffer;
	const unsigned int* __restrict m_pdds3Dbuffercopy;
	const unsigned int* __restrict m_pdds3Dbufferzcopy;
	unsigned char* __restrict m_pdds3Dbuffermask;

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

	float skewX;
	float skewY;

	int m_width;
	int m_height;
	HWND m_hwnd;
	float m_maxtu;
	float m_maxtv;

	float m_rotation, m_inclination, m_layback;
	float m_scalex, m_scaley;
	float m_xlatex, m_xlatey;

	GPINFLOAT m_rznear, m_rzfar;
	Vertex3Ds m_vertexcamera;

	LightProjected m_lightproject;

	float m_maxSeparation, m_ZPD;
	};
