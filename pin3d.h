#pragma once
#include "RenderDevice.h"
#include "Texture.h"

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
	inline void MultiplyVector(const float x, const float y, const float z, Vertex3D_NoTex2 * const pv3DOut) const
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
	inline Vertex3Ds MultiplyVector(const Vertex3Ds &v) const
	{
	// Transform it through the current matrix set
	const float xp = _11*v.x + _21*v.y + _31*v.z + _41;
	const float yp = _12*v.x + _22*v.y + _32*v.z + _42;
	const float wp = _14*v.x + _24*v.y + _34*v.z + _44;

	const float zp = _13*v.x + _23*v.y + _33*v.z + _43;

	const float inv_wp = 1.0f/wp;
	Vertex3Ds pv3DOut;
	pv3DOut.x = xp*inv_wp;
	pv3DOut.y = yp*inv_wp;
	pv3DOut.z = zp*inv_wp;
	return pv3DOut;
	}
	inline Vertex3Ds MultiplyVectorNoTranslate(const Vertex3Ds &v) const
	{
	// Transform it through the current matrix set
	const float xp = _11*v.x + _21*v.y + _31*v.z;
	const float yp = _12*v.x + _22*v.y + _32*v.z;
	const float zp = _13*v.x + _23*v.y + _33*v.z;

	Vertex3Ds pv3DOut;
	pv3DOut.x = xp;
	pv3DOut.y = yp;
	pv3DOut.z = zp;
	return pv3DOut;
	}

	void Invert();
	};

class PinProjection
	{
public:
	void Scale(const float x, const float y, const float z);
	void Multiply(const Matrix3D& mat);
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

	HRESULT InitDD(const HWND hwnd, const bool fFullScreen, const int screenwidth, const int screenheight, const int colordepth, int &refreshrate, const bool stereo3DFXAA, const bool AA);
	HRESULT Create3DDevice(const GUID * const pDeviceGUID);
	HRESULT CreateZBuffer(const GUID * const pDeviceGUID);

	void DrawBackground();

	void InitRenderState();

	void InitPlayfieldGraphics();
   void RenderPlayfieldGraphics();

	void InitLights();
   void ClearSpriteRectangle( AnimObject *animObj, ObjFrame *pof );
   void CreateAndCopySpriteBuffers( AnimObject *animObj, ObjFrame *pof );

	void InitLayout(const float left, const float top, const float right, const float bottom, const float inclination, const float FOV, const float rotation, const float scalex, const float scaley, const float xlatex, const float xlatey, const float layback, const float maxSeparation, const float ZPD);
	void SetFieldOfView(const GPINFLOAT rFOV, const GPINFLOAT raspect, const GPINFLOAT rznear, const GPINFLOAT rzfar);
	void Identity();
	void Rotate(const GPINFLOAT x, const GPINFLOAT y, const GPINFLOAT z);
	void Scale(const float x, const float y, const float z);
	void Translate(const float x, const float y, const float z);
	void FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, const int cvert, const GPINFLOAT aspect, const GPINFLOAT rotation, const GPINFLOAT inclination, const GPINFLOAT FOV, const GPINFLOAT skew);
	void CacheTransform();

	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex3D * const rgvout) const;
	void TransformVertices(const Vertex3D_NoTex2 * const rgv, const WORD * const rgi, const int count, Vertex3D_NoTex2 * const rgvout) const;
	void TransformVertices(const Vertex3D_NoLighting * const rgv, const WORD * const rgi, const int count, Vertex3D_NoLighting * const rgvout) const;
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;
	void TransformVertices(const Vertex3D_NoTex2 * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;

	BaseTexture* CreateShadow(const float height);

	void CreateBallShadow();

	void SetUpdatePos(const int left, const int top);
   void Flip(const int offsetx, const int offsety, const BOOL vsync);

	void SetRenderTarget(const BaseTexture* pddsSurface, const BaseTexture* pddsZ) const;
	void SetTextureFilter(const int TextureNum, const int Mode) const;
	
	void SetTexture(BaseTexture* pddsTexture);
	void EnableLightMap(const BOOL fEnable, const float z);

	void SetColorKeyEnabled(const BOOL fColorKey) const;
	void EnableAlphaTestReference(DWORD alphaRefValue) const;
    void EnableAlphaBlend( DWORD alphaRefValue, BOOL additiveBlending=fFalse );

	// Handy functions for creating obj frames

	void ClearExtents(RECT * const prc, float * const pznear, float * const pzfar);
	void ExpandExtents(RECT * const prc, Vertex3D* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed);
	void ExpandExtents(RECT * const prc, Vertex3D_NoTex2* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed);
	void ExpandExtents(RECT * const prc, Vertex3D_NoLighting* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed);
	void ExpandExtentsPlus(RECT * const prc, Vertex3D_NoTex2* const rgv, float * const pznear, float * const pzfar, const int count, const BOOL fTransformed);
	void ExpandRectByRect(RECT * const prc, const RECT * const prcNew) const;
   void AntiAliasingScene();
   void InitAntiAliasing();

	void ClipRectToVisibleArea(RECT * const prc) const;

	BaseTexture* CreateOffscreen(const int width, const int height) const;
	BaseTexture* CreateOffscreenWithCustomTransparency(const int width, const int height, const int color) const;
	BaseTexture* CreateZBufferOffscreen(const int width, const int height) const;
   
	LPDIRECTDRAW7 m_pDD;
	BaseTexture* m_pddsFrontBuffer;
	BaseTexture* m_pddsBackBuffer;

	BaseTexture* m_pdds3DBackBuffer;
	const unsigned int* __restrict m_pdds3Dbuffercopy;
	const unsigned int* __restrict m_pdds3Dbufferzcopy;
	unsigned char* __restrict m_pdds3Dbuffermask;

	BaseTexture* m_pddsZBuffer;
	LPDIRECT3D7 m_pD3D;
	RenderDevice* m_pd3dDevice;

	BaseTexture* m_pddsStatic;
	BaseTexture* m_pddsStaticZ;

   BaseTexture *antiAliasTexture;
	Texture ballTexture;
	Texture ballShadowTexture;
	Texture lightTexture;
	//LPDIRECTDRAWSURFACE7 m_pddsLightProjectTexture;
	BaseTexture* m_pddsLightWhite;

   VertexBuffer *backgroundVBuffer;
   VertexBuffer *tableVBuffer;
   WORD *playfieldPolyIndices;
   DWORD numPolys;
   DWORD numVerts;
	ExVector<void> m_xvShadowMap;

	Matrix3D m_matrixTotal;

	RECT m_rcScreen;
	int m_dwRenderWidth;
	int m_dwRenderHeight;

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
   //bool fullscreen;
	float m_maxSeparation, m_ZPD;
   D3DVIEWPORT7 vp;
   VertexBuffer *spriteVertexBuffer;
};
