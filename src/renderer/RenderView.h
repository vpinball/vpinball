// license:GPLv3+

#pragma once

#include <memory>
#include <vector>

#include "math/ModelViewProj.h"
#include "math/matrix.h"
#include "parts/PartGroup.h"
#include "renderer/RenderTarget.h"
#include "renderer/typedefs3D.h"

class Renderer;
class Shader;
class Vertex3D_TexelOnly;
namespace VPX
{
class Window;
}

// A RenderView is one rendering of the scene toward a target: the desktop player window, the VR headset,
// the VR desktop mirror window, or an editor viewport. It owns everything that is view dependent: the
// camera (MVP chain), the render target chain (back buffer ping-pong, MSAA, AO, bloom, postprocess, static
// part prerendering accumulation), the visibility classification, and the enabled pipeline features.
// The Renderer keeps the scene-level state (table, shaders, parts, ancillary outputs) and records one or
// more views into a single RenderFrame each frame.
class RenderView final
{
public:
   enum class Kind
   {
      Player, // Main output view (playfield window, or VR headset)
      VRMirror, // Desktop display scene rendering while playing in VR
      Editor // Offscreen editor viewport
   };

   RenderView(Renderer* renderer, Kind kind);
   ~RenderView();

   bool IsMirror() const { return m_kind == Kind::VRMirror; }

   // Output: a window backbuffer, possibly shared with other views, each view drawing to its own
   // normalized sub area of it (e.g. multiple editor views inside a single window's backbuffer)
   void SetOutputWindow(VPX::Window* wnd) { m_outputWnd = wnd; } // Must be set before the first Render (the target chain is created lazily from it)
   void SetOutputArea(const vec4& area)
   {
      if (!(m_outputArea == area))
      {
         m_outputArea = area;
         InvalidateLayout();
      }
   }
   void SetDeferResize(const bool defer) { m_deferResize = defer; }
   RenderTarget* GetOutput() const;
   bool IsFullOutputArea() const { return m_outputArea.x <= 0.f && m_outputArea.y <= 0.f && m_outputArea.z >= 1.f && m_outputArea.w >= 1.f; }
   vec4 GetOutputAreaClip() const; // Output area in clip space (left, bottom, right, top), matching RenderPass::m_areaOfInterest convention
   void GetOutputAreaSize(int& w, int& h) const; // Output area size in pixels

   // Render size: set together with the output it was computed from and the supersampling factor, then
   // followed by SetupTargets (or picked up by the next ResizeRenderTargets)
   void SetRenderSize(const int width, const int height, const int sourceWidth, const int sourceHeight, const float aaFactor);
   void GetRenderSize(int& w, int& h) const
   {
      w = m_renderWidth;
      h = m_renderHeight;
   }
   void GetRenderSizeAA(int& w, int& h) const
   {
      w = m_pOffscreenBackBufferTexture1->GetWidth();
      h = m_pOffscreenBackBufferTexture1->GetHeight();
   }
   bool IsStereo() const { return m_pOffscreenBackBufferTexture1->m_type == SurfaceType::RT_STEREO; }
   colorFormat GetRenderFormat() const { return m_pOffscreenBackBufferTexture1->GetColorFormat(); }
   int GetDisplayWidth() const; // Playfield display width, taking in consideration stretching applied by some stereo modes
   int GetDisplayHeight() const; // Playfield display height, taking in consideration stretching applied by some stereo modes
   float GetDisplayAspectRatio() const;

   // Pipeline features (what the view's record performs; replaces the VR desktop render's hardcoded differences)
   void SetRenderScene(const bool render) { m_renderScene = render; }
   bool IsRenderingScene() const { return m_renderScene; } // False: nothing is recorded for the view, except that the primary view still
   // clears the output and ticks the game logic while the editor areas own the frame, see RenderView::Render
   void SetRenderOverlay3D(const bool render) { m_renderOverlay3D = render; } // Record LiveUI's 3D overlays (editor gizmos, selection,...)
   void SetRenderLiveUI(const bool render) { m_renderLiveUI = render; } // Record a LiveUI pass into this view's output (or the VR UI surface for the VR view)
   bool RendersLiveUI() const { return m_renderLiveUI; }
   void SetRunGameLogic(const bool run) { m_runGameLogic = run; } // Call Player::UpdateGameLogic mid-record to keep latency low
   void SetApplyPostFX(const bool apply) { m_applyPostFX = apply; } // Run the post process tail (AA / sharpen / upscale / stereo) after tonemapping
   void SetOutputLayer(const int layer) { m_outputLayer = layer; } // If >= 0, only render to this layer of the (stereo) render targets

   // Per view editor state, replacing the global LiveUI mode queries (several editor views may coexist with different setups)
   // The flags feed the backdrop/visibility state derived by each Render (self-healing, no explicit invalidation needed)
   void SetEditorState(const bool editorView, const bool backdropEdit)
   {
      m_editorView = editorView; // Rendered for the live editor: disables the static prepass and applies the editor backdrop rules
      m_editorBackdropEdit = backdropEdit; // The view shows the desktop backdrop edit mode
   }
   bool IsEditorView() const { return m_editorView; }
   bool IsEditorBackdropEdit() const { return m_editorBackdropEdit; }
   void SetViewModeOverride(ViewSetupID mode); // Per-view backdrop mode override (editor preview panes), BG_INVALID = table's mode

   // Scene shading override (editor)
   enum class ShadeMode
   {
      Default,
      Wireframe,
      NoDepthWireframe
   };
   void SetShadeMode(const ShadeMode mode) { m_shadeMode = mode; }
   ShadeMode GetShadeMode() const { return m_shadeMode; }

   // Per-frame derived state, recomputed by each Render from the flags above and the table's view mode
   bool HasNoBackdrop() const { return m_noBackdrop; }
   unsigned int GetVisibilityMask() const { return m_visibilityMask; }

   // Camera state
   const ModelViewProj& GetMVP()
   {
      EnsureLayout();
      return m_mvp;
   }
   void InvalidateLayout() { m_layoutDirty = true; } // Mark the default view setup camera as stale: recomputed lazily by EnsureLayout
   void EnsureLayout(); // Recompute both the base and the current MVP from the default view setup, when dirty
   void SetFlip(ModelViewProj::FlipMode flipMode); // Flip the rendered image horizontally and/or vertically (e.g. for rear projection setups), applied to base MVP
   void SetViewProj(const Matrix3D& view, const Matrix3D& proj); // Override the MVP, applied to the base MVP
   void ClearViewProj()
   {
      m_customViewProj = false;
      InvalidateLayout();
   } // Drop the SetViewProj override, back to the default view setup camera
   void SetReflection(const Matrix3D& reflectionMatrix); // Set a reflection matrix to be applied to the view matrix, used for mirror reflections, applied to current MVP, NOT the base MVP
   void ApplyViewJitter(const float xpixoff, const float ypixoff); // Add a sub pixel offset to the projection, kept across space switches (undone by the opposite offset)
   void SetSpaceReference(PartGroupData::SpaceReference spaceReference, bool force);
   PartGroupData::SpaceReference GetSpaceReference() const { return m_mvpSpaceReference; }
   // Clip plane in playfield space (nullptr to clear), applied in the reference space of the parts rendered next (see SetSpaceReference)
   void SetClipPlane(const vec4* playfieldPlane);
   void SetScreenOffset(const float x, const float y); // Set render offset in screen coordinates, e.g., for the nudge shake
   Vertex3Ds Unproject(const int width, const int height, const Vertex3Ds& point);
   Vertex3Ds Get3DPointFrom2D(const int width, const int height, const Vertex2D& p, float z);
   Vertex2D BackdropToClip(const Vertex2D& pos); // Maps a point in desktop backdrop coordinates (0..EDITOR_BG_WIDTH, 0..EDITOR_BG_HEIGHT) to clip space
   void UpdateBasicShaderMatrix(const Matrix3D& objectTrafo = Matrix3D::MatrixIdentity());
   void UpdateBallShaderMatrix();
   void UpdateDesktopBackdropShaderMatrix(bool basic, bool light, bool flasherDMD);

   // Per view render target chain
   RenderTarget* GetMSAABackBufferTexture() const
   {
      return m_pOffscreenMSAABackBufferTexture ? m_pOffscreenMSAABackBufferTexture : m_pOffscreenBackBufferTexture1;
   } // Main render target, may be MSAA enabled and not suited for sampling, also may have stereo output (2 layers)

   RenderTarget* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture1; } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)
   RenderTarget* GetPreviousBackBufferTexture() const { return m_pOffscreenBackBufferTexture2; } // Same as back buffer but for previous frame
   void SwapBackBufferRenderTargets();
   void ResizeRenderTargets(); // Follows the output size (resizable window, panel resize)

   RenderTarget* GetAORenderTarget(int idx);
   void SwapAORenderTargets();
   void ReleaseAORenderTargets()
   {
      delete m_pAORenderTarget1;
      m_pAORenderTarget1 = nullptr;
      delete m_pAORenderTarget2;
      m_pAORenderTarget2 = nullptr;
   }

   RenderTarget* GetPostProcessRenderTarget1();
   RenderTarget* GetPostProcessRenderTarget2();
   RenderTarget* GetPostProcessRenderTarget3(); // Third post process buffer (SMAA): the previous back buffer when it has the post process size
   RenderTarget* GetPostProcessRenderTarget(RenderTarget* renderedRT);

   RenderTarget* GetReflectionBufferTexture();

   RenderTarget* GetMotionBlurBufferTexture();

   RenderTarget* GetBloomBufferTexture() const { return m_pBloomBufferTexture; }
   RenderTarget* GetBloomTmpBufferTexture() const { return m_pBloomTmpBufferTexture; }

   RenderTarget* GetOffscreenVR(int eye) const { return eye == 0 ? m_pOffscreenVRLeft : m_pOffscreenVRRight; }

   // Static part prerendering (temporal accumulation)
   bool IsUsingStaticPrepass() const;
   bool IsTemporalAccumulationInProgress() const; // True while the static parts prerender is still accumulating samples (this view or any render probe)
   void MarkPrepassDirty() { m_isStaticPrepassDirty = true; } // The accumulated static part prerender is stale: re-accumulate it on the next render
   void ResetStaticPrepass()
   {
      m_isStaticPrepassDirty = true;
      m_staticPrepassAccumCount = 0;
      m_statsDrawnStaticTriangles = 0;
   } // Fully re-evaluate the static prerendering (e.g. after a table switch)
   void SetStaticPrepassDisabled(const bool disable); // Reference counted disable, re-accumulates when re-enabled
   unsigned int GetDrawnStaticTriangles() const { return m_statsDrawnStaticTriangles; }

   // Lazily created render target chain (called once targets size is known), released by destructor
   void SetupTargets(const int width, const int height, const SurfaceType rtType, const colorFormat format, const int nMSAASamples);
   void ReleaseTargets();

   // Records the passes of a full frame rendering the 3D scene to this view's output
   void Render();

   // Pipeline steps driven by the Renderer mid-record (on its active view)
   void DrawBulbLightBuffer();
   RenderTarget* ApplyStereo(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);

private:
   void SetupSpaceViews(); // Define the per reference space view/projection pairs of a render (from the scene layout)
   void RenderStatics(); // Render background, then static parts if IsUsingStaticPrepass
   void RenderDynamics(); // Render static parts if not IsUsingStaticPrepass, then render dynamic parts
   void DrawBackground();

   // Output area mapping of the composition quads drawn on the output render target
   void MapToOutputArea(Vertex3D_TexelOnly* verts) const; // Map quad vertices from view clip space (-1..1) to this view's output area clip space
   void DrawOutputQuad(Shader* shader, RenderTarget* target) const; // Draw a fullscreen quad, mapped to this view's area when target is its output

   // Postprocess passes
   void UpdateAmbientOcclusion(RenderTarget* renderedRT, unsigned int jitterIndex);
   void UpdateBloom(RenderTarget* renderedRT);
   void SetupTonemapping(RenderTarget* renderedRT, RenderTarget* tonemapRT, bool isFullTonemap);
   RenderTarget* ApplyTonemapping(RenderTarget* renderedRT, RenderTarget* tonemapRT);
   RenderTarget* ApplyAdditiveScreenSpaceReflection(RenderTarget* renderedRT);
   RenderTarget* ApplyBallMotionBlur(RenderTarget* beforeTonemapRT, RenderTarget* afterTonemapRT);
   RenderTarget* ApplyPostProcessedAntialiasing(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);
   RenderTarget* ApplySharpening(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);
   RenderTarget* ApplyUpscaling(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);

   Renderer* const m_renderer;
   const Kind m_kind;

   // Output
   VPX::Window* m_outputWnd = nullptr;
   vec4 m_outputArea = vec4(0.f, 0.f, 1.f, 1.f); // Normalized [left, top, right, bottom] area of the output this view draws to
   bool m_deferResize = false; // Keep the current render targets when the output area resizes (e.g. splitter drag): the render is stretched to the new area until the resize is applied
   int m_renderWidth = 0, m_renderHeight = 0; // Render size without supersampling (AAFactor) applied
   float m_aaFactor = 1.f; // Supersampling factor of the render targets (kept through resizes)
   int m_renderSourceWidth = 0, m_renderSourceHeight = 0; // Output size the render size was computed from

   // Pipeline features
   bool m_renderScene = true;
   bool m_renderOverlay3D = false;
   bool m_renderLiveUI = false;
   bool m_runGameLogic = false;
   bool m_applyPostFX = true;
   int m_outputLayer = -1;

   // Per view editor state
   bool m_editorView = false;
   bool m_editorBackdropEdit = false;
   ViewSetupID m_viewModeOverride = BG_INVALID; // Per-view backdrop mode override (editor preview panes), BG_INVALID = table's mode

   // Camera state
   ModelViewProj m_mvp; // Active Model / View / Projection (includes visual nudge)
   ModelViewProj m_initialMVP; // Base playfield MVP in desktop mode (unused in VR)
   PartGroupData::SpaceReference m_mvpSpaceReference = PartGroupData::SpaceReference::Playfield;
   bool m_customViewProj = false; // The camera is driven through SetViewProj (editor panes): it owns the MVP, layout recomputes are skipped
   bool m_layoutDirty = true; // The camera layout inputs (view setup, mode, stereo, output aspect, fly camera) changed since the last EnsureLayout
   Vertex2D m_screenOffset = Vertex2D(0.f, 0.f); // Screen shake effect during nudge

   // Per-frame derived state (see Render)
   unsigned int m_visibilityMask = 0xFFFF;
   bool m_noBackdrop = false;

   ShadeMode m_shadeMode = ShadeMode::Default;

   // Per view render target chain
   RenderTarget* m_pOffscreenMSAABackBufferTexture = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture1 = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture2 = nullptr;
   Matrix3D m_prevFrameViewProj[2]; // View x projection the previous frame's back buffer was rendered with (used to reproject it, e.g. as ball reflection probe)
   bool m_prevFrameViewProjValid = false;

   RenderTarget* m_pAORenderTarget1 = nullptr;
   RenderTarget* m_pAORenderTarget2 = nullptr;

   RenderTarget* m_pPostProcessRenderTarget1 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget2 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget3 = nullptr;

   RenderTarget* m_pReflectionBufferTexture = nullptr;

   RenderTarget* m_pMotionBlurBufferTexture = nullptr;

   RenderTarget* m_pBloomBufferTexture = nullptr;
   RenderTarget* m_pBloomTmpBufferTexture = nullptr;

   RenderTarget* m_pOffscreenVRLeft = nullptr;
   RenderTarget* m_pOffscreenVRRight = nullptr;

   // Static part prerendering (temporal accumulation)
   bool m_isStaticPrepassDirty = true;
   bool m_wasUsingStaticPrepass = false; // Static prepass use on the previous frame (see RenderStatics)
   int m_disableStaticPrepass = 0;
   RenderTarget* m_staticPrepassRT = nullptr;
   int m_staticPrepassAccumCount = 0; // Number of samples accumulated so far in the static parts prerender
   unsigned int m_statsDrawnStaticTriangles = 0;

   // Per reference space view/projection pairs defined once for the current render (indexed by
   // VPX::SpaceReference): all the space switches of a render share the same stable matrices
   Matrix3D m_spaceView[3][2];
   Matrix3D m_spaceProj[3][2];
   Vertex2D m_viewJitter { 0.f, 0.f }; // Sub pixel offset applied on top of the space projections (see ApplyViewJitter)

   bool m_hasReflection = false;
   Matrix3D m_reflection = Matrix3D::MatrixIdentity(); // Playfield space reflection (see SetReflection), applied per reference space by SetSpaceReference

   void ApplyClipPlane(); // Apply m_clipPlane in the current reference space (and under DX9 with the current transforms)
   bool m_hasClipPlane = false;
   vec4 m_clipPlane { 0.f, 0.f, 0.f, 0.f }; // Playfield space clip plane (see SetClipPlane)
};
