// license:GPLv3+

#include "core/stdafx.h"
#include "EditorChrome.h"
#include "ViewportPane.h" // GetTargetViewport()->m_imageRect (status overlay anchor)

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
         // View actions apply to the edit viewport's camera (the bound pane's own camera definition)
         ViewportPane *const vp = editor.GetTargetViewport();
         const auto viewItem = [vp](const char *label, const char *keys, const ViewportPane::PredefinedView view)
         {
            if (ImGui::MenuItem(label, keys, vp != nullptr && vp->m_viewType == ViewMode::EditorCam && vp->m_predefinedView == view, vp != nullptr))
               vp->SetView(view);
         };
         viewItem("Top", "Numpad 7", ViewportPane::PredefinedView::Top);
         viewItem("Bottom", "Ctrl+Numpad 7", ViewportPane::PredefinedView::Bottom);
         viewItem("Front", "Numpad 1", ViewportPane::PredefinedView::Front);
         viewItem("Back", "Ctrl+Numpad 1", ViewportPane::PredefinedView::Back);
         viewItem("Left", "Numpad 3", ViewportPane::PredefinedView::Left);
         viewItem("Right", "Ctrl+Numpad 3", ViewportPane::PredefinedView::Right);
         ImGui::Separator();
         if (ImGui::MenuItem("Perspective / Orthographic", "Numpad 5", false, vp != nullptr))
         {
            vp->m_perspectiveCam = !vp->m_perspectiveCam;
         }
         if (ImGui::MenuItem("Lock Orbit", "Numpad /", vp != nullptr && vp->m_orbitLock, vp != nullptr))
            vp->m_orbitLock = !vp->m_orbitLock;
         ImGui::Separator();
         if (ImGui::BeginMenu("Shading"))
         {
            const auto shadeItem = [vp](const char *label, const Renderer::ShadeMode mode)
            {
               if (ImGui::MenuItem(label, nullptr, vp != nullptr && vp->m_shadeMode == mode) && vp != nullptr)
                  vp->m_shadeMode = mode;
            };
            shadeItem("Solid", Renderer::ShadeMode::Default);
            shadeItem("Wireframe", Renderer::ShadeMode::Wireframe);
            shadeItem("Wireframe (no depth)", Renderer::ShadeMode::NoDepthWireframe);
            ImGui::EndMenu();
         }
         ImGui::Separator();
         if (ImGui::MenuItem("Frame Selected", "Numpad .", false, vp != nullptr && editor.CanFrameSelection()))
            vp->FrameSelection();
         if (ImGui::MenuItem("Toggle Maximize Area", "Ctrl+Space"))
            editor.ToggleMaximizeArea();
         ImGui::Separator();
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

      ImGui::Separator();

      // Layout selection as buttons after the menus (Blender workspaces)
      ImGui::BeginDisabled(editor.m_table->IsLocked());
      static const char *const layoutNames[] = { "Layout", "Library", "Scripting", "Backdrop" };
      for (int i = 0; i < 4; ++i)
      {
         const char *const label = layoutNames[i];
         const bool active = (editor.m_activeLayoutIndex == i);
         if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
         if (ImGui::SmallButton(label))
            editor.SetAreaLayoutIndex(i);
         if (active)
            ImGui::PopStyleColor();
         ImGui::SameLine();
      }
      ImGui::EndDisabled();
      float buttonWidth = 0.f;
      if (editor.IsInspectMode())
         buttonWidth += ImGui::CalcTextSize(ICON_FK_REPLY, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      buttonWidth += ImGui::CalcTextSize(ICON_FK_WINDOW_CLOSE, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      // Play button, centered in the free space left between the layout buttons and the close buttons
      const float playWidth = ImGui::CalcTextSize(ICON_FK_PLAY, nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
      const float freeWidth = ImGui::GetContentRegionAvail().x - buttonWidth;
      if (freeWidth > playWidth)
      {
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (freeWidth - playWidth) * 0.5f);
         ImGui::BeginDisabled(editor.m_table->IsLocked());
         if (ImGui::Button(ICON_FK_PLAY))
            editor.PlayTest();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Play test (F5)");
         ImGui::EndDisabled();
         ImGui::SameLine();
      }
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

}
