// license:GPLv3+

#pragma once

#include "gpuprofiler.h"
#include "math/ModelViewProj.h"
#include "renderer/RenderDevice.h"
#include "renderer/Texture.h"
#include "parts/backGlass.h"
#include "plugins/CorePlugin.h"

class Renderable;

class Renderer
{
public:
   Renderer(PinTable* const table, VPX::Window* wnd, VideoSyncMode& syncMode, const StereoMode stereo3D);
   ~Renderer();

   void GetRenderSize(int& w, int& h) const { w = m_renderWidth; h = m_renderHeight; }
   void GetRenderSizeAA(int& w, int& h) const { w = m_pOffscreenBackBufferTexture1->GetWidth(); h = m_pOffscreenBackBufferTexture1->GetHeight(); }
   bool IsStereo() const { return m_pOffscreenBackBufferTexture1->m_type == SurfaceType::RT_STEREO; }
   colorFormat GetRenderFormat() const { return m_pOffscreenBackBufferTexture1->GetColorFormat(); }

   void InitLayout(const float xpixoff = 0.f, const float ypixoff = 0.f);
   ModelViewProj& GetMVP() { return *m_mvp; }
   const ModelViewProj& GetMVP() const { return *m_mvp; }
   Vertex3Ds Unproject(const int width, const int height, const Vertex3Ds& point);
   Vertex3Ds Get3DPointFrom2D(const int width, const int height, const POINT& p);

   void MarkShaderDirty() { m_shaderDirty = true; }
   void UpdateBasicShaderMatrix(const Matrix3D& objectTrafo = Matrix3D::MatrixIdentity());
   void UpdateBallShaderMatrix();
   void UpdateStereoShaderState();

   void DisableStaticPrePass(const bool disable) { bool wasUsingStaticPrepass = IsUsingStaticPrepass(); m_disableStaticPrepass += disable ? 1 : -1; m_isStaticPrepassDirty |= wasUsingStaticPrepass != IsUsingStaticPrepass(); }
   bool IsUsingStaticPrepass() const { return m_disableStaticPrepass <= 0; }
   unsigned int GetNPrerenderTris() const { return m_statsDrawnStaticTriangles; }
   void RenderStaticPrepass();

   void RenderFrame();

   enum ColorSpace
   {
      Linear,
      Reinhard,
      Reinhard_sRGB
   };
   enum SegmentFamily
   {
      Generic,
      Gottlieb,
      Williams,
      Bally,
      Atari,
   };
   void SetupSegmentRenderer(int profile, const bool isBackdrop, const vec3& color, const float brightness, const SegmentFamily family, const SegElementType type, float* segs, const float alpha, const ColorSpace colorSpace, Vertex3D_NoTex2* vertices,
      const vec4& emitterPad, const vec3& glassTint, const float glassRougness, Texture* const glassTex, const vec4& glassArea, const vec3& glassAmbient);
   void SetupDMDRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, BaseTexture* dmd, const float alpha, const ColorSpace colorSpace, Vertex3D_NoTex2 *vertices,
      const vec4& emitterPad, const vec3& glassTint, const float glassRougness, Texture* const glassTex, const vec4& glassArea, const vec3& glassAmbient);
   void DrawStatics();
   void DrawDynamics(bool onlyBalls);
   void DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, Sampler* const tex, const float intensity, const bool backdrop = false);

   void ReinitRenderable(Renderable* part) { m_renderableToInit.push_back(part); }

   RenderProbe::ReflectionMode GetMaxReflectionMode() const {
      // For dynamic mode, static reflections are not available so adapt the mode
      return !IsUsingStaticPrepass() && m_maxReflectionMode >= RenderProbe::REFL_STATIC ? RenderProbe::REFL_DYNAMIC : m_maxReflectionMode;
   }
   int GetAOMode() const // 0=Off, 1=Static, 2=Dynamic
   {
      // We must evaluate this dynamically since AO scale and enabled/disable can be changed from script
      if (m_disableAO || !m_table->m_enableAO || !m_renderDevice->DepthBufferReadBackAvailable() || m_table->m_AOScale == 0.f)
         return 0;
      // The existing implementation suffers for high temporal artefact that make it unsuitable for dynamic camera situation
      if (m_stereo3D == STEREO_VR)
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
   bool m_overwriteBallImages = false;
   Texture* m_ballImage = nullptr;
   Texture* m_decalImage = nullptr;

   // Post processing
   void SetScreenOffset(const float x, const float y); // set render offset in screen coordinates, e.g., for the nudge shake
   bool m_bloomOff = false;
   bool m_motionBlurOff = false;
   int m_FXAA; // =FXAASettings
   int m_sharpen; // 0=off, 1=CAS, 2=bilateral CAS
   
   VRPreviewMode m_vrPreview;

   enum RenderMask : unsigned int
   {
      DEFAULT = 0,                // No flag, just render everything
      STATIC_ONLY = 1 << 0,       // Disable non static part rendering (for static prerendering)
      DYNAMIC_ONLY = 1 << 1,      // Disable static part rendering
      LIGHT_BUFFER = 1 << 2,      // Transmitted light rendering
      REFLECTION_PASS = 1 << 3,   // Reflection pass, only render reflected elements
      DISABLE_LIGHTMAPS = 1 << 4, // Disable lightmaps, usefull for reflection probe parallel to lightmap ot avoid doubling them
      DISABLE_BACKDROP = 1 << 5,  // Disable backdrop, used for VR and cabinet rendering (background is just black for these)
   };
   unsigned int m_render_mask = DEFAULT; // Active pass render bit mask
   inline bool IsRenderPass(const RenderMask pass_mask) const { return (m_render_mask & pass_mask) != 0; }

   ToneMapper m_toneMapper = TM_AGX;
   bool m_HDRforceDisableToneMapper = true;
   float m_exposure = 1.f;

   CGpuProfiler m_gpu_profiler;

   RenderDevice* m_renderDevice = nullptr;

   Texture* m_envTexture = nullptr;
   Texture m_pinballEnvTexture; // loaded from assets folder

   // free-camera-mode-fly-around parameters
   Vertex3Ds m_cam = Vertex3Ds(0.f, 0.f, 0.f);
   float m_inc = 0.f;

   StereoMode m_stereo3D;
   const bool m_stereo3DfakeStereo;
   bool m_stereo3Denabled;
   float m_stereo3DDefocus = 0.f;

   RenderTarget* GetOffscreenVR(int eye) const { return eye == 0 ? m_pOffscreenVRLeft : m_pOffscreenVRRight; }
   RenderTarget* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture1; } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)

private:
   void RenderDynamics();
   void DrawBackground();
   void DrawBulbLightBuffer();
   void PrepareVideoBuffers(RenderTarget* outputBackBuffer);
   void Bloom();
   void SSRefl();
   BaseTexture* EnvmapPrecalc(const Texture* envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres);

   bool m_shaderDirty = true;
   void SetupShaders();

   int m_renderWidth, m_renderHeight; // Render size without supersampling (AAFactor) applied (depends on output VR/Display and stereo mode)

   RenderTarget* m_pOffscreenMSAABackBufferTexture = nullptr;
   RenderTarget* GetMSAABackBufferTexture() const { return m_pOffscreenMSAABackBufferTexture ? m_pOffscreenMSAABackBufferTexture : m_pOffscreenBackBufferTexture1; } // Main render target, may be MSAA enabled and not suited for sampling, also may have stereo output (2 layers)

   RenderTarget* m_pOffscreenBackBufferTexture1 = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture2 = nullptr;
   RenderTarget* GetPreviousBackBufferTexture() const { return m_pOffscreenBackBufferTexture2; } // Same as back buffer but for previous frame
   void SwapBackBufferRenderTargets();

   RenderTarget* m_pAORenderTarget1 = nullptr;
   RenderTarget* m_pAORenderTarget2 = nullptr;
   RenderTarget* GetAORenderTarget(int idx);
   void SwapAORenderTargets();
   void ReleaseAORenderTargets() { delete m_pAORenderTarget1; m_pAORenderTarget1 = nullptr; delete m_pAORenderTarget2; m_pAORenderTarget2 = nullptr; }

   RenderTarget* m_pPostProcessRenderTarget1 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget2 = nullptr;
   RenderTarget* GetPostProcessRenderTarget1();
   RenderTarget* GetPostProcessRenderTarget2();
   RenderTarget* GetPostProcessRenderTarget(RenderTarget* renderedRT);

   RenderTarget* m_pReflectionBufferTexture = nullptr;
   RenderTarget* GetReflectionBufferTexture();

   RenderTarget* m_pMotionBlurBufferTexture = nullptr;
   RenderTarget* GetMotionBlurBufferTexture();

   RenderTarget* m_pBloomBufferTexture = nullptr;
   RenderTarget* m_pBloomTmpBufferTexture = nullptr;
   RenderTarget* GetBloomBufferTexture() const { return m_pBloomBufferTexture; }
   RenderTarget* GetBloomTmpBufferTexture() const { return m_pBloomTmpBufferTexture; }

   RenderTarget* m_pOffscreenVRLeft = nullptr;
   RenderTarget* m_pOffscreenVRRight = nullptr;

   PinTable* const m_table;

   ModelViewProj* m_mvp = nullptr; // Store the active Model / View / Projection

   bool m_isStaticPrepassDirty = true;
   int m_disableStaticPrepass = 0;
   RenderTarget* m_staticPrepassRT = nullptr;
   unsigned int m_statsDrawnStaticTriangles = 0;
   RenderProbe::ReflectionMode m_maxReflectionMode;

   vector<Renderable*> m_renderableToInit;

   Texture m_builtinEnvTexture; // loaded from assets folder

   bool m_dynamicAO;
   bool m_disableAO;
   Texture m_aoDitherTexture; // loaded from assets folder

   int m_BWrendering; // 0=off, 1=Black&White from RedGreen, 2=B&W from Red only

   bool m_ss_refl;
   bool m_vrPreviewShrink = false;
   Vertex2D m_ScreenOffset = Vertex2D(0.f, 0.f); // for screen shake effect during nudge

   Texture* m_tonemapLUT = nullptr;

   #if defined(ENABLE_OPENGL) && !defined(__OPENGLES__)
   RenderTarget* m_envRadianceTexture = nullptr;
   #else
   BaseTexture* m_envRadianceTexture = nullptr;
   #endif

   // Segment display rendering
   Texture m_segDisplaySDF[4][9];
   vec4 m_segColor[8]; // Base seg color and brightness
   vec4 m_segUnlitColor[8]; // unlit color and back glow

   // DMD rendering
   vec4 m_dmdDefaultDotTint; // Table's default Dmd tint
   bool m_dmdUseNewRenderer[7];
   vec4 m_dmdDotColor[7]; // Base dot color and brightness
   vec4 m_dmdDotProperties[7]; // size, sharpness, rounding, back glow
   vec4 m_dmdUnlitDotColor[7]; // unlit color
};
