#pragma once

#include "gpuprofiler.h"
#include "math/ModelViewProj.h"
#include "renderer/RenderDevice.h"
#include "renderer/Texture.h"
#include "backGlass.h"

class FrameQueueLimiter;

class Renderer
{
public:
   Renderer(PinTable* const table, const bool fullScreen, const int width, const int height, const int colordepth, int& refreshrate, VideoSyncMode& syncMode, const StereoMode stereo3D);
   ~Renderer();

   void InitLayout(const float xpixoff = 0.f, const float ypixoff = 0.f);
   ModelViewProj& GetMVP() { return *m_mvp; }
   const ModelViewProj& GetMVP() const { return *m_mvp; }
   void TransformVertices(const Vertex3D_NoTex2* const __restrict rgv, const WORD* const __restrict rgi, const int count, Vertex2D* const __restrict rgvout) const;
   void TransformVertices(const Vertex3Ds* const __restrict rgv, const WORD* const __restrict rgi, const int count, Vertex2D* const __restrict rgvout) const;
   Vertex3Ds Unproject(const Vertex3Ds& point);
   Vertex3Ds Get3DPointFrom2D(const POINT& p);

   void SetupShaders();
   void UpdateBasicShaderMatrix(const Matrix3D& objectTrafo = Matrix3D::MatrixIdentity());
   void UpdateBallShaderMatrix();
   void UpdateStereoShaderState();

   void DisableStaticPrePass(const bool disable) { if (m_disableStaticPrepass != disable) { m_disableStaticPrepass = disable; m_isStaticPrepassDirty = true; } }
   bool IsUsingStaticPrepass() const;
   unsigned int GetNPrerenderTris() const { return m_statsDrawnStaticTriangles; }
   void RenderStaticPrepass();

   void PrepareFrame();
   void SubmitFrame();

   void DrawStatics();
   void DrawDynamics(bool onlyBalls);
   void DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, Texture* const tex, const float intensity, const bool backdrop = false);
   void DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, Sampler* const tex, const float intensity, const bool backdrop = false);

   RenderProbe::ReflectionMode GetMaxReflectionMode() const {
      // For dynamic mode, static reflections are not available so adapt the mode
      return !IsUsingStaticPrepass() && m_maxReflectionMode >= RenderProbe::REFL_STATIC ? RenderProbe::REFL_DYNAMIC : m_maxReflectionMode;
   }
   int GetAOMode() const // 0=Off, 1=Static, 2=Dynamic
   {
      // We must evaluate this dynamically since AO scale and enabled/disable can be changed from script
      if (m_disableAO || !m_table->m_enableAO || !m_pd3dPrimaryDevice->DepthBufferReadBackAvailable() || m_table->m_AOScale == 0.f)
         return 0;
      if (m_dynamicAO)
         return 2;
      return IsUsingStaticPrepass() ? 1 : 0; // If AO is static prepass only and we are running without it, disable AO
   }

   BackGlass* m_backGlass = nullptr;

   float m_globalEmissionScale;

   vector<Light*> m_ballReflectedLights;
   MeshBuffer* m_ballMeshBuffer = nullptr;
   MeshBuffer* m_ballTrailMeshBuffer = nullptr;
   #ifdef DEBUG_BALL_SPIN
   MeshBuffer* m_ballDebugPoints = nullptr;
   #endif
   int m_ballTrailMeshBufferPos = 0;
   bool m_trailForBalls = false;
   float m_ballTrailStrength = 0.5f;
   bool m_disableLightingForBalls = false;
   bool m_overwriteBallImages = false;
   Texture* m_ballImage = nullptr;
   Texture* m_decalImage = nullptr;

   // Post processing
   void PrepareVideoBuffers();
   void SetScreenOffset(const float x, const float y); // set render offset in screen coordinates, e.g., for the nudge shake
   bool m_bloomOff;
   int m_FXAA; // =FXAASettings
   int m_sharpen; // 0=off, 1=CAS, 2=bilateral CAS
   VRPreviewMode m_vrPreview;

   enum RenderMask : unsigned int
   {
      DEFAULT = 0, // Render everything
      STATIC_ONLY = 1 << 0, // Disable non static part rendering (for static prerendering)
      DYNAMIC_ONLY = 1 << 1, // Disable static part rendering
      LIGHT_BUFFER = 1 << 2, // Transmitted light rendering
      REFLECTION_PASS = 1 << 3,
      DISABLE_LIGHTMAPS = 1 << 4
   };
   unsigned int m_render_mask = DEFAULT; // Active pass render bit mask
   inline bool IsRenderPass(const RenderMask pass_mask) const { return (m_render_mask & pass_mask) != 0; }

   ToneMapper m_toneMapper = TM_TONY_MC_MAPFACE;

   CGpuProfiler m_gpu_profiler;

   RenderDevice* m_pd3dPrimaryDevice = nullptr;

   Texture m_pinballEnvTexture; // loaded from Resources
   Texture m_builtinEnvTexture; // loaded from Resources
   Texture m_aoDitherTexture;   // loaded from Resources

   Texture* m_envTexture = nullptr;

   // free-camera-mode-fly-around parameters
   Vertex3Ds m_cam = Vertex3Ds(0.f, 0.f, 0.f);
   float m_inc = 0.f;

   ViewPort m_viewPort; // Viewport of the screen output (different from render size for VR, anaglyph, superscaling,...)
   float m_AAfactor;

   bool m_stereo3DfakeStereo;
   bool m_stereo3Denabled;
   float m_stereo3DDefocus = 0.f;

private:
   void RenderDynamics();
   void DrawBackground();
   void DrawBulbLightBuffer();
   void Bloom();
   void SSRefl();
   BaseTexture* EnvmapPrecalc(const Texture* envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres);

   PinTable* const m_table;

   ModelViewProj* m_mvp = nullptr; // Store the active Model / View / Projection

   bool m_isStaticPrepassDirty = true;
   bool m_disableStaticPrepass = false;
   RenderTarget* m_staticPrepassRT = nullptr;
   unsigned int m_statsDrawnStaticTriangles = 0;
   RenderProbe::ReflectionMode m_maxReflectionMode;

   bool m_dynamicAO;
   bool m_disableAO;

   int m_BWrendering; // 0=off, 1=Black&White from RedGreen, 2=B&W from Red only

   bool m_ss_refl;
   StereoMode m_stereo3D;
   bool m_vrPreviewShrink = false;
   Vertex2D m_ScreenOffset = Vertex2D(0.f, 0.f); // for screen shake effect during nudge

   Texture* m_tonemapLUT = nullptr;

   FrameQueueLimiter* m_limiter = nullptr;

   #ifdef ENABLE_SDL
   RenderTarget* m_envRadianceTexture = nullptr;
   #else
   BaseTexture* m_envRadianceTexture = nullptr;
   #endif
};
