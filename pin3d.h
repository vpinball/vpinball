#pragma once

#include "gpuprofiler.h"
#include "renderer/RenderDevice.h"
#include "renderer/Texture.h"
#include "backGlass.h"

class ModelViewProj
{
public:
   enum FlipMode { NONE, FLIPX, FLIPY };
   ModelViewProj(const unsigned int nEyes = 1) : m_nEyes(nEyes) {}

   void SetFlip(const FlipMode flip) { m_dirty = true; m_flip = flip; }
   void SetModel(const Matrix3D& Model) { MarkDirty(Model, m_matModel); m_matModel = Model; }
   void SetView(const Matrix3D& view) { MarkDirty(view, m_matView); m_matView = view; }
   void SetProj(const unsigned int index, const Matrix3D& proj) { MarkDirty(proj, m_matProj[index]); m_matProj[index] = proj; }

   const Matrix3D& GetModel() const { return m_matModel; }
   const Matrix3D& GetView() const { return m_matView; }
   const Matrix3D& GetProj(const unsigned int eye) const { return m_matProj[eye]; }
   const Matrix3D& GetModelView() const { Update(); return m_matModelView; }
   const Matrix3D& GetModelViewInverse() const { Update(); return m_matModelViewInverse; }
   const Matrix3D& GetModelViewInverseTranspose() const { Update(); return m_matModelViewInverseTranspose; }
   const Matrix3D& GetModelViewProj(const unsigned int eye) const { Update(); return m_matModelViewProj[eye]; }
   const Vertex3Ds& GetViewVec() const { Update(); return m_viewVec; }

   const unsigned int m_nEyes;

private:
   void MarkDirty(const Matrix3D& newMat, const Matrix3D& oldMat)
   {
      if (!m_dirty)
         m_dirty = memcmp(oldMat.m, newMat.m, 4 * 4 * sizeof(float)) != 0;
   }

   void Update() const
   {
      if (m_dirty)
      {
         m_dirty = false;
         m_matModelView = m_matModel * m_matView;
         memcpy(m_matModelViewInverse.m, m_matModelView.m, 4 * 4 * sizeof(float));
         m_matModelViewInverse.Invert();
         memcpy(m_matModelViewInverseTranspose.m, m_matModelViewInverse.m, 4 * 4 * sizeof(float));
         m_matModelViewInverseTranspose.Transpose();
         switch (m_flip)
         {
         case NONE:
         {
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelView * m_matProj[eye];
            break;
         }
         case FLIPX:
         {
            static const Matrix3D flipx(-1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelView * m_matProj[eye] * flipx;
            break;
         }
         case FLIPY:
         {
            static const Matrix3D flipy(1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
            for (unsigned int eye = 0; eye < m_nEyes; eye++)
               m_matModelViewProj[eye] = m_matModelView * m_matProj[eye] * flipy;
            break;
         }
         }
         Matrix3D temp, viewRot;
         temp = m_matView;
         temp.Invert();
         temp.GetRotationPart(viewRot);
         viewRot.MultiplyVector(Vertex3Ds(0, 0, 1), m_viewVec);
         m_viewVec.NormalizeSafe();
      }
   }

   Matrix3D m_matModel;
   Matrix3D m_matView;
   Matrix3D m_matProj[6];

   mutable bool m_dirty = true;
   FlipMode m_flip = NONE;
   mutable Matrix3D m_matModelView;
   mutable Matrix3D m_matModelViewInverse;
   mutable Matrix3D m_matModelViewInverseTranspose;
   mutable Matrix3D m_matModelViewProj[6];
   mutable Vertex3Ds m_viewVec;
};

class Pin3D
{
public:
   Pin3D();
   ~Pin3D();

   HRESULT InitPin3D(const bool fullScreen, const int width, const int height, const int colordepth, int &refreshrate, VideoSyncMode& syncMode, 
      const float AAfactor, const StereoMode stereo3D, const unsigned int FXAA, const bool sharpen, const bool ss_refl);

   void InitLayout(const float xpixoff = 0.f, const float ypixoff = 0.f);

   void TransformVertices(const Vertex3D_NoTex2 * const __restrict rgv, const WORD * const __restrict rgi, const int count, Vertex2D * const __restrict rgvout) const;
   void TransformVertices(const Vertex3Ds* const __restrict rgv, const WORD* const __restrict rgi, const int count, Vertex2D* const __restrict rgvout) const;

   Vertex3Ds Unproject(const Vertex3Ds& point);
   Vertex3Ds Get3DPointFrom2D(const POINT& p);

   void DrawBackground();

   ModelViewProj& GetMVP() { return *m_mvp; }
   const ModelViewProj& GetMVP() const { return *m_mvp; }

   void InitLights();

   BackGlass* m_backGlass;

private:
   HRESULT InitPrimary(const bool fullScreen, const int colordepth, int& refreshrate, VideoSyncMode& syncMode, const float AAfactor, const StereoMode stereo3D,
      const unsigned int FXAA, const bool sharpen, const bool ss_refl);

   StereoMode m_stereo3D;

   ModelViewProj* m_mvp = nullptr; // Store the active Model / View / Projection

   #ifdef ENABLE_SDL
#ifndef __OPENGLES__
   RenderTarget* m_envRadianceTexture = nullptr;
#else
   BaseTexture* m_envRadianceTexture = nullptr;
#endif
   #else
   BaseTexture* m_envRadianceTexture = nullptr;
   #endif

   // Data members
public:
   CGpuProfiler m_gpu_profiler;

   RenderDevice* m_pd3dPrimaryDevice = nullptr;

   Texture m_pinballEnvTexture; // loaded from Resources
   Texture m_builtinEnvTexture; // loaded from Resources
   Texture m_aoDitherTexture;   // loaded from Resources

   Texture* m_envTexture = nullptr;

   // free-camera-mode-fly-around parameters
   Vertex3Ds m_cam;
   float m_inc;

   ViewPort m_viewPort; // Viewport of the screen output (different from render size for VR, anaglyph, superscaling,...)
   float m_AAfactor;

   void UpdateBAMHeadTracking();                 // #ravarcade: UpdateBAMHeadTracking will set proj/view matrix to add BAM view and head tracking
};
