// license:GPLv3+

#pragma once

#include "input/InputManager.h"
#include "core/pinundo.h"
#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"
#include "editor/EditorUIPart.h"
#include "editor/Selection.h"
#include "editor/EditorChrome.h"
#include "editor/OutlinerPanel.h"
#include "editor/PartLibraryPanel.h"
#include "editor/PropertiesPanel.h"
#include "editor/RendererInspectionModal.h"
#include "editor/ScriptPanel.h"
#include "math/matrix.h"
#include "renderer/Renderer.h"
#include "unordered_dense.h"

#include <optional>

class LiveUI;
class PinTable;
class Player;
class InputManager;
class Renderer;
class DragPoint;
class PartGroup;
class FRect3D;

namespace VPX::EditorUI
{

class EditorUI final
   : private DragPointEditContext
{
   // The editor's UI is split between sub components which, for the time being, access
   // the shared editor state directly (to be refined as domain objects are extracted)
   friend class EditorChrome;
   friend class OutlinerPanel;
   friend class PropertiesPanel;
   friend class RendererInspectionModal;
   friend class PartLibraryPanel;
   friend class ScriptPanel;

public:
   EditorUI(LiveUI &liveUI);
   ~EditorUI();

   void Open();
   bool IsOpened() const { return m_isOpened; }
   void Render3D();
   void RenderUI();
   void Close();

   // Switch the editor to a new table (either the base edited table or one of its live copies)
   void SetTable(PinTable *table);

   // Start playing a shallow copy of the edited table (edition is suspended, restored when the play session ends)
   void PlayTest();

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

   // Sub components (UI panels & modals)
   EditorChrome m_chrome;
   OutlinerPanel m_outliner;
   PropertiesPanel m_properties;
   RendererInspectionModal m_inspectionModal;
   PartLibraryPanel m_partLibrary;
   ScriptPanel m_scriptPanel;

   Selection m_selection;

   // Multi selection of editable parts: all currently selected parts, with m_selection.GetPart() being the
   // active one (property pane target and gizmo pivot). Empty unless m_selection.GetType() == S_EDITABLE.
   vector<std::shared_ptr<EditorUIPart>> m_multiSel;
   std::shared_ptr<EditorUIPart> m_outlinerAnchor; // Anchor part for shift+click range selection in the outliner
   // Multi selection of shared resources (images, sounds, materials): all selected items, with
   // m_selection's payload being the active one. Empty unless m_selection has the matching type.
   vector<Texture *> m_multiSelImages;
   vector<VPX::Sound *> m_multiSelSounds;
   vector<Material *> m_multiSelMaterials;
   Texture *m_outlinerImageAnchor = nullptr; // Anchors for shift+click range selection in the outliner
   VPX::Sound *m_outlinerSoundAnchor = nullptr;
   Material *m_outlinerMaterialAnchor = nullptr;
   bool m_boxSelectActive = false;
   ImVec2 m_boxSelectStart;
   bool IsPartSelected(const std::shared_ptr<EditorUIPart> &part) const;
   bool IsEditableSelected(const IEditable *editable) const;
   void ClearSelection();
   void SetSelection(const Selection &selection);
   void TogglePartSelection(const std::shared_ptr<EditorUIPart> &part);
   void SelectOutlinerRange(const std::shared_ptr<EditorUIPart> &part);
   void SelectPartsInGroup(const PartGroup *group);
   void MoveSelectionToPartGroup(PartGroup *group); // Move all selected parts/groups to the given group (nullptr = root, part groups only)
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
      bool pointEditCenter = false; // Whether the part's center point is being edited instead of its drag point curve
   };
   UndoSelectionState CaptureUndoSelection() const;
   void RestoreUndoSelection(const UndoSelectionState &state);

   // Drag point edit mode (entered/exited with Tab when the active selected part has a DragPointCurve):
   // while active, the part's curve points are rendered and can be selected & transformed in the table XY
   // plane. For parts exposing an editable center (light bulb, light sequencer animation center), Tab
   // switches the mode between the drag point curve and the center point before exiting.
   std::shared_ptr<EditorUIPart> m_pointEditPart; // Part whose DragPointCurve is being edited (nullptr when not in point edit mode)
   bool m_pointEditCenter = false; // Edit the part's center point instead of its drag point curve
   bool m_centerSelected = false; // Whether the part's center point is selected in center edit mode
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
   bool HitTestEditCenter(const ImVec2 &mousePos) const;
   void BoxSelectPoints(const ImVec2 &cornerA, const ImVec2 &cornerB, bool add);
   Vertex2D UnprojectToPlane(const ImVec2 &mousePos, float z) const;
   void AddPointOnNearestSegment();
   void DeleteSelectedPoints();

   // DragPointEditContext implementation
   const vector<DragPoint *> &GetSelectedPoints() const override { return m_pointSel; }
   bool IsCenterEditMode() const override { return m_pointEditCenter; }
   void BeginPointEdit() override;
   void EndPointEdit() override;

   // Decorated editable parts (kept sorted for the outliner, and indexed by editable)
   vector<std::shared_ptr<EditorUIPart>> m_editables;
   ankerl::unordered_dense::map<const IEditable *, std::shared_ptr<EditorUIPart>> m_editableMap;
   void UpdateEditableList();
   void RestorePartsVisibility() const; // Revert the editor visibility overrides of the table parts (reapplied on next render)

   // Enter/Exit edit mode (manage table backup, dynamic mode,...)
   void ResetCameraFromPlayer();
   void SetBackdropCamera();
   void SetPlayfieldCamera();
   Vertex2D GetUIVisibleFraction() const;

   // Undo support
   void PushUndo(IEditable *part, unsigned int editId);
   PinUndo m_undo;
   IEditable *m_lastUndoPart = nullptr;
   unsigned int m_lastUndoId = 0;

   // Add/Remove parts
   void DeleteSelection();
   ItemTypeEnum m_addPartType = eItemInvalid; // Part type pending placement (eItemInvalid when not in add part mode)
   void CreatePart(ItemTypeEnum type, const Vertex2D &pos);
   void CreateCollection(bool fromSelection); // Create a collection, optionally populated with the selected parts
   PartGroup *GetPartGroupForNewPart();
   vector<PartGroup *> m_partGroupUseHistory; // Recently used insertion target part groups, most recent first

   enum class NewTableTemplate
   {
      Blank,
      Stripped,
      Example,
      LightSeq
   };

   // File operations (the 'Save As' and 'Load' file dialogs are asynchronous: their result is applied in RenderUI)
   bool SaveTable(); // Returns true if the table was saved
   void SaveTableAs();
   void SaveTableAsPackFolder(); // Save as a VPZ folder pack
   void LoadTable();
   void LoadTableFolder(); // Load a VPZ folder pack
   void NewTable(NewTableTemplate templateType);
   void ShowLoadTableDialog(bool folder = false);
   void LoadTableDialog(bool folder);
   void LoadTableTemplate(NewTableTemplate templateType);
   std::shared_ptr<string> m_pendingSaveAsPath;
   std::shared_ptr<string> m_pendingLoadPath;
   bool m_pendingSaveAsFolder = false; // Pending 'Save As' result is a folder pack, not a file
   bool m_pendingLoadFolder = false; // Pending 'Load' result is a folder pack, not a file
   std::optional<NewTableTemplate> m_pendingNewTable; // New table template awaiting the 'discard unsaved changes' confirmation
   bool m_confirmLoadTable = false; // Request the 'discard unsaved changes' confirmation popup in RenderUI

   // Part library (import/export of parts through partial VPZ packs, handled by the PartLibraryPanel dialog)
   void OpenPartLibrary() { m_partLibrary.Show(); }
   bool CanExportPartSelection() const { return m_partLibrary.CanExportSelection(); }
   void ExportPartSelection() { m_partLibrary.ExportSelection(); }

public:
   // Closes the session with the given Player::CloseState, first asking to discard unsaved changes when they would be lost
   void RequestClose(int closeState);
private:
   std::optional<int> m_pendingClose; // Close state awaiting the 'discard unsaved changes' confirmation

   // Clipboard (copy/paste of parts through the OS clipboard, and of drag point coordinates in point edit mode)
   void CopySelection();
   void PasteSelection(const ImVec2 &pos);

   // Rendering
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
   enum class Units
   {
      VPX, Metric, Imperial
   } m_units = Units::VPX;

   // 3D editor
   ImGuizmo::OPERATION m_gizmoOperation = (ImGuizmo::OPERATION)0;
   ImGuizmo::MODE m_gizmoMode = ImGuizmo::WORLD;
   void SetGizmoOperation(ImGuizmo::OPERATION operation); // Edit mode actions of the Esc (select), G (grab), S (scale) and R (rotate) shortcuts
   bool GetSelectionTransform(Matrix3D &transform) const;
   bool GetSelectionBounds(FRect3D &bounds) const;
   void SetSelectionTransform(const Matrix3D &transform, bool clearPosition = false, bool clearScale = false, bool clearRotation = false) const;

   // Editor camera
   ViewMode m_camMode = ViewMode::PreviewCam;
   bool m_perspectiveCam = true;
   Renderer::ShadeMode m_shadeMode = Renderer::ShadeMode::Default;
   enum class PredefinedView
   {
      None, Front, Back, Right, Left, Top, Bottom
   } m_predefinedView = PredefinedView::None;
   bool m_orbitLock = false; // When locked, camera drags pan instead of orbiting
   bool m_fitPlayfieldCamera = false;
   Matrix3D m_camView, m_camProj;
   float m_camDistance;
};

}
