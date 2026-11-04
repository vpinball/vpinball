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

#include "editor/EditorUIPart.h"
#include "editor/EditorUIPartRegistry.h"
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
#include "renderer/Shader.h"
#include "renderer/VRDevice.h"

#include "ui/EditorClipboard.h"
#include "ui/VPXFileFeedback.h"
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

// Vertical field of view (in degrees) of the perspective editor camera
constexpr float editorCamFovY = 39.6f;


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

   // Editor camera position. We use a right handed system for easy ImGuizmo integration while VPX renderer is left handed, so reverse X axis
   m_camDistance = m_table->m_bottom * 0.7f;
   const vec3 eye(m_table->m_right * 0.5f, m_table->m_bottom * 0.5f, -m_camDistance);
   const vec3 at(m_table->m_right * 0.5f, m_table->m_bottom * 0.5f, 0.f);
   constexpr vec3 up { 0.f, -1.f, 0.f };
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
   ImGuizmo::AllowAxisFlip(false);
}

EditorUI::~EditorUI()
{
   // The table outlives the editor (Player deletes the UI first)
   m_fileOp.reset(); // Wait for a file operation in progress, its result being discarded with the session
   RestorePartsVisibility();
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
   m_boxSelectActive = false;
   m_renderer->DisableStaticPrePass(true);
   m_player->SetPlayState(false); // Suspend play while editing

   // Drop input events queued while the editor was closed (e.g. the key press that ended a play test), so they don't trigger editor shortcuts
   ImGui::GetIO().ClearEventsQueue();
   ImGui::GetIO().ClearInputKeys();

   if (IsInspectMode())
   {
      // Inspecting a live copy (play testing): keep the game playing, use the normal player camera view with default shading
      m_camMode = ViewMode::PreviewCam;
      m_shadeMode = Renderer::ShadeMode::Default;
   }
   else
   {
      // Enter with the user camera setup as an orthographic top view fitted on the playfield bounds, rendered as masked wireframe
      m_camMode = ViewMode::EditorCam;
      m_perspectiveCam = false;
      m_predefinedView = PredefinedView::Top;
      m_shadeMode = Renderer::ShadeMode::Wireframe;
      // The playfield fit needs a valid display size and chrome height which may not be known yet (editor opened during startup): defer to the first rendered frame
      m_fitPlayfieldCamera = true;
   }
}

void EditorUI::Close()
{
   if (!m_isOpened)
      return;
   m_isOpened = false;
   m_flyMode = false;
   ExitPointEditMode(false);
   RestorePartsVisibility();
   m_inspectionModal.Close();
   m_partLibrary.Close();
   m_scriptPanel.Close();
   m_renderer->DisableStaticPrePass(false);
   m_renderer->SetShadeMode(Renderer::ShadeMode::Default);
   m_renderer->GetPrimaryView()->m_renderScene = true; // The primary view renders the scene again once the editor areas are gone
}

void EditorUI::SetTable(PinTable *const table)
{
   assert(table != nullptr);
   // Only tables of a same base table / live copy pair are supported (guaranteed by the player's SetTable)
   PinTable *const baseTable = m_table->m_liveBaseTable ? m_table->m_liveBaseTable : m_table;
   assert(table == baseTable || table->m_liveBaseTable == baseTable);
   m_table = table;

   // Drop all the state bound to the previous table's editables (undo is bound to the base table which outlives the switch)
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
   m_savedOutlinerAnchor = nullptr;
   ClearSelection();
   m_dynamicParts.clear();
   m_editables.clear();
   m_editableMap.clear();
   m_partGroupUseHistory.clear();
   m_lastUndoPart = nullptr;
   m_addPartType = eItemInvalid;
   m_partLibrary.CancelPlacement();
   m_boxSelectActive = false;
}

void EditorUI::PlayTest()
{
   if (IsInspectMode())
      return;
   // The editor visibility overrides must not leak into the duplicated table used for the play session
   RestorePartsVisibility();
   PinTable *const liveTable = m_table->CopyForPlay();
   m_player->SetTable(liveTable, Player::TableTransition::Stack);
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
      m_pendingClose.reset();
      m_confirmLoadTable = true;
      return;
   }
   ShowLoadTableDialog(folder);
}

void EditorUI::RequestClose(const int closeState)
{
   // Only closing a full editor session loses the edited table (a live edit started from the Win32 editor or a play test gets back to its editor)
   if (m_player->IsEditorMode() && !IsInspectMode() && m_table->GetDirtyState() != SaveDirtyState::Clean)
   {
      m_pendingNewTable.reset();
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
            g_settingsService.GetAppSettings().SetRecentDir_LoadDir(PathToString(op->filename.parent_path()), false);
         PinTable *const table = op->table;
         op->table = nullptr;
         m_player->SetTable(table, Player::TableTransition::Replace); // The loaded table reference is adopted by the player
      }
      else
         m_liveUI.PushNotification(op->type == AsyncFileOp::Type::NewTable ? "Failed to create new table from '"s + op->displayPath : "Failed to load table '"s + op->displayPath, 10000);
      break;

   case AsyncFileOp::Type::Save:
      // Saves must always report their result: a failed save was previously silent (and looked like a successful one)
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
   if (m_fileOp == nullptr)
      return;
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

void EditorUI::ResetCameraFromPlayer()
{
   // Try to setup editor camera to match the used one, but only mostly since the EditorUI does not have some view setup features like off-center, ...
   m_camView = Matrix3D::MatrixScale(1.f, 1.f, -1.f) * m_renderer->GetMVP().GetView(0) * Matrix3D::MatrixScale(1.f, -1.f, 1.f);
}

void EditorUI::SetBackdropCamera()
{
   // Setup the editor camera to frame the desktop backdrop (a flat rectangle in EDITOR_BG_WIDTH x EDITOR_BG_HEIGHT coordinates, Y axis going down)
   const ImGuiIO &io = ImGui::GetIO();
   const float aspect = m_editVp != nullptr && m_editVp->m_imageRect.Height() > 0.f ? m_editVp->m_imageRect.Width() / m_editVp->m_imageRect.Height() : io.DisplaySize.x / io.DisplaySize.y;
   m_camDistance = 0.5f * max((float)EDITOR_BG_HEIGHT, (float)EDITOR_BG_WIDTH / aspect);
   const vec3 eye((float)EDITOR_BG_WIDTH * 0.5f, (float)EDITOR_BG_HEIGHT * 0.5f, -m_camDistance);
   const vec3 at((float)EDITOR_BG_WIDTH * 0.5f, (float)EDITOR_BG_HEIGHT * 0.5f, 0.f);
   constexpr vec3 up { 0.f, -1.f, 0.f };
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
}

Vertex2D EditorUI::GetUIVisibleFraction() const
{
   // When the editor UI is not displayed (fly mode, inspection modal), nothing is drawn over the render
   if (m_flyMode || m_inspectionModal.IsVisible())
      return Vertex2D(1.f, 1.f);
   // A viewport pane fits its content in its own rect: no chrome margin applies
   if (m_editVp != nullptr)
      return Vertex2D(1.f, 1.f);
   // Otherwise, the top chrome (and the side panes, unless the table is locked) are drawn over the render:
   // evaluate the remaining visible fraction, applied as a symmetric margin so that fitted content stays centered
   const ImGuiIO &io = ImGui::GetIO();
   const float marginX = m_table->IsLocked() ? 0.f : max(OutlinerPanel::PaneWidth, PropertiesPanel::PaneWidth) * m_liveUI.GetDPI();
   const float marginY = m_chrome.GetTopBarHeight();
   return Vertex2D(max(0.1f, 1.f - 2.f * marginX / io.DisplaySize.x), max(0.1f, 1.f - 2.f * marginY / io.DisplaySize.y));
}

void EditorUI::SetPlayfieldCamera()
{
   // Setup the editor camera to frame the playfield bounds from a top view (table XY plane, Y axis going down)
   const ImGuiIO &io = ImGui::GetIO();
   const Vertex2D visible = GetUIVisibleFraction();
   const float aspect = m_editVp != nullptr && m_editVp->m_imageRect.Height() > 0.f ? m_editVp->m_imageRect.Width() / m_editVp->m_imageRect.Height() : io.DisplaySize.x / io.DisplaySize.y;
   m_camDistance = 0.5f * max(m_table->m_bottom / visible.y, m_table->m_right / (aspect * visible.x));
   const vec3 eye(m_table->m_right * 0.5f, m_table->m_bottom * 0.5f, -m_camDistance);
   const vec3 at(m_table->m_right * 0.5f, m_table->m_bottom * 0.5f, 0.f);
   constexpr vec3 up { 0.f, -1.f, 0.f };
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
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
   // The view being recorded may belong to a viewport pane: its own camMode and shading apply
   // to the 3D overlays (each pane draws them with its own camera and render setup)
   ViewMode camMode = m_camMode;
   Renderer::ShadeMode shadeMode = m_shadeMode;
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()); vp != nullptr && vp->GetView() == m_renderer->GetActiveView())
      {
         camMode = vp == m_editVp ? m_camMode : vp->m_camMode;
         shadeMode = vp == m_editVp ? m_shadeMode : vp->m_shadeMode;
         break;
      }
   LiveRenderContext ctx(m_player, nullptr, camMode, shadeMode, (m_table->m_liveBaseTable != nullptr) && m_player->IsPlaying());
   for (const auto &uiPart : m_editables)
   {
      if ((camMode == ViewMode::DesktopBackdrop && !uiPart->GetEditable()->m_desktopBackdrop) || (camMode != ViewMode::DesktopBackdrop && uiPart->GetEditable()->m_desktopBackdrop))
         continue;
      uiPart->Render(ctx);
   }
   m_partLibrary.Render3D();
}

void EditorUI::RenderUI()
{
   const ImGuiIO &io = ImGui::GetIO();
   ImGuizmo::SetOrthographic(m_camMode == ViewMode::DesktopBackdrop || (m_camMode == ViewMode::EditorCam && !m_perspectiveCam));
   ImGuizmo::BeginFrame();
   const AreaRect gizmoRect = m_editVp != nullptr ? m_editVp->m_imageRect : AreaRect { 0.f, 0.f, io.DisplaySize.x, io.DisplaySize.y };
   ImGuizmo::SetRect(gizmoRect.x0, gizmoRect.y0, gizmoRect.Width(), gizmoRect.Height());

   // In desktop backdrop mode, only backdrop parts can be selected and edited, and conversely
   {
      const bool backdropMode = m_camMode == ViewMode::DesktopBackdrop;
      std::erase_if(m_multiSel, [backdropMode](const std::shared_ptr<EditorUIPart> &part) { return part->GetEditable()->m_desktopBackdrop != backdropMode; });
      if (m_selection.GetType() == Selection::S_EDITABLE && !IsPartSelected(m_selection.GetPart()))
         m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
      if (m_outlinerAnchor && m_outlinerAnchor->GetEditable()->GetItemType() != eItemPartGroup && m_outlinerAnchor->GetEditable()->m_desktopBackdrop != backdropMode)
         m_outlinerAnchor.reset();
   }

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

   const bool showFullUI = !m_inspectionModal.IsVisible() && !m_flyMode;
   m_renderer->GetPrimaryView()->m_renderScene = !showFullUI; // While areas are drawn, the scene lives in the viewport panes
   UpdateAreaLayout(); // Area rects are computed before the chrome draws the status overlay into the viewport area
   m_chrome.Render(showFullUI);
   if (showFullUI)
   {
      // Area panes, then their splitters and corner zones (above the pane windows)
      RenderAreas();
      RenderSplitters();
   }
   else
   {
      // The areas are hidden: the viewport panes must not draw their scene over the primary view's one
      for (const auto &[id, view] : m_areaViews)
         if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()))
            vp->SetRendering(false);
   }
   m_inspectionModal.Render();
   m_partLibrary.Render();
   m_scriptPanel.Render();

#endif

   // Deferred playfield fit on editor opening (display size and chrome height are valid once the UI has been rendered)
   if (m_fitPlayfieldCamera && io.DisplaySize.x > 0.f && io.DisplaySize.y > 0.f)
   {
      m_fitPlayfieldCamera = false;
      SetPlayfieldCamera();
   }

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
   const char *const confirmTitle = m_pendingClose ? "Quit###ConfirmDiscard" : m_pendingNewTable ? "New Table###ConfirmDiscard" : "Load Table###ConfirmDiscard";
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
         else
            ShowLoadTableDialog(m_pendingLoadFolder);
      }
      ImGui::SameLine();
      ImGui::SetItemDefaultFocus();
      if (ImGui::Button("Cancel"))
      {
         m_pendingNewTable.reset();
         m_pendingClose.reset();
         ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
   }

   // Progress feedback of the asynchronous file operation (load/save run on a worker thread): a modal popup
   // that blocks all interaction while the operation is running
   RenderFileOpModal();

   ImGui::End();
   ImGui::PopStyleVar();
   ImGui::PopStyleColor(2);

   // Update editor camera (the state of the active viewport pane, or the fullscreen camera when no pane is bound)
   RenderView *const camOut = m_editVp != nullptr && m_editVp->GetView() != nullptr ? m_editVp->GetView() : m_renderer->GetPrimaryView();
   const float camAspect = m_editVp != nullptr && m_editVp->m_imageRect.Height() > 0.f ? m_editVp->m_imageRect.Width() / m_editVp->m_imageRect.Height() : io.DisplaySize.x / io.DisplaySize.y;
   const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
   const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
   if (m_camMode == ViewMode::PreviewCam)
   {
      camOut->InitLayout();
      m_camView = RH2LH * camOut->GetMVP().GetView(0) * YAxis;
      m_camProj = YAxis * camOut->GetMVP().GetProj(0);
   }
   else
   {
      if (m_perspectiveCam && m_camMode != ViewMode::DesktopBackdrop) // The desktop backdrop is a 2D view, always edited with an orthographic camera
      {
         // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
         // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
         //const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
         //const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, -1.f);
         //float zNear, zFar;
         //m_table->ComputeNearFarPlane(RH2LH * m_camView * YAxis, 1.f, zNear, zFar);
         // Near plane follows the orbit distance (a fixed 5 left a 10000:1 range, so near-coplanar surfaces z-fought when zoomed out)
         const float zNear = clamp(m_camDistance * 0.05f, 5.f, 250.f);
         constexpr float zFar = 50000.f;
         m_camProj = Matrix3D::MatrixPerspectiveFovRH(editorCamFovY, camAspect, zNear, zFar);
      }
      else
      {
         constexpr float zNear = 0.5f;
         constexpr float zFar = 50000.f;
         const float viewHeight = m_camDistance;
         const float viewWidth = viewHeight * camAspect;
         m_camProj = Matrix3D::MatrixOrthoOffCenterRH(-viewWidth, viewWidth, -viewHeight, viewHeight, zNear, -zFar);
      }

      // Apply editor camera to renderer (move view/projection from right handed to left handed)
      // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
      const Matrix3D view = RH2LH * m_camView * YAxis;
      const Matrix3D proj = YAxis * m_camProj;
      camOut->SetViewProj(view, proj);
   }

   // Selection manipulator
   Matrix3D transform;
   const bool isSelectionTransformValid = GetSelectionTransform(transform);
   if (isSelectionTransformValid && !m_table->IsLocked())
   {
      float camViewLH[16];
      memcpy(camViewLH, &m_camView.m[0][0], sizeof(float) * 4 * 4);
      for (int i = 8; i < 12; i++)
         camViewLH[i] = -camViewLH[i];
      const Matrix3D prevTransform(transform);
      ImGuizmo::OPERATION gizmoOperation = m_gizmoOperation;
      if (m_pointEditPart || m_camMode == ViewMode::DesktopBackdrop)
      {
         // Drag point curves are 2D in the table XY plane, and backdrop parts are 2D in the backdrop XY plane: restrict gizmo operations to this plane
         if (gizmoOperation == ImGuizmo::TRANSLATE)
            gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y);
         else if (gizmoOperation == ImGuizmo::ROTATE)
            gizmoOperation = ImGuizmo::ROTATE_Z;
         else if (gizmoOperation == ImGuizmo::SCALE)
            gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y);
      }
      else if (m_camMode == ViewMode::EditorCam)
      {
         // Predefined views are axis aligned 2D views: restrict gizmo operations to the view plane
         switch (m_predefinedView)
         {
         case EditorUI::PredefinedView::None: break;
         case EditorUI::PredefinedView::Left:
         case EditorUI::PredefinedView::Right:
            if (gizmoOperation == ImGuizmo::TRANSLATE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_Y | ImGuizmo::TRANSLATE_Z);
            else if (gizmoOperation == ImGuizmo::ROTATE)
               gizmoOperation = ImGuizmo::ROTATE_X;
            else if (gizmoOperation == ImGuizmo::SCALE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_Y | ImGuizmo::SCALE_Z);
            break;
         case EditorUI::PredefinedView::Top:
         case EditorUI::PredefinedView::Bottom:
            if (gizmoOperation == ImGuizmo::TRANSLATE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y);
            else if (gizmoOperation == ImGuizmo::ROTATE)
               gizmoOperation = ImGuizmo::ROTATE_Z;
            else if (gizmoOperation == ImGuizmo::SCALE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y);
            break;
         case EditorUI::PredefinedView::Front:
         case EditorUI::PredefinedView::Back:
            if (gizmoOperation == ImGuizmo::TRANSLATE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Z);
            else if (gizmoOperation == ImGuizmo::ROTATE)
               gizmoOperation = ImGuizmo::ROTATE_Y;
            else if (gizmoOperation == ImGuizmo::SCALE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Z);
            break;
         }
      }
      // A gizmo drag whose selection changed under the mouse (outliner click, undo,...) is over: the new selection must not follow it without an undo record
      const bool gizmoLost = m_gizmoDragActive && m_multiSel != m_gizmoDragSelection;
      if (gizmoLost)
      {
         ImGuizmo::Enable(false); // Clears ImGuizmo's using state
         ImGuizmo::Enable(true);
      }
      else
      {
         ImGuizmo::Manipulate(camViewLH, (float *)(m_camProj.m), gizmoOperation, m_gizmoMode, (float *)(transform.m));
         if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
         {
            // Mark all selected parts for undo once per drag (m_lastUndoId is reset when the gizmo is released)
            if (!m_gizmoDragActive && m_table->m_liveBaseTable == nullptr)
            {
               m_gizmoDragActive = true;
               m_gizmoDragSelection = m_multiSel;
               m_undo.BeginUndo();
               for (const auto &part : m_multiSel)
                  m_undo.MarkForUndo(part->GetEditable());
               m_undo.EndUndo();
               m_lastUndoPart = nullptr;
               m_lastUndoId = 0x1000;
            }
            SetSelectionTransform(transform);
         }
      }
   }
   else if (ImGuizmo::IsUsingAny())
   {
      // A drag interrupted with the mouse still held (invalid selection, table locked, Esc,...) is not manipulated anymore,
      // so ImGuizmo would stay in its using state: reset it (the drag's undo record stands)
      ImGuizmo::Enable(false);
      ImGuizmo::Enable(true);
   }
   // Reset gizmo undo deduplication once the gizmo is released (each drag should create a single undo entry)
   if (m_lastUndoId == 0x1000 && !ImGuizmo::IsUsing())
   {
      m_gizmoDragActive = false;
      m_gizmoDragSelection.clear();
      m_lastUndoPart = nullptr;
      m_lastUndoId = 0;
   }

   m_renderer->SetShadeMode(m_shadeMode);

   // Selection and physic colliders overlay (suspended while a save is running: the part renders
   // would write into the parts' data being serialized on the worker thread, see Render3D)
   {
      LiveRenderContext ctx(m_player, overlayDrawList, m_camMode, m_shadeMode, (m_table->m_liveBaseTable != nullptr) && m_player->IsPlaying());
      SetupPaneProjection(&ctx); // Selection overlays project through the edit viewport's camera, clipped to its rect
      const bool clipToPane = m_editVp != nullptr;
      if (clipToPane)
         overlayDrawList->PushClipRect(ImVec2(m_editVp->m_imageRect.x0, m_editVp->m_imageRect.y0), ImVec2(m_editVp->m_imageRect.x1, m_editVp->m_imageRect.y1), true);
      if (!m_multiSel.empty() && !IsSaveInProgress())
      {
         if (m_renderer->m_renderDevice->GetCurrentRenderTarget()->HasDepth())
            m_renderer->m_renderDevice->Clear(clearType::ZBUFFER, 0);
         ctx.m_isSelected = true;
         for (const auto &part : m_multiSel)
         {
            ctx.m_isActive = part == m_selection.GetPart();
            part->Render(ctx);
         }
         ctx.m_isActive = false;
         ctx.m_isSelected = false;
      }

      if (isSelectionTransformValid)
      {
         const ImVec2 pos = ctx.Project(transform.GetOrthoNormalPos());
         overlayDrawList->AddCircleFilled(pos, 3.f * m_liveUI.GetDPI(), IM_COL32(255, 255, 255, 255), 16);
      }

      // In desktop backdrop mode, draw the bounds of the backdrop area
      if (m_camMode == ViewMode::DesktopBackdrop)
      {
         const Vertex3Ds bounds[4] = { Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds((float)EDITOR_BG_WIDTH, 0.f, 0.f), Vertex3Ds((float)EDITOR_BG_WIDTH, (float)EDITOR_BG_HEIGHT, 0.f),
            Vertex3Ds(0.f, (float)EDITOR_BG_HEIGHT, 0.f) };
         for (int i = 0; i < 4; i++)
            ctx.DrawLine(bounds[i], bounds[(i + 1) % 4], IM_COL32(255, 255, 255, 64));
      }

      // In drag point edit mode, render the drag points of the edited part's curve (or its center point)
      if (m_pointEditPart)
      {
         if (m_pointEditCenter)
         {
            const ImVec2 pos = ctx.Project(m_pointEditPart->GetEditCenter());
            if (pos.x != FLT_MAX)
            {
               const ImU32 color = m_centerSelected ? IM_COL32(255, 128, 0, 255) : IM_COL32(80, 160, 255, 255);
               const float radius = 5.f * m_liveUI.GetDPI();
               overlayDrawList->AddCircleFilled(pos, radius, color, 12);
               overlayDrawList->AddCircle(pos, radius + m_liveUI.GetDPI(), IM_COL32(0, 0, 0, 255), 12, 1.5f);
            }
         }
         else
         {
            const auto &points = m_pointEditPart->GetDragPointCurve()->GetPoints();
            for (size_t i = 0; i < points.size(); i++)
            {
               const ImVec2 pos = ctx.Project(Vertex3Ds(points[i]->GetX(), points[i]->GetY(), m_pointEditPart->GetDragPointZ(points[i].get())));
               if (pos.x == FLT_MAX)
                  continue;
               const ImU32 color = IsPointSelected(points[i].get()) ? IM_COL32(255, 128, 0, 255) : (points[i]->IsSmooth() ? IM_COL32(80, 160, 255, 255) : IM_COL32(255, 96, 96, 255));
               const float radius = (i == 0 ? 5.f : 4.f) * m_liveUI.GetDPI(); // First point is drawn slightly larger to mark the curve start
               overlayDrawList->AddCircleFilled(pos, radius, color, 12);
               overlayDrawList->AddCircle(pos, radius + m_liveUI.GetDPI(), IM_COL32(0, 0, 0, 255), 12, 1.5f);
            }
         }
      }

      if (m_physOverlay == PhysicOverlay::All || (m_physOverlay == PhysicOverlay::Selected && !m_multiSel.empty()))
      {
         auto project = [ctx](Vertex3Ds v)
         {
            const ImVec2 pt = ctx.Project(v);
            return Vertex2D(pt.x, pt.y);
         };
         ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 0, 0, 255)); // We abuse ImGui colors to pass render colors
         ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 0, 0, 64));
         for (auto pho : m_player->m_physics->GetHitObjects())
            if (pho != nullptr && (m_physOverlay == PhysicOverlay::All || (m_physOverlay == PhysicOverlay::Selected && IsEditableSelected(pho->m_editable))))
               pho->DrawUI(project, overlayDrawList, true);
         ImGui::PopStyleColor(2);
      }
      if (clipToPane)
         overlayDrawList->PopClipRect();
   }

   // Handle uncaught mouse & keyboard shortcuts: while a viewport pane is bound, scene interactions
   // (camera, picking, box select) only apply inside its image; fullscreen input otherwise
   const bool viewportInput = m_editVp != nullptr ? (m_editVp->m_hovered || m_editVp->m_active) : !io.WantCaptureMouse;
   if (viewportInput && m_fileOp == nullptr)
   {
      // Zoom in/out with mouse wheel
      if (io.MouseWheel != 0)
      {
         if (m_camMode == ViewMode::PreviewCam)
            m_camMode = ViewMode::EditorCam;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 up = view.GetOrthoNormalUp(), dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         m_camDistance *= powf(1.1f, -ImGui::GetIO().MouseWheel);
         const vec3 newEye = camTarget + dir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, up);
      }

      // Mouse pan
      if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      {
         ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
         ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
         if (m_camMode != ViewMode::PreviewCam && (drag.x != 0.f || drag.y != 0.f))
         {
            Matrix3D viewInverse(m_camView);
            viewInverse.Invert();
            vec3 dir = viewInverse.GetOrthoNormalDir();
            const vec3 up = viewInverse.GetOrthoNormalUp();
            const vec3 pos = viewInverse.GetOrthoNormalPos();
            const vec3 right = viewInverse.GetOrthoNormalRight();
            vec3 camTarget = pos - dir * m_camDistance;
            if (io.KeyShift || m_camMode == ViewMode::DesktopBackdrop || m_orbitLock)
            {
               // Pan so that the point under the cursor follows it (the view height at the target depends on the projection;
               // in perspective the raw pixel drag used to be applied as VPU, ignoring the zoom)
               const float halfH = m_perspectiveCam && m_camMode != ViewMode::DesktopBackdrop ? m_camDistance * tanf(0.5f * ANGTORAD(editorCamFovY)) : m_camDistance;
               const float viewH = m_editVp != nullptr && m_editVp->m_imageRect.Height() > 0.f ? m_editVp->m_imageRect.Height() : io.DisplaySize.y;
               drag.x *= 2.f * halfH / viewH;
               drag.y *= 2.f * halfH / viewH;
               camTarget = camTarget - right * drag.x + up * drag.y;
               m_camView = Matrix3D::MatrixLookAtRH(pos - right * drag.x + up * drag.y, camTarget, up);
            }
            else
            {
               m_predefinedView = PredefinedView::None;
               const Matrix3D rx = Matrix3D::MatrixRotate(drag.x * 0.01f, up);
               const Matrix3D ry = Matrix3D::MatrixRotate(drag.y * 0.01f, right);
               const Matrix3D roll = rx * ry;
               dir = roll.MultiplyVectorNoPerspective(dir);
               dir.Normalize();

               // clamp
               vec3 planDir = CrossProduct(right, up);
               planDir.y = 0.f;
               planDir.Normalize();
               if (float dt = planDir.Dot(dir); dt < 0.0f)
               {
                  dir += planDir * dt;
                  dir.Normalize();
               }

               m_camView = Matrix3D::MatrixLookAtRH(camTarget + dir * m_camDistance, camTarget, up);
            }
         }
      }

      // Select: click selects the front-most part (click again to cycle through overlapping parts),
      // shift+click toggles a part in the multi selection, drag box selects parts (shift+drag adds to the selection)
      // In drag point edit mode, click selects a drag point of the edited curve (shift toggles), dragging a
      // selected point moves the selected points in the table XY plane, drag box selects points
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver())
      {
         m_boxSelectStart = ImGui::GetMousePos();
         if (m_partLibrary.IsPlacementArmed())
         {
            // Part library placement mode: drop the previewed part at its ghost position
            m_partLibrary.CommitPlacement();
         }
         else if (m_addPartType != eItemInvalid)
         {
            // Add part mode: create the pending part at the picked point of the playfield XY plane
            const ItemTypeEnum type = m_addPartType;
            m_addPartType = eItemInvalid;
            CreatePart(type, UnprojectToPlane(m_boxSelectStart, 0.f));
         }
         else if (m_pointEditPart != nullptr && m_pointEditCenter)
         {
            // Center point edit mode: clicking on the center selects it and starts a pending drag
            if (HitTestEditCenter(m_boxSelectStart))
            {
               if (io.KeyShift)
                  m_centerSelected = !m_centerSelected;
               else
                  m_centerSelected = true;
               m_pointDragPending = m_centerSelected;
               m_pointDragZ = m_pointEditPart->GetEditCenter().z;
               m_pointDragPos = UnprojectToPlane(m_boxSelectStart, m_pointDragZ);
            }
            else
               m_boxSelectActive = true;
         }
         else
         {
            DragPoint *const hitPoint = (m_pointEditPart != nullptr) ? HitTestDragPoint(m_boxSelectStart) : nullptr;
            if (hitPoint != nullptr)
            {
               if (io.KeyShift)
                  TogglePointSelection(hitPoint);
               else if (!IsPointSelected(hitPoint))
               {
                  m_pointSel.clear();
                  m_pointSel.push_back(hitPoint);
               }
               m_pointDragPending = IsPointSelected(hitPoint) && !hitPoint->m_uiLocked;
               m_pointDragZ = m_pointEditPart->GetDragPointZ(hitPoint);
               m_pointDragPos = UnprojectToPlane(m_boxSelectStart, m_pointDragZ);
            }
            else
               m_boxSelectActive = true;
         }
      }
      if (m_boxSelectActive)
      {
         if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
         {
            const ImVec2 end = ImGui::GetMousePos();
            if (fabsf(end.x - m_boxSelectStart.x) > 2.f || fabsf(end.y - m_boxSelectStart.y) > 2.f)
            {
               overlayDrawList->AddRectFilled(m_boxSelectStart, end, IM_COL32(255, 128, 0, 32));
               overlayDrawList->AddRect(m_boxSelectStart, end, IM_COL32(255, 128, 0, 255));
            }
         }
         else
         {
            m_boxSelectActive = false;
            const ImVec2 end = ImGui::GetMousePos();
            if (m_pointEditPart)
            {
               if (fabsf(end.x - m_boxSelectStart.x) > 4.f || fabsf(end.y - m_boxSelectStart.y) > 4.f)
                  BoxSelectPoints(m_boxSelectStart, end, io.KeyShift);
               else if (!io.KeyShift)
               {
                  m_pointSel.clear();
                  m_centerSelected = false;
               }
            }
            else if (fabsf(end.x - m_boxSelectStart.x) > 4.f || fabsf(end.y - m_boxSelectStart.y) > 4.f)
               BoxSelectParts(m_boxSelectStart, end, io.KeyShift);
            else
            {
               vector<HitTestResult> vhoHit;
               RayCastParts(end, vhoHit);
               if (io.KeyShift)
               {
                  // Shift+click toggles the front-most hit part in the multi selection
                  if (!vhoHit.empty())
                  {
                     const auto it = m_editableMap.find(vhoHit.front().m_obj->m_editable);
                     if (it != m_editableMap.end())
                     {
                        TogglePartSelection(it->second);
                        m_outlinerAnchor = it->second;
                     }
                  }
               }
               else if (vhoHit.empty())
                  ClearSelection();
               else
               {
                  // Click selects the nearest hit part, subsequent clicks cycle to the parts behind it
                  vector<std::shared_ptr<EditorUIPart>> hitParts;
                  for (const HitTestResult &hr : vhoHit)
                  {
                     const auto it = m_editableMap.find(hr.m_obj->m_editable);
                     if (it != m_editableMap.end() && std::ranges::find(hitParts, it->second) == hitParts.end())
                        hitParts.push_back(it->second);
                  }
                  if (hitParts.empty())
                     ClearSelection();
                  else
                  {
                     size_t selectionIndex = 0;
                     const auto selIt = std::ranges::find(hitParts, m_selection.GetPart());
                     if (selIt != hitParts.end())
                        selectionIndex = static_cast<size_t>(selIt - hitParts.begin() + 1) % hitParts.size();
                     SetSelection(Selection(hitParts[selectionIndex]));
                     m_outlinerAnchor = hitParts[selectionIndex];
                  }
                  // TODO add debug action to make ball active: m_player->m_pactiveballDebug = m_pBall;
               }
            }
         }
      }

      // Drag the selected drag points in the table XY plane
      if (m_pointEditPart && (m_pointDragPending || m_pointDragActive))
      {
         if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
         {
            const Vertex2D pos = UnprojectToPlane(ImGui::GetMousePos(), m_pointDragZ);
            const Vertex2D delta(pos.x - m_pointDragPos.x, pos.y - m_pointDragPos.y);
            if (m_pointDragPending)
            {
               const ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
               if (fabsf(dragDelta.x) > 2.f || fabsf(dragDelta.y) > 2.f)
               {
                  // Drag actually starts: mark the edited part for undo once per drag
                  m_pointDragPending = false;
                  m_pointDragActive = m_pointEditCenter ? m_centerSelected : !m_pointSel.empty();
                  if (m_pointDragActive)
                  {
                     m_undo.BeginUndo();
                     m_undo.MarkForUndo(m_pointEditPart->GetEditable());
                     m_undo.EndUndo();
                  }
               }
            }
            if (m_pointDragActive && (delta.x != 0.f || delta.y != 0.f))
            {
               if (m_pointEditCenter)
               {
                  const Vertex3Ds center = m_pointEditPart->GetEditCenter();
                  m_pointEditPart->SetEditCenter(Vertex2D(center.x + delta.x, center.y + delta.y));
               }
               else
               {
                  DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve();
                  for (DragPoint *point : m_pointSel)
                     point->Translate(delta);
                  curve->OnPointsModified();
               }
               m_renderer->ReinitRenderable(m_pointEditPart->GetEditable()->GetIRenderable());
               m_player->m_physics->Update(m_pointEditPart->GetEditable());
            }
            m_pointDragPos = pos;
         }
         else
         {
            m_pointDragPending = false;
            m_pointDragActive = false;
         }
      }
   }
   if (m_boxSelectActive && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
      m_boxSelectActive = false; // Canceled: button released outside of the playfield
   if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
   {
      m_pointDragPending = false; // Released outside of the viewport (the drag flags would otherwise stay set until the mouse comes back)
      m_pointDragActive = false;
   }
   // Ctrl+S must work while a widget (e.g. a slider) has keyboard focus; only a text input being edited takes precedence
   if (m_fileOp == nullptr && !io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_S, false) && io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      SaveTable();
   // Ctrl+Space maximizes / restores the area under the mouse (Blender)
   if (m_fileOp == nullptr && !io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Space, false) && io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      ToggleMaximizeArea();
   if (!io.WantCaptureKeyboard && m_fileOp == nullptr)
   {
      if (ImGui::IsKeyReleased(ImGuiKey_Escape))
      {
         if (m_suppressNextEscape)
            m_suppressNextEscape = false; // Consumed by a field the Esc deactivated (e.g. the script editor)
         else if (ConsumedAreaEscape())
         {
         } // Consumed while cancelling an area split / join (not an editor Esc)
         else if (m_gizmoOperation != ImGuizmo::OPERATION(0))
            m_gizmoOperation = ImGuizmo::OPERATION(0); // Cancel current operation
         else if (m_boxSelectActive)
            m_boxSelectActive = false; // Cancel current box selection
         else if (m_addPartType != eItemInvalid)
            m_addPartType = eItemInvalid; // Cancel add part mode
         else if (m_partLibrary.IsPlacementArmed())
            m_partLibrary.CancelPlacement(); // Cancel part library placement mode
         else if (m_pointEditPart)
            ExitPointEditMode(true); // Exit drag point edit mode
         else if (m_selection.GetType() != Selection::S_NONE)
            ClearSelection(); // Cancel current selection
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_F5) && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
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
      else if (ImGui::IsKeyPressed(ImGuiKey_Tab, false) && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      {
         // Toggle drag point edit mode on the active selected part. For parts exposing an editable
         // center (light bulb, light sequencer animation center), Tab first switches the mode from
         // the drag point curve to the center point before exiting.
         if (m_pointEditPart)
         {
            if (!m_pointEditCenter && m_pointEditPart->HasEditCenter())
            {
               m_pointEditCenter = true;
               m_centerSelected = true;
               m_pointSel.clear();
               m_pointDragPending = false;
               m_pointDragActive = false;
            }
            else
               ExitPointEditMode(true);
         }
         else
            EnterPointEditMode();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_F) && !io.KeyCtrl)
      {
         m_flyMode = !m_flyMode;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_A))
      {
         if (io.KeyAlt && !io.KeyCtrl && !io.KeyShift)
            ClearSelection();
         else if (io.KeyShift && !io.KeyCtrl && !io.KeyAlt)
         {
            // Add a drag point on the curve segment nearest to the mouse position
            if (m_pointEditPart && !m_pointEditCenter && !io.WantCaptureMouse)
               AddPointOnNearestSegment();
            else if (!m_pointEditPart && !IsInspectMode() && !m_table->IsLocked())
            {
               // Request the part type picker popup at the mouse position (it is opened from the
               // toolbar window scope below, as OpenPopup must be called in the same window scope
               // as the matching BeginPopup)
               m_chrome.RequestAddPartPopup(ImGui::GetMousePos());
            }
         }
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
            PasteSelection(ImGui::GetMousePos());
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
                  if (part->GetEditable()->GetItemType() != eItemPartGroup && (part->GetEditable()->m_desktopBackdrop == (m_camMode == ViewMode::DesktopBackdrop))
                     && !part->GetEditable()->IsUIVisible(false))
                  {
                     part->GetEditable()->SetUIVisible(true);
                     changed = true;
                  }
            }
            else if (io.KeyShift)
            { // Hide unselected
               for (auto &part : m_editables)
                  if (part->GetEditable()->GetItemType() != eItemPartGroup && (part->GetEditable()->m_desktopBackdrop == (m_camMode == ViewMode::DesktopBackdrop)) && !IsPartSelected(part)
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
      else if (ImGui::IsKeyPressed(ImGuiKey_G))
      {
         // Grab (translate)
         if (m_camMode == ViewMode::PreviewCam)
            m_camMode = ViewMode::EditorCam;
         if (io.KeyAlt && !m_pointEditPart)
         {
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               SetSelectionTransform(tmp, true, false, false);
         }
         else if (!io.KeyCtrl)
            SetGizmoOperation(ImGuizmo::TRANSLATE);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
         // Ctrl+S (save) is handled earlier, before the WantCaptureKeyboard check
         if (!io.KeyCtrl)
         {
            // Scale
            if (m_camMode == ViewMode::PreviewCam)
               m_camMode = ViewMode::EditorCam;
            if (io.KeyAlt && !m_pointEditPart)
            {
               Matrix3D tmp;
               if (GetSelectionTransform(tmp))
                  SetSelectionTransform(tmp, false, true, false);
            }
            else
               SetGizmoOperation(ImGuizmo::SCALE);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_R))
      {
         // Rotate
         if (m_camMode == ViewMode::PreviewCam)
            m_camMode = ViewMode::EditorCam;
         if (io.KeyAlt && !m_pointEditPart)
         {
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               SetSelectionTransform(tmp, false, false, true);
         }
         else if (!io.KeyCtrl)
            SetGizmoOperation(ImGuizmo::ROTATE);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Z))
      {
         if (io.KeyCtrl)
         { // Undo
            m_lastUndoPart = nullptr;
            m_lastUndoId = 0;
            if (m_table->m_liveBaseTable == nullptr)
            {
               // TODO handle IsUndoPastCleanPoint
               const std::any state = m_undo.Undo();
               if (state.has_value())
                  RestoreUndoSelection(std::any_cast<const UndoSelectionState &>(state));
            }
         }
         else if (!io.KeyShift && !io.KeyAlt)
         { // Wireframe shade mode selection
            switch (m_shadeMode)
            {
            case Renderer::ShadeMode::Default: m_shadeMode = Renderer::ShadeMode::Wireframe; break;
            case Renderer::ShadeMode::Wireframe: m_shadeMode = Renderer::ShadeMode::NoDepthWireframe; break;
            case Renderer::ShadeMode::NoDepthWireframe: m_shadeMode = Renderer::ShadeMode::Default; break;
            }
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad0))
      {
         m_predefinedView = PredefinedView::None;
         switch (m_camMode)
         {
         case ViewMode::PreviewCam:
            m_camMode = ViewMode::EditorCam;
            ResetCameraFromPlayer();
            break;
         case ViewMode::EditorCam:
            m_camMode = ViewMode::DesktopBackdrop;
            SetBackdropCamera();
            break;
         case ViewMode::DesktopBackdrop: m_camMode = ViewMode::PreviewCam; break;
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad5))
      {
         m_camMode = ViewMode::EditorCam;
         m_perspectiveCam = !m_perspectiveCam;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal))
      {
         // Editor Camera center on the whole selection, adjusting the zoom so that it fills the view
         FRect3D bounds;
         if (GetSelectionBounds(bounds))
         {
            m_camMode = ViewMode::EditorCam;
            Matrix3D view(m_camView);
            view.Invert();
            const vec3 up = view.GetOrthoNormalUp();
            const vec3 dir = view.GetOrthoNormalDir();
            const vec3 right = view.GetOrthoNormalRight();
            const vec3 newTarget(0.5f * (bounds.left + bounds.right), 0.5f * (bounds.top + bounds.bottom), -0.5f * (bounds.zlow + bounds.zhigh));

            // Evaluate the distance needed for all the bounds corners to be inside the view, keeping the camera orientation
            // and fitting inside the area left visible by the UI (a symmetric margin accounts for the UI drawn over the render)
            const float aspect = io.DisplaySize.x / io.DisplaySize.y;
            const Vertex2D visible = GetUIVisibleFraction();
            const float tanHalfFovY = tanf(0.5f * ANGTORAD(editorCamFovY));
            float distance = 0.f;
            for (int i = 0; i < 8; i++)
            {
               const vec3 toCorner = vec3((i & 1) ? bounds.right : bounds.left, (i & 2) ? bounds.bottom : bounds.top, (i & 4) ? -bounds.zlow : -bounds.zhigh) - newTarget;
               const float dx = fabsf(right.Dot(toCorner));
               const float dy = fabsf(up.Dot(toCorner));
               distance = max(distance,
                  m_perspectiveCam ? dir.Dot(toCorner) + max(dy / (tanHalfFovY * visible.y), dx / (tanHalfFovY * aspect * visible.x)) : max(dy / visible.y, dx / (aspect * visible.x)));
            }
            if (distance > 0.f) // Keep the current distance for degenerate (point) selections
               m_camDistance = distance * 1.1f; // Small margin so that the selection does not exactly touch the view borders
            const vec3 newEye = newTarget + dir * m_camDistance;
            m_camView = Matrix3D::MatrixLookAtRH(newEye, newTarget, up);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDivide))
      {
         m_orbitLock = !m_orbitLock;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad7))
      {
         // Editor Camera to Top / Bottom
         m_predefinedView = ImGui::GetIO().KeyCtrl ? PredefinedView::Bottom : PredefinedView::Top;
         m_camMode = ViewMode::EditorCam;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         constexpr vec3 newUp { 0.f, -1.f, 0.f };
         const vec3 newDir(0.f, 0.f, ImGui::GetIO().KeyCtrl ? 1.f : -1.f);
         const vec3 newEye = camTarget + newDir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad1))
      {
         // Editor Camera to Front / Back
         m_predefinedView = ImGui::GetIO().KeyCtrl ? PredefinedView::Back : PredefinedView::Front;
         m_camMode = ViewMode::EditorCam;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         constexpr vec3 newUp { 0.f, 0.f, -1.f };
         const vec3 newDir(0.f, ImGui::GetIO().KeyCtrl ? -1.f : 1.f, 0.f);
         const vec3 newEye = camTarget + newDir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad3))
      {
         // Editor Camera to Right / Left
         m_predefinedView = ImGui::GetIO().KeyCtrl ? PredefinedView::Right : PredefinedView::Left;
         m_camMode = ViewMode::EditorCam;
         Matrix3D view(m_camView);
         view.Invert();
         const vec3 /*up = view.GetOrthoNormalUp(),*/ dir = view.GetOrthoNormalDir(), pos = view.GetOrthoNormalPos();
         const vec3 camTarget = pos - dir * m_camDistance;
         constexpr vec3 newUp { 0.f, 0.f, -1.f };
         const vec3 newDir(ImGui::GetIO().KeyCtrl ? 1.f : -1.f, 0.f, 0.f);
         const vec3 newEye = camTarget + newDir * m_camDistance;
         m_camView = Matrix3D::MatrixLookAtRH(newEye, camTarget, newUp);
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

void EditorUI::PushUndo(IEditable *part, unsigned int undoId)
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

void EditorUI::ClearSelection()
{
   m_selection = Selection();
   m_multiSel.clear();
   m_multiSelImages.clear();
   m_multiSelSounds.clear();
   m_multiSelMaterials.clear();
   m_outlinerAnchor.reset();
   m_outlinerImageAnchor = nullptr;
   m_outlinerSoundAnchor = nullptr;
   m_outlinerMaterialAnchor = nullptr;
}

EditorUI::UndoSelectionState EditorUI::CaptureUndoSelection() const
{
   UndoSelectionState state { m_selection, m_multiSel, m_outlinerAnchor, m_pointEditPart, {}, m_pointEditCenter };
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
   m_outlinerAnchor = resolve(state.outlinerAnchor);
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

void EditorUI::SelectOutlinerRange(const std::shared_ptr<EditorUIPart> &part)
{
   // Range selection between the anchor part (last clicked one) and the given part
   if (m_outlinerAnchor == nullptr)
   {
      SetSelection(Selection(part));
      return;
   }
   int anchorPos = -1, partPos = -1, pos = 0;
   for (const auto &edit : m_editables)
   {
      if (edit->GetEditable()->GetItemType() == eItemPartGroup || (edit->GetEditable()->m_desktopBackdrop != (m_camMode == ViewMode::DesktopBackdrop)))
         continue;
      if (edit == m_outlinerAnchor)
         anchorPos = pos;
      if (edit == part)
         partPos = pos;
      pos++;
   }
   if (anchorPos < 0 || partPos < 0)
   {
      SetSelection(Selection(part));
      return;
   }
   if (partPos < anchorPos)
      std::swap(anchorPos, partPos);
   m_multiSel.clear();
   pos = 0;
   for (const auto &edit : m_editables)
   {
      if (edit->GetEditable()->GetItemType() == eItemPartGroup || (edit->GetEditable()->m_desktopBackdrop != (m_camMode == ViewMode::DesktopBackdrop)))
         continue;
      if (pos >= anchorPos && pos <= partPos)
         m_multiSel.push_back(edit);
      pos++;
   }
   m_selection = Selection(part);
}

void EditorUI::SelectPartsInGroup(const PartGroup *group)
{
   // Select all the parts contained in the given group (and in its sub groups, recursively)
   m_multiSel.clear();
   for (const auto &uiPart : m_editables)
   {
      IEditable *const editable = uiPart->GetEditable();
      if (editable->GetItemType() != eItemPartGroup && editable->IsChild(group) && (editable->m_desktopBackdrop == (m_camMode == ViewMode::DesktopBackdrop)))
         m_multiSel.push_back(uiPart);
   }
   m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   m_outlinerAnchor.reset();
}

void EditorUI::MoveSelectionToPartGroup(PartGroup *group)
{
   // Gather the parts that can be moved to the target group: a part can not be moved to itself, and a
   // group can not be moved to itself or one of its descendants (this would create a cycle). Only part
   // groups may be moved to the root of the hierarchy (group-less parts are live objects).
   vector<IEditable *> moved;
   for (const auto &uiPart : m_multiSel)
   {
      IEditable *const editable = uiPart->GetEditable();
      if (editable == nullptr || editable == group || editable->GetPartGroup() == group)
         continue;
      if (editable->GetItemType() == eItemPartGroup && group != nullptr && group->IsChild(static_cast<PartGroup *>(editable)))
         continue;
      if (group == nullptr && editable->GetItemType() != eItemPartGroup)
         continue;
      moved.push_back(editable);
   }
   if (moved.empty())
      return;
   m_undo.BeginUndo();
   for (IEditable *const editable : moved)
   {
      m_undo.MarkForUndo(editable);
      editable->SetPartGroup(group);
   }
   m_undo.EndUndo();
   UpdateEditableList(); // Refresh the outliner paths and ordering
}

void EditorUI::RayCastParts(const ImVec2 &mousePos, vector<HitTestResult> &vhoHit) const
{
   // Compute mouse position in clip space (the edit viewport's render space for a pane, the window for fullscreen)
   const RenderView *const view = m_editVp != nullptr ? m_editVp->GetView() : nullptr;
   int areaW = 0, areaH = 0;
   if (view != nullptr)
      view->GetOutputAreaSize(areaW, areaH);
   const ImVec2 pos = ToPickPos(mousePos);
   const float rClipWidth = areaW > 0 ? (float)areaW * 0.5f : (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
   const float rClipHeight = areaH > 0 ? (float)areaH * 0.5f : (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
   const float xcoord = (pos.x - rClipWidth) / rClipWidth;
   const float ycoord = (rClipHeight - pos.y) / rClipHeight;

   // Use the inverse of our 3D transform to determine where in 3D space the
   // screen pixel the user clicked on is at.  Get the point at the near
   // clipping plane (z=0) and the far clipping plane (z=1) to get the whole
   // range we need to hit test
   Matrix3D invMVP = (view != nullptr ? view->GetMVP() : m_renderer->GetMVP()).GetModelViewProj(0);
   invMVP.Invert();
   const Vertex3Ds v3d = invMVP * Vertex3Ds { xcoord, ycoord, 0.f };
   const Vertex3Ds v3d2 = invMVP * Vertex3Ds { xcoord, ycoord, 1.f };

   // FIXME This is not really great as:
   // - picking depends on what was visible/enabled when quadtree was built (lazily at first pick), and also uses the physics quadtree for some parts
   // - primitives can have hit bug (Apron Top and Gottlieb arm of default table for example): degenerated geometry ?
   // We would need a dedicated quadtree for UI with all parts, and filter after picking by visibility
   vector<HitTestResult> vhoUnfilteredHit;
   m_player->m_physics->RayCast(v3d, v3d2, true, vhoUnfilteredHit);

   // Filter out the colliders that should not be picked
   for (const auto &hr : vhoUnfilteredHit)
   {
      const auto editable = hr.m_obj->m_editable;
      if (editable && !IsEditablePickable(editable))
         continue;
      vhoHit.push_back(hr);
   }
}

bool EditorUI::IsEditablePickable(const IEditable *editable) const
{
   // In desktop backdrop mode, only backdrop parts can be picked, and conversely in the other view modes
   if (editable->m_desktopBackdrop != (m_camMode == ViewMode::DesktopBackdrop))
      return false;
   const PartGroup *parent = editable->GetPartGroup();
   bool visible = editable->IsUIVisible(false);
   while (parent && visible)
   {
      if ((parent->GetPlayerModeVisibilityMask() & m_renderer->GetPlayerModeVisibilityMask()) == 0)
         visible = false;
      visible &= parent->IsUIVisible(false);
      parent = parent->GetPartGroup();
   }
   if (!visible)
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
      if (editable != nullptr && editable->GetItemType() != eItemPartGroup && IsEditablePickable(editable))
         m_multiSel.push_back(uiPart);
   }
   m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
}

void EditorUI::BoxSelectParts(const ImVec2 &cornerA, const ImVec2 &cornerB, bool add)
{
   // Select all the pickable parts whose transform position projects inside the given screen box
   if (!add)
   {
      m_multiSel.clear();
      m_selection = Selection();
   }
   LiveRenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, false);
   SetupPaneProjection(&ctx);
   const ImVec2 boxMin(std::min(cornerA.x, cornerB.x), std::min(cornerA.y, cornerB.y));
   const ImVec2 boxMax(std::max(cornerA.x, cornerB.x), std::max(cornerA.y, cornerB.y));
   for (const auto &uiPart : m_editables)
   {
      IEditable *const editable = uiPart->GetEditable();
      if (editable == nullptr || editable->GetItemType() == eItemPartGroup || !IsEditablePickable(editable))
         continue;
      Matrix3D transform;
      if (uiPart->GetTransform(transform) == EditorUIPart::TM_None)
         continue;
      const ImVec2 pos = ctx.Project(transform.GetOrthoNormalPos());
      if (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y && !IsPartSelected(uiPart))
         m_multiSel.push_back(uiPart);
   }
   if (!m_multiSel.empty() && (m_selection.GetType() != Selection::S_EDITABLE || !IsPartSelected(m_selection.GetPart())))
      m_selection = Selection(m_multiSel.back());
}

void EditorUI::CreatePart(const ItemTypeEnum type, const Vertex2D &pos)
{
   IEditable *const pie = EditableRegistry::CreateAndInit(type, m_table, pos.x, pos.y);
   if (pie == nullptr)
      return;

   // Same initialization sequence as the WinUI editor
   if (auto *const scriptable = pie->GetIScriptable(); scriptable)
      m_table->GetUniqueName(type, scriptable->m_name);
   pie->m_desktopBackdrop = (m_camMode == ViewMode::DesktopBackdrop);
   m_table->AddPart(pie);
   pie->SetPartGroup(GetPartGroupForNewPart());

   // Same player side setup as player initialization
   if (type == eItemBall)
      m_player->m_vball.push_back(static_cast<Ball *>(pie));
   m_player->TimerSetup(pie);
   if (auto *const renderable = pie->GetIRenderable(); renderable)
      renderable->RenderSetup(m_renderer.get());
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
      m_outlinerAnchor = it->second;
   }
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

void EditorUI::PasteSelection(const ImVec2 &pos)
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
   const bool backdrop = (m_camMode == ViewMode::DesktopBackdrop);
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
   // Move the pasted parts so that their combined center lands at the given position (in the table XY plane, keeping their Z)
   const Vertex2D target = UnprojectToPlane(pos, 0.f);
   const Vertex2D offset(target.x - 0.5f * (pasteMin.x + pasteMax.x), target.y - 0.5f * (pasteMin.y + pasteMax.y));
   for (IEditable *const editable : pasted)
   {
      editable->Translate(offset);
      // Player side setup, same as part creation
      if (editable->GetItemType() == eItemBall)
         m_player->m_vball.push_back(static_cast<Ball *>(editable));
      m_player->TimerSetup(editable);
      if (auto *const renderable = editable->GetIRenderable(); renderable)
         renderable->RenderSetup(m_renderer.get());
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
   m_outlinerAnchor = m_multiSel.empty() ? nullptr : m_multiSel.back();
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
   if (m_outlinerAnchor && std::ranges::find(m_editables, m_outlinerAnchor) == m_editables.end())
      m_outlinerAnchor.reset();
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
   if (m_outlinerAnchor && (!liveParts.contains(m_outlinerAnchor->GetEditable()) || isLiveObject(m_outlinerAnchor->GetEditable())))
      m_outlinerAnchor.reset();
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

void EditorUI::SetGizmoOperation(ImGuizmo::OPERATION operation)
{
   if (operation == ImGuizmo::OPERATION(0))
   {
      m_gizmoOperation = operation;
      return;
   }
   if (m_camMode == ViewMode::PreviewCam)
      m_camMode = ViewMode::EditorCam;
   // Repeating the same operation toggles between world and local coordinates, switching to another one resets to world
   m_gizmoMode = m_gizmoOperation == operation ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
   m_gizmoOperation = operation;
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
         rot.x = -RADTOANG(atan2f(transform._23, transform._22));
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

void EditorUI::EnterPointEditMode()
{
   if (m_pointEditPart || IsInspectMode() || m_table->IsLocked())
      return;
   if (m_selection.GetType() != Selection::S_EDITABLE || m_selection.GetPart() == nullptr
      || (m_selection.GetPart()->GetDragPointCurve() == nullptr && !m_selection.GetPart()->HasEditCenter()))
      return;
   m_savedSelection = m_selection;
   m_savedMultiSel = m_multiSel;
   m_savedOutlinerAnchor = m_outlinerAnchor;
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
   m_outlinerAnchor = (m_savedOutlinerAnchor == nullptr || liveParts.contains(m_savedOutlinerAnchor->GetEditable())) ? m_savedOutlinerAnchor : nullptr;
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
   // Unproject the mouse position on the drag plane of the edited curve
   const float z = m_pointEditPart->GetDragPointZ(m_pointSel.empty() ? points.front().get() : m_pointSel.front());
   const Vertex2D pos = UnprojectToPlane(ImGui::GetMousePos(), z);
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

Vertex2D EditorUI::UnprojectToPlane(const ImVec2 &mousePos, float z) const
{
   // Compute the mouse ray and intersect it with the table plane at the given Z coordinate
   // (through the edit viewport's MVP and render space for a pane)
   const RenderView *const view = m_editVp != nullptr ? m_editVp->GetView() : nullptr;
   int areaW = 0, areaH = 0;
   if (view != nullptr)
      view->GetOutputAreaSize(areaW, areaH);
   const ImVec2 pos = ToPickPos(mousePos);
   const float rClipWidth = areaW > 0 ? (float)areaW * 0.5f : (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
   const float rClipHeight = areaH > 0 ? (float)areaH * 0.5f : (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
   const float xcoord = (pos.x - rClipWidth) / rClipWidth;
   const float ycoord = (rClipHeight - pos.y) / rClipHeight;
   Matrix3D invMVP = (view != nullptr ? view->GetMVP() : m_renderer->GetMVP()).GetModelViewProj(0);
   invMVP.Invert();
   const Vertex3Ds v3d = invMVP * Vertex3Ds { xcoord, ycoord, 0.f };
   const Vertex3Ds v3d2 = invMVP * Vertex3Ds { xcoord, ycoord, 1.f };
   const float dz = v3d2.z - v3d.z;
   const float t = (fabsf(dz) > 1e-10f) ? (z - v3d.z) / dz : 0.f;
   return Vertex2D(v3d.x + t * (v3d2.x - v3d.x), v3d.y + t * (v3d2.y - v3d.y));
}

DragPoint *EditorUI::HitTestDragPoint(const ImVec2 &mousePos) const
{
   LiveRenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, false);
   SetupPaneProjection(&ctx);
   const float maxDist = 10.f * m_liveUI.GetDPI();
   DragPoint *best = nullptr;
   float bestDist = maxDist;
   for (const auto &point : m_pointEditPart->GetDragPointCurve()->GetPoints())
   {
      const ImVec2 pos = ctx.Project(Vertex3Ds(point->GetX(), point->GetY(), m_pointEditPart->GetDragPointZ(point.get())));
      if (pos.x == FLT_MAX)
         continue;
      const float dx = pos.x - mousePos.x;
      const float dy = pos.y - mousePos.y;
      const float dist = sqrtf(dx * dx + dy * dy);
      if (dist < bestDist)
      {
         bestDist = dist;
         best = point.get();
      }
   }
   return best;
}

bool EditorUI::HitTestEditCenter(const ImVec2 &mousePos) const
{
   LiveRenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, false);
   SetupPaneProjection(&ctx);
   const ImVec2 pos = ctx.Project(m_pointEditPart->GetEditCenter());
   if (pos.x == FLT_MAX)
      return false;
   const float maxDist = 10.f * m_liveUI.GetDPI();
   const float dx = pos.x - mousePos.x;
   const float dy = pos.y - mousePos.y;
   return dx * dx + dy * dy < maxDist * maxDist;
}

void EditorUI::BoxSelectPoints(const ImVec2 &cornerA, const ImVec2 &cornerB, bool add)
{
   // Select all the drag points of the edited curve projecting inside the given screen box
   if (!add)
      m_pointSel.clear();
   LiveRenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, false);
   SetupPaneProjection(&ctx);
   if (m_pointEditCenter)
   {
      // In center edit mode, select the part's center point if it projects inside the box
      const ImVec2 boxMin(std::min(cornerA.x, cornerB.x), std::min(cornerA.y, cornerB.y));
      const ImVec2 boxMax(std::max(cornerA.x, cornerB.x), std::max(cornerA.y, cornerB.y));
      const ImVec2 pos = ctx.Project(m_pointEditPart->GetEditCenter());
      if (!add)
         m_centerSelected = false;
      if (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y)
         m_centerSelected = true;
      return;
   }
   const ImVec2 boxMin(std::min(cornerA.x, cornerB.x), std::min(cornerA.y, cornerB.y));
   const ImVec2 boxMax(std::max(cornerA.x, cornerB.x), std::max(cornerA.y, cornerB.y));
   for (const auto &point : m_pointEditPart->GetDragPointCurve()->GetPoints())
   {
      const ImVec2 pos = ctx.Project(Vertex3Ds(point->GetX(), point->GetY(), m_pointEditPart->GetDragPointZ(point.get())));
      if (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y && !IsPointSelected(point.get()))
         m_pointSel.push_back(point.get());
   }
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

namespace
{
   // Accent orange of the area chrome (split previews, highlighted splitters)
   constexpr ImU32 kAreaAccent = IM_COL32(237, 145, 62, 255);
   constexpr ImU32 kAreaText = IM_COL32(230, 230, 230, 255);
   constexpr ImU32 kAreaTextDim = IM_COL32(160, 160, 160, 255);
}

const char *EditorUI::AreaTypeName(const AreaType type) { return type == AreaType::Viewport ? "3D View" : AreaTypeJsonName(type); }

bool EditorUI::IsAreaTypeImplemented(const AreaType type)
{
   // The area types a leaf may take through the editor type switcher (the others are added with their views)
   return type == AreaType::Outliner || type == AreaType::Properties || type == AreaType::Viewport;
}

std::unique_ptr<EditorAreaView> EditorUI::CreateAreaView(const AreaType type)
{
   switch (type)
   {
   case AreaType::Outliner: return std::make_unique<OutlinerPanel>(*this);
   case AreaType::Properties: return std::make_unique<PropertiesPanel>(*this);
   case AreaType::Viewport: return std::make_unique<ViewportPane>(*this);
   default: return nullptr; // Script and PartLibrary are added with their views
   }
}

void EditorUI::Report(const string &message) { m_liveUI.PushNotification(message, 3000); }

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
      if (leaf.visible && leaf.id != leafId && leaf.type != AreaType::Viewport && (mineX1 <= mineX0 || leaf.rect.x0 >= mineX1 || leaf.rect.x1 <= mineX0))
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
   m_activeLayoutIndex = clamp(settings.GetLiveEditor_LayoutActive() - 1, 0, 1);
   const string json = m_activeLayoutIndex == 0 ? settings.GetLiveEditor_Layout1() : settings.GetLiveEditor_Layout2();
   const nlohmann::json parsed = nlohmann::json::parse(json, nullptr, false);
   if (parsed.is_discarded() || !m_layout.FromJson(parsed))
      m_layout = AreaLayout(); // Missing or invalid layout: back to the default tree
   m_maximizedLeafId = -1;
   m_hoverLeaf = -1;
   CancelAreaInteraction(false);
   SetEditViewport(nullptr);
}

void EditorUI::SaveAreaLayoutSettings() const
{
   Settings &settings = g_settingsService.GetAppSettings();
   const string json = m_layout.ToJson().dump();
   if (m_activeLayoutIndex == 0)
      settings.SetLiveEditor_Layout1(json, false);
   else
      settings.SetLiveEditor_Layout2(json, false);
}

void EditorUI::SetAreaLayoutIndex(const int index)
{
   if (index == m_activeLayoutIndex)
      return;
   SaveAreaLayoutSettings(); // The current layout leaves its slot
   m_activeLayoutIndex = index;
   g_settingsService.GetAppSettings().SetLiveEditor_LayoutActive(index + 1, false);
   Settings &settings = g_settingsService.GetAppSettings();
   const string json = index == 0 ? settings.GetLiveEditor_Layout1() : settings.GetLiveEditor_Layout2();
   const nlohmann::json parsed = nlohmann::json::parse(json, nullptr, false);
   if (parsed.is_discarded() || !m_layout.FromJson(parsed))
      m_layout = AreaLayout();
   m_maximizedLeafId = -1;
   m_hoverLeaf = -1;
   CancelAreaInteraction(false);
   SetEditViewport(nullptr);
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
   // The tree fills the window below the top bar
   const float top = viewport->Pos.y + m_chrome.GetTopBarHeight();
   const AreaRect tree { viewport->Pos.x, top, viewport->Pos.x + viewport->Size.x, max(top, viewport->Pos.y + viewport->Size.y) };
   const bool areasShown = !m_table->IsLocked() && !m_flyMode && !m_inspectionModal.IsVisible();
   m_layout.Compute(tree, dpi, m_leafRects, m_splitterRects, areasShown ? m_maximizedLeafId : -1);

   // The viewport area is the window onto the 3D render; when no area is drawn (locked table, fly mode,
   // inspection) it is the whole tree
   m_viewportRect = tree;
   m_viewportVisible = true;
   if (areasShown)
   {
      m_viewportVisible = false;
      for (const LeafRect &leaf : m_leafRects)
         if (leaf.type == AreaType::Viewport && leaf.visible)
         {
            m_viewportRect = leaf.rect;
            m_viewportVisible = true;
            break;
         }
      if (!m_viewportVisible)
         m_viewportRect = AreaRect(); // A panel is maximized: nothing of the viewport is shown
   }
   m_areaRectsValid = true;
}

bool EditorUI::ShouldRaiseAreaWindows() const { return m_raiseAreaWindows && !ImGui::IsPopupOpen(static_cast<ImGuiID>(0), ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel); }

bool EditorUI::CanUseAreaTools() const
{
   // The corner zones, the edge menu and their modes only exist while nothing else owns the mouse
   if (m_table->IsLocked() || m_flyMode || m_inspectionModal.IsVisible() || m_fileOp != nullptr)
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
      = (leaf->type == AreaType::Viewport ? (vertical ? AreaLayout::kViewportMinW : AreaLayout::kViewportMinH) : (vertical ? AreaLayout::kPanelMinW : AreaLayout::kPanelMinH)) * dpi;
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
      centeredText(plan.newRect, AreaTypeName(plan.newType), kAreaText);
      drawList->AddLine(a, b, kAreaAccent, 2.f);
   }
   else
   {
      drawList->AddLine(a, b, IM_COL32(150, 150, 150, 220), 2.f);
      centeredText(plan.leaf, plan.reason, kAreaText);
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
   drawList->AddRect(ImVec2(keep->rect.x0 + dpi, keep->rect.y0 + dpi), ImVec2(keep->rect.x1 - dpi, keep->rect.y1 - dpi), kAreaAccent, 0.f, 0, 2.f * dpi);
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
   drawList->AddLine(tail, base, kAreaAccent, max(3.f * dpi, 0.12f * length));
   drawList->AddTriangleFilled(tip, ImVec2(base.x + perp.x * halfHead, base.y + perp.y * halfHead), ImVec2(base.x - perp.x * halfHead, base.y - perp.y * halfHead), kAreaAccent);
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
   for (const auto &[id, view] : m_areaViews)
      if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()))
      {
         vp->SetRendering(shown[id]);
         if (vp->m_hovered)
            m_hoverVp = vp;
      }
   for (const LeafRect &leaf : m_leafRects)
   {
      if (!leaf.visible)
         continue;
      auto [it, inserted] = m_areaViews.try_emplace(leaf.id, nullptr);
      if (it->second == nullptr)
         it->second = CreateAreaView(leaf.type);
      if (it->second != nullptr)
         it->second->RenderPane(leaf.rect, leaf.id);
   }
   // Route the interactions to the viewport pane under the mouse; an ongoing drag keeps its pane
   m_captureVp
      = (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Middle) || ImGui::IsMouseDown(ImGuiMouseButton_Right)) && m_editVp != nullptr && m_editVp->m_active
      ? m_editVp
      : nullptr;
   ViewportPane *const wanted = m_captureVp != nullptr ? m_captureVp : m_hoverVp;
   if (wanted != nullptr && wanted != m_editVp)
      SetEditViewport(wanted);
   if (m_editVp == nullptr) // Bind a pane right away: its camera is ready before the first hover
      for (const auto &[id, view] : m_areaViews)
         if (auto *const vp = dynamic_cast<ViewportPane *>(view.get()))
         {
            SetEditViewport(vp);
            break;
         }
}

void EditorUI::RenderAreaTypeButton(const int leafId)
{
   // The area's own editor type button, drawn inside its window header (Blender's top left icon)
   const AreaNode *const node = m_layout.Find(leafId);
   if (node == nullptr || node->isSplit)
      return;
   char popup[40];
   snprintf(popup, sizeof(popup), "##EditorType%d", leafId); // Keyed by the leaf id: several areas may share a type
   char label[40];
   snprintf(label, sizeof(label), "%s##EditorType", AreaTypeName(node->type));
   if (ImGui::Button(label))
      ImGui::OpenPopup(popup);
   if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Editor type");
   if (ImGui::BeginPopup(popup))
   {
      ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
      ImGui::TextUnformatted("Editor Type");
      ImGui::PopStyleColor();
      ImGui::Separator();
      for (int t = 0; t < static_cast<int>(AreaType::Count); ++t)
      {
         const AreaType type = static_cast<AreaType>(t);
         if (!IsAreaTypeImplemented(type))
            continue;
         if (ImGui::MenuItem(AreaTypeName(type), nullptr, node->type == type))
            SetAreaEditor(leafId, type);
      }
      ImGui::EndPopup();
   }
}

void EditorUI::RenderAreaTypeButton(const int leafId, const AreaRect &rect)
{
   // Floating variant for the windowless leaves: a small button at the top left corner of the area
   const float dpi = m_liveUI.GetDPI();
   char name[40];
   snprintf(name, sizeof(name), "##AreaTypeBtn%d", leafId);
   ImGui::SetNextWindowPos(ImVec2(rect.x0 + 4.f * dpi, rect.y0 + 4.f * dpi));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.f * dpi, 2.f * dpi));
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
         | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav);
   RenderAreaTypeButton(leafId);
   ImGui::End();
   ImGui::PopStyleVar();
}

void EditorUI::RenderSplitters()
{
   // Divider strips between the areas (Blender: drag to resize, right click for the area options). They are
   // small windows created after the panes, kept above them, and give way to any open popup
   const float dpi = m_liveUI.GetDPI();
   const bool interactive = CanUseAreaTools() && m_areaModal == AreaModalKind::None && !m_areaDrag.active;
   const bool raiseNow = ShouldRaiseAreaWindows();
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
      dl->AddLine(a, b, hovered ? kAreaAccent : ImGui::GetColorU32(ImGuiCol_Border), 1.f);
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
      const float fullSide = min(14.f * dpi, 0.4f * min(leaf.rect.Width(), leaf.rect.Height()));
      if (fullSide < 6.f)
         continue;
      for (int corner = 0; corner < 4; ++corner)
      {
         const float side = fullSide;
         const float x = (corner & 1) ? leaf.rect.x1 - side : leaf.rect.x0;
         const float y = (corner & 2) ? leaf.rect.y1 - side : leaf.rect.y0;
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
         bool overWidget = hoveredId != 0 && (cornerWindow == nullptr || hoveredId != cornerWindow->GetID("##corner"));
         ImGuiWindow *const hw = ImGui::GetCurrentContext()->HoveredWindow;
         const char *const hwName = hw != nullptr ? hw->RootWindow->Name : "";
         const bool overStrip = strncmp(hwName, "##Split", 7) == 0;
         const bool overArea = hw == nullptr || strncmp(hwName, "##Pane", 6) == 0 || strncmp(hwName, "##AreaTypeBtn", 13) == 0
            || strcmp(hwName, "orbit lock toggle") == 0; // The status overlay belongs to the viewport area
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
         {
            ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
            dl->AddTriangleFilled(origin, ImVec2(origin.x + sx * side, origin.y), ImVec2(origin.x, origin.y + sy * side), (kAreaAccent & ~IM_COL32_A_MASK) | (70u << IM_COL32_A_SHIFT));
         }
         if (!m_areaDrag.active)
         {
            // Permanent hatch (two diagonal strokes), brighter while hovered
            const ImU32 hatch = hot ? kAreaText : ((kAreaTextDim & ~IM_COL32_A_MASK) | (90u << IM_COL32_A_SHIFT));
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

// ---------------------------------------------------------------------------------------------------------------------
// Viewport panes

void EditorUI::SetEditViewport(ViewportPane *pane)
{
   if (pane == m_editVp)
      return;
   if (m_editVp != nullptr)
      m_editVp->CaptureCamera(); // Store the editor's camera fields back into the pane being left
   m_editVp = pane;
   if (m_editVp != nullptr)
   {
      if (m_editVp->m_cameraValid)
         m_editVp->ReleaseCamera(); // Restore the pane's own camera into the editor fields
      else
         m_editVp->CaptureCamera(); // A never bound pane adopts the current camera (split panes clone it)
   }
}

ViewportPane *EditorUI::HoveredViewportPane() const { return m_hoverVp; }

ImVec2 EditorUI::ToPickPos(const ImVec2 &displayPos) const
{
   // The picking helpers work in the viewport's render space: for a pane, display px are
   // mapped into its output area in the shared backbuffer (fullscreen: display and render spaces coincide)
   const RenderView *const view = m_editVp != nullptr ? m_editVp->GetView() : nullptr;
   if (view == nullptr || m_editVp->m_imageRect.Width() <= 0.f || m_editVp->m_imageRect.Height() <= 0.f)
      return displayPos;
   int w = 0, h = 0;
   view->GetOutputAreaSize(w, h);
   const AreaRect &r = m_editVp->m_imageRect;
   return ImVec2 { (displayPos.x - r.x0) * (float)w / r.Width(), (displayPos.y - r.y0) * (float)h / r.Height() };
}

void EditorUI::SetupPaneProjection(LiveRenderContext *const ctx) const
{
   const RenderView *const view = m_editVp != nullptr ? m_editVp->GetView() : nullptr;
   const AreaRect r = m_editVp != nullptr ? m_editVp->m_imageRect : AreaRect {};
   if (view == nullptr || r.Width() <= 0.f || r.Height() <= 0.f)
      return;
   int w = 0, h = 0;
   view->GetOutputAreaSize(w, h);
   ctx->SetPaneProjection(view->GetMVP(), ImVec2(r.x0, r.y0), ImVec2(r.Width() / (float)w, r.Height() / (float)h), (float)w * 0.5f, (float)h * 0.5f);
}
}
