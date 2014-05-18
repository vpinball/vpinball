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

class PinProjection
	{
public:
	void ScaleView(const float x, const float y, const float z);
	void MultiplyView(const Matrix3D& mat);
	void RotateView(float x, float y, float z);
	void TranslateView(const float x, const float y, const float z);

	void FitCameraToVertices(Vector<Vertex3Ds> * const pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback);
	void CacheTransform();      // compute m_matrixTotal = m_World * m_View * m_Proj
	void TransformVertices(const Vertex3D * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;
	void SetFieldOfView(float rFOV, float raspect, float rznear, float rzfar);
	void SetupProjectionMatrix(float rFOV, float raspect, float rznear, float rzfar);

    void ComputeNearFarPlane(const Vector<Vertex3Ds>& verts);

	Matrix3D m_matWorld;
	Matrix3D m_matView;
	Matrix3D m_matProj;
	Matrix3D m_matrixTotal;

	RECT m_rcviewport;

	float m_rznear, m_rzfar;
	Vertex3Ds m_vertexcamera;
	};

class Pin3D
	{
public:
	Pin3D();
	~Pin3D();

	HRESULT InitPin3D(const HWND hwnd, const bool fFullScreen, const int screenwidth, const int screenheight, const int colordepth, int &refreshrate, const int VSync, const bool useAA, const bool stereo3DFXAA);

	void InitLayout();

	void TransformVertices(const Vertex3D_NoTex2 * rgv, const WORD * rgi, int count, Vertex2D * rgvout) const;

   Vertex3Ds Unproject( Vertex3Ds *point );
   Vertex3Ds Get3DPointFrom2D( POINT *p );

    void Flip(bool vsync);

	void SetRenderTarget(RenderTarget* pddsSurface, RenderTarget* pddsZ) const;
	void SetTextureFilter(const int TextureNum, const int Mode) const;
	
    void SetTexture(Texture* pTexture);
	void SetBaseTexture(DWORD texUnit, BaseTexture* pddsTexture);

    void EnableLightMap(const float z);
    void DisableLightMap();

	void EnableAlphaTestReference(DWORD alphaRefValue) const;
    void EnableAlphaBlend( DWORD alphaRefValue, BOOL additiveBlending=fFalse );
    void DisableAlphaBlend();

    void DrawBackground();
    void RenderPlayfieldGraphics();

	void CalcShadowCoordinates(Vertex3D * const pv, const unsigned int count) const;

    const Matrix3D& GetWorldTransform() const   { return m_proj.m_matWorld; }
    const Matrix3D& GetViewTransform() const    { return m_proj.m_matView; }

private:
    void InitRenderState();
    void InitLights();

	void Identity();
   
	BaseTexture* CreateShadow(const float height);
	void CreateBallShadow();

	// Data members
public:
	RenderDevice* m_pd3dDevice;
	RenderTarget* m_pddsBackBuffer;

	D3DTexture* m_pdds3DBackBuffer;
	D3DTexture* m_pdds3DZBuffer;

	RenderTarget* m_pddsZBuffer;

	RenderTarget* m_pddsStatic;
	RenderTarget* m_pddsStaticZ;

   RenderTarget* m_bloomTexture;

	Texture ballTexture;
	MemTexture *ballShadowTexture;
	Texture lightTexture[2]; // 0=bumper, 1=lights
	Texture m_pddsLightWhite;

    PinProjection m_proj;

	int m_dwRenderWidth;
	int m_dwRenderHeight;

	HWND m_hwnd;

    Vertex3Ds m_viewVec;        // direction the camera is facing

    //bool fullscreen;
    ViewPort vp;

private:
    VertexBuffer *backgroundVBuffer;
    VertexBuffer *tableVBuffer;
    IndexBuffer *tableIBuffer;
    std::map<int, MemTexture*> m_xvShadowMap;

};

Matrix3D ComputeLaybackTransform(float layback);
