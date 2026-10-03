// license:GPLv3+

#include "core/stdafx.h"
#include "PartLibraryPanel.h"

#include "core/Settings.h"
#include "core/SettingsService.h"
#include "fonts/IconsForkAwesome.h"
#include "parts/ball.h"
#include "parts/pintable.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"
#include "ui/VPXFileFeedback.h"
#include "utils/JSONSerializer.h"

// Title (used as Id) of the modal dialog
#define ID_PART_LIBRARY "Part Library"

namespace VPX::EditorUI
{

PartLibraryPanel::PartLibraryPanel(EditorUI &editor)
   : m_editor(editor)
{
}

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
   SDL_ShowSaveFileDialog(
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

void PartLibraryPanel::Render()
{
   // Apply the file picked by the asynchronous 'Export' save dialog (deferred to the main thread)
   if (m_pendingExportPath && !m_pendingExportPath->empty())
   {
      std::filesystem::path file = PathFromUTF8(*m_pendingExportPath);
      m_pendingExportPath = nullptr;
      if (lowerCase(PathToUTF8(file.extension())) != ".vpz"s)
         file.replace_extension(".vpz");
      vector<IEditable *> parts;
      for (const auto &part : m_editor.m_multiSel)
         parts.push_back(part->GetEditable());
      VPXFileFeedback feedback;
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

   if (!m_visible)
      return;

   if (m_rescanLibrary)
   {
      m_rescanLibrary = false;
      RescanLibrary();
   }

   const float dpi = m_editor.m_liveUI.GetDPI();
   ImGui::SetNextWindowSize(ImVec2(560.f * dpi, 420.f * dpi), ImGuiCond_FirstUseEver);
   if (ImGui::Begin(ID_PART_LIBRARY, &m_visible))
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
            SDL_ShowOpenFileDialog(
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
            SDL_ShowOpenFolderDialog(
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
            if (ImGui::SmallButton(ICON_FK_TRASH_O))
            {
               RemoveLibraryFolder(i);
               ImGui::PopID();
               break;
            }
            if (ImGui::IsItemHovered())
               ImGui::SetTooltip("Remove this library folder");
            ImGui::SameLine();
            const LibraryFolder &root = m_library[i];
            const string label = root.path.filename().empty() ? PathToUTF8(root.path) : PathToUTF8(root.path.filename());
            const bool open = ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen);
            if (ImGui::IsItemHovered())
               ImGui::SetTooltip("%s", PathToUTF8(root.path).c_str());
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
         ImGui::BeginDisabled(m_editor.m_table->IsLocked() || !m_selectedPackInfo.valid);
         if (ImGui::Button(ICON_FK_DOWNLOAD " Import"))
            ImportPack(m_selectedPack);
         ImGui::EndDisabled();
      }
   }
   ImGui::End();
}

void PartLibraryPanel::RenderLibraryFolder(const LibraryFolder &folder, const string &label)
{
   for (const std::filesystem::path &pack : folder.packs)
   {
      ImGui::TreeNodeEx(PathToUTF8(pack.filename()).c_str(),
         ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth | (pack == m_selectedPack ? ImGuiTreeNodeFlags_Selected : 0));
      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      {
         m_selectedPack = pack;
         m_selectedPackInfo = ScanPack(pack);
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
   vector<std::filesystem::path> folders;
   const string stored = g_settingsService.GetAppSettings().GetEditor_PartLibraryFolders();
   size_t start = 0;
   while (start <= stored.size())
   {
      const size_t end = stored.find(';', start);
      const string entry = stored.substr(start, end == string::npos ? string::npos : end - start);
      if (!entry.empty())
         folders.push_back(PathFromString(entry));
      if (end == string::npos)
         break;
      start = end + 1;
   }
   return folders;
}

void PartLibraryPanel::SetLibraryFolders(const vector<std::filesystem::path> &folders)
{
   string stored;
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
   folders.erase(folders.begin() + static_cast<ptrdiff_t>(index));
   SetLibraryFolders(folders);
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
      m_editor.m_undo.MarkForCreate(part);

   // Player side setup, same as part creation
   for (IEditable *const part : imported)
   {
      if (part->GetItemType() == eItemBall)
         m_editor.m_player->m_vball.push_back(static_cast<Ball *>(part));
      m_editor.m_player->TimerSetup(part);
      if (auto *const renderable = part->GetIRenderable(); renderable)
         renderable->RenderSetup(m_editor.m_renderer.get());
      if (part->GetIHitable())
         m_editor.m_player->m_physics->Add(part);
   }
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

}
