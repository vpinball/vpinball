// license:GPLv3+

#include "core/stdafx.h"
#include "PartLibraryPanel.h"

#include "core/Settings.h"
#include "core/SettingsService.h"
#include "core/VPApp.h"
#include "fonts/IconsForkAwesome.h"
#include "parts/ball.h"
#include "parts/pintable.h"
#include "physics/PhysicsEngine.h"
#include "ui/live/EditorUI.h"
#include "ui/live/FileDialog.h"
#include "BlueprintPane.h"
#include "ui/live/editor/parts/EditorUIPartRegistry.h"
#include "ui/live/LiveUI.h"
#include "ui/VPXFileFeedback.h"
#include "utils/JSONSerializer.h"

// Title (used as Id) of the modal dialog
#define ID_PART_LIBRARY "##Pane"

namespace
{

// Decompose a transform into the position/scale/rotation components used by the parts'
// SetTransform (same decomposition as EditorUI::SetSelectionTransform)
void ExtractTRS(Matrix3D transform, vec3 &pos, vec3 &scale, vec3 &rot)
{
   const Vertex3Ds right(transform._11, transform._12, transform._13);
   const Vertex3Ds up(transform._21, transform._22, transform._23);
   const Vertex3Ds dir(transform._31, transform._32, transform._33);
   scale.Set(max(right.Length(), 1e-8f), max(up.Length(), 1e-8f), max(dir.Length(), 1e-8f)); // Clamp to avoid division by zero

   transform._11 /= scale.x; // Normalize transform to evaluate rotation
   transform._12 /= scale.x;
   transform._13 /= scale.x;
   transform._21 /= scale.y;
   transform._22 /= scale.y;
   transform._23 /= scale.y;
   transform._31 /= scale.z;
   transform._32 /= scale.z;
   transform._33 /= scale.z;

   pos.Set(transform._41, transform._42, transform._43);

   // Derived from https://learnopencv.com/rotation-matrix-to-euler-angles/
   const float sy = sqrtf(transform._11 * transform._11 + transform._21 * transform._21);
   if (sy > 1e-6f)
   {
      rot.x = -RADTOANG(atan2f(transform._32, transform._33));
      rot.y = -RADTOANG(atan2f(-transform._31, sy));
      rot.z = -RADTOANG(atan2f(transform._21, transform._11));
   }
   else
   {
      rot.x = -RADTOANG(atan2f(transform._23, transform._22));
      rot.y = -RADTOANG(atan2f(-transform._31, sy));
      rot.z = 0.f;
   }
}

}

namespace VPX::EditorUI
{

PartLibraryPanel::PartLibraryPanel(EditorUI &editor)
   : m_editor(editor)
{
}

PartLibraryPanel::~PartLibraryPanel() { ClearPreview(); }

void PartLibraryPanel::Show()
{
   m_rescanLibrary = true;
   if (!IsVisible())
      m_editor.ShowAreaEditor(AreaType::PartLibrary);
}

void PartLibraryPanel::Close()
{
   m_placementPack.clear();
   ClearPreview();
   const int leaf = m_editor.m_layout.FindLeaf(AreaType::PartLibrary);
   if (leaf >= 0)
   {
      EditorUI::PendingAreaOp op {};
      op.kind = EditorUI::PendingAreaOp::Kind::Close;
      op.leaf = leaf;
      m_editor.m_pendingAreaOps.push_back(op);
   }
}

bool PartLibraryPanel::IsVisible() const { return m_editor.m_layout.IsHosted(AreaType::PartLibrary); }

bool PartLibraryPanel::CanExportSelection() const
{
   // Only editable parts can be exported: drag points and light centers are edited inside their owning part
   return !m_editor.IsInspectMode() && !m_editor.m_table->IsLocked() && (m_editor.m_pointEditPart == nullptr) && !m_editor.m_multiSel.empty();
}

void PartLibraryPanel::ExportSelection()
{
   if (!CanExportSelection() || m_editor.m_player->m_playfieldWnd == nullptr)
      return;
   m_pendingExportPath = std::make_shared<string>();
   const std::filesystem::path defaultLocation
      = m_editor.m_table->m_filename.empty() ? PathFromString(m_editor.m_table->GetSettings().GetRecentDir_LoadDir()) : m_editor.m_table->m_filename.parent_path();
   const string location = PathToUTF8(defaultLocation); // SDL expects UTF-8
   const SDL_DialogFileFilter filters[] = { { "Visual Pinball Packs", "vpz" } };
   ShowSaveFileDialog(
      [](void *userdata, const char *const *filelist, int filter)
      {
         auto *res = static_cast<std::shared_ptr<string> *>(userdata);
         if (filelist != nullptr && filelist[0] != nullptr)
            **res = filelist[0];
         delete res;
      },
      new std::shared_ptr<string>(m_pendingExportPath), //
      m_editor.m_player->m_playfieldWnd->GetCore(), filters, 1, location.empty() ? nullptr : location.c_str());
}

void PartLibraryPanel::Update()
{
   // Apply the file picked by the asynchronous 'Export' save dialog (deferred to the main thread)
   if (m_pendingExportPath && !m_pendingExportPath->empty())
   {
      std::filesystem::path file = PathFromUTF8(*m_pendingExportPath);
      m_pendingExportPath = nullptr;
      if (lowerCase(PathToUTF8(file.extension())) != ".vpz"s)
         file += ".vpz";
      vector<IEditable *> parts;
      for (const auto &part : m_editor.m_multiSel)
         parts.push_back(part->GetEditable());
      VPXFileFeedback feedback;
      m_editor.RestorePartsVisibility(); // Export the authored visibility, not the editor display state
      const HRESULT hr = m_editor.m_table->SavePartsToJSONPack(file, parts, feedback);
      if (SUCCEEDED(hr))
      {
         string exported;
         for (size_t i = 0; i < parts.size(); i++)
         {
            exported += (exported.empty() ? ""s : ", "s) + parts[i]->GetName();
            if (i == 4 && parts.size() > 5)
            {
               exported += ", ..."s;
               break;
            }
         }
         m_editor.m_liveUI.PushNotification("Exported "s + std::to_string(parts.size()) + " part(s) to '"s + PathToUTF8(file.filename()) + "': "s + exported, 10000);
      }
      else
         m_editor.m_liveUI.PushNotification("Failed to export selection to '"s + PathToUTF8(file) + '\'', 10000);
   }
   // Apply the file picked by the asynchronous 'Import' file dialog (deferred to the main thread)
   if (m_pendingImportPath && !m_pendingImportPath->empty())
   {
      m_selectedPack = PathFromUTF8(*m_pendingImportPath);
      m_pendingImportPath = nullptr;
      m_selectedPackInfo = ScanPack(m_selectedPack);
   }

   // Apply the folder picked by the asynchronous 'Add library folder' dialog (deferred to the main thread)
   if (m_pendingFolderPath && !m_pendingFolderPath->empty())
   {
      AddLibraryFolder(PathFromUTF8(*m_pendingFolderPath));
      m_pendingFolderPath = nullptr;
   }

   if (!IsVisible())
   {
      m_placementPack.clear();
      ClearPreview();
      return;
   }

   // Placement mode housekeeping: it is only available while the table is editable and the
   // playfield view is interactive (not in drag point edit mode, not hidden by the fly mode)
   if (IsPlacementArmed() && (m_editor.IsInspectMode() || m_editor.m_table->IsLocked() || m_editor.m_pointEditPart != nullptr || m_editor.IsFlyMode()))
      CancelPlacement();
   if (IsPlacementArmed())
      UpdatePlacement(ImGui::GetIO().MousePos);
   else
      m_previewVisible = false;

   if (m_rescanLibrary)
   {
      m_rescanLibrary = false;
      RescanLibrary();
   }
}

void PartLibraryPanel::RenderPane(const AreaRect &rect, const int leafId)
{
   char name[40];
   snprintf(name, sizeof(name), "%s%d", ID_PART_LIBRARY, leafId);
   ImGui::SetNextWindowPos(ImVec2(rect.x0, rect.y0));
   ImGui::SetNextWindowSize(ImVec2(rect.Width(), rect.Height()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus);
   ImGui::PushID(leafId);
   m_editor.RenderAreaTypeButton(leafId);
   ImGui::SameLine();
   ImGui::AlignTextToFramePadding();
   ImGui::TextDisabled("Part Library");
   ImGui::Separator();
   {
      // Destination of the import, same as for newly added parts: the group of the current selection
      const PartGroup *const target = m_editor.GetPartGroupForNewPart();
      ImGui::Text("Import into part group: %s", (target != nullptr) ? target->GetName().c_str() : "(table root)");
      if (m_editor.m_table->IsLocked())
         ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "The table is locked: import is disabled");

      if (ImGui::CollapsingHeader("Import options"))
      {
         int strategy = static_cast<int>(m_mergeStrategy);
         ImGui::RadioButton("Rename on conflict when assets differ", &strategy, static_cast<int>(PartImportMergeStrategy::RenameConflict));
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Imported assets sharing the name of a different table asset are renamed\n"
                              "and the references of the imported parts are updated to the new names.\n"
                              "Identical assets are shared.");
         ImGui::RadioButton("Newer wins on name conflict", &strategy, static_cast<int>(PartImportMergeStrategy::ExistingWins));
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("On a name conflict, the imported parts reuse the asset already present in the table.");
         m_mergeStrategy = static_cast<PartImportMergeStrategy>(strategy);
      }

      ImGui::SeparatorText("Import a pack file");
      if (ImGui::Button(ICON_FK_FILE_O " Select VPZ file..."))
      {
         if (m_editor.m_player->m_playfieldWnd != nullptr)
         {
            m_pendingImportPath = std::make_shared<string>();
            const std::filesystem::path defaultLocation = PathFromString(m_editor.m_table->GetSettings().GetRecentDir_LoadDir());
            const string location = PathToUTF8(defaultLocation); // SDL expects UTF-8
            const SDL_DialogFileFilter filters[] = { { "Visual Pinball Packs", "vpz" } };
            ShowOpenFileDialog(
               [](void *userdata, const char *const *filelist, int filter)
               {
                  auto *res = static_cast<std::shared_ptr<string> *>(userdata);
                  if (filelist != nullptr && filelist[0] != nullptr)
                     **res = filelist[0];
                  delete res;
               },
               new std::shared_ptr<string>(m_pendingImportPath), //
               m_editor.m_player->m_playfieldWnd->GetCore(), filters, 1, location.empty() ? nullptr : location.c_str(), false);
         }
      }

      ImGui::SeparatorText("Library folders");
      if (ImGui::Button(ICON_FK_FOLDER_OPEN " Add library folder..."))
      {
         if (m_editor.m_player->m_playfieldWnd != nullptr)
         {
            m_pendingFolderPath = std::make_shared<string>();
            const std::filesystem::path defaultLocation = PathFromString(m_editor.m_table->GetSettings().GetRecentDir_LoadDir());
            const string location = PathToUTF8(defaultLocation); // SDL expects UTF-8
            ShowOpenFolderDialog(
               [](void *userdata, const char *const *filelist, int filter)
               {
                  auto *res = static_cast<std::shared_ptr<string> *>(userdata);
                  if (filelist != nullptr && filelist[0] != nullptr)
                     **res = filelist[0];
                  delete res;
               },
               new std::shared_ptr<string>(m_pendingFolderPath), //
               m_editor.m_player->m_playfieldWnd->GetCore(), location.empty() ? nullptr : location.c_str(), false);
         }
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_REFRESH " Refresh"))
         m_rescanLibrary = true;
      if (m_library.empty())
         ImGui::TextDisabled("No library folder defined");

      const float footerHeight = ImGui::GetFrameHeightWithSpacing() * 4.f;
      if (ImGui::BeginChild("##LibraryTree", ImVec2(0.f, -footerHeight), ImGuiChildFlags_Borders))
      {
         for (size_t i = 0; i < m_library.size(); i++)
         {
            ImGui::PushID(static_cast<int>(i));
            const LibraryFolder &root = m_library[i];
            const string label = root.path.filename().empty() ? PathToUTF8(root.path) : PathToUTF8(root.path.filename());
            const bool open = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);
            if (ImGui::IsItemHovered())
               ImGui::SetTooltip("%s", PathToUTF8(root.path).c_str());
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - (ImGui::CalcTextSize(ICON_FK_TRASH_O).x + 2.f * ImGui::GetStyle().FramePadding.x));
            if (ImGui::SmallButton(ICON_FK_TRASH_O))
            {
               RemoveLibraryFolder(i);
               if (open)
                  ImGui::TreePop();
               ImGui::PopID();
               break;
            }
            if (ImGui::IsItemHovered())
               ImGui::SetTooltip("Remove this library folder");
            if (open)
            {
               RenderLibraryFolder(root, label);
               ImGui::TreePop();
            }
            ImGui::PopID();
         }
      }
      ImGui::EndChild();

      // Selected pack preview and import action
      ImGui::SeparatorText("Import");
      if (!m_placementPack.empty())
         ImGui::TextWrapped("Placing \"%s\": click in the 3D view to drop it, press Esc to cancel", PathToUTF8(m_placementPack.filename()).c_str());
      if (m_selectedPack.empty())
         ImGui::TextDisabled("Select a VPZ pack file to import");
      else
      {
         ImGui::TextUnformatted(PathToUTF8(m_selectedPack.filename()).c_str());
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", PathToUTF8(m_selectedPack).c_str());
         if (!m_selectedPackInfo.valid)
            ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "Failed to read the pack");
         else
            ImGui::Text("Content: %d part(s), %d collection(s), %d material(s), %d image(s), %d sound(s), %d font(s)%s", m_selectedPackInfo.parts, m_selectedPackInfo.collections,
               m_selectedPackInfo.materials, m_selectedPackInfo.images, m_selectedPackInfo.sounds, m_selectedPackInfo.fonts, m_selectedPackInfo.hasTable ? " + table" : "");
      }
   }
   ImGui::PopID();
   ImGui::End();
   ImGui::PopStyleVar(2);
}

void PartLibraryPanel::RenderLibraryFolder(const LibraryFolder &folder, const string &label)
{
   for (const std::filesystem::path &pack : folder.packs)
   {
      ImGui::TreeNodeEx(PathToUTF8(pack.filename()).c_str(),
         ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (pack == m_selectedPack ? ImGuiTreeNodeFlags_Selected : 0));
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("%s", PathToUTF8(pack).c_str());
      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      {
         m_selectedPack = pack;
         m_selectedPackInfo = ScanPack(pack);
         // Selecting a pack arms the placement of its whole content
         m_placementPack = pack;
         StartPlacement();
      }
   }
   for (const LibraryFolder &sub : folder.subFolders)
   {
      const bool open = ImGui::TreeNodeEx(PathToUTF8(sub.path.filename()).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth);
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("%s", PathToUTF8(sub.path).c_str());
      if (open)
      {
         RenderLibraryFolder(sub, label);
         ImGui::TreePop();
      }
   }
}

vector<std::filesystem::path> PartLibraryPanel::GetLibraryFolders() const
{
   // The shipped 'assets/library' folder is the default part library folder: it is created when missing
   // (best effort, the install folder may be read only) and prepended to the user folders unless the user
   // removed it, which is persisted as an exclusion entry prefixed with '-' in the stored folder list
   const std::filesystem::path defaultFolder = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "library";
   std::error_code ec;
   std::filesystem::create_directories(defaultFolder, ec);

   vector<std::filesystem::path> folders;
   ankerl::unordered_dense::set<string> excluded;
   const string stored = g_settingsService.GetAppSettings().GetEditor_PartLibraryFolders();
   size_t start = 0;
   while (start <= stored.size())
   {
      const size_t end = stored.find(';', start);
      const string entry = stored.substr(start, end == string::npos ? string::npos : end - start);
      if (!entry.empty())
      {
         if (entry[0] == '-')
            excluded.insert(lowerCase(PathToUTF8(PathFromString(entry.substr(1)).lexically_normal())));
         else
            folders.push_back(PathFromString(entry));
      }
      if (end == string::npos)
         break;
      start = end + 1;
   }

   const string defaultKey = lowerCase(PathToUTF8(defaultFolder.lexically_normal()));
   const bool listed = std::ranges::find_if(folders, [&defaultKey](const std::filesystem::path &f) { return lowerCase(PathToUTF8(f.lexically_normal())) == defaultKey; }) != folders.end();
   if (!listed && !excluded.contains(defaultKey))
      folders.insert(folders.begin(), defaultFolder.lexically_normal());
   return folders;
}

void PartLibraryPanel::SetLibraryFolders(const vector<std::filesystem::path> &folders)
{
   // Preserve the exclusion entries (prefixed with '-') of the stored list
   string stored;
   const string previous = g_settingsService.GetAppSettings().GetEditor_PartLibraryFolders();
   size_t start = 0;
   while (start <= previous.size())
   {
      const size_t end = previous.find(';', start);
      const string entry = previous.substr(start, end == string::npos ? string::npos : end - start);
      if (entry.starts_with('-'))
         stored += (stored.empty() ? ""s : ";"s) + entry;
      if (end == string::npos)
         break;
      start = end + 1;
   }
   for (const std::filesystem::path &folder : folders)
      stored += (stored.empty() ? ""s : ";"s) + PathToString(folder);
   g_settingsService.GetAppSettings().SetEditor_PartLibraryFolders(stored, false);
   m_rescanLibrary = true;
}

void PartLibraryPanel::AddLibraryFolder(const std::filesystem::path &folder)
{
   vector<std::filesystem::path> folders = GetLibraryFolders();
   const std::filesystem::path normalized = folder.lexically_normal();
   for (const std::filesystem::path &existing : folders)
      if (StrCompareNoCase(PathToUTF8(existing.lexically_normal()), PathToUTF8(normalized)))
         return;
   folders.push_back(normalized);
   SetLibraryFolders(folders);
}

void PartLibraryPanel::RemoveLibraryFolder(size_t index)
{
   vector<std::filesystem::path> folders = GetLibraryFolders();
   if (index >= folders.size())
      return;
   const std::filesystem::path removed = folders[index];
   folders.erase(folders.begin() + static_cast<ptrdiff_t>(index));
   // Removing the default library folder is persisted as an exclusion entry so it does not come back
   const std::filesystem::path defaultFolder = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets) / "library";
   string stored;
   if (StrCompareNoCase(PathToUTF8(removed.lexically_normal()), PathToUTF8(defaultFolder.lexically_normal())))
      stored = "-"s + PathToString(defaultFolder.lexically_normal());
   for (const std::filesystem::path &folder : folders)
      stored += (stored.empty() ? ""s : ";"s) + PathToString(folder);
   g_settingsService.GetAppSettings().SetEditor_PartLibraryFolders(stored, false);
   m_rescanLibrary = true;
}

void PartLibraryPanel::RescanLibrary()
{
   m_library.clear();
   const std::function<void(LibraryFolder &)> scanFolder = [&scanFolder](LibraryFolder &node)
   {
      std::error_code ec;
      vector<std::filesystem::path> subdirs;
      for (const std::filesystem::directory_entry &entry : std::filesystem::directory_iterator(node.path, ec))
      {
         if (entry.is_directory(ec))
            subdirs.push_back(entry.path());
         else if (entry.is_regular_file(ec) && lowerCase(PathToUTF8(entry.path().extension())) == ".vpz"s)
            node.packs.push_back(entry.path());
      }
      std::ranges::sort(subdirs);
      std::ranges::sort(node.packs);
      for (const std::filesystem::path &subdir : subdirs)
      {
         node.subFolders.emplace_back().path = subdir;
         scanFolder(node.subFolders.back());
      }
   };
   for (const std::filesystem::path &folder : GetLibraryFolders())
   {
      std::error_code ec;
      if (!std::filesystem::is_directory(folder, ec))
         continue;
      m_library.emplace_back().path = folder;
      scanFolder(m_library.back());
   }
}

PartLibraryPanel::PackInfo PartLibraryPanel::ScanPack(const std::filesystem::path &path)
{
   PackInfo info;
   const auto pack = JSONSerializer::CreateReader(path);
   if (!pack)
      return info;
   info.valid = true;
   for (const std::filesystem::path &file : pack->ListFiles())
   {
      const std::filesystem::path folder = file.parent_path();
      if (file == "table.json"s)
         info.hasTable = true;
      else if (file.extension() == ".json" && folder == "parts"s)
         info.parts++;
      else if (file.extension() == ".json" && folder == "collections"s)
         info.collections++;
      else if (file.extension() == ".json" && folder == "materials"s)
         info.materials++;
      else if (file.extension() == ".json" && folder == "images"s)
         info.images++;
      else if (file.extension() == ".json" && folder == "sounds"s)
         info.sounds++;
      else if (file.extension() == ".json" && folder == "fonts"s)
         info.fonts++;
   }
   return info;
}

// Imports the parts and referenced assets of a VPZ pack into the edited table, merging them
// with the existing content: assets are shared by name, part and collection names are made unique
void PartLibraryPanel::ImportPack(const std::filesystem::path &path)
{
   if (m_editor.m_table->IsLocked() || m_editor.IsInspectMode())
      return;

   CComObject<PinTable> *temp;
   CComObject<PinTable>::CreateInstance(&temp);
   temp->AddRef();
   VPXFileFeedback feedback;
   // The merge strategy may rename the assets of the pack on load (conflicting and not equal ones)
   const std::unique_ptr<JSONSerializer::Deserializer> pack = m_editor.m_table->CreateImportDeserializer(path, m_mergeStrategy);
   const HRESULT hr = pack != nullptr ? temp->LoadGameFromJSONPack(*pack, feedback) : E_FAIL;
   // Same as the table load: hash/corrupt content errors still keep the loaded parts
   if (FAILED(hr) && (hr != APPX_E_BLOCK_HASH_INVALID) && (hr != APPX_E_CORRUPT_CONTENT))
   {
      temp->Release();
      m_editor.m_liveUI.PushNotification("Failed to import '"s + PathToUTF8(path) + '\'', 10000);
      return;
   }

   PinTable *const table = m_editor.m_table;
   PartGroup *const target = m_editor.GetPartGroupForNewPart();

   // Move the parts, collections and assets of the pack to the edited table (merged by name)
   m_editor.m_undo.BeginUndo();
   const vector<IEditable *> imported = table->ImportParts(temp, target);
   temp->Release();
   for (IEditable *const part : imported)
   {
      part->AddRef(); // Created parts keep a creation reference, released by the undo record (see PinTable::Uncreate)
      m_editor.m_undo.MarkForCreate(part);
   }

   // Player side setup, same as part creation
   for (IEditable *const part : imported)
      SetupPlayerSide(part, true);
   m_editor.m_undo.EndUndo();

   // Select the imported parts
   m_editor.UpdateEditableList();
   m_editor.m_multiSel.clear();
   for (IEditable *const part : imported)
      if (const auto it = m_editor.m_editableMap.find(part); it != m_editor.m_editableMap.end())
         m_editor.m_multiSel.push_back(it->second);
   if (!m_editor.m_multiSel.empty())
      m_editor.m_selection = Selection(m_editor.m_multiSel.back());

   m_editor.m_liveUI.PushNotification(
      "Imported "s + std::to_string(imported.size()) + " part(s) from '"s + PathToUTF8(path.filename()) + "' into '"s + (target != nullptr ? target->GetName() : "(table root)"s) + '\'',
      10000);
}

void PartLibraryPanel::SetupPlayerSide(IEditable *part, const bool initRenderable)
{
   if (part->GetItemType() == eItemBall)
      m_editor.m_player->m_vball.push_back(static_cast<Ball *>(part));
   m_editor.m_player->TimerSetup(part);
   if (initRenderable)
   {
      if (auto *const renderable = part->GetIRenderable(); renderable)
         renderable->RenderSetup(m_editor.m_renderer.get());
   }
   else
      m_editor.m_renderer->ReinitRenderable(part->GetIRenderable());
   if (part->GetIHitable())
      m_editor.m_player->m_physics->Add(part);
}

// Pack placement: the selected pack is loaded in a scratch table so that it never pollutes the
// edited one until it is committed. Its parts are kept invisible and only rendered as ghosts by
// Render3D while the mouse hovers the 3D playfield view, positioned by UpdatePlacement.
void PartLibraryPanel::StartPlacement()
{
   ClearPreview();
   if (m_placementPack.empty() || m_editor.m_table->IsLocked() || m_editor.IsInspectMode())
      return;

   CComObject<PinTable> *temp;
   CComObject<PinTable>::CreateInstance(&temp);
   temp->AddRef();
   VPXFileFeedback feedback;
   // The merge strategy is applied now so that the previewed parts already carry the asset names they will get when committed
   const std::unique_ptr<JSONSerializer::Deserializer> pack = m_editor.m_table->CreateImportDeserializer(m_placementPack, m_mergeStrategy);
   const HRESULT hr = pack != nullptr ? temp->LoadGameFromJSONPack(*pack, feedback) : E_FAIL;
   if ((FAILED(hr) && (hr != APPX_E_BLOCK_HASH_INVALID) && (hr != APPX_E_CORRUPT_CONTENT)) || temp->GetParts().empty())
   {
      temp->Release();
      m_editor.m_liveUI.PushNotification("Failed to load '"s + PathToUTF8(m_placementPack) + '\'', 10000);
      return;
   }

   m_previewTable = temp;
   for (IEditable *const part : temp->GetParts())
   {
      if (part->GetItemType() == eItemPartGroup)
         continue;
      m_previewParts.push_back(part);
      std::shared_ptr<EditorUIPart> uiPart = EditorUIPartRegistry::Create(part);
      m_previewUIParts.push_back(uiPart != nullptr ? uiPart : std::make_shared<BaseUIPart>(part));
      if (auto *const renderable = part->GetIRenderable(); renderable)
         renderable->RenderSetup(m_editor.m_renderer.get());
   }
   m_previewPos = Vertex2D(FLT_MAX, FLT_MAX); // Force the first placement update
   m_previewZ = FLT_MAX;
}

void PartLibraryPanel::CancelPlacement()
{
   m_placementPack.clear();
   ClearPreview();
}

void PartLibraryPanel::UpdatePlacement(const ImVec2 &mousePos)
{
   const ImGuiIO &io = ImGui::GetIO();
   ViewportPane *const vp = m_editor.GetTargetViewport();
   BlueprintPane *const bp = m_editor.m_hoverBp;
   const bool hovering = ((bp != nullptr) || ((vp != nullptr) && (m_editor.m_hoverVp == vp) && (vp->m_viewType != ViewMode::DesktopBackdrop))) && m_editor.m_fileOp == nullptr
      && m_editor.m_player->m_playfieldWnd != nullptr;
   if (!hovering)
   {
      // Re-hide the ghost parts when the mouse leaves the view panes
      m_previewVisible = false;
      return;
   }

   // Project the pointed point on the playfield XY plane (the blueprint maps it directly, the 3D view
   // projects on the playfield plane), then drop a vertical ray on it: the parts rest on the topmost
   // surface it hits (the UI quadtree holds all parts, collidable or not)
   const Vertex2D pos = (bp != nullptr) ? bp->Unproject(mousePos) : vp->UnprojectToPlane(mousePos, 0.f);
   const float zTop = m_editor.m_table->m_glassTopHeight + 500.f;
   const float zBottom = -500.f;
   vector<HitTestResult> hits;
   m_editor.m_player->m_physics->RayCast(Vertex3Ds(pos.x, pos.y, zTop), Vertex3Ds(pos.x, pos.y, zBottom), true, hits);
   const float restZ = hits.empty() ? 0.f : zTop + hits.front().m_time * (zBottom - zTop);

   if (pos.x == m_previewPos.x && pos.y == m_previewPos.y && restZ == m_previewZ)
   {
      m_previewVisible = true;
      return;
   }
   m_previewPos = pos;
   m_previewZ = restZ;

   // Move the whole pack content rigidly: the centroid of its part centers tracks the pointed point
   Vertex2D centroid(0.f, 0.f);
   for (IEditable *const part : m_previewParts)
   {
      const Vertex2D c = part->GetCenter();
      centroid.x += c.x;
      centroid.y += c.y;
   }
   if (!m_previewParts.empty())
   {
      centroid.x /= static_cast<float>(m_previewParts.size());
      centroid.y /= static_cast<float>(m_previewParts.size());
   }
   const Vertex2D delta(pos.x - centroid.x, pos.y - centroid.y);
   for (IEditable *const part : m_previewParts)
      part->Translate(delta);

   // Raise the content on the Z axis so its lowest bound lies on the hit surface. Only parts exposing
   // a Z translation get raised; curve based parts (ramps, surfaces, ...) keep their authored Z
   float minZ = FLT_MAX;
   for (IEditable *const part : m_previewParts)
   {
      vector<Vertex3Ds> bounds;
      part->GetBoundingVertices(bounds, nullptr);
      for (const Vertex3Ds &v : bounds)
         minZ = min(minZ, v.z);
   }
   if (minZ != FLT_MAX)
      for (size_t i = 0; i < m_previewParts.size(); i++)
      {
         Matrix3D transform;
         if ((m_previewUIParts[i]->GetTransform(transform) & EditorUIPart::TM_TransZ) != 0)
         {
            vec3 p, s, r;
            ExtractTRS(transform, p, s, r);
            p.z += restZ - minZ;
            m_previewUIParts[i]->SetTransform(p, s, r);
         }
      }
   for (IEditable *const part : m_previewParts)
      m_editor.m_renderer->ReinitRenderable(part->GetIRenderable());
   m_previewVisible = true;
}

void PartLibraryPanel::CommitPlacement()
{
   if (!IsPlacementArmed() || !m_previewVisible || m_editor.m_table->IsLocked() || m_editor.IsInspectMode())
      return;

   PinTable *const table = m_editor.m_table;
   PartGroup *const target = m_editor.GetPartGroupForNewPart();

   // Move the preview parts to the edited table: they are already at the pointed position. Their
   // assets (already merged by name at preview load) and authored part groups follow them
   m_previewParts.clear();
   m_previewUIParts.clear();
   m_editor.m_undo.BeginUndo();
   const vector<IEditable *> imported = table->ImportParts(m_previewTable, target);
   for (IEditable *const part : imported)
   {
      part->AddRef(); // Created parts keep a creation reference, released by the undo record (see PinTable::Uncreate)
      m_editor.m_undo.MarkForCreate(part);
   }
   // Player side setup, same as part creation; the renderables are re-initialized so that they resolve
   // the assets of the edited table (pack assets skipped by the merge are released with the scratch table)
   for (IEditable *const part : imported)
      SetupPlayerSide(part, false);
   m_editor.m_undo.EndUndo();
   m_previewTable->Release();
   m_previewTable = nullptr;

   // Select the imported parts
   m_editor.UpdateEditableList();
   m_editor.m_multiSel.clear();
   for (IEditable *const part : imported)
      if (part->GetItemType() != eItemPartGroup)
         if (const auto it = m_editor.m_editableMap.find(part); it != m_editor.m_editableMap.end())
            m_editor.m_multiSel.push_back(it->second);
   if (!m_editor.m_multiSel.empty())
      m_editor.m_selection = Selection(m_editor.m_multiSel.back());

   m_editor.m_liveUI.PushNotification("Placed "s + std::to_string(imported.size()) + " part(s) from '"s + PathToUTF8(m_placementPack.filename()) + "' into '"s
         + (target != nullptr ? target->GetName() : "(table root)"s) + '\'',
      10000);

   // Stay armed: reload the preview so that another copy can be placed on the next click
   if (!m_placementPack.empty())
      StartPlacement();
}

void PartLibraryPanel::ClearPreview()
{
   m_previewParts.clear();
   m_previewUIParts.clear();
   if (m_previewTable != nullptr)
   {
      if (m_editor.m_renderer != nullptr)
         for (IEditable *const edit : m_previewTable->GetParts())
         {
            if (edit->GetIRenderable() == nullptr)
               continue;
            // Same deferred release as part deletion: the render thread may still use the render state
            m_editor.m_renderer->DropRenderable(edit->GetIRenderable());
            edit->AddRef();
            edit->m_renderReleasePending = true;
            m_editor.m_renderer->m_renderDevice->AddEndOfFrameCmd(
               [edit]()
               {
                  edit->GetIRenderable()->RenderRelease();
                  edit->m_renderReleasePending = false;
                  edit->Release();
               });
         }
      m_previewTable->Release();
      m_previewTable = nullptr;
   }
   m_previewVisible = false;
}

void PartLibraryPanel::Render3D()
{
   if (m_previewVisible)
      for (IEditable *const part : m_previewParts)
         if (part->GetIRenderable() != nullptr)
            m_editor.m_renderer->RenderItem(part, false);
}

void PartLibraryPane::RenderPane(const AreaRect &rect, const int leafId) { m_editor.m_partLibrary.RenderPane(rect, leafId); }

void PartLibraryPane::OnUnhosted() { m_editor.m_partLibrary.ClearPreview(); }
}
