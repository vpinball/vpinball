// license:GPLv3+

#include "core/stdafx.h"
#include "EditorChrome.h"
#include "BlueprintPane.h"
#include "EditorTheme.h"
#include "IconsLucide.h"
#include "ViewportPane.h" // GetTargetViewport()->m_imageRect (status overlay anchor)

#include "core/player.h"
#include "ui/EditorClipboard.h"
#include "ui/live/EditorUI.h"
#include "ui/live/LiveUI.h"

#include "imgui/imgui_internal.h" // ImTextCharFromUtf8 (status hint key chips ink bounds)

namespace VPX::EditorUI
{

namespace
{
   constexpr float StatusBarHeight = 28.f; // Bottom status bar, in pixels at 100% DPI
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
         {
            const ImVec2 center = editor.GetViewportContentCenter();
            editor.PasteSelection(&center);
         }
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
            shadeItem(ICON_LC_GLOBE " Wireframe", Renderer::ShadeMode::NoDepthWireframe);
            shadeItem(ICON_LC_CIRCLE " Solid", Renderer::ShadeMode::Wireframe);
            shadeItem(ICON_LC_LENS " Rendered", Renderer::ShadeMode::Default);
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
      static const char *const layoutNames[] = { "Layout", "Library", "Scripting", "Backdrop", "Blueprint" };
      for (int i = 0; i < 5; ++i)
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
      else if (targetVp != nullptr && ImGuizmo::IsUsing())
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
   ImFontBaked *const baked = ImGui::GetFontBaked();

   // Key chips are sized on the glyphs' ink bounds: CalcTextSize only covers advances and the
   // line height, but the merged icon glyphs are drawn a bit larger than the text and would
   // overflow the chip. One height for the whole row (expands when a key contains icons).
   const auto keyInkBounds = [baked, textH](const string &key, float &x0, float &x1, float &y0, float &y1)
   {
      x0 = y0 = 0.f;
      x1 = y1 = 0.f;
      float penX = 0.f;
      const char *const keyEnd = key.c_str() + key.size();
      for (const char *s = key.c_str(); s < keyEnd;)
      {
         unsigned int c = 0;
         s += ImTextCharFromUtf8(&c, s, keyEnd);
         const ImFontGlyph *const glyph = c != 0 ? baked->FindGlyph(static_cast<ImWchar>(c)) : nullptr;
         if (glyph == nullptr)
            continue;
         if (glyph->Visible)
         {
            x0 = ImMin(x0, penX + glyph->X0);
            x1 = ImMax(x1, penX + glyph->X1);
            y0 = ImMin(y0, glyph->Y0);
            y1 = ImMax(y1, glyph->Y1);
         }
         penX += glyph->AdvanceX;
      }
      x1 = ImMax(x1, penX); // union with the layout box
      y1 = ImMax(y1, textH);
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
