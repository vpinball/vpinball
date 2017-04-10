#pragma once
#include "RenderDevice.h"
#include "Texture.h"

extern int NumVideoBytes;

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

   void FitCameraToVerticesFS(std::vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback);
   void FitCameraToVertices(std::vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback);
   void CacheTransform();      // compute m_matrixTotal = m_World * m_View * m_Proj
   void TransformVertices(const Vertex3Ds * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;

   void ComputeNearFarPlane(std::vector<Vertex3Ds>& verts);

   Matrix3D m_matWorld;
   Matrix3D m_matView;
   Matrix3D m_matProj;
   Matrix3D m_matrixTotal;

   RECT m_rcviewport;

   float m_rznear, m_rzfar;
   Vertex3Ds m_vertexcamera;
   //float m_cameraLength;
};

class Pin3D
{
public:
   Pin3D();
   ~Pin3D();

   HRESULT InitPin3D(const HWND hwnd, const bool fullScreen, const int width, const int height, const int colordepth, int &refreshrate, const int VSync, const bool useAA, const bool stereo3D, const unsigned int FXAA, const bool useAO);

   void InitLayoutFS();
   void InitLayout(const bool FSS_mode);

   void TransformVertices(const Vertex3D_NoTex2 * rgv, const WORD * rgi, int count, Vertex2D * rgvout) const;

   Vertex3Ds Unproject(const Vertex3Ds& point);
   Vertex3Ds Get3DPointFrom2D(const POINT& p);

   void Flip(bool vsync);

   void SetRenderTarget(RenderTarget* pddsSurface, RenderTarget* pddsZ) const;
   void SetRenderTarget(RenderTarget* pddsSurface, D3DTexture* pddsZ) const;
   void SetRenderTarget(RenderTarget* pddsSurface, void* pddsZ) const;
   void SetTextureFilter(const int TextureNum, const int Mode) const;

   void EnableAlphaTestReference(const DWORD alphaRefValue) const;
   void EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend = true, const bool set_blend_op = true) const;
   void DisableAlphaBlend() const;

   void DrawBackground();
   void RenderPlayfieldGraphics(const bool depth_only);

   const Matrix3D& GetWorldTransform() const   { return m_proj.m_matWorld; }
   const Matrix3D& GetViewTransform() const    { return m_proj.m_matView; }
   void InitPlayfieldGraphics();
   void InitLights();

private:
   void InitRenderState();

   void Identity();

   // Data members
public:
   RenderDevice* m_pd3dDevice;
   RenderTarget* m_pddsBackBuffer;

   D3DTexture* m_pddsAOBackBuffer;
   D3DTexture* m_pddsAOBackTmpBuffer;

   D3DTexture* m_pdds3DZBuffer;

   void* m_pddsZBuffer; // D3DTexture* or RenderTarget*, depending on HW support

   RenderTarget* m_pddsStatic;
   void* m_pddsStaticZ; // D3DTexture* or RenderTarget*, depending on HW support

   //RenderTarget* m_bloomTexture;

   Texture pinballEnvTexture; // loaded from Resources
   Texture envTexture; // loaded from Resources
   Texture aoDitherTexture; // loaded from Resources

   Texture* m_envTexture;
   BaseTexture* m_envRadianceTexture;

   PinProjection m_proj;

   // free-camera-mode-fly-around parameters
   float m_camx;
   float m_camy;
   float m_camz;
   float m_inc;
   HWND m_hwnd;

   //Vertex3Ds m_viewVec;        // direction the camera is facing

   ViewPort vp;
   bool m_useAA;

private:
   VertexBuffer *tableVBuffer;
   IndexBuffer *tableIBuffer;
};

Matrix3D ComputeLaybackTransform(float layback);
