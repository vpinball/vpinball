// license:GPLv3+

#include "core/stdafx.h"
#include "EditorUI.h"

#include "core/TableDB.h"
#include "core/VPXPluginAPIImpl.h"
#include "core/editablereg.h"
#include "core/FileLocator.h"
#include "core/vpapp.h"

#include "editor/EditorUIPart.h"
#include "editor/EditorUIPartRegistry.h"

#include "parts/PartGroup.h"
#include "parts/ball.h"
#include "parts/primitive.h"
#include "parts/dragpoint.h"

#include "plugins/VPXPlugin.h"

#include "renderer/Anaglyph.h"
#include "renderer/Renderer.h"
#include "renderer/Sampler.h"
#include "renderer/Shader.h"
#include "renderer/VRDevice.h"

#include "ui/VPXFileFeedback.h"
#include "ui/live/LiveUI.h"

#include "utils/color.h"

#include "imgui/imgui.h"

#if defined(ENABLE_DX9)
#include <shellapi.h>
#endif

#include "imgui/imgui_stdlib.h"
#include "imguizmo/ImGuizmo.h"
#include "imgui_markdown/imgui_markdown.h"

namespace VPX::EditorUI
{

// Titles (used as Ids) of modal dialogs
#define ID_RENDERER_INSPECTION "Renderer Inspection"

template <class T> static std::vector<T> SortedCaseInsensitive(std::vector<T> &list, const std::function<string(T)> &map)
{
   std::vector<T> sorted(list.begin(), list.end());
   std::ranges::sort(sorted,
      [map](const T &a, const T &b) -> bool
      {
         const string str1 = map(a), str2 = map(b);
         for (string::const_iterator c1 = str1.begin(), c2 = str2.begin(); c1 != str1.end() && c2 != str2.end(); ++c1, ++c2)
         {
            const auto cl1 = cLower(*c1);
            const auto cl2 = cLower(*c2);
            if (cl1 > cl2)
               return false;
            if (cl1 < cl2)
               return true;
         }
         return str1.size() > str2.size();
      });
   return sorted;
}


EditorUI::EditorUI(LiveUI &liveUI)
   : m_liveUI(liveUI)
   , m_player(g_pplayer)
   , m_renderer(m_player->m_renderer)
   , m_undo(m_player->m_ptable)
{
   m_table = m_player->m_ptable;
   m_pininput = &(m_player->m_pininput);

   EditorUIPartRegistry::InitRegistry();

   ClearSelection();

   // Editor camera position. We use a right handed system for easy ImGuizmo integration while VPX renderer is left handed, so reverse X axis
   m_camDistance = m_table->m_bottom * 0.7f;
   const vec3 eye(m_table->m_right * 0.5f, m_table->m_bottom * 0.5f, -m_camDistance);
   const vec3 at(m_table->m_right * 0.5f, m_table->m_bottom * 0.5f, 0.f);
   constexpr vec3 up { 0.f, -1.f, 0.f };
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
   ImGuizmo::AllowAxisFlip(false);
}

EditorUI::~EditorUI() { }

void EditorUI::Open()
{
   if (m_isOpened)
      return;
   m_isOpened = true;
   m_boxSelectActive = false;
   ResetCameraFromPlayer();
   m_player->SetPlayState(false);
   m_renderer->DisableStaticPrePass(true);
}

void EditorUI::Close()
{
   if (!m_isOpened)
      return;
   m_isOpened = false;
   m_flyMode = false;
   ExitPointEditMode(false);
   if (m_showRendererInspection)
   {
      m_showRendererInspection = false;
      m_player->m_infoMode = IF_FPS;
   }
   m_renderer->DisableStaticPrePass(false);
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
   m_camDistance = 0.5f * max((float)EDITOR_BG_HEIGHT, (float)EDITOR_BG_WIDTH * io.DisplaySize.y / io.DisplaySize.x);
   const vec3 eye((float)EDITOR_BG_WIDTH * 0.5f, (float)EDITOR_BG_HEIGHT * 0.5f, -m_camDistance);
   const vec3 at((float)EDITOR_BG_WIDTH * 0.5f, (float)EDITOR_BG_HEIGHT * 0.5f, 0.f);
   constexpr vec3 up { 0.f, -1.f, 0.f };
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
}

void EditorUI::Render3D()
{
   UpdateEditableList();
   RenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, (m_table->m_liveBaseTable != nullptr) && m_player->IsPlaying());
   for (const auto &uiPart : m_editables)
   {
      if ((m_camMode == ViewMode::DesktopBackdrop && !uiPart->GetEditable()->m_desktopBackdrop) || (m_camMode != ViewMode::DesktopBackdrop && uiPart->GetEditable()->m_desktopBackdrop))
         continue;
      uiPart->Render(ctx);
   }
}

void EditorUI::RenderUI()
{
   const ImGuiIO &io = ImGui::GetIO();
   ImGuizmo::SetOrthographic(m_camMode == ViewMode::DesktopBackdrop || (m_camMode == ViewMode::EditorCam && !m_perspectiveCam));
   ImGuizmo::BeginFrame();
   ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

   const auto previousMultiSel = m_multiSel;

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
      if (IsInspectMode() || curve == nullptr || m_selection.GetType() != Selection::S_EDITABLE || m_selection.GetPart() != m_pointEditPart)
         ExitPointEditMode(false);
      else
      {
         const vector<CComObject<DragPoint> *> &points = curve->GetPoints();
         std::erase_if(m_pointSel, [&points](const DragPoint *point) { return std::ranges::find(points, point) == points.end(); });
      }
   }

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))

   // Gives some transparency when positioning camera to better view camera view bounds
   // TODO for some reasons, this breaks the modal background behavior
   //SetupImGuiStyle(m_selection.GetType() == Selection::S_CAMERA ? 0.3f : 1.0f);

   bool showFullUI = true;
   showFullUI &= !m_showRendererInspection;
   showFullUI &= !m_flyMode;

   m_menubar_height = 0.0f;
   m_toolbar_height = showFullUI ? 20.f * m_liveUI.GetDPI() : 0.f;

   if (showFullUI)
   {
      // Main menubar
      if (ImGui::BeginMainMenuBar())
      {
         if (!IsInspectMode() && ImGui::BeginMenu("File"))
         {
            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
               // TODO cursor feedback
               VPXFileFeedback feedback;
               if (SUCCEEDED(m_table->Save(feedback)))
                  m_undo.SetCleanPoint(eSaveClean);
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit"))
               m_player->SetCloseState(Player::CS_CLOSE_APP);
            ImGui::EndMenu();
         }
         if (IsInspectMode() && !m_table->IsLocked() && ImGui::BeginMenu("Debug"))
         {
            if (ImGui::MenuItem("Open debugger"))
               m_player->m_showDebugger = true;
            if (ImGui::MenuItem("Renderer Inspection"))
               m_showRendererInspection = true;
            if (ImGui::MenuItem(m_player->IsPlaying() ? "Pause" : "Play"))
               m_player->SetPlayState(!m_player->IsPlaying());
            ImGui::EndMenu();
         }
         float buttonWidth = 0.f;
         if (IsInspectMode())
            buttonWidth += ImGui::CalcTextSize(ICON_FK_REPLY, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
         buttonWidth += ImGui::CalcTextSize(ICON_FK_WINDOW_CLOSE, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
         ImVec2 padding = ImGui::GetCursorScreenPos();
         padding.x += ImGui::GetContentRegionAvail().x - buttonWidth;
         ImGui::SetCursorScreenPos(padding);
         if (IsInspectMode())
         {
            if (ImGui::Button(ICON_FK_REPLY)) // ICON_FK_STOP
               Close();
            if (ImGui::IsItemHovered())
               ImGui::SetTooltip("Get back to player");
         }
         if (ImGui::Button(ICON_FK_WINDOW_CLOSE))
            m_table->QuitPlayer(IsInspectMode() ? Player::CS_STOP_PLAY : Player::CS_CLOSE_APP);
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Close editor");
         m_menubar_height = ImGui::GetWindowSize().y;
         ImGui::EndMainMenuBar();
      }

      // Main toolbar
      const ImGuiViewport *const viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height));
      ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, m_toolbar_height));
      constexpr ImGuiWindowFlags window_flags
         = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
      ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
      ImGui::Begin("TOOLBAR", nullptr, window_flags);
      ImGui::PopStyleVar();
      if (IsInspectMode())
      {
         if (ImGui::Button(m_player->IsPlaying() ? ICON_FK_PAUSE : ICON_FK_PLAY))
            m_player->SetPlayState(!m_player->IsPlaying());
         ImGui::SameLine();
         ImGui::BeginDisabled(m_player->IsPlaying());
         if (ImGui::Button(ICON_FK_STEP_FORWARD))
            m_player->m_step = true;
         ImGui::EndDisabled();
      }
      else
      {
         if (m_selection.GetType() == Selection::S_EDITABLE && ImGui::Button(ICON_FK_TRASH_O))
            DeleteSelection();

         // Add part buttons (same parts and icons as the WinUI toolbar)
         if (m_addPartButtons.empty())
         {
            static const struct
            {
               ItemTypeEnum type;
               const char *name;
               const char *file;
            } addParts[] = {
               { eItemSurface, "Surface", "editor/wall.png" },          //
               { eItemGate, "Gate", "editor/gate.png" },                //
               { eItemRamp, "Ramp", "editor/ramp.png" },                //
               { eItemFlipper, "Flipper", "editor/flipper.png" },       //
               { eItemPlunger, "Plunger", "editor/plunger.png" },       //
               { eItemBall, "Ball", "editor/ball.png" },                //
               { eItemBumper, "Bumper", "editor/bumper.png" },          //
               { eItemSpinner, "Spinner", "editor/spinner.png" },       //
               { eItemTimer, "Timer", "editor/timer.png" },             //
               { eItemTrigger, "Trigger", "editor/trigger.png" },       //
               { eItemLight, "Light", "editor/light.png" },             //
               { eItemKicker, "Kicker", "editor/kicker.png" },          //
               { eItemHitTarget, "Target", "editor/target.png" },       //
               { eItemDecal, "Decal", "editor/decal.png" },             //
               { eItemTextbox, "Textbox", "editor/textbox.png" },       //
               { eItemDispReel, "Display Reel", "editor/reel.png" },    //
               { eItemLightSeq, "Light Seq", "editor/lightseq.png" },   //
               { eItemPrimitive, "Primitive", "editor/primitive.png" }, //
               { eItemFlasher, "Flasher", "editor/flasher.png" },       //
               { eItemRubber, "Rubber", "editor/rubber.png" },          //
            };
            for (const auto &def : addParts)
            {
               std::shared_ptr<const BaseTexture> tex
                  = BaseTexture::CreateFromFile(g_app->m_fileLocator.GetAppPath(FileLocator::AppSubFolder::Assets, def.file));
               m_addPartButtons.push_back(
                  { def.type, def.name, tex ? std::make_shared<Sampler>(m_renderer->m_renderDevice, def.name, tex, false) : nullptr });
            }
         }
         const float iconSize = ImGui::GetContentRegionAvail().y;
         ImGui::BeginDisabled(m_pointEditPart != nullptr || m_table->IsLocked());
         for (const auto &button : m_addPartButtons)
         {
            ImGui::SameLine();
            if (button.icon == nullptr)
               continue;
            const bool active = (m_addPartType == button.type);
            if (active)
               ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            if (ImGui::ImageButton(button.name, button.icon, ImVec2(iconSize, iconSize)))
               m_addPartType = active ? eItemInvalid : button.type;
            if (active)
               ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
               ImGui::SetTooltip("Add %s", button.name);
         }
         ImGui::EndDisabled();
      }
      const float buttonWidth = //
         ImGui::CalcTextSize(ICON_FK_EXCHANGE, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f //
         + ImGui::CalcTextSize(ICON_FK_STICKY_NOTE, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f //
         + ImGui::CalcTextSize(ICON_FK_FILTER, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      ImGui::SameLine(ImGui::GetContentRegionAvail().x - buttonWidth);
      if (ImGui::Button(ICON_FK_EXCHANGE)) // Unit selection menu
         ImGui::OpenPopup("Unit Popup");
      if (ImGui::BeginPopup("Unit Popup"))
      {
         ImGui::TextUnformatted("Units:");
         ImGui::Separator();
         if (ImGui::RadioButton("VP Units", m_units == Units::VPX))
            m_units = Units::VPX;
         if (ImGui::RadioButton("Metric", m_units == Units::Metric))
            m_units = Units::Metric;
         if (ImGui::RadioButton("Imperial", m_units == Units::Imperial))
            m_units = Units::Imperial;
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_STICKY_NOTE)) // Overlay option menu
         ImGui::OpenPopup("Overlay Popup");
      if (ImGui::BeginPopup("Overlay Popup"))
      {
         ImGui::TextUnformatted("Overlays:");
         ImGui::Separator();
         ImGui::Checkbox("Overlay selection", &m_selectionOverlay);
         ImGui::Separator();
         ImGui::TextUnformatted("Physic Overlay:");
         if (ImGui::RadioButton("None", m_physOverlay == PhysicOverlay::None))
            m_physOverlay = PhysicOverlay::None;
         if (ImGui::RadioButton("Selected", m_physOverlay == PhysicOverlay::Selected))
            m_physOverlay = PhysicOverlay::Selected;
         if (ImGui::RadioButton("All", m_physOverlay == PhysicOverlay::All))
            m_physOverlay = PhysicOverlay::All;
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_FILTER)) // Selection filter
         ImGui::OpenPopup("Selection filter Popup");
      if (ImGui::BeginPopup("Selection filter Popup"))
      {
         bool pf = HasFlag(m_selectionFilter, SelectionFilter::Playfield);
         bool prims = HasFlag(m_selectionFilter, SelectionFilter::Primitives);
         bool lights = HasFlag(m_selectionFilter, SelectionFilter::Lights);
         bool flashers = HasFlag(m_selectionFilter, SelectionFilter::Flashers);
         ImGui::TextUnformatted("Selection filters:");
         ImGui::Separator();
         if (ImGui::Checkbox("Playfield", &pf))
            m_selectionFilter = pf ? (m_selectionFilter | SelectionFilter::Playfield) : (m_selectionFilter & ~SelectionFilter::Playfield);
         if (ImGui::Checkbox("Primitives", &prims))
            m_selectionFilter = prims ? (m_selectionFilter | SelectionFilter::Primitives) : (m_selectionFilter & ~SelectionFilter::Primitives);
         if (ImGui::Checkbox("Lights", &lights))
            m_selectionFilter = lights ? (m_selectionFilter | SelectionFilter::Lights) : (m_selectionFilter & ~SelectionFilter::Lights);
         if (ImGui::Checkbox("Flashers", &flashers))
            m_selectionFilter = flashers ? (m_selectionFilter | SelectionFilter::Flashers) : (m_selectionFilter & ~SelectionFilter::Flashers);
         ImGui::EndPopup();
      }
      ImGui::End();

      // Overlay Info Text
      ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 200.f * m_liveUI.GetDPI(),
         io.DisplaySize.y - m_toolbar_height - m_menubar_height - 5.f * m_liveUI.GetDPI())); // Fixed outliner width (to be adjusted when moving ImGui to the docking branch)
      ImGui::SetNextWindowPos(ImVec2(200.f * m_liveUI.GetDPI(), m_toolbar_height + m_menubar_height + 5.f * m_liveUI.GetDPI()));
      ImGui::Begin("text overlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
      switch (m_camMode)
      {
      case ViewMode::PreviewCam: ImGui::TextUnformatted("Preview Camera"); break;
      case ViewMode::EditorCam:
      {
         string text;
         switch (m_predefinedView)
         {
         case PredefinedView::None: text = "User"sv; break;
         case PredefinedView::Left: text = "Left"sv; break;
         case PredefinedView::Right: text = "Right"sv; break;
         case PredefinedView::Top: text = "Top"sv; break;
         case PredefinedView::Bottom: text = "Bottom"sv; break;
         case PredefinedView::Front: text = "Front"sv; break;
         case PredefinedView::Back: text = "Back"sv; break;
         }
         ImGui::TextUnformatted((text + (m_perspectiveCam ? " Perspective" : " Orthographic")).c_str());
         break;
      }
      case ViewMode::DesktopBackdrop: ImGui::TextUnformatted("Desktop Backdrop"); break;
      }
      switch (m_gizmoOperation)
      {
      case ImGuizmo::OPERATION(0): ImGui::TextUnformatted("Select"); break;
      case ImGuizmo::TRANSLATE: ImGui::TextUnformatted("Grab"); break;
      case ImGuizmo::ROTATE: ImGui::TextUnformatted("Rotate"); break;
      case ImGuizmo::SCALE: ImGui::TextUnformatted("Scale"); break;
      default: break;
      }
      if (m_pointEditPart)
         ImGui::TextUnformatted("Drag Point Edit (Tab to exit)");
      ImGui::End();

      // Side panels
      UpdateOutlinerUI();
      UpdatePropertyUI();
   }

   if (m_showRendererInspection)
      UpdateRendererInspectionModal();

#endif

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
   ImGui::End();
   ImGui::PopStyleVar();
   ImGui::PopStyleColor(2);

   // Update editor camera
   const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
   const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
   if (m_camMode == ViewMode::PreviewCam)
   {
      m_renderer->InitLayout();
      m_camView = RH2LH * m_renderer->GetMVP().GetView(0) * YAxis;
      m_camProj = YAxis * m_renderer->GetMVP().GetProj(0);
   }
   else
   {
      // Apply editor camera to renderer (move view/projection from right handed to left handed)
      // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
      const Matrix3D view = RH2LH * m_camView * YAxis;
      const Matrix3D proj = YAxis * m_camProj;
      m_renderer->SetViewProj(view, proj);

      if (m_perspectiveCam && m_camMode != ViewMode::DesktopBackdrop) // The desktop backdrop is a 2D view, always edited with an orthographic camera
      {
         // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
         // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
         //const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
         //const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, -1.f);
         //float zNear, zFar;
         //m_table->ComputeNearFarPlane(RH2LH * m_camView * YAxis, 1.f, zNear, zFar);
         constexpr float zNear = 5.f;
         constexpr float zFar = 50000.f;
         m_camProj = Matrix3D::MatrixPerspectiveFovRH(39.6f, io.DisplaySize.x / io.DisplaySize.y, zNear, zFar);
      }
      else
      {
         constexpr float zNear = 0.5f;
         constexpr float zFar = 50000.f;
         const float viewHeight = m_camDistance;
         const float viewWidth = viewHeight * io.DisplaySize.x / io.DisplaySize.y;
         m_camProj = Matrix3D::MatrixOrthoOffCenterRH(-viewWidth, viewWidth, -viewHeight, viewHeight, zNear, -zFar);
      }
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
      ImGuizmo::Manipulate(camViewLH, (float *)(m_camProj.m), gizmoOperation, m_gizmoMode, (float *)(transform.m));
      if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
      {
         // Mark all selected parts for undo once per drag (m_lastUndoId is reset when the gizmo is released)
         if (m_lastUndoId != 0x1000 && m_table->m_liveBaseTable == nullptr)
         {
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
   // Reset gizmo undo deduplication once the gizmo is released (each drag should create a single undo entry)
   if (!ImGuizmo::IsUsing() && m_lastUndoId == 0x1000)
   {
      m_lastUndoPart = nullptr;
      m_lastUndoId = 0;
   }

   m_renderer->SetShadeMode(m_shadeMode);

   // Selection and physic colliders overlay
   {
      RenderContext ctx(m_player, overlayDrawList, m_camMode, m_shadeMode, (m_table->m_liveBaseTable != nullptr) && m_player->IsPlaying());
      if (!m_multiSel.empty())
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

      // In drag point edit mode, render the drag points of the edited part's curve
      if (m_pointEditPart)
      {
         const vector<CComObject<DragPoint> *> &points = m_pointEditPart->GetDragPointCurve()->GetPoints();
         for (size_t i = 0; i < points.size(); i++)
         {
            const ImVec2 pos = ctx.Project(Vertex3Ds(points[i]->m_v.x, points[i]->m_v.y, m_pointEditPart->GetDragPointZ(points[i])));
            if (pos.x == FLT_MAX)
               continue;
            const ImU32 color = IsPointSelected(points[i]) ? IM_COL32(255, 128, 0, 255) : (points[i]->m_smooth ? IM_COL32(80, 160, 255, 255) : IM_COL32(255, 96, 96, 255));
            const float radius = (i == 0 ? 5.f : 4.f) * m_liveUI.GetDPI(); // First point is drawn slightly larger to mark the curve start
            overlayDrawList->AddCircleFilled(pos, radius, color, 12);
            overlayDrawList->AddCircle(pos, radius + m_liveUI.GetDPI(), IM_COL32(0, 0, 0, 255), 12, 1.5f);
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
   }

   // Handle uncaught mouse & keyboard shortcuts
   if (!io.WantCaptureMouse)
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
            if (io.KeyShift || m_camMode == ViewMode::DesktopBackdrop)
            {
               if (!m_perspectiveCam || m_camMode == ViewMode::DesktopBackdrop)
               {
                  const float viewScale = 2.f * m_camDistance / io.DisplaySize.y;
                  drag.x *= viewScale;
                  drag.y *= viewScale;
               }
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
         if (m_addPartType != eItemInvalid)
         {
            // Add part mode: create the pending part at the picked point of the playfield XY plane
            const ItemTypeEnum type = m_addPartType;
            m_addPartType = eItemInvalid;
            CreatePart(type, UnprojectToPlane(m_boxSelectStart, 0.f));
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
                  m_pointSel.clear();
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
                  size_t selectionIndex = vhoHit.size();
                  for (size_t i = 0; i <= vhoHit.size(); i++)
                  {
                     if (i < vhoHit.size() && m_selection.GetType() == Selection::S_EDITABLE && vhoHit[i].m_obj->m_editable == m_selection.GetPart()->GetEditable())
                        selectionIndex = i + 1;
                     if (i == selectionIndex)
                     {
                        const size_t p = selectionIndex % vhoHit.size();
                        const IEditable *select = vhoHit[p].m_obj->m_editable;
                        const auto it = m_editableMap.find(select);
                        if (it != m_editableMap.end())
                        {
                           SetSelection(Selection(it->second));
                           m_outlinerAnchor = it->second;
                        }
                        else
                           ClearSelection();
                     }
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
                  m_pointDragActive = !m_pointSel.empty();
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
               DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve();
               for (DragPoint *point : m_pointSel)
               {
                  point->m_v.x += delta.x;
                  point->m_v.y += delta.y;
               }
               curve->OnPointsModified();
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
   if (!io.WantCaptureKeyboard)
   {
      if (ImGui::IsKeyReleased(ImGuiKey_Escape))
      {
         if (m_gizmoOperation != ImGuizmo::OPERATION(0))
            m_gizmoOperation = ImGuizmo::OPERATION(0); // Cancel current operation
         else if (m_boxSelectActive)
            m_boxSelectActive = false; // Cancel current box selection
         else if (m_addPartType != eItemInvalid)
            m_addPartType = eItemInvalid; // Cancel add part mode
         else if (m_pointEditPart)
            ExitPointEditMode(true); // Exit drag point edit mode
         else if (m_selection.GetType() != Selection::S_NONE)
            ClearSelection(); // Cancel current selection
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Tab, false) && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      {
         // Toggle drag point edit mode on the active selected part
         if (m_pointEditPart)
            ExitPointEditMode(true);
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
            if (m_pointEditPart && !io.WantCaptureMouse)
               AddPointOnNearestSegment();
         }
         else if (!io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
         {
            // Select all: all curve points in drag point edit mode, all pickable parts otherwise
            if (m_pointEditPart)
            {
               m_pointSel.clear();
               for (CComObject<DragPoint> *point : m_pointEditPart->GetDragPointCurve()->GetPoints())
                  m_pointSel.push_back(point);
            }
            else
               SelectAllParts();
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
            if (io.KeyAlt)
            { // Unhide all
               for (auto &part : m_editables)
                  if (part->GetEditable()->GetItemType() != eItemPartGroup && (part->GetEditable()->m_desktopBackdrop == (m_camMode == ViewMode::DesktopBackdrop)))
                     part->GetEditable()->SetUIVisible(true);
            }
            else if (io.KeyShift)
            { // Hide unselected
               for (auto &part : m_editables)
                  if (part->GetEditable()->GetItemType() != eItemPartGroup && (part->GetEditable()->m_desktopBackdrop == (m_camMode == ViewMode::DesktopBackdrop)) && !IsPartSelected(part))
                     part->GetEditable()->SetUIVisible(false);
            }
            else
            { // Hide selected
               for (const auto &part : m_multiSel)
                  part->GetEditable()->SetUIVisible(false);
               ClearSelection();
            }
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
         {
            m_gizmoOperation = ImGuizmo::TRANSLATE;
            m_gizmoMode = m_gizmoOperation == ImGuizmo::TRANSLATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
         if (io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
         {
            // Save table
            if (!IsInspectMode() && !m_table->IsLocked())
            {
               VPXFileFeedback feedback;
               if (SUCCEEDED(m_table->Save(feedback)))
                  m_undo.SetCleanPoint(eSaveClean);
            }
         }
         else if (!io.KeyCtrl)
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
            {
               m_gizmoOperation = ImGuizmo::SCALE;
               m_gizmoMode = m_gizmoOperation == ImGuizmo::SCALE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
            }
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
         {
            m_gizmoOperation = ImGuizmo::ROTATE;
            m_gizmoMode = m_gizmoOperation == ImGuizmo::ROTATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
         }
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
               m_undo.Undo();
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
         // Editor Camera center on selection
         Matrix3D tmp;
         if (GetSelectionTransform(tmp))
         {
            m_camMode = ViewMode::EditorCam;
            Matrix3D view(m_camView);
            view.Invert();
            const vec3 up = view.GetOrthoNormalUp();
            const vec3 dir = view.GetOrthoNormalDir();
            const vec3 newTarget(tmp._41, tmp._42, -tmp._43);
            const vec3 newEye = newTarget + dir * m_camDistance;
            m_camView = Matrix3D::MatrixLookAtRH(newEye, newTarget, up);
         }
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

   if (m_multiSel != previousMultiSel)
   {
      for (const auto &part : previousMultiSel)
      {
         IEditable *edit = part->GetEditable();
         if (std::ranges::find(m_multiSel, part) == m_multiSel.end() // Not selected anymore
            && FindIndexOf(m_table->GetParts(), edit) != -1 // Not deleted
            && (edit->GetIHitable() != nullptr) && (edit->GetItemType() != eItemBall))
            m_player->m_physics->SetStatic(edit);
      }
      for (const auto &part : m_multiSel)
      {
         IEditable *edit = part->GetEditable();
         if (std::ranges::find(previousMultiSel, part) == previousMultiSel.end() // Newly selected
            && (edit->GetIHitable() != nullptr) && (edit->GetItemType() != eItemBall))
            m_player->m_physics->SetDynamic(edit);
      }
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
   m_outlinerAnchor.reset();
}

void EditorUI::SetSelection(const Selection &selection)
{
   m_selection = selection;
   m_multiSel.clear();
   if (selection.GetType() == Selection::S_EDITABLE)
      m_multiSel.push_back(selection.GetPart());
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

void EditorUI::RayCastParts(const ImVec2 &mousePos, vector<HitTestResult> &vhoHit) const
{
   // Compute mouse position in clip space
   const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
   const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
   const float xcoord = (mousePos.x - rClipWidth) / rClipWidth;
   const float ycoord = (rClipHeight - mousePos.y) / rClipHeight;

   // Use the inverse of our 3D transform to determine where in 3D space the
   // screen pixel the user clicked on is at.  Get the point at the near
   // clipping plane (z=0) and the far clipping plane (z=1) to get the whole
   // range we need to hit test
   Matrix3D invMVP = m_renderer->GetMVP().GetModelViewProj(0);
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
   const RenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, false);
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
      m_table->GetUniqueName(type, scriptable->m_wzName);
   pie->m_desktopBackdrop = (m_camMode == ViewMode::DesktopBackdrop);
   m_table->AddPart(pie);
   // Assign the part to the group of the current selection, defaulting to the first root part group
   PartGroup *partGroup = nullptr;
   if (m_selection.GetType() == Selection::S_EDITABLE)
      partGroup = m_selection.GetPart()->GetEditable()->GetItemType() == eItemPartGroup ? static_cast<PartGroup *>(m_selection.GetPart()->GetEditable())
                                                                                     : m_selection.GetPart()->GetEditable()->GetPartGroup();
   if (partGroup == nullptr)
      for (IEditable *const part : m_table->GetParts())
         if (part->GetItemType() == eItemPartGroup && part->GetPartGroup() == nullptr)
            partGroup = static_cast<PartGroup *>(part);
   pie->SetPartGroup(partGroup);

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

void EditorUI::DeleteSelection()
{
   if (m_table->IsLocked())
      return;
   const vector<std::shared_ptr<EditorUIPart>> parts(m_multiSel); // Work on a copy since parts are removed while iterating
   for (const auto &part : parts)
   {
      if (part->GetEditable()->GetItemType() == eItemBall || part->GetEditable()->GetPartGroup() == nullptr)
         continue;
      IEditable *const edit = part->GetEditable();
      RemoveFromVectorSingle(m_editables, part);
      m_editableMap.erase(edit);
      if (edit->GetIHitable())
         m_player->m_physics->Remove(edit);
      m_table->RemovePart(edit);
      m_renderer->m_renderDevice->AddEndOfFrameCmd([edit, player=m_player]()
         {
            if (edit->GetIRenderable())
               edit->GetIRenderable()->RenderRelease();
            if (edit->m_phittimer && edit->m_timerEnabled)
               player->TimerStateChange(edit->m_phittimer.get(), false);
            edit->m_phittimer = nullptr;
            edit->Release();
         });
   }
   // Remove the deleted parts from the selection (non deletable parts stay selected)
   std::erase_if(m_multiSel, [this](const auto &part) { return std::ranges::find(m_editables, part) == m_editables.end(); });
   if (m_selection.GetType() == Selection::S_EDITABLE && !IsPartSelected(m_selection.GetPart()))
      m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   if (m_outlinerAnchor && std::ranges::find(m_editables, m_outlinerAnchor) == m_editables.end())
      m_outlinerAnchor.reset();
}

void EditorUI::UpdateEditableList()
{
   // Remove UI parts of removed editables
   const ankerl::unordered_dense::set<const IEditable *> liveParts(m_table->GetParts().begin(), m_table->GetParts().end());
   std::erase_if(m_editables,
      [this, &liveParts](const auto &uiPart)
      {
         if (!liveParts.contains(uiPart->GetEditable()))
         {
            m_editableMap.erase(uiPart->GetEditable());
            return true;
         }
         return false;
      });
   // Drop removed parts from the multi selection, keeping a valid active part
   std::erase_if(m_multiSel, [&liveParts](const auto &part) { return !liveParts.contains(part->GetEditable()); });
   if (m_selection.GetType() == Selection::S_EDITABLE && !IsPartSelected(m_selection.GetPart()))
      m_selection = m_multiSel.empty() ? Selection() : Selection(m_multiSel.back());
   if (m_outlinerAnchor && !liveParts.contains(m_outlinerAnchor->GetEditable()))
      m_outlinerAnchor.reset();
   // Add UI parts for new editables
   bool needSort = false;
   ankerl::unordered_dense::set<PartGroup *> newGroups;
   for (const auto &edit : m_table->GetParts())
   {
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
      else if (!it->second->GetOutlinerPath().ends_with(edit->GetName())) // Name and therefore outliner path has changed
      {
         needSort = true;
         if (edit->GetItemType() == eItemPartGroup) // Also update all children
            for (const auto &uiPart : m_editables)
               uiPart->SetOutlinerPath(uiPart->GetEditable()->GetPathString(false));
         else
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
   // Sort according to outliner path to ease its rendering
   if (needSort)
      std::ranges::sort(m_editables,
         [this](const auto &a, const auto &b)
         {
            const bool isRootA = a->GetEditable()->GetPartGroup() == nullptr && a->GetEditable()->GetItemType() != eItemPartGroup;
            const bool isRootB = b->GetEditable()->GetPartGroup() == nullptr && b->GetEditable()->GetItemType() != eItemPartGroup;
            return (isRootA != isRootB) ? isRootA : (a->GetOutlinerPath() < b->GetOutlinerPath());
         });
}

bool EditorUI::GetSelectionTransform(Matrix3D &transform) const
{
   if (m_pointEditPart)
   {
      // In drag point edit mode, the gizmo operates on the selected points (positioned at their bounding box center)
      if (m_pointSel.empty())
         return false;
      float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX, z = 0.f;
      for (const DragPoint *point : m_pointSel)
      {
         minX = min(minX, point->m_v.x);
         maxX = max(maxX, point->m_v.x);
         minY = min(minY, point->m_v.y);
         maxY = max(maxY, point->m_v.y);
         z += m_pointEditPart->GetDragPointZ(point);
      }
      transform = Matrix3D::MatrixTranslate(0.5f * (minX + maxX), 0.5f * (minY + maxY), z / (float)m_pointSel.size());
      return true;
   }
   if (m_selection.GetType() == Selection::S_EDITABLE)
   {
      const EditorUIPart::TransformMask mask = m_selection.GetPart()->GetTransform(transform);
      return mask != EditorUIPart::TransformMask::TM_None;
   }
   return false;
}

void EditorUI::SetSelectionTransform(const Matrix3D &newTransform, bool clearPosition, bool clearScale, bool clearRotation) const
{
   if (m_pointEditPart)
   {
      // In drag point edit mode, apply the gizmo transform delta to the selected points in the table XY plane
      if (m_pointSel.empty())
         return;
      Matrix3D oldTransform;
      GetSelectionTransform(oldTransform);
      Matrix3D invOldTransform(oldTransform);
      invOldTransform.Invert();
      const Matrix3D delta = newTransform * invOldTransform;
      for (DragPoint *point : m_pointSel)
      {
         const Vertex3Ds v = delta * point->m_v;
         point->m_v.x = v.x;
         point->m_v.y = v.y;
      }
      DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve();
      curve->OnPointsModified();
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

bool EditorUI::MatchesOutlinerFilter(const string &name) const
{
   if (m_outlinerFilter.empty())
      return true;
   const string name_lcase = lowerCase(name);
   const string filter_lcase = lowerCase(m_outlinerFilter);
   return name_lcase.find(filter_lcase) != std::string::npos;
}

void EditorUI::UpdateOutlinerUI()
{
   if (m_table->IsLocked())
      return;

   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = 200.f * m_liveUI.GetDPI();
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
      | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_liveUI.GetDPI(), 4.0f * m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("OUTLINER", nullptr, window_flags);

   const float clearFilterWidth = ImGui::CalcTextSize(ICON_FK_TIMES).x + ImGui::GetStyle().FramePadding.x * 2.0f;
   ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - clearFilterWidth - ImGui::GetStyle().ItemSpacing.x);
   ImGui::InputTextWithHint("##OutlinerFilter", "Name part filter", &m_outlinerFilter);
   ImGui::SameLine();
   ImGui::BeginDisabled(m_outlinerFilter.empty());
   if (ImGui::Button(ICON_FK_TIMES "##ClearOutlinerFilter"))
      m_outlinerFilter.clear();
   ImGui::EndDisabled();

   if (ImGui::TreeNodeEx("View Setups"))
   {
      if (ImGui::Selectable("Editor Camera"))
      {
         ClearSelection();
         m_camMode = ViewMode::EditorCam;
      }
      const Selection cam0 = Selection::Camera(0);
      if (ImGui::Selectable("Preview: Desktop", m_selection == cam0))
      {
         SetSelection(cam0);
         m_camMode = ViewMode::PreviewCam;
         m_table->SetViewSetupOverride(BG_DESKTOP);
      }
      const Selection cam1 = Selection::Camera(1);
      if (ImGui::Selectable("Preview: Cabinet", m_selection == cam1))
      {
         SetSelection(cam1);
         m_camMode = ViewMode::PreviewCam;
         m_table->SetViewSetupOverride(BG_FULLSCREEN);
      }
      const Selection cam2 = Selection::Camera(2);
      if (ImGui::Selectable("Preview: Full Single Screen", m_selection == cam2))
      {
         SetSelection(cam2);
         m_camMode = ViewMode::PreviewCam;
         m_table->SetViewSetupOverride(BG_FSS);
      }
      ImGui::TreePop();
   }
   if (ImGui::TreeNode("Materials"))
   {
      const std::function<string(Material *)> map = [](Material *image) -> string { return image->m_name; };
      for (Material *&material : SortedCaseInsensitive(m_table->m_materials, map))
      {
         Selection sel(material);
         if (MatchesOutlinerFilter(material->m_name) && ImGui::Selectable(material->m_name.c_str(), m_selection == sel))
            SetSelection(sel);
      }
      ImGui::TreePop();
   }
   if (ImGui::TreeNode("Images"))
   {
      const std::function<string(Texture *)> map = [](Texture *image) -> string { return image->m_name; };
      for (Texture *&image : SortedCaseInsensitive(m_table->m_vimage, map))
      {
         Selection sel(image);
         if (MatchesOutlinerFilter(image->m_name) && ImGui::Selectable(image->m_name.c_str(), m_selection == sel))
            SetSelection(sel);
      }
      ImGui::TreePop();
   }
   if (ImGui::TreeNode("Render Probes"))
   {
      for (RenderProbe *probe : m_table->m_vrenderprobe)
      {
         Selection sel(probe);
         if (ImGui::Selectable(probe->GetName().c_str(), m_selection == sel))
            SetSelection(sel);
      }
      ImGui::TreePop();
   }
   if (ImGui::TreeNodeEx("Layers", ImGuiTreeNodeFlags_DefaultOpen))
   {
      // Table definition parts
      struct Node
      {
         PartGroup *group;
         bool opened;
      };
      // When a filter is applied, only display groups that match it or contain a matching part in their subtree
      ankerl::unordered_dense::set<const PartGroup *> matchedGroups;
      ankerl::unordered_dense::set<const PartGroup *> visibleGroups;
      if (!m_outlinerFilter.empty())
      {
         vector<PartGroup *> groupStack;
         for (const auto &edit : m_editables)
         {
            IEditable *const editable = edit->GetEditable();
            while (!groupStack.empty() && (editable->GetPartGroup() == nullptr || !editable->IsChild(groupStack.back())))
               groupStack.pop_back();
            if (MatchesOutlinerFilter(editable->GetName())
               || (editable->GetItemType() != eItemPartGroup && std::ranges::any_of(groupStack, [&matchedGroups](const PartGroup *group) { return matchedGroups.contains(group); })))
            {
               for (PartGroup *ancestor : groupStack)
                  visibleGroups.insert(ancestor);
               if (editable->GetItemType() == eItemPartGroup)
               {
                  matchedGroups.insert(static_cast<PartGroup *>(editable));
                  visibleGroups.insert(static_cast<PartGroup *>(editable));
               }
            }
            if (editable->GetItemType() == eItemPartGroup)
               groupStack.push_back(static_cast<PartGroup *>(editable));
         }
      }
      vector<Node> stack;
      int outlinerItem = 0;
      const float eyeX = ImGui::GetContentRegionAvail().x;
      for (const auto &edit : m_editables)
      {
         const PartGroup *parent = edit->GetEditable()->GetPartGroup();
         while (!stack.empty()
            && ((parent == nullptr && (edit->GetEditable()->GetItemType() == eItemPartGroup)) // Root partgroup: pop all
               || (parent == nullptr && (edit->GetEditable()->GetItemType() != eItemPartGroup) && (stack.back().group != nullptr)) // Live object: pop all unless in 'Live Object' group
               || (parent != nullptr && (!edit->GetEditable()->IsChild(stack.back().group))))) // Child object: pop up to the parent group
         {
            if (stack.back().opened)
               ImGui::TreePop();
            stack.pop_back();
         }
         // TODO allow selection => ImGuiTreeNodeFlags_Selected
         // TODO support empty nodes => ImGuiTreeNodeFlags_Leaf
         if (edit->GetEditable()->GetItemType() == eItemPartGroup && !m_outlinerFilter.empty() && !visibleGroups.contains(static_cast<PartGroup *>(edit->GetEditable())))
            continue;
         // In desktop backdrop mode, only display backdrop parts, and conversely in the other view modes
         if (edit->GetEditable()->GetItemType() != eItemPartGroup && (edit->GetEditable()->m_desktopBackdrop != (m_camMode == ViewMode::DesktopBackdrop)))
            continue;
         ImGui::AlignTextToFramePadding();
         if (edit->GetEditable()->GetItemType() == eItemPartGroup)
         {
            PartGroup *group = static_cast<PartGroup *>(edit->GetEditable());
            const bool opened = ImGui::TreeNodeEx(edit->GetEditable()->GetName().c_str(), ImGuiTreeNodeFlags_AllowOverlap);
            if (m_table->m_liveBaseTable == nullptr)
            {
               ImGui::SameLine(eyeX);
               ImGui::PushStyleColor(ImGuiCol_Text, group->IsUIVisible(false) ? IM_COL32_WHITE : IM_COL32(128, 128, 128, 255));
               if (ImGui::SmallButton(((group->IsUIVisible(false) ? ICON_FK_EYE : ICON_FK_EYE_SLASH) + "##Eye__"s + edit->GetEditable()->GetName()).c_str()))
                  group->SetUIVisible(!group->IsUIVisible(false));
               ImGui::PopStyleColor();
            }
            stack.emplace_back(static_cast<PartGroup *>(edit->GetEditable()), (stack.empty() || stack.back().opened) ? opened : false);
         }
         else
         {
            const bool show = MatchesOutlinerFilter(edit->GetEditable()->GetName())
               || std::ranges::any_of(stack, [&matchedGroups](const Node &node) { return node.group != nullptr && matchedGroups.contains(node.group); });
            if (parent == nullptr && stack.empty() && show)
               stack.push_back({ nullptr, ImGui::TreeNodeEx("[Live Objects]", ImGuiTreeNodeFlags_AllowOverlap) });
            if (!stack.empty() && stack.back().opened)
            {
               Selection sel(edit);
               if (show)
               {
                  if (ImGui::Selectable((edit->GetEditable()->GetName() + "##Outliner"s + std::to_string(outlinerItem++)).c_str(), IsPartSelected(edit), ImGuiSelectableFlags_AllowOverlap))
                  {
                     const ImGuiIO &selIO = ImGui::GetIO();
                     if (selIO.KeyCtrl)
                     {
                        TogglePartSelection(edit);
                        m_outlinerAnchor = edit;
                     }
                     else if (selIO.KeyShift)
                        SelectOutlinerRange(edit);
                     else
                     {
                        SetSelection(sel);
                        m_outlinerAnchor = edit;
                     }
                  }
                  IEditable* editable = edit->GetEditable();
                  if (editable && m_table->m_liveBaseTable == nullptr)
                  {
                     ImGui::SameLine(eyeX);
                     ImGui::PushStyleColor(ImGuiCol_Text, editable->IsUIVisible(false) ? IM_COL32_WHITE : IM_COL32(128, 128, 128, 255));
                     if (ImGui::SmallButton(((editable->IsUIVisible(false) ? ICON_FK_EYE : ICON_FK_EYE_SLASH) + "##Eye__"s + edit->GetEditable()->GetName()).c_str()))
                        editable->SetUIVisible(!editable->IsUIVisible(false));
                     ImGui::PopStyleColor();
                  }
               }
            }
         }
      }
      while (!stack.empty())
      {
         if (stack.back().opened)
            ImGui::TreePop();
         stack.pop_back();
      }
      ImGui::TreePop();
   }

   ImGui::End();
   ImGui::PopStyleVar(3);
}

void EditorUI::UpdatePropertyUI()
{
   if (m_table->IsLocked())
      return;

   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float pane_width = 280.f * m_liveUI.GetDPI();
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + viewport->Size.x - pane_width, viewport->Pos.y + m_menubar_height + m_toolbar_height));
   ImGui::SetNextWindowSize(ImVec2(pane_width, viewport->Size.y - m_menubar_height - m_toolbar_height));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f * m_liveUI.GetDPI(), 4.0f * m_liveUI.GetDPI()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
   ImGui::Begin("PROPERTIES", nullptr,
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
         | ImGuiWindowFlags_NoNavFocus);

   if (m_multiSel.size() > 1)
      ImGui::TextDisabled("%d parts selected (editing the active one)", (int)m_multiSel.size());

   PropertyPane props(m_table);
   switch (m_units)
   {
   case Units::VPX: props.SetLengthUnit(PropertyPane::Unit::VPLength); break;
   case Units::Metric: props.SetLengthUnit(PropertyPane::Unit::Millimeters); break;
   case Units::Imperial: props.SetLengthUnit(PropertyPane::Unit::Inches); break;
   }
   if (IsInspectMode() && m_selection.GetType() != Selection::S_IMAGE) // Images are shared between live and startup instance, so they do not have 2 states
   {
      if (ImGui::BeginTabBar("Startup/Live", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
      {
         for (int tab = 0; tab < 2; tab++)
         {
            const bool is_live = (tab == 1);
            if (ImGui::BeginTabItem(is_live ? "Live" : "Startup", nullptr, (is_live && m_propertiesSelectLiveTab) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
            {
               if (is_live)
                  m_propertiesSelectLiveTab = false;
               props.SetShowStartup(!is_live);
               switch (m_selection.GetType())
               {
               case Selection::S_NONE: TableProperties(props); break;
               case Selection::S_EDITABLE:
                  m_selection.GetPart()->UpdatePropertyPane(props);
                  if (props.GetModifiedField() > 0)
                  {
                     m_renderer->ReinitRenderable(m_selection.GetPart()->GetEditable()->GetIRenderable());
                     m_player->m_physics->Update(m_selection.GetPart()->GetEditable());
                  }
                  break;
               case Selection::S_IMAGE: ImageProperties(props, m_selection.GetImage()); break;
               case Selection::S_CAMERA: CameraProperties(props, m_selection.GetCamera()); break;
               case Selection::S_MATERIAL: MaterialProperties(props, m_selection.GetMaterial()); break;
               case Selection::S_RENDERPROBE: RenderProbeProperties(props, m_selection.GetProbe()); break;
               }
               ImGui::EndTabItem();
            }
         }
         ImGui::EndTabBar();
      }
   }
   else
   {
      switch (m_selection.GetType())
      {
      case Selection::S_NONE: TableProperties(props); break;
      case Selection::S_EDITABLE:
         m_undo.BeginUndo();
         m_undo.MarkForUndo(m_selection.GetPart()->GetEditable());
         m_undo.EndUndo();
         m_selection.GetPart()->UpdatePropertyPane(props);
         if (props.GetModifiedField() > 0 && (m_lastUndoPart != m_selection.GetPart()->GetEditable() || m_lastUndoId != (0x2000 | props.GetModifiedField())))
         {
            m_lastUndoPart = m_selection.GetPart()->GetEditable();
            m_lastUndoId = 0x2000 | props.GetModifiedField();
         }
         else
         {
            m_undo.Discard();
         }
         if (props.GetModifiedField() > 0)
         {
            m_renderer->ReinitRenderable(m_selection.GetPart()->GetEditable()->GetIRenderable());
            m_player->m_physics->Update(m_selection.GetPart()->GetEditable());
         }
         break;
      case Selection::S_IMAGE: ImageProperties(props, m_selection.GetImage()); break;
      case Selection::S_CAMERA: CameraProperties(props, m_selection.GetCamera()); break;
      case Selection::S_MATERIAL: MaterialProperties(props, m_selection.GetMaterial()); break;
      case Selection::S_RENDERPROBE: RenderProbeProperties(props, m_selection.GetProbe()); break;
      }
   }

   ImGui::End();
   ImGui::PopStyleVar(3);
}

void EditorUI::UpdateRendererInspectionModal()
{
   // FIXME m_renderer->DisableStaticPrePass(false);
   m_camMode = ViewMode::PreviewCam;

   static int pass_selection = IF_FPS;
   ImGui::SetNextWindowSize(ImVec2(350.f * m_liveUI.GetDPI(), 0));
   if (ImGui::Begin(ID_RENDERER_INSPECTION, &m_showRendererInspection))
   {
      ImGui::TextUnformatted("Display single render pass:");
      ImGui::RadioButton("Disabled", &pass_selection, IF_FPS);
#if defined(ENABLE_DX9) // No GPU profiler for OpenGL or BGFX for the time being
      ImGui::RadioButton("Profiler", &pass_selection, IF_PROFILING);
#endif
      ImGui::RadioButton("Static prerender pass", &pass_selection, IF_STATIC_ONLY);
      ImGui::RadioButton("Dynamic render pass", &pass_selection, IF_DYNAMIC_ONLY);
      ImGui::RadioButton("Transmitted light pass", &pass_selection, IF_LIGHT_BUFFER_ONLY);
      if (m_player->m_renderer->GetAOMode() != 0)
         ImGui::RadioButton("Ambient Occlusion pass", &pass_selection, IF_AO_ONLY);
      for (size_t i = 0; i < m_table->m_vrenderprobe.size(); i++)
      {
         ImGui::RadioButton(m_table->m_vrenderprobe[i]->GetName().c_str(), &pass_selection, 100 + (int)i);
      }
      if (pass_selection < 100)
         m_player->m_infoMode = (InfoMode)pass_selection;
      else
      {
         m_player->m_infoMode = IF_RENDER_PROBES;
         m_player->m_infoProbeIndex = pass_selection - 100;
      }
      ImGui::NewLine();

      // Latency timing table
      if (ImGui::BeginTable("Latencies", 4, ImGuiTableFlags_Borders))
      {
         const uint32_t period = m_player->m_renderProfiler->GetPrev(FrameProfiler::PROFILE_FRAME);
         ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Min", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Max", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableSetupColumn("Avg", ImGuiTableColumnFlags_WidthFixed);
         ImGui::TableHeadersRow();
#define PROF_ROW(name, section)                                                                                                                                                              \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::TextUnformatted(name);                                                                                                                                                             \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingMin(section) * 1e-3);                                                                                                          \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingMax(section) * 1e-3);                                                                                                          \
   ImGui::TableNextColumn();                                                                                                                                                                 \
   ImGui::Text("%4.1fms", m_player->m_logicProfiler.GetSlidingAvg(section) * 1e-3);
         PROF_ROW("Input to Script lag", FrameProfiler::PROFILE_INPUT_POLL_PERIOD)
         PROF_ROW("Input to Present lag", FrameProfiler::PROFILE_INPUT_TO_PRESENT)
#undef PROF_ROW
         ImGui::EndTable();
         ImGui::NewLine();
      }

      /* ImGui::TextUnformatted("Press F11 to reset min/max/average timings");
      if (ImGui::IsKeyPressed(LiveUI::GetImGuiKeyFromSDLScancode(m_player->m_actionToSDLScanCodeMapping[eFrameCount])))
         m_player->InitFPS();*/

      // Other detailed information
      ImGui::TextUnformatted(m_player->GetPerfInfo().c_str());
   }
   ImGui::End();

   // Restore default rendering when the modal is closed (leaving the render pass override active would stick into gameplay)
   if (!m_showRendererInspection)
   {
      pass_selection = IF_FPS;
      m_player->m_infoMode = IF_FPS;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property panes
//

void EditorUI::TableProperties(PropertyPane &props)
{
   PinTable *table = props.GetEditedPart<PinTable>(m_table);
   props.Header("Table"s, [table]() { return table->GetWName(); }, [table](const wstring &v) { table->SetName(v); });

   if (props.BeginSection("User Settings"s))
   {

      props.EndSection();
   }

   if (props.BeginSection("Visuals"s))
   {

      props.EndSection();
   }

   if (props.BeginSection("Physics"s))
   {

      props.EndSection();
   }

   if (props.BeginSection("Lighting"s))
   {
      props.InputRGB<PinTable>(
         table, "Ambient Color", //
         [](const PinTable *table) { return convertColor(table->m_lightAmbient); }, //
         [this](PinTable *table, const vec3 &v)
         {
            table->m_lightAmbient = convertColorRGB(v);
            m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });

      props.InputRGB<PinTable>(
         table, "Light Em. Color", //
         [](const PinTable *table) { return convertColor(table->m_Light[0].emission); }, //
         [this](PinTable *table, const vec3 &v)
         {
            table->m_Light[0].emission = convertColorRGB(v);
            m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         });
      props.InputFloat<PinTable>(
         table, "Light Em. Scale"s, //
         [](const PinTable *table) { return table->m_lightEmissionScale; }, //
         [this](PinTable *table, float v)
         {
            table->m_lightEmissionScale = v;
            m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::None, 0);
      props.InputFloat<PinTable>(
         table, "Light Height"s, //
         [](const PinTable *table) { return table->m_lightHeight; }, //
         [this](PinTable *table, float v)
         {
            table->m_lightHeight = v;
            m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::VPLength, 1);
      props.InputFloat<PinTable>(
         table, "Light Range"s, //
         [](const PinTable *table) { return table->m_lightRange; }, //
         [this](PinTable *table, float v)
         {
            table->m_lightRange = v;
            m_renderer->MarkShaderDirty(); // Needed to update shaders with new light settings
         },
         PropertyPane::Unit::VPLength, 1);

      // TODO Missing: environment texture combo

      props.InputFloat<PinTable>(
         table, "Environment Em. Scale"s, //
         [](const PinTable *table) { return table->m_envEmissionScale; }, //
         [this](PinTable *table, float v) { table->m_envEmissionScale = v; }, PropertyPane::Unit::Percent, 3);
      props.InputFloat<PinTable>(
         table, "Ambient Occlusion Scale"s, //
         [](const PinTable *table) { return table->m_AOScale; }, //
         [this](PinTable *table, float v) { table->m_AOScale = v; }, PropertyPane::Unit::Percent, 1);
      props.InputFloat<PinTable>(
         table, "Bloom Strength"s, //
         [](const PinTable *table) { return table->m_bloom_strength; }, //
         [this](PinTable *table, float v) { table->m_bloom_strength = v; }, PropertyPane::Unit::Percent, 1);
      props.InputFloat<PinTable>(
         table, "Screen Space Reflection Scale"s, //
         [](const PinTable *table) { return table->m_SSRScale; }, //
         [this](PinTable *table, float v) { table->m_SSRScale = v; }, PropertyPane::Unit::Percent, 1);

      // TODO Missing: tonemapper
      // TODO Missing: exposure

      props.EndSection();
   }
}

void EditorUI::CameraProperties(PropertyPane &props, int bgSet)
{
   ImGui::BeginDisabled(true);
   props.Header("Camera"s, [bgSet]() { return bgSet == 0 ? L"Desktop"s : bgSet == 1 ? L"Cabinet"s : L"Full Single Screen"s; }, [](const wstring &) {});
   ImGui::EndDisabled();

   {
      if (ImGui::Button("Import"))
      {
         m_table->ImportBackdropPOV(m_table->GetSettingsFileName(), false);
         m_renderer->MarkShaderDirty();
      }
      ImGui::SameLine();
      if (ImGui::Button("Export"))
         m_table->ExportBackdropPOV(m_table->GetSettingsFileName());
      ImGui::NewLine();
   }

   if (props.BeginSection("Visuals"s))
   {
      ViewSetup *const vs = &props.GetEditedPart<PinTable>(m_table)->mViewSetups[bgSet];
      props.Combo<ViewSetup>(
         vs, "View Mode"s, vector { "Legacy"s, "Camera"s, "Window"s }, //
         [](const ViewSetup *viewSetup) { return static_cast<int>(viewSetup->mMode); }, //
         [](ViewSetup *viewSetup, int v) { viewSetup->mMode = static_cast<ViewLayoutMode>(v); });
      props.InputFloat<ViewSetup>(
         vs, "Field of View"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mFOV; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mFOV = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<ViewSetup>(
         vs, "Layback"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mLayback; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mLayback = v; }, PropertyPane::Unit::Degree, 1);
      props.InputFloat<ViewSetup>(
         vs, "Look At"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mLookAt; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mLookAt = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<ViewSetup>(
         vs, "X Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewX; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewX = v; }, PropertyPane::Unit::None, 0);
      props.InputFloat<ViewSetup>(
         vs, "Y Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewY; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewY = v; }, PropertyPane::Unit::None, 0);
      props.InputFloat<ViewSetup>(
         vs, "Z Offset"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewZ; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewZ = v; }, PropertyPane::Unit::None, 0);
      props.InputFloat<ViewSetup>(
         vs, "Rotation"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mViewportRotation; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mViewportRotation = v; }, PropertyPane::Unit::Degree, 0);
      props.InputFloat<ViewSetup>(
         vs, "X Scale"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mSceneScaleX; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mSceneScaleX = v; }, PropertyPane::Unit::Percent, 3);
      props.InputFloat<ViewSetup>(
         vs, "Y Scale"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mSceneScaleY; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mSceneScaleY = v; }, PropertyPane::Unit::Percent, 3);
      props.InputFloat<ViewSetup>(
         vs, "Z Scale"s, //
         [](const ViewSetup *viewSetup) { return viewSetup->mSceneScaleZ; }, //
         [](ViewSetup *viewSetup, float v) { viewSetup->mSceneScaleZ = v; }, PropertyPane::Unit::Percent, 3);
      props.EndSection();
   }
}

void EditorUI::ImageProperties(PropertyPane &props, Texture *texture)
{
   ImGui::BeginDisabled(m_table->m_liveBaseTable != nullptr); // Disable edition in inspection mode as images are shared between startup & inspected table

   props.Header("Image"s, [texture]() { return MakeWString(texture->m_name); }, [texture](const wstring &v) { texture->m_name = MakeString(v); });

   ImTextureID image = m_renderer->m_renderDevice->m_texMan.LoadTexture(texture, false);
   if (image)
   {
      if (props.BeginSection("Visuals"s))
      {
         std::shared_ptr<const BaseTexture> tex = texture->GetRawBitmap(false, 0);

         ImGui::BeginDisabled(tex == nullptr || !tex->HasAlpha());
         props.InputFloat<Texture>(
            m_selection.GetImage(), "Alpha Mask", //
            [](const Texture *image) { return image->m_alphaTestValue; }, //
            [](Texture *image, float v) { image->m_alphaTestValue = v; }, PropertyPane::Unit::None, 2);
         ImGui::EndDisabled();

         const string info
            = std::to_string(image->GetWidth()) + 'x' + std::to_string(image->GetHeight()) + ' ' + ((tex != nullptr && tex->m_format) ? BaseTexture::GetFormatString(tex->m_format) : ""s);
         props.Separator(info);

         props.EndSection();

         const float w = ImGui::GetWindowWidth();
         ImGui::Image(image, ImVec2(w, static_cast<float>(image->GetHeight()) * w / static_cast<float>(image->GetWidth())));
      }
   }
   else
   {
      ImGui::Text("Failed to load image");
   }

   ImGui::EndDisabled();
}

void EditorUI::RenderProbeProperties(PropertyPane &props, RenderProbe *probe)
{
   RenderProbe *editedProbe = props.GetEditedPart<RenderProbe>(probe);
   props.Header("Render Probe"s, [editedProbe]() { return MakeWString(editedProbe->GetName()); }, [editedProbe](const wstring &v) { editedProbe->SetName(MakeString(v)); });

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<RenderProbe>(
         probe, "Type"s, vector { "Reflection"s, "Refraction"s }, //
         [](const RenderProbe *probe) { return static_cast<int>(probe->GetType()); }, //
         [](RenderProbe *probe, int v) { probe->SetType(static_cast<RenderProbe::ProbeType>(v)); });
      props.InputFloat3<RenderProbe>(
         probe, "Normal"s, //
         [](const RenderProbe *probe) { return probe->GetReflectionPlaneNormal(); }, //
         [](RenderProbe *probe, const vec3 &v) { probe->SetReflectionPlaneNormal(v); }, PropertyPane::Unit::None, 2);
      props.InputFloat<RenderProbe>(
         probe, "Distance"s, //
         [](const RenderProbe *probe) { return probe->GetReflectionPlaneDistance(); }, //
         [](RenderProbe *probe, float v) { probe->SetReflectionPlaneDistance(v); }, PropertyPane::Unit::VPLength, 1);
      props.EndSection();
   }

   if (ImGui::CollapsingHeader("Users", ImGuiTreeNodeFlags_DefaultOpen))
   {
      // Add a white line above
      const ImVec2 headerMin = ImGui::GetItemRectMin();
      const ImVec2 headerMax = ImGui::GetItemRectMax();
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      const ImVec2 lineStart(headerMin.x, headerMin.y);
      const ImVec2 lineEnd(headerMax.x, headerMin.y);
      drawList->AddLine(lineStart, lineEnd, ImGui::GetColorU32(ImGuiCol_Text), 1.0f);

      for (const IEditable *editable : m_table->GetParts())
      {
         if (editable->GetItemType() != eItemPrimitive)
            continue;
         const Primitive *const primitive = static_cast<const Primitive *>(editable);
         if ((probe->GetType() == RenderProbe::PLANE_REFLECTION) && (primitive->m_d.m_szReflectionProbe != probe->GetName()))
            continue;
         if ((probe->GetType() == RenderProbe::SCREEN_SPACE_TRANSPARENCY) && (primitive->m_d.m_szRefractionProbe != probe->GetName()))
            continue;
         const auto it = m_editableMap.find(editable);
         if (it == m_editableMap.end())
            continue;
         if (ImGui::Selectable(primitive->GetName().c_str()))
            SetSelection(Selection(it->second));
      }
   }
}

void EditorUI::MaterialProperties(PropertyPane &props, Material *material)
{
   Material *editedMaterial = props.GetEditedPart<Material>(material);
   props.Header("Material"s, [editedMaterial]() { return MakeWString(editedMaterial->m_name); }, [editedMaterial](const wstring &v) { editedMaterial->m_name = MakeString(v); });

   if (props.BeginSection("Visuals"s))
   {
      props.Combo<Material>(
         material, "Type"s, vector { "Default"s, "Metal"s }, //
         [](const Material *material) { return material->m_type; }, //
         [](Material *material, int v) { material->m_type = static_cast<Material::MaterialType>(v); });
      props.InputRGB<Material>(
         material, "Color", //
         [](const Material *material) { return convertColor(material->m_cBase); }, //
         [](Material *material, const vec3 &v) { material->m_cBase = convertColorRGB(v); });
      props.InputFloat<Material>(
         material, "Wrap Lighting"s, //
         [](const Material *material) { return material->m_fWrapLighting; }, //
         [](Material *material, float v) { material->m_fWrapLighting = v; }, PropertyPane::Unit::None, 2);
      if (material->m_type != Material::METAL)
      {
         props.InputRGB<Material>(
            material, "Glossy Color", //
            [](const Material *material) { return convertColor(material->m_cGlossy); }, //
            [](Material *material, const vec3 &v) { material->m_cGlossy = convertColorRGB(v); });
         props.InputFloat<Material>(
            material, "Glossy Image Lerp"s, //
            [](const Material *material) { return material->m_fGlossyImageLerp; }, //
            [](Material *material, float v) { material->m_fGlossyImageLerp = v; }, PropertyPane::Unit::None, 2);
      }
      props.InputFloat<Material>(
         material, "Shininess"s, //
         [](const Material *material) { return material->m_fRoughness; }, //
         [](Material *material, float v) { material->m_fRoughness = v; }, PropertyPane::Unit::None, 2);
      props.InputRGB<Material>(
         material, "Clearcoat Color", //
         [](const Material *material) { return convertColor(material->m_cClearcoat); }, //
         [](Material *material, const vec3 &v) { material->m_cClearcoat = convertColorRGB(v); });
      props.InputFloat<Material>(
         material, "Edge Brightness"s, //
         [](const Material *material) { return material->m_fEdge; }, //
         [](Material *material, float v) { material->m_fEdge = v; }, PropertyPane::Unit::None, 2);
      props.EndSection();
   }

   if (props.BeginSection("Transparency"s))
   {
      props.Checkbox<Material>(
         material, "Enable Transparency"s, //
         [](const Material *material) { return material->m_bOpacityActive; }, //
         [](Material *material, bool v) { material->m_bOpacityActive = v; });
      props.InputFloat<Material>(
         material, "Opacity"s, //
         [](const Material *material) { return material->m_fOpacity; }, //
         [](Material *material, float v) { material->m_fOpacity = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Material>(
         material, "Edge Opacity"s, //
         [](const Material *material) { return material->m_fEdgeAlpha; }, //
         [](Material *material, float v) { material->m_fEdgeAlpha = v; }, PropertyPane::Unit::None, 2);
      props.InputFloat<Material>(
         material, "Thickness"s, //
         [](const Material *material) { return material->m_fThickness; }, //
         [](Material *material, float v) { material->m_fThickness = v; }, PropertyPane::Unit::None, 2);
      props.InputRGB<Material>(
         material, "Refraction Tint", //
         [](const Material *material) { return convertColor(material->m_cRefractionTint); }, //
         [](Material *material, const vec3 &v) { material->m_cRefractionTint = convertColorRGB(v); });
      props.EndSection();
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
   if (m_selection.GetType() != Selection::S_EDITABLE || m_selection.GetPart() == nullptr || m_selection.GetPart()->GetDragPointCurve() == nullptr)
      return;
   m_savedSelection = m_selection;
   m_savedMultiSel = m_multiSel;
   m_savedOutlinerAnchor = m_outlinerAnchor;
   m_pointEditPart = m_selection.GetPart();
   m_pointEditPart->SetPointEditContext(this);
   m_pointSel.clear();
}

void EditorUI::ExitPointEditMode(bool restoreSelection)
{
   if (m_pointEditPart)
      m_pointEditPart->SetPointEditContext(nullptr);
   m_pointEditPart.reset();
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
   if (m_pointEditPart == nullptr || m_pointEditPart->GetDragPointCurve() == nullptr)
      return;
   m_pointEditPart->GetDragPointCurve()->OnPointsModified();
   m_renderer->ReinitRenderable(m_pointEditPart->GetEditable()->GetIRenderable());
   m_player->m_physics->Update(m_pointEditPart->GetEditable());
}

void EditorUI::AddPointOnNearestSegment()
{
   DragPointCurve *const curve = (m_pointEditPart != nullptr && !m_table->IsLocked()) ? m_pointEditPart->GetDragPointCurve() : nullptr;
   if (curve == nullptr)
      return;
   const vector<CComObject<DragPoint> *> &points = curve->GetPoints();
   if (points.empty())
      return;
   // Unproject the mouse position on the drag plane of the edited curve
   const float z = m_pointEditPart->GetDragPointZ(m_pointSel.empty() ? points.front() : m_pointSel.front());
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
   const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
   const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
   const float xcoord = (mousePos.x - rClipWidth) / rClipWidth;
   const float ycoord = (rClipHeight - mousePos.y) / rClipHeight;
   Matrix3D invMVP = m_renderer->GetMVP().GetModelViewProj(0);
   invMVP.Invert();
   const Vertex3Ds v3d = invMVP * Vertex3Ds { xcoord, ycoord, 0.f };
   const Vertex3Ds v3d2 = invMVP * Vertex3Ds { xcoord, ycoord, 1.f };
   const float dz = v3d2.z - v3d.z;
   const float t = (fabsf(dz) > 1e-10f) ? (z - v3d.z) / dz : 0.f;
   return Vertex2D(v3d.x + t * (v3d2.x - v3d.x), v3d.y + t * (v3d2.y - v3d.y));
}

DragPoint *EditorUI::HitTestDragPoint(const ImVec2 &mousePos) const
{
   const RenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, false);
   const float maxDist = 10.f * m_liveUI.GetDPI();
   DragPoint *best = nullptr;
   float bestDist = maxDist;
   for (CComObject<DragPoint> *point : m_pointEditPart->GetDragPointCurve()->GetPoints())
   {
      const ImVec2 pos = ctx.Project(Vertex3Ds(point->m_v.x, point->m_v.y, m_pointEditPart->GetDragPointZ(point)));
      if (pos.x == FLT_MAX)
         continue;
      const float dx = pos.x - mousePos.x;
      const float dy = pos.y - mousePos.y;
      const float dist = sqrtf(dx * dx + dy * dy);
      if (dist < bestDist)
      {
         bestDist = dist;
         best = point;
      }
   }
   return best;
}

void EditorUI::BoxSelectPoints(const ImVec2 &cornerA, const ImVec2 &cornerB, bool add)
{
   // Select all the drag points of the edited curve projecting inside the given screen box
   if (!add)
      m_pointSel.clear();
   const RenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, false);
   const ImVec2 boxMin(std::min(cornerA.x, cornerB.x), std::min(cornerA.y, cornerB.y));
   const ImVec2 boxMax(std::max(cornerA.x, cornerB.x), std::max(cornerA.y, cornerB.y));
   for (CComObject<DragPoint> *point : m_pointEditPart->GetDragPointCurve()->GetPoints())
   {
      const ImVec2 pos = ctx.Project(Vertex3Ds(point->m_v.x, point->m_v.y, m_pointEditPart->GetDragPointZ(point)));
      if (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y && !IsPointSelected(point))
         m_pointSel.push_back(point);
   }
}

void EditorUI::DeleteSelectedPoints()
{
   if (m_pointEditPart == nullptr || m_pointSel.empty() || m_table->IsLocked())
      return;
   DragPointCurve *const curve = m_pointEditPart->GetDragPointCurve();
   vector<CComObject<DragPoint> *> deletable;
   for (DragPoint *point : m_pointSel)
      if (point->CanDelete())
         deletable.push_back(static_cast<CComObject<DragPoint> *>(point));
   if (deletable.empty())
      return;
   BeginPointEdit();
   for (CComObject<DragPoint> *point : deletable)
   {
      m_pointSel.erase(std::ranges::find(m_pointSel, point));
      curve->DeletePoint(point);
   }
   EndPointEdit();
}


EditorUI::RenderContext::RenderContext(Player *player, ImDrawList *drawlist, ViewMode viewMode, Renderer::ShadeMode shadeMode, bool needsLiveTableSync)
   : m_player(player)
   , m_drawlist(drawlist)
   , m_viewMode(viewMode)
   , m_shadeMode(shadeMode)
   , m_needsLiveTableSync(needsLiveTableSync)
{
}

bool EditorUI::RenderContext::IsShowInvisible() const
{
   // Don't show invisible part in Live edit mode as there is no editor visibility management in this mode
   return (m_player->m_ptable->m_liveBaseTable == nullptr) && (GetViewMode() != ViewMode::PreviewCam);
}

ImVec2 EditorUI::RenderContext::Project(const Vertex3Ds &v) const
{
   const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
   const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
   const Matrix3D mvp = m_player->m_renderer->GetMVP().GetModelViewProj(0);
   const float xp = mvp._11 * v.x + mvp._21 * v.y + mvp._31 * v.z + mvp._41;
   const float yp = mvp._12 * v.x + mvp._22 * v.y + mvp._32 * v.z + mvp._42;
   //const float zp = mvp._13 * v.x + mvp._23 * v.y + mvp._33 * v.z + mvp._43;
   const float wp = mvp._14 * v.x + mvp._24 * v.y + mvp._34 * v.z + mvp._44;
   if (wp <= 1e-10f) // behind camera (or degenerated)
      return ImVec2 { FLT_MAX, FLT_MAX };
   const float inv_wp = 1.0f / wp;
   return ImVec2 { (wp + xp) * rClipWidth * inv_wp, (wp - yp) * rClipHeight * inv_wp };
}

void EditorUI::RenderContext::DrawLine(const Vertex3Ds &a, const Vertex3Ds &b, ImU32 color) const
{
   if (m_drawlist)
   {
      const ImVec2 p1 = Project(a);
      const ImVec2 p2 = Project(b);
      m_drawlist->AddLine(p1, p2, color);
   }
   // TODO also render when running in 3D (for logic parts like timers,...)
}

void EditorUI::RenderContext::DrawCircle(const Vertex3Ds &center, const Vertex3Ds &x, const Vertex3Ds &y, float radius, ImU32 color) const
{
   if (m_drawlist)
   {
      ImVec2 prev;
      constexpr int n = 32;
      for (int i = 0; i <= n; i++)
      {
         const float c = radius * cos((float)i * (float)(2. * M_PI / n));
         const float s = radius * sin((float)i * (float)(2. * M_PI / n));
         const ImVec2 p = Project(Vertex3Ds(center.x + c * x.x + s * y.x, center.y + c * x.y + s * y.y, center.z + c * x.z + s * y.z));
         if (i > 0 && p.x != FLT_MAX && prev.x != FLT_MAX) // Skip segments ending behind the camera
            GetDrawList()->AddLine(prev, p, color, 1.f);
         prev = p;
      }
   }
   // TODO also render when running in 3D (for logic parts like timers,...)
}

void EditorUI::RenderContext::DrawHitObjects(IEditable *editable) const
{
   if (m_drawlist)
   {
      auto project = [this](Vertex3Ds v)
      {
         const ImVec2 pt = Project(v);
         return Vertex2D(pt.x, pt.y);
      };
      const ImU32 color = GetColor(m_isSelected);
      const ImU32 alpha = (color & 0x00FFFFFFu) | 0x20000000u;
      ImGui::PushStyleColor(ImGuiCol_PlotLines, color);
      ImGui::PushStyleColor(ImGuiCol_PlotHistogram, alpha);
      for (auto pho : m_player->m_physics->GetUIHitObjects(editable))
         pho->DrawUI(project, m_drawlist, true);
      ImGui::PopStyleColor(2);
   }
}

void EditorUI::RenderContext::DrawWireframe(IEditable *editable) const
{
   if (IsSelected())
   {
      // Selection overlay
      g_pplayer->m_renderer->DrawWireframe(editable, convertColor(GetColor(true), 32.f / 255.f), convertColor(GetColor(true), 1.f), false);
   }
   else
   {
      // "Invisible" part, but UI visible
      const vec4 fillColor = m_shadeMode == Renderer::ShadeMode::NoDepthWireframe ? vec4(0.f, 0.f, 0.f, 32.f / 255.f) : vec4(32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 1.f);
      const vec4 edgeColor(0.f, 0.f, 0.f, 1.f);
      g_pplayer->m_renderer->DrawWireframe(editable, fillColor, edgeColor, m_shadeMode != Renderer::ShadeMode::NoDepthWireframe);
   }
}

}
