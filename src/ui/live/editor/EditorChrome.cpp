// license:GPLv3+

#include "core/stdafx.h"
#include "EditorChrome.h"
#include "BlueprintPane.h"
#include "EditorTheme.h"
#include "ops/SetOriginOp.h"
#include "ui/live/IconsLucide.h"
#include "ViewportPane.h" // GetTargetViewport() (View menu camera actions, status bar hints)

#include "core/player.h"
#include "ui/EditorClipboard.h"
#include "ui/live/EditorUI.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"
#include "utils/bulb.h"

#include "parts/light.h" // Light States submenu (light state update)

namespace VPX::EditorUI
{

namespace
{
   constexpr float StatusBarHeight = 28.f; // Bottom status bar, in pixels at 100% DPI
}

// Chevron pointing down (open) or right (closed), drawn with lines
void EditorChrome::DrawChevron(ImDrawList *drawList, const ImVec2 &c, const ChevronDir dir, const float size, const ImU32 color)
{
   const float thickness = max(1.f, ImGui::GetFontSize() * 0.09f);
   const float h = size * 0.5f;
   if (dir == ChevronDir::Down)
   {
      drawList->AddLine(ImVec2(c.x - size, c.y - h), ImVec2(c.x, c.y + h), color, thickness);
      drawList->AddLine(ImVec2(c.x, c.y + h), ImVec2(c.x + size, c.y - h), color, thickness);
   }
   else
   {
      drawList->AddLine(ImVec2(c.x - h, c.y - size), ImVec2(c.x + h, c.y), color, thickness);
      drawList->AddLine(ImVec2(c.x + h, c.y), ImVec2(c.x - h, c.y + size), color, thickness);
   }
}

void EditorChrome::Render(bool showFullUI)
{
   // Gives some transparency when positioning camera to better view camera view bounds
   // TODO for some reasons, this breaks the modal background behavior
   //SetupImGuiStyle(m_editor.m_selection.GetType() == Selection::S_CAMERA ? 0.3f : 1.0f);

   m_menuBarHeight = 0.0f;
   m_statusBarHeight = showFullUI ? StatusBarHeight * m_editor.m_liveUI.GetDPI() : 0.f;

   if (!showFullUI)
      return;

   RenderMenuBar();
   RenderPreferences();
}

void EditorChrome::RenderMenuBar()
{
   EditorUI &editor = m_editor;
   // Grow the bar beyond the line height so the icon glyphs (whose ink overshoots it) fit with
   // a margin, then drop the item row so labels and icons sit vertically centered in the bar
   const ImVec4 xInk = TextInkBounds(ICON_LC_X);
   const float barExtraY = ImMax(0.f, (xInk.w - xInk.y - ImGui::GetFontSize()) * 0.5f) + ImGui::GetStyle().FramePadding.y;
   ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, ImGui::GetStyle().FramePadding.y + barExtraY);
   const bool barOpen = ImGui::BeginMainMenuBar();
   ImGui::PopStyleVar();
   if (barOpen)
   {
      ImGui::SetCursorPosY(ImGui::GetCursorPosY() + barExtraY);
      // Menu rows are only the text line height, and the icon glyphs slightly overshoot it: give the
      // rows the extra room the icon ink needs (the pushed spacing applies to the spawned popup windows too)
      const ImVec4 iconInk = TextInkBounds(ICON_LC_SAVE);
      ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, ImGui::GetStyle().ItemSpacing.y + ImMax(0.f, iconInk.w - iconInk.y - ImGui::GetFontSize()) + 2.f * editor.m_liveUI.GetDPI());
      if (!editor.IsInspectMode() && ImGui::BeginMenu("File"))
      {
         if (ImGui::BeginMenu(ICON_LC_FILE_PLUS " New"))
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
         if (ImGui::MenuItem(ICON_LC_FOLDER_OPEN " Open...", "Ctrl+O"))
            editor.LoadTable();
         if (ImGui::MenuItem(ICON_LC_FOLDER " Open Pack Folder...", "Ctrl+Shift+O"))
            editor.LoadTableFolder();
         const auto &recentTables = editor.GetRecentTables();
         if (ImGui::BeginMenu(ICON_LC_TIMER " Open Recent...", !recentTables.empty()))
         {
            for (size_t i = 0; i < recentTables.size(); i++)
            {
               ImGui::PushID(static_cast<int>(i));
               if (ImGui::MenuItem(PathToUTF8(recentTables[i]).c_str()))
                  editor.LoadTable(recentTables[i]);
               ImGui::PopID();
            }
            ImGui::EndMenu();
         }
         if (ImGui::MenuItem(ICON_LC_SAVE " Save", "Ctrl+S"))
            editor.SaveTable();
         if (ImGui::MenuItem(ICON_LC_DISC " Save As..."))
            editor.SaveTableAs();
         if (ImGui::MenuItem(ICON_LC_FOLDER " Save As Pack Folder..."))
            editor.SaveTableAsPackFolder();
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_UPLOAD " Export Selection to a VPZ Pack...", nullptr, false, editor.CanExportPartSelection()))
            editor.ExportPartSelection();
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_PLAY " Play", "F5", false, !editor.m_table->IsLocked()))
            editor.PlayTest();
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_X " Quit"))
            editor.RequestClose(Player::CS_CLOSE_APP);
         ImGui::EndMenu();
      }
      if (!editor.IsInspectMode() && ImGui::BeginMenu("Edit"))
      {
         // Blender style: the menu names the step ("Undo Move: Part1"), cut at 40 characters
         const auto stepTitle = [](const char *verb, const string &step)
         {
            size_t n = 37; // Cut on a code point boundary
            if (step.size() > 40)
               while (n > 0 && (static_cast<unsigned char>(step[n]) & 0xC0) == 0x80)
                  --n;
            return step.empty() ? string(verb) : string(verb) + ' ' + (step.size() > 40 ? step.substr(0, n) + "..." : step);
         };
         if (ImGui::MenuItem((ICON_LC_UNDO_2 " " + stepTitle("Undo", editor.CanUndo() && editor.m_undo.UndoCount() > 0 ? editor.m_undo.UndoLabel(0) : string()) + "###EditUndo").c_str(),
                "Ctrl+Z", false, editor.CanUndo()))
            editor.UndoSteps(1);
         if (ImGui::MenuItem(
                (ICON_LC_SKIP_FORWARD " " + stepTitle("Redo", editor.CanRedo() && editor.m_undo.RedoCount() > 0 ? editor.m_undo.RedoLabel(0) : string()) + "###EditRedo").c_str(),
                "Ctrl+Shift+Z", false, editor.CanRedo()))
            editor.RedoSteps(1);
         const bool canHistory = editor.m_table->m_liveBaseTable == nullptr && (editor.m_undo.HasUndo() || editor.m_undo.HasRedo());
         if (ImGui::BeginMenu(ICON_LC_LIST " Undo History", canHistory))
         {
            // Steps that can be redone, dimmed, the furthest first: picking one redoes that many steps
            // (the ids are keyed since steps often share the same label)
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(EditorTheme::TextDim));
            for (size_t i = editor.m_undo.RedoCount(); i-- > 0;)
            {
               ImGui::PushID(static_cast<int>(i) + 1000);
               if (ImGui::MenuItem(editor.m_undo.RedoLabel(i).c_str()))
                  editor.RedoSteps(i + 1);
               ImGui::PopID();
            }
            ImGui::PopStyleColor();
            // Done steps, the newest first: the top one is the current state (marked, grayed out)
            for (size_t i = 0; i < editor.m_undo.UndoCount(); ++i)
            {
               ImGui::PushID(static_cast<int>(i));
               if (ImGui::MenuItem(editor.m_undo.UndoLabel(i).c_str(), nullptr, i == 0, i != 0))
                  editor.UndoSteps(i);
               ImGui::PopID();
            }
            if (ImGui::MenuItem("Original", nullptr, editor.m_undo.UndoCount() == 0, editor.m_undo.UndoCount() > 0))
               editor.UndoSteps(editor.m_undo.UndoCount());
            ImGui::EndMenu();
         }
         if (ImGui::MenuItem(ICON_LC_REFRESH_CW " Adjust Last Operation...", "F9", false, editor.m_lastOp.type != EditorUI::LastOperation::Type::None))
         {
            editor.m_adjustPopupOpen = true;
            editor.m_adjustPopupJustOpened = true;
            editor.m_adjustPopupPos = ImGui::GetMousePos();
         }
         ImGui::Separator();
         // Selection actions of the Esc, Ctrl+C / Ctrl+V and Del shortcuts (the add part picker itself opens in the viewport pane)
         ImGui::BeginDisabled(editor.m_table->IsLocked());
         const bool canCopy
            = editor.m_pointEditPart ? (editor.m_pointEditCenter ? editor.m_centerSelected : editor.m_pointSel.size() == 1) : (editor.m_selection.GetType() == Selection::S_EDITABLE);
         if (ImGui::MenuItem(ICON_LC_COPY " Copy", "Ctrl+C", false, canCopy))
            editor.CopySelection();
         const bool canPaste = editor.m_pointEditPart
            ? (editor.m_pointEditCenter ? editor.m_centerSelected && VPX::EditorClipboard::HasPoint() : editor.m_pointSel.size() == 1 && VPX::EditorClipboard::HasPoint())
            : VPX::EditorClipboard::HasParts();
         if (ImGui::MenuItem(ICON_LC_CLIPBOARD_PASTE " Paste", "Ctrl+V", false, canPaste))
         {
            const ImVec2 center = editor.GetViewportContentCenter();
            editor.PasteSelection(&center);
         }
         if (ImGui::MenuItem(ICON_LC_TRASH_2 " Delete", "Del", false, editor.m_selection.GetType() == Selection::S_EDITABLE))
            editor.DeleteSelection();
         if (ImGui::MenuItem(ICON_LC_PLUS " Add Part...", "Shift+A", false, editor.m_pointEditPart == nullptr))
            editor.RequestAddPartPopup(editor.GetViewportContentCenter());
         ImGui::EndDisabled();
         ImGui::Separator();
         const bool canSelect = !editor.m_table->IsLocked() && editor.m_fileOp == nullptr;
         if (ImGui::MenuItem(ICON_LC_MOUSE_POINTER_2 " Select All", "A", false, canSelect))
            editor.SelectAllParts();
         if (ImGui::MenuItem(ICON_LC_X " Deselect", "Alt+A", false, canSelect && editor.m_selection.GetType() != Selection::S_NONE))
            editor.ClearSelection();
         if (ImGui::MenuItem(ICON_LC_LIST_FILTER " Invert Selection", "Ctrl+I", false, canSelect))
            editor.InvertSelection();
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_MAGNET " Snapping", "Shift+Tab", editor.m_snapEnabled))
            editor.ToggleSnapping();
         if (ImGui::BeginMenu(ICON_LC_MAGNET " Snap Options"))
         {
            Settings &settings = g_settingsService.GetAppSettings();
            if (ImGui::MenuItem("Target: Increment", nullptr, editor.m_snapTarget == 0))
            {
               editor.m_snapTarget = 0;
               settings.SetEditor_SnapTarget(0, false);
            }
            if (ImGui::MenuItem("Target: Points", nullptr, editor.m_snapTarget == 1))
            {
               editor.m_snapTarget = 1;
               settings.SetEditor_SnapTarget(1, false);
            }
            ImGui::Separator();
            if (ImGui::DragFloat("Move Increment", &editor.m_snapMove, 0.1f, 0.01f, 1000.f, "%.2f vpu"))
               settings.SetEditor_SnapMove(editor.m_snapMove, false);
            if (ImGui::DragFloat("Rotate Increment", &editor.m_snapRotate, 0.5f, 0.1f, 180.f, "%.1f deg"))
               settings.SetEditor_SnapRotate(editor.m_snapRotate, false);
            if (ImGui::DragFloat("Scale Increment", &editor.m_snapScale, 0.005f, 0.001f, 10.f, "%.3f"))
               settings.SetEditor_SnapScale(editor.m_snapScale, false);
            ImGui::Separator();
            if (ImGui::MenuItem("Affect Moves", nullptr, editor.m_snapAffectMove))
            {
               editor.m_snapAffectMove = !editor.m_snapAffectMove;
               settings.SetEditor_SnapAffectMove(editor.m_snapAffectMove, false);
            }
            if (ImGui::MenuItem("Affect Rotations", nullptr, editor.m_snapAffectRotate))
            {
               editor.m_snapAffectRotate = !editor.m_snapAffectRotate;
               settings.SetEditor_SnapAffectRotate(editor.m_snapAffectRotate, false);
            }
            if (ImGui::MenuItem("Affect Scales", nullptr, editor.m_snapAffectScale))
            {
               editor.m_snapAffectScale = !editor.m_snapAffectScale;
               settings.SetEditor_SnapAffectScale(editor.m_snapAffectScale, false);
            }
            ImGui::EndMenu();
         }
         ImGui::Separator();
         if (ImGui::BeginMenu(ICON_LC_RULER " Units"))
         {
            Settings &settings = g_settingsService.GetAppSettings();
            if (ImGui::RadioButton("VP Units", editor.m_units == EditorUI::Units::VPX))
               editor.m_units = EditorUI::Units::VPX;
            if (ImGui::RadioButton("Metric", editor.m_units == EditorUI::Units::Metric))
               editor.m_units = EditorUI::Units::Metric;
            if (ImGui::RadioButton("Imperial", editor.m_units == EditorUI::Units::Imperial))
               editor.m_units = EditorUI::Units::Imperial;
            if (static_cast<int>(editor.m_units) != settings.GetLiveEditor_Units())
               settings.SetLiveEditor_Units(static_cast<int>(editor.m_units), false);
            ImGui::EndMenu();
         }
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_SLIDERS_HORIZONTAL " Preferences..."))
            OpenPreferences();
         ImGui::EndMenu();
      }
      if (!editor.IsInspectMode() && ImGui::BeginMenu("Object"))
      {
         // Object menu: actions on the selection (the G/R/S items arm the transform op of the pane under the mouse, like their keys)
         ViewportPane *const vp = editor.GetTargetViewport();
         const bool canTransform = vp != nullptr && editor.m_selection.GetType() == Selection::S_EDITABLE;
         if (ImGui::MenuItem(ICON_LC_COPY " Duplicate", "Shift+D", false, editor.CanDuplicate()))
            editor.DuplicateSelection();
         if (ImGui::MenuItem(ICON_LC_TRASH_2 " Delete", "Del", false, editor.m_selection.GetType() == Selection::S_EDITABLE))
            editor.DeleteSelection();
         if (ImGui::MenuItem(ICON_LC_PLUS " Add Part...", "Shift+A", false, !editor.m_table->IsLocked() && editor.m_pointEditPart == nullptr))
            editor.RequestAddPartPopup(editor.GetViewportContentCenter());
         if (ImGui::MenuItem(ICON_LC_IMAGE " Add Reference Image...", nullptr, false, editor.m_referenceImageOp.CanAdd()))
            editor.m_referenceImageOp.OpenAddDialog();
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_MOVE_3D " Move", "G", vp != nullptr && vp->m_gizmoOperation == ImGuizmo::TRANSLATE, canTransform))
            vp->BeginTransformOp(ImGuizmo::TRANSLATE);
         if (ImGui::MenuItem(ICON_LC_ROTATE_3D " Rotate", "R", vp != nullptr && vp->m_gizmoOperation == ImGuizmo::ROTATE, canTransform))
            vp->BeginTransformOp(ImGuizmo::ROTATE);
         if (ImGui::MenuItem(ICON_LC_SCALE_3D " Scale", "S", vp != nullptr && vp->m_gizmoOperation == ImGuizmo::SCALE, canTransform))
            vp->BeginTransformOp(ImGuizmo::SCALE);
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_LAYERS " New Collection from Selection", nullptr, false, editor.m_selection.GetType() == Selection::S_EDITABLE))
            editor.CreateCollection(true);
         if (ImGui::MenuItem(ICON_LC_MOUSE_POINTER_2 " Edit Points", "Tab", false, editor.CanEnterPointEdit()))
            editor.EnterPointEditMode();
         ImGui::Separator();
         if (ImGui::BeginMenu(ICON_LC_CIRCLE_DOT " Set Origin"))
         {
            const bool canSetOrigin = SetOriginOp::CanApply(editor);
            if (ImGui::MenuItem("Origin to Geometry", nullptr, false, canSetOrigin))
               SetOriginOp::Apply(editor, SetOriginOp::Mode::Geometry);
            if (ImGui::MenuItem("Origin to Geometry Bottom", nullptr, false, canSetOrigin))
               SetOriginOp::Apply(editor, SetOriginOp::Mode::GeometryBottom);
            if (ImGui::MenuItem("Origin to Scene Origin", nullptr, false, canSetOrigin))
               SetOriginOp::Apply(editor, SetOriginOp::Mode::Scene);
            ImGui::EndMenu();
         }
         if (ImGui::BeginMenu(ICON_LC_IMAGE " Reference Image"))
         {
            const bool canApply = editor.m_referenceImageOp.CanApplyToSelection();
            if (ImGui::MenuItem("Fit to Image", nullptr, false, canApply))
               editor.m_referenceImageOp.FitSelection();
            if (ImGui::MenuItem("Replace Image...", nullptr, false, canApply))
               editor.m_referenceImageOp.OpenReplaceDialog();
            ImGui::EndMenu();
         }
         ImGui::EndMenu();
      }
      if (!editor.IsInspectMode() && ImGui::BeginMenu("Tools"))
      {
         if (ImGui::BeginMenu(ICON_LC_LIGHTBULB " Light States"))
         {
            // The editor does not run the table script (which usually sets the lamp states): set the light state of the table or of a collection here
            // (same update as the State property: persisted value + emitted intensity + live play state)
            const auto setStates = [](const vector<IEditable *> &parts, const float state)
            {
               for (IEditable *const part : parts)
                  if (part->GetItemType() == eItemLight)
                  {
                     Light *const light = static_cast<Light *>(part);
                     light->m_d.m_state = state;
                     light->m_currentIntensity = light->m_d.m_intensity * light->m_d.m_intensity_scale * light->m_d.m_state;
                     light->m_currentFilamentTemperature = bulb_emission_to_filament_temperature(state);
                     light->setInPlayState(state > 1.f ? (float)LightStateBlinking : state);
                  }
            };
            const float states[] = { 0.f, 0.706f, 1.f };
            const char *stateNames[] = { "Off", "0.706 (lamp top)", "On" };
            if (ImGui::BeginMenu("All Lights"))
            {
               for (int i = 0; i < 3; i++)
                  if (ImGui::MenuItem(stateNames[i]))
                     setStates(editor.m_table->GetParts(), states[i]);
               ImGui::EndMenu();
            }
            for (auto *const collection : editor.m_table->GetCollections())
            {
               bool hasLight = false;
               for (const IEditable *const part : collection->GetParts())
                  hasLight |= part->GetItemType() == eItemLight;
               if (hasLight && ImGui::BeginMenu(collection->m_name.c_str()))
               {
                  for (int i = 0; i < 3; i++)
                     if (ImGui::MenuItem(stateNames[i]))
                        setStates(collection->GetParts(), states[i]);
                  ImGui::EndMenu();
               }
            }
            ImGui::EndMenu();
         }
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_EYE " Inspect Renderer..."))
            editor.m_inspectionModal.Show();
         ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View"))
      {
         // View actions apply to the edit viewport's camera (the bound pane's own camera definition)
         ViewportPane *const vp = editor.GetTargetViewport();
         const auto viewItem = [vp](const char *label, const char *keys, const ViewportPane::PredefinedView view)
         {
            if (ImGui::MenuItem(label, keys, vp != nullptr && vp->m_viewType == ViewMode::EditorCam && vp->m_predefinedView == view, vp != nullptr))
               vp->AnimateViewTo(view);
         };
         viewItem("Top", "Numpad 7", ViewportPane::PredefinedView::Top);
         viewItem("Bottom", "Ctrl+Numpad 7", ViewportPane::PredefinedView::Bottom);
         viewItem("Front", "Numpad 1", ViewportPane::PredefinedView::Front);
         viewItem("Back", "Ctrl+Numpad 1", ViewportPane::PredefinedView::Back);
         viewItem("Left", "Numpad 3", ViewportPane::PredefinedView::Left);
         viewItem("Right", "Ctrl+Numpad 3", ViewportPane::PredefinedView::Right);
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_VIDEO " Perspective / " ICON_LC_VIDEO_OFF " Orthographic", "Numpad 5", false, vp != nullptr))
         {
            vp->SetPerspectiveCam(!vp->m_perspectiveCam);
         }
         if (ImGui::MenuItem(ICON_LC_LOCK " Lock Orbit", "Numpad /", vp != nullptr && vp->m_orbitLock, vp != nullptr))
            vp->m_orbitLock = !vp->m_orbitLock;
         ImGui::Separator();
         if (ImGui::BeginMenu(ICON_LC_LENS " Shading"))
         {
            const auto shadeItem = [vp](const char *label, const Renderer::ShadeMode mode)
            {
               if (ImGui::MenuItem(label, nullptr, vp != nullptr && vp->m_shadeMode == mode) && vp != nullptr)
                  vp->m_shadeMode = mode;
            };
            shadeItem(ICON_LC_GLOBE " Wireframe", Renderer::ShadeMode::NoDepthWireframe);
            shadeItem(ICON_LC_CIRCLE " Solid", Renderer::ShadeMode::Wireframe);
            shadeItem(ICON_LC_LENS " Rendered", Renderer::ShadeMode::Default);
            ImGui::EndMenu();
         }
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_VIDEO " Frame Selected", "Numpad .", false, vp != nullptr && editor.CanFrameSelection()))
            vp->FrameSelection();
         if (ImGui::MenuItem(ICON_LC_GLOBE " Frame All", "Home", false, vp != nullptr))
            vp->FrameAll();
         if (ImGui::MenuItem(ICON_LC_SQUARE " Toggle Maximize Area", "Ctrl+Space"))
            editor.ToggleMaximizeArea();
         if (ImGui::MenuItem(ICON_LC_LAYERS " Tool Shelf", "T", vp != nullptr && vp->IsToolShelfShown(), vp != nullptr))
            vp->ToggleToolShelf();
         ImGui::Separator();
         if (ImGui::MenuItem(ICON_LC_REFRESH_CW " Reset Layouts", nullptr, false, !editor.m_table->IsLocked()))
            editor.ResetAreaLayouts();
         ImGui::EndMenu();
      }
      if (editor.IsInspectMode() && !editor.m_table->IsLocked() && ImGui::BeginMenu("Debug"))
      {
         if (ImGui::MenuItem(ICON_LC_CODE_XML " Open debugger"))
            editor.m_player->m_showDebugger = true;
         if (ImGui::MenuItem(ICON_LC_EYE " Renderer Inspection"))
            editor.m_inspectionModal.Show();
         if (ImGui::MenuItem(editor.m_player->IsPlaying() ? ICON_LC_PAUSE " Pause" : ICON_LC_PLAY " Play"))
            editor.m_player->SetPlayState(!editor.m_player->IsPlaying());
         if (ImGui::MenuItem(ICON_LC_SKIP_FORWARD " Step", nullptr, false, !editor.m_player->IsPlaying()))
            editor.m_player->m_step = true;
         ImGui::EndMenu();
      }
      ImGui::Separator();

      // Layout selection as buttons after the menus (Blender workspaces)
      ImGui::BeginDisabled(editor.m_table->IsLocked());
      static const char *const layoutNames[] = { "Layout", "Library", "Scripting", "Backdrop", "Blueprint" };
      for (int i = 0; i < 5; ++i)
      {
         const char *const label = layoutNames[i];
         const bool active = (editor.m_activeLayoutIndex == i);
         if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
         if (ImGui::Button(label))
            editor.SetAreaLayoutIndex(i);
         if (active)
            ImGui::PopStyleColor();
         ImGui::SameLine();
      }
      ImGui::EndDisabled();
      float buttonWidth = 0.f;
      if (editor.IsInspectMode())
         buttonWidth += IconButtonInkSize(ICON_LC_REPLY).x;
      buttonWidth += IconButtonInkSize(ICON_LC_X).x;
      // Play button, centered in the free space left between the layout buttons and the close buttons
      const float playWidth = IconButtonInkSize(ICON_LC_PLAY).x;
      const float freeWidth = ImGui::GetContentRegionAvail().x - buttonWidth;
      if (freeWidth > playWidth)
      {
         ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (freeWidth - playWidth) * 0.5f);
         ImGui::BeginDisabled(editor.m_table->IsLocked());
         if (BarIconButtonInk(ICON_LC_PLAY, "##PlayTest"))
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
         if (BarIconButtonInk(ICON_LC_REPLY, "##BackToPlayer")) // ICON_LC_SQUARE
            editor.Close();
         if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Get back to player");
      }
      if (BarIconButtonInk(ICON_LC_X, "##CloseEditor"))
         editor.RequestClose(editor.IsInspectMode() ? Player::CS_STOP_PLAY : Player::CS_CLOSE_APP);
      if (ImGui::IsItemHovered())
         ImGui::SetTooltip("Close editor");
      m_menuBarHeight = ImGui::GetWindowSize().y;
      ImGui::PopStyleVar();
      ImGui::EndMainMenuBar();
   }
}


void EditorChrome::RenderPreferences()
{
   if (!m_preferencesOpen)
      return;
   EditorUI &editor = m_editor;
   const float dpi = editor.m_liveUI.GetDPI();
   ImGui::SetNextWindowSize(ImVec2(560.f * dpi, 420.f * dpi), ImGuiCond_Appearing);
   if (ImGui::Begin("Preferences", &m_preferencesOpen, ImGuiWindowFlags_NoSavedSettings))
   {
      static const char *pageNames[] = { "Navigation" }; // Page rail entries, matching the content switch below
      if (ImGui::BeginChild("##PrefRail", ImVec2(140.f * dpi, 0.f), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX))
      {
         for (int i = 0; i < IM_ARRAYSIZE(pageNames); ++i)
            if (ImGui::Selectable(pageNames[i], m_preferencesPage == i))
               m_preferencesPage = i;
      }
      ImGui::EndChild();
      ImGui::SameLine();
      if (ImGui::BeginChild("##PrefContent"))
      {
         switch (m_preferencesPage)
         {
         case 0: // Navigation
         {
            Settings &settings = g_settingsService.GetAppSettings();
            ImGui::SeparatorText("Orbit && Zoom");
            if (ImGui::Checkbox("Orbit Around Selection", &editor.m_navOrbitSelection))
               settings.SetEditor_NavOrbitSelection(editor.m_navOrbitSelection, false);
            if (ImGui::Checkbox("Zoom To Mouse Position", &editor.m_navZoomToMouse))
               settings.SetEditor_NavZoomToMouse(editor.m_navZoomToMouse, false);
            if (ImGui::Checkbox("Auto Depth (Orbit Around Point Under Cursor)", &editor.m_navAutoDepth))
               settings.SetEditor_NavAutoDepth(editor.m_navAutoDepth, false);
            if (ImGui::Checkbox("Invert Zoom", &editor.m_navInvertZoom))
               settings.SetEditor_NavInvertZoom(editor.m_navInvertZoom, false);
            if (ImGui::SliderFloat("Orbit Sensitivity", &editor.m_navOrbitSensitivity, 0.1f, 5.f, "%.1f"))
               settings.SetEditor_NavOrbitSensitivity(editor.m_navOrbitSensitivity, false);
            ImGui::SeparatorText("Input");
            if (ImGui::Checkbox("Alt + Drag Acts As Middle Button", &editor.m_navAltMiddleButton))
               settings.SetEditor_NavAltMiddleButton(editor.m_navAltMiddleButton, false);
            if (ImGui::Checkbox("Trackpad (Scroll Pans, Ctrl/Cmd+Scroll Zooms)", &editor.m_navTrackpad))
               settings.SetEditor_NavTrackpad(editor.m_navTrackpad, false);
            if (ImGui::Checkbox("Emulate Numpad (Top Row Digits)", &editor.m_navEmulateNumpad))
               settings.SetEditor_NavEmulateNumpad(editor.m_navEmulateNumpad, false);
            if (ImGui::Checkbox("Blender Transform Keys (G / R / S)", &editor.m_navBlenderTransform))
               settings.SetEditor_NavBlenderTransform(editor.m_navBlenderTransform, false);
            ImGui::SeparatorText("View");
            ViewportPane *const vp = editor.GetTargetViewport();
            float fov = editor.m_navFovY;
            if (ImGui::SliderFloat("Field Of View", &fov, 15.f, 100.f, "%.1f deg"))
            {
               if (vp != nullptr)
                  vp->SetFov(fov);
               else
                  editor.m_navFovY = clamp(fov, 15.f, 100.f);
               settings.SetEditor_NavFovY(editor.m_navFovY, false);
            }
            static const char *selectionModes[] = { "Edges", "Edges + Tint", "Hidden" };
            if (ImGui::BeginCombo("Selection Display", selectionModes[editor.m_navSelectionDisplay]))
            {
               for (int i = 0; i < 3; i++)
                  if (ImGui::Selectable(selectionModes[i], editor.m_navSelectionDisplay == i))
                  {
                     editor.m_navSelectionDisplay = i;
                     settings.SetEditor_NavSelectionDisplay(i, false);
                  }
               ImGui::EndCombo();
            }
            break;
         }
         }
      }
      ImGui::EndChild();
   }
   ImGui::End();
}

void EditorChrome::RenderStatusBar(const bool showFullUI)
{
   if (!showFullUI || m_statusBarHeight <= 0.f)
      return;
   EditorUI &editor = m_editor;
   const ImGuiIO &io = ImGui::GetIO();
   const float dpi = editor.m_liveUI.GetDPI();
   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   const float height = m_statusBarHeight;

   // Key hints of the current context: [key chip] label. The pane owning the active interaction,
   // or the one under the mouse, provides its own helper messages (see EditorAreaView::AppendStatusHints)
   vector<StatusHint> hints;
   const string mod = StatusModKey;
   const ImVec2 mouse = ImGui::GetMousePos();
   if (io.WantTextInput)
      hints = { { "Enter", "Confirm" }, { "Esc", "Cancel" }, { mod + "+A", "Select All Text" } };
   else if (editor.m_areaDrag.active)
      hints = { { "Drag inside the area", "Split" }, { "Drag into a neighbour", "Join" }, { "Esc", "Cancel" } };
   else if (editor.m_areaModal != EditorUI::AreaModalKind::None)
      hints = { { "Move Mouse", editor.m_areaModal == EditorUI::AreaModalKind::Split ? "Place Split" : "Choose Area to Remove" }, { ICON_LC_MOUSE_LEFT, "Confirm" },
         { ICON_LC_MOUSE_RIGHT " / Esc", "Cancel" } };
   else if (editor.m_splitDragNode >= 0)
      hints = { { "Drag", "Resize Areas" }, { "Release " ICON_LC_MOUSE_LEFT, "Confirm" } };
   else
   {
      // Context view: a modal transform op owns the input wherever the mouse is, then an in-flight
      // gizmo or scene drag (the mouse may have left the pane's rect), then the pane under the mouse
      EditorAreaView *contextView = nullptr;
      ViewportPane *const targetVp = editor.GetTargetViewport();
      if (targetVp != nullptr && targetVp->m_op.op != ImGuizmo::OPERATION(0))
         contextView = targetVp;
      else if (targetVp != nullptr && ImGuizmo::IsUsingAny()) // IsUsing() only matches inside a pane's gizmo id scope
         contextView = targetVp;
      else if (editor.m_captureVp != nullptr)
         contextView = editor.m_captureVp;
      else if (editor.m_captureBp != nullptr)
         contextView = editor.m_captureBp;
      else
      {
         const int leaf = AreaLayout::LeafAt(editor.m_leafRects, mouse.x, mouse.y);
         const auto it = leaf >= 0 ? editor.m_areaViews.find(leaf) : editor.m_areaViews.end();
         if (it != editor.m_areaViews.end())
            contextView = it->second.get();
      }
      if (contextView != nullptr)
         contextView->AppendStatusHints(hints);
      else
      {
         bool overSplitter = false;
         for (const SplitterRect &splitter : editor.m_splitterRects)
            overSplitter |= mouse.x >= splitter.grab.x0 && mouse.x <= splitter.grab.x1 && mouse.y >= splitter.grab.y0 && mouse.y <= splitter.grab.y1;
         if (overSplitter)
            hints = { { "Drag", "Resize Areas" } };
         else if (editor.IsInspectMode())
            hints = { { mod + "+S", "Save Changes" } };
         else if (!editor.m_table->IsLocked())
            hints = { { mod + "+S", "Save Table" }, { "F5", "Play Test" } };
      }
   }

   // Last operation report on the right, fading out
   const double age = ImGui::GetTime() - editor.m_reportTime;
   const float reportAlpha = age < 3.5 ? 1.f : age < 4.5 ? static_cast<float>(4.5 - age) : 0.f;
   const bool hasReport = reportAlpha > 0.f && !editor.m_report.empty();

   const float pad = 8.f * dpi;
   ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - height));
   ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, height));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
   ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::V(editor.IsInspectMode() ? EditorTheme::PlayBar : EditorTheme::Bar));
   ImGui::Begin("STATUSBAR", nullptr,
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav
         | ImGuiWindowFlags_NoFocusOnAppearing);
   ImGui::PopStyleColor();
   ImGui::PopStyleVar(3);
   ImDrawList *const draw = ImGui::GetWindowDrawList();
   const ImVec2 barMin = ImGui::GetWindowPos();
   const ImVec2 barMax(barMin.x + viewport->Size.x, barMin.y + height);
   draw->AddLine(barMin, ImVec2(barMax.x, barMin.y), EditorTheme::Border, 1.f); // 1 px top border

   const float textH = ImGui::GetFontSize();
   const float textY = barMin.y + 0.5f * (height - textH);
   float rightLimit = barMax.x - pad;
   if (hasReport)
   {
      const ImVec2 size = ImGui::CalcTextSize(editor.m_report.c_str());
      const float x = barMax.x - pad - size.x;
      const auto fade = [reportAlpha](ImU32 color) { return (color & 0x00FFFFFFu) | (static_cast<ImU32>(static_cast<float>((color >> 24) & 0xFFu) * reportAlpha) << 24); };
      draw->AddText(ImVec2(x, textY), fade(EditorTheme::Text), editor.m_report.c_str());
      rightLimit = x - 2.f * pad;
   }

   // Hints from the left, as many as fit before the report
   const float chipPadX = 4.f * dpi, chipPadY = 1.f * dpi, gap = 4.f * dpi, hintGap = 12.f * dpi;

   // Key chips are sized on the glyphs' ink bounds: CalcTextSize only covers advances and the
   // line height, but the merged icon glyphs are drawn a bit larger than the text and would
   // overflow the chip. One height for the whole row (expands when a key contains icons).
   const auto keyInkBounds = [textH](const string &key, float &x0, float &x1, float &y0, float &y1)
   {
      const ImVec4 ink = TextInkBounds(key.c_str());
      x0 = ink.x;
      x1 = ImMax(ink.z, ImGui::CalcTextSize(key.c_str()).x); // union with the layout box
      y0 = ink.y;
      y1 = ImMax(ink.w, textH);
   };
   float chipTop = 0.f, chipBottom = textH;
   for (const StatusHint &hint : hints)
   {
      float x0, x1, y0, y1;
      keyInkBounds(hint.key, x0, x1, y0, y1);
      chipTop = ImMin(chipTop, y0);
      chipBottom = ImMax(chipBottom, y1);
   }
   float x = barMin.x + pad;
   for (const StatusHint &hint : hints)
   {
      float inkX0, inkX1, inkY0, inkY1;
      keyInkBounds(hint.key, inkX0, inkX1, inkY0, inkY1);
      const ImVec2 labelSize = ImGui::CalcTextSize(hint.label.c_str());
      const float width = (inkX1 - inkX0) + 2.f * chipPadX + gap + labelSize.x;
      if (x + width > rightLimit)
         break;
      const ImVec2 chipMin(x, textY + chipTop - chipPadY);
      const ImVec2 chipMax(x + (inkX1 - inkX0) + 2.f * chipPadX, textY + chipBottom + chipPadY);
      draw->AddRectFilled(chipMin, chipMax, EditorTheme::Rgb(255, 255, 255, 20), 3.f * dpi);
      draw->AddRect(chipMin, chipMax, EditorTheme::Hover, 3.f * dpi);
      draw->AddText(ImVec2(chipMin.x + chipPadX - inkX0, chipMin.y + chipPadY - chipTop), EditorTheme::TextDim, hint.key.c_str());
      draw->AddText(ImVec2(chipMax.x + gap, textY), EditorTheme::TextDim, hint.label.c_str());
      x += width + hintGap;
   }
   ImGui::End();
}
}
