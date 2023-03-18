#pragma once

#include "inc/gpuprofiler.h"
#include "RenderDevice.h"
#include "Texture.h"
#include "backGlass.h"

extern int NumVideoBytes;

class PinProjection
{
public:
   void ScaleView(const float x, const float y, const float z);
   void MultiplyView(const Matrix3D& mat);
   void RotateView(float x, float y, float z);
   void TranslateView(const float x, const float y, const float z);

   void FitCameraToVerticesFS(const vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback);
   void FitCameraToVertices(const vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback);
   void CacheTransform();      // compute m_matrixTotal = m_World * m_View * m_Proj
   void TransformVertices(const Vertex3Ds * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;

   void ComputeNearFarPlane(const vector<Vertex3Ds>& verts);

   Matrix3D m_matWorld;
   Matrix3D m_matView;
   Matrix3D m_matProj[2];
   Matrix3D m_matrixTotal[2];
   StereoMode m_stereo3D;

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

   HRESULT InitPin3D(const bool fullScreen, const int width, const int height, const int colordepth, int &refreshrate, const int VSync, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl);

   void InitLayoutFS();
   void InitLayout(const bool FSS_mode, const float max_separation, const float xpixoff = 0.f, const float ypixoff = 0.f);

   void TransformVertices(const Vertex3D_NoTex2 * const __restrict rgv, const WORD * const __restrict rgi, const int count, Vertex2D * const __restrict rgvout) const;

   Vertex3Ds Unproject(const Vertex3Ds& point);
   Vertex3Ds Get3DPointFrom2D(const POINT& p);

   void Flip(const bool vsync);

   void EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend = true, const bool set_blend_op = true) const;

   void DrawBackground();

   const Matrix3D& GetWorldTransform() const   { return m_proj.m_matWorld; }
   const Matrix3D& GetViewTransform() const    { return m_proj.m_matView; }
   void InitLights();
   void UpdateMatrices();

   BackGlass* m_backGlass;

private:
   void InitRenderState(RenderDevice * const pd3dDevice);
   void InitPrimaryRenderState();
   void InitSecondaryRenderState();
   HRESULT InitPrimary(const bool fullScreen, const int colordepth, int &refreshrate, const int VSync, const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl);

   StereoMode m_stereo3D;

   // Data members
public:
   CGpuProfiler m_gpu_profiler;

   RenderDevice* m_pd3dPrimaryDevice;
   RenderDevice* m_pd3dSecondaryDevice;

   RenderTarget* m_pddsStatic;

   Texture m_pinballEnvTexture; // loaded from Resources
   Texture m_builtinEnvTexture; // loaded from Resources
   Texture m_aoDitherTexture;   // loaded from Resources

   Texture* m_envTexture;
   BaseTexture* m_envRadianceTexture;

   PinProjection m_proj;

   // free-camera-mode-fly-around parameters
   Vertex3Ds m_cam;
   float m_inc;

   //Vertex3Ds m_viewVec;        // direction the camera is facing

   ViewPort m_viewPort; // Viewport of the screen output (different from render size for VR, anaglyph, superscaling,...)
   float m_AAfactor;

   void UpdateBAMHeadTracking();                 // #ravarcade: UpdateBAMHeadTracking will set proj/view matrix to add BAM view and head tracking
};

Matrix3D ComputeLaybackTransform(float layback);
