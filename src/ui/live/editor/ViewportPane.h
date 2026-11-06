// license:GPLv3+

#pragma once

#include "EditorAreaView.h"
#include "parts/EditorUIPart.h" // ViewMode
#include "imguizmo/ImGuizmo.h"
#include "math/matrix.h"
#include "renderer/Renderer.h" // Renderer::ShadeMode

class DragPoint;
class RenderView;
struct HitTestResult;
struct ImDrawList;

namespace VPX::EditorUI
{

class LiveRenderContext;

// 3D View pane: a leaf hosting a live render of the table. Each instance owns its own
// camera and rendering setup through a dedicated RenderView registered to the renderer
// while the leaf is shown, drawing straight to the pane's area of the shared backbuffer.
//
// The pane owning the current interaction (hovered or captured) processes the 3D scene
// interactions: camera moves, picking, box & point drags, the selection gizmo and the
// viewport shortcuts are all handled by RenderUI through this pane's camera.
// A pane bound for the first time adopts the camera of the pane it replaced (split
// panes clone the active camera, like Blender) or a default playfield view.
class ViewportPane final : public EditorAreaView
{
public:
   explicit ViewportPane(EditorUI& editor, ViewMode viewType = ViewMode::EditorCam);
   ~ViewportPane() override;

   // The area type matching the pane's fixed view mode (a type switch must recreate the pane)
   AreaType GetType() const override
   {
      return m_viewType == ViewMode::DesktopBackdrop ? AreaType::ViewportBackdrop : m_viewType == ViewMode::PreviewCam ? AreaType::ViewportPreview : AreaType::Viewport;
   }
   void RenderPane(const AreaRect& rect, int leafId) override;
   void OnUnhosted() override;

   // Renders the scene parts for this pane's render pass (called by the editor while this pane's
   // render view is being recorded): filtered by the pane's own camera mode and shading setup
   void Render3D();

   RenderView* GetView() const { return m_view; }
   void SetRendering(const bool on)
   {
      if (m_view != nullptr)
         m_view->m_renderScene = on;
   }

   // The pane's camera definition (mode, pose and shading), owned by the pane
   enum class PredefinedView
   {
      None,
      Front,
      Back,
      Right,
      Left,
      Top,
      Bottom
   };

   // Camera actions on this pane's own camera definition
   void AdoptCameraFrom(const ViewportPane* src); // Clone the camera of another pane (or a default playfield view when none)
   void ResetCameraFromPlayer();
   void SetBackdropCamera();
   void SetPlayfieldCamera();
   void SetView(PredefinedView view); // Axis aligned view, keeping the orbit target and distance
   // The pane's view mode is fixed by its area type: there is no camera mode cycle anymore
   void FrameSelection(); // Camera centered on the selection, zoomed to fill the view

   // 3D view setup of the pane (camera mode and pose, shading, orbit lock), serialized into the
   // hosting leaf's persisted payload so it is restored with the layout on the next session
   nlohmann::json SaveSetup() const override;
   void LoadSetup(const nlohmann::json& j) override; // No-op on missing or invalid data (keeps the defaults)

   // Screen <-> table-space mapping of this pane, in display pixels over m_imageRect, from the
   // camera matrices the pane renders with, so picking, the 2D overlays and the gizmo always
   // agree with what is on screen
   bool GetViewProjLH(Matrix3D& view, Matrix3D& proj) const; // Camera this pane renders with
   ImVec2 Project(const Vertex3Ds& v) const; // Table-space point -> display px (FLT_MAX when behind the camera)
   bool PickRay(const ImVec2& pos, Vertex3Ds& nearPt, Vertex3Ds& farPt) const; // Display px -> world ray
   void RayCastParts(const ImVec2& mousePos, vector<HitTestResult>& vhoHit); // Pick the colliders under the mouse (filtered by pickability)
   Vertex2D UnprojectToPlane(const ImVec2& mousePos, float z) const; // Display px -> table plane point at the given Z
   DragPoint* HitTestDragPoint(const ImVec2& mousePos) const; // Drag point of the edited curve under the mouse
   bool HitTestEditCenter(const ImVec2& mousePos) const; // Is the mouse on the edited part's center point
   bool SetupOverlay(LiveRenderContext* ctx, ImDrawList* drawList) const; // Pane projection + image clip rect, false when it cannot project

   // Per-frame processing of the "edit viewport" (called by the editor on the bound pane):
   // camera update, viewport keyboard shortcuts, selection gizmo & overlays and scene mouse
   // interaction, all through the pane projection (see SetupOverlay / PickRay)
   void RenderUI(ImDrawList* overlayDrawList);

   int m_leafId = -1; // Leaf hosting this pane, set in RenderPane

   // Image interaction state, set in RenderPane and read by the editor for input routing
   bool m_hovered = false;
   bool m_active = false;
   bool m_flyMode = false; // Fly mode: the pane's 3D view takes the whole window, hiding the editor UI (F shortcut)
   ImGuiWindow* m_window = nullptr; // This pane's ImGui window (registered to ImGuizmo so it accepts its hover)
   ImGuiID m_viewItemId = 0; // ID of the pane's scene item (the corner zones ignore it as a widget)
   ViewSetupID m_viewMode = BG_DESKTOP; // Backdrop mode of a render preview (desktop / FSS / cabinet)
   AreaRect m_imageRect {}; // Display-space rect of the rendered image, matching the view's output area

   // The pane's camera definition, also used by the editor while this pane is bound to it
   // (m_camMode removed: m_viewType is the mode, fixed at creation)
   bool m_perspectiveCam = true;
   Renderer::ShadeMode m_shadeMode = Renderer::ShadeMode::Default;
   float m_camAspect = 1.f; // Aspect ratio the stored projection was authored for, re-fitted on pane resize
   PredefinedView m_predefinedView = PredefinedView::None;
   bool m_orbitLock = false;
   Matrix3D m_camView, m_camProj;
   float m_camDistance = 1000.f;
   const ViewMode m_viewType; // The pane's area type: 3D Viewport (EditorCam), Desktop Backdrop or Render Preview — fixed at creation
   bool m_cameraValid = false;

   // Selection gizmo state: the armed edit operation (0 = select), its coordinate mode, and the
   // in-progress drag bookkeeping (a drag records a single undo step on the selection it started on)
   ImGuizmo::OPERATION m_gizmoOperation = (ImGuizmo::OPERATION)0;
   ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
   bool m_gizmoDragActive = false;
   vector<std::shared_ptr<EditorUIPart>> m_gizmoDragSelection;
   void SetGizmoOperation(ImGuizmo::OPERATION operation); // Edit mode actions of the Esc (select), G (grab), S (scale) and R (rotate) shortcuts

private:
   RenderView* m_view = nullptr; // Owned through the renderer's extra views while set

   // Overlay info text drawn at the top left of the pane's image (camera & gizmo status)
   void RenderStatusOverlay();

   // Box selection drag state (a left button drag that hit no part nor drag point)
   bool m_boxSelectActive = false;
   ImVec2 m_boxSelectStart;
   void BoxSelectParts(const ImVec2& cornerA, const ImVec2& cornerB, bool add);
   void BoxSelectPoints(const ImVec2& cornerA, const ImVec2& cornerB, bool add);
};

}
