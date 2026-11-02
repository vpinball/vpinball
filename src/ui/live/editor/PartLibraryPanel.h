// license:GPLv3+

#pragma once

#include "parts/pintable.h"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

struct ImVec2;

namespace VPX::EditorUI
{

class EditorUI;
class EditorUIPart;

// Part library: export of the current part selection to a partial VPZ pack, and import of
// parts from a specific VPZ file or from a browsable list of library folders holding VPZ
// packs organized in subfolders. The import merge strategy defines how conflicts between
// imported assets and same named table assets are resolved.
//
// Pack placement: selecting a pack in the library loads its whole content in a temporary
// table and previews it as ghost parts following the mouse over the 3D view (resting on the
// topmost surface under the pointer). Clicking commits the pack content to the edited table;
// unselecting or closing the dialog discards it.
class PartLibraryPanel
{
public:
   explicit PartLibraryPanel(EditorUI &editor);
   ~PartLibraryPanel();

   void Show()
   {
      m_visible = true;
      m_rescanLibrary = true;
   }
   void Close();
   bool IsVisible() const { return m_visible; }

   // Export of the current selection to a VPZ pack file, disabled when there is no exportable selection
   bool CanExportSelection() const;
   void ExportSelection();

   // Pack placement mode: a pack of the library is selected for preview & placement
   bool IsPlacementArmed() const { return !m_previewParts.empty(); }
   void CancelPlacement();
   void CommitPlacement();

   void Render();
   void Render3D(); // Renders the ghost preview parts into the 3D scene (called by EditorUI::Render3D)

   // Drops the preview parts, e.g. when the edited table is switched or the editor is closed
   void ClearPreview();

private:
   // A library folder, recursively scanned for importable VPZ packs
   struct LibraryFolder
   {
      std::filesystem::path path;
      vector<LibraryFolder> subFolders;
      vector<std::filesystem::path> packs;
   };

   // Summary of the importable content of a VPZ pack
   struct PackInfo
   {
      bool valid = false;
      bool hasTable = false;
      int parts = 0;
      int collections = 0;
      int materials = 0;
      int images = 0;
      int sounds = 0;
      int fonts = 0;
   };

   void RescanLibrary();
   void RenderLibraryFolder(const LibraryFolder &folder, const string &label);
   vector<std::filesystem::path> GetLibraryFolders() const;
   void SetLibraryFolders(const vector<std::filesystem::path> &folders);
   void AddLibraryFolder(const std::filesystem::path &folder);
   void RemoveLibraryFolder(size_t index);
   PackInfo ScanPack(const std::filesystem::path &path);
   void ImportPack(const std::filesystem::path &path);

   // Pack preview: the selected pack is loaded in a scratch table so that it never pollutes the
   // edited one (no outliner entry, no undo, not in the picking quadtree) until it is committed.
   // Its authored positions are overridden by UpdatePlacement (moved as a rigid group)
   void StartPlacement();
   void UpdatePlacement(const ImVec2 &mousePos);
   void SetupPlayerSide(IEditable *part, bool initRenderable);

   EditorUI &m_editor;
   bool m_visible = false;

   // Results of the asynchronous SDL file dialogs, applied on the main thread in Render
   std::shared_ptr<string> m_pendingExportPath;
   std::shared_ptr<string> m_pendingImportPath;
   std::shared_ptr<string> m_pendingFolderPath;

   // Import options
   PartImportMergeStrategy m_mergeStrategy = PartImportMergeStrategy::RenameConflict;

   // Library content, rebuilt on Show and whenever the folder list changes
   bool m_rescanLibrary = true;
   vector<LibraryFolder> m_library;
   std::filesystem::path m_selectedPack;
   PackInfo m_selectedPackInfo;

   // Pack placement preview (see class comment): m_placementPack is the armed pack, the preview
   // table holds its loaded content (non group parts are tracked for translation & rendering)
   std::filesystem::path m_placementPack;
   CComObject<PinTable> *m_previewTable = nullptr;
   vector<IEditable *> m_previewParts;
   vector<std::shared_ptr<EditorUIPart>> m_previewUIParts;
   bool m_previewVisible = false; // Ghost currently rendered (mouse over the 3D view)
   Vertex2D m_previewPos;
   float m_previewZ = 0.f;
};

}
