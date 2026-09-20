// license:GPLv3+

#pragma once

#include "input/InputManager.h"
#include "core/pinundo.h"
#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"
#include "editor/EditorUIPart.h"
#include "renderer/Renderer.h"
#include "unordered_dense.h"
#include <variant>

class LiveUI;
class PinTable;
class Player;
class InputManager;
class Renderer;
class DragPoint;
class Sampler;

namespace VPX::EditorUI
{

class EditorUI final
   : private DragPointEditContext
{
public:
   EditorUI(LiveUI &liveUI);
   ~EditorUI();

   void Open();
   bool IsOpened() const { return m_isOpened; }
   void Render3D();
   void RenderUI();
   void Close();
   
   bool IsPreview() const { return m_camMode == ViewMode::PreviewCam; }
   bool IsBackdropEditMode() const { return m_camMode == ViewMode::DesktopBackdrop; }

   bool IsInspectMode() const { return m_table->m_liveBaseTable != nullptr; }

private:
   // UI Context
   LiveUI &m_liveUI;
   Player *m_player;
   PinTable *m_table; // The table displayed by the player
   InputManager *m_pininput;
   std::unique_ptr<Renderer>& m_renderer;
   struct Selection
   {
      enum SelectionType
      {
         S_NONE,
         S_CAMERA,
         S_MATERIAL,
         S_IMAGE,
         S_EDITABLE,
         S_RENDERPROBE
      };
      struct CameraSel
      {
         int viewSetup;
         bool operator==(const CameraSel &) const = default;
      };

      // Alternative order must match SelectionType (GetType relies on it)
      using Payload = std::variant<std::monostate, CameraSel, Material *, Texture *, std::shared_ptr<EditorUIPart>, RenderProbe *>;
      static_assert(std::variant_size_v<Payload> == S_RENDERPROBE + 1);
      Payload payload;

      Selection() = default;
      Selection(Material *material)
         : payload(material)
      {
      }
      Selection(Texture *image)
         : payload(image)
      {
      }
      Selection(const std::shared_ptr<EditorUIPart> &uiPart)
         : payload(uiPart)
      {
      }
      Selection(RenderProbe *probe)
         : payload(probe)
      {
      }
      static Selection Camera(int viewSetup)
      {
         Selection sel;
         sel.payload = CameraSel { viewSetup };
         return sel;
      }

      SelectionType GetType() const { return static_cast<SelectionType>(payload.index()); }
      int GetCamera() const
      {
         const CameraSel *cam = std::get_if<CameraSel>(&payload);
         return cam ? cam->viewSetup : -1;
      }
      Material *GetMaterial() const
      {
         const auto *p = std::get_if<Material *>(&payload);
         return p ? *p : nullptr;
      }
      Texture *GetImage() const
      {
         const auto *p = std::get_if<Texture *>(&payload);
         return p ? *p : nullptr;
      }
      std::shared_ptr<EditorUIPart> GetPart() const
      {
         const auto *p = std::get_if<std::shared_ptr<EditorUIPart>>(&payload);
         return p ? *p : nullptr;
      }
      RenderProbe *GetProbe() const
      {
         const auto *p = std::get_if<RenderProbe *>(&payload);
         return p ? *p : nullptr;
      }

      bool operator==(const Selection &) const = default;
   } m_selection;

   // Multi selection of editable parts: all currently selected parts, with m_selection.GetPart() being the
   // active one (property pane target and gizmo pivot). Empty unless m_selection.GetType() == S_EDITABLE.
   vector<std::shared_ptr<EditorUIPart>> m_multiSel;
   std::shared_ptr<EditorUIPart> m_outlinerAnchor; // Anchor part for shift+click range selection in the outliner
   bool m_boxSelectActive = false;
   ImVec2 m_boxSelectStart;
   bool IsPartSelected(const std::shared_ptr<EditorUIPart> &part) const;
   bool IsEditableSelected(const IEditable *editable) const;
   void ClearSelection();
   void SetSelection(const Selection &selection);
   void TogglePartSelection(const std::shared_ptr<EditorUIPart> &part);
   void SelectOutlinerRange(const std::shared_ptr<EditorUIPart> &part);
   void RayCastParts(const ImVec2 &mousePos, vector<HitTestResult> &vhoHit) const;
   bool IsEditablePickable(const IEditable *editable) const;
   void BoxSelectParts(const ImVec2 &cornerA, const ImVec2 &cornerB, bool add);
   void SelectAllParts();

   // Selection snapshot stored in undo records and restored on undo
   struct UndoSelectionState
   {
      Selection selection;
      vector<std::shared_ptr<EditorUIPart>> multiSel;
      std::shared_ptr<EditorUIPart> outlinerAnchor;
      std::shared_ptr<EditorUIPart> pointEditPart; // Part in drag point edit mode, nullptr when not in that mode
      vector<int> pointSel; // Indices of the selected points in the edited part's curve
   };
   UndoSelectionState CaptureUndoSelection() const;
   void RestoreUndoSelection(const UndoSelectionState &state);

   // Drag point edit mode (entered/exited with Tab when the active selected part has a DragPointCurve):
   // while active, the part's curve points are rendered and can be selected & transformed in the table XY plane
   std::shared_ptr<EditorUIPart> m_pointEditPart; // Part whose DragPointCurve is being edited (nullptr when not in point edit mode)
   vector<DragPoint *> m_pointSel; // Selected drag points of the edited part's curve
   Selection m_savedSelection; // Selection state saved on mode entry, restored on exit
   vector<std::shared_ptr<EditorUIPart>> m_savedMultiSel;
   std::shared_ptr<EditorUIPart> m_savedOutlinerAnchor;
   bool m_pointDragPending = false; // Left button is down on a selected point (drag not started yet)
   bool m_pointDragActive = false; // Left button drag is moving the selected points
   float m_pointDragZ = 0.f; // Table Z coordinate of the plane in which points are dragged
   Vertex2D m_pointDragPos; // Last drag position in table coordinates
   void EnterPointEditMode();
   void ExitPointEditMode(bool restoreSelection);
   bool IsPointSelected(const DragPoint *point) const;
   void TogglePointSelection(DragPoint *point);
   DragPoint *HitTestDragPoint(const ImVec2 &mousePos) const;
   void BoxSelectPoints(const ImVec2 &cornerA, const ImVec2 &cornerB, bool add);
   Vertex2D UnprojectToPlane(const ImVec2 &mousePos, float z) const;
   void AddPointOnNearestSegment();
   void DeleteSelectedPoints();

   // DragPointEditContext implementation
   const vector<DragPoint *> &GetSelectedPoints() const override { return m_pointSel; }
   void BeginPointEdit() override;
   void EndPointEdit() override;

   // Decorated editable parts (kept sorted for the outliner, and indexed by editable)
   vector<std::shared_ptr<EditorUIPart>> m_editables;
   ankerl::unordered_dense::map<const IEditable *, std::shared_ptr<EditorUIPart>> m_editableMap;
   void UpdateEditableList();

   // Main UI frame & panels
   void UpdateOutlinerUI();
   void UpdatePropertyUI();

   // Popups & Modals
   void UpdateRendererInspectionModal();

   // Properties
   void TableProperties(PropertyPane &props);
   void ImageProperties(PropertyPane &props, Texture* image);
   void RenderProbeProperties(PropertyPane &props, RenderProbe* probe);
   void CameraProperties(PropertyPane &props, int bgSet);
   void MaterialProperties(PropertyPane &props, Material* material);

   // Enter/Exit edit mode (manage table backup, dynamic mode,...)
   void ResetCameraFromPlayer();
   void SetBackdropCamera();

   // Undo support
   void PushUndo(IEditable *part, unsigned int editId);
   PinUndo m_undo;
   IEditable *m_lastUndoPart = nullptr;
   unsigned int m_lastUndoId = 0;

   // Add/Remove parts
   void DeleteSelection();
   ItemTypeEnum m_addPartType = eItemInvalid; // Part type pending placement (eItemInvalid when not in add part mode)
   struct AddPartButton
   {
      ItemTypeEnum type;
      const char *name;
      std::shared_ptr<Sampler> icon;
   };
   vector<AddPartButton> m_addPartButtons; // Lazily initialized add part toolbar buttons
   ImVec2 m_addPartPopupPos; // Position at which the Shift+A part type picker popup was opened
   bool m_openAddPartPopup = false; // Request to open the part type picker popup (consumed in the toolbar window scope)
   void CreatePart(ItemTypeEnum type, const Vertex2D &pos);

   // Outliner
   string m_outlinerFilter;
   bool m_outlinerSelectLiveTab = true;
   bool MatchesOutlinerFilter(const string &name) const;

   // Properties
   bool m_propertiesSelectLiveTab = true;

   // Rendering
   float m_menubar_height = 0.0f;
   float m_toolbar_height = 0.0f;
   enum class PhysicOverlay
   {
      None,
      Selected,
      All
   };
   PhysicOverlay m_physOverlay = PhysicOverlay::None;
   bool m_selectionOverlay = true;
   enum class SelectionFilter : uint32_t
   {
      None = 0,
      Playfield = 0x0002,
      Primitives = 0x0004,
      Lights = 0x0008,
      Flashers = 0x0010,
      All = 0x0002 | 0x0004 | 0x0008 | 0x0010
   };
   friend constexpr SelectionFilter operator|(const SelectionFilter a, const SelectionFilter b) { return static_cast<SelectionFilter>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b)); }
   friend constexpr SelectionFilter operator&(const SelectionFilter a, const SelectionFilter b) { return static_cast<SelectionFilter>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b)); }
   friend constexpr SelectionFilter operator~(const SelectionFilter a) { return static_cast<SelectionFilter>(~static_cast<uint32_t>(a)); }
   static constexpr bool HasFlag(const SelectionFilter flags, const SelectionFilter flag) { return (flags & flag) != SelectionFilter::None; }
   SelectionFilter m_selectionFilter = SelectionFilter::All;

   // UI state
   bool m_isOpened = false;
   bool m_flyMode = false;
   bool m_showRendererInspection = false;
   enum class Units
   {
      VPX, Metric, Imperial
   } m_units = Units::VPX;

   // 3D editor
   ImGuizmo::OPERATION m_gizmoOperation = (ImGuizmo::OPERATION)0;
   ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
   bool GetSelectionTransform(Matrix3D &transform) const;
   void SetSelectionTransform(const Matrix3D &transform, bool clearPosition = false, bool clearScale = false, bool clearRotation = false) const;

   // Editor camera
   ViewMode m_camMode = ViewMode::PreviewCam;
   bool m_perspectiveCam = true;
   Renderer::ShadeMode m_shadeMode = Renderer::ShadeMode::Default;
   enum class PredefinedView
   {
      None, Front, Back, Right, Left, Top, Bottom
   } m_predefinedView = PredefinedView::None;
   Matrix3D m_camView, m_camProj;
   float m_camDistance;

   class RenderContext : public EditorRenderContext
   {
   public:
      RenderContext(Player *player, ImDrawList *drawlist, ViewMode viewMode, Renderer::ShadeMode shadeMode, bool needsLiveTableSync);
      ~RenderContext() override = default;

      bool NeedsLiveTableSync() const override { return m_needsLiveTableSync; }
      ImU32 GetColor(bool selected) const override { return selected ? (m_isActive ? IM_COL32(255, 128, 0, 255) : IM_COL32(192, 96, 0, 255)) : IM_COL32_BLACK; };
      bool IsSelected() const override { return m_isSelected; }
      bool IsShowInvisible() const override;
      ViewMode GetViewMode() const override { return m_viewMode; }
      ImDrawList *GetDrawList() const override { return m_drawlist; }

      ImVec2 Project(const Vertex3Ds &v) const override;
      void DrawLine(const Vertex3Ds &a, const Vertex3Ds &b, ImU32 color) const override;
      void DrawCircle(const Vertex3Ds &center, const Vertex3Ds &x, const Vertex3Ds &y, float radius, ImU32 color) const override;
      void DrawHitObjects(IEditable *editable) const override;
      void DrawWireframe(IEditable *editable) const override;

      bool m_isSelected = false;
      bool m_isActive = false;

   private:
      Player *m_player;
      ImDrawList *const m_drawlist;
      const ViewMode m_viewMode;
      const Renderer::ShadeMode m_shadeMode;
      const bool m_needsLiveTableSync;
   };
};

}
