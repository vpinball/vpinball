// license:GPLv3+

#pragma once

#include "gpuprofiler.h"
#include "math/matrix.h"
#include "math/ModelViewProj.h"
#include "renderer/Renderable.h"
#include "renderer/RenderDevice.h"
#include "renderer/Texture.h"
#include "plugins/ControllerPlugin.h"
#include "parts/PartGroup.h"
#include "renderer/RenderProbe.h"
#include "renderer/RenderView.h"
#include "renderer/Sampler.h"

class IEditable;
class Renderable;
class PinTable;
class MeshBuffer;
class Material;
class Texture;
enum class ShaderTechnique : unsigned int;

// How the editor UI edge mesh is derived from a part's triangle index list
enum class UIEdgeMode
{
   None, // No edge mesh
   TriWire, // Every triangle edge is drawn (face diagonals included)
   Outline, // Only silhouette and crease edges (edges shared by coplanar triangles are dropped)
   Quads, // Each consecutive pair of triangles is a quad: draw its outline (shared diagonal dropped)
   VertexRing, // A single ring through consecutive vertices (i, i+1) wrapping around
};

// Definition of how a mesh is shaded, resolved by the part on each render (materials and images can be changed during play)
struct VisualMaterial
{
   static constexpr ShaderTechnique AUTO_TECHNIQUE = static_cast<ShaderTechnique>(0xFFFFFFFFu);

   const Material* material = nullptr; // The base material (null only for editor UI passes which do not use it)
   Texture* image = nullptr; // Optional base color texture
   Texture* normalMap = nullptr; // Optional normal map
   bool objectSpaceNormalMap = false;
   ShaderTechnique technique = AUTO_TECHNIQUE; // AUTO_TECHNIQUE: basic_with/without_texture is derived from 'image'
   bool doReflection = false;
   bool doRefraction = false;
   SamplerFilter filter = SamplerFilter::SF_UNDEFINED;
   SamplerAddressMode clampU = SamplerAddressMode::SA_UNDEFINED;
   SamplerAddressMode clampV = SamplerAddressMode::SA_UNDEFINED;
   bool hasTextureAlpha = false;
   bool forceAlphaBlend = false; // Force non-additive alpha blending after material setup (e.g. kicker hole)
};

// Definition of a (sub)mesh to render, with slots holding the editor UI variants that are lazily built by the renderer
struct RenderMeshDesc
{
   std::shared_ptr<MeshBuffer> mesh; // The mesh to render
   RenderDevice::PrimitiveTypes type = RenderDevice::TRIANGLELIST;
   uint32_t startIndex = 0;
   uint32_t indexCount = 0; // 0 = render the mesh's full index buffer
   Vertex3Ds sortCenter; // Center used for transparency sorting
   float depthBias = 0.f;
   const Matrix3D* transform = nullptr; // Optional object transform, applied while drawing
   bool transparent = false; // True to sort the draw with the transparent commands

   std::shared_ptr<MeshBuffer>* uiFillMesh = nullptr; // Optional UI fill mesh (defaults to 'mesh')
   std::shared_ptr<MeshBuffer>* uiEdgeMesh = nullptr; // UI edge mesh, lazily built by the renderer
   UIEdgeMode edgeMode = UIEdgeMode::None;
   const void* edgeTriIndices = nullptr; // Triangle index list used to build the UI edge mesh
   size_t edgeTriCount = 0;
   bool edgeTriIndices16 = false; // True if edgeTriIndices elements are 16 bits (else 32 bits)
   const Vertex3D_NoTex2* edgeVertices = nullptr; // Vertices, required by UIEdgeMode::Outline
};

class Renderer final
{
   // RenderViews record their frame passes through the renderer and therefore need access to its internals
   friend class RenderView;

public:
   Renderer(PinTable* const table, VPX::Window* wnd, VideoSyncMode& syncMode, const StereoMode stereo3D);
   ~Renderer();

   // Scene views rendered each frame: the primary view (playfield window or VR headset), the VR
   // desktop mirror view, and additional views (e.g. editor viewports). Public accessors delegate
   // to the active view, which is the primary view outside of frame recording.
   RenderView* GetActiveView() const { return m_activeView; }
   RenderView* GetPrimaryView() const { return m_primaryView.get(); }
   RenderView* GetVRMirrorView() const { return m_vrMirrorView.get(); }
   const vector<RenderView*>& GetViews() const { return m_renderViews; }
   RenderView* CreateEditorView(); // Registers a Kind::Editor view (an editor viewport drawing to its area of a shared output)
   void ReleaseEditorView(RenderView* view); // Unregisters a view made by CreateEditorView (destroyed at the next frame's record start)

   // Additional views recorded each frame after the primary one (e.g. editor viewports). Their output is
   // defined by the caller: a window backbuffer, a shared render target (each view drawing to its own
   // normalized output area), or an owned offscreen render target.
   RenderView* AddView(RenderView::Kind kind = RenderView::Kind::Editor);
   void RemoveView(RenderView* view);

   void GetRenderSize(int& w, int& h) const { m_activeView->GetRenderSize(w, h); }
   void GetRenderSizeAA(int& w, int& h) const { m_activeView->GetRenderSizeAA(w, h); }
   bool IsStereo() const { return m_activeView->IsStereo(); }
   colorFormat GetRenderFormat() const { return m_activeView->GetRenderFormat(); }

   int GetDisplayWidth() const { return m_activeView->GetDisplayWidth(); } // Playfield display width, taking in consideration stretching applied by some stereo modes
   int GetDisplayHeight() const { return m_activeView->GetDisplayHeight(); } // Playfield display width, taking in consideration stretching applied by some stereo modes
   float GetDisplayAspectRatio() const { return m_activeView->GetDisplayAspectRatio(); }
   void InitLayout(const float xpixoff = 0.f, const float ypixoff = 0.f)
   {
      m_activeView->InitLayout(xpixoff, ypixoff);
   } // Reset both the base and the current MVP to the default view setup, with an optional pixel offset for AA
   void SetFlip(ModelViewProj::FlipMode flipMode)
   {
      m_activeView->SetFlip(flipMode);
   } // Flip the rendered image horizontally and/or vertically (e.g. for rear projection setups), applied to base MVP
   void SetViewProj(const Matrix3D& view, const Matrix3D& proj) { m_activeView->SetViewProj(view, proj); } // Override the MVP, applied to the base MVP
   void SetReflection(const Matrix3D& reflectionMatrix)
   {
      m_activeView->SetReflection(reflectionMatrix);
   } // Set a reflection matrix to be applied to the view matrix, used for mirror reflections, applied to current MVP, NOT the base MVP
   void ApplyViewJitter(const float xpixoff, const float ypixoff)
   {
      m_activeView->ApplyViewJitter(xpixoff, ypixoff);
   } // Apply a sub pixel offset to the projection of the current MVP (same offset as InitLayout but without resetting view and base MVP, allowing to jitter a reflected MVP)
   const ModelViewProj& GetMVP() const { return m_activeView->m_mvp; }
   Vertex3Ds Unproject(const int width, const int height, const Vertex3Ds& point) const { return m_activeView->Unproject(width, height, point); }
   Vertex3Ds Get3DPointFrom2D(const int width, const int height, const Vertex2D& p, float z) { return m_activeView->Get3DPointFrom2D(width, height, p, z); }
   Vertex2D BackdropToClip(const Vertex2D& pos) const
   {
      return m_activeView->BackdropToClip(pos);
   } // Maps a point in desktop backdrop coordinates (0..EDITOR_BG_WIDTH, 0..EDITOR_BG_HEIGHT) to clip space, going through the editor's orthographic camera when editing the backdrop in the live editor, or to the full render target otherwise

   void MarkShaderDirty() { m_shaderDirty = true; }
   void UpdateBasicShaderMatrix(const Matrix3D& objectTrafo = Matrix3D::MatrixIdentity()) { m_activeView->UpdateBasicShaderMatrix(objectTrafo); }
   void UpdateBallShaderMatrix() { m_activeView->UpdateBallShaderMatrix(); }
   void UpdateDesktopBackdropShaderMatrix(bool basic, bool light, bool flasherDMD) { m_activeView->UpdateDesktopBackdropShaderMatrix(basic, light, flasherDMD); }
   void UpdateStereoShaderState();

   void DisableStaticPrePass(const bool disable); // Disable static part prerendering on all views (reference counted)
   bool IsUsingStaticPrepass() const { return m_activeView->IsUsingStaticPrepass(); }
   bool IsTemporalAccumulationInProgress() const
   {
      for (const RenderView* view : m_renderViews)
         if (view->IsTemporalAccumulationInProgress())
            return true;
      return false;
   } // True while the static parts prerender is still accumulating samples (any view or any render probe)
   unsigned int GetNPrerenderTris() const { return m_activeView->m_statsDrawnStaticTriangles; }

   using ShadeMode = RenderView::ShadeMode;
   void SetShadeMode(ShadeMode mode) { m_activeView->m_shadeMode = mode; };
   ShadeMode GetShadeMode() const { return m_activeView->m_shadeMode; };

   void Render3DScene(); // Records the passes of a full frame rendering the 3D scene to the output backbuffer
   void RenderUIScene(); // Records a frame containing only the LiveUI (used while the initial table content is being loaded)

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
   void SetupDisplayRenderer(const bool isBackdrop, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness,
      ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient, const bool unmirror);
   // Display axis to flip for 'unmirror': 0 = u, 1 = v, -1 = none
   int GetDisplayUnmirrorAxis(const bool isBackdrop, const Vertex3D_NoTex2* const vertices, const bool unmirror) const;

public:
   // Rotation of an ancillary window content, in clockwise degrees (0, 90, 180 or 270). The playfield rotates through its view setup instead, so this is only defined for the backglass / scoreview / topper windows
   int GetAncillaryWindowRotation(VPXWindowId window) const;
   void SetAncillaryWindowRotation(VPXWindowId window, int clockwiseDegrees);
   // Content rotation of an ancillary output: an output embedded in the playfield window also turns with the playfield
   int GetAncillaryContentRotation(VPXWindowId window, const VPX::RenderOutput& output) const;
   // Playfield view rotation in the window hosting embedded outputs, to the nearest quarter turn, in clockwise degrees
   int GetEmbeddedOutputRotation() const;
   // Area drawn by the window's renderer on the last frame, normalized to the output (left, top, right, bottom), empty (right <= left) if none
   const vec4& GetAncillaryContentBounds(VPXWindowId window) const { return m_ancillaryContentBounds[window]; }
   void ResetAncillaryContentBounds(VPXWindowId window) { m_ancillaryContentBounds[window] = vec4(0.f, 0.f, 0.f, 0.f); }

   // 'unmirror' flips the display content back in mirror mode so that it stays readable. Only for displays that are scene objects of their
   // own (e.g. flashers): ancillary renderer content is flipped as a whole (see Flasher::Render) and backdrop displays are not mirrored
   void SetupSegmentRenderer(int profile, const bool isBackdrop, const vec3& color, const float brightness, const SegmentFamily family, const SegElementType type, const float* segs,
      const ColorSpace colorSpace, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness, ITexManCacheable* const glassTex,
      const vec4& glassArea, const vec3& glassAmbient, const bool unmirror = false);
   // 'alpha' is plain opacity, only applied by the legacy DMD renderer. 'addBlendModulate' is the signed 'modulate vs
   // add' factor the other ones have to encode their output with, 0 asking for a plain opaque one instead, and the
   // sign selecting whether the background is amplified or absorbed (see fs_display.sc). The caller is the one setting
   // up the blend state matching it, see Flasher::Render
   void SetupDMDRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& dmd, const float alpha,
      const float addBlendModulate, const ColorSpace colorSpace, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness,
      ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient, const bool unmirror = false);
   void SetupCRTRender(int profile, const bool isBackdrop, const vec3& color, const float brightness, const std::shared_ptr<BaseTexture>& crt, const float alpha,
      const float addBlendModulate, const ColorSpace colorSpace, const Vertex3D_NoTex2* vertices, const vec4& emitterPad, const vec3& glassTint, const float glassRougness,
      ITexManCacheable* const glassTex, const vec4& glassArea, const vec3& glassAmbient, const bool unmirror = false);
   void DrawStatics();
   void DrawDynamics(bool onlyBalls);
   void DrawSprite(const float posx, const float posy, const float width, const float height, const COLORREF color, const std::shared_ptr<const Sampler>& tex, const float intensity,
      const bool backdrop = false);
   void DrawWireframe(IEditable* const renderable, const vec4& fillColor, const vec4& edgeColor, bool withDepthMask);
   void RenderMesh(const unsigned int renderMask, RenderMeshDesc& draw, const VisualMaterial& mat);

   void ReinitRenderable(IRenderable* part)
   {
      if (part)
         m_renderableToInit.push_back(part);
   }
   void SetupRenderable(IRenderable* part)
   {
      if (part)
         m_renderableToSetup.push_back(part);
   }
   void DropRenderable(IRenderable* part)
   {
      std::erase(m_renderableToInit, part);
      std::erase(m_renderableToSetup, part);
   }

   void SetTable(PinTable* table); // Rebind to a new table (currently limited to base table / live copy swaps)

   RenderProbe::ReflectionMode GetMaxReflectionMode() const;
   int GetAOMode() const; // 0=Off, 1=Static, 2=Dynamic

   bool UseAnisoFiltering() const;
   void SetAnisoFiltering(bool enable);

   std::shared_ptr<Sampler> GetBallEnvironment() const { return m_ballEnvSampler; }

   class SceneLighting
   {
   public:
      SceneLighting(PinTable* const table);

      enum class Mode
      {
         Table,
         User,
         DayNight
      };

      Mode GetMode() const { return m_mode; };
      void SetMode(Mode mode)
      {
         if (m_mode == mode)
            return;
         m_mode = mode;
         Update();
      };
      float GetUserLightLevel() const { return m_userLightLevel; }
      void SetUserLightLevel(float level)
      {
         if (m_userLightLevel == level)
            return;
         m_userLightLevel = level;
         if (m_mode == Mode::User)
            Update();
      };
      float GetLatitude() const { return m_latitude; }
      void SetLatitude(float latitude)
      {
         if (m_latitude == latitude)
            return;
         m_latitude = latitude;
         if (m_mode == Mode::DayNight)
            Update();
      };
      float GetLongitude() const { return m_longitude; }
      void SetLongitude(float longitude)
      {
         if (m_longitude == longitude)
            return;
         m_longitude = longitude;
         if (m_mode == Mode::DayNight)
            Update();
      };

      float GetGlobalEmissionScale() const { return m_emissionScale; }

      void SetTable(PinTable* table)
      {
         m_table = table;
         Update();
      } // Rebind to another table of the same base/live copy pair

   private:
      void Update();

      float m_emissionScale = 0.f;

      PinTable* m_table;
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
   void SetScreenOffset(const float x, const float y) { m_activeView->SetScreenOffset(x, y); } // set render offset in screen coordinates, e.g., for the nudge shake
   bool m_bloomOff = false;
   bool m_motionBlurOff = false;
   int m_FXAA; // =FXAASettings
   int m_sharpen; // 0=off, 1=CAS, 2=bilateral CAS
   bool HasAdditiveScreenSpaceReflection() const { return m_ss_refl; }
   void EnableAdditiveScreenSpaceReflection(bool ssr) { m_ss_refl = ssr; }

   VRDesktopMode m_vrDesktop = VRDesktopMode::Disabled; // Content shown on the desktop display while playing in VR
   bool NeedsDesktopSceneRender() const
   {
      return m_vrDesktop == VRDesktopMode::All || m_vrDesktop == VRDesktopMode::Playfield;
   } // True when the desktop display shows a dedicated scene render instead of a mirrored eye

   enum RenderMask : unsigned int
   {
      DEFAULT = 0, // No flag, just render everything
      STATIC_ONLY = 1 << 0, // Static part prerendering
      DYNAMIC_ONLY = 1 << 1, // Dynamic part rendering (after a static part prerendering)
      LIGHT_BUFFER = 1 << 2, // Transmitted light rendering
      REFLECTION_PASS = 1 << 3, // Reflection pass, only render reflected elements
      DISABLE_LIGHTMAPS = 1 << 4, // Disable lightmaps, useful for reflection probe parallel to lightmap ot avoid doubling them
      UI_EDGES = 1 << 5, // Render as wireframe
      UI_FILL = 1 << 6, // Render filled interior
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

   RenderTarget* GetOffscreenVR(int eye) const { return m_activeView->GetOffscreenVR(eye); }
   RenderTarget* GetBackBufferTexture() const { return m_activeView->GetBackBufferTexture(); } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)

   unsigned int GetPlayerModeVisibilityMask() const { return m_activeView->m_visibilityMask; }

   // Renders a single part into the current pass, e.g. for editor previews of parts that are not (yet) in the table
   void RenderItem(IEditable* const renderable, bool isNoBackdrop);

   VPXRenderContext2D& GetAncillaryRenderContext(
      VPXWindowId window, float width, float height, bool is2D, bool isOutputLinear, float depthbias, const Matrix3D& displayTransform = Matrix3D::MatrixIdentity());

private:
   void SetSpaceReference(PartGroupData::SpaceReference spaceReference, bool force) { m_activeView->SetSpaceReference(spaceReference, force); }
   bool IsBloomEnabled() const;
   std::shared_ptr<BaseTexture> EnvmapPrecalc(const std::shared_ptr<const BaseTexture>& envTex, const unsigned int rad_env_xres, const unsigned int rad_env_yres);

   // Per view render target accessors, delegated to the active view (used by ancillary and postprocess helpers)
   RenderTarget* GetMSAABackBufferTexture() const { return m_activeView->GetMSAABackBufferTexture(); }
   RenderTarget* GetPreviousBackBufferTexture() const { return m_activeView->GetPreviousBackBufferTexture(); }
   RenderTarget* GetAORenderTarget(int idx) { return m_activeView->GetAORenderTarget(idx); }
   RenderTarget* GetPostProcessRenderTarget1() { return m_activeView->GetPostProcessRenderTarget1(); }
   RenderTarget* GetPostProcessRenderTarget2() { return m_activeView->GetPostProcessRenderTarget2(); }
   RenderTarget* GetPostProcessRenderTarget3() { return m_activeView->GetPostProcessRenderTarget3(); }
   RenderTarget* GetPostProcessRenderTarget(RenderTarget* renderedRT) { return m_activeView->GetPostProcessRenderTarget(renderedRT); }
   RenderTarget* GetReflectionBufferTexture() { return m_activeView->GetReflectionBufferTexture(); }
   RenderTarget* GetMotionBlurBufferTexture() { return m_activeView->GetMotionBlurBufferTexture(); }
   RenderTarget* GetBloomBufferTexture() const { return m_activeView->GetBloomBufferTexture(); }
   RenderTarget* GetBloomTmpBufferTexture() const { return m_activeView->GetBloomTmpBufferTexture(); }

   // Ancillary window rendering

   // Normalized [x1,y1,x2,y2] of each embedded ancillary window composited into the linear render
   // buffer this frame, which ApplyTonemapping regrades without the table's color grade
   vector<vec4> m_embeddedRegions;
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
   RenderTarget* SetupAncillaryRenderTarget(
      VPXWindowId window, const VPX::RenderOutput& output, RenderTarget* embedRT, int& outputX, int& outputY, int& outputW, int& outputH, bool& isOutputLinear);
   void ClearEmbeddedAncillaryWindow(VPXWindowId window, const VPX::RenderOutput& output, RenderTarget* embedRT);
   void DrawEmbeddedQuad(RenderTarget* outputRT, int x, int y, int w, int h, float r, float g, float b);
   void RenderAncillaryWindow(VPXWindowId window, const VPX::RenderOutput& output, RenderTarget* embedRT, const vector<AncillaryRendererDef>& ancillaryWndRenderers);
   std::unique_ptr<RenderTarget> m_ancillaryWndHdrRT[VPXWindowId::VPXWINDOW_Topper + 1];
   bool m_ancillaryWndRendered[VPXWindowId::VPXWINDOW_Topper + 1] = {}; // Whether a renderer claimed the window on the last frame
   bool m_ancillaryWndAdjusted[VPXWindowId::VPXWINDOW_Topper + 1] = {}; // Whether the window's display settings page was open on the last frame
   uint32_t m_ancillaryWndShowTime[VPXWindowId::VPXWINDOW_Topper + 1] = {}; // msec() of the last lazy show (a demotion is only repaired shortly after it)
   int m_ancillaryWndRotation[VPXWindowId::VPXWINDOW_Topper + 1] = {}; // Live rotation of the window content, in clockwise degrees
   vec4 m_ancillaryContentBounds[VPXWindowId::VPXWINDOW_Topper + 1] = {}; // See GetAncillaryContentBounds
   struct AncillaryRenderSetup
   {
      bool isOutputLinear;
      float depthbias;
      Matrix3D displayTransform;
      vec4 contentBounds; // Union of the drawn source rectangles, normalized to the source surface (left, top, right, bottom)
   } m_ancillaryRenderSetup;
   static void AddAncillaryContent(VPXRenderContext2D* ctx, float srcX, float srcY, float srcW, float srcH);
   VPXRenderContext2D m_ancillaryRenderContext;

   bool m_shaderDirty = true;
   void SetupShaders();

   // Views rendered each frame into the single RenderFrame: the primary view (playfield window or
   // VR headset), then the optional VR desktop mirror view and additional views (editor viewports).
   // Owned through the unique_ptr members; m_renderViews is the non-owning render order registry.
   std::unique_ptr<RenderView> m_primaryView;
   std::unique_ptr<RenderView> m_vrMirrorView;
   std::vector<std::unique_ptr<RenderView>> m_extraViews;
   std::vector<RenderView*> m_renderViews;
   std::vector<std::unique_ptr<RenderView>> m_deadViews; // Released views kept alive until the next frame's record starts
   RenderView* m_activeView = nullptr;

   PinTable* m_table;

   RenderProbe::ReflectionMode m_maxReflectionMode;

   vector<IRenderable*> m_renderableToInit;
   vector<IRenderable*> m_renderableToSetup;

   bool m_dynamicAO;
   bool m_disableAO;
   std::shared_ptr<Sampler> m_aoDitherSampler = nullptr;

   std::shared_ptr<Sampler> m_envSampler = nullptr;
   std::shared_ptr<Sampler> m_ballEnvSampler = nullptr;

   bool m_ss_refl;

public:
   bool m_vrApplyColorKey = false;
   bool m_vrDesktopShrink = false;

   float GetVisualNudgeStrength() const { return m_visualNudgeStrength; }
   void SetVisualNudgeStrength(float strength) { m_visualNudgeStrength = strength; }

private:
   void ApplyTableSettings(); // (Re)load the cached table settings (called from ctor and SetTable)

   float m_visualNudgeStrength; // whether to shake the table/screen during nudges and how much

   Texture* m_tonemapLUT = nullptr;

#if defined(ENABLE_DX9) || defined(__OPENGLES__) || defined(__APPLE__) || defined(__EMSCRIPTEN__) || (defined(__ANDROID__) && defined(ENABLE_XR))
   std::shared_ptr<BaseTexture> m_envRadianceTexture = nullptr;
#else
   RenderTarget* m_envRadianceTexture = nullptr;
#endif

   // Segment display rendering, one atlas pair per SegmentFamily and segment layout
   struct SegDisplayAtlas
   {
      std::unique_ptr<Texture> sdf; // segment SDF texture (for magnified rendering)
      std::unique_ptr<Texture> cov; // prefiltered coverage (for minified rendering)
   };
   SegDisplayAtlas m_segDisplay[5][9];
   void GetSegSDF(SegDisplayAtlas& atlas, const char* const name) const;

public:
   vec4 m_segColor[8]; // Base seg color and brightness
   vec4 m_segUnlitColor[8]; // unlit color and back glow

   // DMD rendering
   bool m_dmdUseLegacyRenderer[7];
   // The legacy DMD renderer is the only one available outside of BGFX, whatever the profile setting says
   bool IsLegacyDMDRenderer(const int profile) const
   {
#if defined(ENABLE_BGFX)
      return m_dmdUseLegacyRenderer[profile];
#else
      return true;
#endif
   }
   vec4 m_dmdDotColor[7]; // Base dot color and brightness
   vec4 m_dmdDotProperties[7]; // size, sharpness, rounding, back glow
   vec4 m_dmdUnlitDotColor[7]; // unlit color
};

// Tonemapping parameter helpers shared by the scene tonemapper (RenderView) and the ancillary window HDR output path (Renderer)
void PrecompSplineTonemap(const float displayMaxLum, float out[6]);
void PrecompAgXTonemap(float out[4]);
