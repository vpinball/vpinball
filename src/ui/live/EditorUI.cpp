// license:GPLv3+

#include "core/stdafx.h"

#include "EditorUI.h"
#include "editor/EditableUIPart.h"
#include "editor/BallUIPart.h"
#include "editor/BumperUIPart.h"
#include "editor/DecalUIPart.h"
#include "editor/DispReelUIPart.h"
#include "editor/FlasherUIPart.h"
#include "editor/FlipperUIPart.h"
#include "editor/GateUIPart.h"
#include "editor/HitTargetUIPart.h"
#include "editor/KickerUIPart.h"
#include "editor/LightUIPart.h"
#include "editor/LightSeqUIPart.h"
#include "editor/PartGroupUIPart.h"
#include "editor/PlungerUIPart.h"
#include "editor/PrimitiveUIPart.h"
#include "editor/RampUIPart.h"
#include "editor/RubberUIPart.h"
#include "editor/SpinnerUIPart.h"
#include "editor/SurfaceUIPart.h"
#include "editor/TextBoxUIPart.h"
#include "editor/TimerUIPart.h"
#include "editor/TriggerUIPart.h"

#include "renderer/VRDevice.h"
#include "renderer/Shader.h"
#include "renderer/Anaglyph.h"

#include "core/TableDB.h"

#include "plugins/VPXPlugin.h"
#include "core/VPXPluginAPIImpl.h"

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
{
   m_StartTime_msec = msec();
   m_app = g_pvp;
   m_player = g_pplayer;
   m_table = m_player->m_ptable;
   m_pininput = &(m_player->m_pininput);
   m_renderer = m_player->m_renderer;

   m_selection.type = Selection::SelectionType::S_NONE;

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
   m_renderer->DisableStaticPrePass(false);
}

void EditorUI::ResetCameraFromPlayer()
{
   // Try to setup editor camera to match the used one, but only mostly since the EditorUI does not have some view setup features like off-center, ...
   m_camView = Matrix3D::MatrixScale(1.f, 1.f, -1.f) * m_renderer->GetMVP().GetView() * Matrix3D::MatrixScale(1.f, -1.f, 1.f);
}

void EditorUI::Render3D()
{
   UpdateEditableList();
   RenderContext ctx(m_player, nullptr, m_camMode, m_shadeMode, (m_table->m_liveBaseTable != nullptr) && m_player->IsPlaying());
   for (const auto &uiPart : m_editables)
   {
      if ((m_camMode == ViewMode::DesktopBackdrop && !uiPart->GetEditable()->m_backglass) || (m_camMode != ViewMode::DesktopBackdrop && uiPart->GetEditable()->m_backglass))
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

   Selection previousSelection = m_selection;

#if !((defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__))
   UpdateEditableList();

   // Gives some transparency when positioning camera to better view camera view bounds
   // TODO for some reasons, this breaks the modal background behavior
   //SetupImGuiStyle(m_selection.type == EditorUI::Selection::SelectionType::S_CAMERA ? 0.3f : 1.0f);

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
            if (ImGui::MenuItem("Save"))
               m_table->Save(false);
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
            if (ImGui::MenuItem(m_player->m_debugWindowActive ? "Play" : "Pause"))
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
         if (m_selection.type == Selection::S_EDITABLE && ImGui::Button(ICON_FK_TRASH_O))
            DeleteSelection();
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
         if (ImGui::RadioButton("None", m_physOverlay == PO_NONE))
            m_physOverlay = PO_NONE;
         if (ImGui::RadioButton("Selected", m_physOverlay == PO_SELECTED))
            m_physOverlay = PO_SELECTED;
         if (ImGui::RadioButton("All", m_physOverlay == PO_ALL))
            m_physOverlay = PO_ALL;
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_FILTER)) // Selection filter
         ImGui::OpenPopup("Selection filter Popup");
      if (ImGui::BeginPopup("Selection filter Popup"))
      {
         bool pf = m_selectionFilter & SelectionFilter::SF_Playfield;
         bool prims = m_selectionFilter & SelectionFilter::SF_Primitives;
         bool lights = m_selectionFilter & SelectionFilter::SF_Lights;
         bool flashers = m_selectionFilter & SelectionFilter::SF_Flashers;
         ImGui::TextUnformatted("Selection filters:");
         ImGui::Separator();
         if (ImGui::Checkbox("Playfield", &pf))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Playfield) | (pf ? SelectionFilter::SF_Playfield : 0x0000);
         if (ImGui::Checkbox("Primitives", &prims))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Primitives) | (prims ? SelectionFilter::SF_Primitives : 0x0000);
         if (ImGui::Checkbox("Lights", &lights))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Lights) | (lights ? SelectionFilter::SF_Lights : 0x0000);
         if (ImGui::Checkbox("Flashers", &flashers))
            m_selectionFilter = (m_selectionFilter & ~SelectionFilter::SF_Flashers) | (flashers ? SelectionFilter::SF_Flashers : 0x0000);
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
         case PredefinedView::None: text = "User"s; break;
         case PredefinedView::Left: text = "Left"s; break;
         case PredefinedView::Right: text = "Right"s; break;
         case PredefinedView::Top: text = "Top"s; break;
         case PredefinedView::Bottom: text = "Bottom"s; break;
         case PredefinedView::Front: text = "Front"s; break;
         case PredefinedView::Back: text = "Back"s; break;
         }
         ImGui::TextUnformatted((text + (m_perspectiveCam ? " Perspective" : " Orthographic")).c_str());
         break;
      }
      case ViewMode::DesktopBackdrop: ImGui::TextUnformatted("Desktop Backdrop"); break;
      }
      switch (m_gizmoOperation)
      {
      case ImGuizmo::NONE: ImGui::TextUnformatted("Select"); break;
      case ImGuizmo::TRANSLATE: ImGui::TextUnformatted("Grab"); break;
      case ImGuizmo::ROTATE: ImGui::TextUnformatted("Rotate"); break;
      case ImGuizmo::SCALE: ImGui::TextUnformatted("Scale"); break;
      default: break;
      }
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
      m_camView = RH2LH * m_renderer->GetMVP().GetView() * YAxis;
      m_camProj = YAxis * m_renderer->GetMVP().GetProj(0);
   }
   else
   {
      // Apply editor camera to renderer (move view/projection from right handed to left handed)
      // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
      // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
      const Matrix3D view = RH2LH * m_camView * YAxis;
      const Matrix3D proj = YAxis * m_camProj;
      m_renderer->GetMVP().SetView(view);
      m_renderer->GetMVP().SetProj(0, proj);
      m_renderer->GetMVP().SetProj(1, proj);

      if (m_perspectiveCam)
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
   if (isSelectionTransformValid)
   {
      float camViewLH[16];
      memcpy(camViewLH, &m_camView.m[0][0], sizeof(float) * 4 * 4);
      for (int i = 8; i < 12; i++)
         camViewLH[i] = -camViewLH[i];
      const Matrix3D prevTransform(transform);
      ImGuizmo::Manipulate(camViewLH, (float *)(m_camProj.m), m_gizmoOperation, m_gizmoMode, (float *)(transform.m));
      if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
      {
         PushUndo(m_selection.type == EditorUI::Selection::SelectionType::S_EDITABLE ? m_selection.uiPart->GetEditable() : m_table, 0x1000);
         SetSelectionTransform(transform);
      }
   }

   m_renderer->SetShadeMode(m_shadeMode);

   // Selection and physic colliders overlay
   {
      RenderContext ctx(m_player, overlayDrawList, m_camMode, m_shadeMode, (m_table->m_liveBaseTable != nullptr) && m_player->IsPlaying());
      if (m_selection.type == Selection::S_EDITABLE)
      {
         if (m_renderer->m_renderDevice->GetCurrentRenderTarget()->HasDepth())
            m_renderer->m_renderDevice->Clear(clearType::ZBUFFER, 0);
         ctx.m_isSelected = true;
         m_selection.uiPart->Render(ctx);
         ctx.m_isSelected = false;
      }

      if (isSelectionTransformValid)
      {
         const ImVec2 pos = ctx.Project(transform.GetOrthoNormalPos());
         overlayDrawList->AddCircleFilled(pos, 3.f * m_liveUI.GetDPI(), IM_COL32(255, 255, 255, 255), 16);
      }

      if (m_physOverlay == PO_ALL || (m_physOverlay == PO_SELECTED && m_selection.type == Selection::S_EDITABLE))
      {
         auto project = [ctx](Vertex3Ds v)
         {
            const ImVec2 pt = ctx.Project(v);
            return Vertex2D(pt.x, pt.y);
         };
         ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 0, 0, 255)); // We abuse ImGui colors to pass render colors
         ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 0, 0, 64));
         for (auto pho : m_player->m_physics->GetHitObjects())
            if (pho != nullptr && (m_physOverlay == PO_ALL || (m_physOverlay == PO_SELECTED && pho->m_editable == m_selection.uiPart->GetEditable())))
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
               if (!m_perspectiveCam)
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

      // Select
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
         // Compute mouse position in clip space
         const float rClipWidth = (float)m_player->m_playfieldWnd->GetWidth() * 0.5f;
         const float rClipHeight = (float)m_player->m_playfieldWnd->GetHeight() * 0.5f;
         const float xcoord = (ImGui::GetMousePos().x - rClipWidth) / rClipWidth;
         const float ycoord = (rClipHeight - ImGui::GetMousePos().y) / rClipHeight;

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

         vector<HitTestResult> vhoHit;
         const bool noPF = !(m_selectionFilter & SelectionFilter::SF_Playfield);
         const bool noPrims = !(m_selectionFilter & SelectionFilter::SF_Primitives);
         const bool noLights = !(m_selectionFilter & SelectionFilter::SF_Lights);
         const bool noFlashers = !(m_selectionFilter & SelectionFilter::SF_Flashers);
         for (const auto &hr : vhoUnfilteredHit)
         {
            const auto type = hr.m_obj->m_editable->GetItemType();
            const auto editable = hr.m_obj->m_editable;
            if (editable)
            {
               const PartGroup *parent = editable->GetPartGroup();
               bool visible = editable->GetISelect() ? editable->GetISelect()->m_isVisible : true;
               while (parent && visible)
               {
                  if ((parent->GetPlayerModeVisibilityMask() & m_renderer->GetPlayerModeVisibilityMask()) == 0)
                     visible = false;
                  visible &= parent->m_isVisible;
                  parent = parent->GetPartGroup();
               }
               if (!visible)
                  continue;
               if (noPF && type == ItemTypeEnum::eItemPrimitive && static_cast<Primitive *>(editable)->IsPlayfield())
                  continue;
               if (noPrims && type == ItemTypeEnum::eItemPrimitive)
                  continue;
               if (noLights && type == ItemTypeEnum::eItemLight)
                  continue;
               if (noFlashers && type == ItemTypeEnum::eItemFlasher)
                  continue;
            }
            vhoHit.push_back(hr);
         }

         if (vhoHit.empty())
            m_selection.type = Selection::SelectionType::S_NONE;
         else
         {
            size_t selectionIndex = vhoHit.size();
            for (size_t i = 0; i <= vhoHit.size(); i++)
            {
               if (i < vhoHit.size() && m_selection.type == Selection::S_EDITABLE && vhoHit[i].m_obj->m_editable == m_selection.uiPart->GetEditable())
                  selectionIndex = i + 1;
               if (i == selectionIndex)
               {
                  const size_t p = selectionIndex % vhoHit.size();
                  const IEditable *select = vhoHit[p].m_obj->m_editable;
                  const auto it = std::ranges::find_if(m_editables, [select](const std::shared_ptr<EditableUIPart> &part) { return part->GetEditable() == select; });
                  if (it != m_editables.end())
                     m_selection = Selection(*it);
               }
            }
            // TODO add debug action to make ball active: m_player->m_pactiveballDebug = m_pBall;
         }
      }
   }
   if (!io.WantCaptureKeyboard)
   {
      if (ImGui::IsKeyReleased(ImGuiKey_Escape))
      {
         if (m_gizmoOperation != ImGuizmo::NONE)
            m_gizmoOperation = ImGuizmo::NONE; // Cancel current operation
         else if (m_selection.type != Selection::S_NONE)
            m_selection = Selection(); // Cancel current selection
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_F))
      {
         m_flyMode = !m_flyMode;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_A))
      {
         if (io.KeyAlt && !io.KeyCtrl && !io.KeyShift)
            m_selection = Selection();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Delete))
      {
         if (!io.KeyAlt && !io.KeyCtrl && !io.KeyShift)
            DeleteSelection();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_H))
      {
         if (m_table->m_liveBaseTable)
         {
            // No UI visibility in inspection mode
         }
         if (io.KeyAlt)
         { // Unhide all
            for (auto &part : m_editables)
               if (part->GetEditable()->GetItemType() != eItemPartGroup && part->GetEditable()->GetISelect())
                  part->GetEditable()->GetISelect()->m_isVisible = true;
         }
         else if (io.KeyShift)
         { // Hide unselected
            if (m_selection.type == Selection::S_EDITABLE)
            {
               for (auto &part : m_editables)
                  if (part->GetEditable()->GetItemType() != eItemPartGroup && part != m_selection.uiPart && part->GetEditable()->GetISelect())
                     part->GetEditable()->GetISelect()->m_isVisible = false;
            }
         }
         else
         { // Hide selected
            if (m_selection.type == Selection::S_EDITABLE)
            {
               if (m_selection.uiPart->GetEditable()->GetISelect())
               {
                  m_selection.uiPart->GetEditable()->GetISelect()->m_isVisible = false;
                  m_selection = Selection();
               }
            }
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_G))
      {
         // Grab (translate)
         if (m_camMode == ViewMode::PreviewCam)
            m_camMode = ViewMode::EditorCam;
         if (io.KeyAlt)
         {
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               SetSelectionTransform(tmp, true, false, false);
         }
         else
         {
            m_gizmoOperation = ImGuizmo::TRANSLATE;
            m_gizmoMode = m_gizmoOperation == ImGuizmo::TRANSLATE ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
         // Scale
         if (m_camMode == ViewMode::PreviewCam)
            m_camMode = ViewMode::EditorCam;
         if (io.KeyAlt)
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
      else if (ImGui::IsKeyPressed(ImGuiKey_R))
      {
         // Rotate
         if (m_camMode == ViewMode::PreviewCam)
            m_camMode = ViewMode::EditorCam;
         if (io.KeyAlt)
         {
            Matrix3D tmp;
            if (GetSelectionTransform(tmp))
               SetSelectionTransform(tmp, false, false, true);
         }
         else
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
               m_table->m_undo.Undo();
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
#ifdef _DEBUG
         case ViewMode::EditorCam: m_camMode = ViewMode::DesktopBackdrop; break; // Desktop backdrop editor is not yet operational
#else
         case ViewMode::EditorCam: m_camMode = ViewMode::PreviewCam; break;
#endif
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

   if (m_selection != previousSelection)
   {
      if ((previousSelection.type == Selection::S_EDITABLE) //
         && FindIndexOf(m_table->m_vedit, previousSelection.uiPart->GetEditable()) != -1 // Not deleted
         && (previousSelection.uiPart->GetEditable()->GetIHitable() != nullptr)
         && (previousSelection.uiPart->GetEditable()->GetItemType() != eItemBall))
         m_player->m_physics->SetStatic(previousSelection.uiPart->GetEditable());
      if ((m_selection.type == Selection::S_EDITABLE) && (m_selection.uiPart->GetEditable()->GetIHitable() != nullptr) && (m_selection.uiPart->GetEditable()->GetItemType() != eItemBall))
         m_player->m_physics->SetDynamic(m_selection.uiPart->GetEditable());
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
   m_table->m_undo.BeginUndo();
   m_table->m_undo.MarkForUndo(part);
   m_table->m_undo.EndUndo();
}

void EditorUI::DeleteSelection()
{
   if (m_selection.type == Selection::S_EDITABLE && m_selection.uiPart->GetEditable()->GetItemType() != eItemBall && m_selection.uiPart->GetEditable()->GetPartGroup() != nullptr)
   {
      IEditable* edit = m_selection.uiPart->GetEditable();
      RemoveFromVectorSingle(m_editables, m_selection.uiPart);
      m_selection = Selection();
      if (edit->GetIHitable())
      {
         m_player->m_physics->Remove(edit);
         edit->GetIHitable()->RenderRelease();
      }
      RemoveFromVectorSingle(g_pplayer->m_vhitables, edit);
      RemoveFromVectorSingle(m_table->m_vedit, edit);
      edit->Release();
   }
}

void EditorUI::UpdateEditableList()
{
   // Remove UI parts of removed editables
   std::erase_if(m_editables,
      [this](const auto &uiPart)
      {
         const auto it = std::ranges::find_if(m_table->m_vedit, [uiPartEdit = uiPart->GetEditable()](const auto &edit) { return uiPartEdit == edit; });
         return it == m_table->m_vedit.end();
      });
   // Add UI parts for new editables
   bool needSort = false;
   for (const auto &edit : m_table->m_vedit)
   {
      const auto it = std::ranges::find_if(m_editables, [edit](const auto &uiPart) { return uiPart->GetEditable() == edit; });
      if (it == m_editables.end()) // New part
      {
         std::shared_ptr<EditableUIPart> uiPart;
         switch (edit->GetItemType())
         {
         // eItemTable, eItemLightCenter, eItemDragPoint, eItemCollection
         case eItemBall: uiPart = std::make_shared<BallUIPart>(static_cast<Ball *>(edit)); break;
         case eItemBumper: uiPart = std::make_shared<BumperUIPart>(static_cast<Bumper *>(edit)); break;
         case eItemDecal: uiPart = std::make_shared<DecalUIPart>(static_cast<Decal *>(edit)); break;
         case eItemDispReel: uiPart = std::make_shared<DispReelUIPart>(static_cast<DispReel *>(edit)); break;
         case eItemFlasher: uiPart = std::make_shared<FlasherUIPart>(static_cast<Flasher *>(edit)); break;
         case eItemFlipper: uiPart = std::make_shared<FlipperUIPart>(static_cast<Flipper *>(edit)); break;
         case eItemGate: uiPart = std::make_shared<GateUIPart>(static_cast<Gate *>(edit)); break;
         case eItemHitTarget: uiPart = std::make_shared<HitTargetUIPart>(static_cast<HitTarget *>(edit)); break;
         case eItemKicker: uiPart = std::make_shared<KickerUIPart>(static_cast<Kicker *>(edit)); break;
         case eItemLight: uiPart = std::make_shared<LightUIPart>(static_cast<Light *>(edit)); break;
         case eItemLightSeq: uiPart = std::make_shared<LightSeqUIPart>(static_cast<LightSeq *>(edit)); break;
         case eItemPartGroup: uiPart = std::make_shared<PartGroupUIPart>(static_cast<PartGroup *>(edit)); break;
         case eItemPlunger: uiPart = std::make_shared<PlungerUIPart>(static_cast<Plunger *>(edit)); break;
         case eItemPrimitive: uiPart = std::make_shared<PrimitiveUIPart>(static_cast<Primitive *>(edit)); break;
         case eItemRamp: uiPart = std::make_shared<RampUIPart>(static_cast<Ramp *>(edit)); break;
         case eItemRubber: uiPart = std::make_shared<RubberUIPart>(static_cast<Rubber *>(edit)); break;
         case eItemSpinner: uiPart = std::make_shared<SpinnerUIPart>(static_cast<Spinner *>(edit)); break;
         case eItemSurface: uiPart = std::make_shared<SurfaceUIPart>(static_cast<Surface *>(edit)); break;
         case eItemTextbox: uiPart = std::make_shared<TextBoxUIPart>(static_cast<Textbox *>(edit)); break;
         case eItemTimer: uiPart = std::make_shared<TimerUIPart>(static_cast<Timer *>(edit)); break;
         case eItemTrigger: uiPart = std::make_shared<TriggerUIPart>(static_cast<Trigger *>(edit)); break;
         default: uiPart = std::make_shared<BaseUIPart>(edit); break;
         }
         if (m_table->m_liveBaseTable && edit->GetISelect())
            edit->GetISelect()->m_isVisible = true;
         uiPart->SetOutlinerPath(edit->GetPathString(false));
         m_editables.push_back(std::move(uiPart));
         needSort = true;
      }
      else if (!(*it)->GetOutlinerPath().ends_with(edit->GetName())) // Name and therefore outliner path has changed
      {
         needSort = true;
         if (edit->GetItemType() == eItemPartGroup) // Also update all children
            for (const auto &uiPart : m_editables)
               uiPart->SetOutlinerPath(uiPart->GetEditable()->GetPathString(false));
         else
            (*it)->SetOutlinerPath((*it)->GetEditable()->GetPathString(false));
      }
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
   if (m_selection.type == EditorUI::Selection::SelectionType::S_EDITABLE)
   {
      const EditableUIPart::TransformMask mask = m_selection.uiPart->GetTransform(transform);
      return mask != EditableUIPart::TransformMask::TM_None;
   }
   return false;
}

void EditorUI::SetSelectionTransform(const Matrix3D &newTransform, bool clearPosition, bool clearScale, bool clearRotation) const
{
   Matrix3D transform = newTransform;
   const Vertex3Ds right(transform._11, transform._12, transform._13);
   const Vertex3Ds up(transform._21, transform._22, transform._23);
   const Vertex3Ds dir(transform._31, transform._32, transform._33);
   vec3 scale(right.Length(), up.Length(), dir.Length());

   transform._11 /= scale.x; // Normalize transform to evaluate rotation
   transform._12 /= scale.x;
   transform._13 /= scale.x;
   transform._21 /= scale.y;
   transform._22 /= scale.y;
   transform._23 /= scale.y;
   transform._31 /= scale.z;
   transform._32 /= scale.z;
   transform._33 /= scale.z;
   if (clearScale)
      scale.Set(1.f, 1.f, 1.f);

   vec3 pos;
   if (clearPosition)
      pos.Set(0.f, 0.f, 0.f);
   else
      pos.Set(transform._41, transform._42, transform._43);

   // Derived from https://learnopencv.com/rotation-matrix-to-euler-angles/
   vec3 rot;
   const float sy = sqrtf(transform._11 * transform._11 + transform._21 * transform._21);
   if (clearRotation)
   {
      rot.Set(0.f, 0.f, 0.f);
   }
   else if (sy > 1e-6f)
   {
      rot.x = -RADTOANG(atan2f(transform._32, transform._33));
      rot.y = -RADTOANG(atan2f(-transform._31, sy));
      rot.z = -RADTOANG(atan2f(transform._21, transform._11));
   }
   else
   {
      rot.x = -RADTOANG(atan2f(transform._23, transform._22));
      rot.y = -RADTOANG(atan2f(-transform._22, sy));
      rot.z = 0.f;
   }

   if (m_selection.type == EditorUI::Selection::SelectionType::S_EDITABLE)
   {
      m_selection.uiPart->SetTransform(pos, scale, rot);
      m_renderer->ReinitRenderable(m_selection.uiPart->GetEditable()->GetIHitable());
      m_player->m_physics->Update(m_selection.uiPart->GetEditable());
   }
}

bool EditorUI::IsOutlinerFiltered(const string &name) const
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

   ImGui::InputTextWithHint("Filter", "Name part filter", &m_outlinerFilter);

   if (ImGui::TreeNodeEx("View Setups"))
   {
      if (ImGui::Selectable("Editor Camera"))
      {
         m_selection.type = Selection::SelectionType::S_NONE;
         m_camMode = ViewMode::EditorCam;
      }
      Selection cam0(Selection::SelectionType::S_CAMERA, 0);
      if (ImGui::Selectable("Preview: Desktop", m_selection == cam0))
      {
         m_selection = cam0;
         m_camMode = ViewMode::PreviewCam;
         m_table->SetViewSetupOverride(BG_DESKTOP);
      }
      Selection cam1(Selection::SelectionType::S_CAMERA, 1);
      if (ImGui::Selectable("Preview: Cabinet", m_selection == cam1))
      {
         m_selection = cam1;
         m_camMode = ViewMode::PreviewCam;
         m_table->SetViewSetupOverride(BG_FULLSCREEN);
      }
      Selection cam2(Selection::SelectionType::S_CAMERA, 2);
      if (ImGui::Selectable("Preview: Full Single Screen", m_selection == cam2))
      {
         m_selection = cam2;
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
         if (IsOutlinerFiltered(material->m_name) && ImGui::Selectable(material->m_name.c_str(), m_selection == sel))
            m_selection = sel;
      }
      ImGui::TreePop();
   }
   if (ImGui::TreeNode("Images"))
   {
      const std::function<string(Texture *)> map = [](Texture *image) -> string { return image->m_name; };
      for (Texture *&image : SortedCaseInsensitive(m_table->m_vimage, map))
      {
         Selection sel(image);
         if (IsOutlinerFiltered(image->m_name) && ImGui::Selectable(image->m_name.c_str(), m_selection == sel))
            m_selection = sel;
      }
      ImGui::TreePop();
   }
   if (ImGui::TreeNode("Render Probes"))
   {
      for (RenderProbe *probe : m_table->m_vrenderprobe)
      {
         Selection sel(probe);
         if (ImGui::Selectable(probe->GetName().c_str(), m_selection == sel))
            m_selection = sel;
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
      vector<Node> stack;
      int outlinerItem = 0;
      const float eyeWidth = ImGui::CalcTextSize(ICON_FK_EYE, nullptr, true).x;
      const float eyeX = ImGui::GetContentRegionAvail().x; // - eyeWidth;
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
         ImGui::AlignTextToFramePadding();
         if (edit->GetEditable()->GetItemType() == eItemPartGroup)
         {
            PartGroup *group = static_cast<PartGroup *>(edit->GetEditable());
            const bool opened = ImGui::TreeNodeEx(edit->GetEditable()->GetName().c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);
            if (m_table->m_liveBaseTable == nullptr)
            {
               ImGui::SameLine(eyeX);
               ImGui::PushStyleColor(ImGuiCol_Text, group->m_isVisible ? IM_COL32_WHITE : IM_COL32(128, 128, 128, 255));
               if (ImGui::SmallButton(((group->m_isVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH) + "##Eye__"s + edit->GetEditable()->GetName()).c_str()))
                  group->m_isVisible = !group->m_isVisible;
               ImGui::PopStyleColor();
            }
            stack.push_back({ static_cast<PartGroup *>(edit->GetEditable()), (stack.empty() || stack.back().opened) ? opened : false });
         }
         else
         {
            if (parent == nullptr && stack.empty())
               stack.push_back({ nullptr, ImGui::TreeNodeEx("[Live Objects]", ImGuiTreeNodeFlags_AllowItemOverlap) });
            if (stack.back().opened)
            {
               Selection sel(edit);
               if (IsOutlinerFiltered(edit->GetEditable()->GetName()))
               {
                  if (ImGui::Selectable((edit->GetEditable()->GetName() + "##Outliner"s + std::to_string(outlinerItem++)).c_str(), m_selection == sel, ImGuiSelectableFlags_AllowItemOverlap))
                     m_selection = sel;
                  ISelect *selectable = edit->GetEditable()->GetISelect();
                  if (selectable && m_table->m_liveBaseTable == nullptr)
                  {
                     ImGui::SameLine(eyeX);
                     ImGui::PushStyleColor(ImGuiCol_Text, selectable->m_isVisible ? IM_COL32_WHITE : IM_COL32(128, 128, 128, 255));
                     if (ImGui::SmallButton(((selectable->m_isVisible ? ICON_FK_EYE : ICON_FK_EYE_SLASH) + "##Eye__"s + edit->GetEditable()->GetName()).c_str()))
                        selectable->m_isVisible = !selectable->m_isVisible;
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

   m_outliner_width = ImGui::GetWindowWidth();
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

   PropertyPane props(m_table);
   switch (m_units)
   {
   case Units::VPX: props.SetLengthUnit(PropertyPane::Unit::VPLength); break;
   case Units::Metric: props.SetLengthUnit(PropertyPane::Unit::Millimeters); break;
   case Units::Imperial: props.SetLengthUnit(PropertyPane::Unit::Inches); break;
   }
   if (IsInspectMode() && m_selection.type != Selection::SelectionType::S_IMAGE) // Images are shared between live and startup instance, so they do not have 2 states
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
               switch (m_selection.type)
               {
               case Selection::SelectionType::S_NONE: TableProperties(props); break;
               case Selection::SelectionType::S_EDITABLE:
                  m_selection.uiPart->UpdatePropertyPane(props);
                  if (props.GetModifiedField() > 0)
                  {
                     m_renderer->ReinitRenderable(m_selection.uiPart->GetEditable()->GetIHitable());
                     m_player->m_physics->Update(m_selection.uiPart->GetEditable());
                  }
                  break;
               case Selection::SelectionType::S_IMAGE: ImageProperties(props, m_selection.image); break;
               case Selection::SelectionType::S_CAMERA: CameraProperties(props, m_selection.camera); break;
               case Selection::SelectionType::S_MATERIAL: MaterialProperties(props, m_selection.material); break;
               case Selection::SelectionType::S_RENDERPROBE: RenderProbeProperties(props, m_selection.renderprobe); break;
               }
               ImGui::EndTabItem();
            }
         }
         ImGui::EndTabBar();
      }
   }
   else
   {
      switch (m_selection.type)
      {
      case Selection::SelectionType::S_NONE: TableProperties(props); break;
      case Selection::SelectionType::S_EDITABLE:
         m_table->m_undo.BeginUndo();
         m_table->m_undo.MarkForUndo(m_selection.uiPart->GetEditable());
         m_table->m_undo.EndUndo();
         m_selection.uiPart->UpdatePropertyPane(props);
         if (props.GetModifiedField() > 0 && (m_lastUndoPart != m_selection.uiPart->GetEditable() || m_lastUndoId != (0x2000 | props.GetModifiedField())))
         {
            m_lastUndoPart = m_selection.uiPart->GetEditable();
            m_lastUndoId = 0x2000 | props.GetModifiedField();
         }
         else
         {
            m_table->m_undo.Undo(true);
         }
         if (props.GetModifiedField() > 0)
         {
            m_renderer->ReinitRenderable(m_selection.uiPart->GetEditable()->GetIHitable());
            m_player->m_physics->Update(m_selection.uiPart->GetEditable());
         }
         break;
      case Selection::SelectionType::S_IMAGE: ImageProperties(props, m_selection.image); break;
      case Selection::SelectionType::S_CAMERA: CameraProperties(props, m_selection.camera); break;
      case Selection::SelectionType::S_MATERIAL: MaterialProperties(props, m_selection.material); break;
      case Selection::SelectionType::S_RENDERPROBE: RenderProbeProperties(props, m_selection.renderprobe); break;
      }
   }

   ImGui::End();
   ImGui::PopStyleVar(3);
}

void EditorUI::UpdateRendererInspectionModal()
{
   // FIXME m_renderer->DisableStaticPrePass(false);
   m_camMode = ViewMode::PreviewCam;

   ImGui::SetNextWindowSize(ImVec2(350.f * m_liveUI.GetDPI(), 0));
   if (ImGui::Begin(ID_RENDERER_INSPECTION, &m_showRendererInspection))
   {
      ImGui::TextUnformatted("Display single render pass:");
      static int pass_selection = IF_FPS;
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
         const uint32_t period = m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME);
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Property panes
//

void EditorUI::TableProperties(PropertyPane &props)
{
   PinTable *table = props.GetEditedPart<PinTable>(m_table);
   props.Header("Table"s, [table]() { return table->GetName(); }, [table](const string &v) { table->SetName(v); });

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
   props.Header("Camera", [bgSet]() { return bgSet == 0 ? "Desktop"s : bgSet == 1 ? "Cabinet"s : "Full Single Screen"s; }, [](const string &) {});
   ImGui::EndDisabled();

   {
      if (ImGui::Button("Import"))
      {
         m_table->ImportBackdropPOV(string());
         m_renderer->MarkShaderDirty();
      }
      ImGui::SameLine();
      if (ImGui::Button("Export"))
         m_table->ExportBackdropPOV();
      ImGui::NewLine();
   }

   if (props.BeginSection("Visuals"s))
   {
      ViewSetup *vs = &m_table->mViewSetups[bgSet];
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

   props.Header("Image"s, [texture]() { return texture->m_name; }, [texture](const string &v) { texture->m_name = v; });

   ImTextureID image = m_renderer->m_renderDevice->m_texMan.LoadTexture(texture, false);
   if (image)
   {
      if (props.BeginSection("Visuals"s))
      {
         std::shared_ptr<const BaseTexture> tex = texture->GetRawBitmap(false, 0);

         ImGui::BeginDisabled(tex == nullptr || !tex->HasAlpha());
         props.InputFloat<Texture>(
            m_selection.image, "Alpha Mask", //
            [](const Texture *image) { return image->m_alphaTestValue; }, //
            [](Texture *image, float v) { image->m_alphaTestValue = v; }, PropertyPane::Unit::None, 2);
         ImGui::EndDisabled();

         const string info = std::to_string(image->GetWidth()) + 'x' + std::to_string(image->GetHeight()) + ' ' + (tex->m_format ? BaseTexture::GetFormatString(tex->m_format) : ""s);
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
   props.Header("Render Probe"s, [editedProbe]() { return editedProbe->GetName(); }, [editedProbe](const string &v) { editedProbe->SetName(v); });

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

      for (const IEditable *editable : m_table->m_vedit)
      {
         if (editable->GetItemType() != eItemPrimitive)
            continue;
         const Primitive *const primitive = static_cast<const Primitive *>(editable);
         if ((probe->GetType() == RenderProbe::PLANE_REFLECTION) && (primitive->m_d.m_szReflectionProbe != probe->GetName()))
            continue;
         if ((probe->GetType() == RenderProbe::SCREEN_SPACE_TRANSPARENCY) && (primitive->m_d.m_szRefractionProbe != probe->GetName()))
            continue;
         const auto it = std::ranges::find_if(m_editables, [editable](const auto part) { return part->GetEditable() == editable; });
         if (it == m_editables.end())
            continue;
         if (ImGui::Selectable(primitive->GetName().c_str()))
            m_selection = Selection(*it);
      }
   }
}

void EditorUI::MaterialProperties(PropertyPane &props, Material *material)
{
   Material *editedMaterial = props.GetEditedPart<Material>(material);
   props.Header("Material"s, [editedMaterial]() { return editedMaterial->m_name; }, [editedMaterial](const string &v) { editedMaterial->m_name = v; });

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
         if (i > 0)
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
