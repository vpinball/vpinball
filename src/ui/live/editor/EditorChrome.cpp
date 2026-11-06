// license:GPLv3+

#include "core/stdafx.h"
#include "EditorChrome.h"
#include "ViewportPane.h" // m_editVp->m_imageRect (status overlay anchor)

#include "core/player.h"
#include "ui/EditorClipboard.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"

namespace VPX::EditorUI
{

void EditorChrome::Render(bool showFullUI)
{
   // Gives some transparency when positioning camera to better view camera view bounds
   // TODO for some reasons, this breaks the modal background behavior
   //SetupImGuiStyle(m_editor.m_selection.GetType() == Selection::S_CAMERA ? 0.3f : 1.0f);

   m_menuBarHeight = 0.0f;

   if (!showFullUI)
      return;

   RenderMenuBar();
   RenderStatusOverlay();
}

void EditorChrome::RenderMenuBar()
{
   EditorUI &editor = m_editor;
   if (ImGui::BeginMainMenuBar())
   {
      if (!editor.IsInspectMode() && ImGui::BeginMenu("File"))
      {
         if (ImGui::BeginMenu("New"))
         {
            if (ImGui::MenuItem("New Table", "Ctrl+N"))
               editor.NewTable(EditorUI::NewTableTemplate::Blank);
            if (ImGui::MenuItem("Completely Blank Table"))
               editor.NewTable(EditorUI::NewTableTemplate::Stripped);
            if (ImGui::MenuItem("Full Example Table"))
               editor.NewTable(EditorUI::NewTableTemplate::Example);
            if (ImGui::MenuItem("Light Sequence Demo Table"))
               editor.NewTable(EditorUI::NewTableTemplate::LightSeq);
            ImGui::EndMenu();
         }
         if (ImGui::MenuItem("Load..."))
            editor.LoadTable();
         if (ImGui::MenuItem("Load Pack Folder..."))
            editor.LoadTableFolder();
         if (ImGui::MenuItem("Save", "Ctrl+S"))
            editor.SaveTable();
         if (ImGui::MenuItem("Save As..."))
            editor.SaveTableAs();
         if (ImGui::MenuItem("Save As Pack Folder..."))
            editor.SaveTableAsPackFolder();
         ImGui::Separator();
         if (ImGui::MenuItem("Play", "F5", false, !editor.m_table->IsLocked()))
            editor.PlayTest();
         ImGui::Separator();
         if (ImGui::MenuItem("Quit"))
            editor.RequestClose(Player::CS_CLOSE_APP);
         ImGui::EndMenu();
      }
      if (!editor.IsInspectMode() && ImGui::BeginMenu("Edit"))
      {
         // Selection actions of the Esc, Ctrl+C / Ctrl+V and Del shortcuts (the add part picker itself opens in the viewport pane)
         ImGui::BeginDisabled(editor.m_table->IsLocked());
         const bool canCopy
            = editor.m_pointEditPart ? (editor.m_pointEditCenter ? editor.m_centerSelected : editor.m_pointSel.size() == 1) : (editor.m_selection.GetType() == Selection::S_EDITABLE);
         if (ImGui::MenuItem("Copy", "Ctrl+C", false, canCopy))
            editor.CopySelection();
         const bool canPaste = editor.m_pointEditPart
            ? (editor.m_pointEditCenter ? editor.m_centerSelected && VPX::EditorClipboard::HasPoint() : editor.m_pointSel.size() == 1 && VPX::EditorClipboard::HasPoint())
            : VPX::EditorClipboard::HasParts();
         if (ImGui::MenuItem("Paste", "Ctrl+V", false, canPaste))
            editor.PasteSelection(editor.GetViewportContentCenter());
         if (ImGui::MenuItem("Delete", "Del", false, editor.m_selection.GetType() == Selection::S_EDITABLE))
            editor.DeleteSelection();
         if (ImGui::MenuItem("Add Part...", "Shift+A", false, editor.m_pointEditPart == nullptr))
            editor.RequestAddPartPopup(editor.GetViewportContentCenter());
         ImGui::EndDisabled();
         ImGui::Separator();
         if (ImGui::MenuItem("Import Parts (part library)...", nullptr, false, !editor.m_table->IsLocked()))
            editor.OpenPartLibrary();
         if (ImGui::MenuItem("Export Selection to a VPZ Pack...", nullptr, false, editor.CanExportPartSelection()))
            editor.ExportPartSelection();
         ImGui::Separator();
         if (ImGui::BeginMenu("Units"))
         {
            if (ImGui::RadioButton("VP Units", editor.m_units == EditorUI::Units::VPX))
               editor.m_units = EditorUI::Units::VPX;
            if (ImGui::RadioButton("Metric", editor.m_units == EditorUI::Units::Metric))
               editor.m_units = EditorUI::Units::Metric;
            if (ImGui::RadioButton("Imperial", editor.m_units == EditorUI::Units::Imperial))
               editor.m_units = EditorUI::Units::Imperial;
            ImGui::EndMenu();
         }
         ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View"))
      {
         // View actions apply to the edit viewport (the pane under the mouse): its camera state is bound to the editor fields
         const auto viewItem = [&editor](const char *label, const char *keys, const EditorUI::PredefinedView view)
         {
            if (ImGui::MenuItem(label, keys, editor.m_camMode == ViewMode::EditorCam && editor.m_predefinedView == view))
               editor.SetView(view);
         };
         viewItem("Top", "Numpad 7", EditorUI::PredefinedView::Top);
         viewItem("Bottom", "Ctrl+Numpad 7", EditorUI::PredefinedView::Bottom);
         viewItem("Front", "Numpad 1", EditorUI::PredefinedView::Front);
         viewItem("Back", "Ctrl+Numpad 1", EditorUI::PredefinedView::Back);
         viewItem("Left", "Numpad 3", EditorUI::PredefinedView::Left);
         viewItem("Right", "Ctrl+Numpad 3", EditorUI::PredefinedView::Right);
         ImGui::Separator();
         if (ImGui::MenuItem("Perspective / Orthographic", "Numpad 5"))
         {
            editor.m_camMode = ViewMode::EditorCam;
            editor.m_perspectiveCam = !editor.m_perspectiveCam;
         }
         if (ImGui::MenuItem("Cycle Camera (Preview / Editor / Backdrop)", "Numpad 0"))
            editor.CycleCamera();
         if (ImGui::MenuItem("Lock Orbit", "Numpad /", editor.m_orbitLock))
            editor.m_orbitLock = !editor.m_orbitLock;
         ImGui::Separator();
         if (ImGui::BeginMenu("Shading"))
         {
            const auto shadeItem = [&editor](const char *label, const Renderer::ShadeMode mode)
            {
               if (ImGui::MenuItem(label, nullptr, editor.m_shadeMode == mode))
                  editor.m_shadeMode = mode;
            };
            shadeItem("Solid", Renderer::ShadeMode::Default);
            shadeItem("Wireframe", Renderer::ShadeMode::Wireframe);
            shadeItem("Wireframe (no depth)", Renderer::ShadeMode::NoDepthWireframe);
            ImGui::EndMenu();
         }
         ImGui::Separator();
         if (ImGui::MenuItem("Frame Selected", "Numpad .", false, editor.CanFrameSelection()))
            editor.FrameSelection();
         if (ImGui::MenuItem("Toggle Maximize Area", "Ctrl+Space"))
            editor.ToggleMaximizeArea();
         ImGui::Separator();
         if (ImGui::MenuItem("Table Script", nullptr, editor.m_scriptPanel.IsVisible(), !editor.m_table->IsLocked()))
            editor.m_scriptPanel.Show();
         ImGui::Separator();
         ImGui::BeginDisabled(editor.IsInspectMode() || editor.m_table->IsLocked());
         if (ImGui::MenuItem("Layout 1", nullptr, editor.m_activeLayoutIndex == 0))
            editor.SetAreaLayoutIndex(0);
         if (ImGui::MenuItem("Layout 2", nullptr, editor.m_activeLayoutIndex == 1))
            editor.SetAreaLayoutIndex(1);
         ImGui::EndDisabled();
         ImGui::EndMenu();
      }
      if (editor.IsInspectMode() && !editor.m_table->IsLocked() && ImGui::BeginMenu("Debug"))
      {
         if (ImGui::MenuItem("Open debugger"))
            editor.m_player->m_showDebugger = true;
         if (ImGui::MenuItem("Renderer Inspection"))
            editor.m_inspectionModal.Show();
         if (ImGui::MenuItem(editor.m_player->IsPlaying() ? "Pause" : "Play"))
            editor.m_player->SetPlayState(!editor.m_player->IsPlaying());
         if (ImGui::MenuItem("Step", nullptr, false, !editor.m_player->IsPlaying()))
            editor.m_player->m_step = true;
         ImGui::EndMenu();
      }
      else if (!editor.IsInspectMode() && !editor.m_table->IsLocked() && ImGui::BeginMenu("Debug"))
      {
         if (ImGui::MenuItem("Renderer Inspection"))
            editor.m_inspectionModal.Show();
         ImGui::EndMenu();
      }
      float buttonWidth = 0.f;
      if (editor.IsInspectMode())
         buttonWidth += ImGui::CalcTextSize(ICON_FK_REPLY, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      buttonWidth += ImGui::CalcTextSize(ICON_FK_WINDOW_CLOSE, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      ImVec2 padding = ImGui::GetCursorScreenPos();
      padding.x += ImGui::GetContentRegionAvail().x - buttonWidth;
      ImGui::SetCursorScreenPos(padding);
      if (editor.IsInspectMode())
      {
         if (ImGui::Button(ICON_FK_REPLY)) // ICON_FK_STOP
            editor.Close();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Get back to player");
      }
      if (ImGui::Button(ICON_FK_WINDOW_CLOSE))
         editor.RequestClose(editor.IsInspectMode() ? Player::CS_STOP_PLAY : Player::CS_CLOSE_APP);
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Close editor");
      m_menuBarHeight = ImGui::GetWindowSize().y;
      ImGui::EndMainMenuBar();
   }
}

void EditorChrome::RenderStatusOverlay()
{
   EditorUI &editor = m_editor;
   if (!editor.m_viewportVisible)
      return; // No viewport area is shown (a panel is maximized, or none exists)

   // Overlay Info Text: top left of the edit viewport's render area, below its header bar
   // (the pane's image rect of the last frame; the leaf rect is the fallback until it is set)
   const float dpi = editor.m_liveUI.GetDPI();
   const AreaRect rect = editor.m_editVp != nullptr && editor.m_editVp->m_imageRect.y1 > editor.m_editVp->m_imageRect.y0 ? editor.m_editVp->m_imageRect : editor.m_viewportRect;
   const ImVec2 overlayPos(rect.x0 + 5.f * dpi, rect.y0 + 5.f * dpi);

   ImGui::SetNextWindowSize(ImVec2(max(1.f, rect.x1 - overlayPos.x), max(1.f, rect.y1 - overlayPos.y)));
   ImGui::SetNextWindowPos(overlayPos);
   ImGui::Begin("text overlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
   const float labelAlign = ImGui::GetCursorPosX() + ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.x;
   ImGui::SetCursorPos(ImVec2(labelAlign, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y));
   switch (editor.m_camMode)
   {
   case ViewMode::PreviewCam: ImGui::TextUnformatted("Preview Camera"); break;
   case ViewMode::EditorCam:
   {
      string text;
      switch (editor.m_predefinedView)
      {
      case EditorUI::PredefinedView::None: text = "User"sv; break;
      case EditorUI::PredefinedView::Left: text = "Left"sv; break;
      case EditorUI::PredefinedView::Right: text = "Right"sv; break;
      case EditorUI::PredefinedView::Top: text = "Top"sv; break;
      case EditorUI::PredefinedView::Bottom: text = "Bottom"sv; break;
      case EditorUI::PredefinedView::Front: text = "Front"sv; break;
      case EditorUI::PredefinedView::Back: text = "Back"sv; break;
      }
      ImGui::TextUnformatted((text + (editor.m_perspectiveCam ? " Perspective" : " Orthographic")).c_str());
      break;
   }
   case ViewMode::DesktopBackdrop: ImGui::TextUnformatted("Desktop Backdrop"); break;
   }
   ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + ImGui::GetStyle().FramePadding.x, ImGui::GetCursorPosY() + ImGui::GetStyle().FramePadding.y));
   const char *gizmoIcon = nullptr;
   const char *gizmoLabel = nullptr;
   if (editor.m_gizmoOperation == ImGuizmo::OPERATION(0))
   {
      gizmoIcon = ICON_FK_MOUSE_POINTER;
      gizmoLabel = "Select";
   }
   else if (editor.m_gizmoOperation == ImGuizmo::TRANSLATE)
   {
      gizmoIcon = ICON_FK_ARROWS;
      gizmoLabel = "Grab";
   }
   else if (editor.m_gizmoOperation == ImGuizmo::SCALE)
   {
      gizmoIcon = ICON_FK_EXPAND;
      gizmoLabel = "Scale";
   }
   else if (editor.m_gizmoOperation == ImGuizmo::ROTATE)
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
   if (editor.m_pointEditPart)
      ImGui::TextUnformatted(editor.m_pointEditCenter ? "Center Edit (Tab to exit)" : "Drag Point Edit (Tab to exit)");
   ImGui::End();
}

}
