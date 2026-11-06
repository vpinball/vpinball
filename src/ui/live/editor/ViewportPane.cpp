// license:GPLv3+

#include "core/stdafx.h"
#include "ViewportPane.h"

#include "core/player.h"
#include "EditorTheme.h"
#include "fonts/IconsForkAwesome.h"
#include "renderer/Renderer.h"
#include "renderer/RenderView.h"
#include "renderer/Window.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"

namespace VPX::EditorUI
{

ViewportPane::ViewportPane(EditorUI& editor)
   : EditorAreaView(editor)
{
}

ViewportPane::~ViewportPane() { OnUnhosted(); }

void ViewportPane::OnUnhosted()
{
   if (m_editor.m_editVp == this)
   {
      CaptureCamera(); // Keep the editor's camera fields in sync before unbinding
      m_editor.m_editVp = nullptr;
      m_editor.m_captureVp = nullptr;
   }
   if (m_editor.m_hoverVp == this)
      m_editor.m_hoverVp = nullptr;
   if (m_view != nullptr)
   {
      m_editor.m_renderer->ReleaseEditorView(m_view);
      m_view = nullptr;
   }
}

void ViewportPane::CaptureCamera()
{
   m_camMode = m_editor.m_camMode;
   m_perspectiveCam = m_editor.m_perspectiveCam;
   m_shadeMode = m_editor.m_shadeMode;
   m_predefinedView = static_cast<int>(m_editor.m_predefinedView);
   m_orbitLock = m_editor.m_orbitLock;
   m_camView = m_editor.m_camView;
   m_camProj = m_editor.m_camProj;
   m_camDistance = m_editor.m_camDistance;
   if (m_imageRect.Height() > 0.f)
      m_camAspect = m_imageRect.Width() / m_imageRect.Height();
   m_cameraValid = true;
}

void ViewportPane::ReleaseCamera()
{
   m_editor.m_camMode = m_camMode;
   m_editor.m_perspectiveCam = m_perspectiveCam;
   m_editor.m_shadeMode = m_shadeMode;
   m_editor.m_predefinedView = static_cast<EditorUI::PredefinedView>(m_predefinedView);
   m_editor.m_orbitLock = m_orbitLock;
   m_editor.m_camView = m_camView;
   m_editor.m_camProj = m_camProj;
   m_editor.m_camDistance = m_camDistance;
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
   const float dpi = editor.m_liveUI.GetDPI();

   // Camera state of this pane (the edit viewport's state lives in the editor fields)
   const bool bound = editor.m_editVp == this;
   ViewMode& camMode = bound ? editor.m_camMode : m_camMode;
   bool& perspective = bound ? editor.m_perspectiveCam : m_perspectiveCam;
   Renderer::ShadeMode& shade = bound ? editor.m_shadeMode : m_shadeMode;
   bool& orbitLock = bound ? editor.m_orbitLock : m_orbitLock;

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
   ImGui::SameLine();
   const char* modeName = camMode == ViewMode::PreviewCam ? "Preview" : camMode == ViewMode::DesktopBackdrop ? "Backdrop" : "Camera";
   if (ImGui::BeginCombo("##CamMode", modeName, ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_HeightSmall))
   {
      if (ImGui::MenuItem("Camera", nullptr, camMode == ViewMode::EditorCam))
         camMode = ViewMode::EditorCam;
      if (ImGui::MenuItem("Backdrop", nullptr, camMode == ViewMode::DesktopBackdrop))
         camMode = ViewMode::DesktopBackdrop;
      if (ImGui::MenuItem("Preview", nullptr, camMode == ViewMode::PreviewCam))
         camMode = ViewMode::PreviewCam;
      ImGui::EndCombo();
   }
   if (ImGui::IsItemHovered())
      ImGui::SetTooltip("View mode of this area");
   ImGui::SameLine();
   if (ImGui::Button(perspective ? "P##Persp" : "O##Persp"))
      perspective = !perspective;
   if (ImGui::IsItemHovered())
      ImGui::SetTooltip(perspective ? "Perspective camera" : "Orthographic camera");
   ImGui::SameLine();
   const char* shadeName = shade == Renderer::ShadeMode::Wireframe ? "Wire" : shade == Renderer::ShadeMode::NoDepthWireframe ? "WireN" : "Solid";
   if (ImGui::BeginCombo("##Shade", shadeName, ImGuiComboFlags_NoArrowButton))
   {
      if (ImGui::MenuItem("Solid", nullptr, shade == Renderer::ShadeMode::Default))
         shade = Renderer::ShadeMode::Default;
      if (ImGui::MenuItem("Wireframe", nullptr, shade == Renderer::ShadeMode::Wireframe))
         shade = Renderer::ShadeMode::Wireframe;
      if (ImGui::MenuItem("Wireframe (no depth)", nullptr, shade == Renderer::ShadeMode::NoDepthWireframe))
         shade = Renderer::ShadeMode::NoDepthWireframe;
      ImGui::EndCombo();
   }
   if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Shading of this area");

   if (!editor.IsInspectMode())
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
         const bool active = (editor.m_gizmoOperation == mode.operation);
         if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
         if (ImGui::Button(mode.icon))
            editor.SetGizmoOperation(mode.operation);
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
   m_view->SetOutputArea(vec4(m_imageRect.x0 / disp.x, m_imageRect.y0 / disp.y, m_imageRect.x1 / disp.x, m_imageRect.y1 / disp.y));
   ImGui::InvisibleButton("##view", avail);
   m_hovered = ImGui::IsItemHovered();
   m_active = ImGui::IsItemActive();
   ImGui::PopID();
   ImGui::End();
   ImGui::PopStyleVar(3);

   // Per frame view setup: editor flags and shading; the camera is pushed by the editor's
   // update for the edit viewport, and restored from the stored state for the other panes
   m_view->m_editorView = true;
   m_view->m_editorBackdropEdit = camMode == ViewMode::DesktopBackdrop;
   m_view->m_shadeMode = bound ? editor.m_shadeMode : m_shadeMode;
   if (!bound)
   {
      if (camMode == ViewMode::PreviewCam || !m_cameraValid)
         m_view->InitLayout();
      else
      {
         // Re-fit the stored projection to the pane's current aspect ratio (a splitter drag may have resized it)
         const float aspect = m_imageRect.Height() > 0.f ? m_imageRect.Width() / m_imageRect.Height() : m_camAspect;
         const Matrix3D RH2LH = Matrix3D::MatrixScale(1.f, 1.f, -1.f);
         const Matrix3D YAxis = Matrix3D::MatrixScale(1.f, -1.f, 1.f);
         m_view->SetViewProj(RH2LH * m_camView * YAxis, YAxis * m_camProj * Matrix3D::MatrixScale(m_camAspect / aspect, 1.f, 1.f));
      }
   }
}
}
