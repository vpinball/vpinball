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
#include "editor/ops/LayerSpaceChangeOp.h"
#include "editor/ops/ReferenceImageOp.h"
#include "math/matrix.h"
#include "parts/PartGroup.h"
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
   friend class SetOriginOp;
   friend class LayerSpaceChangeOp;
   friend class ReferenceImageOp;

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
   void InvertSelection(); // Edit > Invert Selection (Ctrl+I)
   void TogglePartSelection(const std::shared_ptr<EditorUIPart> &part);
   void SelectPartsInGroup(const PartGroup *group);
   // Move all selected parts/groups to the given group (nullptr = root, part groups only), re-expressing
   // their transforms when the target's reference space differs (see LayerSpaceChangeOp::RequestMove).
   // Returns false when the move waits for the user's confirmation of lossy space conversions.
   bool MoveSelectionToPartGroup(PartGroup *group);
   // Whether the part may be moved to 'group' (nullptr = root): a part can not be moved to itself, a group
   // not to itself or a descendant (cycle), and only part groups may be moved to the root (group-less parts are live objects)
   static bool CanMovePartToGroup(const IEditable *editable, const PartGroup *group);
   // The selection may not mix parts of the desktop backdrop with parts of the 3D scene: its domain
   // is the m_desktopBackdrop flag of its parts, seeded from the active pane mode while it is empty
   bool SelectionDomain() const { return m_multiSel.empty() ? IsBackdropEditMode() : m_multiSel.front()->GetEditable()->m_desktopBackdrop; }
   bool IsPartInSelectionDomain(const IEditable *editable) const; // Can the part be added to the selection under the domain rule
   bool IsEditablePickable(const IEditable *editable, bool backdropDomain) const; // Pickable in the given domain (3D scene / backdrop) and passing the selection filters
   static PartGroupData::SpaceReference GetPartSpace(
      const IEditable *editable); // Reference space a part lives in (its own for groups, its parent group's otherwise, playfield for ungrouped parts)
   std::optional<PartGroupData::SpaceReference> GetSelectionSpace() const; // Reference space shared by all the selected parts (none when empty, backdrop or mixed)
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
   bool CanEnterPointEdit() const;
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
   void PushUndo(IEditable *part, unsigned int editId, const string &label = {});
   PinUndo m_undo;
   IEditable *m_lastUndoPart = nullptr;
   unsigned int m_lastUndoId = 0;
   // Undo and redo are off while a drag is in progress (its record is the top one: undoing it would leave the drag going unrecorded)
   bool IsDragInProgress() const;
   void CancelGizmoDrags(); // Ends the selection gizmo drag in flight, whichever pane it is in
   bool CanUndo() const;
   bool CanRedo() const;
   void UndoSteps(size_t count); // Edit > Undo (Ctrl+Z), count > 1 for the undo history
   void RedoSteps(size_t count); // Edit > Redo (Ctrl+Shift+Z / Ctrl+Y)
   void RestoreUndoState(const std::any &state);
   static string CountLabel(const char *verb, size_t count, const string &name);

   // Adjust Last Operation (Blender's redo panel, F9): the parameters of the last move / rotate / scale / duplicate / paste / add part,
   // editable as long as nothing else happened (no new undo step, same selection). Adjusting re-evaluates the operation from the state it
   // started from, the undo record (which holds the state before the operation) is unchanged: one Ctrl+Z still reverts all.
   struct LastOperation
   {
      enum class Type
      {
         None,
         Add,
         Paste,
         Duplicate,
         Move,
         Rotate,
         Scale
      };
      Type type = Type::None;
      uint64_t undoId = 0; // PinUndo::TopRecordId() after the operation
      vector<std::shared_ptr<EditorUIPart>> selection;
      Matrix3D initial; // Transform of the selection before (move, rotate, scale) or after (placements) the operation
      Vertex3Ds base { 0.f, 0.f, 0.f }; // Placements: position the offset is relative to (table coordinates)
      Vertex3Ds offset { 0.f, 0.f, 0.f }; // Move: delta, placements: offset from the base (table coordinates)
      Vertex3Ds pivot { 0.f, 0.f, 0.f }; // Rotate / scale: the op's pivot, the part center (table coordinates)
      Vertex3Ds axisView { 0.f, 0.f, 1.f }; // Rotate: the op's axis in table space when axis < 0
      int axis = -1; // Rotate / scale: -1 view axis (rotate) or all axes (scale), 0..2 for X / Y / Z
      bool plane = false; // Scale: the axes perpendicular to the axis
      bool localSpace = false; // Scale: the op's axes were the start transform's basis
      float angle = 0.f; // Degrees
      float factor = 1.f;
      bool expanded = true;
   } m_lastOp;
   bool m_adjustPopupOpen = false; // F9
   bool m_adjustPopupJustOpened = false;
   ImVec2 m_adjustPopupPos {};
   void SetLastOperation(LastOperation::Type type, const Matrix3D &initial);
   void NoteTransformOp(const ViewportPane::TransformOp &op); // Records a confirmed modal transform op as the last operation
   void ApplyLastOperation();
   bool RenderAdjustParameters(); // Widgets of the parameters, true if one changed
   void RenderAdjustPanel(); // Panel at the bottom left of the viewport, and the F9 popup
   static const char *LastOperationName(LastOperation::Type type);
   bool m_suppressNextEscape = false; // The Esc release of an Esc that deactivated an edited field (e.g. the script editor) is not an editor Esc

   // Add/Remove parts
   void DeleteSelection();
   bool CanDuplicate() const;
   void DuplicateSelection(); // Duplicate the selected parts at their position and grab them (Shift+D)
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
   PartGroup *CreatePartGroup(); // Create a new root-level part group (outliner / layer dialog 'New Group')
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
   void LoadTable(const std::filesystem::path &path); // Load the given table file or pack folder (File > Open Recent)
   void NewTable(NewTableTemplate templateType);
   void ShowLoadTableDialog(bool folder = false);
   void LoadTableDialog(bool folder);
   void LoadTableTemplate(NewTableTemplate templateType);
   // Recently opened table files and pack folders (File > Open Recent), most recent first. The list is shared with
   // the Win32 editor's recent file list: it lives in the app's RecentDir.TableFileName settings, re-read on each use
   const vector<std::filesystem::path> &GetRecentTables();
   void NoteRecentTable(const std::filesystem::path &path);
   vector<std::filesystem::path> m_recentTables; // Scratch copy of the shared list, refreshed by GetRecentTables()
   std::shared_ptr<string> m_pendingSaveAsPath;
   std::shared_ptr<string> m_pendingLoadPath;
   bool m_pendingSaveAsFolder = false; // Pending 'Save As' result is a folder pack, not a file
   bool m_pendingLoadFolder = false; // Pending 'Load' result is a folder pack, not a file
   std::optional<NewTableTemplate> m_pendingNewTable; // New table template awaiting the 'discard unsaved changes' confirmation
   std::optional<std::filesystem::path> m_pendingRecentPath; // 'Open Recent' path awaiting the 'discard unsaved changes' confirmation
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
   // FIXME Only cleared by 'Save': pointers of an ended session stay (harmless, they are only used as lookup keys)
   ankerl::unordered_dense::set<Material *> m_liveEditedMaterials;
   ankerl::unordered_dense::set<IEditable *> m_liveEditedParts;
   std::unique_ptr<AsyncFileOp> m_fileOp; // Running asynchronous file operation (all editor interaction is blocked while set)
   void StartFileOp(std::unique_ptr<AsyncFileOp> op);
   void FinishFileOp();
   void RenderFileOpModal();
   // Whether a save of the edited table is running on the file operation worker thread: while it is, the
   // UI parts must not write into the parts' data which is being serialized (see EditorUI::Render3D)
   bool IsSaveInProgress() const { return m_fileOp != nullptr && m_fileOp->type == AsyncFileOp::Type::Save; }

public:
   // Part group (layer) reference space change (property pane): the children coordinates are
   // re-expressed in the new reference space so that the parts do not move. Parts that can not be
   // exactly converted (e.g. a surface can not be tilted out of its plane) are confirmed by a modal
   // before doing anything.
   LayerSpaceChangeOp m_layerSpaceChangeOp { *this };

   // Reference images (flashers of the ReferenceImages part group, hidden in play): Add / Replace dialog and Fit to Image
   ReferenceImageOp m_referenceImageOp { *this };

public:
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
   void Report(const string &message); // Posts a transient message (status bar report; toast while the bar is hidden)
   string m_report; // Last operation report of the status bar (fades out a few seconds after m_reportTime)
   double m_reportTime = -1000.0;
   void ShowAreaEditor(AreaType type); // Hosts the given editor type in an area leaf (splitting the focused one)
   void SetAreaLayoutIndex(int index); // Selects the active area layout slot (saved to and loaded from the LiveEditor settings)
   void ResetAreaLayouts(); // Clears the persisted layouts: every slot loads its factory preset (the active one right away)
   void LoadAreaLayoutSettings(); // Applies the persisted active layout (once, on first frame)
   void SaveAreaLayoutSettings(); // Writes the current layout (and the viewport panes' setups) into its settings slot
   int m_activeLayoutIndex = 0; // Selected layout slot (Layout 1 / Layout 2)
   bool m_layoutSettingsLoaded = false; // The persisted layout has been applied (lazy, on first frame)
   AreaLayout m_layout;
   std::vector<PendingAreaOp> m_pendingAreaOps;
   mutable std::vector<LeafRect> m_leafRects;
   mutable std::vector<SplitterRect> m_splitterRects;
   mutable bool m_areaRectsValid = false;
   bool m_navOrbitSelection = true; // Navigation preference: orbit and zoom around the selection
   bool m_navZoomToMouse = true; // Navigation preference: wheel zoom goes toward the cursor
   int m_navSelectionDisplay = 0; // 0 = edges only, 1 = edges and tint, 2 = hidden
   bool m_navAutoDepth = true; // Navigation preference: with no selection, orbit around the 3D point under the cursor
   float m_navFovY = 39.6f; // Editor perspective camera vertical field of view in degrees (50 mm lens equivalent)
   bool m_navInvertZoom = false; // Navigation preferences (stored in the Editor section of the settings)
   bool m_navAltMiddleButton = true;
   bool m_navTrackpad = true;
   float m_navOrbitSensitivity = 1.f;
   bool m_navBlenderTransform = true; // Navigation preference: G / R / S start a modal transform instead of the gizmo
   bool m_navEmulateNumpad = true; // Navigation preference: top row digits act as the numpad view keys
   enum class SnapKind
   {
      Move,
      Rotate,
      Scale
   };
   bool IsSnapActive(SnapKind kind) const; // Snapping applies right now (Ctrl inverts it while transforming, Blender)
   void ToggleSnapping(); // Shift+Tab
   bool m_snapEnabled = false; // Snapping preference (persisted in the Editor section of the settings)
   int m_snapTarget = 0; // 0 = increment (grid), 1 = points (drag points and centers of other parts, G only)
   float m_snapMove = 10.f; // Move snapping increment, in VP units
   float m_snapRotate = 15.f; // Rotation snapping increment, in degrees
   float m_snapScale = 0.1f; // Scale snapping increment (scale factor)
   bool m_snapAffectMove = true;
   bool m_snapAffectRotate = true;
   bool m_snapAffectScale = true;
   mutable AreaRect m_viewportRect; // Rect of the first visible Viewport leaf (empty when none is shown)
   mutable bool m_viewportVisible = true;
   int m_maximizedLeafId = -1;
   int m_hoverLeaf = -1; // Leaf under the mouse (the maximize toggle target)
   bool m_outlinerFocusPending = false; // Ctrl+F revealed an outliner leaf: focus its search field once its view exists
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
