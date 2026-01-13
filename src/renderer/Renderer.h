// license:GPLv3+

#pragma once

#include "gpuprofiler.h"
#include "math/ModelViewProj.h"
#include "renderer/RenderDevice.h"
#include "renderer/Texture.h"
#include "Backglass.h"
#include "plugins/ControllerPlugin.h"
#include "parts/PartGroup.h"

class Renderable;

class Renderer final
{
public:
   Renderer(PinTable* const table, VPX::Window* wnd, VideoSyncMode& syncMode, const StereoMode stereo3D);
   ~Renderer();

   void GetRenderSize(int& w, int& h) const { w = m_renderWidth; h = m_renderHeight; }
   void GetRenderSizeAA(int& w, int& h) const { w = m_pOffscreenBackBufferTexture1->GetWidth(); h = m_pOffscreenBackBufferTexture1->GetHeight(); }
   bool IsStereo() const { return m_pOffscreenBackBufferTexture1->m_type == SurfaceType::RT_STEREO; }
   colorFormat GetRenderFormat() const { return m_pOffscreenBackBufferTexture1->GetColorFormat(); }

   int GetDisplayWidth() const; // Playfield display width, taking in consideration stretching applied by some stereo modes
   int GetDisplayHeight() const; // Playfield display width, taking in consideration stretching applied by some stereo modes
   float GetDisplayAspectRatio() const;
   void InitLayout(const float xpixoff = 0.f, const float ypixoff = 0.f);
   ModelViewProj& GetMVP() { return *m_mvp; }
   const ModelViewProj& GetMVP() const { return *m_mvp; }
   Vertex3Ds Unproject(const int width, const int height, const Vertex3Ds& point) const;
   Vertex3Ds Get3DPointFrom2D(const int width, const int height, const Vertex2D& p, float z);

   void MarkShaderDirty() { m_shaderDirty = true; }
   void UpdateBasicShaderMatrix(const Matrix3D& objectTrafo = Matrix3D::MatrixIdentity());
   void UpdateBallShaderMatrix();
   void UpdateDesktopBackdropShaderMatrix(bool basic, bool light, bool flasherDMD);
   void UpdateStereoShaderState();

   void DisableStaticPrePass(const bool disable) { bool wasUsingStaticPrepass = IsUsingStaticPrepass(); m_disableStaticPrepass += disable ? 1 : -1; m_isStaticPrepassDirty |= wasUsingStaticPrepass != IsUsingStaticPrepass(); }
   bool IsUsingStaticPrepass() const { return (m_disableStaticPrepass <= 0) && (m_stereo3D != STEREO_VR); }
   unsigned int GetNPrerenderTris() const { return m_statsDrawnStaticTriangles; }

   enum class ShadeMode
   {
      Default,
      Wireframe,
      NoDepthWireframe
   };
   void SetShadeMode(ShadeMode mode) { m_shadeMode = mode; };
   ShadeMode GetShadeMode() const { return m_shadeMode; };

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

private:
   void SetupDisplayRenderer(const bool isBackdrop, Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex,
      const vec4& glassArea, const vec3& glassAmbient);

   ShadeMode m_shadeMode = ShadeMode::Default;

public:
   void SetupSegmentRenderer(int profile, const bool isBackdrop, const vec3& color, const float brightness, const SegmentFamily family, const SegElementType type, const float* segs, const ColorSpace colorSpace, Vertex3D_NoTex2* vertices,
      const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient);
   void SetupDMDRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& dmd, const float alpha, const ColorSpace colorSpace, Vertex3D_NoTex2 *vertices,
      const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient);
   void SetupCRTRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& crt, const float alpha, const ColorSpace colorSpace,
      Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient);
   void DrawStatics();
   void DrawDynamics(bool onlyBalls);
   void DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, const std::shared_ptr<const Sampler>& tex, const float intensity, const bool backdrop = false);
   void DrawWireframe(IEditable* renderable, const vec4& fillColor, const vec4& edgeColor, bool withDepthMask);

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
      // The existing implementation suffers from high temporal artefacts that make it unsuitable for dynamic camera situations
      if (m_stereo3D == STEREO_VR)
         return 0;
      if (m_dynamicAO)
         return 2;
      return IsUsingStaticPrepass() ? 1 : 0; // If AO is static prepass only, and we are running without it, disable AO
   }

   bool UseAnisoFiltering() const;
   void SetAnisoFiltering(bool enable);

   std::shared_ptr<Sampler> GetBallEnvironment() const { return m_ballEnvSampler; }

   BackGlass* m_backGlass = nullptr;

   class SceneLighting
   {
   public:
      SceneLighting(PinTable* const table);
      
      enum class Mode { Table, User, DayNight };
      
      Mode GetMode() const { return m_mode; };
      void SetMode(Mode mode) { if (m_mode == mode) return; m_mode = mode; Update(); };
      float GetUserLightLevel() const { return m_userLightLevel; }
      void SetUserLightLevel(float level) { if (m_userLightLevel == level) return; m_userLightLevel = level; if (m_mode == Mode::User) Update(); };
      float GetLatitude() const { return m_latitude; }
      void SetLatitude(float latitude) { if (m_latitude == latitude) return; m_latitude = latitude; if (m_mode == Mode::DayNight) Update(); };
      float GetLongitude() const { return m_longitude; }
      void SetLongitude(float longitude) { if (m_longitude == longitude) return; m_longitude = longitude; if (m_mode == Mode::DayNight) Update(); };
      
      float GetGlobalEmissionScale() const { return m_emissionScale; }
      
   private:
      void Update();
      
      float m_emissionScale = 0.f;

      PinTable* const m_table;
      Mode m_mode = Mode::Table;
      float m_userLightLevel = 1.f;
      float m_latitude = 0.f;
      float m_longitude = 0.f;
   } m_sceneLighting;

   // Ball rendering
   vector<Light*> m_ballReflectedLights;
   std::shared_ptr<MeshBuffer> m_ballMeshBuffer;
   std::shared_ptr<MeshBuffer> m_ballTrailMeshBuffer;
   #ifdef DEBUG_BALL_SPIN
   std::shared_ptr<MeshBuffer> m_ballDebugPoints;
   #endif
   int m_ballTrailMeshBufferPos = 0;
   bool m_trailForBalls = false;
   float m_ballTrailStrength = 0.5f;
   bool m_overwriteBallImages = false;
   std::shared_ptr<BaseTexture> m_ballImage = nullptr;
   std::shared_ptr<BaseTexture> m_decalImage = nullptr;
   bool m_ballAntiStretch = false;
   bool IsBallLightingDisabled() const;
   void DisableBallLighting(bool disableLightingForBalls);

   // Post processing
   void SetScreenOffset(const float x, const float y); // set render offset in screen coordinates, e.g., for the nudge shake
   bool m_bloomOff = false;
   bool m_motionBlurOff = false;
   int m_FXAA; // =FXAASettings
   int m_sharpen; // 0=off, 1=CAS, 2=bilateral CAS
   bool HasAdditiveScreenSpaceReflection() const { return m_ss_refl; }
   void EnableAdditiveScreenSpaceReflection(bool ssr) { m_ss_refl = ssr; }

   VRPreviewMode m_vrPreview;

   enum RenderMask : unsigned int
   {
      DEFAULT = 0,                // No flag, just render everything
      STATIC_ONLY = 1 << 0,       // Disable non static part rendering (for static prerendering)
      DYNAMIC_ONLY = 1 << 1,      // Disable static part rendering
      LIGHT_BUFFER = 1 << 2,      // Transmitted light rendering
      REFLECTION_PASS = 1 << 3,   // Reflection pass, only render reflected elements
      DISABLE_LIGHTMAPS = 1 << 4, // Disable lightmaps, useful for reflection probe parallel to lightmap ot avoid doubling them
      UI_EDGES = 1 << 5,          // Render as wireframe
      UI_FILL = 1 << 6,           // Render filled interior
   };
   unsigned int m_render_mask = DEFAULT; // Active pass render bit mask
   bool IsRenderPass(const RenderMask pass_mask) const { return (m_render_mask & pass_mask) != 0; }

   ToneMapper m_toneMapper = TM_AGX;
   bool m_HDRforceDisableToneMapper = true;
   float m_exposure = 1.f;

   CGpuProfiler m_gpu_profiler;

   RenderDevice* m_renderDevice = nullptr;

   // free-camera-mode-fly-around parameters
   Vertex3Ds m_cam = Vertex3Ds(0.f, 0.f, 0.f);
   float m_inc = 0.f;

   StereoMode m_stereo3D;
   bool m_stereo3Denabled;
   float m_stereo3DDefocus = 0.f;

   RenderTarget* GetOffscreenVR(int eye) const { return eye == 0 ? m_pOffscreenVRLeft : m_pOffscreenVRRight; }
   RenderTarget* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture1; } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)

   unsigned int GetPlayerModeVisibilityMask() const { return m_visibilityMask; }

private:
   void RenderItem(IEditable* renderable, bool isNoBackdrop);
   void RenderStaticPrepass();
   void RenderDynamics();
   void DrawBackground();
   void DrawBulbLightBuffer();
   bool IsBloomEnabled() const;
   std::shared_ptr<BaseTexture> EnvmapPrecalc(const std::shared_ptr<const BaseTexture>& envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres);

   // Postprocess passes
   void UpdateAmbientOcclusion(RenderTarget* renderedRT);
   void UpdateBloom(RenderTarget* renderedRT);
   RenderTarget* ApplyAdditiveScreenSpaceReflection(RenderTarget* renderedRT);
   ShaderTechniques ApplyTonemapping(RenderTarget* renderedRT, RenderTarget* tonemapRT);
   RenderTarget* ApplyBallMotionBlur(RenderTarget* beforeTonemapRT, RenderTarget* afterTonemapRT, ShaderTechniques tonemapTechnique);
   RenderTarget* ApplyPostProcessedAntialiasing(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);
   RenderTarget* ApplySharpening(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);
   RenderTarget* ApplyUpscaling(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);
   RenderTarget* ApplyStereo(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);

   // Ancillary window rendering
   static void DrawImage(VPXRenderContext2D* ctx, VPXTexture texture, const float tintR, const float tintG, const float tintB, const float alpha, const float texX, const float texY,
      const float texW, const float texH, const float pivotX, const float pivotY, const float rotation, const float srcX, const float srcY, const float srcW, const float srcH);
   static void DrawMatrixDisplay(VPXRenderContext2D* ctx, VPXDisplayRenderStyle style, VPXTexture glassTex, const float glassTintR, const float glassTintG, const float glassTintB,
      const float glassRoughness, const float glassAreaX, const float glassAreaY, const float glassAreaW, const float glassAreaH, const float glassAmbientR, const float glassAmbientG,
      const float glassAmbientB, VPXTexture dispTex, const float dispTintR, const float dispTintG, const float dispTintB, const float brightness, const float alpha, const float dispPadL,
      const float dispPadT, const float dispPadR, const float dispPadB, const float srcX, const float srcY, const float srcW, const float srcH);
   static void DrawSegmentDisplay(VPXRenderContext2D* ctx, VPXSegDisplayRenderStyle style, VPXSegDisplayHint shapeHint, VPXTexture glassTex, const float glassTintR, const float glassTintG,
      const float glassTintB, const float glassRoughness, const float glassAreaX, const float glassAreaY, const float glassAreaW, const float glassAreaH, const float glassAmbientR,
      const float glassAmbientG, const float glassAmbientB, SegElementType type, const float* state, const float dispTintR, const float dispTintG, const float dispTintB,
      const float brightness, const float alpha, const float dispPadL, const float dispPadT, const float dispPadR, const float dispPadB, const float srcX, const float srcY, const float srcW,
      const float srcH);
   RenderTarget* SetupAncillaryRenderTarget(VPXWindowId window, VPX::RenderOutput& output, RenderTarget* embedRT, int& outputX, int& outputY, int& outputW, int& outputH, bool& isOutputLinear);
   void ClearEmbeddedAncillaryWindow(VPXWindowId window, VPX::RenderOutput& output, RenderTarget* embedRT);
   void RenderAncillaryWindow(VPXWindowId window, VPX::RenderOutput& output, RenderTarget* embedRT, const vector<AncillaryRendererDef>& ancillaryWndRenderers);
   std::unique_ptr<RenderTarget> m_ancillaryWndHdrRT[VPXWindowId::VPXWINDOW_Topper + 1];


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
   Matrix3D m_playfieldView; // Store the base playfield view matrix computed at beginning of frame render
   PartGroupData::SpaceReference m_mvpSpaceReference = PartGroupData::SpaceReference::SR_PLAYFIELD;

   bool m_isStaticPrepassDirty = true;
   int m_disableStaticPrepass = 0;
   RenderTarget* m_staticPrepassRT = nullptr;
   unsigned int m_statsDrawnStaticTriangles = 0;
   RenderProbe::ReflectionMode m_maxReflectionMode;
   
   bool m_noBackdrop = false;
   unsigned int m_visibilityMask = 0xFFFF;

   vector<Renderable*> m_renderableToInit;

   bool m_dynamicAO;
   bool m_disableAO;
   std::shared_ptr<Sampler> m_aoDitherSampler = nullptr;

   std::shared_ptr<Sampler> m_envSampler = nullptr;
   std::shared_ptr<Sampler> m_ballEnvSampler = nullptr;

   bool m_ss_refl;

public:
   bool m_vrApplyColorKey = false;
   bool m_vrPreviewShrink = false;
   vec4 m_vrColorKey = vec4(0.f, 0.f, 0.f, 0.f);

private:
   Vertex2D m_ScreenOffset = Vertex2D(0.f, 0.f); // for screen shake effect during nudge

   Texture* m_tonemapLUT = nullptr;

   #if defined(ENABLE_DX9) || defined(__OPENGLES__) || defined(__APPLE__)
   std::shared_ptr<BaseTexture> m_envRadianceTexture = nullptr;
   #else
   RenderTarget* m_envRadianceTexture = nullptr;
   #endif

   // Segment display rendering
   std::unique_ptr<Texture> m_segDisplaySDF[4][9];
public:
   vec4 m_segColor[8]; // Base seg color and brightness
   vec4 m_segUnlitColor[8]; // unlit color and back glow

   // DMD rendering
   bool m_dmdUseLegacyRenderer[7];
   vec4 m_dmdDotColor[7]; // Base dot color and brightness
   vec4 m_dmdDotProperties[7]; // size, sharpness, rounding, back glow
   vec4 m_dmdUnlitDotColor[7]; // unlit color
};
