// license:GPLv3+

#pragma once

#include "input/InputManager.h"
#include "core/pinundo.h"
#include "imgui/imgui.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"
#include "editor/AreaLayout.h"
#include "editor/EditorAreaView.h"
#include "editor/parts/EditorUIPart.h"
#include "editor/Selection.h"
#include "editor/EditorChrome.h"
#include "editor/OutlinerPanel.h"
#include "editor/PartLibraryPanel.h"
#include "editor/PropertiesPanel.h"
#include "editor/RendererInspectionModal.h"
#include "editor/ScriptPanel.h"
#include "editor/ViewportPane.h"
#include "math/matrix.h"
#include "renderer/Renderer.h"
#include "unordered_dense.h"
#include "ui/VPXFileFeedback.h"

#include <atomic>
#include <optional>
#include <thread>

class LiveUI;
class PinTable;
class Player;
class Sampler;
class InputManager;
class Renderer;
class DragPoint;
class PartGroup;
class FRect3D;
struct HitTestResult;
class IEditable;

namespace VPX
{
class Sound;
}

namespace VPX::EditorUI
{

class BlueprintPane;
class LiveRenderContext;
class ViewportPane;

// Vertical field of view (in degrees) of the perspective editor camera
constexpr float editorCamFovY = 39.6f;

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
   friend class PartLibraryPane;
   friend class ScriptPanel;
   friend class ScriptPane;
   friend class BlueprintPane;
   friend class ViewportPane;

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

   bool IsPreview() const
   {
      const ViewportPane *const vp = GetTargetViewport();
      return vp != nullptr && vp->m_viewType == ViewMode::PreviewCam;
   }
   bool IsBackdropEditMode() const
   {
      const ViewportPane *const vp = GetTargetViewport();
      return vp != nullptr && vp->m_viewType == ViewMode::DesktopBackdrop;
   }

   bool IsInspectMode() const { return m_table->m_liveBaseTable != nullptr; }
   bool IsFlyMode() const
   {
      const ViewportPane *const vp = GetTargetViewport();
      return vp != nullptr && vp->m_flyMode;
   }

private:
   // UI Context
   LiveUI &m_liveUI;
   Player *m_player;
   PinTable *m_table; // The table displayed by the player
   InputManager *m_pininput;
   std::unique_ptr<Renderer>& m_renderer;

   // Sub components (UI panels & modals)
   EditorChrome m_chrome;
   RendererInspectionModal m_inspectionModal;
   PartLibraryPanel m_partLibrary;
   ScriptPanel m_scriptPanel;

   Selection m_selection;

   // Multi selection of editable parts: all currently selected parts, with m_selection.GetPart() being the
   // active one (property pane target and gizmo pivot). Empty unless m_selection.GetType() == S_EDITABLE.
   vector<std::shared_ptr<EditorUIPart>> m_multiSel;
   vector<IEditable *> m_dynamicParts; // Parts made dynamic in the UI quadtree because they are selected
   // Multi selection of shared resources (images, sounds, materials): all selected items, with
   // m_selection's payload being the active one. Empty unless m_selection has the matching type.
   vector<Texture *> m_multiSelImages;
   vector<VPX::Sound *> m_multiSelSounds;
   vector<Material *> m_multiSelMaterials;
   Texture *m_outlinerImageAnchor = nullptr; // Anchors for shift+click range selection in the outliner
   VPX::Sound *m_outlinerSoundAnchor = nullptr;
   Material *m_outlinerMaterialAnchor = nullptr;
   bool IsPartSelected(const std::shared_ptr<EditorUIPart> &part) const;
   bool IsEditableSelected(const IEditable *editable) const;
   void ClearSelection();
   void SetSelection(const Selection &selection);
   void TogglePartSelection(const std::shared_ptr<EditorUIPart> &part);
   void SelectPartsInGroup(const PartGroup *group);
   void MoveSelectionToPartGroup(PartGroup *group); // Move all selected parts/groups to the given group (nullptr = root, part groups only)
   bool IsEditablePickable(const IEditable *editable) const;
   void SelectAllParts();
   Vertex2D GetMultiSelCenter() const; // Center of the current selection (bounding box center of the part centers, or of the selected points in drag point edit mode)
   // Selection wide transforms (the win32 context menu Flip/Rotate/Scale/Translate commands): they apply
   // to the multi selection of parts, or to the selected points in drag point edit mode
   void FlipSelection(bool flipX); // Mirror the selection around the multi selection center
   void RotateMultiSel(float ang, const Vertex2D &center, bool useElementCenter);
   void ScaleMultiSel(float scalex, float scaley, const Vertex2D &center, bool useElementCenter);
   void TranslateMultiSel(const Vertex2D &offset);
   void LockElements(); // Toggle the UI lock of the selected parts (unlock all if any is locked, lock all otherwise)
   void MoveSelectionInDrawOrder(bool toFront); // Draw In Front / Draw In Back (PinTable z-order)
   void ToggleCollectionMembership(int collectionIndex); // Toggle the selected parts' membership of a collection

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
   bool m_pointDragPending = false; // Left button is down on a selected point (drag not started yet)
   bool m_pointDragActive = false; // Left button drag is moving the selected points
   float m_pointDragZ = 0.f; // Table Z coordinate of the plane in which points are dragged
   Vertex2D m_pointDragPos; // Last drag position in table coordinates
   void EnterPointEditMode();
   void ExitPointEditMode(bool restoreSelection);
   bool IsPointSelected(const DragPoint *point) const;
   void TogglePointSelection(DragPoint *point);
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

   // Undo support
   void Undo();
   void PushUndo(IEditable *part, unsigned int editId);
   PinUndo m_undo;
   IEditable *m_lastUndoPart = nullptr;
   unsigned int m_lastUndoId = 0;
   bool m_suppressNextEscape = false; // The Esc release of an Esc that deactivated an edited field (e.g. the script editor) is not an editor Esc

   // Add/Remove parts
   void DeleteSelection();
   ItemTypeEnum m_addPartType = eItemInvalid; // Part type pending placement (eItemInvalid when not in add part mode)
   // Part type picker popup: it must be opened in the window scope of the target viewport pane, so
   // requests (header button, Shift+A, Edit menu) are deferred to its RenderPane
   void RequestAddPartPopup(const ImVec2 &pos);
   int m_addPartPopupLeaf = -1; // Leaf hosting the pending picker (-1: none)
   ImVec2 m_addPartPopupPos {}; // Screen position at which the picker popup appears
   struct AddPartButton
   {
      ItemTypeEnum type;
      const char *name;
      std::shared_ptr<Sampler> icon;
   };
   vector<AddPartButton> m_addPartButtons; // Lazily initialized part type picker buttons (shared by the viewport panes)
   void EnsureAddPartButtons();
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
   void SaveTable();
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

   // Asynchronous file operations: table loading and saving run on a worker thread while a modal progress
   // feedback blocks all user interaction. The result is applied on the main thread in RenderUI.
   struct FileOpFeedback final : public VPXFileFeedback
   {
      std::atomic<unsigned int> progress { 0 };
      std::atomic<unsigned int> length { 0 };
      void SetProgress(const unsigned int value) override { progress = value; }
      void SetLength(const unsigned int value) override { length = value; }
   };
   struct AsyncFileOp
   {
      enum class Type
      {
         Load,
         NewTable,
         Save
      };
      ~AsyncFileOp();
      Type type = Type::Load;
      std::filesystem::path filename; // File being loaded or saved
      string displayPath; // Path displayed in the progress modal
      FileOpFeedback feedback;
      HRESULT result = E_FAIL; // Written by the worker thread before flagging the operation as completed
      std::atomic<bool> completed { false };
      std::thread worker;
      PinTable *table = nullptr; // Loaded table (load ops only): reference adopted by the player on success, released on failure
      PinTable *saveTable = nullptr; // Table written by a save op (the startup table when saving from an inspection session)
      // 'Save As' restore data: filename and title to restore when the save fails
      bool restoreOnFailure = false;
      std::filesystem::path previousFilename;
      string previousTitle;
   };
   // Inspection session: live copies edited from the 'Live' tabs, copied back to the startup table by 'Save'
   // (the startup table is the one written to disk; live values are otherwise discarded when the session ends)
   ankerl::unordered_dense::set<Material *> m_liveEditedMaterials;
   ankerl::unordered_dense::set<IEditable *> m_liveEditedParts;
   std::unique_ptr<AsyncFileOp> m_fileOp; // Running asynchronous file operation (all editor interaction is blocked while set)
   void StartFileOp(std::unique_ptr<AsyncFileOp> op);
   void FinishFileOp();
   void RenderFileOpModal();
   // Whether a save of the edited table is running on the file operation worker thread: while it is, the
   // UI parts must not write into the parts' data which is being serialized (see EditorUI::Render3D)
   bool IsSaveInProgress() const { return m_fileOp != nullptr && m_fileOp->type == AsyncFileOp::Type::Save; }

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
   void PasteSelection(const ImVec2 *pos); // Paste the clipboard at the given screen position (original coordinates when nullptr)

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
   enum class Units
   {
      VPX, Metric, Imperial
   } m_units = Units::VPX;

   // 3D editor
   bool GetSelectionTransform(Matrix3D &transform) const;
   bool GetSelectionBounds(FRect3D &bounds) const;
   void SetSelectionTransform(const Matrix3D &transform, bool clearPosition = false, bool clearScale = false, bool clearRotation = false) const;

   // The editor camera definition (mode, pose, shading) belongs to each viewport pane: it is
   // processed by ViewportPane (camera actions, gizmo, picking, shortcuts)
   bool m_fitPlayfieldCamera = false;
   struct CamSetup
   { // Camera setup requested at Open(), applied to the first bound pane with no camera of its own
      ViewMode mode;
      bool persp;
      Renderer::ShadeMode shade;
      ViewportPane::PredefinedView predef;
   };
   std::optional<CamSetup> m_camSetupPending;

   bool CanFrameSelection() const;

   // Areas: Blender-style split tree (see editor/AreaLayout.h). Outliner | Viewport | Properties by default.
   // Each leaf hosts one view instance (several leaves may share a type): panes are kept in m_areaViews keyed
   // by the stable leaf id, created when a leaf appears and dropped when it is removed or retyped.
   std::unique_ptr<EditorAreaView> CreateAreaView(AreaType type);
   std::unordered_map<int, std::unique_ptr<EditorAreaView>> m_areaViews;
   static const char *AreaTypeName(AreaType type);
   static const char *AreaTypeIcon(AreaType type); // Lucide icon of the editor type switcher
   static bool IsAreaTypeImplemented(AreaType type); // Types a leaf may take through the editor type switcher
   // Layout changes (type switch, split, join, close, swap, maximize) are queued and applied by UpdateAreaLayout
   // at the start of the frame: panels must not vanish mid frame (ImGui ids, in-flight drags)
   struct PendingAreaOp
   {
      enum class Kind
      {
         SetType,
         Split,
         Join,
         Close,
         Swap,
         Maximize,
         Show
      } kind
         = Kind::SetType;
      int leaf = -1;
      int leaf2 = -1; // Join: leaf to consume
      SplitAxis axis = SplitAxis::Vertical;
      bool newOnSecondSide = true; // Split: new leaf on the right / bottom
      float size = 0.f; // Split: logical size of the new leaf
      AreaType type = AreaType::Outliner; // SetType / Show
   };
   void QueueAreaOp(const PendingAreaOp &op) { m_pendingAreaOps.push_back(op); }
   void SetAreaEditor(int leafId, AreaType type); // Editor type of the area (leaf id) (queued)
   void ToggleMaximizeArea(); // Ctrl+Space (queued): maximize the area under the mouse, or restore
   ImVec2 GetViewportContentCenter() const; // Centre of the visible viewport leaf (paste position)
   void UpdateAreaLayout(); // Start of the UI frame: applies the pending ops, then computes the area rects
   void ComputeAreaRects() const; // Tree rect from the bar heights, then the cached leaf / splitter rects
   void RenderAreas(); // Hosts the pane views of the visible leaves
   void RenderAreaTypeButton(int leafId); // Header button of an area (inside its window), opening the type popup
   bool ApplyAreaOp(const PendingAreaOp &op);
   void WidenArea(int leafId, AreaType type); // Switching the editor of a too narrow area widens it (once)
   bool CanUseAreaTools() const;
   const LeafRect *FindLeafRect(int leafId) const;
   struct AreaSplitPlan
   {
      bool ok = false;
      const char *reason = "";
      SplitAxis axis = SplitAxis::Vertical;
      float line = 0.f;
      float size = 0.f;
      AreaRect leaf; // Area to split
      AreaRect newRect; // Where the new area would be (ok only)
      AreaType newType = AreaType::Outliner; // Type of the new leaf (the split leaf's type)
   };
   bool PlanAreaSplit(int leafId, SplitAxis axis, bool newOnSecondSide, const ImVec2 &cursor, AreaSplitPlan &plan) const;
   void DrawAreaSplitPreview(const AreaSplitPlan &plan) const;
   void DrawAreaJoinPreview(int keepId, int consumeId, bool ok) const;
   const char *AreaJoinRefusal(int keepId, int consumeId) const;
   struct AreaDrag
   {
      bool active = false;
      int leaf = -1; // Area the drag started in
      int corner = 0;
      ImVec2 start {};
      bool decided = false;
      bool axisChosen = false;
      SplitAxis axis = SplitAxis::Vertical;
   };
   enum class AreaModalKind
   {
      None,
      Split, // Edge menu split: click an area to split it at the mouse
      Join // Edge menu 'Join Areas': the area not under the mouse is consumed, click confirms
   };
   void RenderSplitters(); // Divider strips, corner zones, edge menu and modal join / split mode
   void RenderAreaCorners();
   void RenderAreaEdgeMenu();
   void RenderAreaModal();
   void UpdateAreaDrag();
   void CancelAreaInteraction(bool consumeEscape);
   bool ConsumedAreaEscape();
   bool ShouldRaiseAreaWindows() const;
   void Report(const string &message); // Posts a transient message (status / refusal feedback)
   void ShowAreaEditor(AreaType type); // Hosts the given editor type in an area leaf (splitting the focused one)
   void SetAreaLayoutIndex(int index); // Selects the active area layout slot (saved to and loaded from the LiveEditor settings)
   void LoadAreaLayoutSettings(); // Applies the persisted active layout (once, on first frame)
   void SaveAreaLayoutSettings(); // Writes the current layout (and the viewport panes' setups) into its settings slot
   int m_activeLayoutIndex = 0; // Selected layout slot (Layout 1 / Layout 2)
   bool m_layoutSettingsLoaded = false; // The persisted layout has been applied (lazy, on first frame)
   AreaLayout m_layout;
   std::vector<PendingAreaOp> m_pendingAreaOps;
   mutable std::vector<LeafRect> m_leafRects;
   mutable std::vector<SplitterRect> m_splitterRects;
   mutable bool m_areaRectsValid = false;
   mutable AreaRect m_viewportRect; // Rect of the first visible Viewport leaf (empty when none is shown)
   mutable bool m_viewportVisible = true;
   int m_maximizedLeafId = -1;
   int m_hoverLeaf = -1; // Leaf under the mouse (the maximize toggle target)
   AreaDrag m_areaDrag;
   AreaModalKind m_areaModal = AreaModalKind::None;
   int m_areaModalNode = -1;
   SplitAxis m_areaModalAxis = SplitAxis::Vertical;
   bool m_areaModalFocus = false;
   int m_edgeMenuNode = -1; // Split node the edge menu was opened for
   bool m_edgeMenuRequest = false;
   ImVec2 m_edgeMenuPos {};
   int m_splitDragNode = -1; // Split being dragged through its strip
   float m_splitDragPos = 0.f;
   // The strips and corner zones must be raised above the panes: created with NoBringToFrontOnFocus they land
   // at the bottom of the window list, below the pane windows created before them (and again below the panes
   // created by a layout change). Held until a popup needs the top, so the strips stay grabbable.
   bool m_raiseAreaWindows = true;
   bool m_areaEscPending = false; // An Esc consumed by an area interaction (its release is not an editor Esc)

   // Viewport panes each process their own 3D scene interactions — camera, gizmo, picking,
   // viewport shortcuts — on their own camera definition (see ViewportPane::RenderUI):
   // the pane under the mouse, or the one holding an in-progress drag (captured pane)
   ViewportPane *m_hoverVp = nullptr; // Viewport pane whose image is hovered this frame
   ViewportPane *m_captureVp = nullptr; // Viewport pane holding an in-progress mouse interaction
   ViewportPane *HoveredViewportPane() const;
   ViewportPane *GetTargetViewport() const; // Pane owning viewport actions: flying > dragged > hovered > first

   // Blueprint panes process their own 2D canvas interactions the same way (see
   // BlueprintPane::RenderUI): the pane under the mouse, or the one capturing a drag
   BlueprintPane *m_hoverBp = nullptr; // Blueprint pane whose canvas is hovered this frame
   BlueprintPane *m_captureBp = nullptr; // Blueprint pane holding an in-progress mouse interaction
   BlueprintPane *GetTargetBlueprint() const { return m_captureBp != nullptr ? m_captureBp : m_hoverBp; }

   Vertex2D TablePosAt(const ImVec2 &pos) const; // Display px -> table XY through the view pane under pos
};

}
