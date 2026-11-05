// license:GPLv3+

#include "core/stdafx.h"
#include "ViewportPane.h"

#include "core/player.h"
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
   editor.RenderAreaTypeButton(leafId);
   ImGui::SameLine();

   // Camera mode of this pane (the edit viewport's state lives in the editor fields)
   const bool bound = editor.m_editVp == this;
   ViewMode& camMode = bound ? editor.m_camMode : m_camMode;
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
   bool& perspective = bound ? editor.m_perspectiveCam : m_perspectiveCam;
   if (ImGui::Button(perspective ? "P##Persp" : "O##Persp"))
      perspective = !perspective;
   if (ImGui::IsItemHovered())
      ImGui::SetTooltip(perspective ? "Perspective camera" : "Orthographic camera");
   ImGui::SameLine();
   Renderer::ShadeMode& shade = bound ? editor.m_shadeMode : m_shadeMode;
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
   ImGui::SameLine();
   ImGui::AlignTextToFramePadding();
   ImGui::TextDisabled("3D View");
   ImGui::Separator();

   // The view renders its scene into this pane's normalized area of the shared output
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
