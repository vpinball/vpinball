// license:GPLv3+

#include "core/stdafx.h"
#include "ViewportPane.h"

#include "OutlinerPanel.h"

#include "core/player.h"
#include "EditorTheme.h"
#include "LiveRenderContext.h"
#include "fonts/IconsForkAwesome.h"
#include "physics/PhysicsEngine.h"
#include "renderer/RenderDevice.h"
#include "renderer/Renderer.h"
#include "renderer/RenderView.h"
#include "renderer/Window.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"

namespace VPX::EditorUI
{

ViewportPane::ViewportPane(EditorUI& editor, const ViewMode viewType)
   : EditorAreaView(editor)
   , m_viewType(viewType)
{
}

ViewportPane::~ViewportPane() { OnUnhosted(); }

void ViewportPane::OnUnhosted()
{
   if (m_editor.m_captureVp == this)
      m_editor.m_captureVp = nullptr;
   if (m_editor.m_hoverVp == this)
      m_editor.m_hoverVp = nullptr;
   if (m_view != nullptr)
   {
      m_editor.m_renderer->ReleaseEditorView(m_view);
      m_view = nullptr;
   }
}

void ViewportPane::AdoptCameraFrom(const ViewportPane* const src)
{
   if (m_viewType == ViewMode::PreviewCam)
   {
      // A render preview shows the game camera: no editor camera to adopt
   }
   else if (m_viewType == ViewMode::DesktopBackdrop)
      SetBackdropCamera(); // A backdrop pane starts on the backdrop area framing
   else if (src != nullptr && src->m_cameraValid && src->m_viewType == m_viewType)
   {
      // A newly created pane clones the camera of the same-type target pane (split panes, like Blender)
      m_perspectiveCam = src->m_perspectiveCam;
      m_shadeMode = src->m_shadeMode;
      m_predefinedView = src->m_predefinedView;
      m_orbitLock = src->m_orbitLock;
      m_camView = src->m_camView;
      m_camProj = src->m_camProj;
      m_camDistance = src->m_camDistance;
   }
   else
   {
      // Default pose: the playfield seen from above (right handed system for easy ImGuizmo integration
      // while VPX renderer is left handed, so reverse X axis)
      m_camDistance = m_editor.m_table->m_bottom * 0.7f;
      const vec3 eye(m_editor.m_table->m_right * 0.5f, m_editor.m_table->m_bottom * 0.5f, -m_camDistance);
      const vec3 at(m_editor.m_table->m_right * 0.5f, m_editor.m_table->m_bottom * 0.5f, 0.f);
      constexpr vec3 up { 0.f, -1.f, 0.f };
      m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
   }
   if (m_imageRect.Height() > 0.f)
      m_camAspect = m_imageRect.Width() / m_imageRect.Height();
   m_cameraValid = true;
}

void ViewportPane::SetGizmoOperation(ImGuizmo::OPERATION operation)
{
   if (operation == ImGuizmo::OPERATION(0))
   {
      m_gizmoOperation = operation;
      return;
   }
   // Repeating the same operation toggles between world and local coordinates, switching to another one resets to world
   m_gizmoMode = m_gizmoOperation == operation ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
   m_gizmoOperation = operation;
}

void ViewportPane::ResetCameraFromPlayer()
{
   // Try to setup editor camera to match the used one, but only mostly since the EditorUI does not have some view setup features like off-center, ...
   m_camView = Matrix3D::MatrixScale(1.f, 1.f, -1.f) * m_editor.m_renderer->GetMVP().GetView(0) * Matrix3D::MatrixScale(1.f, -1.f, 1.f);
}

void ViewportPane::SetBackdropCamera()
{
   // Setup the camera to frame the desktop backdrop (a flat rectangle in EDITOR_BG_WIDTH x EDITOR_BG_HEIGHT coordinates, Y axis going down)
   const ImGuiIO& io = ImGui::GetIO();
   const float aspect = m_imageRect.Height() > 0.f ? m_imageRect.Width() / m_imageRect.Height() : io.DisplaySize.x / io.DisplaySize.y;
   m_camDistance = 0.5f * max((float)EDITOR_BG_HEIGHT, (float)EDITOR_BG_WIDTH / aspect);
   const vec3 eye((float)EDITOR_BG_WIDTH * 0.5f, (float)EDITOR_BG_HEIGHT * 0.5f, -m_camDistance);
   const vec3 at((float)EDITOR_BG_WIDTH * 0.5f, (float)EDITOR_BG_HEIGHT * 0.5f, 0.f);
   constexpr vec3 up { 0.f, -1.f, 0.f };
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
}

void ViewportPane::SetView(const PredefinedView view)
{
   // Editor Camera to an axis aligned view, keeping the orbit target and distance
   m_predefinedView = view;
   Matrix3D inv(m_camView);
   inv.Invert();
   const vec3 camTarget = inv.GetOrthoNormalPos() - inv.GetOrthoNormalDir() * m_camDistance;
   vec3 newUp { 0.f, 0.f, -1.f }, newDir;
   switch (view)
   {
   case PredefinedView::Top:
      newUp = vec3(0.f, -1.f, 0.f);
      newDir = vec3(0.f, 0.f, -1.f);
      break;
   case PredefinedView::Bottom:
      newUp = vec3(0.f, -1.f, 0.f);
      newDir = vec3(0.f, 0.f, 1.f);
      break;
   case PredefinedView::Front: newDir = vec3(0.f, 1.f, 0.f); break;
   case PredefinedView::Back: newDir = vec3(0.f, -1.f, 0.f); break;
   case PredefinedView::Left: newDir = vec3(-1.f, 0.f, 0.f); break;
   case PredefinedView::Right: newDir = vec3(1.f, 0.f, 0.f); break;
   default: return;
   }
   m_camView = Matrix3D::MatrixLookAtRH(camTarget + newDir * m_camDistance, camTarget, newUp);
}

void ViewportPane::FrameSelection()
{
   // Camera centered on the whole selection, adjusting the zoom so that it fills the view
   FRect3D bounds;
   if (m_editor.GetSelectionBounds(bounds))
   {
      Matrix3D view(m_camView);
      view.Invert();
      const vec3 up = view.GetOrthoNormalUp();
      const vec3 dir = view.GetOrthoNormalDir();
      const vec3 right = view.GetOrthoNormalRight();
      const vec3 newTarget(0.5f * (bounds.left + bounds.right), 0.5f * (bounds.top + bounds.bottom), -0.5f * (bounds.zlow + bounds.zhigh));

      // Evaluate the distance needed for all the bounds corners to be inside the view, keeping the
      // camera orientation (the pane fits its render in its own rect: no chrome margin applies)
      const ImGuiIO& io = ImGui::GetIO();
      const float aspect = m_imageRect.Height() > 0.f ? m_imageRect.Width() / m_imageRect.Height() : io.DisplaySize.x / io.DisplaySize.y;
      const float tanHalfFovY = tanf(0.5f * ANGTORAD(editorCamFovY));
      float distance = 0.f;
      for (int i = 0; i < 8; i++)
      {
         const vec3 toCorner = vec3((i & 1) ? bounds.right : bounds.left, (i & 2) ? bounds.bottom : bounds.top, (i & 4) ? -bounds.zlow : -bounds.zhigh) - newTarget;
         const float dx = fabsf(right.Dot(toCorner));
         const float dy = fabsf(up.Dot(toCorner));
         distance = max(distance, m_perspectiveCam ? dir.Dot(toCorner) + max(dy / tanHalfFovY, dx / (tanHalfFovY * aspect)) : max(dy, dx / aspect));
      }
      if (distance > 0.f) // Keep the current distance for degenerate (point) selections
         m_camDistance = distance * 1.1f; // Small margin so that the selection does not exactly touch the view borders
      const vec3 newEye = newTarget + dir * m_camDistance;
      m_camView = Matrix3D::MatrixLookAtRH(newEye, newTarget, up);
   }
}

void ViewportPane::SetPlayfieldCamera()
{
   // Setup the camera to frame the playfield bounds from a top view (table XY plane, Y axis going down;
   // the pane fits its render in its own rect: no chrome margin applies)
   const ImGuiIO& io = ImGui::GetIO();
   const float aspect = m_imageRect.Height() > 0.f ? m_imageRect.Width() / m_imageRect.Height() : io.DisplaySize.x / io.DisplaySize.y;
   m_camDistance = 0.5f * max(m_editor.m_table->m_bottom, m_editor.m_table->m_right / aspect);
   const vec3 eye(m_editor.m_table->m_right * 0.5f, m_editor.m_table->m_bottom * 0.5f, -m_camDistance);
   const vec3 at(m_editor.m_table->m_right * 0.5f, m_editor.m_table->m_bottom * 0.5f, 0.f);
   constexpr vec3 up { 0.f, -1.f, 0.f };
   m_camView = Matrix3D::MatrixLookAtRH(eye, at, up);
}

nlohmann::json ViewportPane::SaveSetup() const
{
   nlohmann::json j = nlohmann::json::object();
   j["camMode"] = static_cast<int>(m_viewType);
   j["persp"] = m_perspectiveCam;
   j["shade"] = static_cast<int>(m_shadeMode);
   j["predef"] = static_cast<int>(m_predefinedView);
   j["orbitLock"] = m_orbitLock;
   j["camDist"] = m_camDistance;
   j["camAspect"] = m_camAspect;
   j["camValid"] = m_cameraValid;
   j["camView"] = nlohmann::json::array();
   j["camProj"] = nlohmann::json::array();
   for (int r = 0; r < 4; ++r)
      for (int c = 0; c < 4; ++c)
      {
         j["camView"].push_back(m_camView.m[r][c]);
         j["camProj"].push_back(m_camProj.m[r][c]);
      }
   return j;
}

void ViewportPane::LoadSetup(const nlohmann::json& j)
{
   if (!j.is_object())
      return;
   const auto mat = [&j](const char* const key, Matrix3D& out)
   {
      if (!j.contains(key) || !j[key].is_array() || j[key].size() != 16)
         return false;
      float v[16];
      for (int i = 0; i < 16; ++i)
      {
         if (!j[key][i].is_number())
            return false;
         v[i] = j[key][i].get<float>();
         if (!std::isfinite(v[i]))
            return false;
      }
      for (int r = 0; r < 4; ++r)
         for (int c = 0; c < 4; ++c)
            out.m[r][c] = v[r * 4 + c];
      return true;
   };
   const int camMode = j.value("camMode", -1);
   const int shade = j.value("shade", -1);
   const int predef = j.value("predef", -1);
   if (camMode < 0 || camMode > static_cast<int>(ViewMode::DesktopBackdrop) //
      || shade < 0 || shade > static_cast<int>(Renderer::ShadeMode::NoDepthWireframe) //
      || predef < 0 || predef > static_cast<int>(PredefinedView::Bottom))
      return;
   if (static_cast<ViewMode>(camMode) != m_viewType)
      return; // A type switch reuses the leaf's setup: a camera stored for another mode does not apply
   const float dist = j.value("camDist", 0.f);
   const float aspect = j.value("camAspect", 0.f);
   if (!std::isfinite(dist) || dist <= 0.f || !std::isfinite(aspect) || aspect <= 0.f)
      return;
   Matrix3D view, proj;
   if (!mat("camView", view) || !mat("camProj", proj))
      return;
   m_shadeMode = static_cast<Renderer::ShadeMode>(shade);
   m_predefinedView = static_cast<PredefinedView>(predef);
   m_camDistance = dist;
   m_camAspect = aspect;
   m_camView = view;
   m_camProj = proj;
   m_perspectiveCam = j.value("persp", true);
   m_orbitLock = j.value("orbitLock", false);
   m_cameraValid = j.value("camValid", true);
}

bool ViewportPane::GetViewProjLH(Matrix3D& viewLH, Matrix3D& projLH) const
{
   // The render view's MVP is the single transform shared by the scene rendering (Render3D pushes
   // this pane's camera into it) and the overlays/picking done from RenderUI: both always agree
   if (m_view == nullptr)
      return false;
   viewLH = m_view->GetMVP().GetView(0);
   projLH = m_view->GetMVP().GetProj(0);
   return true;
}

ImVec2 ViewportPane::Project(const Vertex3Ds& v) const
{
   Matrix3D view, proj;
   if (!GetViewProjLH(view, proj) || m_imageRect.Width() <= 0.f || m_imageRect.Height() <= 0.f)
      return ImVec2 { FLT_MAX, FLT_MAX };
   const Matrix3D mvp = view * proj;
   const float xp = mvp._11 * v.x + mvp._21 * v.y + mvp._31 * v.z + mvp._41;
   const float yp = mvp._12 * v.x + mvp._22 * v.y + mvp._32 * v.z + mvp._42;
   const float wp = mvp._14 * v.x + mvp._24 * v.y + mvp._34 * v.z + mvp._44;
   if (wp <= 1e-10f) // Behind the camera (or degenerated)
      return ImVec2 { FLT_MAX, FLT_MAX };
   const float invW = 1.f / wp;
   return ImVec2 { m_imageRect.x0 + (1.f + xp * invW) * 0.5f * m_imageRect.Width(), m_imageRect.y0 + (1.f - yp * invW) * 0.5f * m_imageRect.Height() };
}

bool ViewportPane::PickRay(const ImVec2& pos, Vertex3Ds& nearPt, Vertex3Ds& farPt) const
{
   Matrix3D view, proj;
   if (!GetViewProjLH(view, proj) || m_imageRect.Width() <= 0.f || m_imageRect.Height() <= 0.f)
      return false;
   const float x = 2.f * (pos.x - m_imageRect.x0) / m_imageRect.Width() - 1.f;
   const float y = 1.f - 2.f * (pos.y - m_imageRect.y0) / m_imageRect.Height();
   Matrix3D invMvp = view * proj;
   invMvp.Invert();
   nearPt = invMvp * Vertex3Ds { x, y, 0.f };
   farPt = invMvp * Vertex3Ds { x, y, 1.f };
   return true;
}

bool ViewportPane::SetupOverlay(LiveRenderContext* const ctx, ImDrawList* const drawList) const
{
   Matrix3D view, proj;
   if (!GetViewProjLH(view, proj) || m_imageRect.Width() <= 0.f || m_imageRect.Height() <= 0.f)
      return false;
   ctx->SetPaneProjection(view * proj, ImVec2(m_imageRect.x0, m_imageRect.y0), ImVec2(1.f, 1.f), m_imageRect.Width() * 0.5f, m_imageRect.Height() * 0.5f);
   if (drawList != nullptr)
      drawList->PushClipRect(ImVec2(m_imageRect.x0, m_imageRect.y0), ImVec2(m_imageRect.x1, m_imageRect.y1), true);
   return true;
}

void ViewportPane::RenderPane(const AreaRect& rect, const int leafId)
{
   EditorUI& editor = m_editor;
   m_leafId = leafId;
   if (m_view == nullptr)
   {
      m_view = editor.m_renderer->CreateEditorView();
      m_view->m_outputWnd = editor.m_player->m_playfieldWnd; // The view draws to its output area of the shared window backbuffer
      m_view->m_renderOverlay3D = true;
      m_view->m_applyPostFX = false; // Keep per-pane rendering cheap: no postprocess tail
   }
   m_view->m_renderScene = true; // The leaf is shown (Reset by the editor for hidden leaves)

   char name[40];
   snprintf(name, sizeof(name), "##Pane%d", leafId);
   ImGui::SetNextWindowPos(ImVec2(rect.x0, rect.y0));
   ImGui::SetNextWindowSize(ImVec2(rect.Width(), rect.Height()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
   // NoBackground: the scene is drawn by this pane's view straight into the window backbuffer
   // (the UI is composited over it by the deferred LiveUI pass), so no window background may cover it
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
         | ImGuiWindowFlags_NoNavFocus);
   ImGui::PushID(leafId);
   m_window = ImGui::GetCurrentWindow();
   const float dpi = editor.m_liveUI.GetDPI();

   // Camera state of this pane
   const ViewMode camMode = m_viewType;
   bool& perspective = m_perspectiveCam;
   Renderer::ShadeMode& shade = m_shadeMode;
   bool& orbitLock = m_orbitLock;

   // Header bar (Blender): a strip at the top of the area, opening with the editor type switcher,
   // then the controls of this view. It is drawn on the draw list back channel so that it fills
   // behind the widgets, whatever the height they wrap to on a narrow area
   const float pad = 2.f * dpi;
   const ImVec2 winPos = ImGui::GetWindowPos();
   ImDrawList* const drawList = ImGui::GetWindowDrawList();
   drawList->ChannelsSplit(2);
   drawList->ChannelsSetCurrent(1);
   ImGui::SetCursorPos(ImVec2(pad, pad));

   editor.RenderAreaTypeButton(leafId);
   if (m_viewType == ViewMode::PreviewCam)
   {
      // View mode selection (desktop / FSS / cabinet)
      ImGui::SameLine();
      static const struct
      {
         ViewSetupID mode;
         const char* label;
      } viewModes[] = {
         { BG_DESKTOP, "Desktop" },
         { BG_FSS, "FSS" },
         { BG_FULLSCREEN, "Cabinet" },
      };
      for (const auto& mode : viewModes)
      {
         const bool active = (m_viewMode == mode.mode);
         if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
         if (ImGui::Button(mode.label))
            m_viewMode = mode.mode;
         if (active)
            ImGui::PopStyleColor();
         ImGui::SameLine();
      }
      ImGui::NewLine();
   }
   else // A render preview has no toolbar (no selection, shading, camera edits, ...)
   {
      ImGui::SameLine();
      if (ImGui::Button(perspective ? ICON_FK_VIDEO_CAMERA "##Persp" : ICON_FK_TH "##Persp"))
         perspective = !perspective;
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip(perspective ? "Perspective camera" : "Orthographic camera");
      ImGui::SameLine();
      // Shading mode radio buttons (icon group, same style as the edit modes)
      static const struct
      {
         Renderer::ShadeMode mode;
         const char* icon;
         const char* tooltip;
      } shadeModes[] = {
         { Renderer::ShadeMode::Default, ICON_FK_CUBE, "Solid" },
         { Renderer::ShadeMode::Wireframe, ICON_FK_WIRE, "Wireframe" },
         { Renderer::ShadeMode::NoDepthWireframe, ICON_FK_DASHCUBE, "Wireframe (no depth)" },
      };
      for (const auto& mode : shadeModes)
      {
         const bool active = (shade == mode.mode);
         if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
         if (ImGui::Button(mode.icon))
            shade = mode.mode;
         if (active)
            ImGui::PopStyleColor();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", mode.tooltip);
         ImGui::SameLine();
      }

      if (!editor.IsInspectMode() && m_viewType != ViewMode::PreviewCam)
      {
         ImGui::SameLine();
         ImGui::Separator();
         ImGui::SameLine();

         // Edit mode selection (same actions as the Esc, G, S and R keyboard shortcuts)
         static const struct
         {
            ImGuizmo::OPERATION operation;
            const char* icon;
            const char* tooltip;
         } editModes[] = {
            { (ImGuizmo::OPERATION)0, ICON_FK_MOUSE_POINTER, "Select\n[Esc]" }, //
            { ImGuizmo::TRANSLATE, ICON_FK_ARROWS, "Grab\n[G]" }, //
            { ImGuizmo::SCALE, ICON_FK_EXPAND, "Scale\n[S]" }, //
            { ImGuizmo::ROTATE, ICON_FK_REPEAT, "Rotate\n[R]" }, //
         };
         for (const auto& mode : editModes)
         {
            const bool active = (m_gizmoOperation == mode.operation);
            if (active)
               ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            if (ImGui::Button(mode.icon))
               SetGizmoOperation(mode.operation);
            if (active)
               ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
               ImGui::SetTooltip("%s", mode.tooltip);
            ImGui::SameLine();
         }
         ImGui::Separator();
         ImGui::SameLine();

         // Add part button, opening the part type picker popup (same as the Shift+A shortcut)
         ImGui::BeginDisabled(editor.m_pointEditPart != nullptr || editor.m_table->IsLocked());
         const bool addPartActive = (editor.m_addPartType != eItemInvalid);
         if (addPartActive)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
         if (ImGui::Button(ICON_FK_PLUS))
            editor.RequestAddPartPopup(ImGui::GetMousePos());
         if (addPartActive)
            ImGui::PopStyleColor();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add part\n[Shift+A]");
         ImGui::EndDisabled();
         ImGui::SameLine();
      }

      // Right side of the header: viewport overlays, selection filter and camera orbit lock
      const ImGuiStyle& style = ImGui::GetStyle();
      const float rightW = //
         ImGui::CalcTextSize(ICON_FK_STICKY_NOTE, nullptr, true).x + style.FramePadding.x * 2.0f //
         + ImGui::CalcTextSize(ICON_FK_FILTER, nullptr, true).x + style.FramePadding.x * 2.0f //
         + ImGui::CalcTextSize(ICON_FK_LOCK, nullptr, true).x + style.FramePadding.x * 2.0f //
         + style.ItemSpacing.x * 2.0f;
      const float rightX = rect.Width() - pad - rightW;
      ImGui::SameLine();
      if (rightX > ImGui::GetCursorPosX())
         ImGui::SetCursorPosX(rightX);
      if (ImGui::Button(ICON_FK_STICKY_NOTE)) // Overlay options popover
         ImGui::OpenPopup("Overlay Popup");
      if (ImGui::BeginPopup("Overlay Popup"))
      {
         ImGui::TextUnformatted("Overlays:");
         ImGui::Separator();
         ImGui::Checkbox("Overlay selection", &editor.m_selectionOverlay);
         ImGui::Separator();
         ImGui::TextUnformatted("Physic Overlay:");
         if (ImGui::RadioButton("None", editor.m_physOverlay == EditorUI::PhysicOverlay::None))
            editor.m_physOverlay = EditorUI::PhysicOverlay::None;
         if (ImGui::RadioButton("Selected", editor.m_physOverlay == EditorUI::PhysicOverlay::Selected))
            editor.m_physOverlay = EditorUI::PhysicOverlay::Selected;
         if (ImGui::RadioButton("All", editor.m_physOverlay == EditorUI::PhysicOverlay::All))
            editor.m_physOverlay = EditorUI::PhysicOverlay::All;
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FK_FILTER)) // Selection filter popover
         ImGui::OpenPopup("Selection filter Popup");
      if (ImGui::BeginPopup("Selection filter Popup"))
      {
         bool pf = EditorUI::HasFlag(editor.m_selectionFilter, EditorUI::SelectionFilter::Playfield);
         bool prims = EditorUI::HasFlag(editor.m_selectionFilter, EditorUI::SelectionFilter::Primitives);
         bool lights = EditorUI::HasFlag(editor.m_selectionFilter, EditorUI::SelectionFilter::Lights);
         bool flashers = EditorUI::HasFlag(editor.m_selectionFilter, EditorUI::SelectionFilter::Flashers);
         ImGui::TextUnformatted("Selection filters:");
         ImGui::Separator();
         if (ImGui::Checkbox("Playfield", &pf))
            editor.m_selectionFilter = pf ? (editor.m_selectionFilter | EditorUI::SelectionFilter::Playfield) : (editor.m_selectionFilter & ~EditorUI::SelectionFilter::Playfield);
         if (ImGui::Checkbox("Primitives", &prims))
            editor.m_selectionFilter = prims ? (editor.m_selectionFilter | EditorUI::SelectionFilter::Primitives) : (editor.m_selectionFilter & ~EditorUI::SelectionFilter::Primitives);
         if (ImGui::Checkbox("Lights", &lights))
            editor.m_selectionFilter = lights ? (editor.m_selectionFilter | EditorUI::SelectionFilter::Lights) : (editor.m_selectionFilter & ~EditorUI::SelectionFilter::Lights);
         if (ImGui::Checkbox("Flashers", &flashers))
            editor.m_selectionFilter = flashers ? (editor.m_selectionFilter | EditorUI::SelectionFilter::Flashers) : (editor.m_selectionFilter & ~EditorUI::SelectionFilter::Flashers);
         ImGui::EndPopup();
      }
      ImGui::SameLine();
      if (orbitLock)
         ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
      if (ImGui::Button(orbitLock ? ICON_FK_LOCK : ICON_FK_UNLOCK))
         orbitLock = !orbitLock;
      if (orbitLock)
         ImGui::PopStyleColor();
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Lock camera orbit\n[Numpad /]");
   }

   const float headerBottom = ImGui::GetItemRectMax().y + pad;
   drawList->ChannelsSetCurrent(0);
   drawList->AddRectFilled(winPos, ImVec2(winPos.x + rect.Width(), headerBottom), EditorTheme::Bar);
   drawList->AddLine(ImVec2(winPos.x, headerBottom - 0.5f), ImVec2(winPos.x + rect.Width(), headerBottom - 0.5f), EditorTheme::Border);
   drawList->ChannelsMerge();
   ImGui::SetCursorPosY(headerBottom - winPos.y);

   // Part type picker popup (header button and Shift+A shortcut): picking a type arms add part mode,
   // clicking outside of the popup dismisses it without arming add part mode
   if (editor.m_addPartPopupLeaf == leafId)
   {
      editor.m_addPartPopupLeaf = -1;
      ImGui::OpenPopup("Add Part");
   }
   ImGui::SetNextWindowPos(editor.m_addPartPopupPos, ImGuiCond_Appearing);
   if (ImGui::BeginPopup("Add Part"))
   {
      editor.EnsureAddPartButtons();
      const float iconSize = ImGui::GetFrameHeight();
      int column = 0;
      for (const auto& button : editor.m_addPartButtons)
      {
         if (button.icon == nullptr)
            continue;
         if (column > 0)
            ImGui::SameLine();
         if (ImGui::ImageButton(button.name, button.icon, ImVec2(iconSize, iconSize)))
         {
            editor.m_addPartType = button.type;
            ImGui::CloseCurrentPopup();
         }
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add %s", button.name);
         column = (column + 1) % 5;
      }
      ImGui::EndPopup();
   }

   // The view renders its scene into the part of this pane left by the header
   const ImVec2 avail = ImGui::GetContentRegionAvail();
   const ImVec2 imgPos = ImGui::GetCursorScreenPos();
   m_imageRect = AreaRect { imgPos.x, imgPos.y, imgPos.x + avail.x, imgPos.y + avail.y };
   const ImVec2 disp = ImGui::GetIO().DisplaySize;
   // Defer the render target chain rebuild while an area op is resizing the pane: recreating all the view and
   // probe targets on every mouse move breaks the accumulated renders (darkened view) and stalls the frame
   m_view->m_deferResize = editor.m_splitDragNode >= 0 || editor.m_areaDrag.active;
   const bool validArea = avail.x > 0.f && avail.y > 0.f; // A degenerate area (collapsed pane) must not drive the view's output rect
   if (validArea)
      m_view->SetOutputArea(vec4(m_imageRect.x0 / disp.x, m_imageRect.y0 / disp.y, m_imageRect.x1 / disp.x, m_imageRect.y1 / disp.y));
   else
      m_view->m_renderScene = false;
   ImGui::InvisibleButton("##view", ImVec2(max(1.f, avail.x), max(1.f, avail.y)));
   m_viewItemId = ImGui::GetItemID(); // The pane's scene item: its ID must not count as a widget (see RenderAreaCorners)
   m_hovered = ImGui::IsItemHovered();
   m_active = ImGui::IsItemActive();
   if (m_viewType != ViewMode::PreviewCam) // No status text over a render preview
      RenderStatusOverlay();
   // Thin rounded border around the pane (Blender pane look)
   ImGui::GetWindowDrawList()->AddRect(m_window->Pos, m_window->Pos + m_window->Size, EditorTheme::Border, 3.f * dpi);
   ImGui::PopID();
   ImGui::End();
   ImGui::PopStyleVar(3);

   // Per frame view setup: editor flags and shading; the camera of the edit viewport is pushed by
   // RenderUI, the other panes restore their stored camera state
   m_view->m_editorView = camMode != ViewMode::PreviewCam; // A render preview displays the game view, not an editor view
   m_view->m_editorBackdropEdit = camMode == ViewMode::DesktopBackdrop;
   const ViewSetupID modeOverride = camMode == ViewMode::PreviewCam ? m_viewMode : BG_INVALID;
   if (m_view->m_viewModeOverride != modeOverride)
      m_view->m_isStaticPrepassDirty = true; // A mode change alters the backdrop/visibility: the accumulated scene is stale
   m_view->m_viewModeOverride = modeOverride;
   m_view->m_shadeMode = m_shadeMode;
}

void ViewportPane::BoxSelectParts(const ImVec2& cornerA, const ImVec2& cornerB, bool add)
{
   // Select all the pickable parts whose transform position projects inside the given screen box
   if (!add)
   {
      m_editor.m_multiSel.clear();
      m_editor.m_selection = Selection();
   }
   LiveRenderContext ctx(m_editor.m_player, nullptr, m_viewType, m_shadeMode, false);
   SetupOverlay(&ctx, nullptr);
   const ImVec2 boxMin(std::min(cornerA.x, cornerB.x), std::min(cornerA.y, cornerB.y));
   const ImVec2 boxMax(std::max(cornerA.x, cornerB.x), std::max(cornerA.y, cornerB.y));
   for (const auto& uiPart : m_editor.m_editables)
   {
      IEditable* const editable = uiPart->GetEditable();
      if (editable == nullptr || editable->GetItemType() == eItemPartGroup || !m_editor.IsEditablePickable(editable))
         continue;
      Matrix3D transform;
      if (uiPart->GetTransform(transform) == EditorUIPart::TM_None)
         continue;
      const ImVec2 pos = ctx.Project(transform.GetOrthoNormalPos());
      if (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y && !m_editor.IsPartSelected(uiPart))
         m_editor.m_multiSel.push_back(uiPart);
   }
   if (!m_editor.m_multiSel.empty() && (m_editor.m_selection.GetType() != Selection::S_EDITABLE || !m_editor.IsPartSelected(m_editor.m_selection.GetPart())))
      m_editor.m_selection = Selection(m_editor.m_multiSel.back());
}

void ViewportPane::BoxSelectPoints(const ImVec2& cornerA, const ImVec2& cornerB, bool add)
{
   // Select all the drag points of the edited curve projecting inside the given screen box
   if (!add)
      m_editor.m_pointSel.clear();
   LiveRenderContext ctx(m_editor.m_player, nullptr, m_viewType, m_shadeMode, false);
   SetupOverlay(&ctx, nullptr);
   const ImVec2 boxMin(std::min(cornerA.x, cornerB.x), std::min(cornerA.y, cornerB.y));
   const ImVec2 boxMax(std::max(cornerA.x, cornerB.x), std::max(cornerA.y, cornerB.y));
   if (m_editor.m_pointEditCenter)
   {
      // In center edit mode, select the part's center point if it projects inside the box
      const ImVec2 pos = ctx.Project(m_editor.m_pointEditPart->GetEditCenter());
      if (!add)
         m_editor.m_centerSelected = false;
      if (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y)
         m_editor.m_centerSelected = true;
      return;
   }
   for (const auto& point : m_editor.m_pointEditPart->GetDragPointCurve()->GetPoints())
   {
      const ImVec2 pos = ctx.Project(Vertex3Ds(point->GetX(), point->GetY(), m_editor.m_pointEditPart->GetDragPointZ(point.get())));
      if (pos.x >= boxMin.x && pos.x <= boxMax.x && pos.y >= boxMin.y && pos.y <= boxMax.y && !m_editor.IsPointSelected(point.get()))
         m_editor.m_pointSel.push_back(point.get());
   }
}

void ViewportPane::RenderStatusOverlay()
{
   // Overlay Info Text: top left of the pane's image, below its header bar
   if (m_imageRect.Width() <= 0.f || m_imageRect.Height() <= 0.f)
      return;
   const float dpi = m_editor.m_liveUI.GetDPI();
   const ImVec2 overlayPos(m_imageRect.x0 + 5.f * dpi, m_imageRect.y0 + 5.f * dpi);

   ImGui::SetNextWindowSize(ImVec2(max(1.f, m_imageRect.x1 - overlayPos.x), max(1.f, m_imageRect.y1 - overlayPos.y)));
   ImGui::SetNextWindowPos(overlayPos);
   ImGui::Begin("text overlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
   const float labelAlign = ImGui::GetCursorPosX() + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x;
   ImGui::SetCursorPos(ImVec2(labelAlign, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y));
   switch (m_viewType)
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
   ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + ImGui::GetStyle().FramePadding.x, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y));
   const char* gizmoIcon = nullptr;
   const char* gizmoLabel = nullptr;
   if (m_gizmoOperation == ImGuizmo::OPERATION(0))
   {
      gizmoIcon = ICON_FK_MOUSE_POINTER;
      gizmoLabel = "Select";
   }
   else if (m_gizmoOperation == ImGuizmo::TRANSLATE)
   {
      gizmoIcon = ICON_FK_ARROWS;
      gizmoLabel = "Grab";
   }
   else if (m_gizmoOperation == ImGuizmo::SCALE)
   {
      gizmoIcon = ICON_FK_EXPAND;
      gizmoLabel = "Scale";
   }
   else if (m_gizmoOperation == ImGuizmo::ROTATE)
   {
      gizmoIcon = ICON_FK_REPEAT;
      gizmoLabel = "Rotate";
   }
   if (gizmoIcon)
   {
      ImGui::TextUnformatted(gizmoIcon);
      ImGui::SameLine(labelAlign);
      ImGui::TextUnformatted(gizmoLabel);
   }
   if (m_editor.GetTargetViewport() == this && m_editor.m_pointEditPart)
      ImGui::TextUnformatted(m_editor.m_pointEditCenter ? "Center Edit (Tab to exit)" : "Drag Point Edit (Tab to exit)");
   ImGui::End();
}

void ViewportPane::Render3D()
{
   if (m_view == nullptr)
      return;
   // Setup the MVP of the view being recorded with this pane's camera: the default table layout
   // for the preview camera, else the pane's stored camera converted to the renderer's left handed space
   if (m_viewType == ViewMode::PreviewCam || !m_cameraValid)
      m_view->InitLayout();
   else
   {
      const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
      const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
      m_view->SetViewProj(RH2LH * m_camView * YAxis, YAxis * m_camProj);
   }

   // Render the scene parts into the view being recorded, filtered by this pane's camera mode and shading
   LiveRenderContext ctx(m_editor.m_player, nullptr, m_viewType, m_viewType == ViewMode::PreviewCam ? Renderer::ShadeMode::Default : m_shadeMode,
      (m_editor.m_table->m_liveBaseTable != nullptr) && m_editor.m_player->IsPlaying());
   const bool backdrop = m_viewType == ViewMode::DesktopBackdrop;
   for (const auto& uiPart : m_editor.m_editables)
   {
      if (uiPart->GetEditable()->m_desktopBackdrop != backdrop)
         continue;
      uiPart->Render(ctx);
   }
   // Selection overlay: the parts' 3D-rendered selection wireframes must be recorded into this
   // pane's view (drawn with its MVP), suspended while a save is running (the part renders would
   // write into the parts' data being serialized on the worker thread)
   if (m_viewType != ViewMode::PreviewCam && !m_editor.m_multiSel.empty() && !m_editor.IsSaveInProgress())
   {
      if (m_editor.m_renderer->m_renderDevice->GetCurrentRenderTarget()->HasDepth())
         m_editor.m_renderer->m_renderDevice->Clear(clearType::ZBUFFER, 0);
      ctx.m_isSelected = true;
      for (const auto& part : m_editor.m_multiSel)
      {
         if (part->GetEditable()->m_desktopBackdrop != backdrop)
            continue; // Only the parts of this pane's mode are overlaid here
         ctx.m_isActive = part == m_editor.m_selection.GetPart();
         part->Render(ctx);
      }
      ctx.m_isActive = false;
      ctx.m_isSelected = false;
   }
}

void ViewportPane::RayCastParts(const ImVec2& mousePos, vector<HitTestResult>& vhoHit)
{
   // Use the inverse of this pane's 3D transform to determine where in 3D space the clicked screen
   // pixel is at
   Vertex3Ds v3d, v3d2;
   if (!PickRay(mousePos, v3d, v3d2))
      return;

   // FIXME This is not really great as:
   // - picking depends on what was visible/enabled when quadtree was built (lazily at first pick), and also uses the physics quadtree for some parts
   // - primitives can have hit bug (Apron Top and Gottlieb arm of default table for example): degenerated geometry ?
   // We would need a dedicated quadtree for UI with all parts, and filter after picking by visibility
   vector<HitTestResult> vhoUnfilteredHit;
   m_editor.m_player->m_physics->RayCast(v3d, v3d2, true, vhoUnfilteredHit);

   // Filter out the colliders that should not be picked
   for (const auto& hr : vhoUnfilteredHit)
   {
      const auto editable = hr.m_obj->m_editable;
      if (editable && !m_editor.IsEditablePickable(editable))
         continue;
      vhoHit.push_back(hr);
   }
}

Vertex2D ViewportPane::UnprojectToPlane(const ImVec2& mousePos, float z) const
{
   // Compute this pane's mouse ray and intersect it with the table plane at the given Z coordinate
   Vertex3Ds v3d, v3d2;
   if (!PickRay(mousePos, v3d, v3d2))
      return Vertex2D(0.f, 0.f);
   const float dz = v3d2.z - v3d.z;
   const float t = (fabsf(dz) > 1e-10f) ? (z - v3d.z) / dz : 0.f;
   return Vertex2D(v3d.x + t * (v3d2.x - v3d.x), v3d.y + t * (v3d2.y - v3d.y));
}

bool ViewportPane::HitTestEditCenter(const ImVec2& mousePos) const
{
   LiveRenderContext ctx(m_editor.m_player, nullptr, m_viewType, m_shadeMode, false);
   SetupOverlay(&ctx, nullptr);
   const ImVec2 pos = ctx.Project(m_editor.m_pointEditPart->GetEditCenter());
   if (pos.x == FLT_MAX)
      return false;
   const float maxDist = 10.f * m_editor.m_liveUI.GetDPI();
   const float dx = pos.x - mousePos.x;
   const float dy = pos.y - mousePos.y;
   return dx * dx + dy * dy < maxDist * maxDist;
}

DragPoint* ViewportPane::HitTestDragPoint(const ImVec2& mousePos) const
{
   LiveRenderContext ctx(m_editor.m_player, nullptr, m_viewType, m_shadeMode, false);
   SetupOverlay(&ctx, nullptr);
   const float maxDist = 10.f * m_editor.m_liveUI.GetDPI();
   DragPoint* best = nullptr;
   float bestDist = maxDist;
   for (const auto& point : m_editor.m_pointEditPart->GetDragPointCurve()->GetPoints())
   {
      const ImVec2 pos = ctx.Project(Vertex3Ds(point->GetX(), point->GetY(), m_editor.m_pointEditPart->GetDragPointZ(point.get())));
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

void ViewportPane::RenderUI(ImDrawList* const overlayDrawList)
{
   if (!m_cameraValid)
   {
      // First use: clone the camera of the current target pane (split panes share the setup, like
      // Blender), else start from a default playfield pose; then apply the setup requested at
      // editor opening if one is pending (it only goes to a pane without a restored camera)
      ViewportPane* const other = m_editor.GetTargetViewport();
      AdoptCameraFrom(other != this ? other : nullptr);
      if (m_editor.m_camSetupPending && m_editor.GetTargetViewport() == this && m_viewType == m_editor.m_camSetupPending->mode)
      {
         m_perspectiveCam = m_editor.m_camSetupPending->persp;
         m_shadeMode = m_editor.m_camSetupPending->shade;
         m_predefinedView = m_editor.m_camSetupPending->predef;
         m_editor.m_camSetupPending.reset();
         if (m_editor.m_fitPlayfieldCamera && m_imageRect.Height() > 0.f)
         {
            m_editor.m_fitPlayfieldCamera = false;
            SetPlayfieldCamera();
         }
      }
   }
   // A restored (or adopted) camera wins over the session entry setup: discard it so a later pane
   // does not get an unexpected entry view
   else if (m_editor.GetTargetViewport() == this && m_editor.m_camSetupPending)
   {
      m_editor.m_camSetupPending.reset();
      m_editor.m_fitPlayfieldCamera = false;
   }

   const ImGuiIO& io = ImGui::GetIO();
   // Called on every pane: the processing below works on this pane's own camera definition and its
   // image rect (the one under the mouse, or capturing a drag, handles input and keyboard)

   // The selection gizmo operates on the viewport's image rect
   ImGuizmo::SetOrthographic(m_viewType == ViewMode::DesktopBackdrop || (m_viewType == ViewMode::EditorCam && !m_perspectiveCam));
   const AreaRect gizmoRect = m_imageRect;
   ImGuizmo::SetRect(gizmoRect.x0, gizmoRect.y0, gizmoRect.Width(), gizmoRect.Height());
   // The gizmo only accepts hover over its drawlist window or the registered alternative: the
   // mouse over this pane's window must be a valid gizmo hover (see ImGuizmo::IsHoveringWindow)
   ImGuizmo::SetAlternativeWindow(m_window);

   // Update this pane's camera and push it to its render view
   RenderView* const camOut = m_view;
   const float camAspect = m_imageRect.Height() > 0.f ? m_imageRect.Width() / m_imageRect.Height() : io.DisplaySize.x / io.DisplaySize.y;
   const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
   const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
   if (m_viewType == ViewMode::PreviewCam)
   {
      camOut->InitLayout();
      m_camView = RH2LH * camOut->GetMVP().GetView(0) * YAxis;
      m_camProj = YAxis * camOut->GetMVP().GetProj(0);
   }
   else
   {
      if (m_perspectiveCam && m_viewType != ViewMode::DesktopBackdrop) // The desktop backdrop is a 2D view, always edited with an orthographic camera
      {
         // Convert from right handed (ImGuizmo view manipulate is right handed) to VPX's left handed coordinate system
         // Right Hand to Left Hand (note that RH2LH = inverse(RH2LH), so RH2LH.RH2LH is identity, which property is used below)
         //const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
         //const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, -1.f);
         //float zNear, zFar;
         //m_editor.m_table->ComputeNearFarPlane(RH2LH * m_camView * YAxis, 1.f, zNear, zFar);
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

      // The pane's camera is pushed to the render view by Render3D (during the render pass)
   }

   // Selection manipulator
   Matrix3D transform;
   // A Render Preview pane shows the game view only: no gizmo, no overlays, no scene interaction
   if (m_viewType == ViewMode::PreviewCam)
      return;

   const bool isSelectionTransformValid = m_editor.GetSelectionTransform(transform);
   if (isSelectionTransformValid && !m_editor.m_table->IsLocked())
   {
      // The gizmo shares the render view's MVP (the same transform as the scene render), converted
      // back to its right handed convention
      Matrix3D viewLH, projLH;
      if (!GetViewProjLH(viewLH, projLH))
      {
         viewLH = RH2LH * m_camView * YAxis;
         projLH = YAxis * m_camProj;
      }
      const Matrix3D gizmoView = viewLH * YAxis;
      const Matrix3D gizmoProj = YAxis * projLH;
      float camViewLH[16];
      memcpy(camViewLH, &gizmoView.m[0][0], sizeof(float) * 4 * 4);
      const Matrix3D prevTransform(transform);
      ImGuizmo::OPERATION gizmoOperation = m_gizmoOperation;
      if (m_editor.m_pointEditPart || m_viewType == ViewMode::DesktopBackdrop)
      {
         // Drag point curves are 2D in the table XY plane, and backdrop parts are 2D in the backdrop XY plane: restrict gizmo operations to this plane
         if (gizmoOperation == ImGuizmo::TRANSLATE)
            gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y);
         else if (gizmoOperation == ImGuizmo::ROTATE)
            gizmoOperation = ImGuizmo::ROTATE_Z;
         else if (gizmoOperation == ImGuizmo::SCALE)
            gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y);
      }
      else if (m_viewType == ViewMode::EditorCam)
      {
         // Predefined views are axis aligned 2D views: restrict gizmo operations to the view plane
         switch (m_predefinedView)
         {
         case PredefinedView::None: break;
         case PredefinedView::Left:
         case PredefinedView::Right:
            if (gizmoOperation == ImGuizmo::TRANSLATE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_Y | ImGuizmo::TRANSLATE_Z);
            else if (gizmoOperation == ImGuizmo::ROTATE)
               gizmoOperation = ImGuizmo::ROTATE_X;
            else if (gizmoOperation == ImGuizmo::SCALE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_Y | ImGuizmo::SCALE_Z);
            break;
         case PredefinedView::Top:
         case PredefinedView::Bottom:
            if (gizmoOperation == ImGuizmo::TRANSLATE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y);
            else if (gizmoOperation == ImGuizmo::ROTATE)
               gizmoOperation = ImGuizmo::ROTATE_Z;
            else if (gizmoOperation == ImGuizmo::SCALE)
               gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y);
            break;
         case PredefinedView::Front:
         case PredefinedView::Back:
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
      const bool gizmoLost = m_gizmoDragActive && m_editor.m_multiSel != m_gizmoDragSelection;
      if (gizmoLost)
      {
         ImGuizmo::Enable(false); // Clears ImGuizmo's using state
         ImGuizmo::Enable(true);
      }
      else
      {
         ImGuizmo::Manipulate(camViewLH, (float*)(gizmoProj.m), gizmoOperation, m_gizmoMode, (float*)(transform.m));
         if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
         {
            // Mark all selected parts for undo once per drag (m_editor.m_lastUndoId is reset when the gizmo is released)
            if (!m_gizmoDragActive && m_editor.m_table->m_liveBaseTable == nullptr)
            {
               m_gizmoDragActive = true;
               m_gizmoDragSelection = m_editor.m_multiSel;
               m_editor.m_undo.BeginUndo();
               for (const auto& part : m_editor.m_multiSel)
                  m_editor.m_undo.MarkForUndo(part->GetEditable());
               m_editor.m_undo.EndUndo();
               m_editor.m_lastUndoPart = nullptr;
               m_editor.m_lastUndoId = 0x1000;
            }
            m_editor.SetSelectionTransform(transform);
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
   if (m_editor.m_lastUndoId == 0x1000 && !ImGuizmo::IsUsing())
   {
      m_gizmoDragActive = false;
      m_gizmoDragSelection.clear();
      m_editor.m_lastUndoPart = nullptr;
      m_editor.m_lastUndoId = 0;
   }

   m_editor.m_renderer->SetShadeMode(m_shadeMode);

   // 2D overlays projected through the pane's camera (the parts' 3D selection wireframes are
   // recorded by Render3D into the pane's view)
   {
      LiveRenderContext ctx(m_editor.m_player, overlayDrawList, m_viewType, m_shadeMode, (m_editor.m_table->m_liveBaseTable != nullptr) && m_editor.m_player->IsPlaying());
      const bool clipToPane = SetupOverlay(&ctx, overlayDrawList);

      if (isSelectionTransformValid)
      {
         const ImVec2 pos = ctx.Project(transform.GetOrthoNormalPos());
         overlayDrawList->AddCircleFilled(pos, 3.f * m_editor.m_liveUI.GetDPI(), IM_COL32(255, 255, 255, 255), 16);
      }

      // In desktop backdrop mode, draw the bounds of the backdrop area
      if (m_viewType == ViewMode::DesktopBackdrop)
      {
         const Vertex3Ds bounds[4] = { Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds((float)EDITOR_BG_WIDTH, 0.f, 0.f), Vertex3Ds((float)EDITOR_BG_WIDTH, (float)EDITOR_BG_HEIGHT, 0.f),
            Vertex3Ds(0.f, (float)EDITOR_BG_HEIGHT, 0.f) };
         for (int i = 0; i < 4; i++)
            ctx.DrawLine(bounds[i], bounds[(i + 1) % 4], IM_COL32(255, 255, 255, 64));
      }

      // In drag point edit mode, render the drag points of the edited part's curve (or its center point)
      if (m_editor.m_pointEditPart)
      {
         if (m_editor.m_pointEditCenter)
         {
            const ImVec2 pos = ctx.Project(m_editor.m_pointEditPart->GetEditCenter());
            if (pos.x != FLT_MAX)
            {
               const ImU32 color = m_editor.m_centerSelected ? IM_COL32(255, 128, 0, 255) : IM_COL32(80, 160, 255, 255);
               const float radius = 5.f * m_editor.m_liveUI.GetDPI();
               overlayDrawList->AddCircleFilled(pos, radius, color, 12);
               overlayDrawList->AddCircle(pos, radius + m_editor.m_liveUI.GetDPI(), IM_COL32(0, 0, 0, 255), 12, 1.5f);
            }
         }
         else
         {
            const auto& points = m_editor.m_pointEditPart->GetDragPointCurve()->GetPoints();
            for (size_t i = 0; i < points.size(); i++)
            {
               const ImVec2 pos = ctx.Project(Vertex3Ds(points[i]->GetX(), points[i]->GetY(), m_editor.m_pointEditPart->GetDragPointZ(points[i].get())));
               if (pos.x == FLT_MAX)
                  continue;
               const ImU32 color
                  = m_editor.IsPointSelected(points[i].get()) ? IM_COL32(255, 128, 0, 255) : (points[i]->IsSmooth() ? IM_COL32(80, 160, 255, 255) : IM_COL32(255, 96, 96, 255));
               const float radius = (i == 0 ? 5.f : 4.f) * m_editor.m_liveUI.GetDPI(); // First point is drawn slightly larger to mark the curve start
               overlayDrawList->AddCircleFilled(pos, radius, color, 12);
               overlayDrawList->AddCircle(pos, radius + m_editor.m_liveUI.GetDPI(), IM_COL32(0, 0, 0, 255), 12, 1.5f);
            }
         }
      }

      if (m_editor.m_physOverlay == EditorUI::PhysicOverlay::All || (m_editor.m_physOverlay == EditorUI::PhysicOverlay::Selected && !m_editor.m_multiSel.empty()))
      {
         auto project = [ctx](Vertex3Ds v)
         {
            const ImVec2 pt = ctx.Project(v);
            return Vertex2D(pt.x, pt.y);
         };
         ImGui::PushStyleColor(ImGuiCol_PlotLines, IM_COL32(255, 0, 0, 255)); // We abuse ImGui colors to pass render colors
         ImGui::PushStyleColor(ImGuiCol_PlotHistogram, IM_COL32(255, 0, 0, 64));
         for (auto pho : m_editor.m_player->m_physics->GetHitObjects())
            if (pho != nullptr
               && (m_editor.m_physOverlay == EditorUI::PhysicOverlay::All || (m_editor.m_physOverlay == EditorUI::PhysicOverlay::Selected && m_editor.IsEditableSelected(pho->m_editable))))
               pho->DrawUI(project, overlayDrawList, true);
         ImGui::PopStyleColor(2);
      }
      if (clipToPane)
         overlayDrawList->PopClipRect();
   }

   // Handle uncaught mouse interaction: scene interactions (camera, picking, box select) only
   // apply inside this pane's image (its ImGui item state says if it is hovered or held)
   // The pane owns scene input while hovered/active, or the whole window while flying over it
   const bool viewportInput = m_flyMode || m_hovered || m_active;
   if (viewportInput && m_editor.m_fileOp == nullptr)
   {
      // Zoom in/out with mouse wheel
      if (io.MouseWheel != 0)
      {
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
         if (m_viewType != ViewMode::PreviewCam && (drag.x != 0.f || drag.y != 0.f))
         {
            Matrix3D viewInverse(m_camView);
            viewInverse.Invert();
            vec3 dir = viewInverse.GetOrthoNormalDir();
            const vec3 up = viewInverse.GetOrthoNormalUp();
            const vec3 pos = viewInverse.GetOrthoNormalPos();
            const vec3 right = viewInverse.GetOrthoNormalRight();
            vec3 camTarget = pos - dir * m_camDistance;
            if (io.KeyShift || m_viewType == ViewMode::DesktopBackdrop || m_orbitLock)
            {
               // Pan so that the point under the cursor follows it (the view height at the target depends on the projection;
               // in perspective the raw pixel drag used to be applied as VPU, ignoring the zoom)
               const float halfH = m_perspectiveCam && m_viewType != ViewMode::DesktopBackdrop ? m_camDistance * tanf(0.5f * ANGTORAD(editorCamFovY)) : m_camDistance;
               const float viewH = m_imageRect.Height() > 0.f ? m_imageRect.Height() : io.DisplaySize.y;
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
         if (m_editor.m_partLibrary.IsPlacementArmed())
         {
            // Part library placement mode: drop the previewed part at its ghost position
            m_editor.m_partLibrary.CommitPlacement();
         }
         else if (m_editor.m_addPartType != eItemInvalid)
         {
            // Add part mode: create the pending part at the picked point of the playfield XY plane
            const ItemTypeEnum type = m_editor.m_addPartType;
            m_editor.m_addPartType = eItemInvalid;
            m_editor.CreatePart(type, UnprojectToPlane(m_boxSelectStart, 0.f));
         }
         else if (m_editor.m_pointEditPart != nullptr && m_editor.m_pointEditCenter)
         {
            // Center point edit mode: clicking on the center selects it and starts a pending drag
            if (HitTestEditCenter(m_boxSelectStart))
            {
               if (io.KeyShift)
                  m_editor.m_centerSelected = !m_editor.m_centerSelected;
               else
                  m_editor.m_centerSelected = true;
               m_editor.m_pointDragPending = m_editor.m_centerSelected;
               m_editor.m_pointDragZ = m_editor.m_pointEditPart->GetEditCenter().z;
               m_editor.m_pointDragPos = UnprojectToPlane(m_boxSelectStart, m_editor.m_pointDragZ);
            }
            else
               m_boxSelectActive = true;
         }
         else
         {
            DragPoint* const hitPoint = (m_editor.m_pointEditPart != nullptr) ? HitTestDragPoint(m_boxSelectStart) : nullptr;
            if (hitPoint != nullptr)
            {
               if (io.KeyShift)
                  m_editor.TogglePointSelection(hitPoint);
               else if (!m_editor.IsPointSelected(hitPoint))
               {
                  m_editor.m_pointSel.clear();
                  m_editor.m_pointSel.push_back(hitPoint);
               }
               m_editor.m_pointDragPending = m_editor.IsPointSelected(hitPoint) && !hitPoint->m_uiLocked;
               m_editor.m_pointDragZ = m_editor.m_pointEditPart->GetDragPointZ(hitPoint);
               m_editor.m_pointDragPos = UnprojectToPlane(m_boxSelectStart, m_editor.m_pointDragZ);
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
            if (m_editor.m_pointEditPart)
            {
               if (fabsf(end.x - m_boxSelectStart.x) > 4.f || fabsf(end.y - m_boxSelectStart.y) > 4.f)
                  BoxSelectPoints(m_boxSelectStart, end, io.KeyShift);
               else if (!io.KeyShift)
               {
                  m_editor.m_pointSel.clear();
                  m_editor.m_centerSelected = false;
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
                     const auto it = m_editor.m_editableMap.find(vhoHit.front().m_obj->m_editable);
                     if (it != m_editor.m_editableMap.end())
                     {
                        m_editor.TogglePartSelection(it->second);
                        OutlinerPanel::m_anchor = it->second;
                     }
                  }
               }
               else if (vhoHit.empty())
                  m_editor.ClearSelection();
               else
               {
                  // Click selects the nearest hit part, subsequent clicks cycle to the parts behind it
                  vector<std::shared_ptr<EditorUIPart>> hitParts;
                  for (const HitTestResult& hr : vhoHit)
                  {
                     const auto it = m_editor.m_editableMap.find(hr.m_obj->m_editable);
                     if (it != m_editor.m_editableMap.end() && std::ranges::find(hitParts, it->second) == hitParts.end())
                        hitParts.push_back(it->second);
                  }
                  if (hitParts.empty())
                     m_editor.ClearSelection();
                  else
                  {
                     size_t selectionIndex = 0;
                     const auto selIt = std::ranges::find(hitParts, m_editor.m_selection.GetPart());
                     if (selIt != hitParts.end())
                        selectionIndex = static_cast<size_t>(selIt - hitParts.begin() + 1) % hitParts.size();
                     m_editor.SetSelection(Selection(hitParts[selectionIndex]));
                     OutlinerPanel::m_anchor = hitParts[selectionIndex];
                  }
                  // TODO add debug action to make ball active: m_editor.m_player->m_pactiveballDebug = m_editor.m_pBall;
               }
            }
         }
      }

      // Drag the selected drag points in the table XY plane
      if (m_editor.m_pointEditPart && (m_editor.m_pointDragPending || m_editor.m_pointDragActive))
      {
         if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
         {
            const Vertex2D pos = UnprojectToPlane(ImGui::GetMousePos(), m_editor.m_pointDragZ);
            const Vertex2D delta(pos.x - m_editor.m_pointDragPos.x, pos.y - m_editor.m_pointDragPos.y);
            if (m_editor.m_pointDragPending)
            {
               const ImVec2 dragDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
               if (fabsf(dragDelta.x) > 2.f || fabsf(dragDelta.y) > 2.f)
               {
                  // Drag actually starts: mark the edited part for undo once per drag
                  m_editor.m_pointDragPending = false;
                  m_editor.m_pointDragActive = m_editor.m_pointEditCenter ? m_editor.m_centerSelected : !m_editor.m_pointSel.empty();
                  if (m_editor.m_pointDragActive)
                  {
                     m_editor.m_undo.BeginUndo();
                     m_editor.m_undo.MarkForUndo(m_editor.m_pointEditPart->GetEditable());
                     m_editor.m_undo.EndUndo();
                  }
               }
            }
            if (m_editor.m_pointDragActive && (delta.x != 0.f || delta.y != 0.f))
            {
               if (m_editor.m_pointEditCenter)
               {
                  const Vertex3Ds center = m_editor.m_pointEditPart->GetEditCenter();
                  m_editor.m_pointEditPart->SetEditCenter(Vertex2D(center.x + delta.x, center.y + delta.y));
               }
               else
               {
                  DragPointCurve* const curve = m_editor.m_pointEditPart->GetDragPointCurve();
                  for (DragPoint* point : m_editor.m_pointSel)
                     point->Translate(delta);
                  curve->OnPointsModified();
               }
               m_editor.m_renderer->ReinitRenderable(m_editor.m_pointEditPart->GetEditable()->GetIRenderable());
               m_editor.m_player->m_physics->Update(m_editor.m_pointEditPart->GetEditable());
            }
            m_editor.m_pointDragPos = pos;
         }
         else
         {
            m_editor.m_pointDragPending = false;
            m_editor.m_pointDragActive = false;
         }
      }
   }
   if (m_boxSelectActive && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
      m_boxSelectActive = false; // Canceled: button released outside of the playfield
   if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
   {
      m_editor.m_pointDragPending = false; // Released outside of the viewport (the drag flags would otherwise stay set until the mouse comes back)
      m_editor.m_pointDragActive = false;
   }

   // Viewport keyboard actions: camera views & options, gizmo ops, drag point edit, mode cancels
   // (a hovered blueprint pane owns the keyboard while the mouse is over its canvas)
   if (m_editor.GetTargetViewport() == this && m_editor.m_hoverBp == nullptr && !io.WantCaptureKeyboard && m_editor.m_fileOp == nullptr)
   {
      if (ImGui::IsKeyReleased(ImGuiKey_Escape))
      {
         if (m_editor.m_suppressNextEscape)
            m_editor.m_suppressNextEscape = false; // Consumed by a field the Esc deactivated (e.g. the script editor)
         else if (m_editor.ConsumedAreaEscape())
         {
         } // Consumed while cancelling an area split / join (not an editor Esc)
         else if (m_gizmoOperation != ImGuizmo::OPERATION(0))
            m_gizmoOperation = ImGuizmo::OPERATION(0); // Cancel current operation
         else if (m_boxSelectActive)
            m_boxSelectActive = false; // Cancel current box selection
         else if (m_editor.m_addPartType != eItemInvalid)
            m_editor.m_addPartType = eItemInvalid; // Cancel add part mode
         else if (m_editor.m_partLibrary.IsPlacementArmed())
            m_editor.m_partLibrary.CancelPlacement(); // Cancel part library placement mode
         else if (m_editor.m_pointEditPart)
            m_editor.ExitPointEditMode(true); // Exit drag point edit mode
         else if (m_editor.m_selection.GetType() != Selection::S_NONE)
            m_editor.ClearSelection(); // Cancel current selection
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Tab, false) && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      {
         // Toggle drag point edit mode on the active selected part. For parts exposing an editable
         // center (light bulb, light sequencer animation center), Tab first switches the mode from
         // the drag point curve to the center point before exiting.
         if (m_editor.m_pointEditPart)
         {
            if (!m_editor.m_pointEditCenter && m_editor.m_pointEditPart->HasEditCenter())
            {
               m_editor.m_pointEditCenter = true;
               m_editor.m_centerSelected = true;
               m_editor.m_pointSel.clear();
               m_editor.m_pointDragPending = false;
               m_editor.m_pointDragActive = false;
            }
            else
               m_editor.ExitPointEditMode(true);
         }
         else
            m_editor.EnterPointEditMode();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_A) && io.KeyShift && !io.KeyCtrl && !io.KeyAlt)
      {
         // Add a drag point on the curve segment nearest to the mouse position
         if (m_editor.m_pointEditPart && !m_editor.m_pointEditCenter && !io.WantCaptureMouse)
            m_editor.AddPointOnNearestSegment();
         else if (!m_editor.m_pointEditPart && !m_editor.IsInspectMode() && !m_editor.m_table->IsLocked())
         {
            // Request the part type picker popup at the mouse position (it is opened in the window
            // scope of the viewport pane under the mouse, where the matching BeginPopup is called)
            m_editor.RequestAddPartPopup(ImGui::GetMousePos());
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_F) && !io.KeyCtrl)
      {
         // Fly mode: the pane's view takes the whole window, hiding the editor UI
         m_flyMode = !m_flyMode;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_G))
      {
         // Grab (translate)

         if (io.KeyAlt && !m_editor.m_pointEditPart)
         {
            Matrix3D tmp;
            if (m_editor.GetSelectionTransform(tmp))
               m_editor.SetSelectionTransform(tmp, true, false, false);
         }
         else if (!io.KeyCtrl)
            SetGizmoOperation(ImGuizmo::TRANSLATE);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
         // Ctrl+S (save) is handled by the editor before this; Scale otherwise
         if (!io.KeyCtrl)
         {

            if (io.KeyAlt && !m_editor.m_pointEditPart)
            {
               Matrix3D tmp;
               if (m_editor.GetSelectionTransform(tmp))
                  m_editor.SetSelectionTransform(tmp, false, true, false);
            }
            else
               SetGizmoOperation(ImGuizmo::SCALE);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_R))
      {
         // Rotate

         if (io.KeyAlt && !m_editor.m_pointEditPart)
         {
            Matrix3D tmp;
            if (m_editor.GetSelectionTransform(tmp))
               m_editor.SetSelectionTransform(tmp, false, false, true);
         }
         else if (!io.KeyCtrl)
            SetGizmoOperation(ImGuizmo::ROTATE);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Z) && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt)
      {
         // Wireframe shade mode cycle (Ctrl+Z undo is an editor command)
         switch (m_shadeMode)
         {
         case Renderer::ShadeMode::Default: m_shadeMode = Renderer::ShadeMode::Wireframe; break;
         case Renderer::ShadeMode::Wireframe: m_shadeMode = Renderer::ShadeMode::NoDepthWireframe; break;
         case Renderer::ShadeMode::NoDepthWireframe: m_shadeMode = Renderer::ShadeMode::Default; break;
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad5))
         m_perspectiveCam = !m_perspectiveCam;
      else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal))
      {
         FrameSelection();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDivide))
      {
         m_orbitLock = !m_orbitLock;
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad7))
      {
         SetView(io.KeyCtrl ? PredefinedView::Bottom : PredefinedView::Top);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad1))
      {
         SetView(io.KeyCtrl ? PredefinedView::Back : PredefinedView::Front);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Keypad3))
      {
         SetView(io.KeyCtrl ? PredefinedView::Left : PredefinedView::Right);
      }
   }
}
}
