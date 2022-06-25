#pragma once

#include "inc\gpuprofiler.h"
#include "RenderDevice.h"
#include "Texture.h"

extern int NumVideoBytes;

enum TextureFilter
{
   TEXTURE_MODE_NONE,			// No filtering at all, single texel returned.
   TEXTURE_MODE_POINT,			// Point sampled (aka nearest mipmap) texture filtering.
   TEXTURE_MODE_BILINEAR,		// Bilinar texture filtering. 
   TEXTURE_MODE_TRILINEAR,		// Trilinar texture filtering. 
   TEXTURE_MODE_ANISOTROPIC		// Anisotropic texture filtering. 
};

enum StereoMode
{
   STEREO_OFF = 0, // Disabled
   STEREO_TB = 1, // TB (Top / Bottom)
   STEREO_INT = 2, // Interlaced (e.g. LG TVs)
   STEREO_FLIPPED_INT = 3, // Flipped Interlaced (e.g. LG TVs)
   STEREO_SBS = 4, // SBS (Side by Side)
   STEREO_ANAGLYPH_RC = 5, // Anaglyph Red/Cyan
   STEREO_ANAGLYPH_GM = 6, // Anaglyph Green/Magenta
   STEREO_ANAGLYPH_DUBOIS_RC = 7, // Anaglyph Dubois Red/Cyan
   STEREO_ANAGLYPH_DUBOIS_GM = 8, // Anaglyph Dubois Green/Magenta
   STEREO_ANAGLYPH_DEGHOSTED_RC = 9, // Anaglyph Deghosted Red/Cyan
   STEREO_ANAGLYPH_DEGHOSTED_GM = 10, // Anaglyph Deghosted Green/Magenta
   STEREO_ANAGLYPH_BA = 11, // Anaglyph Blue/Amber
   STEREO_ANAGLYPH_CR = 12, // Anaglyph Cyan/Red
   STEREO_ANAGLYPH_MR, // Anaglyph Magenta/Green
   STEREO_ANAGLYPH_DUBOIS_CR = 14, // Anaglyph Dubois Cyan/Red
   STEREO_ANAGLYPH_DUBOIS_MG = 15, // Anaglyph Dubois Magenta/Green
   STEREO_ANAGLYPH_DEGHOSTED_CR = 16, // Anaglyph Deghosted Cyan/Red
   STEREO_ANAGLYPH_DEGHOSTED_MG = 17, // Anaglyph Deghosted Magenta/Green
   STEREO_ANAGLYPH_AB = 18, // Anaglyph Amber/Blue
   STEREO_VR = 19, // Hardware VR set (not supported by DX9)
};

class PinProjection
{
public:
   void ScaleView(const float x, const float y, const float z);
   void MultiplyView(const Matrix3D& mat);
   void RotateView(float x, float y, float z);
   void TranslateView(const float x, const float y, const float z);

   void FitCameraToVerticesFS(const std::vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback);
   void FitCameraToVertices(const std::vector<Vertex3Ds>& pvvertex3D, float aspect, float rotation, float inclination, float FOV, float xlatez, float layback);
   void CacheTransform();      // compute m_matrixTotal = m_World * m_View * m_Proj
   void TransformVertices(const Vertex3Ds * const rgv, const WORD * const rgi, const int count, Vertex2D * const rgvout) const;

   void ComputeNearFarPlane(const std::vector<Vertex3Ds>& verts);

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

   HRESULT InitPin3D(const bool fullScreen, const int width, const int height, const int colordepth, int &refreshrate, const int VSync, const bool useAA, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool useAO, const bool ss_refl);

   void InitLayoutFS();
   void InitLayout(const bool FSS_mode, const float xpixoff = 0.f, const float ypixoff = 0.f);

   void TransformVertices(const Vertex3D_NoTex2 * const __restrict rgv, const WORD * const __restrict rgi, const int count, Vertex2D * const __restrict rgvout) const;

   Vertex3Ds Unproject(const Vertex3Ds& point);
   Vertex3Ds Get3DPointFrom2D(const POINT& p);

   void Flip(const bool vsync);

   void SetTextureFilter(RenderDevice * const pd3dDevice, const int TextureNum, const int Mode) const;
   void SetPrimaryTextureFilter(const int TextureNum, const int Mode) const;
   void SetSecondaryTextureFilter(const int TextureNum, const int Mode) const;

   void EnableAlphaTestReference(const DWORD alphaRefValue) const;
   void EnableAlphaBlend(const bool additiveBlending, const bool set_dest_blend = true, const bool set_blend_op = true) const;

   void DrawBackground();
   void RenderPlayfieldGraphics(const bool depth_only);

   const Matrix3D& GetWorldTransform() const   { return m_proj.m_matWorld; }
   const Matrix3D& GetViewTransform() const    { return m_proj.m_matView; }
   void InitPlayfieldGraphics();
   void InitLights();
   void UpdateMatrices();

private:
   void InitRenderState(RenderDevice * const pd3dDevice);
   void InitPrimaryRenderState();
   void InitSecondaryRenderState();
   HRESULT InitPrimary(const bool fullScreen, const int colordepth, int &refreshrate, const int VSync, const bool useAA, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool useAO, const bool ss_refl);

   // Data members
public:
   CGpuProfiler m_gpu_profiler;

   RenderDevice* m_pd3dPrimaryDevice;
   RenderDevice* m_pd3dSecondaryDevice;

   RenderTarget* m_pddsAOBackBuffer;
   RenderTarget* m_pddsAOBackTmpBuffer;

   RenderTarget* m_pddsBackBuffer;

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

   ViewPort m_viewPort;

private:
   VertexBuffer *m_tableVBuffer;
#ifdef ENABLE_BAM
public:
   void UpdateBAMHeadTracking();                 // #ravarcade: UpdateBAMHeadTracking will set proj/view matrix to add BAM view and head tracking
#endif
};

Matrix3D ComputeLaybackTransform(float layback);
