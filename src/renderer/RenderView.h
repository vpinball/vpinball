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

   // Renderer drives the view recording and may need its private pipeline steps
   friend class Renderer;

   Renderer* const m_renderer;
   const Kind m_kind;
   bool IsMirror() const { return m_kind == Kind::VRMirror; }

   // Output: either a window backbuffer (player / VR mirror), an owned offscreen render target (editor), or
   // a render target shared with other views, each view drawing to its own normalized sub area of it
   // (e.g. multiple editor views inside a single window's backbuffer)
   VPX::Window* m_outputWnd = nullptr;
   RenderTarget* m_sharedOutputRT = nullptr;
   std::unique_ptr<RenderTarget> m_ownedOutputRT;
   vec4 m_outputArea = vec4(0.f, 0.f, 1.f, 1.f); // Normalized [left, top, right, bottom] area of the output this view draws to
   void SetOutputArea(const vec4& area) { m_outputArea = area; }
   bool m_deferResize = false; // Keep the current render targets when the output area resizes (e.g. splitter drag): the render is stretched to the new area until the resize is applied
   RenderTarget* GetOutput() const;
   bool IsFullOutputArea() const { return m_outputArea.x <= 0.f && m_outputArea.y <= 0.f && m_outputArea.z >= 1.f && m_outputArea.w >= 1.f; }
   vec4 GetOutputAreaClip() const; // Output area in clip space (left, bottom, right, top), matching RenderPass::m_areaOfInterest convention
   void GetOutputAreaSize(int& w, int& h) const; // Output area size in pixels
   int m_renderWidth = 0, m_renderHeight = 0; // Render size without supersampling (AAFactor) applied
   float m_aaFactor = 1.f; // Supersampling factor of the render targets (kept through resizes)
   int m_renderSourceWidth = 0, m_renderSourceHeight = 0; // Output size the render size was computed from

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
   bool m_renderOverlay3D = false; // Record LiveUI's 3D overlays (editor gizmos, selection,...)
   bool m_renderLiveUI = false; // Record a LiveUI pass into this view's output (or the XR UI layer for the VR view)
   bool m_runGameLogic = false; // Call Player::UpdateGameLogic mid-record to keep latency low
   bool m_applyPostFX = true; // Run the post process tail (AA / sharpen / upscale / stereo) after tonemapping
   int m_outputLayer = -1; // If >= 0, only render to this layer of the (stereo) render targets
   bool m_renderScene = true; // Record the scene passes (false: nothing is recorded for the view, except that the primary view still
   // clears the output and ticks the game logic while the editor areas own the frame, see RenderView::Render)

   // Per view editor state, replacing the global LiveUI mode queries (several editor views may coexist with different setups)
   bool m_editorView = false; // Rendered for the live editor: disables the static prepass and applies the editor backdrop rules
   ViewSetupID m_viewModeOverride = BG_INVALID; // Per-view backdrop mode override (editor preview panes), BG_INVALID = table's mode
   bool m_editorBackdropEdit = false; // The view shows the desktop backdrop edit mode

   // Camera state
   ModelViewProj m_mvp; // Active Model / View / Projection (includes visual nudge)
   PartGroupData::SpaceReference m_mvpSpaceReference = PartGroupData::SpaceReference::SR_PLAYFIELD;
   ModelViewProj m_initialMVP; // Base playfield MVP in desktop mode (unused in VR)
   bool m_customViewProj = false; // The camera is driven through SetViewProj (editor panes): InitLayout keeps it instead of the default view setup
   Matrix3D m_playfieldView[2]; // Base playfield view matrices of m_initialMVP from which all views are derived in desktop mode, or the cabinet view for the VR desktop display scene render
   Vertex2D m_screenOffset = Vertex2D(0.f, 0.f); // Screen shake effect during nudge
   const ModelViewProj& GetMVP() const { return m_mvp; }
   void InitLayout(const float xpixoff = 0.f, const float ypixoff = 0.f); // Reset both the base and the current MVP to the default view setup, with an optional pixel offset for AA
   void SetFlip(ModelViewProj::FlipMode flipMode); // Flip the rendered image horizontally and/or vertically (e.g. for rear projection setups), applied to base MVP
   void SetViewProj(const Matrix3D& view, const Matrix3D& proj); // Override the MVP, applied to the base MVP
   void SetReflection(const Matrix3D& reflectionMatrix); // Set a reflection matrix to be applied to the view matrix, used for mirror reflections, applied to current MVP, NOT the base MVP
   void ApplyViewJitter(const float xpixoff, const float ypixoff); // Apply a sub pixel offset to the projection of the current MVP
   void SetSpaceReference(PartGroupData::SpaceReference spaceReference, bool force);
   void SetScreenOffset(const float x, const float y); // Set render offset in screen coordinates, e.g., for the nudge shake
   Vertex3Ds Unproject(const int width, const int height, const Vertex3Ds& point) const;
   Vertex3Ds Get3DPointFrom2D(const int width, const int height, const Vertex2D& p, float z);
   Vertex2D BackdropToClip(const Vertex2D& pos) const; // Maps a point in desktop backdrop coordinates (0..EDITOR_BG_WIDTH, 0..EDITOR_BG_HEIGHT) to clip space
   void UpdateBasicShaderMatrix(const Matrix3D& objectTrafo = Matrix3D::MatrixIdentity());
   void UpdateBallShaderMatrix();
   void UpdateDesktopBackdropShaderMatrix(bool basic, bool light, bool flasherDMD);

   // View classification
   unsigned int m_visibilityMask = 0xFFFF;
   bool m_noBackdrop = false;

   // Scene shading override (editor)
   enum class ShadeMode
   {
      Default,
      Wireframe,
      NoDepthWireframe
   };
   ShadeMode m_shadeMode = ShadeMode::Default;

   // Per view render target chain
   RenderTarget* m_pOffscreenMSAABackBufferTexture = nullptr;
   RenderTarget* GetMSAABackBufferTexture() const
   {
      return m_pOffscreenMSAABackBufferTexture ? m_pOffscreenMSAABackBufferTexture : m_pOffscreenBackBufferTexture1;
   } // Main render target, may be MSAA enabled and not suited for sampling, also may have stereo output (2 layers)

   RenderTarget* m_pOffscreenBackBufferTexture1 = nullptr;
   RenderTarget* m_pOffscreenBackBufferTexture2 = nullptr;
   RenderTarget* GetBackBufferTexture() const { return m_pOffscreenBackBufferTexture1; } // Main render target, with MSAA resolved if any, also may have stereo output (2 viewports)
   RenderTarget* GetPreviousBackBufferTexture() const { return m_pOffscreenBackBufferTexture2; } // Same as back buffer but for previous frame
   void SwapBackBufferRenderTargets();
   void ResizeRenderTargets(); // Follows the output size (resizable window, panel resize)

   RenderTarget* m_pAORenderTarget1 = nullptr;
   RenderTarget* m_pAORenderTarget2 = nullptr;
   RenderTarget* GetAORenderTarget(int idx);
   void SwapAORenderTargets();
   void ReleaseAORenderTargets()
   {
      delete m_pAORenderTarget1;
      m_pAORenderTarget1 = nullptr;
      delete m_pAORenderTarget2;
      m_pAORenderTarget2 = nullptr;
   }

   RenderTarget* m_pPostProcessRenderTarget1 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget2 = nullptr;
   RenderTarget* m_pPostProcessRenderTarget3 = nullptr;
   RenderTarget* GetPostProcessRenderTarget1();
   RenderTarget* GetPostProcessRenderTarget2();
   RenderTarget* GetPostProcessRenderTarget3(); // Third post process buffer (SMAA): the previous back buffer when it has the post process size
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
   RenderTarget* GetOffscreenVR(int eye) const { return eye == 0 ? m_pOffscreenVRLeft : m_pOffscreenVRRight; }

   // Static part prerendering (temporal accumulation)
   bool m_isStaticPrepassDirty = true;
   bool m_wasUsingStaticPrepass = false; // Static prepass use on the previous frame (see RenderStatics)
   int m_disableStaticPrepass = 0;
   RenderTarget* m_staticPrepassRT = nullptr;
   int m_staticPrepassAccumCount = 0; // Number of samples accumulated so far in the static parts prerender
   unsigned int m_statsDrawnStaticTriangles = 0;
   bool IsUsingStaticPrepass() const;
   bool IsTemporalAccumulationInProgress() const; // True while the static parts prerender is still accumulating samples (this view or any render probe)

   // Lazily created render target chain (called once targets size is known), released by destructor
   void SetupTargets(const int width, const int height, const SurfaceType rtType, const colorFormat format, const int nMSAASamples);
   void ReleaseTargets();

   // Records the passes of a full frame rendering the 3D scene to this view's output
   void Render();

private:
   void RenderStatics(); // Render background, then static parts if IsUsingStaticPrepass
   void RenderDynamics(); // Render static parts if not IsUsingStaticPrepass, then render dynamic parts
   void DrawBackground();
   void DrawBulbLightBuffer();

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
   RenderTarget* ApplyStereo(RenderTarget* renderedRT, RenderTarget* outputBackBuffer);
};
