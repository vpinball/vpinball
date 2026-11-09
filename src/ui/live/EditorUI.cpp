// license:GPLv3+

#include "core/stdafx.h"
#include "EditorUI.h"

#include "core/TableDB.h"
#include "core/VPXPluginAPIImpl.h"
#include "core/editablereg.h"
#include "core/extern.h"
#include "core/FileLocator.h"
#include "core/VPApp.h"
#include "core/vpversion.h"
#include "ui/live/FileDialog.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "editor/BlueprintPane.h"
#include "editor/EditorTheme.h"
#include "editor/parts/EditorUIPart.h"
#include "editor/parts/EditorUIPartRegistry.h"
#include "IconsLucide.h"
#include "editor/LiveRenderContext.h"
#include "editor/ViewportPane.h"

#include "imgui/imgui_internal.h"

#include "parts/Collection.h"
#include "parts/PartGroup.h"
#include "parts/ball.h"
#include "parts/light.h"
#include "parts/Material.h"
#include "parts/primitive.h"
#include "math/dragpoint.h"
#include "math/matrix.h"

#include "physics/PhysicsEngine.h"
#include "plugins/VPXPlugin.h"

#include "renderer/Anaglyph.h"
#include "renderer/Renderer.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "renderer/Shader.h"
#include "renderer/VRDevice.h"

#include "ui/EditorClipboard.h"
#include "ui/VPXFileFeedback.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"

#include "utils/BiffReader.h"
#include "utils/color.h"
#include "utils/JSONSerializer.h"

#include "imgui/imgui.h"

#if defined(ENABLE_DX9)
#include <shellapi.h>
#endif

#include "imgui/imgui_stdlib.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

namespace VPX::EditorUI
{

EditorUI::EditorUI(LiveUI &liveUI)
   : m_liveUI(liveUI)
   , m_player(g_pplayer)
   , m_renderer(m_player->m_renderer)
   , m_chrome(*this)
   , m_inspectionModal(*this)
   , m_partLibrary(*this)
   , m_scriptPanel(*this)
   , m_undo(m_player->m_ptable)
{
   m_table = m_player->m_ptable;
   m_pininput = &(m_player->m_pininput);

   EditorUIPartRegistry::InitRegistry();

   // Store the current selection in each undo record, so that undoing also restores it
   m_undo.SetEditorStateCapture([this]() { return std::any(CaptureUndoSelection()); });

   ClearSelection();
   ImGuizmo::AllowAxisFlip(false);

   const Settings &settings = g_settingsService.GetAppSettings();
   m_navOrbitSelection = settings.GetEditor_NavOrbitSelection();
   m_navZoomToMouse = settings.GetEditor_NavZoomToMouse();
   m_navInvertZoom = settings.GetEditor_NavInvertZoom();
   m_navOrbitSensitivity = settings.GetEditor_NavOrbitSensitivity();
   m_navFovY = settings.GetEditor_NavFovY();
   m_navAutoDepth = settings.GetEditor_NavAutoDepth();
   m_navSelectionDisplay = settings.GetEditor_NavSelectionDisplay();
   m_navAltMiddleButton = settings.GetEditor_NavAltMiddleButton();
   m_navTrackpad = settings.GetEditor_NavTrackpad();
   m_navEmulateNumpad = settings.GetEditor_NavEmulateNumpad();
   m_navBlenderTransform = settings.GetEditor_NavBlenderTransform();
   m_snapEnabled = settings.GetEditor_SnapEnabled();
   m_snapTarget = settings.GetEditor_SnapTarget();
   m_snapMove = settings.GetEditor_SnapMove();
   m_snapRotate = settings.GetEditor_SnapRotate();
   m_snapScale = settings.GetEditor_SnapScale();
   m_snapAffectMove = settings.GetEditor_SnapAffectMove();
   m_snapAffectRotate = settings.GetEditor_SnapAffectRotate();
   m_snapAffectScale = settings.GetEditor_SnapAffectScale();
   m_units = static_cast<Units>(settings.GetLiveEditor_Units());
}

bool EditorUI::IsSnapActive(const SnapKind kind) const
{
   // Ctrl inverts the snapping while it is held (Blender)
   if (m_snapEnabled == ImGui::GetIO().KeyCtrl)
      return false;
   switch (kind)
   {
   case SnapKind::Move: return m_snapAffectMove;
   case SnapKind::Rotate: return m_snapAffectRotate;
   case SnapKind::Scale: return m_snapAffectScale;
   }
   return false;
}

void EditorUI::ToggleSnapping()
{
   m_snapEnabled = !m_snapEnabled;
   g_settingsService.GetAppSettings().SetEditor_SnapEnabled(m_snapEnabled, false);
   Report(m_snapEnabled ? "Snapping on" : "Snapping off");
}

EditorUI::~EditorUI()
{
   // The table outlives the editor (Player deletes the UI first)
   m_fileOp.reset(); // Wait for a file operation in progress, its result being discarded with the session
   RestorePartsVisibility();
   SaveAreaLayoutSettings(); // Persist the layout and the view setups for the next session
}

EditorUI::AsyncFileOp::~AsyncFileOp()
{
   if (worker.joinable())
      worker.join();
   if (table != nullptr)
      table->Release();
}

void EditorUI::Open()
{
   if (m_isOpened)
      return;
   m_isOpened = true;
   m_renderer->DisableStaticPrePass(true);
   m_player->SetPlayState(false); // Suspend play while editing

   // Drop input events queued while the editor was closed (e.g. the key press that ended a play test), so they don't trigger editor shortcuts
   ImGui::GetIO().ClearEventsQueue();
   ImGui::GetIO().ClearInputKeys();

   if (IsInspectMode())
   {
      // Inspecting a live copy (play testing): use the normal player camera view with default shading
      m_camSetupPending = CamSetup { ViewMode::PreviewCam, true, Renderer::ShadeMode::Default, ViewportPane::PredefinedView::None };
   }
   else
   {
      // Enter with the user camera setup as an orthographic top view fitted on the playfield bounds, rendered as masked wireframe
      m_camSetupPending = CamSetup { ViewMode::EditorCam, false, Renderer::ShadeMode::Wireframe, ViewportPane::PredefinedView::Top };
      // The playfield fit needs a bound viewport pane with a valid image rect: defer to the first rendered frame
      m_fitPlayfieldCamera = true;
   }
}

void EditorUI::Close()
{
   if (!m_isOpened)
      return;
   m_isOpened = false;
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()))
      {
         vp->m_flyMode = false;
         vp->OnUnhosted(); // Release the pane's render view: it must not draw over the game once the editor is closed
      }
   ExitPointEditMode(false);
   RestorePartsVisibility();
   m_inspectionModal.Close();
   m_partLibrary.CancelPlacement(); // Drops a pending pack placement; the part library and script areas stay in the layout
   SaveAreaLayoutSettings(); // The layout and the view setups survive the session end
   m_renderer->DisableStaticPrePass(false);
   m_renderer->SetShadeMode(Renderer::ShadeMode::Default);
   m_renderer->GetPrimaryView()->m_renderScene = true; // The primary view renders the scene again once the editor areas are gone
   if (!m_player->IsPlaying(false))
      m_player->SetPlayState(true); // Resume the play session suspended while editing
}

void EditorUI::SetTable(PinTable *const table)
{
   assert(table != nullptr);
   // Only tables of a same base table / live copy pair are supported (guaranteed by the player's SetTable)
   PinTable *const baseTable = m_table->m_liveBaseTable ? m_table->m_liveBaseTable : m_table;
   assert(table == baseTable || table->m_liveBaseTable == baseTable);
   m_table = table;

   // Drop all the state bound to the previous table's editables (undo is bound to the base table which outlives the switch)
   m_lastOp.type = LastOperation::Type::None;
   if (m_pointEditPart)
      m_pointEditPart->SetPointEditContext(nullptr);
   m_pointEditPart.reset();
   m_pointEditCenter = false;
   m_centerSelected = false;
   m_pointSel.clear();
   m_pointDragPending = false;
   m_pointDragActive = false;
   m_savedSelection = Selection();
   m_savedMultiSel.clear();
   OutlinerPanel::m_savedAnchor = nullptr;
   ClearSelection();
   m_dynamicParts.clear();
   m_editables.clear();
   m_editableMap.clear();
   m_partGroupUseHistory.clear();
   m_lastUndoPart = nullptr;
   m_addPartType = eItemInvalid;
   m_partLibrary.CancelPlacement();
}

void EditorUI::PlayTest()
{
   if (IsInspectMode())
      return;
   // The editor visibility overrides must not leak into the duplicated table used for the play session
   RestorePartsVisibility();
   PinTable *const liveTable = m_table->CopyForPlay();
   m_player->SetTable(liveTable, Player::TableTransition::Stack);
   // The pane views must not draw over the play session (RenderPane re-arms them when the editor comes back)
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()); vp != nullptr && vp->GetView() != nullptr) // A pane never rendered has no view
         vp->GetView()->m_renderScene = false; // The leaf is hidden while a playtest table is stacked
}

void EditorUI::SaveTable()
{
   if (m_table->IsLocked() || m_fileOp != nullptr)
   {
      m_liveUI.PushNotification(m_table->IsLocked() ? "Nothing saved: the table is locked"s : "Nothing saved: a file operation is still in progress"s, 8000);
      return;
   }
   PinTable *saveTable = m_table;
   if (IsInspectMode())
   {
      // Inspection session: write the startup table, after copying back what was edited in the 'Live' tabs
      saveTable = m_table->m_liveBaseTable;
      if (saveTable->m_filename.empty())
      {
         m_liveUI.PushNotification("Nothing saved: the table has no file yet, use Save As from the editor"s, 8000);
         return;
      }
      // FIXME The whole live m_d is copied, with what the running script changed in it (Visible, X, RotZ,... write m_d directly):
      // an animated or script hidden part is saved in its runtime state, not only with the fields edited in the 'Live' tab
      int skipped = 0;
      for (Material *const live : m_liveEditedMaterials)
         if (Material *const startup = m_table->GetStartupFromLive<Material>(live))
         {
            const string name = startup->m_name;
            *startup = *live;
            startup->m_name = name;
         }
      for (IEditable *const live : m_liveEditedParts)
      {
         IEditable *const startup = m_table->GetStartupFromLive<IEditable>(live);
         if (startup == nullptr)
            continue;
         if (live->GetItemType() == eItemPrimitive && startup->GetItemType() == eItemPrimitive)
            static_cast<Primitive *>(startup)->m_d = static_cast<Primitive *>(live)->m_d;
         else if (live->GetItemType() == eItemLight && startup->GetItemType() == eItemLight)
            static_cast<Light *>(startup)->m_d = static_cast<Light *>(live)->m_d;
         else
            skipped++;
      }
      m_liveEditedMaterials.clear();
      m_liveEditedParts.clear();
      if (skipped > 0)
         m_liveUI.PushNotification(std::to_string(skipped) + " edited part(s) of types other than primitive or light were NOT copied to the startup table"s, 8000);
   }
   else if (m_table->m_filename.empty())
   {
      SaveTableAs();
      return;
   }
   auto op = std::make_unique<AsyncFileOp>();
   op->type = AsyncFileOp::Type::Save;
   op->saveTable = saveTable;
   op->filename = saveTable->m_filename;
   op->displayPath = PathToUTF8(saveTable->m_filename);
   StartFileOp(std::move(op));
}

void EditorUI::SaveTableAs()
{
   if (IsInspectMode() || m_table->IsLocked() || m_player->m_playfieldWnd == nullptr)
      return;
   m_pendingSaveAsPath = std::make_shared<string>();
   m_pendingSaveAsFolder = false;
   std::filesystem::path defaultLocation = m_table->m_filename;
   const bool isPack = !defaultLocation.empty() && JSONSerializer::IsPack(defaultLocation);
   const SDL_DialogFileFilter tableFilter = { "Visual Pinball Tables", "vpx" };
   const SDL_DialogFileFilter packFilter = { "Visual Pinball Packs", "vpz" };
   const SDL_DialogFileFilter filters[] = { isPack ? packFilter : tableFilter, isPack ? tableFilter : packFilter };
   if (defaultLocation.empty())
      defaultLocation = PathFromString(m_table->GetSettings().GetRecentDir_LoadDir()) / "new_table.vpx";
   else if (lowerCase(PathToUTF8(defaultLocation.extension())) != ".vpz"s)
   {
      std::error_code ec;
      if (!std::filesystem::is_directory(defaultLocation, ec))
         defaultLocation.replace_extension(".vpx");
   }
   const string location = PathToUTF8(defaultLocation); // SDL expects UTF-8
   ShowSaveFileDialog(
      [](void *userdata, const char *const *filelist, int filter)
      {
         auto *res = static_cast<std::shared_ptr<string> *>(userdata);
         if (filelist != nullptr && filelist[0] != nullptr)
            **res = filelist[0];
         delete res;
      },
      new std::shared_ptr<string>(m_pendingSaveAsPath), //
      m_player->m_playfieldWnd->GetCore(), filters, static_cast<int>(std::size(filters)), location.empty() ? nullptr : location.c_str());
}

void EditorUI::SaveTableAsPackFolder()
{
   if (IsInspectMode() || m_table->IsLocked() || m_player->m_playfieldWnd == nullptr)
      return;
   m_pendingSaveAsPath = std::make_shared<string>();
   m_pendingSaveAsFolder = true;
   const std::filesystem::path defaultLocation = m_table->m_filename.empty() ? PathFromString(m_table->GetSettings().GetRecentDir_LoadDir()) : m_table->m_filename.parent_path();
   const string location = PathToUTF8(defaultLocation); // SDL expects UTF-8
   ShowOpenFolderDialog(
      [](void *userdata, const char *const *filelist, int filter)
      {
         auto *res = static_cast<std::shared_ptr<string> *>(userdata);
         if (filelist != nullptr && filelist[0] != nullptr)
            **res = filelist[0];
         delete res;
      },
      new std::shared_ptr<string>(m_pendingSaveAsPath), //
      m_player->m_playfieldWnd->GetCore(), location.empty() ? nullptr : location.c_str(), false);
}

void EditorUI::LoadTable() { LoadTableDialog(false); }

void EditorUI::LoadTableFolder() { LoadTableDialog(true); }

void EditorUI::LoadTableDialog(const bool folder)
{
   if (IsInspectMode() || m_player->m_playfieldWnd == nullptr)
      return;
   // Loading another table discards the edited table's unsaved changes: ask for confirmation first
   m_pendingLoadFolder = folder;
   if (m_table->GetDirtyState() != SaveDirtyState::Clean)
   {
      m_pendingNewTable.reset();
      m_pendingRecentPath.reset();
      m_pendingClose.reset();
      m_confirmLoadTable = true;
      return;
   }
   ShowLoadTableDialog(folder);
}

void EditorUI::LoadTable(const std::filesystem::path &path)
{
   if (IsInspectMode())
      return;
   // Same as LoadTableDialog: loading another table discards the edited table's unsaved changes, ask for confirmation first
   if (m_table->GetDirtyState() != SaveDirtyState::Clean)
   {
      m_pendingRecentPath = path;
      m_pendingNewTable.reset();
      m_pendingClose.reset();
      m_confirmLoadTable = true;
      return;
   }
   // The deferred file op of RenderUI (shared with the file dialog's pick) starts the load
   m_pendingLoadPath = std::make_shared<string>(PathToUTF8(path));
}

const vector<std::filesystem::path> &EditorUI::GetRecentTables()
{
   // The list is shared with the Win32 editor through the RecentDir.TableFileName settings: re-read it on
   // each call, as it may have been updated by the other editor since the last read
   m_recentTables.clear();
   for (int i = 0; i < LAST_OPENED_TABLE_COUNT; i++)
   {
      const string &tableName = g_settingsService.GetAppSettings().GetRecentDir_TableFileName(i);
      if (!tableName.empty())
         m_recentTables.push_back(PathFromUTF8OrString(tableName)); // UTF-8, older versions stored native narrow paths
   }
   return m_recentTables;
}

void EditorUI::NoteRecentTable(const std::filesystem::path &path)
{
   if (path.empty())
      return;
   GetRecentTables();
   // Most recently opened first, each path only once, capped at the setting slot count
   vector<std::filesystem::path> recents { path };
   for (const std::filesystem::path &recent : m_recentTables)
      if (recent != path && recents.size() < LAST_OPENED_TABLE_COUNT)
         recents.push_back(recent);
   m_recentTables = std::move(recents);
   for (size_t i = 0; i < m_recentTables.size(); i++)
      g_settingsService.GetAppSettings().SetRecentDir_TableFileName(static_cast<int>(i), PathToUTF8(m_recentTables[i]), false);
}

void EditorUI::RequestClose(const int closeState)
{
   // Only closing a full editor session loses the edited table (a live edit started from the Win32 editor or a play test gets back to its editor)
   if (m_player->IsEditorMode() && !IsInspectMode() && m_table->GetDirtyState() != SaveDirtyState::Clean)
   {
      m_pendingNewTable.reset();
      m_pendingRecentPath.reset();
      m_pendingClose = closeState;
      m_confirmLoadTable = true;
      return;
   }
   m_player->SetCloseState(static_cast<Player::CloseState>(closeState));
}

void EditorUI::ShowLoadTableDialog(const bool folder)
{
   m_pendingLoadPath = std::make_shared<string>();
   // Start on the edited table's file (right folder, preselected file), falling back to the recent load dir
   std::filesystem::path defaultLocation = m_table->m_filename;
   if (defaultLocation.empty())
      defaultLocation = PathFromString(m_table->GetSettings().GetRecentDir_LoadDir()) / "";
   else if (folder)
      defaultLocation = defaultLocation.parent_path();
   const string location = PathToUTF8(defaultLocation); // SDL expects UTF-8
   if (folder)
   {
      ShowOpenFolderDialog(
         [](void *userdata, const char *const *filelist, int filter)
         {
            auto *res = static_cast<std::shared_ptr<string> *>(userdata);
            if (filelist != nullptr && filelist[0] != nullptr)
               **res = filelist[0];
            delete res;
         },
         new std::shared_ptr<string>(m_pendingLoadPath), //
         m_player->m_playfieldWnd->GetCore(), location.empty() ? nullptr : location.c_str(), false);
      return;
   }
   const SDL_DialogFileFilter filters[] = { { "Visual Pinball Tables", "vpx;vpt;vpz" } };
   ShowOpenFileDialog(
      [](void *userdata, const char *const *filelist, int filter)
      {
         auto *res = static_cast<std::shared_ptr<string> *>(userdata);
         if (filelist != nullptr && filelist[0] != nullptr)
            **res = filelist[0];
         delete res;
      },
      new std::shared_ptr<string>(m_pendingLoadPath), //
      m_player->m_playfieldWnd->GetCore(), filters, 1, location.empty() ? nullptr : location.c_str(), false);
}

void EditorUI::NewTable(const NewTableTemplate templateType)
{
   if (IsInspectMode())
      return;
   // Creating a new table discards the edited table's unsaved changes: ask for confirmation first
   if (m_table->GetDirtyState() != SaveDirtyState::Clean)
   {
      m_pendingNewTable = templateType;
      m_pendingRecentPath.reset();
      m_pendingClose.reset();
      m_confirmLoadTable = true;
      return;
   }
   LoadTableTemplate(templateType);
}

void EditorUI::LoadTableTemplate(const NewTableTemplate templateType)
{
   if (m_fileOp != nullptr)
      return;
   string path;
   switch (templateType)
   {
   case NewTableTemplate::Stripped: path = "strippedTable.vpz"s; break;
   case NewTableTemplate::Example: path = "exampleTable.vpz"s; break;
   case NewTableTemplate::LightSeq: path = "lightSeqTable.vpz"s; break;
   case NewTableTemplate::Blank:
   default: path = "blankTable.vpz"s;
   }
   auto op = std::make_unique<AsyncFileOp>();
   op->type = AsyncFileOp::Type::NewTable;
   op->filename = g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, path);
   op->displayPath = path;
   StartFileOp(std::move(op));
}

void EditorUI::StartFileOp(std::unique_ptr<AsyncFileOp> op)
{
   assert(m_fileOp == nullptr);
   switch (op->type)
   {
   case AsyncFileOp::Type::Save:
   {
      m_lastOp.type = LastOperation::Type::None; // Adjusting after a save would change the table without marking it dirty
      if (!IsInspectMode())
         RestorePartsVisibility(); // Save the authored visibility, not the editor display state
      AsyncFileOp *const fileOp = op.get();
      PinTable *const table = op->saveTable != nullptr ? op->saveTable : m_table;
      op->worker = std::thread(
         [fileOp, table]()
         {
            fileOp->result = table->Save(fileOp->feedback);
            fileOp->completed = true;
         });
      break;
   }
   case AsyncFileOp::Type::Load:
   case AsyncFileOp::Type::NewTable:
   {
      AsyncFileOp *const fileOp = op.get();
      op->worker = std::thread(
         [fileOp]()
         {
            CComObject<PinTable> *table;
            CComObject<PinTable>::CreateInstance(&table);
            table->AddRef();
            if (fileOp->type == AsyncFileOp::Type::NewTable)
            {
               // Same initial table setup as the Win32 editor (WinEditor::OpenNewTable)
               table->m_glassTopHeight = table->m_glassBottomHeight = 210;
               for (int i = 0; i < 16; i++)
                  table->m_rgcolorcustom[i] = RGB(0, 0, 0);
            }
            const HRESULT hr = (fileOp->type == AsyncFileOp::Type::NewTable && !g_app->m_fileLocator.FetchAppFile(fileOp->filename, fileOp->feedback)) ? E_FAIL : table->LoadGameFromFilename(fileOp->filename, fileOp->feedback);
            // Same as the Win32 editor: hash/corrupt content errors still keep the loaded table
            if (SUCCEEDED(hr) || (hr == APPX_E_BLOCK_HASH_INVALID) || (hr == APPX_E_CORRUPT_CONTENT))
            {
               if (fileOp->type == AsyncFileOp::Type::NewTable)
               {
                  table->m_title = "New Table"s;
                  table->GetSettings().SetIniPath(std::filesystem::path());
                  table->m_filename.clear();
               }
               fileOp->table = table;
            }
            else
               table->Release();
            fileOp->result = hr;
            fileOp->completed = true;
         });
      break;
   }
   }
   m_fileOp = std::move(op);
}

void EditorUI::FinishFileOp()
{
   AsyncFileOp *const op = m_fileOp.get();
   const HRESULT result = op->result;
   switch (op->type)
   {
   case AsyncFileOp::Type::Load:
   case AsyncFileOp::Type::NewTable:
      if (op->table != nullptr)
      {
         if (op->type == AsyncFileOp::Type::Load)
         {
            g_settingsService.GetAppSettings().SetRecentDir_LoadDir(PathToString(op->filename.parent_path()), false);
            NoteRecentTable(op->filename);
         }
         PinTable *const table = op->table;
         op->table = nullptr;
         m_player->SetTable(table, Player::TableTransition::Replace); // The loaded table reference is adopted by the player
      }
      else
         m_liveUI.PushNotification(op->type == AsyncFileOp::Type::NewTable ? "Failed to create new table from '"s + op->displayPath : "Failed to load table '"s + op->displayPath, 10000);
      break;

   case AsyncFileOp::Type::Save:
      // Saves must always report their result: a failed save was previously silent (and looked like a successful one)
      // FIXME A failed write is reported twice: PinTable::WriteTableFile already shows IDS_SAVEERROR through ShowError
      if (FAILED(result))
         m_liveUI.PushNotification("SAVE FAILED: '" + op->displayPath + "'", 15000);
      else
         m_liveUI.PushNotification("Saved '" + op->displayPath + "'", 5000);
      if (SUCCEEDED(result))
      {
         m_undo.SetCleanPoint(SaveDirtyState::Clean);
         // Start a new undo record on next edit (instead of merging it with the previous one), so that the table gets marked dirty again
         m_lastUndoPart = nullptr;
         m_lastUndoId = 0;
         if (op->restoreOnFailure)
            g_settingsService.GetAppSettings().SetRecentDir_LoadDir(PathToString(m_table->m_filename.parent_path()), false);
#ifdef __EMSCRIPTEN__
         MAIN_THREAD_EM_ASM({ if (Module.onFileSaved) Module.onFileSaved(UTF8ToString($0)); }, op->displayPath.c_str());
#endif
      }
      else if (op->restoreOnFailure)
      {
         m_table->m_filename = op->previousFilename;
         m_table->m_title = op->previousTitle;
      }
      break;
   }
   m_fileOp.reset();
}

void EditorUI::RenderFileOpModal()
{
   // Once the operation is completed, the modal must still be closed explicitly: an open modal keeps capturing the inputs
   if (m_fileOp == nullptr)
   {
      if (ImGui::IsPopupOpen("###FileOp"))
         if (ImGui::BeginPopupModal("###FileOp", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
         {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
         }
      return;
   }
   const char *action;
   switch (m_fileOp->type)
   {
   case AsyncFileOp::Type::Load: action = "Loading table"; break;
   case AsyncFileOp::Type::NewTable: action = "Creating table"; break;
   default: action = "Saving table"; break;
   }
   const string title = action + "###FileOp"s;
   ImGui::OpenPopup(title.c_str());
   if (ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
   {
      if (!m_fileOp->displayPath.empty())
         ImGui::TextDisabled("%s", m_fileOp->displayPath.c_str());
      const unsigned int length = m_fileOp->feedback.length.load();
      const unsigned int progress = m_fileOp->feedback.progress.load();
      ImGui::ProgressBar(length != 0 ? static_cast<float>(progress) / length : 0.f, ImVec2(280.f * m_liveUI.GetDPI(), 0.f));
      if (length != 0)
         ImGui::Text("%u / %u", progress, length);
      ImGui::EndPopup();
   }
}

bool EditorUI::CanFrameSelection() const
{
   FRect3D bounds;
   return GetSelectionBounds(bounds);
}

void EditorUI::Render3D()
{
   // While a table save is running on the file operation worker thread, the table parts must not be
   // touched: the UI part renders write the editor visibility overrides into the parts' data (m_d),
   // racing with the serialization and saving the display state instead of the authored visibility
   // restored by RestorePartsVisibility when the save was started
   if (IsSaveInProgress())
      return;

   UpdateEditableList();

   // The view being recorded may belong to a viewport pane: the pane renders the scene parts with
   // its own camera mode and shading setup (the primary view renders the game, no editor parts)
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()); vp != nullptr && vp->GetView() == m_renderer->GetActiveView())
      {
         vp->Render3D();
         break;
      }

   m_partLibrary.Render3D();
}

void EditorUI::RenderUI()
{
   const ImGuiIO &io = ImGui::GetIO();
   ImGuizmo::BeginFrame();

   // The selection may not mix parts of the desktop backdrop with parts of the 3D scene: it keeps
   // the domain of its first part (selection entry points already filter, this is the last resort)
   if (!m_multiSel.empty())
   {
      const bool domain = m_multiSel.front()->GetEditable()->m_desktopBackdrop;
      std::erase_if(m_multiSel, [domain](const std::shared_ptr<EditorUIPart> &part) { return part->GetEditable()->m_desktopBackdrop != domain; });
   }
   if (m_selection.GetType() == Selection::S_EDITABLE && !IsPartSelected(m_selection.GetPart()))
      m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   if (OutlinerPanel::m_anchor && OutlinerPanel::m_anchor->GetEditable()->GetItemType() != eItemPartGroup && OutlinerPanel::m_anchor->GetEditable()->m_desktopBackdrop != SelectionDomain())
      OutlinerPanel::m_anchor.reset();

   // Add part mode housekeeping: it is only available in edit mode, when the table is not locked, and
   // while not in drag point edit mode, so cancel it if one of these conditions is no longer met
   if (m_addPartType != eItemInvalid && (IsInspectMode() || m_table->IsLocked() || m_pointEditPart != nullptr))
      m_addPartType = eItemInvalid;

   // Drag point edit mode housekeeping: exit without restoring the selection if the edited part is no
   // longer the active selected part, and drop selected points that do not exist anymore (points are
   // recreated on undo, ...)
   if (m_pointEditPart)
   {
      DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve();
      if (IsInspectMode() || (curve == nullptr && !m_pointEditPart->HasEditCenter()) || m_selection.GetType() != Selection::S_EDITABLE || m_selection.GetPart() != m_pointEditPart)
         ExitPointEditMode(false);
      else if (curve != nullptr)
      {
         std::erase_if(m_pointSel, [curve](const DragPoint *point) { return curve->GetPointIndex(point) < 0; });
      }
   }

   // Apply the file picked by the asynchronous 'Save As' file dialog (deferred to the main thread, in edit mode)
   if (!IsInspectMode() && m_pendingSaveAsPath && !m_pendingSaveAsPath->empty() && m_fileOp == nullptr)
   {
      // The table is saved through its filename: set it for the save, and restore it if the save fails (which reports the error)
      auto op = std::make_unique<AsyncFileOp>();
      op->type = AsyncFileOp::Type::Save;
      op->restoreOnFailure = true;
      op->previousFilename = m_table->m_filename;
      op->previousTitle = m_table->m_title;
      m_table->m_filename = PathFromUTF8(*m_pendingSaveAsPath);
      m_table->m_title = TitleFromFilename(m_table->m_filename);
      op->filename = m_table->m_filename;
      op->displayPath = PathToUTF8(m_table->m_filename);
      m_pendingSaveAsPath = nullptr;
      // A folder selection is a VPZ folder pack: the directory must exist for the table to be saved as a folder (not a .vpx file)
      if (m_pendingSaveAsFolder)
      {
         m_pendingSaveAsFolder = false;
         std::filesystem::create_directories(m_table->m_filename);
      }
      StartFileOp(std::move(op));
   }

   // Apply the file picked by the asynchronous 'Load' file dialog (deferred to the main thread, in edit mode):
   // the loaded table replaces the edited one through a player session switch (see Player::SetTable)
   if (!IsInspectMode() && m_pendingLoadPath && !m_pendingLoadPath->empty() && m_fileOp == nullptr)
   {
      auto op = std::make_unique<AsyncFileOp>();
      op->type = AsyncFileOp::Type::Load;
      op->filename = PathFromUTF8(*m_pendingLoadPath);
      op->displayPath = PathToUTF8(op->filename);
      m_pendingLoadPath = nullptr;
      StartFileOp(std::move(op));
   }

   // Apply the result of a completed asynchronous file operation (running a worker thread load/save)
   if (m_fileOp && m_fileOp->completed)
      FinishFileOp();

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))

   bool showFullUI = !m_inspectionModal.IsVisible() && !IsFlyMode();
   m_renderer->GetPrimaryView()->m_renderScene = !showFullUI; // While areas are drawn, the scene lives in the viewport panes
   if (!showFullUI)
   {
      // While the scene covers the window, it is driven by the user camera of the target viewport:
      // the flying pane's camera while flying (the renderer inspection dialog pushes its own override
      // after this, so it wins when both states apply)
      ViewportPane *const flyVp = GetTargetViewport();
      if (flyVp != nullptr && flyVp->m_flyMode && flyVp->m_cameraValid)
      {
         const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
         const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
         const float aspect = ImGui::GetIO().DisplaySize.x / ImGui::GetIO().DisplaySize.y;
         Matrix3D proj;
         if (flyVp->m_perspectiveCam)
            proj = Matrix3D::MatrixPerspectiveFovRH(m_navFovY, aspect, clamp(flyVp->m_camDistance * 0.05f, 5.f, 250.f), 50000.f);
         else
         {
            const float halfW = flyVp->m_camDistance * aspect, halfH = flyVp->m_camDistance;
            proj = Matrix3D::MatrixOrthoOffCenterRH(-halfW, halfW, -halfH, halfH, 0.5f, -50000.f);
         }
         m_renderer->GetPrimaryView()->SetViewProj(RH2LH * flyVp->m_camView * YAxis, YAxis * proj);
      }
      else if (!m_inspectionModal.IsVisible())
         m_renderer->GetPrimaryView()->m_customViewProj = false; // Back to the table camera once the fly/inspection ends
   }
   UpdateAreaLayout(); // Area rects are computed before the chrome draws the status overlay into the viewport area
   m_chrome.Render(showFullUI);
   if (!m_isOpened)
      return; // The editor was closed while rendering the chrome (e.g. the back-to-player button): its area views were released, stop here
   showFullUI &= !(m_player->IsEditorMode() && m_player->HasPendingTableTransition()); // The user may decide to play test the table
   if (showFullUI)
   {
      // Area panes, then their splitters and corner zones (above the pane windows)
      RenderAreas();
      RenderSplitters();
      RenderAdjustPanel(); // Floating panel of the viewport area, and the F9 popup
      if (ViewportPane *const vp = GetTargetViewport()) // Tool shelf (T) overlay of the target viewport pane
         vp->RenderToolShelf();
      m_chrome.RenderStatusBar(showFullUI); // Above the areas (the overlay-window popups are created later and stay on top)
   }
   else
   {
      // The areas are hidden: the viewport panes must not draw their scene over the primary view's one
      for (const auto &[id, view] : m_areaViews)
         if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()))
            vp->SetRendering(false);
   }
   m_inspectionModal.Render();
   m_partLibrary.Update();
   m_scriptPanel.Update();

#endif

   // Deferred playfield fit on editor opening, owned by the panes (it only applies when a pane has
   // no restored camera): just drop it when there is no viewport to fit
   if (m_fitPlayfieldCamera && GetTargetViewport() == nullptr)
      m_fitPlayfieldCamera = false;

   // Invisible full frame window for overlays
   ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
   ImGui::SetNextWindowPos(ImVec2(0, 0));
   ImGui::PushStyleColor(ImGuiCol_WindowBg, 0);
   ImGui::PushStyleColor(ImGuiCol_Border, 0);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::Begin("overlays", nullptr,
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings
         | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
   ImDrawList *const overlayDrawList = ImGui::GetWindowDrawList();

   // Confirmation popup for discarding unsaved changes when loading another table or closing (OpenPopup must be
   // called in the same window scope as the matching BeginPopupModal)
   // All variants share one popup id, so that a request arriving while it is open retargets it
   const char *const confirmTitle = m_pendingClose ? "Quit###ConfirmDiscard" : m_pendingNewTable ? "New Table###ConfirmDiscard" : "Open Table###ConfirmDiscard";
   if (m_confirmLoadTable)
   {
      m_confirmLoadTable = false;
      ImGui::OpenPopup(confirmTitle);
   }
   if (ImGui::BeginPopupModal(confirmTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
   {
      ImGui::TextUnformatted("The current table has unsaved changes which will be lost.");
      ImGui::NewLine();
      if (ImGui::Button("Discard changes"))
      {
         ImGui::CloseCurrentPopup();
         if (m_pendingClose)
         {
            m_player->SetCloseState(static_cast<Player::CloseState>(*m_pendingClose));
            m_pendingClose.reset();
         }
         else if (m_pendingNewTable)
         {
            LoadTableTemplate(*m_pendingNewTable);
            m_pendingNewTable.reset();
         }
         else if (m_pendingRecentPath)
         {
            m_pendingLoadPath = std::make_shared<string>(PathToUTF8(*m_pendingRecentPath));
            m_pendingRecentPath.reset();
         }
         else
            ShowLoadTableDialog(m_pendingLoadFolder);
      }
      ImGui::SameLine();
      ImGui::SetItemDefaultFocus();
      if (ImGui::Button("Cancel"))
      {
         m_pendingNewTable.reset();
         m_pendingRecentPath.reset();
         m_pendingClose.reset();
         ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
   }

   // Confirmation for layer reference space changes that can not re-express all of their children
   m_layerSpaceChangeOp.RenderModal();

   // Progress feedback of the asynchronous file operation (load/save run on a worker thread): a modal popup
   // that blocks all interaction while the operation is running
   RenderFileOpModal();

   ImGui::End();
   ImGui::PopStyleVar();
   ImGui::PopStyleColor(2);

   // 3D viewport interaction (camera moves, picking, box & point drags) and overlays (selection,
   // drag points, physic colliders): processed by the bound viewport pane through its camera
   // projection (no 3D viewport pane, no 3D interaction)
   for (const auto &[id, view] : m_areaViews)
   {
      // A pane whose leaf is not shown (hidden, or behind a maximized area) does not draw nor take
      // input: its image rect and item state are stale from the last frame it was drawn
      if (FindLeafRect(id) == nullptr)
         continue;
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()))
         vp->RenderUI(overlayDrawList);
      else if (auto *const bp = dynamic_cast<BlueprintPane *>(view.get()))
         bp->RenderUI(overlayDrawList);
   }

   // While a viewport pane runs a modal transform op (Blender G/S/R), it owns the keyboard:
   // editor-level shortcuts must not fire (and the op's undo record stays the last one)
   const ViewportPane *const modalVp = GetTargetViewport();
   const bool transformModal = modalVp != nullptr && modalVp->m_op.op != 0;
   // Ctrl+S must work while a widget (e.g. a slider) has keyboard focus; only a text input being edited takes precedence
   if (m_fileOp == nullptr && !transformModal && !io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_S, false) && io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      SaveTable();
   // Ctrl+Space maximizes / restores the area under the mouse (Blender)
   if (m_fileOp == nullptr && !transformModal && !io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Space, false) && io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      ToggleMaximizeArea();
   // Ctrl+F focuses the search field of the outliner under the mouse (or the first hosted one)
   if (m_fileOp == nullptr && !transformModal && !io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F, false) && io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
   {
      OutlinerPanel *outliner = nullptr;
      if (const int leaf = AreaLayout::LeafAt(m_leafRects, io.MousePos.x, io.MousePos.y); leaf >= 0)
         if (const auto it = m_areaViews.find(leaf); it != m_areaViews.end())
            outliner = dynamic_cast<OutlinerPanel *>(it->second.get());
      for (const auto &[id, view] : m_areaViews)
         if (outliner == nullptr)
            outliner = dynamic_cast<OutlinerPanel *>(view.get());
      if (outliner != nullptr)
         outliner->FocusFilter();
      else
      {
         // The outliner area is revealed (next frame) if a leaf of that type exists or can be created
         ShowAreaEditor(AreaType::Outliner);
         m_outlinerFocusPending = true;
      }
   }
   if (!io.WantCaptureKeyboard && m_fileOp == nullptr && !transformModal)
   {
      if (ImGui::IsKeyPressed(ImGuiKey_F5) && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      {
         // Play a shallow copy of the edited table
         if (!IsInspectMode() && !m_table->IsLocked())
            PlayTest();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_N))
      {
         // New table from the default blank template
         if (io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
            NewTable(NewTableTemplate::Blank);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_O))
      {
         // Open a table file, or a table pack folder with Shift
         if (io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
            LoadTable();
         else if (io.KeyCtrl && io.KeyShift && !io.KeyAlt)
            LoadTableFolder();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_A))
      {
         if (io.KeyAlt && !io.KeyCtrl && !io.KeyShift)
            ClearSelection();
         else if (!io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
         {
            // Select all: all curve points in drag point edit mode, the center point in center edit mode, all pickable parts otherwise
            if (m_pointEditPart)
            {
               if (m_pointEditCenter)
                  m_centerSelected = true;
               else
               {
                  m_pointSel.clear();
                  for (const auto &point : m_pointEditPart->GetDragPointCurve()->GetPoints())
                     m_pointSel.push_back(point.get());
               }
            }
            else
               SelectAllParts();
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_I, false) && io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
         InvertSelection(); // Blender's Select > Invert (Ctrl+I)
      else if (ImGui::IsKeyPressed(ImGuiKey_C))
      {
         // Copy the selected parts (or the coordinates of the selected drag point in point edit mode)
         if (io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
            CopySelection();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_V))
      {
         // Paste at the mouse position (or to the selected drag point in point edit mode)
         if (io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
         {
            const ImVec2 mousePos = ImGui::GetMousePos();
            PasteSelection(&mousePos);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
      {
         if (!io.KeyAlt && !io.KeyCtrl && !io.KeyShift)
         {
            if (m_pointEditPart)
               DeleteSelectedPoints();
            else
               DeleteSelection();
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_H))
      {
         if (m_table->m_liveBaseTable == nullptr && !io.KeyCtrl) // No UI visibility in inspection mode
         {
            bool changed = false;
            if (io.KeyAlt)
            { // Unhide all
               for (auto &part : m_editables)
                  if (part->GetEditable()->GetItemType() != eItemPartGroup && (part->GetEditable()->m_desktopBackdrop == IsBackdropEditMode()) && !part->GetEditable()->IsUIVisible(false))
                  {
                     part->GetEditable()->SetUIVisible(true);
                     changed = true;
                  }
            }
            else if (io.KeyShift)
            { // Hide unselected
               for (auto &part : m_editables)
                  if (part->GetEditable()->GetItemType() != eItemPartGroup && (part->GetEditable()->m_desktopBackdrop == IsBackdropEditMode()) && !IsPartSelected(part)
                     && part->GetEditable()->IsUIVisible(false))
                  {
                     part->GetEditable()->SetUIVisible(false);
                     changed = true;
                  }
            }
            else
            { // Hide selected
               for (const auto &part : m_multiSel)
                  if (part->GetEditable()->IsUIVisible(false))
                  {
                     part->GetEditable()->SetUIVisible(false);
                     changed = true;
                  }
               ClearSelection();
            }
            if (changed)
               m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
         }
      }
#ifndef __APPLE__
      else if (ImGui::IsKeyPressed(ImGuiKey_Y) && io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
         RedoSteps(1); // Ctrl+Y (on macOS ImGui maps Cmd to Ctrl and Cmd+Shift+Z is the redo)
#endif
      else if (ImGui::IsKeyPressed(ImGuiKey_Z) && io.KeyCtrl && !io.KeyAlt)
      { // Undo (Ctrl+Z), Redo (Ctrl+Shift+Z)
         if (io.KeyShift)
            RedoSteps(1);
         else
            UndoSteps(1);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_F9) && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      { // F9: Adjust Last Operation popup (opens only when an adjustable operation is current)
         if (m_lastOp.type != LastOperation::Type::None)
         {
            m_adjustPopupOpen = true;
            m_adjustPopupJustOpened = true;
            m_adjustPopupPos = ImGui::GetMousePos();
         }
      }
   }

   // Selected parts are kept dynamic in the UI quadtree so that they can be edited interactively. The selection also changes outside
   // of this pass (automation, undo, table switches) and parts get re-added as static (undelete), so the state is synced rather than
   // derived from selection changes
   std::erase_if(m_dynamicParts, [this](IEditable *edit) { return FindIndexOf(m_table->GetParts(), edit) == -1; });
   for (IEditable *const edit : m_dynamicParts)
      if (!IsEditableSelected(edit) && !m_player->m_physics->IsStatic(edit))
         m_player->m_physics->SetStatic(edit);
   std::erase_if(m_dynamicParts, [this](IEditable *edit) { return !IsEditableSelected(edit); });
   for (const auto &part : m_multiSel)
   {
      IEditable *const edit = part->GetEditable();
      if (edit->GetIHitable() == nullptr || edit->GetItemType() == eItemBall)
         continue;
      if (m_player->m_physics->IsStatic(edit))
         m_player->m_physics->SetDynamic(edit);
      if (FindIndexOf(m_dynamicParts, edit) == -1)
         m_dynamicParts.push_back(edit);
   }
}

void EditorUI::PushUndo(IEditable *part, unsigned int undoId, const string &label)
{
   // No undo support in inspection mode (not implemented as it needs to handle both version of the table)
   if (m_table->m_liveBaseTable)
      return;

   // Filter out continuous modification after first one (dragging a part, editing values,...)
   if (part == m_lastUndoPart && undoId == m_lastUndoId)
      return;

   m_lastUndoPart = part;
   m_lastUndoId = undoId;
   m_undo.BeginUndo();
   m_undo.MarkForUndo(part);
   if (!label.empty())
      m_undo.SetFieldLabel(label);
   m_undo.EndUndo();
}

bool EditorUI::IsPartSelected(const std::shared_ptr<EditorUIPart> &part) const { return part != nullptr && std::ranges::find(m_multiSel, part) != m_multiSel.end(); }

bool EditorUI::IsEditableSelected(const IEditable *editable) const
{
   for (const auto &part : m_multiSel)
      if (part->GetEditable() == editable)
         return true;
   return false;
}

void EditorUI::InvertSelection()
{
   if (m_pointEditPart)
   {
      if (m_pointEditCenter)
         m_centerSelected = !m_centerSelected;
      else
      {
         vector<DragPoint *> inverted;
         for (const auto &point : m_pointEditPart->GetDragPointCurve()->GetPoints())
            if (!IsPointSelected(point.get()))
               inverted.push_back(point.get());
         m_pointSel = std::move(inverted);
      }
      return;
   }
   // Same set as the select all: the pickable parts that are not selected
   const vector<std::shared_ptr<EditorUIPart>> previous = m_multiSel;
   SelectAllParts();
   std::erase_if(m_multiSel, [&previous](const auto &part) { return std::ranges::find(previous, part) != previous.end(); });
   m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   OutlinerPanel::m_anchor.reset();
}

void EditorUI::ClearSelection()
{
   m_selection = Selection();
   m_multiSel.clear();
   m_multiSelImages.clear();
   m_multiSelSounds.clear();
   m_multiSelMaterials.clear();
   OutlinerPanel::m_anchor.reset();
   m_outlinerImageAnchor = nullptr;
   m_outlinerSoundAnchor = nullptr;
   m_outlinerMaterialAnchor = nullptr;
}

EditorUI::UndoSelectionState EditorUI::CaptureUndoSelection() const
{
   UndoSelectionState state { m_selection, m_multiSel, OutlinerPanel::m_anchor, m_pointEditPart, {}, m_pointEditCenter };
   // Drag points are deleted and recreated when their part is reloaded (undo, ...): store their index in the curve
   if (DragPointCurve *const curve = m_pointEditPart ? m_pointEditPart->GetDragPointCurve() : nullptr)
   {
      for (const DragPoint *point : m_pointSel)
         state.pointSel.push_back(curve->GetPointIndex(point));
   }
   return state;
}

void EditorUI::RestoreUndoSelection(const UndoSelectionState &state)
{
   // Ensure the editable map is up to date (undo may have deleted or undeleted parts)
   UpdateEditableList();
   // Parts are resolved through the editable map since undo may have removed and recreated them
   const auto resolve = [this](const std::shared_ptr<EditorUIPart> &part) -> std::shared_ptr<EditorUIPart>
   {
      if (part == nullptr)
         return nullptr;
      const auto it = m_editableMap.find(part->GetEditable());
      return it != m_editableMap.end() ? it->second : nullptr;
   };
   // Restore the drag point edit mode state (selected points are resolved by index since they are recreated on part reload)
   const std::shared_ptr<EditorUIPart> pointEditPart = resolve(state.pointEditPart);
   if (m_pointEditPart != pointEditPart)
   {
      if (m_pointEditPart)
         m_pointEditPart->SetPointEditContext(nullptr);
      m_pointEditPart = pointEditPart;
      if (m_pointEditPart)
         m_pointEditPart->SetPointEditContext(this);
      m_pointDragPending = false;
      m_pointDragActive = false;
   }
   m_pointEditCenter = state.pointEditCenter && m_pointEditPart && m_pointEditPart->HasEditCenter();
   m_centerSelected = m_pointEditCenter;
   m_pointSel.clear();
   if (DragPointCurve *const curve = m_pointEditPart ? m_pointEditPart->GetDragPointCurve() : nullptr)
   {
      const auto &points = curve->GetPoints();
      for (const int index : state.pointSel)
         if (index >= 0 && index < (int)points.size())
            m_pointSel.push_back(points[index].get());
   }
   m_multiSel.clear();
   for (const auto &part : state.multiSel)
      if (std::shared_ptr<EditorUIPart> resolved = resolve(part))
         m_multiSel.push_back(resolved);
   // The selection may not mix desktop backdrop and 3D scene parts (defensive: snapshots are uniform already)
   if (!m_multiSel.empty())
   {
      const bool domain = m_multiSel.front()->GetEditable()->m_desktopBackdrop;
      std::erase_if(m_multiSel, [domain](const auto &part) { return part->GetEditable()->m_desktopBackdrop != domain; });
   }
   // Drop resources that no longer exist in the table
   std::erase_if(m_multiSelImages, [this](const Texture *image) { return std::ranges::find(m_table->m_vimage, image) == m_table->m_vimage.end(); });
   std::erase_if(m_multiSelSounds, [this](const VPX::Sound *sound) { return std::ranges::find(m_table->m_vsound, sound) == m_table->m_vsound.end(); });
   std::erase_if(m_multiSelMaterials, [this](const Material *material) { return std::ranges::find(m_table->m_materials, material) == m_table->m_materials.end(); });
   switch (state.selection.GetType())
   {
   case Selection::S_EDITABLE:
   {
      std::shared_ptr<EditorUIPart> part = resolve(state.selection.GetPart());
      if (part == nullptr || !IsPartSelected(part))
         part = m_multiSel.empty() ? nullptr : m_multiSel.back();
      m_selection = part ? Selection(part) : Selection();
      break;
   }
   case Selection::S_MATERIAL:
      m_selection = std::ranges::find(m_table->GetMaterialList(), state.selection.GetMaterial()) != m_table->GetMaterialList().end() ? state.selection : Selection();
      break;
   case Selection::S_IMAGE: m_selection = std::ranges::find(m_table->GetImageList(), state.selection.GetImage()) != m_table->GetImageList().end() ? state.selection : Selection(); break;
   case Selection::S_RENDERPROBE:
      m_selection = std::ranges::find(m_table->GetRenderProbeList(), state.selection.GetProbe()) != m_table->GetRenderProbeList().end() ? state.selection : Selection();
      break;
   case Selection::S_SOUND: m_selection = std::ranges::find(m_table->m_vsound, state.selection.GetSound()) != m_table->m_vsound.end() ? state.selection : Selection(); break;
   case Selection::S_COLLECTION:
      m_selection = std::ranges::find(m_table->GetCollections(), state.selection.GetCollection()) != m_table->GetCollections().end() ? state.selection : Selection();
      break;
   default: m_selection = state.selection; break; // S_NONE, S_CAMERA
   }
   OutlinerPanel::m_anchor = resolve(state.outlinerAnchor);
}

void EditorUI::SetSelection(const Selection &selection)
{
   m_selection = selection;
   m_multiSel.clear();
   m_multiSelImages.clear();
   m_multiSelSounds.clear();
   m_multiSelMaterials.clear();
   switch (selection.GetType())
   {
   case Selection::S_EDITABLE: m_multiSel.push_back(selection.GetPart()); break;
   case Selection::S_IMAGE: m_multiSelImages.push_back(selection.GetImage()); break;
   case Selection::S_SOUND: m_multiSelSounds.push_back(selection.GetSound()); break;
   case Selection::S_MATERIAL: m_multiSelMaterials.push_back(selection.GetMaterial()); break;
   default: break;
   }
}

void EditorUI::TogglePartSelection(const std::shared_ptr<EditorUIPart> &part)
{
   const auto it = std::ranges::find(m_multiSel, part);
   if (it == m_multiSel.end())
   {
      if (!IsPartInSelectionDomain(part->GetEditable()))
         return; // The selection may not mix desktop backdrop and 3D scene parts
      m_multiSel.push_back(part);
      m_selection = Selection(part);
   }
   else
   {
      const bool wasActive = m_selection.GetType() == Selection::S_EDITABLE && m_selection.GetPart() == part;
      m_multiSel.erase(it);
      if (m_multiSel.empty())
         m_selection = Selection();
      else if (wasActive)
         m_selection = Selection(m_multiSel.back());
   }
}

void EditorUI::SelectPartsInGroup(const PartGroup *group)
{
   // Select all the parts contained in the given group (and in its sub groups, recursively)
   m_multiSel.clear();
   for (const auto &uiPart : m_editables)
   {
      IEditable *const editable = uiPart->GetEditable();
      if (editable->GetItemType() != eItemPartGroup && editable->IsChild(group) && IsPartInSelectionDomain(editable))
         m_multiSel.push_back(uiPart);
   }
   m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   OutlinerPanel::m_anchor.reset();
}

bool EditorUI::CanMovePartToGroup(const IEditable *editable, const PartGroup *group)
{
   // A part can not be moved to itself, and a group can not be moved to itself or one of its descendants
   // (this would create a cycle). Only part groups may be moved to the root of the hierarchy (group-less parts are live objects).
   if (editable == nullptr || editable == group || editable->GetPartGroup() == group)
      return false;
   if (editable->GetItemType() == eItemPartGroup && group != nullptr && group->IsChild(static_cast<const PartGroup *>(editable)))
      return false;
   if (group == nullptr && editable->GetItemType() != eItemPartGroup)
      return false;
   return true;
}

bool EditorUI::MoveSelectionToPartGroup(PartGroup *group)
{
   // Gather the parts that can be moved to the target group (see CanMovePartToGroup)
   vector<IEditable *> moved;
   for (const auto &uiPart : m_multiSel)
      if (IEditable *const editable = uiPart->GetEditable(); CanMovePartToGroup(editable, group))
         moved.push_back(editable);
   if (moved.empty())
      return false;
   // Parts moved to a group on another reference space have their transforms re-expressed so that
   // they do not move (the move may be deferred for confirmation, see LayerSpaceChangeOp::RequestMove)
   if (!m_layerSpaceChangeOp.RequestMove(moved, group))
      return false;
   UpdateEditableList(); // Refresh the outliner paths and ordering
   return true;
}

bool EditorUI::IsPartInSelectionDomain(const IEditable *editable) const { return editable->m_desktopBackdrop == SelectionDomain(); }

PartGroupData::SpaceReference EditorUI::GetPartSpace(const IEditable *editable)
{
   if (editable->GetItemType() == eItemPartGroup)
      return static_cast<const PartGroup *>(editable)->GetReferenceSpace();
   const PartGroup *const group = editable->GetPartGroup();
   return group != nullptr ? group->GetReferenceSpace() : PartGroupData::SpaceReference::SR_PLAYFIELD;
}

std::optional<PartGroupData::SpaceReference> EditorUI::GetSelectionSpace() const
{
   // The selection defines a reference space only when all of its parts share the same one, and it
   // has none when a desktop backdrop part is involved (they live on their own 2D plane)
   std::optional<PartGroupData::SpaceReference> space;
   for (const auto &part : m_multiSel)
   {
      IEditable *const editable = part->GetEditable();
      if (editable == nullptr || editable->m_desktopBackdrop)
         return std::nullopt;
      const PartGroupData::SpaceReference partSpace = GetPartSpace(editable);
      if (space.has_value() && *space != partSpace)
         return std::nullopt;
      space = partSpace;
   }
   return space;
}

bool EditorUI::IsEditablePickable(const IEditable *editable, const bool backdropDomain) const
{
   // Picking is per view domain: a view only picks the parts it renders (3D scene or desktop backdrop)
   if (editable->m_desktopBackdrop != backdropDomain)
      return false;
   // The player mode visibility mask does not apply in the editor: UI visibility (including parent groups) decides
   if (!editable->IsUIVisible(true))
      return false;
   const auto type = editable->GetItemType();
   if (!HasFlag(m_selectionFilter, SelectionFilter::Playfield) && type == ItemTypeEnum::eItemPrimitive && static_cast<const Primitive *>(editable)->IsPlayfield())
      return false;
   if (!HasFlag(m_selectionFilter, SelectionFilter::Primitives) && type == ItemTypeEnum::eItemPrimitive)
      return false;
   if (!HasFlag(m_selectionFilter, SelectionFilter::Lights) && type == ItemTypeEnum::eItemLight)
      return false;
   if (!HasFlag(m_selectionFilter, SelectionFilter::Flashers) && type == ItemTypeEnum::eItemFlasher)
      return false;
   return true;
}

void EditorUI::SelectAllParts()
{
   // Select all the pickable parts
   m_multiSel.clear();
   for (const auto &uiPart : m_editables)
   {
      IEditable *const editable = uiPart->GetEditable();
      if (editable != nullptr && editable->GetItemType() != eItemPartGroup && IsEditablePickable(editable, IsBackdropEditMode()))
         m_multiSel.push_back(uiPart);
   }
   m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
}

// Undo and redo are off while a drag is in progress (its record is the top one: undoing it would leave the drag going unrecorded)
bool EditorUI::IsDragInProgress() const
{
   const ViewportPane *const vp = GetTargetViewport();
   return (vp != nullptr && (vp->m_op.op != ImGuizmo::OPERATION(0) || vp->m_gizmoDragActive)) || ImGuizmo::IsUsingAny() || m_pointDragActive || m_pointDragPending;
}

void EditorUI::CancelGizmoDrags()
{
   for (const auto &[id, view] : m_areaViews)
      view->CancelGizmoDrag();
}

bool EditorUI::CanUndo() const { return m_table->m_liveBaseTable == nullptr && m_undo.HasUndo() && !IsDragInProgress() && m_fileOp == nullptr; }

bool EditorUI::CanRedo() const { return m_table->m_liveBaseTable == nullptr && m_undo.HasRedo() && !IsDragInProgress() && m_fileOp == nullptr; }

void EditorUI::RestoreUndoState(const std::any &state)
{
   if (state.has_value())
      RestoreUndoSelection(std::any_cast<const UndoSelectionState &>(state));
}

string EditorUI::CountLabel(const char *verb, const size_t count, const string &name)
{
   return count == 1 ? string(verb) + ": " + name : string(verb) + ": " + std::to_string(count) + " parts";
}

void EditorUI::UndoSteps(const size_t count)
{
   if (!CanUndo() || count == 0)
      return;
   m_lastUndoPart = nullptr;
   m_lastUndoId = 0;
   string label;
   std::any state;
   size_t done = 0;
   bool stoppedEarly = false;
   // TODO handle IsUndoPastCleanPoint
   for (; done < count && m_undo.HasUndo(); ++done)
   {
      // A record that can not be redone would wipe the redo steps built so far: a multi-step undo stops before it (a single undo may clear redo)
      if (done > 0 && m_undo.NextUndoClearsRedo())
      {
         stoppedEarly = true;
         break;
      }
      label = m_undo.UndoLabel(0); // The label reported is the one of the last step actually undone
      if (std::any step = m_undo.Undo(); step.has_value())
         state = std::move(step);
   }
   RestoreUndoState(state);
   Report((done == 1 ? "Undo: " + label : "Undo: " + std::to_string(done) + " steps (" + label + ")") + (stoppedEarly ? ", stopped before a step that can not be redone" : ""));
}

void EditorUI::RedoSteps(const size_t count)
{
   if (!CanRedo() || count == 0)
      return;
   m_lastUndoPart = nullptr;
   m_lastUndoId = 0;
   string label;
   std::any state;
   size_t done = 0;
   for (; done < count && m_undo.HasRedo(); ++done)
   {
      label = m_undo.RedoLabel(0);
      if (std::any step = m_undo.Redo(); step.has_value())
         state = std::move(step);
   }
   RestoreUndoState(state);
   Report(done == 1 ? "Redo: " + label : "Redo: " + std::to_string(done) + " steps (" + label + ")");
}

const char *EditorUI::LastOperationName(const LastOperation::Type type)
{
   switch (type)
   {
   case LastOperation::Type::Add: return "Add Part";
   case LastOperation::Type::Paste: return "Paste";
   case LastOperation::Type::Duplicate: return "Duplicate";
   case LastOperation::Type::Move: return "Move";
   case LastOperation::Type::Rotate: return "Rotate";
   case LastOperation::Type::Scale: return "Scale";
   case LastOperation::Type::None: break;
   }
   return "";
}

void EditorUI::SetLastOperation(const LastOperation::Type type, const Matrix3D &initial)
{
   if (m_table->m_liveBaseTable != nullptr || m_pointEditPart != nullptr) // No undo record in inspection mode, and point edits are not operations on the selection
   {
      m_lastOp.type = LastOperation::Type::None;
      return;
   }
   const bool expanded = m_lastOp.expanded;
   m_lastOp = LastOperation();
   m_lastOp.expanded = expanded;
   m_lastOp.type = type;
   m_lastOp.initial = initial;
   m_lastOp.selection = m_multiSel;
   m_lastOp.undoId = m_undo.TopRecordId();
}

void EditorUI::NoteTransformOp(const ViewportPane::TransformOp &op)
{
   Matrix3D current;
   if (m_pointEditPart != nullptr || m_multiSel.empty() || !GetSelectionTransform(current))
      return;
   const auto axisIndex = [](const unsigned mask)
   {
      for (int i = 0; i < 3; ++i)
         if (mask == (1u << i))
            return i;
      return -1;
   };
   switch (op.op)
   {
   case ImGuizmo::TRANSLATE:
      SetLastOperation(LastOperation::Type::Move, op.startTransform);
      m_lastOp.offset = Vertex3Ds(current._41 - op.startTransform._41, current._42 - op.startTransform._42, current._43 - op.startTransform._43);
      break;
   case ImGuizmo::ROTATE:
   {
      SetLastOperation(LastOperation::Type::Rotate, op.startTransform);
      m_lastOp.pivot = op.pivot;
      const int axis = axisIndex(op.axisMask);
      Vertex3Ds dir = axis >= 0 ? op.startTransform.MultiplyVectorNoTranslate(Vertex3Ds(axis == 0 ? 1.f : 0.f, axis == 1 ? 1.f : 0.f, axis == 2 ? 1.f : 0.f))
                                : Vertex3Ds(op.viewDir.x, op.viewDir.y, op.viewDir.z);
      // Like ViewportPane::OpAxisDir, a degenerate local axis falls back to the world axis
      if (axis >= 0 && (op.space == ImGuizmo::WORLD || dir.LengthSquared() < 1e-12f))
         m_lastOp.axis = axis; // Re-evaluated along the matching world axis
      else
      {
         m_lastOp.axis = -1;
         dir.Normalize();
         m_lastOp.axisView = dir;
      }
      m_lastOp.angle = op.delta[0];
      break;
   }
   case ImGuizmo::SCALE:
   {
      SetLastOperation(LastOperation::Type::Scale, op.startTransform);
      m_lastOp.pivot = op.pivot;
      const int axis = axisIndex(op.axisMask);
      if (op.axisMask == 0x7)
      {
         m_lastOp.axis = -1;
         m_lastOp.factor = op.delta[0];
      }
      else if (axis >= 0)
      {
         m_lastOp.axis = axis;
         m_lastOp.factor = op.delta[axis];
      }
      else
      {
         m_lastOp.axis = axisIndex(0x7 & ~op.axisMask); // The excluded axis: the two others scale
         m_lastOp.plane = true;
         for (int i = 0; i < 3; ++i)
            if (op.axisMask & (1u << i))
               m_lastOp.factor = op.delta[i];
      }
      m_lastOp.localSpace = op.space == ImGuizmo::LOCAL;
      break;
   }
   default: break;
   }
}

// Evaluates the operation again from the state it started from: the undo record holds the state before the operation, so it stays valid.
// Rotate and scale use the same formulas as the modal transform op (ViewportPane::UpdateTransformOp), about the same pivot
void EditorUI::ApplyLastOperation()
{
   Matrix3D result(m_lastOp.initial);
   const Vertex3Ds &pivot = m_lastOp.pivot;
   switch (m_lastOp.type)
   {
   case LastOperation::Type::Add:
   case LastOperation::Type::Paste:
   case LastOperation::Type::Duplicate:
      result._41 = m_lastOp.base.x + m_lastOp.offset.x;
      result._42 = m_lastOp.base.y + m_lastOp.offset.y;
      result._43 = m_lastOp.base.z + m_lastOp.offset.z;
      break;
   case LastOperation::Type::Move:
      result._41 += m_lastOp.offset.x;
      result._42 += m_lastOp.offset.y;
      result._43 += m_lastOp.offset.z;
      break;
   case LastOperation::Type::Rotate:
   {
      const Vertex3Ds axisDir = m_lastOp.axis < 0 ? m_lastOp.axisView : Vertex3Ds(m_lastOp.axis == 0 ? 1.f : 0.f, m_lastOp.axis == 1 ? 1.f : 0.f, m_lastOp.axis == 2 ? 1.f : 0.f);
      // Row-vector convention: the left factor applies first, and the rotation spins the opposite way, hence the negated angle
      result = m_lastOp.initial * Matrix3D::MatrixTranslate(-pivot) * Matrix3D::MatrixRotate(-ANGTORAD(m_lastOp.angle), axisDir) * Matrix3D::MatrixTranslate(pivot);
      break;
   }
   case LastOperation::Type::Scale:
   {
      float factor[3] = { 1.f, 1.f, 1.f };
      for (int i = 0; i < 3; ++i)
         if (m_lastOp.axis < 0 || (m_lastOp.plane ? i != m_lastOp.axis : i == m_lastOp.axis))
            factor[i] = m_lastOp.factor;
      Matrix3D basis = Matrix3D::MatrixIdentity();
      if (m_lastOp.localSpace)
         for (int i = 0; i < 3; ++i)
         {
            Vertex3Ds dir = m_lastOp.initial.MultiplyVectorNoTranslate(Vertex3Ds(i == 0 ? 1.f : 0.f, i == 1 ? 1.f : 0.f, i == 2 ? 1.f : 0.f));
            if (dir.LengthSquared() < 1e-12f) // Like ViewportPane::OpAxisDir, a degenerate local axis falls back to the world axis
               dir = Vertex3Ds(i == 0 ? 1.f : 0.f, i == 1 ? 1.f : 0.f, i == 2 ? 1.f : 0.f);
            dir.Normalize();
            basis.m[i][0] = dir.x;
            basis.m[i][1] = dir.y;
            basis.m[i][2] = dir.z;
            basis.m[i][3] = 0.f;
         }
      Matrix3D invBasis(basis);
      invBasis.Invert();
      const Matrix3D scale = invBasis * Matrix3D::MatrixScale(factor[0], factor[1], factor[2]) * basis;
      result = m_lastOp.initial * Matrix3D::MatrixTranslate(-pivot) * scale * Matrix3D::MatrixTranslate(pivot);
      break;
   }
   case LastOperation::Type::None: return;
   }
   SetSelectionTransform(result);
}

bool EditorUI::RenderAdjustParameters()
{
   bool changed = false;
   const float width = 220.f * m_liveUI.GetDPI();
   // Esc while one of the fields is being edited cancels the edit (a typed field clears its own active id on Esc, so it is tested before the widgets),
   // it must not reach the editor (deselect)
   if (ImGui::IsKeyPressed(ImGuiKey_Escape, false) && ImGui::IsAnyItemActive())
      m_suppressNextEscape = true;
   // Same colors as the editor fields
   ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::V(EditorTheme::Inactive));
   ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, EditorTheme::V(EditorTheme::Hover));
   ImGui::PushStyleColor(ImGuiCol_FrameBgActive, EditorTheme::V(EditorTheme::Raised));
   ImGui::PushStyleColor(ImGuiCol_SliderGrab, EditorTheme::V(EditorTheme::Fill));
   ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, EditorTheme::V(EditorTheme::FillActive));
   ImGui::PushItemWidth(width);
   const auto row = [dpi = m_liveUI.GetDPI()](const char *label)
   {
      ImGui::AlignTextToFramePadding();
      ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(EditorTheme::TextDim));
      ImGui::TextUnformatted(label);
      ImGui::PopStyleColor();
      ImGui::SameLine(84.f * dpi);
   };
   // X / Y / Z drag fields with a letter cue; values that display as zero are shown as 0.00, never "-0.00" (the stored value is only written back when edited)
   const auto dragVec = [width](const char *id, float *v, const int n)
   {
      bool edited = false;
      ImGui::BeginGroup(); // One item for the tooltip
      static constexpr const char *formats[] = { "X  %.2f", "Y  %.2f", "Z  %.2f" };
      const float w = (width - ImGui::GetStyle().ItemSpacing.x * static_cast<float>(n - 1)) / static_cast<float>(n);
      for (int i = 0; i < n; i++)
      {
         float shown = fabsf(v[i]) < 0.005f ? 0.f : v[i];
         if (i > 0)
            ImGui::SameLine();
         ImGui::SetNextItemWidth(w);
         if (ImGui::DragFloat((string(id) + std::to_string(i)).c_str(), &shown, 1.f, 0.f, 0.f, formats[i]))
         {
            v[i] = shown;
            edited = true;
         }
      }
      ImGui::EndGroup();
      return edited;
   };
   switch (m_lastOp.type)
   {
   case LastOperation::Type::Move:
      row("Delta");
      changed |= dragVec("##Delta", &m_lastOp.offset.x, 3);
      ImGui::SetItemTooltip("X, Y, Z in VP units");
      break;
   case LastOperation::Type::Paste:
   case LastOperation::Type::Duplicate:
      row("Offset");
      changed |= dragVec("##Offset", &m_lastOp.offset.x, 3);
      ImGui::SetItemTooltip("X, Y, Z in VP units, from the %s", m_lastOp.type == LastOperation::Type::Duplicate ? "duplicated parts" : "copied parts");
      break;
   case LastOperation::Type::Add:
      row("Position");
      changed |= dragVec("##Position", &m_lastOp.offset.x, 2);
      ImGui::SetItemTooltip("X, Y in VP units");
      break;
   case LastOperation::Type::Rotate:
   {
      row("Angle");
      changed |= ImGui::DragFloat("##Angle", &m_lastOp.angle, 0.5f, 0.f, 0.f, "%.1f deg");
      row("Axis");
      static constexpr const char *axes[] = { "View", "X", "Y", "Z" };
      int axis = m_lastOp.axis + 1;
      if (ImGui::Combo("##Axis", &axis, axes, 4))
      {
         m_lastOp.axis = axis - 1;
         changed = true;
      }
      break;
   }
   case LastOperation::Type::Scale:
   {
      row("Factor");
      changed |= ImGui::DragFloat("##Factor", &m_lastOp.factor, 0.01f, 0.001f, 1000.f, "%.3f");
      row("Axis");
      static constexpr const char *axes[] = { "All", "X", "Y", "Z" };
      int axis = m_lastOp.axis + 1;
      if (ImGui::Combo("##Axis", &axis, axes, 4))
      {
         m_lastOp.axis = axis - 1;
         changed = true;
      }
      if (m_lastOp.axis >= 0)
      {
         row("");
         changed |= ImGui::Checkbox("Perpendicular plane", &m_lastOp.plane);
      }
      break;
   }
   case LastOperation::Type::None: break;
   }
   ImGui::PopItemWidth();
   ImGui::PopStyleColor(5);
   return changed;
}

void EditorUI::RenderAdjustPanel()
{
   // The operation can be adjusted as long as it is the last undo step and the selection is the one it was done on
   const ViewportPane *const targetVp = GetTargetViewport();
   if (m_lastOp.type != LastOperation::Type::None
      && ((targetVp != nullptr && targetVp->m_op.op != ImGuizmo::OPERATION(0)) || m_pointEditPart != nullptr || m_table->IsLocked() || m_undo.TopRecordId() != m_lastOp.undoId
         || m_multiSel != m_lastOp.selection))
      m_lastOp.type = LastOperation::Type::None;
   if (m_lastOp.type == LastOperation::Type::None)
   {
      m_adjustPopupOpen = false;
      return;
   }
   bool changed = false;
   // The floating panel belongs to the target viewport's area: it is hidden with it (e.g. behind a
   // maximized area); the F9 popup below does not (and replaces the panel while open)
   if (m_viewportVisible && targetVp != nullptr && FindLeafRect(targetVp->m_leafId) != nullptr && !m_adjustPopupOpen)
   {
      const float dpi = m_liveUI.GetDPI();
      const char *const name = LastOperationName(m_lastOp.type);

      // Collapsible panel at the bottom left of the viewport area (above the status bar)
      ImGui::SetNextWindowPos(ImVec2(m_viewportRect.x0 + 8.f * dpi, m_viewportRect.y1 - 8.f * dpi), ImGuiCond_Always, ImVec2(0.f, 1.f));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f * dpi, 6.f * dpi));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
      ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::V(EditorTheme::Bar));
      ImGui::PushStyleColor(ImGuiCol_Border, EditorTheme::V(EditorTheme::Border));
      if (ImGui::Begin("Adjust Last Operation", nullptr,
             ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
                | ImGuiWindowFlags_NoMove))
      {
         // Header: a triangle (right when collapsed, down when expanded) and the name of the operation
         const ImVec2 headerPos = ImGui::GetCursorScreenPos();
         const ImVec2 textSize = ImGui::CalcTextSize(name);
         const float headerWidth = max(220.f * dpi, textSize.x + 28.f * dpi);
         if (ImGui::InvisibleButton("##AdjustHeader", ImVec2(headerWidth, ImGui::GetFrameHeight())))
            m_lastOp.expanded = !m_lastOp.expanded;
         const bool hovered = ImGui::IsItemHovered();
         ImDrawList *const draw = ImGui::GetWindowDrawList();
         const float cy = headerPos.y + 0.5f * ImGui::GetFrameHeight();
         const float h = ImGui::GetFontSize() * 0.3f;
         if (m_lastOp.expanded)
            draw->AddTriangleFilled(ImVec2(headerPos.x + 2.f * dpi, cy - h * 0.6f), ImVec2(headerPos.x + 2.f * dpi + 2.f * h, cy - h * 0.6f),
               ImVec2(headerPos.x + 2.f * dpi + h, cy + h * 0.6f), EditorTheme::TextDim);
         else
            draw->AddTriangleFilled(ImVec2(headerPos.x + 2.f * dpi, cy - h), ImVec2(headerPos.x + 2.f * dpi, cy + h), ImVec2(headerPos.x + 2.f * dpi + h * 1.2f, cy), EditorTheme::TextDim);
         draw->AddText(ImVec2(headerPos.x + 20.f * dpi, cy - 0.5f * textSize.y), hovered ? EditorTheme::Text : EditorTheme::TextDim, name);
         ImGui::SetItemTooltip("Adjust Last Operation (F9)");
         if (m_lastOp.expanded)
         {
            ImGui::Separator();
            changed = RenderAdjustParameters();
         }
      }
      ImGui::End();
      ImGui::PopStyleColor(2);
      ImGui::PopStyleVar(2);
   }

   // F9 popup at the mouse
   if (m_adjustPopupOpen)
   {
      ImGui::SetNextWindowPos(m_adjustPopupPos, ImGuiCond_Appearing);
      bool open = true;
      ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::V(EditorTheme::Bar));
      if (ImGui::Begin("Adjust Last Operation##Popup", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
      {
         changed |= RenderAdjustParameters();
         if (!m_adjustPopupJustOpened && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
            && !ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
            open = false;
      }
      ImGui::End();
      ImGui::PopStyleColor();
      m_adjustPopupJustOpened = false;
      if (!open)
         m_adjustPopupOpen = false;
   }

   if (changed)
      ApplyLastOperation();
}

Vertex2D EditorUI::GetMultiSelCenter() const
{
   // Bounding box center of the part centers (win32 GetMultiSelCenter); in drag point edit mode, the
   // bounding box center of the selected points (or the edit center point alone in center edit mode)
   float minx = FLT_MAX, maxx = -FLT_MAX, miny = FLT_MAX, maxy = -FLT_MAX;
   if (m_pointEditPart)
   {
      if (m_pointEditCenter)
      {
         const Vertex3Ds center = m_pointEditPart->GetEditCenter();
         return Vertex2D(center.x, center.y);
      }
      for (const DragPoint *point : m_pointSel)
      {
         minx = min(minx, point->GetX());
         maxx = max(maxx, point->GetX());
         miny = min(miny, point->GetY());
         maxy = max(maxy, point->GetY());
      }
   }
   else
   {
      for (const auto &part : m_multiSel)
      {
         const Vertex2D center = part->GetEditable()->GetCenter();
         minx = min(minx, center.x);
         maxx = max(maxx, center.x);
         miny = min(miny, center.y);
         maxy = max(maxy, center.y);
      }
   }
   return (minx > maxx) ? Vertex2D(0.f, 0.f) : Vertex2D((minx + maxx) * 0.5f, (miny + maxy) * 0.5f);
}

void EditorUI::FlipSelection(const bool flipX)
{
   const Vertex2D center = GetMultiSelCenter();
   if (m_pointEditPart)
   {
      BeginPointEdit();
      if (m_pointEditCenter)
      {
         const Vertex3Ds c = m_pointEditPart->GetEditCenter();
         m_pointEditPart->SetEditCenter(flipX ? Vertex2D(2.f * center.x - c.x, c.y) : Vertex2D(c.x, 2.f * center.y - c.y));
      }
      else
         for (DragPoint *point : m_pointSel)
         {
            if (flipX)
               point->SetX(2.f * center.x - point->GetX());
            else
               point->SetY(2.f * center.y - point->GetY());
         }
      EndPointEdit();
      return;
   }
   m_undo.BeginUndo();
   for (const auto &part : m_multiSel)
   {
      IEditable *const editable = part->GetEditable();
      m_undo.MarkForUndo(editable);
      if (flipX)
         editable->FlipX(center);
      else
         editable->FlipY(center);
      m_renderer->ReinitRenderable(editable->GetIRenderable());
      m_player->m_physics->Update(editable);
   }
   m_undo.EndUndo();
}

void EditorUI::RotateMultiSel(const float ang, const Vertex2D &center, const bool useElementCenter)
{
   if (m_pointEditPart)
   {
      const float sn = sinf(ANGTORAD(ang)), cs = cosf(ANGTORAD(ang));
      BeginPointEdit();
      const auto rotatePoint = [&center, sn, cs](Vertex2D &v)
      {
         const float dx = v.x - center.x, dy = v.y - center.y;
         v.x = center.x + cs * dx - sn * dy;
         v.y = center.y + cs * dy + sn * dx;
      };
      if (m_pointEditCenter)
      {
         Vertex2D pos(m_pointEditPart->GetEditCenter().x, m_pointEditPart->GetEditCenter().y);
         rotatePoint(pos);
         m_pointEditPart->SetEditCenter(pos);
      }
      else
         for (DragPoint *point : m_pointSel)
         {
            Vertex2D pos(point->GetX(), point->GetY());
            rotatePoint(pos);
            point->SetX(pos.x);
            point->SetY(pos.y);
         }
      EndPointEdit();
      return;
   }
   m_undo.BeginUndo();
   for (const auto &part : m_multiSel)
   {
      IEditable *const editable = part->GetEditable();
      m_undo.MarkForUndo(editable);
      editable->Rotate(ang, center, useElementCenter);
      m_renderer->ReinitRenderable(editable->GetIRenderable());
      m_player->m_physics->Update(editable);
   }
   m_undo.EndUndo();
}

void EditorUI::ScaleMultiSel(const float scalex, const float scaley, const Vertex2D &center, const bool useElementCenter)
{
   if (m_pointEditPart)
   {
      BeginPointEdit();
      if (m_pointEditCenter)
      {
         const Vertex3Ds c = m_pointEditPart->GetEditCenter();
         m_pointEditPart->SetEditCenter(Vertex2D(center.x + (c.x - center.x) * scalex, center.y + (c.y - center.y) * scaley));
      }
      else
         for (DragPoint *point : m_pointSel)
         {
            point->SetX(center.x + (point->GetX() - center.x) * scalex);
            point->SetY(center.y + (point->GetY() - center.y) * scaley);
         }
      EndPointEdit();
      return;
   }
   m_undo.BeginUndo();
   for (const auto &part : m_multiSel)
   {
      IEditable *const editable = part->GetEditable();
      m_undo.MarkForUndo(editable);
      editable->Scale(scalex, scaley, center, useElementCenter);
      m_renderer->ReinitRenderable(editable->GetIRenderable());
      m_player->m_physics->Update(editable);
   }
   m_undo.EndUndo();
}

void EditorUI::TranslateMultiSel(const Vertex2D &offset)
{
   if (m_pointEditPart)
   {
      BeginPointEdit();
      if (m_pointEditCenter)
      {
         const Vertex3Ds c = m_pointEditPart->GetEditCenter();
         m_pointEditPart->SetEditCenter(Vertex2D(c.x + offset.x, c.y + offset.y));
      }
      else
         for (DragPoint *point : m_pointSel)
            point->Translate(offset);
      EndPointEdit();
      return;
   }
   m_undo.BeginUndo();
   for (const auto &part : m_multiSel)
   {
      IEditable *const editable = part->GetEditable();
      if (editable->IsUILocked())
         continue; // UI locked parts are not moved by the nudge commands (win32 arrow keys)
      m_undo.MarkForUndo(editable);
      editable->Translate(offset);
      m_renderer->ReinitRenderable(editable->GetIRenderable());
      m_player->m_physics->Update(editable);
   }
   m_undo.EndUndo();
}

void EditorUI::LockElements()
{
   // Same semantics as the win32 editor (PinTableWnd::LockElements): if any selected part is locked, unlock
   // them all, otherwise lock them all
   bool anyLocked = false;
   for (const auto &part : m_multiSel)
      anyLocked |= part->GetEditable()->IsUILocked();
   m_undo.BeginUndo();
   for (const auto &part : m_multiSel)
   {
      IEditable *const editable = part->GetEditable();
      m_undo.MarkForUndo(editable);
      editable->SetUILock(!anyLocked);
   }
   m_undo.EndUndo();
}

void EditorUI::MoveSelectionInDrawOrder(const bool toFront)
{
   // Draw In Front / Draw In Back of the win32 context menu (non undoable, like the win32 command)
   for (const auto &part : m_multiSel)
   {
      IEditable *const editable = part->GetEditable();
      if (editable->GetItemType() == eItemPartGroup)
         continue;
      if (toFront)
         m_table->MovePartToFront(editable);
      else
         m_table->MovePartToBack(editable);
   }
}

void EditorUI::ToggleCollectionMembership(const int collectionIndex)
{
   vector<IEditable *> selection;
   selection.reserve(m_multiSel.size());
   for (const auto &part : m_multiSel)
      selection.push_back(part->GetEditable());
   m_table->ToggleCollectionMembership(collectionIndex, selection);
}

void EditorUI::RequestAddPartPopup(const ImVec2 &pos)
{
   // The picker popup must be opened in the window scope of a viewport pane (OpenPopup must be called
   // in the same window scope as the matching BeginPopup): defer it to the pane under the position,
   int leaf = AreaLayout::LeafAt(m_leafRects, pos.x, pos.y);
   const AreaNode *const node = leaf >= 0 ? m_layout.Find(leaf) : nullptr;
   if (node == nullptr || node->isSplit || (!IsViewportType(node->type) && node->type != AreaType::Blueprint))
      leaf = -1;
   m_addPartPopupLeaf = leaf;
   m_addPartPopupPos = pos;
}

void EditorUI::EnsureAddPartButtons()
{
   // Add part buttons (same parts and icons as the WinUI toolbar), shared by the viewport pane pickers
   if (!m_addPartButtons.empty())
      return;
   static const struct
   {
      ItemTypeEnum type;
      const char *name;
      const char *file;
   } addParts[] = {
      { eItemSurface, "Surface", "editor/wall.png" }, //
      { eItemGate, "Gate", "editor/gate.png" }, //
      { eItemRamp, "Ramp", "editor/ramp.png" }, //
      { eItemFlipper, "Flipper", "editor/flipper.png" }, //
      { eItemPlunger, "Plunger", "editor/plunger.png" }, //
      { eItemBall, "Ball", "editor/ball.png" }, //
      { eItemBumper, "Bumper", "editor/bumper.png" }, //
      { eItemSpinner, "Spinner", "editor/spinner.png" }, //
      { eItemTimer, "Timer", "editor/timer.png" }, //
      { eItemTrigger, "Trigger", "editor/trigger.png" }, //
      { eItemLight, "Light", "editor/light.png" }, //
      { eItemKicker, "Kicker", "editor/kicker.png" }, //
      { eItemHitTarget, "Target", "editor/target.png" }, //
      { eItemDecal, "Decal", "editor/decal.png" }, //
      { eItemTextbox, "Textbox", "editor/textbox.png" }, //
      { eItemDispReel, "Display Reel", "editor/reel.png" }, //
      { eItemLightSeq, "Light Seq", "editor/lightseq.png" }, //
      { eItemPrimitive, "Primitive", "editor/primitive.png" }, //
      { eItemFlasher, "Flasher", "editor/flasher.png" }, //
      { eItemRubber, "Rubber", "editor/rubber.png" }, //
   };
   for (const auto &def : addParts)
   {
      std::shared_ptr<const BaseTexture> tex = BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, def.file));
      m_addPartButtons.push_back({ def.type, def.name, tex ? std::make_shared<Sampler>(m_renderer->m_renderDevice, def.name, tex, false) : nullptr });
   }
}

void EditorUI::CreatePart(const ItemTypeEnum type, const Vertex2D &pos)
{
   IEditable *const pie = EditableRegistry::CreateAndInit(type, m_table, pos.x, pos.y);
   if (pie == nullptr)
      return;

   // Same initialization sequence as the WinUI editor
   if (auto *const scriptable = pie->GetIScriptable(); scriptable)
      m_table->GetUniqueName(type, scriptable->m_name);
   pie->m_desktopBackdrop = IsBackdropEditMode();
   m_table->AddPart(pie);
   pie->SetPartGroup(GetPartGroupForNewPart());

   // Same player side setup as player initialization
   if (type == eItemBall)
      m_player->m_vball.push_back(static_cast<Ball *>(pie));
   m_player->TimerSetup(pie);
   if (auto *const renderable = pie->GetIRenderable(); renderable)
   {
      renderable->RenderSetup(m_renderer.get());
      m_renderer->MarkStaticsDirty();
   }
   if (pie->GetIHitable())
      m_player->m_physics->Add(pie);

   m_undo.BeginUndo();
   m_undo.MarkForCreate(pie);
   m_undo.EndUndo();

   // Create the UI part now (it would otherwise be lazily added on next frame) and select the new part
   UpdateEditableList();
   if (const auto it = m_editableMap.find(pie); it != m_editableMap.end())
   {
      SetSelection(Selection(it->second));
      OutlinerPanel::m_anchor = it->second;
   }
   Matrix3D placed;
   if (GetSelectionTransform(placed))
   {
      SetLastOperation(LastOperation::Type::Add, placed);
      m_lastOp.base = Vertex3Ds(0.f, 0.f, placed._43);
      m_lastOp.offset = Vertex3Ds(placed._41, placed._42, 0.f);
   }
}

PartGroup *EditorUI::CreatePartGroup()
{
   // Same initialization sequence as CreatePart; a new group is created at the root level (the
   // user can drag it into another group in the outliner to nest it)
   IEditable *const pie = EditableRegistry::CreateAndInit(eItemPartGroup, m_table, 0.f, 0.f);
   if (pie == nullptr)
      return nullptr;
   if (auto *const scriptable = pie->GetIScriptable(); scriptable)
      m_table->GetUniqueName(eItemPartGroup, scriptable->m_name);
   pie->m_desktopBackdrop = IsBackdropEditMode();
   m_table->AddPart(pie);
   pie->SetPartGroup(nullptr);

   m_undo.BeginUndo();
   m_undo.MarkForCreate(pie);
   m_undo.EndUndo();

   // Create the UI part now (it would otherwise be lazily added on next frame) and select the new group
   UpdateEditableList();
   if (const auto it = m_editableMap.find(pie); it != m_editableMap.end())
   {
      SetSelection(Selection(it->second));
      OutlinerPanel::m_anchor = it->second;
   }
   return static_cast<PartGroup *>(pie);
}

void EditorUI::CreateCollection(bool fromSelection)
{
   // Same initialization sequence as the WinUI editor (PinTableWnd::NewCollection)
   CComObject<Collection> *pcol;
   CComObject<Collection>::CreateInstance(&pcol);
   pcol->AddRef();
   pcol->m_name = m_table->GetUniqueName("Collection"s);
   if (fromSelection)
      for (const auto &uiPart : m_multiSel)
      {
         IEditable *const editable = uiPart->GetEditable();
         if (editable && editable->GetIScriptable()) // Collections only reference scriptable parts (persisted by name)
            m_table->AddPartToCollection(pcol, editable);
      }
   m_table->AddCollection(pcol);
   pcol->Release();
   m_table->SetNonUndoableDirty(SaveDirtyState::Dirty);
   SetSelection(Selection(pcol));
}

PartGroup *EditorUI::GetPartGroupForNewPart()
{
   // Assign new parts to the group of the current selection. When the selection does not define a
   // group, favor the most recently used ones (still respecting UI visibility), then fall back to
   // the visibility mask (first UI visible root group, else first root group). The target is only
   // empty when the table does not have any part group at all.
   PartGroup *partGroup = nullptr;
   if (const std::shared_ptr<EditorUIPart> &part = m_selection.GetPart())
      partGroup = part->GetEditable()->GetItemType() == eItemPartGroup ? static_cast<PartGroup *>(part->GetEditable()) : part->GetEditable()->GetPartGroup();
   if (partGroup == nullptr)
   {
      const vector<IEditable *> &parts = m_table->GetParts();
      std::erase_if(m_partGroupUseHistory, [&parts](const PartGroup *group) { return std::ranges::find(parts, group) == parts.end(); });
      for (PartGroup *const group : m_partGroupUseHistory)
         if (group->IsUIVisible(true))
         {
            partGroup = group;
            break;
         }
   }
   if (partGroup == nullptr)
   {
      for (IEditable *const part : m_table->GetParts())
      {
         if (part->GetItemType() == eItemPartGroup && part->GetPartGroup() == nullptr)
         {
            if (part->IsUIVisible(false))
            {
               partGroup = static_cast<PartGroup *>(part);
               break;
            }
            if (partGroup == nullptr)
               partGroup = static_cast<PartGroup *>(part);
         }
      }
   }
   if (partGroup != nullptr)
   {
      std::erase(m_partGroupUseHistory, partGroup);
      m_partGroupUseHistory.insert(m_partGroupUseHistory.begin(), partGroup);
   }
   return partGroup;
}

void EditorUI::CopySelection()
{
   if (m_pointEditPart)
   {
      // Copy the coordinates of the selected drag point or center point
      if (m_pointEditCenter)
      {
         if (m_centerSelected)
            VPX::EditorClipboard::CopyPoint(m_pointEditPart->GetEditCenter());
      }
      else if (m_pointSel.size() == 1)
         VPX::EditorClipboard::CopyPoint(m_pointSel.front()->GetVertex());
      return;
   }
   vector<IEditable *> parts;
   for (const auto &part : m_multiSel)
      if (IEditable *const editable = part->GetEditable(); editable != nullptr)
         parts.push_back(editable);
   RestorePartsVisibility(); // Copy the authored visibility, not the editor display state
   VPX::EditorClipboard::CopyParts(parts);
}

void EditorUI::PasteSelection(const ImVec2 *const pos)
{
   if (m_table->IsLocked() || IsInspectMode())
      return;
   if (m_pointEditPart)
   {
      // Paste the copied coordinates to the selected drag point or center point
      Vertex3Ds pointPos;
      if (m_pointEditCenter)
      {
         if (m_centerSelected && VPX::EditorClipboard::GetPoint(pointPos))
         {
            BeginPointEdit();
            m_pointEditPart->SetEditCenter(Vertex2D(pointPos.x, pointPos.y));
            EndPointEdit();
         }
      }
      else if (m_pointSel.size() == 1 && VPX::EditorClipboard::GetPoint(pointPos))
      {
         BeginPointEdit();
         m_pointSel.front()->SetX(pointPos.x);
         m_pointSel.front()->SetY(pointPos.y);
         m_pointSel.front()->SetZ(pointPos.z);
         EndPointEdit();
      }
      return;
   }
   const vector<vector<uint8_t>> parts = VPX::EditorClipboard::GetParts();
   if (parts.empty())
      return;
   const bool backdrop = IsBackdropEditMode();
   PartGroup *const partGroup = GetPartGroupForNewPart();
   vector<IEditable *> pasted;
   Vertex2D pasteMin(FLT_MAX, FLT_MAX), pasteMax(-FLT_MAX, -FLT_MAX);
   m_undo.BeginUndo();
   for (const vector<uint8_t> &partData : parts)
   {
      if (partData.size() <= sizeof(int))
         continue;
      const ItemTypeEnum type = *reinterpret_cast<const ItemTypeEnum *>(partData.data());
      IEditable *const editable = EditableRegistry::Create(type);
      if (editable == nullptr)
         continue;
      BiffReader reader(partData.data() + sizeof(int), static_cast<uint32_t>(partData.size() - sizeof(int)), CURRENT_FILE_FORMAT_VERSION, nullptr, 0);
      editable->Load(reader);
      editable->m_desktopBackdrop = backdrop;
      // If the original name is not yet used, use that one, otherwise add/increase the suffix until we find a name that's not used yet
      if (!m_table->IsNameUnique(editable->GetName()))
      {
         // First remove the existing suffix
         const string input = editable->GetName();
         size_t lastNonDigit = input.length();
         while (lastNonDigit > 0 && input[lastNonDigit - 1] >= '0' && input[lastNonDigit - 1] <= '9')
            --lastNonDigit;
         editable->SetName(m_table->GetUniqueName(input.substr(0, lastNonDigit)));
      }
      editable->SetPartGroup(partGroup);
      m_table->AddPart(editable);
      m_undo.MarkForCreate(editable);
      pasted.push_back(editable);
      const Vertex2D center = editable->GetCenter();
      pasteMin.x = min(pasteMin.x, center.x);
      pasteMin.y = min(pasteMin.y, center.y);
      pasteMax.x = max(pasteMax.x, center.x);
      pasteMax.y = max(pasteMax.y, center.y);
   }
   m_undo.EndUndo();
   if (pasted.empty())
   {
      m_undo.Discard();
      return;
   }
   // Move the pasted parts so that their combined center lands at the given position (in the table XY
   // plane, keeping their Z). No position = 'Paste' at the original coordinates (win32 Ctrl+V vs Paste At)
   Vertex2D offset(0.f, 0.f);
   if (pos != nullptr)
   {
      const Vertex2D target = TablePosAt(*pos);
      offset = Vertex2D(target.x - 0.5f * (pasteMin.x + pasteMax.x), target.y - 0.5f * (pasteMin.y + pasteMax.y));
   }
   for (IEditable *const editable : pasted)
   {
      editable->Translate(offset);
      // Player side setup, same as part creation
      if (editable->GetItemType() == eItemBall)
         m_player->m_vball.push_back(static_cast<Ball *>(editable));
      m_player->TimerSetup(editable);
      if (auto *const renderable = editable->GetIRenderable(); renderable)
      {
         renderable->RenderSetup(m_renderer.get());
         m_renderer->MarkStaticsDirty();
      }
      if (editable->GetIHitable())
         m_player->m_physics->Add(editable);
   }
   // Select the pasted parts
   UpdateEditableList();
   m_multiSel.clear();
   for (IEditable *const editable : pasted)
      if (const auto it = m_editableMap.find(editable); it != m_editableMap.end())
         m_multiSel.push_back(it->second);
   m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   OutlinerPanel::m_anchor = m_multiSel.empty() ? nullptr : m_multiSel.back();
   Matrix3D placed;
   if (!m_multiSel.empty() && GetSelectionTransform(placed))
   {
      SetLastOperation(LastOperation::Type::Paste, placed);
      m_lastOp.base = Vertex3Ds(placed._41 - offset.x, placed._42 - offset.y, placed._43);
      m_lastOp.offset = Vertex3Ds(offset.x, offset.y, 0.f);
   }
}

bool EditorUI::CanDuplicate() const { return m_pointEditPart == nullptr && !IsInspectMode() && !m_table->IsLocked() && !m_multiSel.empty(); }

void EditorUI::DuplicateSelection()
{
   // Duplicate the selected parts (through the editor clipboard) and grab them (Blender's Shift+D)
   if (!CanDuplicate())
      return;
   const size_t count = m_multiSel.size();
   Matrix3D origin;
   const bool hasOrigin = GetSelectionTransform(origin);
   CopySelection();
   const uint64_t topBefore = m_undo.TopRecordId();
   PasteSelection(nullptr); // In place: the grab moves them
   if (m_undo.TopRecordId() == topBefore) // Nothing was pasted: the grab would fold into an unrelated step
   {
      Report("Nothing to duplicate");
      return;
   }
   const string name = m_multiSel.empty() ? string() : m_multiSel.front()->GetEditable()->GetName();
   m_undo.SetLabel(CountLabel("Duplicate", m_multiSel.size(), name)); // Not "Paste" (kept by the merge with the grab)
   Report(CountLabel("Duplicated", count, name));
   if (ViewportPane *const vp = GetTargetViewport())
   {
      if (m_navBlenderTransform)
      {
         vp->BeginTransformOp(ImGuizmo::TRANSLATE);
         if (vp->m_op.op != ImGuizmo::OPERATION(0) && hasOrigin)
         {
            vp->m_op.duplicate = true;
            vp->m_op.duplicateOrigin = Vertex3Ds(origin._41, origin._42, origin._43);
         }
      }
      else
         vp->m_gizmoOperation = ImGuizmo::TRANSLATE;
   }
}

void EditorUI::DeleteSelection()
{
   if (m_table->IsLocked() || IsInspectMode())
      return;
   if (m_pointEditPart) // Toolbar delete while editing drag points: delete the points, the edited part must stay alive
   {
      DeleteSelectedPoints();
      return;
   }
   // Balls are runtime objects, parts without a part group are live objects or root groups: they can not be deleted (and stay selected)
   vector<IEditable *> partsToDelete;
   for (const auto &part : m_multiSel)
      if (IEditable *const edit = part->GetEditable(); edit->GetItemType() != eItemBall && edit->GetPartGroup() != nullptr && std::ranges::find(partsToDelete, edit) == partsToDelete.end())
         partsToDelete.push_back(edit);
   // Delete group content, nested groups included (partsToDelete grows while iterating)
   for (size_t i = 0; i < partsToDelete.size(); i++)
      if (partsToDelete[i]->GetItemType() == eItemPartGroup)
         for (IEditable *const edit : m_table->GetParts())
            if (edit->GetPartGroup() == partsToDelete[i] && edit->GetItemType() != eItemBall && std::ranges::find(partsToDelete, edit) == partsToDelete.end())
               partsToDelete.push_back(edit);
   if (partsToDelete.empty())
      return;

   // Begun before the selection changes, so that undoing restores the deleted parts as selected
   m_undo.BeginUndo();
   for (IEditable *const edit : partsToDelete)
   {
      if (const auto it = m_editableMap.find(edit); it != m_editableMap.end())
      {
         const std::shared_ptr<EditorUIPart> uiPart = it->second;
         uiPart->RestorePartVisibility(); // While the part is alive, so that it is authored when undeleted
         m_editableMap.erase(it);
         RemoveFromVectorSingle(m_editables, uiPart);
      }
      // Release the player side resources (restored by PinUndo::Undo), the render ones at the end of the frame as the render thread may still use them.
      // The release runs on the render thread: an undelete that happens before it must not touch the render state (see PinUndo::Undo)
      if (edit->GetIHitable())
         m_player->m_physics->Remove(edit);
      if (edit->m_phittimer)
         m_player->TimerRelease(edit);
      m_renderer->DropRenderable(edit->GetIRenderable());
      edit->AddRef(); // Keep the part alive until its deferred render release has been processed
      edit->m_renderReleasePending = true;
      m_renderer->m_renderDevice->AddEndOfFrameCmd([edit]()
         {
            if (edit->GetIRenderable())
               edit->GetIRenderable()->RenderRelease();
            edit->m_renderReleasePending = false;
            edit->Release();
         });
      m_undo.MarkForDelete(edit); // The undo record keeps the part alive (until undone or dropped)
      for (Collection *const collection : edit->m_vCollection) // m_vCollection is kept for Undelete
         collection->RemovePart(edit);
      m_table->RemovePart(edit);
   }
   m_undo.EndUndo();
   m_lastUndoPart = nullptr; // Start a new record on next edit (a deleted part's address may be reused)
   m_lastUndoId = 0;

   // Remove the deleted parts from the selection (non deletable parts stay selected)
   std::erase_if(m_multiSel, [this](const auto &part) { return std::ranges::find(m_editables, part) == m_editables.end(); });
   if (m_selection.GetType() == Selection::S_EDITABLE && !IsPartSelected(m_selection.GetPart()))
      m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   if (OutlinerPanel::m_anchor && std::ranges::find(m_editables, OutlinerPanel::m_anchor) == m_editables.end())
      OutlinerPanel::m_anchor.reset();
}

void EditorUI::RestorePartsVisibility() const
{
   // The editor keeps overriding the parts' m_d visibility fields with the outliner state (see EditableUIPart::Render).
   // Only parts still in the table are known to be alive: UI parts of removed parts are only dropped by UpdateEditableList
   const ankerl::unordered_dense::set<const IEditable *> liveParts(m_table->GetParts().begin(), m_table->GetParts().end());
   for (const auto &uiPart : m_editables)
      if (liveParts.contains(uiPart->GetEditable()))
         uiPart->RestorePartVisibility();
}

void EditorUI::UpdateEditableList()
{
   // In full edit mode, parts without a part group are live objects generated at runtime (implicit
   // playfield mesh, script created parts, ...): they are not part of the table file, so they are
   // not listed in the outliner and can not be selected
   const auto isLiveObject = [this](const IEditable *edit) { return (m_table->m_liveBaseTable == nullptr) && (edit->GetPartGroup() == nullptr) && (edit->GetItemType() != eItemPartGroup); };
   // Remove UI parts of removed editables (or of live objects in full edit mode)
   const ankerl::unordered_dense::set<const IEditable *> liveParts(m_table->GetParts().begin(), m_table->GetParts().end());
   std::erase_if(m_editables,
      [this, &liveParts, &isLiveObject](const auto &uiPart)
      {
         if (!liveParts.contains(uiPart->GetEditable()) || isLiveObject(uiPart->GetEditable()))
         {
            m_editableMap.erase(uiPart->GetEditable());
            return true;
         }
         return false;
      });
   // Drop removed parts from the multi selection, keeping a valid active part
   std::erase_if(m_multiSel, [&liveParts, &isLiveObject](const auto &part) { return !liveParts.contains(part->GetEditable()) || isLiveObject(part->GetEditable()); });
   if (m_selection.GetType() == Selection::S_EDITABLE && !IsPartSelected(m_selection.GetPart()))
      m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   if (OutlinerPanel::m_anchor && (!liveParts.contains(OutlinerPanel::m_anchor->GetEditable()) || isLiveObject(OutlinerPanel::m_anchor->GetEditable())))
      OutlinerPanel::m_anchor.reset();
   // Add UI parts for new editables
   bool needSort = false;
   ankerl::unordered_dense::set<PartGroup *> newGroups;
   for (const auto &edit : m_table->GetParts())
   {
      if (isLiveObject(edit))
         continue;
      const auto it = m_editableMap.find(edit);
      if (it == m_editableMap.end()) // New part
      {
         std::shared_ptr<EditorUIPart> uiPart = EditorUIPartRegistry::Create(edit);
         if (uiPart == nullptr) // eItemTable, eItemLightCenter, eItemDragPoint, eItemCollection
            uiPart = std::make_shared<BaseUIPart>(edit);
         if (m_table->m_liveBaseTable)
            edit->SetUIVisible(true);
         else if (edit->GetItemType() == eItemPartGroup)
            newGroups.insert(static_cast<PartGroup *>(edit));
         uiPart->SetOutlinerPath(edit->GetPathString(false));
         m_editables.push_back(uiPart);
         m_editableMap[edit] = std::move(uiPart);
         needSort = true;
      }
      else if (it->second->GetOutlinerPath() != edit->GetPathString(false)) // Name or parent group has changed
      {
         needSort = true;
         it->second->SetOutlinerPath(edit->GetPathString(false));
      }
   }
   // Win32 UI does not manage PartGroup UI hidden/shown state, so we lazily initialize new groups in a
   // single pass: hidden by default in edit mode, shown if they contain at least one visible part
   if (!newGroups.empty())
   {
      for (PartGroup *group : newGroups)
         group->SetUIVisible(false);
      for (const auto &edit : m_table->GetParts())
         if (edit->GetItemType() != eItemPartGroup && edit->IsUIVisible(false))
            for (PartGroup *group = edit->GetPartGroup(); group != nullptr; group = group->GetPartGroup())
               if (newGroups.contains(group))
                  group->SetUIVisible(true);
   }
   // Sort in tree order for the outliner (root live objects first, then each group directly followed by its content, siblings by name).
   // Paths are compared name by name: comparing the joined path strings would put "Lights GI/..." between "Lights" and "Lights/..."
   // (' ' sorts before '/'), and the outliner would then lose the parts of "Lights"
   if (needSort)
   {
      const auto pathOf = [](const IEditable *edit)
      {
         vector<const string *> path;
         for (const IEditable *part = edit; part != nullptr; part = part->GetPartGroup())
            path.push_back(&part->GetName());
         std::ranges::reverse(path);
         return path;
      };
      vector<std::pair<vector<const string *>, std::shared_ptr<EditorUIPart>>> sorted;
      sorted.reserve(m_editables.size());
      for (auto &uiPart : m_editables)
         sorted.emplace_back(pathOf(uiPart->GetEditable()), std::move(uiPart));
      std::ranges::sort(sorted,
         [](const auto &a, const auto &b)
         {
            const bool isRootA = a.second->GetEditable()->GetPartGroup() == nullptr && a.second->GetEditable()->GetItemType() != eItemPartGroup;
            const bool isRootB = b.second->GetEditable()->GetPartGroup() == nullptr && b.second->GetEditable()->GetItemType() != eItemPartGroup;
            if (isRootA != isRootB)
               return isRootA;
            return std::ranges::lexicographical_compare(a.first, b.first, [](const string *x, const string *y) { return StrLessNoCase(*x, *y); });
         });
      for (size_t i = 0; i < sorted.size(); i++)
         m_editables[i] = std::move(sorted[i].second);
   }
}

bool EditorUI::GetSelectionTransform(Matrix3D &transform) const
{
   if (m_pointEditPart)
   {
      // In center edit mode, the gizmo operates on the part's center point
      if (m_pointEditCenter)
      {
         if (!m_centerSelected)
            return false;
         const Vertex3Ds center = m_pointEditPart->GetEditCenter();
         transform = Matrix3D::MatrixTranslate(center.x, center.y, center.z);
         return true;
      }
      // In drag point edit mode, the gizmo operates on the selected points (positioned at their centroid,
      // which stays fixed when rotating or scaling the points around it, unlike the bounding box center)
      if (m_pointSel.empty())
         return false;
      Vertex2D center(0.f, 0.f);
      float z = 0.f;
      for (const DragPoint *point : m_pointSel)
      {
         center += Vertex2D(point->GetX(), point->GetY());
         z += m_pointEditPart->GetDragPointZ(point);
      }
      const float invCount = 1.f / (float)m_pointSel.size();
      transform = Matrix3D::MatrixTranslate(center.x * invCount, center.y * invCount, z * invCount);
      return true;
   }
   if (m_selection.GetType() == Selection::S_EDITABLE)
   {
      const EditorUIPart::TransformMask mask = m_selection.GetPart()->GetTransform(transform);
      return mask != EditorUIPart::TransformMask::TM_None;
   }
   return false;
}

bool EditorUI::GetSelectionBounds(FRect3D &bounds) const
{
   bounds.Clear();
   if (m_pointEditPart)
   {
      // In center edit mode, the bounds are the part's center point
      if (m_pointEditCenter)
      {
         if (m_centerSelected)
         {
            const Vertex3Ds center = m_pointEditPart->GetEditCenter();
            bounds.Extend(FRect3D(center.x, center.x, center.y, center.y, center.z, center.z));
         }
      }
      else
         // In drag point edit mode, the bounds are the ones of the selected points
         for (const DragPoint *point : m_pointSel)
         {
            const float z = m_pointEditPart->GetDragPointZ(point);
            bounds.Extend(FRect3D(point->GetX(), point->GetX(), point->GetY(), point->GetY(), z, z));
         }
   }
   else
   {
      const auto extendWithPart = [this, &bounds](const std::shared_ptr<EditorUIPart> &part)
      {
         IEditable *const editable = part->GetEditable();
         if (editable->GetIHitable() != nullptr)
            for (const HitObject *const hitObject : m_player->m_physics->GetUIHitObjects(editable))
               bounds.Extend(hitObject->m_hitBBox);
         // Also include the part's position to cover parts without hit objects
         Matrix3D transform;
         if (part->GetTransform(transform) != EditorUIPart::TM_None)
         {
            const Vertex3Ds pos = transform.GetOrthoNormalPos();
            bounds.Extend(FRect3D(pos.x, pos.x, pos.y, pos.y, pos.z, pos.z));
         }
      };
      for (const auto &part : m_multiSel)
         extendWithPart(part);
      if (m_multiSel.empty() && m_selection.GetPart())
         extendWithPart(m_selection.GetPart());
   }
   return bounds.left <= bounds.right;
}

void EditorUI::SetSelectionTransform(const Matrix3D &newTransform, bool clearPosition, bool clearScale, bool clearRotation) const
{
   if (m_pointEditPart)
   {
      // In point edit mode, apply the gizmo transform delta to the selected points in the table XY plane
      // (to the part's center point in center edit mode)
      if (m_pointEditCenter)
      {
         if (!m_centerSelected)
            return;
         Matrix3D oldTransform;
         GetSelectionTransform(oldTransform);
         Matrix3D invOldTransform(oldTransform);
         invOldTransform.Invert();
         const Matrix3D delta = invOldTransform * newTransform;
         const Vertex3Ds v = delta * m_pointEditPart->GetEditCenter();
         m_pointEditPart->SetEditCenter(Vertex2D(v.x, v.y));
      }
      else
      {
         if (m_pointSel.empty())
            return;
         Matrix3D oldTransform;
         GetSelectionTransform(oldTransform);
         Matrix3D invOldTransform(oldTransform);
         invOldTransform.Invert();
         const Matrix3D delta = invOldTransform * newTransform;
         for (DragPoint *point : m_pointSel)
         {
            const Vertex3Ds v = delta * point->GetVertex();
            point->SetX(v.x);
            point->SetY(v.y);
         }
         m_pointEditPart->GetDragPointCurve()->OnPointsModified();
      }
      m_renderer->ReinitRenderable(m_pointEditPart->GetEditable()->GetIRenderable());
      m_player->m_physics->Update(m_pointEditPart->GetEditable());
      return;
   }

   if (m_selection.GetType() != Selection::S_EDITABLE)
      return;

   // Decompose a transform into the position/scale/rotation components used by the parts' SetTransform
   const auto extractTRS = [](Matrix3D transform, vec3 &pos, vec3 &scale, vec3 &rot)
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
         rot.x = -RADTOANG(atan2f(-transform._23, transform._22)); // Gimbal lock (Y at +/-90 degrees): Z is 0, X takes the remaining rotation
         rot.y = -RADTOANG(atan2f(-transform._31, sy));
         rot.z = 0.f;
      }
   };

   const auto applyTransform = [this, &extractTRS](const std::shared_ptr<EditorUIPart> &part, const Matrix3D &partTransform, bool clearPos, bool clearScale, bool clearRot)
   {
      vec3 pos, scale, rot;
      extractTRS(partTransform, pos, scale, rot);
      if (clearPos)
         pos.Set(0.f, 0.f, 0.f);
      if (clearScale)
         scale.Set(1.f, 1.f, 1.f);
      if (clearRot)
         rot.Set(0.f, 0.f, 0.f);
      part->SetTransform(pos, scale, rot);
      m_renderer->ReinitRenderable(part->GetEditable()->GetIRenderable());
      m_player->m_physics->Update(part->GetEditable());
   };

   if (clearPosition || clearScale || clearRotation)
   {
      // Reset the requested transform components of each selected part
      for (const auto &part : m_multiSel)
      {
         Matrix3D partTransform;
         if (part->GetTransform(partTransform) == EditorUIPart::TM_None)
            continue;
         applyTransform(part, partTransform, clearPosition, clearScale, clearRotation);
      }
      return;
   }

   // Apply the active part's transform delta to every selected part
   Matrix3D delta;
   m_selection.GetPart()->GetTransform(delta);
   delta.Invert();
   delta = delta * newTransform;
   for (const auto &part : m_multiSel)
   {
      Matrix3D partTransform;
      if (part->GetTransform(partTransform) == EditorUIPart::TM_None)
         continue;
      applyTransform(part, partTransform * delta, false, false, false);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Drag point edit mode: edit the points of the DragPointCurve of the active selected part.
// Entered/exited with Tab (or Escape). The selection is saved on entry and restored on exit.
//

bool EditorUI::CanEnterPointEdit() const
{
   return m_pointEditPart == nullptr && !IsInspectMode() && !m_table->IsLocked() && m_selection.GetType() == Selection::S_EDITABLE && m_selection.GetPart() != nullptr
      && (m_selection.GetPart()->GetDragPointCurve() != nullptr || m_selection.GetPart()->HasEditCenter());
}

void EditorUI::EnterPointEditMode()
{
   if (!CanEnterPointEdit())
      return;
   m_savedSelection = m_selection;
   m_savedMultiSel = m_multiSel;
   OutlinerPanel::m_savedAnchor = OutlinerPanel::m_anchor;
   m_pointEditPart = m_selection.GetPart();
   m_pointEditPart->SetPointEditContext(this);
   // Parts without outline points but with an editable center (light sequencer, lights of circle shape) edit it directly
   const DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve();
   m_pointEditCenter = m_pointEditPart->HasEditCenter() && (curve == nullptr || curve->GetPoints().empty());
   m_centerSelected = m_pointEditCenter;
   m_pointSel.clear();
}

void EditorUI::ExitPointEditMode(bool restoreSelection)
{
   if (m_pointEditPart)
      m_pointEditPart->SetPointEditContext(nullptr);
   m_pointEditPart.reset();
   m_pointEditCenter = false;
   m_centerSelected = false;
   m_pointSel.clear();
   m_pointDragPending = false;
   m_pointDragActive = false;
   if (!restoreSelection)
      return;
   // Restore the selection as it was when entering this mode, filtering out parts deleted since then
   const ankerl::unordered_dense::set<const IEditable *> liveParts(m_table->GetParts().begin(), m_table->GetParts().end());
   std::erase_if(m_savedMultiSel, [&liveParts](const std::shared_ptr<EditorUIPart> &part) { return !liveParts.contains(part->GetEditable()); });
   m_multiSel = m_savedMultiSel;
   m_selection = (m_savedSelection.GetType() == Selection::S_EDITABLE && (m_savedSelection.GetPart() == nullptr || !liveParts.contains(m_savedSelection.GetPart()->GetEditable())))
      ? (m_multiSel.empty() ? Selection() : Selection(m_multiSel.back()))
      : m_savedSelection;
   OutlinerPanel::m_anchor = (OutlinerPanel::m_savedAnchor == nullptr || liveParts.contains(OutlinerPanel::m_savedAnchor->GetEditable())) ? OutlinerPanel::m_savedAnchor : nullptr;
}

bool EditorUI::IsPointSelected(const DragPoint *point) const { return std::ranges::find(m_pointSel, point) != m_pointSel.end(); }

void EditorUI::TogglePointSelection(DragPoint *point)
{
   const auto it = std::ranges::find(m_pointSel, point);
   if (it == m_pointSel.end())
      m_pointSel.push_back(point);
   else
      m_pointSel.erase(it);
}

void EditorUI::BeginPointEdit()
{
   if (m_pointEditPart == nullptr)
      return;
   m_undo.BeginUndo();
   m_undo.MarkForUndo(m_pointEditPart->GetEditable());
   m_undo.EndUndo();
}

void EditorUI::EndPointEdit()
{
   if (m_pointEditPart == nullptr)
      return;
   if (DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve())
      curve->OnPointsModified();
   m_renderer->ReinitRenderable(m_pointEditPart->GetEditable()->GetIRenderable());
   m_player->m_physics->Update(m_pointEditPart->GetEditable());
}

void EditorUI::AddPointOnNearestSegment()
{
   DragPointCurve *const curve = (m_pointEditPart != nullptr && !m_table->IsLocked()) ? m_pointEditPart->GetDragPointCurve() : nullptr;
   if (curve == nullptr)
      return;
   const auto &points = curve->GetPoints();
   if (points.empty())
      return;
   // Unproject the mouse position on the drag plane of the edited curve (a blueprint pane maps
   // directly to the table XY plane, a viewport pane projects at the curve's display height)
   Vertex2D pos(0.f, 0.f);
   if (m_hoverBp != nullptr)
      pos = m_hoverBp->Unproject(ImGui::GetMousePos());
   else
   {
      const float z = m_pointEditPart->GetDragPointZ(m_pointSel.empty() ? points.front().get() : m_pointSel.front());
      ViewportPane *const vp = GetTargetViewport();
      if (vp == nullptr)
         return;
      pos = vp->UnprojectToPlane(ImGui::GetMousePos(), z);
   }
   m_undo.BeginUndo();
   m_undo.MarkForUndo(m_pointEditPart->GetEditable());
   m_undo.EndUndo();
   DragPoint *const point = m_pointEditPart->AddPointOnCurve(pos);
   if (point == nullptr)
   {
      m_undo.Discard();
      return;
   }
   // Select the newly created point
   m_pointSel.clear();
   m_pointSel.push_back(point);
   m_renderer->ReinitRenderable(m_pointEditPart->GetEditable()->GetIRenderable());
   m_player->m_physics->Update(m_pointEditPart->GetEditable());
}

void EditorUI::DeleteSelectedPoints()
{
   if (m_pointEditPart == nullptr || m_pointSel.empty() || m_table->IsLocked() || IsInspectMode())
      return;
   DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve();
   vector<DragPoint *> deletable;
   for (DragPoint *point : m_pointSel)
      if (point->CanDelete())
         deletable.push_back(point);
   if (deletable.empty())
      return;
   BeginPointEdit();
   for (DragPoint *point : deletable)
   {
      m_pointSel.erase(std::ranges::find(m_pointSel, point));
      curve->DeletePoint(point);
   }
   EndPointEdit();
}

// ---------------------------------------------------------------------------------------------------------------------
// Areas (Blender-style split tree, see editor/AreaLayout.h)

const char *EditorUI::AreaTypeName(const AreaType type)
{
   return type == AreaType::Viewport       ? "3D Viewport"
      : type == AreaType::ViewportBackdrop ? "Desktop Backdrop"
      : type == AreaType::ViewportPreview  ? "Render Preview"
      : type == AreaType::Blueprint        ? "Blueprint"
                                           : AreaTypeJsonName(type);
}

const char *EditorUI::AreaTypeIcon(const AreaType type)
{
   switch (type)
   {
   case AreaType::Outliner: return ICON_LC_LIST;
   case AreaType::Properties: return ICON_LC_SLIDERS_HORIZONTAL;
   case AreaType::Script: return ICON_LC_CODE_XML;
   case AreaType::PartLibrary: return ICON_LC_BOX;
   case AreaType::Viewport: return ICON_LC_MOVE_3D;
   case AreaType::ViewportBackdrop: return ICON_LC_IMAGE;
   case AreaType::ViewportPreview: return ICON_LC_PLAY;
   case AreaType::Blueprint: return ICON_LC_RULER;
   case AreaType::Count: break;
   }
   return "";
}

bool EditorUI::IsAreaTypeImplemented(const AreaType type)
{
   // The area types a leaf may take through the editor type switcher (the others are added with their views)
   return type == AreaType::Outliner || type == AreaType::Properties || IsViewportType(type) || type == AreaType::PartLibrary || type == AreaType::Script || type == AreaType::Blueprint;
}

std::unique_ptr<EditorAreaView> EditorUI::CreateAreaView(const AreaType type)
{
   switch (type)
   {
   case AreaType::Outliner: return std::make_unique<OutlinerPanel>(*this);
   case AreaType::Properties: return std::make_unique<PropertiesPanel>(*this);
   case AreaType::Viewport: return std::make_unique<ViewportPane>(*this, ViewMode::EditorCam);
   case AreaType::ViewportBackdrop: return std::make_unique<ViewportPane>(*this, ViewMode::DesktopBackdrop);
   case AreaType::ViewportPreview: return std::make_unique<ViewportPane>(*this, ViewMode::PreviewCam);
   case AreaType::Blueprint: return std::make_unique<BlueprintPane>(*this);
   case AreaType::PartLibrary: return std::make_unique<PartLibraryPane>(*this);
   case AreaType::Script: return std::make_unique<ScriptPane>(*this);
   default: return nullptr;
   }
}

void EditorUI::Report(const string &message)
{
   // Transient report shown on the right of the status bar (fading out); a toast keeps the
   // feedback visible while the bar itself is hidden (fly mode, inspection modal)
   m_report = message;
   m_reportTime = ImGui::GetTime();
   if (m_chrome.GetStatusBarHeight() <= 0.f)
      m_liveUI.PushNotification(message, 3000);
}

bool EditorUI::ConsumedAreaEscape()
{
   if (m_areaEscPending)
   {
      if (!ImGui::IsKeyDown(ImGuiKey_Escape))
         m_areaEscPending = false;
      return true;
   }
   return false;
}

void EditorUI::SetAreaEditor(const int leafId, const AreaType type)
{
   PendingAreaOp op;
   op.kind = PendingAreaOp::Kind::SetType;
   op.leaf = leafId;
   op.type = type;
   QueueAreaOp(op);
}

void EditorUI::ToggleMaximizeArea()
{
   PendingAreaOp op;
   op.kind = PendingAreaOp::Kind::Maximize;
   op.leaf = m_maximizedLeafId >= 0 ? -1 : m_hoverLeaf; // leaf -1 restores
   QueueAreaOp(op);
}

ImVec2 EditorUI::GetViewportContentCenter() const
{
   if (!m_areaRectsValid)
      ComputeAreaRects();
   if (!m_viewportVisible)
      return ImVec2(0.5f * ImGui::GetIO().DisplaySize.x, 0.5f * ImGui::GetIO().DisplaySize.y);
   return ImVec2(0.5f * (m_viewportRect.x0 + m_viewportRect.x1), 0.5f * (m_viewportRect.y0 + m_viewportRect.y1));
}

void EditorUI::WidenArea(const int leafId, const AreaType type)
{
   // Switching the type of an area that is too narrow for the new editor widens it (once, the splitter can shrink it again afterwards)
   const float minimum = type == AreaType::Properties ? 300.f : type == AreaType::Script ? 400.f : 0.f;
   if (minimum <= 0.f)
      return;
   // The split whose stored size is the width of the area: the nearest vertical ancestor whose fixed child holds the area
   const auto &nodes = m_layout.Nodes();
   const AreaNode *split = nullptr;
   int child = leafId;
   for (int parent = m_layout.ParentId(child); parent >= 0; child = parent, parent = m_layout.ParentId(child))
   {
      const AreaNode *const candidate = m_layout.Find(parent);
      if (candidate != nullptr && candidate->axis == SplitAxis::Vertical && nodes[candidate->child[candidate->fixed]].id == static_cast<uint32_t>(child))
      {
         split = candidate;
         break;
      }
   }
   if (split == nullptr)
      return;
   // Same limit as the splitters (45% of the window), and what the other areas take is kept
   const float dpi = m_liveUI.GetDPI();
   float mineX0 = 0.f, mineX1 = 0.f;
   for (const LeafRect &leaf : m_leafRects)
      if (leaf.id == leafId && leaf.visible)
      {
         mineX0 = leaf.rect.x0;
         mineX1 = leaf.rect.x1;
      }
   std::map<float, float> columns; // Left edge -> width of the other visible panel areas (stacked areas count once)
   for (const LeafRect &leaf : m_leafRects)
      if (leaf.visible && leaf.id != leafId && !IsViewportType(leaf.type) && (mineX1 <= mineX0 || leaf.rect.x0 >= mineX1 || leaf.rect.x1 <= mineX0))
         columns[leaf.rect.x0] = max(columns[leaf.rect.x0], leaf.rect.Width() / dpi);
   float taken = 0.f;
   for (const auto &column : columns)
      taken += column.second;
   const float limit = max(0.f, 0.45f * ImGui::GetIO().DisplaySize.x / dpi - taken);
   const float target = min(minimum, limit);
   if (split->size < target)
      m_layout.SetSplitSize(static_cast<int>(split->id), target);
}

bool EditorUI::ApplyAreaOp(const PendingAreaOp &op)
{
   switch (op.kind)
   {
   case PendingAreaOp::Kind::SetType:
   {
      const AreaNode *const node = m_layout.Find(op.leaf);
      if (node == nullptr || node->isSplit || node->type == op.type || !IsAreaTypeImplemented(op.type))
         return false;
      if (!m_layout.SetType(op.leaf, op.type))
         return false;
      WidenArea(op.leaf, op.type);
      if (op.type == AreaType::PartLibrary)
         m_partLibrary.m_rescanLibrary = true;
      return true;
   }
   case PendingAreaOp::Kind::Close:
   {
      if (m_maximizedLeafId == op.leaf)
         m_maximizedLeafId = -1;
      return m_layout.Close(op.leaf);
   }
   case PendingAreaOp::Kind::Show:
   {
      // Hosts the given type: reveals a hidden leaf of that type if there is one, else splits the first
      // visible leaf (the new leaf takes the shown type)
      for (const AreaNode &n : m_layout.Nodes())
         if (!n.isSplit && n.type == op.type && n.hidden)
         {
            if (m_maximizedLeafId >= 0 && m_maximizedLeafId != static_cast<int>(n.id))
               m_maximizedLeafId = -1;
            return m_layout.SetHidden(static_cast<int>(n.id), false);
         }
      if (m_layout.IsHosted(op.type))
         return true; // Already shown
      int target = -1;
      for (const LeafRect &leaf : m_leafRects)
         if (leaf.visible && (target < 0 || leaf.rect.x0 < m_leafRects[0].rect.x0))
            target = leaf.id;
      if (target < 0 || !m_layout.Split(target, SplitAxis::Vertical, true, 280.f))
         return false;
      // The new leaf (the other child of the target's parent) takes the shown type
      const AreaNode *const kept = m_layout.Find(target);
      const int parent = kept != nullptr ? m_layout.ParentId(target) : -1;
      const AreaNode *const split = m_layout.Find(parent);
      int newLeaf = -1;
      if (split != nullptr)
         for (size_t i = 0; i < m_layout.Nodes().size(); ++i)
            if (static_cast<int>(i) == split->child[0] || static_cast<int>(i) == split->child[1])
               if (static_cast<int>(m_layout.Nodes()[i].id) != target)
                  newLeaf = static_cast<int>(m_layout.Nodes()[i].id);
      return newLeaf >= 0 && m_layout.SetType(newLeaf, op.type);
   }
   case PendingAreaOp::Kind::Split:
   {
      m_maximizedLeafId = -1; // The new layout must be visible
      return m_layout.Split(op.leaf, op.axis, op.newOnSecondSide, op.size);
   }
   case PendingAreaOp::Kind::Join:
   {
      m_maximizedLeafId = -1;
      if (!m_layout.Join(op.leaf, op.leaf2))
         return false;
      if (m_maximizedLeafId >= 0 && m_layout.Find(m_maximizedLeafId) == nullptr)
         m_maximizedLeafId = -1;
      return true;
   }
   case PendingAreaOp::Kind::Swap: m_maximizedLeafId = -1; return m_layout.Swap(op.leaf);
   case PendingAreaOp::Kind::Maximize:
   {
      const AreaNode *const node = op.leaf >= 0 ? m_layout.Find(op.leaf) : nullptr;
      m_maximizedLeafId = (node != nullptr && !node->isSplit && !node->hidden) ? op.leaf : -1;
      return true;
   }
   }
   return false;
}

void EditorUI::LoadAreaLayoutSettings()
{
   m_layoutSettingsLoaded = true;
   Settings &settings = g_settingsService.GetAppSettings();
   m_activeLayoutIndex = clamp(settings.GetLiveEditor_LayoutActive() - 1, 0, 4);
   const string json = m_activeLayoutIndex == 0 ? settings.GetLiveEditor_Layout1()
      : m_activeLayoutIndex == 1                ? settings.GetLiveEditor_Layout2()
      : m_activeLayoutIndex == 2                ? settings.GetLiveEditor_Layout3()
      : m_activeLayoutIndex == 3                ? settings.GetLiveEditor_Layout4()
                                                : settings.GetLiveEditor_Layout5();
   const nlohmann::json parsed = nlohmann::json::parse(json, nullptr, false);
   if (parsed.is_discarded() || !m_layout.FromJson(parsed))
      m_layout = AreaLayout::Default(m_activeLayoutIndex); // Missing or invalid layout: back to the slot's preset
   m_maximizedLeafId = -1;
   m_hoverLeaf = -1;
   CancelAreaInteraction(false);
   m_hoverVp = nullptr;
   m_captureVp = nullptr;
   if (settings.GetEditor_AlwaysViewScript() && !m_layout.IsHosted(AreaType::Script))
      ShowAreaEditor(AreaType::Script); // The editor option keeps a script area open
}

void EditorUI::SaveAreaLayoutSettings()
{
   // The persisted layout is only loaded once the editor areas are shown: until then, m_layout is a placeholder that
   // must not overwrite it (e.g. at the end of a play session that never opened the editor)
   if (!m_layoutSettingsLoaded)
      return;
   // Persist each pane's view setup (3D camera, blueprint zoom/offset, ...) in its leaf's payload
   for (const auto &[id, view] : m_areaViews)
      if (nlohmann::json *const data = m_layout.LeafData(id); data != nullptr)
         *data = view->SaveSetup();
   Settings &settings = g_settingsService.GetAppSettings();
   const string json = m_layout.ToJson().dump();
   switch (m_activeLayoutIndex)
   {
   case 0: settings.SetLiveEditor_Layout1(json, false); break;
   case 1: settings.SetLiveEditor_Layout2(json, false); break;
   case 2: settings.SetLiveEditor_Layout3(json, false); break;
   case 3: settings.SetLiveEditor_Layout4(json, false); break;
   default: settings.SetLiveEditor_Layout5(json, false); break;
   }
}

void EditorUI::SetAreaLayoutIndex(const int index)
{
   if (index == m_activeLayoutIndex)
      return;
   SaveAreaLayoutSettings(); // The current layout leaves its slot
   m_activeLayoutIndex = index;
   g_settingsService.GetAppSettings().SetLiveEditor_LayoutActive(index + 1, false);
   LoadAreaLayoutSettings(); // Loads the slot's saved layout, or its preset when empty/invalid
}

void EditorUI::ResetAreaLayouts()
{
   Settings &settings = g_settingsService.GetAppSettings();
   settings.ResetLiveEditor_Layout1();
   settings.ResetLiveEditor_Layout2();
   settings.ResetLiveEditor_Layout3();
   settings.ResetLiveEditor_Layout4();
   settings.ResetLiveEditor_Layout5();
   LoadAreaLayoutSettings(); // The empty slot falls back to its preset
   Report("Layouts reset to their defaults");
}

void EditorUI::UpdateAreaLayout()
{
   if (!m_layoutSettingsLoaded)
      LoadAreaLayoutSettings();
   if (m_maximizedLeafId >= 0 && m_layout.Find(m_maximizedLeafId) == nullptr)
      m_maximizedLeafId = -1;
   // Queued layout changes are applied here, before any pane is drawn: no pane disappears between two frames.
   // They wait while a field is being dragged or edited
   if (!m_pendingAreaOps.empty() && !ImGui::IsAnyItemActive())
   {
      const vector<PendingAreaOp> ops = std::move(m_pendingAreaOps);
      m_pendingAreaOps.clear();
      ComputeAreaRects(); // Widths and sides are evaluated on the current rects
      for (const PendingAreaOp &op : ops)
      {
         const bool applied = ApplyAreaOp(op);
         m_raiseAreaWindows |= applied;
         if (applied)
            SaveAreaLayoutSettings(); // The layout is persisted in its settings slot on every change
         else if (op.kind == PendingAreaOp::Kind::Split)
            Report("Cannot split this area");
         else if (op.kind == PendingAreaOp::Kind::Join)
            Report("Cannot join these areas");
      }
   }
   ComputeAreaRects();
   // Area under the mouse, for the maximize toggle: not while a popup (editor type menu,...) covers the areas
   if (!ImGui::IsPopupOpen(static_cast<ImGuiID>(0), ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
   {
      const ImVec2 mouse = ImGui::GetMousePos();
      const int under = AreaLayout::LeafAt(m_leafRects, mouse.x, mouse.y);
      if (under >= 0)
         m_hoverLeaf = under;
   }
   if (m_hoverLeaf >= 0 && m_layout.Find(m_hoverLeaf) == nullptr)
      m_hoverLeaf = -1;
}

void EditorUI::ComputeAreaRects() const
{
   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float dpi = m_liveUI.GetDPI();
   // The tree fills the window below the main menu bar (WorkPos excludes it: unlike the measured
   // m_menuBarHeight it has no one frame lag and is never 0 while the chrome toggles)
   const float top = viewport->WorkPos.y;
   const AreaRect tree { viewport->Pos.x, top, viewport->Pos.x + viewport->Size.x, max(top, viewport->Pos.y + viewport->Size.y - m_chrome.GetStatusBarHeight()) };
   const bool areasShown = !m_table->IsLocked() && !IsFlyMode() && !m_inspectionModal.IsVisible();
   m_layout.Compute(tree, dpi, m_leafRects, m_splitterRects, areasShown ? m_maximizedLeafId : -1);

   // The viewport area is the window onto the 3D render; when no area is drawn (locked table, fly mode,
   // inspection) it is the whole tree
   m_viewportRect = tree;
   m_viewportVisible = true;
   if (areasShown)
   {
      m_viewportVisible = false;
      m_viewportRect = AreaRect(); // A panel is maximized: nothing of the viewport is shown
      for (const LeafRect &leaf : m_leafRects)
         if (IsViewportType(leaf.type) && leaf.visible)
         {
            m_viewportVisible = true;
            const auto it = m_areaViews.find(leaf.id);
            if (it != m_areaViews.end() && it->second.get() == GetTargetViewport())
            {
               m_viewportRect = leaf.rect; // The edit viewport's area is the reference for the overlay and the camera fit
               break;
            }
            if (m_viewportRect.x1 <= m_viewportRect.x0)
               m_viewportRect = leaf.rect; // Fall back to the first shown viewport area
         }
   }
   m_areaRectsValid = true;
}

bool EditorUI::ShouldRaiseAreaWindows() const { return m_raiseAreaWindows && !ImGui::IsPopupOpen(static_cast<ImGuiID>(0), ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel); }

bool EditorUI::CanUseAreaTools() const
{
   // The corner zones, the edge menu and their modes only exist while nothing else owns the mouse
   if (m_table->IsLocked() || IsFlyMode() || m_inspectionModal.IsVisible() || m_fileOp != nullptr)
      return false;
   if (m_pointEditPart != nullptr)
      return false;
   if (IsInspectMode() && m_player->IsPlaying())
      return false; // The game owns the keys and the mouse
   return m_maximizedLeafId < 0 && ImGui::GetTopMostPopupModal() == nullptr;
}

const LeafRect *EditorUI::FindLeafRect(const int leafId) const
{
   for (const LeafRect &leaf : m_leafRects)
      if (leaf.id == leafId && leaf.visible)
         return &leaf;
   return nullptr;
}

bool EditorUI::PlanAreaSplit(const int leafId, const SplitAxis axis, const bool newOnSecondSide, const ImVec2 &cursor, AreaSplitPlan &plan) const
{
   // Divider of a split of the area at the cursor: the new area is on the side of newOnSecondSide (right / bottom), both parts keep their minimum size
   plan = AreaSplitPlan();
   plan.axis = axis;
   const LeafRect *const leaf = FindLeafRect(leafId);
   if (leaf == nullptr)
   {
      plan.reason = "Area not found";
      return false;
   }
   plan.leaf = leaf->rect;
   plan.newType = leaf->type; // A split duplicates the area (Blender)
   const float dpi = m_liveUI.GetDPI();
   const bool vertical = axis == SplitAxis::Vertical;
   const float lo = vertical ? leaf->rect.x0 : leaf->rect.y0;
   const float hi = vertical ? leaf->rect.x1 : leaf->rect.y1;
   plan.line = clamp(roundf(vertical ? cursor.x : cursor.y), lo, hi);
   if (!m_layout.CanSplit(leafId))
   {
      plan.reason = "Cannot split this area";
      return false;
   }
   const float newMin = (vertical ? AreaLayout::kPanelMinW : AreaLayout::kPanelMinH) * dpi;
   // Same minimum as the layout solver (AreaLayout::Compute)
   const float keepMin
      = (IsViewportType(leaf->type) ? (vertical ? AreaLayout::kViewportMinW : AreaLayout::kViewportMinH) : (vertical ? AreaLayout::kPanelMinW : AreaLayout::kPanelMinH)) * dpi;
   if (hi - lo < newMin + keepMin)
   {
      plan.reason = "Area too small to split";
      return false;
   }
   plan.line = roundf(clamp(plan.line, newOnSecondSide ? lo + keepMin : lo + newMin, newOnSecondSide ? hi - newMin : hi - keepMin));
   plan.size = (newOnSecondSide ? hi - plan.line : plan.line - lo) / dpi;
   plan.newRect = leaf->rect;
   if (vertical)
   {
      if (newOnSecondSide)
         plan.newRect.x0 = plan.line;
      else
         plan.newRect.x1 = plan.line;
   }
   else if (newOnSecondSide)
      plan.newRect.y0 = plan.line;
   else
      plan.newRect.y1 = plan.line;
   plan.ok = true;
   return true;
}

const char *EditorUI::AreaJoinRefusal(const int keepId, const int consumeId) const
{
   const AreaNode *const keep = m_layout.Find(keepId);
   const AreaNode *const consume = m_layout.Find(consumeId);
   if (keep == nullptr || consume == nullptr)
      return "Area not found";
   if (keep->isSplit || consume->isSplit)
      return "Areas with sub-areas cannot be joined";
   if (m_layout.ParentId(keepId) < 0 || m_layout.ParentId(keepId) != m_layout.ParentId(consumeId))
      return "Areas must share a full edge to be joined";
   return "Cannot join these areas";
}

void EditorUI::DrawAreaSplitPreview(const AreaSplitPlan &plan) const
{
   ImDrawList *const drawList = ImGui::GetForegroundDrawList();
   const bool vertical = plan.axis == SplitAxis::Vertical;
   const ImVec2 a = vertical ? ImVec2(plan.line, plan.leaf.y0) : ImVec2(plan.leaf.x0, plan.line);
   const ImVec2 b = vertical ? ImVec2(plan.line, plan.leaf.y1) : ImVec2(plan.leaf.x1, plan.line);
   const auto centeredText = [&](const AreaRect &rect, const char *text, const ImU32 color)
   {
      const ImVec2 size = ImGui::CalcTextSize(text);
      drawList->AddText(ImVec2(0.5f * (rect.x0 + rect.x1) - 0.5f * size.x, 0.5f * (rect.y0 + rect.y1) - 0.5f * size.y), color, text);
   };
   if (plan.ok)
   {
      // The new area is tinted and named, the divider is where the split will be
      drawList->AddRectFilled(ImVec2(plan.newRect.x0, plan.newRect.y0), ImVec2(plan.newRect.x1, plan.newRect.y1), IM_COL32(237, 145, 62, 38));
      centeredText(plan.newRect, AreaTypeName(plan.newType), EditorTheme::Text);
      drawList->AddLine(a, b, EditorTheme::Accent, 2.f);
   }
   else
   {
      drawList->AddLine(a, b, IM_COL32(150, 150, 150, 220), 2.f);
      centeredText(plan.leaf, plan.reason, EditorTheme::Text);
      ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
   }
}

void EditorUI::DrawAreaJoinPreview(const int keepId, const int consumeId, const bool ok) const
{
   const LeafRect *const keep = FindLeafRect(keepId);
   const LeafRect *const consume = FindLeafRect(consumeId);
   if (keep == nullptr || consume == nullptr)
      return;
   ImDrawList *const drawList = ImGui::GetForegroundDrawList();
   const float dpi = m_liveUI.GetDPI();
   const ImVec2 cmin(consume->rect.x0, consume->rect.y0), cmax(consume->rect.x1, consume->rect.y1);
   if (!ok)
   {
      drawList->AddRect(cmin, cmax, IM_COL32(220, 60, 60, 255), 0.f, 0, 2.f * dpi);
      ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
      return;
   }
   // The consumed area is darkened and an arrow points into it from the kept one, which gets an outline
   drawList->AddRectFilled(cmin, cmax, IM_COL32(0, 0, 0, 115));
   drawList->AddRect(ImVec2(keep->rect.x0 + dpi, keep->rect.y0 + dpi), ImVec2(keep->rect.x1 - dpi, keep->rect.y1 - dpi), EditorTheme::Accent, 0.f, 0, 2.f * dpi);
   const ImVec2 from(0.5f * (keep->rect.x0 + keep->rect.x1), 0.5f * (keep->rect.y0 + keep->rect.y1));
   const ImVec2 centre(0.5f * (cmin.x + cmax.x), 0.5f * (cmin.y + cmax.y));
   ImVec2 dir(centre.x - from.x, centre.y - from.y);
   const float dirLength = sqrtf(dir.x * dir.x + dir.y * dir.y);
   if (dirLength < 1e-3f)
      return;
   dir = ImVec2(dir.x / dirLength, dir.y / dirLength);
   const ImVec2 perp(-dir.y, dir.x);
   const float length = 0.3f * min(consume->rect.Width(), consume->rect.Height());
   const float head = 0.5f * length; // Length of the triangle
   const float halfHead = 0.35f * length;
   const ImVec2 tail(centre.x - dir.x * 0.5f * length, centre.y - dir.y * 0.5f * length);
   const ImVec2 tip(centre.x + dir.x * 0.5f * length, centre.y + dir.y * 0.5f * length);
   const ImVec2 base(tip.x - dir.x * head, tip.y - dir.y * head);
   drawList->AddLine(tail, base, EditorTheme::Accent, max(3.f * dpi, 0.12f * length));
}

void EditorUI::RenderAreas()
{
   // Drop the views whose leaf is gone or was retyped (their OnUnhosted releases held resources)
   for (auto it = m_areaViews.begin(); it != m_areaViews.end();)
   {
      const AreaNode *const node = m_layout.Find(it->first);
      if (node == nullptr || node->isSplit || node->type != it->second->GetType())
      {
         it->second->OnUnhosted();
         it = m_areaViews.erase(it);
      }
      else
         ++it;
   }
   // Update the viewport pane states: the hovered one (for input routing), and pause
   // rendering of the panes whose leaf is not shown (hidden or behind a maximized area)
   std::unordered_map<int, bool> shown;
   for (const LeafRect &leaf : m_leafRects)
      shown[leaf.id] = leaf.visible;
   m_hoverVp = nullptr;
   m_hoverBp = nullptr;
   for (const auto &[id, view] : m_areaViews)
   {
      if (!shown[id])
         view->CancelGizmoDrag(); // A pane that is not drawn could not release its gizmo drag
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()))
      {
         vp->SetRendering(shown[id]);
         if (!shown[id])
            vp->m_hovered = vp->m_active = false; // A pane that is not drawn must not keep stale item state
         else if (vp->m_hovered)
            m_hoverVp = vp;
      }
      else if (auto *const bp = dynamic_cast<BlueprintPane *>(view.get()))
      {
         if (!shown[id])
            bp->m_hovered = bp->m_active = false;
         else if (bp->m_hovered)
            m_hoverBp = bp;
      }
   }
   for (const LeafRect &leaf : m_leafRects)
   {
      if (!leaf.visible)
         continue;
      auto [it, inserted] = m_areaViews.try_emplace(leaf.id, nullptr);
      if (it->second == nullptr)
      {
         it->second = CreateAreaView(leaf.type);
         // A leaf restored from the persisted layout may carry its view's setup (the 3D view's camera, shading, ...)
         if (const AreaNode *const node = m_layout.Find(leaf.id); node != nullptr)
            it->second->LoadSetup(node->data);
         if (m_outlinerFocusPending)
            if (auto *const op = dynamic_cast<OutlinerPanel *>(it->second.get()); op != nullptr)
            {
               op->FocusFilter();
               m_outlinerFocusPending = false;
            }
      }
      if (it->second != nullptr)
      {
         // Blender look: a thin black gap between the panes
         AreaRect paneRect = leaf.rect;
         const float gap = 2.f * m_liveUI.GetDPI();
         paneRect.x0 += gap;
         paneRect.y0 += gap;
         paneRect.x1 -= gap;
         paneRect.y1 -= gap;
         it->second->RenderPane(paneRect, leaf.id);
      }
   }
   // Route the interactions to the viewport pane under the mouse; an ongoing drag keeps its pane
   if (m_hoverVp != nullptr && m_hoverVp->m_active)
      m_captureVp = m_hoverVp;
   else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
      m_captureVp = nullptr;
   // Same routing for the blueprint panes (2D canvas interactions)
   if (m_hoverBp != nullptr && m_hoverBp->m_active)
      m_captureBp = m_hoverBp;
   else if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) && !ImGui::IsMouseDown(ImGuiMouseButton_Middle) && !ImGui::IsMouseDown(ImGuiMouseButton_Right))
      m_captureBp = nullptr;
}

void EditorUI::RenderAreaTypeButton(const int leafId)
{
   // The area's own editor type button, drawn inside its window header (Blender's top left icon and chevron)
   const AreaNode *const node = m_layout.Find(leafId);
   if (node == nullptr || node->isSplit)
      return;
   const ImGuiStyle &style = ImGui::GetStyle();
   const char *const icon = AreaTypeIcon(node->type);
   const ImVec4 iconInk = TextInkBounds(icon);
   const ImVec2 iconSize = ImGui::CalcTextSize(icon);
   const float chevronWidth = ImGui::GetFontSize() * 0.4f; // Small, drawn in TextDim
   const ImVec2 buttonSize(style.FramePadding.x * 2.f + iconSize.x + style.ItemInnerSpacing.x + chevronWidth, ImMax(iconSize.y, iconInk.w - iconInk.y) + style.FramePadding.y * 2.f);
   char popup[40];
   snprintf(popup, sizeof(popup), "##EditorType%d", leafId); // Keyed by the leaf id: several areas may share a type

   const bool clicked = ImGui::Button("##AreaTypeButton", buttonSize);
   const ImVec2 itemMin = ImGui::GetItemRectMin();
   const ImVec2 itemMax = ImGui::GetItemRectMax();
   {
      ImDrawList *const drawList = ImGui::GetWindowDrawList();
      const float centerY = (itemMin.y + itemMax.y) * 0.5f;
      drawList->AddText(ImVec2(itemMin.x + style.FramePadding.x - iconInk.x, itemMin.y + style.FramePadding.y - iconInk.y), EditorTheme::Text, icon);
      const float half = chevronWidth * 0.5f;
      const float centerX = itemMax.x - style.FramePadding.x - half;
      drawList->AddTriangleFilled(ImVec2(centerX - half, centerY - half * 0.5f), ImVec2(centerX + half, centerY - half * 0.5f), ImVec2(centerX, centerY + half * 0.5f), EditorTheme::TextDim);
   }
   ImGui::SetItemTooltip("Editor type");
   if (clicked)
      ImGui::OpenPopup(popup);
   ImGui::SetNextWindowPos(ImVec2(itemMin.x, itemMax.y), ImGuiCond_Always);
   if (ImGui::BeginPopup(popup))
   {
      if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
      {
         ImGui::CloseCurrentPopup();
         m_areaEscPending = true;
      }
      ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(EditorTheme::TextDim));
      ImGui::TextUnformatted("Editor Type");
      ImGui::PopStyleColor();
      ImGui::Separator();
      for (int t = 0; t < static_cast<int>(AreaType::Count); ++t)
      {
         const AreaType type = static_cast<AreaType>(t);
         if (!IsAreaTypeImplemented(type))
            continue;
         const string label = string(AreaTypeIcon(type)) + "  " + AreaTypeName(type);
         if (ImGui::Selectable(label.c_str(), type == node->type))
            SetAreaEditor(leafId, type);
      }
      if (m_layout.CanClose(leafId))
      {
         ImGui::Separator();
         if (ImGui::Selectable(ICON_LC_X "  Close Area"))
         {
            PendingAreaOp op;
            op.kind = PendingAreaOp::Kind::Close;
            op.leaf = leafId;
            QueueAreaOp(op);
         }
      }
      ImGui::EndPopup();
   }
}

void EditorUI::RenderSplitters()
{
   // Divider strips between the areas (Blender: drag to resize, right click for the area options). They are
   // small windows created after the panes, kept above them, and give way to any open popup
   const float dpi = m_liveUI.GetDPI();
   const bool interactive = CanUseAreaTools() && m_areaModal == AreaModalKind::None && !m_areaDrag.active;
   const bool raiseNow = ShouldRaiseAreaWindows();
   // A seam line is drawn only between areas that are not the 3D view: the panes have no border of their own, while the
   // render fills the viewport area
   const auto &nodes = m_layout.Nodes();
   const auto hasViewport = [&nodes](const auto &self, const int index) -> bool
   {
      const AreaNode &node = nodes[index];
      return node.isSplit ? (self(self, node.child[0]) || self(self, node.child[1])) : IsViewportType(node.type);
   };
   for (const SplitterRect &s : m_splitterRects)
   {
      char name[40];
      snprintf(name, sizeof(name), "##Split%d", s.nodeId);
      ImGui::SetNextWindowPos(ImVec2(s.grab.x0, s.grab.y0));
      ImGui::SetNextWindowSize(ImVec2(s.grab.Width(), s.grab.Height()));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.f, 1.f));
      ImGui::Begin(name, nullptr,
         ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | (interactive ? 0 : ImGuiWindowFlags_NoInputs));
      if (raiseNow)
         ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
      ImGui::InvisibleButton("##grab", ImVec2(s.grab.Width(), s.grab.Height()));
      const bool hovered = ImGui::IsItemHovered() || m_splitDragNode == s.nodeId;
      if (hovered)
         ImGui::SetMouseCursor(s.axis == SplitAxis::Vertical ? ImGuiMouseCursor_ResizeEW : ImGuiMouseCursor_ResizeNS);
      ImDrawList *const dl = ImGui::GetWindowDrawList();
      const ImVec2 a = s.axis == SplitAxis::Vertical ? ImVec2(s.pos, s.bounds.y0) : ImVec2(s.bounds.x0, s.pos);
      const ImVec2 b = s.axis == SplitAxis::Vertical ? ImVec2(s.pos, s.bounds.y1) : ImVec2(s.bounds.x1, s.pos);
      const AreaNode *const splitNode = m_layout.Find(s.nodeId);
      if (splitNode != nullptr && !hasViewport(hasViewport, static_cast<int>(splitNode - nodes.data())))
         dl->AddLine(a, b, EditorTheme::Border, 1.f);
      if (hovered)
         dl->AddLine(a, b, EditorTheme::Accent, 2.f);
      if (ImGui::IsItemActivated())
      {
         m_splitDragNode = s.nodeId;
         m_splitDragPos = s.pos;
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
      {
         m_edgeMenuRequest = true;
         m_edgeMenuPos = ImGui::GetMousePos();
         m_edgeMenuNode = s.nodeId;
      }
      ImGui::End();
      ImGui::PopStyleVar(2);
   }
   if (m_splitDragNode >= 0)
   {
      if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
         m_splitDragNode = -1;
         SaveAreaLayoutSettings(); // The dragged sizes are persisted like any other layout change
      }
      else
         for (const SplitterRect &s : m_splitterRects)
            if (s.nodeId == m_splitDragNode)
            {
               const float pos = s.axis == SplitAxis::Vertical ? ImGui::GetMousePos().x : ImGui::GetMousePos().y;
               m_splitDragPos = clamp(pos, s.minPos, s.maxPos);
               m_layout.SetSplitSize(s.nodeId, s.SizeForPos(m_splitDragPos, dpi));
               break;
            }
   }
   RenderAreaCorners();
   RenderAreaEdgeMenu();
   RenderAreaModal();
}

void EditorUI::CancelAreaInteraction(const bool consumeEscape)
{
   const bool wasActive = m_areaDrag.active || m_areaModal != AreaModalKind::None;
   m_areaDrag = AreaDrag();
   m_areaModal = AreaModalKind::None;
   m_areaModalNode = -1;
   m_edgeMenuNode = -1; // The menu closes itself when its node is gone
   m_edgeMenuRequest = false;
   if (consumeEscape)
      m_areaEscPending = true; // The Esc that cancelled is not an editor Esc (it would deselect)
   if (wasActive)
      ImGui::SetWindowFocus(nullptr); // The zone or catcher window must not keep the keyboard focus away from the viewport keys
}

void EditorUI::RenderAreaCorners()
{
   // Corner action zones (Blender): a small square flush in each corner of each visible area. Dragging from one into the same area splits it, into
   // another one joins them. They are created after the splitter strips and kept above them, and give way to any open popup
   if (!CanUseAreaTools() || m_areaModal != AreaModalKind::None)
   {
      if (m_areaDrag.active)
         CancelAreaInteraction(false);
      return;
   }
   if (!m_areaDrag.active && ImGui::IsPopupOpen(static_cast<ImGuiID>(0), ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
      return;
   const float dpi = m_liveUI.GetDPI();
   const bool raiseNow = ShouldRaiseAreaWindows();
   const vector<LeafRect> leaves = m_leafRects;
   for (const LeafRect &leaf : leaves)
   {
      if (!leaf.visible)
         continue;
      // The pane sits inside a thin gap (see RenderAreas): the corner zones follow the pane edge
      AreaRect paneRect = leaf.rect;
      const float gap = 2.f * dpi;
      paneRect.x0 += gap;
      paneRect.y0 += gap;
      paneRect.x1 -= gap;
      paneRect.y1 -= gap;
      const float fullSide = min(14.f * dpi, 0.4f * min(paneRect.Width(), paneRect.Height()));
      if (fullSide < 6.f)
         continue;
      for (int corner = 0; corner < 4; ++corner)
      {
         const float side = fullSide;
         const float x = (corner & 1) ? paneRect.x1 - side : paneRect.x0;
         const float y = (corner & 2) ? paneRect.y1 - side : paneRect.y0;
         // Triangular hit zone: the half of the square touching the area's corner (Blender). Outside it the window takes no input, so the header
         // buttons and scrollbars next to the corner keep their clicks
         const ImVec2 mouse = ImGui::GetIO().MousePos;
         const float lx = (corner & 1) ? x + side - mouse.x : mouse.x - x; // Distance from the corner along x / y
         const float ly = (corner & 2) ? y + side - mouse.y : mouse.y - y;
         char name[40];
         snprintf(name, sizeof(name), "##Corner%d_%d", leaf.id, corner); // Keyed by the stable node id
         // A widget under the pointer (header search field, buttons, scrollbar ends) keeps the click: ImGui's hovered item of the last frame, which is
         // the corner's own button while the zone is the one hovered
         ImGuiWindow *const cornerWindow = ImGui::FindWindowByName(name);
         const ImGuiID hoveredId = ImGui::GetHoveredID();
         ImGuiWindow *const hw = ImGui::GetCurrentContext()->HoveredWindow;
         const char *const hwName = hw != nullptr ? hw->RootWindow->Name : "";
         // The viewport's scene item is the pane's background, not a widget: it must not block the corner
         const auto viewIt = m_areaViews.find(leaf.id);
         const auto *const viewPane = viewIt != m_areaViews.end() ? dynamic_cast<const ViewportPane *>(viewIt->second.get()) : nullptr;
         const bool overViewItem = viewPane != nullptr && hoveredId == viewPane->m_viewItemId;
         // The corner button's ID is hashed from the window ID: a corner window left unused while a popup stayed open has its ID stack released by
         // ImGui's memory compaction (after ConfigMemoryCompactTimer), and ImGuiWindow::GetID would then read before its start
         bool overWidget = hoveredId != 0 && !overViewItem && (cornerWindow == nullptr || hoveredId != ImGui::GetIDWithSeed("##corner", nullptr, cornerWindow->ID));
         const bool overStrip = strncmp(hwName, "##Split", 7) == 0;
         const bool overArea = hw == nullptr || strncmp(hwName, "##Pane", 6) == 0 || strcmp(hwName, "text overlay") == 0;
         // A floating window (popup, tooltip, overlay) keeps all its clicks: no corner acts through it
         const bool overFloating = !overArea && !overStrip && strncmp(hwName, "##Corner", 8) != 0;
         if (overStrip)
            overWidget = false; // The splitter strip overlaps the corner's edge band: the corner wins inside its triangle
         else if (overWidget && (corner & 2) != 0 && overArea && hw != nullptr)
            overWidget = hoveredId == ImGui::GetWindowScrollbarID(hw, ImGuiAxis_X)
               || hoveredId == ImGui::GetWindowScrollbarID(hw, ImGuiAxis_Y); // Bottom, on an area window: list rows do not count, only scrollbars
         if (overFloating)
            overWidget = true;
         const bool inTriangle = lx >= 0.f && ly >= 0.f && lx + ly <= side && (!overWidget || m_areaDrag.active);
         ImGui::SetNextWindowPos(ImVec2(x, y));
         ImGui::SetNextWindowSize(ImVec2(side, side));
         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
         ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.f, 1.f));
         ImGui::Begin(name, nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
               | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ((inTriangle || m_areaDrag.active) ? 0 : ImGuiWindowFlags_NoInputs));
         if (raiseNow)
            ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());
         ImGui::InvisibleButton("##corner", ImVec2(side, side));
         const bool activated = ImGui::IsItemActivated() && inTriangle;
         const ImVec2 origin((corner & 1) ? x + side : x, (corner & 2) ? y + side : y); // The area's corner
         const float sx = (corner & 1) ? -1.f : 1.f, sy = (corner & 2) ? -1.f : 1.f;
         const bool hot = ImGui::IsItemHovered() && inTriangle && !m_areaDrag.active;
         ImDrawList *const dl = ImGui::GetWindowDrawList(); // Under tooltips and floating windows
         if (hot)
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
         if (!m_areaDrag.active)
         {
            // Permanent hatch (two diagonal strokes), brighter while hovered
            const ImU32 hatch = hot ? EditorTheme::Text : ((EditorTheme::TextDim & ~IM_COL32_A_MASK) | (90u << IM_COL32_A_SHIFT));
            for (const float f : { 0.85f, 0.45f })
               dl->AddLine(ImVec2(origin.x + sx * side * f, origin.y), ImVec2(origin.x, origin.y + sy * side * f), hatch, hot ? 1.5f : 1.f);
         }
         ImGui::End();
         ImGui::PopStyleVar(2);
         if (activated && !m_areaDrag.active)
         {
            m_areaDrag = AreaDrag();
            m_areaDrag.active = true;
            m_areaDrag.leaf = leaf.id;
            m_areaDrag.corner = corner;
            m_areaDrag.start = ImGui::GetMousePos();
         }
      }
   }
   UpdateAreaDrag();
}

void EditorUI::UpdateAreaDrag()
{
   if (!m_areaDrag.active)
      return;
   if (FindLeafRect(m_areaDrag.leaf) == nullptr)
   {
      CancelAreaInteraction(false); // The area is gone
      return;
   }
   const bool escape = ImGui::IsKeyPressed(ImGuiKey_Escape, false);
   if (escape || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
   {
      CancelAreaInteraction(escape);
      return;
   }
   const ImVec2 mouse = ImGui::GetIO().MousePos;
   const float dx = mouse.x - m_areaDrag.start.x, dy = mouse.y - m_areaDrag.start.y;
   const float threshold = 8.f * m_liveUI.GetDPI();
   if (!m_areaDrag.decided && dx * dx + dy * dy >= threshold * threshold)
      m_areaDrag.decided = true;
   const bool released = !ImGui::IsMouseDown(ImGuiMouseButton_Left);
   ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
   if (m_areaDrag.decided)
   {
      // Inside the area: split (the axis follows the dominant movement for the first 20 px, then locks; the other axis when the chosen one cannot fit). Inside another area: join, the source takes over
      const int under = AreaLayout::LeafAt(m_leafRects, mouse.x, mouse.y);
      if (under == m_areaDrag.leaf)
      {
         // Blender: the axis follows the dominant direction of the drag until it has gone ~20 px, then it is kept
         if (!m_areaDrag.axisChosen)
         {
            m_areaDrag.axis = fabsf(dx) > fabsf(dy) ? SplitAxis::Vertical : SplitAxis::Horizontal;
            const float lockDistance = 20.f * m_liveUI.GetDPI();
            m_areaDrag.axisChosen = dx * dx + dy * dy >= lockDistance * lockDistance;
         }
         const auto sideOf
            = [this](const SplitAxis a) { return a == SplitAxis::Vertical ? (m_areaDrag.corner & 1) != 0 : (m_areaDrag.corner & 2) != 0; }; // The new area is on the side of the start corner
         AreaSplitPlan plan;
         PlanAreaSplit(m_areaDrag.leaf, m_areaDrag.axis, sideOf(m_areaDrag.axis), mouse, plan);
         if (!plan.ok)
         {
            // The chosen axis does not fit: the other one is used when it does
            const SplitAxis other = m_areaDrag.axis == SplitAxis::Vertical ? SplitAxis::Horizontal : SplitAxis::Vertical;
            AreaSplitPlan alt;
            if (PlanAreaSplit(m_areaDrag.leaf, other, sideOf(other), mouse, alt))
            {
               plan = alt;
               m_areaDrag.axis = other;
            }
         }
         const bool newOnSecondSide = sideOf(m_areaDrag.axis);
         DrawAreaSplitPreview(plan);
         if (released)
         {
            if (plan.ok)
            {
               PendingAreaOp op;
               op.kind = PendingAreaOp::Kind::Split;
               op.leaf = m_areaDrag.leaf;
               op.axis = m_areaDrag.axis;
               op.newOnSecondSide = newOnSecondSide;
               op.size = plan.size;
               QueueAreaOp(op);
            }
            else
               Report(plan.reason);
         }
      }
      else if (under >= 0)
      {
         const bool ok = m_layout.CanJoin(m_areaDrag.leaf, under);
         DrawAreaJoinPreview(m_areaDrag.leaf, under, ok);
         if (released)
         {
            if (ok)
            {
               PendingAreaOp op;
               op.kind = PendingAreaOp::Kind::Join;
               op.leaf = m_areaDrag.leaf;
               op.leaf2 = under;
               QueueAreaOp(op);
            }
            else
               Report(AreaJoinRefusal(m_areaDrag.leaf, under));
         }
      }
   }
   if (released)
   {
      m_areaDrag = AreaDrag();
      ImGui::SetWindowFocus(nullptr);
   }
}

void EditorUI::RenderAreaEdgeMenu()
{
   // Right click on a divider. The popup is opened and drawn from a window of its own (the strip windows are gone when the menu is shown)
   ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
   ImGui::SetNextWindowSize(ImVec2(1.f, 1.f));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.f, 1.f));
   ImGui::Begin("##AreaEdgeMenuHost", nullptr,
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
         | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);
   if (m_edgeMenuRequest)
   {
      m_edgeMenuRequest = false;
      ImGui::OpenPopup("##AreaEdgeMenu");
   }
   ImGui::SetNextWindowPos(m_edgeMenuPos, ImGuiCond_Appearing);
   if (ImGui::BeginPopup("##AreaEdgeMenu"))
   {
      const AreaNode *const split = m_layout.Find(m_edgeMenuNode);
      if (split == nullptr || !split->isSplit || m_areaModal != AreaModalKind::None || !CanUseAreaTools())
         ImGui::CloseCurrentPopup(); // The edge is gone (or something else took over)
      else
      {
         if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
         {
            ImGui::CloseCurrentPopup();
            m_areaEscPending = true;
         }
         const auto &nodes = m_layout.Nodes();
         const AreaNode &first = nodes[split->child[0]], &second = nodes[split->child[1]];
         const bool canJoin = !first.isSplit && !second.isSplit && !first.hidden && !second.hidden;
         const bool canSplit = m_layout.LeafCount() < AreaLayout::kMaxLeaves;
         ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
         ImGui::TextUnformatted("Area Options");
         ImGui::PopStyleColor();
         ImGui::Separator();
         const auto startMode = [this](const AreaModalKind kind, const SplitAxis axis)
         {
            m_areaModal = kind;
            m_areaModalAxis = axis;
            m_areaModalNode = m_edgeMenuNode;
            m_areaModalFocus = true;
         };
         if (ImGui::MenuItem("Vertical Split", nullptr, false, canSplit))
            startMode(AreaModalKind::Split, SplitAxis::Vertical);
         if (ImGui::MenuItem("Horizontal Split", nullptr, false, canSplit))
            startMode(AreaModalKind::Split, SplitAxis::Horizontal);
         if (ImGui::MenuItem("Join Areas", nullptr, false, canJoin))
            startMode(AreaModalKind::Join, SplitAxis::Vertical);
         if (ImGui::MenuItem("Swap Areas"))
         {
            PendingAreaOp op;
            op.kind = PendingAreaOp::Kind::Swap;
            op.leaf = m_edgeMenuNode;
            QueueAreaOp(op);
         }
      }
      ImGui::EndPopup();
   }
   ImGui::End();
   ImGui::PopStyleVar(2);
}

void EditorUI::RenderAreaModal()
{
   // Interactive split / join started from the edge menu: a transparent window over the whole display takes the mouse (no click reaches a panel),
   // the preview follows the mouse and a click confirms. Esc or right click cancels
   if (m_areaModal == AreaModalKind::None)
      return;
   const AreaNode *const split = m_layout.Find(m_areaModalNode);
   if (split == nullptr || !split->isSplit || !CanUseAreaTools())
   {
      CancelAreaInteraction(false);
      return;
   }
   if (ImGui::GetTopMostPopupModal() != nullptr)
   {
      CancelAreaInteraction(false); // A modal dialog (discard changes,...) opened: it must stay clickable
      return;
   }
   const bool escape = ImGui::IsKeyPressed(ImGuiKey_Escape, false);
   if (escape || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
   {
      CancelAreaInteraction(escape);
      return;
   }
   const ImGuiIO &io = ImGui::GetIO();
   if (m_areaModalFocus)
   {
      ImGui::SetNextWindowFocus(); // On top of everything for the length of the mode
      m_areaModalFocus = false;
   }
   ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
   ImGui::SetNextWindowSize(io.DisplaySize);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(1.f, 1.f));
   ImGui::Begin(
      "##AreaModalCatcher", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNav);
   ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow()); // Every frame: nothing (re-raised strips, popups) may end up above it
   ImGui::InvisibleButton("##catch", io.DisplaySize);
   const bool clicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
   ImGui::End();
   ImGui::PopStyleVar(2);
   ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

   const ImVec2 mouse = io.MousePos;
   const int under = AreaLayout::LeafAt(m_leafRects, mouse.x, mouse.y);
   bool done = false;
   if (m_areaModal == AreaModalKind::Split)
   {
      if (under >= 0)
      {
         AreaSplitPlan plan;
         PlanAreaSplit(under, m_areaModalAxis, true, mouse, plan); // The new area is the right / bottom part
         DrawAreaSplitPreview(plan);
         if (clicked)
         {
            done = true;
            if (plan.ok)
            {
               PendingAreaOp op;
               op.kind = PendingAreaOp::Kind::Split;
               op.leaf = under;
               op.axis = m_areaModalAxis;
               op.newOnSecondSide = true;
               op.size = plan.size;
               QueueAreaOp(op);
            }
            else
               Report(plan.reason);
         }
      }
   }
   else
   {
      // The area not under the mouse is consumed by the other one
      const auto &nodes = m_layout.Nodes();
      const int id0 = static_cast<int>(nodes[split->child[0]].id), id1 = static_cast<int>(nodes[split->child[1]].id);
      const int keep = under == id0 ? id0 : under == id1 ? id1 : -1;
      if (keep >= 0)
      {
         const int consume = keep == id0 ? id1 : id0;
         const bool ok = m_layout.CanJoin(keep, consume);
         DrawAreaJoinPreview(keep, consume, ok);
         if (clicked)
         {
            done = true;
            if (ok)
            {
               PendingAreaOp op;
               op.kind = PendingAreaOp::Kind::Join;
               op.leaf = keep;
               op.leaf2 = consume;
               QueueAreaOp(op);
            }
            else
               Report(AreaJoinRefusal(keep, consume));
         }
      }
      else if (clicked)
         done = true;
   }
   if (done)
      CancelAreaInteraction(false);
}

void EditorUI::ShowAreaEditor(const AreaType type)
{
   PendingAreaOp op {};
   op.kind = PendingAreaOp::Kind::Show;
   op.type = type;
   m_pendingAreaOps.push_back(op);
}

// ---------------------------------------------------------------------------------------------------------------------
// Viewport panes

ViewportPane *EditorUI::HoveredViewportPane() const { return m_hoverVp; }

Vertex2D EditorUI::TablePosAt(const ImVec2 &pos) const
{
   // Display-space position to table XY through the view pane under it (0,0 when none)
   const int leaf = AreaLayout::LeafAt(m_leafRects, pos.x, pos.y);
   if (leaf >= 0)
   {
      const auto it = m_areaViews.find(leaf);
      if (it != m_areaViews.end() && it->second != nullptr)
      {
         if (const auto *const bp = dynamic_cast<BlueprintPane *>(it->second.get()); bp != nullptr)
            return bp->Unproject(pos);
         if (const auto *const vp = dynamic_cast<ViewportPane *>(it->second.get()); vp != nullptr)
            return vp->UnprojectToPlane(pos, 0.f);
      }
   }
   return Vertex2D(0.f, 0.f);
}

ViewportPane *EditorUI::GetTargetViewport() const
{
   // The pane that owns viewport actions: the flying pane (its view covers the window), else the
   // one captured by an in-progress mouse drag, else the one under the mouse, else the first one.
   // A Render Preview pane is display-only: it is never the target.
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()); vp != nullptr && vp->m_viewType != ViewMode::PreviewCam && vp->m_flyMode)
         return vp;
   // A pane running a modal transform op owns the viewport actions wherever the mouse is
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()); vp != nullptr && vp->m_viewType != ViewMode::PreviewCam && vp->m_op.op != 0)
         return vp;
   if (m_captureVp != nullptr && m_captureVp->m_viewType != ViewMode::PreviewCam)
      return m_captureVp;
   if (m_hoverVp != nullptr && m_hoverVp->m_viewType != ViewMode::PreviewCam)
      return m_hoverVp;
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()); vp != nullptr && vp->m_viewType != ViewMode::PreviewCam)
         return vp;
   return nullptr;
}
}
