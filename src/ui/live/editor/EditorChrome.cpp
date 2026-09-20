// license:GPLv3+

#include "core/stdafx.h"
#include "EditorChrome.h"

#include "core/FileLocator.h"
#include "core/VPApp.h"
#include "renderer/Sampler.h"
#include "renderer/Texture.h"
#include "ui/VPXFileFeedback.h"
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
   m_toolBarHeight = showFullUI ? 20.f * m_editor.m_liveUI.GetDPI() : 0.f;

   if (!showFullUI)
      return;

   RenderMenuBar();
   RenderToolbar();
   RenderStatusOverlay();
}

void EditorChrome::RenderMenuBar()
{
   EditorUI &editor = m_editor;
   if (ImGui::BeginMainMenuBar())
   {
      if (!editor.IsInspectMode() && ImGui::BeginMenu("File"))
      {
         if (ImGui::MenuItem("Save", "Ctrl+S"))
         {
            // TODO cursor feedback
            VPXFileFeedback feedback;
            if (SUCCEEDED(editor.m_table->Save(feedback)))
               editor.m_undo.SetCleanPoint(eSaveClean);
         }
         ImGui::Separator();
         if (ImGui::MenuItem("Quit"))
            editor.m_player->SetCloseState(Player::CS_CLOSE_APP);
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
         editor.m_table->QuitPlayer(editor.IsInspectMode() ? Player::CS_STOP_PLAY : Player::CS_CLOSE_APP);
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Close editor");
      m_menuBarHeight = ImGui::GetWindowSize().y;
      ImGui::EndMainMenuBar();
   }
}

void EditorChrome::RenderToolbar()
{
   EditorUI &editor = m_editor;
   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menuBarHeight));
   ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, m_toolBarHeight));
   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
   ImGui::Begin("TOOLBAR", nullptr, window_flags);
   ImGui::PopStyleVar();
   if (editor.IsInspectMode())
   {
      if (ImGui::Button(editor.m_player->IsPlaying() ? ICON_FK_PAUSE : ICON_FK_PLAY))
         editor.m_player->SetPlayState(!editor.m_player->IsPlaying());
      ImGui::SameLine();
      ImGui::BeginDisabled(editor.m_player->IsPlaying());
      if (ImGui::Button(ICON_FK_STEP_FORWARD))
         editor.m_player->m_step = true;
      ImGui::EndDisabled();
   }
   else
   {
      const float iconSize = ImGui::GetContentRegionAvail().y;

      // Add part buttons (same parts and icons as the WinUI toolbar)
      if (m_addPartButtons.empty())
      {
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
            m_addPartButtons.push_back({ def.type, def.name, tex ? std::make_shared<Sampler>(editor.m_renderer->m_renderDevice, def.name, tex, false) : nullptr });
         }
      }
      ImGui::BeginDisabled(editor.m_pointEditPart != nullptr || editor.m_table->IsLocked());
      for (const auto &button : m_addPartButtons)
      {
         if (button.icon == nullptr)
            continue;
         ImGui::SameLine();
         const bool active = (editor.m_addPartType == button.type);
         if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
         if (ImGui::ImageButton(button.name, button.icon, ImVec2(iconSize, iconSize)))
            editor.m_addPartType = active ? eItemInvalid : button.type;
         if (active)
            ImGui::PopStyleColor();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Add %s", button.name);
      }
      ImGui::EndDisabled();

      if (editor.m_selection.GetType() == Selection::S_EDITABLE)
      {
         ImGui::SameLine();
         ImGui::Separator();
         ImGui::SameLine();
         const float popupIconSize = iconSize + 2.f * ImGui::GetStyle().FramePadding.y;
         if (ImGui::Button(ICON_FK_TRASH_O, ImVec2(popupIconSize, popupIconSize)))
            editor.DeleteSelection();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Delete selection");
      }

      // Part type picker popup (Shift+A in standard mode): picking a type arms add part mode,
      // clicking outside of the popup dismisses it without arming add part mode
      if (m_openAddPartPopup)
      {
         m_openAddPartPopup = false;
         ImGui::OpenPopup("Add Part");
      }
      ImGui::SetNextWindowPos(m_addPartPopupPos, ImGuiCond_Appearing);
      if (ImGui::BeginPopup("Add Part"))
      {
         int column = 0;
         for (const auto &button : m_addPartButtons)
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
      if (ImGui::RadioButton("VP Units", editor.m_units == EditorUI::Units::VPX))
         editor.m_units = EditorUI::Units::VPX;
      if (ImGui::RadioButton("Metric", editor.m_units == EditorUI::Units::Metric))
         editor.m_units = EditorUI::Units::Metric;
      if (ImGui::RadioButton("Imperial", editor.m_units == EditorUI::Units::Imperial))
         editor.m_units = EditorUI::Units::Imperial;
      ImGui::EndPopup();
   }
   ImGui::SameLine();
   if (ImGui::Button(ICON_FK_STICKY_NOTE)) // Overlay option menu
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
   if (ImGui::Button(ICON_FK_FILTER)) // Selection filter
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
   ImGui::End();
}

void EditorChrome::RenderStatusOverlay()
{
   EditorUI &editor = m_editor;
   const ImGuiIO &io = ImGui::GetIO();

   // Overlay Info Text
   ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - OutlinerPanel::PaneWidth * editor.m_liveUI.GetDPI(),
      io.DisplaySize.y - m_toolBarHeight - m_menuBarHeight - 5.f * editor.m_liveUI.GetDPI())); // Fixed outliner width (to be adjusted when moving ImGui to the docking branch)
   ImGui::SetNextWindowPos(ImVec2(OutlinerPanel::PaneWidth * editor.m_liveUI.GetDPI(), m_toolBarHeight + m_menuBarHeight + 5.f * editor.m_liveUI.GetDPI()));
   ImGui::Begin("text overlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
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
   switch (editor.m_gizmoOperation)
   {
   case ImGuizmo::OPERATION(0): ImGui::TextUnformatted("Select"); break;
   case ImGuizmo::TRANSLATE: ImGui::TextUnformatted("Grab"); break;
   case ImGuizmo::ROTATE: ImGui::TextUnformatted("Rotate"); break;
   case ImGuizmo::SCALE: ImGui::TextUnformatted("Scale"); break;
   default: break;
   }
   if (editor.m_pointEditPart)
      ImGui::TextUnformatted("Drag Point Edit (Tab to exit)");
   ImGui::End();
}

}
