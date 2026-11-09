// license:GPLv3+

#pragma once

#include "EditorAreaView.h"
#include "parts/EditorUIPart.h" // ViewMode
#include "imguizmo/ImGuizmo.h"
#include "math/matrix.h"
#include "renderer/Renderer.h" // Renderer::ShadeMode

class DragPoint;
class MeshBuffer;
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
   void CancelGizmoDrag() override;

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
   void AnimateViewTo(PredefinedView view); // Same snap as SetView, animated (gizmo click, numpad 1/3/7, View menu)
   // The pane's view mode is fixed by its area type: there is no camera mode cycle anymore
   void FrameSelection(); // Camera centered on the selection, zoomed to fill the view
   void FrameAll(); // Camera framing the whole table (Home)
   void FrameBounds(const FRect3D& bounds); // Shared bounds fit of FrameSelection and FrameAll
   bool GetNavPivot(vec3& target) const; // Selection center when 'orbit around selection' is on and something is selected
   bool PickDepthPoint(const ImVec2& mousePos, vec3& point); // 3D point under the cursor (front-most hit, else the playfield plane)
   void OrbitCamera(float dx, float dy); // Orbit the pane camera around its target (MMB & view gizmo & renderer inspection)
   void SetFov(float fovY); // Change the vertical FOV keeping the framing (Editor_NavFovY is updated)
   void SetPerspectiveCam(bool perspective); // Switch between perspective and orthographic cameras, keeping the framing

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

   // Helper messages of the status bar while this pane is the context view (active op/drag or hovered)
   void AppendStatusHints(vector<StatusHint>& hints) const override;

   // Tool shelf: the T overlay of this pane's image (edit modes, add part, gizmo orientation).
   // Drawn once per frame for the target viewport pane only (see EditorUI::RenderUI)
   void RenderToolShelf();
   bool IsToolShelfShown() const { return m_showToolShelf; }
   void ToggleToolShelf() { m_showToolShelf = !m_showToolShelf; }
   float GetToolShelfWidth() const { return m_toolShelfWidth; } // Drawn shelf width last frame (0 when hidden)

   int m_leafId = -1; // Leaf hosting this pane, set in RenderPane

   // Image interaction state, set in RenderPane and read by the editor for input routing
   bool m_hovered = false;
   bool m_active = false;
   bool m_flyMode = false; // Fly mode: the pane's 3D view takes the whole window, hiding the editor UI (F shortcut)

   // Measure mode: click-drag on the playfield plane measures a distance (the last measurement stays until a new drag or a mode change)
   bool m_measureMode = false;
   bool m_measureDragging = false;
   bool m_measureValid = false;
   int m_measureAxis = -1; // Axis constraint during a measure drag (0 = X, 1 = Y, -1 = free)
   Vertex3Ds m_measureFrom, m_measureTo;
   void SetMeasureMode(const bool on)
   {
      SetGizmoOperation(ImGuizmo::OPERATION(0)); // The gizmo is inactive while measuring
      m_measureMode = on;
      m_measureDragging = false;
      if (on)
      {
         m_measureValid = false;
         m_measureAxis = -1;
      }
   }
   ImGuiWindow* m_window = nullptr; // This pane's ImGui window (registered to ImGuizmo so it accepts its hover)
   ImGuiID m_viewItemId = 0; // ID of the pane's scene item (the corner zones ignore it as a widget)
   ViewSetupID m_viewMode = BG_DESKTOP; // Backdrop mode of a render preview (desktop / FSS / cabinet)
   AreaRect m_imageRect {}; // Display-space rect of the rendered image, matching the view's output area

   // The pane's camera definition, also used by the editor while this pane is bound to it
   // (m_camMode removed: m_viewType is the mode, fixed at creation)
   bool m_perspectiveCam = true;
   Renderer::ShadeMode m_shadeMode = Renderer::ShadeMode::Default;
   bool m_showSpaceAxes = false; // Show the selection's reference space axes (EditorCam panes, persisted in the pane setup)
   float m_camAspect = 1.f; // Aspect ratio the stored projection was authored for, re-fitted on pane resize
   PredefinedView m_predefinedView = PredefinedView::None;
   bool m_orbitLock = false;
   bool m_orbitDragging = false; // A nav-button drag is orbiting the camera
   bool m_orbitPivotValid = false; // m_orbitPivot is set for the current drag
   vec3 m_orbitPivot { 0.f, 0.f, 0.f }; // Orbit pivot of the current drag (selection center or picked depth point, else the view target)
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
   Matrix3D m_gizmoStartTransform; // Selection transform at gizmo drag start (Adjust Last Operation)
   void SetGizmoOperation(ImGuizmo::OPERATION operation); // Edit mode actions of the tool shelf buttons (and of Esc to get back to select)

   // Blender-style modal transform op (keyboard G/S/R): the selection follows the mouse until
   // LMB/Enter confirms or RMB/Esc cancels and restores the start state. Distinct from the
   // persistent gizmo tool armed by the tool shelf buttons: while active the op owns the pane's
   // input (and the editor's target viewport routing) and the gizmo is not drawn.
   struct TransformOp
   {
      ImGuizmo::OPERATION op = (ImGuizmo::OPERATION)0; // Active operation: TRANSLATE, ROTATE or SCALE (0 = none)
      ImGuizmo::MODE space = ImGuizmo::WORLD; // Constraint space: world (table) axes, or the start transform's basis (local)
      unsigned axisMask = 0x7; // Axes the op currently applies to (bit 0/1/2 = X/Y/Z); restricted to the context axes
      bool constrained = false; // Whether an axis (or plane, with two bits set) constraint is active
      Matrix3D startTransform; // Selection transform snapshot at op start (its position is the pivot)
      Vertex3Ds pivot; // Table-space pivot point
      ImVec2 startMouse; // Display px of the mouse at activation
      Vertex3Ds grabStartPoint; // Mouse ray / pivot view-plane intersection at activation (free grab)
      Vertex3Ds viewDir; // Table-space camera forward at activation (free grab plane and free rotate axis)
      Vertex3Ds constraintPoint; // Delta reference point, re-seeded on constraint changes (axis/plane grab, rotation plane)
      bool constraintPointValid = false;
      float angleOffset = 0.f; // Rotation accumulated across constraint changes (rotate)
      vector<std::shared_ptr<EditorUIPart>> selection; // Part selection snapshot (the op aborts if it changes)
      vector<DragPoint*> pointSelection; // Drag point selection snapshot (drag point edit mode)
      bool centerSelected = false; // Center point selection snapshot (center edit mode)
      bool undoMarked = false; // An undo record was marked at op start (not in inspect sessions)
      size_t undoTop = 0; // Undo record count right after marking (our record is on top while it matches)
      bool changed = false; // The op actually moved the selection (a no-op confirm discards the undo record)
      bool duplicate = false; // The op follows a Duplicate (Shift+D): its record is folded into the paste's on confirm
      Vertex3Ds duplicateOrigin { 0.f, 0.f, 0.f }; // Position of the selection before the duplicate's grab
      bool snapPointsBuilt = false; // m_snapPoints holds the point-snap candidates (built on first snapped move)
      vector<Vertex2D> snapPoints; // Point snapping: drag points and centers of the unselected parts
      // Numeric delta entry (Blender: digits edit the delta fields, Tab cycles the allowed ones)
      bool numUsed = false; // Any delta field was typed into: parsed values replace the mouse delta
      int numField = 0; // Field currently edited (grab & scale: allowed axes; rotate: field 0)
      string numText[3]; // Per-field text being typed (grab & scale; rotate uses field 0)
      float delta[3] = { 0.f, 0.f, 0.f }; // Last applied delta for the status readout (grab: offset; scale: factors; rotate: field 0 = degrees)
   };
   TransformOp m_op; // Active while m_op.op != 0
   void BeginTransformOp(ImGuizmo::OPERATION operation); // Arm a modal op on the current selection (no-op when it has no transform)
   void EndTransformOp(bool confirm); // Apply (keep the undo record) or cancel (restore the snapshot, discard the record)
   void UpdateTransformOp(); // Per-frame modal update: evaluate the constraint keys & mouse/numeric delta, apply it

private:
   RenderView* m_view = nullptr; // Owned through the renderer's extra views while set

   // Modal transform helpers
   unsigned ContextAxisMask() const; // Axes the modal ops may use in this pane's context (point edit, backdrop, predefined view)
   unsigned SelectionAxisMask(ImGuizmo::OPERATION op) const; // Axes the op may use on the selected parts (surface: rotate Z only, no scale Z)
   int AxisIndex(unsigned mask) const; // First set axis index of a single-axis mask (-1 when not exactly one axis)
   Vertex3Ds OpAxisDir(int axis) const; // Constraint axis direction in table space (world axis or start transform's basis column)
   bool RayPlaneHit(const ImVec2& pos, const Vertex3Ds& planePt, const Vertex3Ds& normal, Vertex3Ds& hit) const; // Mouse ray / plane intersection
   Vertex3Ds ClosestPointOnAxis(const ImVec2& pos, const Vertex3Ds& axisDir) const; // Point of the pivot axis line closest to the mouse ray
   void ReseedOpConstraint(); // Re-seed the delta reference after a constraint change (grab axis/plane, rotation plane)
   void ApplyTransformOpDelta(); // Compute the op delta from the mouse or the typed values, apply it, update the readout

   // Overlay info text drawn at the top left of the pane's image (camera & gizmo status)
   void RenderStatusOverlay();

   // View orientation gizmo (Blender navball): an XYZ tripod at the top right of the pane's image.
   // A LMB drag orbits the camera (independently of the orbit lock, which only remaps MMB), a click
   // on an axis end animates the camera to that axis aligned predefined view. EditorCam panes only,
   // and not while flying (the pane's image is off screen then)
   bool m_viewGizmoDragging = false; // LMB drag in progress (orbiting)
   ImVec2 m_viewGizmoPress; // Mouse position at drag start (click/drag disambiguation)
   int m_viewGizmoPressEnd = -1; // Axis end under the press (0..5 = +X,-X,+Y,-Y,+Z,-Z; -1 = none)
   bool m_viewGizmoHover = false; // Mouse inside the gizmo's backing circle this frame
   int m_viewGizmoHoverEnd = -1; // Axis end currently hovered (for its -X label and highlight)
   // View snap animation: the camera dir/up is lerped toward the clicked end's orientation
   int m_viewAnimFrames = 0;
   vec3 m_viewAnimDir, m_viewAnimUp;
   PredefinedView m_viewAnimTarget = PredefinedView::None;
   void RenderViewGizmo(ImDrawList* drawList); // Draw and input, called from RenderUI (EditorCam only)
   void DragCamera(ImVec2 drag, bool pan);
   void ZoomCamera(float factor);
   void TwistCamera(float degrees);
   static void GetViewOrientation(PredefinedView view, vec3& dir, vec3& up); // Orbit dir/up of a predefined view

   // Reference space axes (an axes/plane reference of a reference space: playfield bounds & glass
   // box, or infinite room axes — the cabinet & cabinet feet spaces share the room transform in
   // live edit as they only differ by the user space settings which are not applied there)
   struct SpaceAxes
   {
      std::shared_ptr<MeshBuffer> mesh;
      float bounds[6] = { 0.f, 0.f, 0.f, 0.f, 0.f, 0.f }; // Playfield bounds the mesh was built for (left, top, right, bottom, glass bottom/top)
      uint32_t ranges[14] = { 0 }; // Index ranges of the 7 segments (square, X, Y, glass, Z, -X, -Y)
      Vertex3Ds center { 0.f, 0.f, 0.f };
   };
   const SpaceAxes* EnsureSpaceAxesMesh(PartGroupData::SpaceReference space); // (Re)build the reference mesh when the space or the table bounds changed
   SpaceAxes m_spaceAxes[2]; // [0] playfield space, [1] room space (also cabinet & cabinet feet)

   // Tool shelf (T): the View menu / T toggle, and its drawn size which keeps the status info text next to it
   bool ToolShelfVisible() const; // Shelf enabled and allowed in the current state (it may still not fit, see m_toolShelfDrawn)
   bool m_showToolShelf = true; // View > Tool Shelf
   bool m_toolShelfDrawn = false; // The shelf was drawn last frame
   float m_toolShelfWidth = 0.f; // Drawn shelf width last frame (0 when hidden)
   // Tooltip with a title, an optional shortcut chip and an optional dim description (works on disabled items)
   static void Tip(const char* title, const char* shortcut = nullptr, const char* description = nullptr);

   // Box selection drag state (a left button drag that hit no part nor drag point)
   bool m_boxSelectActive = false;
   ImVec2 m_boxSelectStart;
   void BoxSelectParts(const ImVec2& cornerA, const ImVec2& cornerB, bool add);
   void BoxSelectPoints(const ImVec2& cornerA, const ImVec2& cornerB, bool add);
};

}
