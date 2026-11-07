// license:GPLv3+

#pragma once

#include "EditorAreaView.h"
#include "BlueprintRenderContext.h"
#include "parts/EditorUIPart.h"
#include "imguizmo/ImGuizmo.h"

class DragPoint;
struct ImDrawList;

namespace VPX::EditorUI
{

// Blueprint pane: a leaf hosting a 2D top-down schematic view of the playfield, rendered
// like the win32 editor's view (filled or textured part shapes under an optional grid,
// black outlines on top, drag point handles on selected parts) and offering the same
// editing interactions: click & box selection, part move drags, drag point and center
// drags, an XY-plane gizmo and part placement.
//
// The view state shared with the win32 editor (display grid, render solid, backdrop image)
// lives on the table so both editors stay in sync; zoom & offset are per-pane and persist
// in the hosting leaf's data payload.
class BlueprintPane final : public EditorAreaView
{
public:
   explicit BlueprintPane(EditorUI& editor);

   AreaType GetType() const override { return AreaType::Blueprint; }
   void RenderPane(const AreaRect& rect, int leafId) override;
   void OnUnhosted() override;

   nlohmann::json SaveSetup() const override;
   void LoadSetup(const nlohmann::json& j) override;

   // Table <-> display px mapping (zoom in px per table unit, offset = table coords at the
   // image center, same convention as the win32 view)
   ImVec2 Project(const Vertex2D& v) const;
   Vertex2D Unproject(const ImVec2& pos) const;
   void FitTable(); // Zoom & offset to fit the playfield bounds
   void FrameSelection();

   // Per-frame interactions (called by the editor when this pane is hovered or capturing):
   // pan/zoom, selection, part & point drags, gizmo, part placement, pane shortcuts
   void RenderUI(ImDrawList* overlayDrawList);

   // Helper messages of the status bar while this pane is the context view (drag in progress or hovered)
   void AppendStatusHints(vector<StatusHint>& hints) const override;

   int m_leafId = -1;
   bool m_hovered = false;
   bool m_active = false;
   ImGuiWindow* m_window = nullptr;
   ImGuiID m_viewItemId = 0; // The pane's canvas item id (the corner zones ignore it as a widget)
   AreaRect m_imageRect {};

   // Selection gizmo state (same convention as ViewportPane)
   ImGuizmo::OPERATION m_gizmoOperation = (ImGuizmo::OPERATION)0;
   ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
   bool m_gizmoDragActive = false;
   vector<std::shared_ptr<EditorUIPart>> m_gizmoDragSelection;
   void SetGizmoOperation(ImGuizmo::OPERATION operation);

private:
   void RenderScene(ImDrawList* drawList); // The blueprint pass: background/backdrop/fills/grid/outlines
   void RenderPart(BlueprintRenderContext& ctx, const std::shared_ptr<EditorUIPart>& part) const;
   void RenderGhostParts(BlueprintRenderContext& ctx) const; // Part library placement ghost preview
   void HitTestParts(const ImVec2& pos, vector<BlueprintHitContext::Hit>& hits) const;
   void BoxSelectParts(const ImVec2& cornerA, const ImVec2& cornerB, bool add);
   void BoxSelectPoints(const ImVec2& cornerA, const ImVec2& cornerB, bool add);
   std::shared_ptr<EditorUIPart> Resolve(const EditorUIPart* part) const;
   void ZoomAt(const ImVec2& pos, float factor);
   void UpdateDraggedPoint(const ImVec2& pos, bool start);

   // Right mouse context menu (win32 parity): RenderUI records the menu target on right-down and
   // arms the popup on right-up, RenderPane opens it (the popup must be in the pane's window scope)
   void RenderContextMenu();
   void RenderDialogs();
   vector<std::shared_ptr<EditorUIPart>> MenuHitParts() const; // Dedup'd parts under the menu position, front to back
   void ApplyTransformDialog();

   bool m_contextMenuPending = false;
   bool m_menuArmed = false; // Right button went down on the canvas (menu opens on release)
   ImVec2 m_menuPos {}; // Screen position the menu was opened at
   vector<BlueprintHitContext::Hit> m_menuHits;
   const EditorUIPart* m_menuPointPart = nullptr; // Owner of the hit drag point handle (drag point menu)
   const DragPoint* m_menuPoint = nullptr;

   // Numeric transform / drawing order popups (win32 context menu commands)
   enum class DialogKind
   {
      None,
      Rotate,
      Scale,
      Translate,
      OrderHit,
      OrderSelect
   };
   DialogKind m_dialog = DialogKind::None;
   bool m_dialogRequest = false; // Deferred OpenPopup in RenderPane (one frame after the menu command)
   int m_dialogApplyCount = 0; // Apply count of the open transform dialog (win32 Apply/Undo/Cancel semantics)
   bool m_dialogInit = false;
   float m_dlgAngle = 0.f, m_dlgSX = 1.f, m_dlgSY = 1.f, m_dlgTX = 0.f, m_dlgTY = 0.f;
   Vertex2D m_dlgPivot {};
   bool m_dlgUseCenter = true, m_dlgSquare = true;
   vector<IEditable*> m_orderList; // Working list of the drawing order dialog (front to back)
   int m_orderIndex = -1;

   float m_zoom = 2.f; // Display px per table unit
   Vertex2D m_offset {}; // Table coordinate at the image center
   bool m_setupValid = false; // No stored setup & no fit done yet

   bool m_boxSelectActive = false;
   ImVec2 m_boxSelectStart;

   // Move drag of the current selection (left drag that started on a selected part)
   bool m_moveDragPending = false;
   bool m_moveDragActive = false;
   Vertex2D m_moveDragPos;

   // Direct drag on a drag point handle or on an edit center (win32-style point dragging)
   bool m_pointDragPending = false;
   bool m_pointDragActive = false;
   std::shared_ptr<EditorUIPart> m_dragPart;
   DragPoint* m_dragPoint = nullptr;
   bool m_dragCenter = false;
   Vertex2D m_dragPos;
};

}
