// license:GPLv3+

#include "core/stdafx.h"
#include "ViewportSidebar.h"

#include "EditorTheme.h"
#include "ViewportPane.h"
#include "core/SettingsService.h"
#include "math/bbox.h"
#include "ui/live/EditorUI.h"
#include "ui/live/IconsLucide.h"
#include "ui/live/ImGuiHelpers.h"
#include "ui/live/LiveUI.h"

namespace VPX::EditorUI
{

float ViewportSidebar::ToDisplayLen(const float vpu) const
{
   switch (m_editor.m_units)
   {
   case EditorUI::Units::Metric: return VPUTOMM(vpu);
   case EditorUI::Units::Imperial: return VPUTOINCHES(vpu);
   default: return vpu;
   }
}
float ViewportSidebar::ToVpuLen(const float v) const
{
   switch (m_editor.m_units)
   {
   case EditorUI::Units::Metric: return MMTOVPU(v);
   case EditorUI::Units::Imperial: return INCHESTOVPU(v);
   default: return v;
   }
}
const char* ViewportSidebar::LenUnitLabel() const
{
   switch (m_editor.m_units)
   {
   case EditorUI::Units::Metric: return " mm";
   case EditorUI::Units::Imperial: return " in";
   default: return " vpu";
   }
}

void ViewportSidebar::CopyFrom(const ViewportSidebar& src)
{
   m_shown = src.m_shown;
   m_expanded = src.m_expanded;
   m_tab = src.m_tab;
   m_width = src.m_width;
}

nlohmann::json ViewportSidebar::SaveSetup() const
{
   nlohmann::json j = nlohmann::json::object();
   j["open"] = m_shown;
   j["expanded"] = m_expanded;
   j["tab"] = static_cast<int>(m_tab);
   j["w"] = m_width;
   return j;
}

void ViewportSidebar::LoadSetup(const nlohmann::json& j)
{
   if (!j.is_object())
      return;
   if (const auto it = j.find("open"); it != j.end() && it->is_boolean())
      m_shown = it->get<bool>();
   if (const auto it = j.find("expanded"); it != j.end() && it->is_boolean())
      m_expanded = it->get<bool>();
   if (const auto it = j.find("tab"); it != j.end() && it->is_number_integer())
      m_tab = static_cast<Tab>(clamp(it->get<int>(), 0, static_cast<int>(Tab::View)));
   if (const auto it = j.find("w"); it != j.end() && it->is_number())
   {
      const float w = it->get<float>();
      if (std::isfinite(w) && w > 0.f)
         m_width = w;
   }
}

void ViewportSidebar::Render()
{
   m_drawnWidth = 0.f;
   m_hovered = false;
   // Same exclusions as the tool shelf, except the sidebar also serves the desktop backdrop (its Item
   // tab transforms backdrop parts) and the inspection session (view options stay available there)
   if (!m_shown || m_pane.m_viewType == ViewMode::PreviewCam || m_pane.m_flyMode || m_editor.m_inspectionModal.IsVisible() || m_editor.FindLeafRect(m_pane.m_leafId) == nullptr)
      return;
   const AreaRect& rect = m_pane.m_imageRect;
   const float dpi = m_editor.m_liveUI.GetDPI();
   const float gap = 3.f * dpi;
   if (rect.Width() < 300.f * dpi || rect.Height() < 140.f * dpi)
      return;

   // Tab chips: a floating column of icon buttons, vertically centered on the right edge of the
   // image (Blender's region tabs; they stay clear of the nav gizmo in the top corner and of the
   // shelf which is anchored at the top)
   static const struct
   {
      Tab id;
      const char* icon;
      const char* label; // Tooltip (the chip carries only the icon)
   } sidebarTabs[] = {
      { Tab::Item, ICON_LC_TAG, "Item" },
      { Tab::Tool, ICON_LC_SLIDERS_HORIZONTAL, "Tool" },
      { Tab::View, ICON_LC_EYE, "View" },
   };
   float railW = 0.f, railH = (IM_ARRAYSIZE(sidebarTabs) - 1) * gap;
   for (const auto& tab : sidebarTabs)
   {
      const ImVec2 chipSize = IconButtonInkSize(tab.icon);
      railW = max(railW, chipSize.x);
      railH += chipSize.y;
   }
   if (railH > rect.Height())
      return; // The chips do not fit: the whole sidebar is hidden (the shelf would not either)

   char name[40];
   const ImVec2 railPos(rect.x1 - railW, rect.y0 + (rect.Height() - railH) * 0.5f);
   ImGui::SetNextWindowPos(railPos);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
   ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, gap));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f * dpi);
   ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::V(0u));
   snprintf(name, sizeof(name), "##SidebarRail%d", m_pane.m_leafId); // Unique per pane (window names are global)
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
         | ImGuiWindowFlags_NoNav);
   ImGui::PopStyleColor();
   ImGui::PopStyleVar(5);
   m_hovered = ImGui::IsWindowHovered();
   for (const auto& tab : sidebarTabs)
   {
      const bool active = m_tab == tab.id && m_expanded;
      ImGui::PushStyleColor(ImGuiCol_Button, EditorTheme::V(active ? EditorTheme::TabActive : (EditorTheme::Strip & 0x00FFFFFFu) | (217u << 24)));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::V(EditorTheme::TabHover));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive, EditorTheme::V(EditorTheme::TabHover));
      ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(active ? EditorTheme::Accent : EditorTheme::Text));
      if (IconButtonInk(tab.icon, (string("##") + tab.label).c_str()))
      {
         if (m_tab == tab.id)
            m_expanded = !m_expanded; // Clicking the open tab's chip collapses the shelf to its chips
         else
         {
            m_tab = tab.id;
            m_expanded = true;
         }
      }
      ImGui::SetItemTooltip("%s", tab.label);
      ImGui::PopStyleColor(4);
   }
   ImGui::End();
   m_drawnWidth = railW;
   if (!m_expanded)
      return;

   // Shelf: an overlay hugging its content, anchored to the top right of the image next to the
   // chips — the same floating look as the tool shelf (translucent strip, rounded corners)
   // rather than a pane docked along the whole edge
   const float maxWidth = min(400.f * dpi, rect.Width() * 0.66f);
   m_width = clamp(m_width, 180.f, maxWidth / dpi);
   const float width = m_width * dpi;
   const float pad = 4.f * dpi;
   // The height hugs the content measured last frame (a fixed estimate until the first measure),
   // capped to the image height: the content child scrolls beyond that
   const float contentH = min(m_contentHeight > 0.f ? m_contentHeight : 300.f * dpi, rect.Height() - 2.f * pad);
   ImGui::SetNextWindowPos(ImVec2(rect.x1 - railW - gap - width, rect.y0));
   ImGui::SetNextWindowSize(ImVec2(width, 2.f * pad + contentH));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(pad + 6.f * dpi, pad));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.f * dpi);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.f);
   ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::V((EditorTheme::Strip & 0x00FFFFFFu) | (217u << 24))); // ~85% alpha, like the tool shelf
   ImGui::PushStyleColor(ImGuiCol_Border, EditorTheme::V(EditorTheme::Border));
   snprintf(name, sizeof(name), "##Sidebar%d", m_pane.m_leafId);
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
   ImGui::PopStyleColor(2);
   ImGui::PopStyleVar(3);
   m_drawnWidth += width + gap;
   m_hovered |= ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

   ImGui::BeginChild("##content", ImVec2(0.f, contentH));
   const float y0 = ImGui::GetCursorPosY();
   switch (m_tab)
   {
   case Tab::Item: RenderItemTab(); break;
   case Tab::Tool: RenderToolTab(); break;
   case Tab::View: RenderViewTab(); break;
   }
   m_contentHeight = ImGui::GetCursorPosY() - y0; // The measured height the window is sized on next frame
   ImGui::EndChild();

   // Resize grab strip on the left edge (submitted last so it wins its zone over the content)
   ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
   ImGui::InvisibleButton("##resize", ImVec2(6.f * dpi, ImGui::GetWindowHeight()));
   if (ImGui::IsItemHovered() || ImGui::IsItemActive())
      ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
   if (ImGui::IsItemActive())
      m_width = clamp(m_width - ImGui::GetIO().MouseDelta.x / dpi, 180.f, maxWidth / dpi);
   ImGui::End();
}

void ViewportSidebar::MarkSelectionUndo()
{
   if (m_editor.m_table->m_liveBaseTable != nullptr)
      return; // No undo in inspection mode (same guard as the gizmo drags)
   m_editor.m_undo.BeginUndo();
   if (m_editor.m_pointEditPart != nullptr)
      m_editor.m_undo.MarkForUndo(m_editor.m_pointEditPart->GetEditable());
   else
      for (const auto& part : m_editor.m_multiSel)
         if (!part->GetEditable()->IsUILocked())
            m_editor.m_undo.MarkForUndo(part->GetEditable());
   m_editor.m_undo.EndUndo();
}

void ViewportSidebar::RenderItemTab()
{
   EditorUI& editor = m_editor;
   const float dpi = editor.m_liveUI.GetDPI();

   // Selection header: the active part's icon and name (multi-selection count), or the point edit context
   if (editor.m_pointEditPart != nullptr)
   {
      if (editor.m_pointEditCenter)
         ImGui::TextDisabled("Center Point");
      else
         ImGui::TextDisabled("Drag Points (%d)", (int)editor.m_pointSel.size());
   }
   else if (const std::shared_ptr<EditorUIPart>& part = editor.m_selection.GetPart(); part != nullptr)
   {
      if (const std::shared_ptr<Sampler>& icon = editor.PartIcon(part->GetEditable()); icon != nullptr)
         ImGui::Image(icon, ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight()));
      else
         ImGui::TextUnformatted(ICON_LC_TAG);
      ImGui::SameLine();
      if (editor.m_multiSel.size() > 1)
         ImGui::Text("%s (%d parts)", part->GetEditable()->GetName().c_str(), (int)editor.m_multiSel.size());
      else
         ImGui::TextUnformatted(part->GetEditable()->GetName().c_str());
   }
   else
      ImGui::TextDisabled("No selection");

   // Transform of the selection (the same values the gizmo operates on: active part's transform, or the
   // selected points' centroid in drag point edit mode)
   ImGui::SeparatorText("Transform");
   Matrix3D transform;
   const bool hasTransform = editor.GetSelectionTransform(transform);
   if (!hasTransform)
   {
      transform = Matrix3D::MatrixIdentity();
      // The selection's transform may still be read when it can not be edited (locked part): show
      // its actual values in the disabled fields instead of the identity placeholders
      if (editor.m_selection.GetType() == Selection::S_EDITABLE && editor.m_selection.GetPart() != nullptr)
         editor.m_selection.GetPart()->GetTransform(transform);
   }
   vec3 pos(0.f, 0.f, 0.f), scale(1.f, 1.f, 1.f), rot(0.f, 0.f, 0.f);
   EditorUI::ExtractTRS(transform, pos, scale, rot);

   // The transform components the current selection supports: drag points only move in their XY
   // plane; otherwise the axes intersect every selected part's supported mask (the same rule the
   // gizmo applies to its axes — TM_ScaleAll counts for every scale axis)
   bool supported[3][3]; // [component 0=Location 1=Rotation 2=Scale][axis]
   if (editor.m_pointEditPart != nullptr)
   {
      for (int c = 0; c < 3; ++c)
      {
         supported[c][0] = supported[c][1] = true;
         supported[c][2] = false;
      }
   }
   else
   {
      for (int c = 0; c < 3; ++c)
         for (int a = 0; a < 3; ++a)
            supported[c][a] = !editor.m_multiSel.empty();
      for (const auto& part : editor.m_multiSel)
      {
         Matrix3D partTransform;
         const EditorUIPart::TransformMask tm = part->GetTransform(partTransform);
         // TM_ScaleAll without per-axis bits (ball, gate, plunger) means a uniform scale any axis
         // can drive; combined with per-axis bits (curve parts: X|Y|ScaleAll) the bits define the
         // axes the part can actually scale
         const bool uniformOnly = (tm & EditorUIPart::TM_ScaleAny) == 0;
         for (int a = 0; a < 3; ++a)
         {
            supported[0][a] &= (tm & (EditorUIPart::TM_TransX << a)) != 0;
            supported[1][a] &= (tm & (EditorUIPart::TM_RotX << a)) != 0;
            supported[2][a] &= (tm & (EditorUIPart::TM_ScaleX << a)) != 0 || (uniformOnly && (tm & EditorUIPart::TM_ScaleAll) != 0);
         }
      }
   }

   // Length fields: 3 decimals for inches (a coarser unit needs the extra precision), 1 otherwise
   const string locFmtS = string(editor.m_units == EditorUI::Units::Imperial ? "%.3f" : "%.1f") + LenUnitLabel();
   const char* const fmts[3] = { locFmtS.c_str(), "%.1f", "%.3f" };
   // Step sizes in display units (length adapts to the unit system), fast step on Ctrl+step
   const bool imperial = editor.m_units == EditorUI::Units::Imperial;
   const float steps[3] = { imperial ? 0.05f : 1.f, 1.f, 0.01f };
   const float fastSteps[3] = { imperial ? 0.5f : 10.f, 10.f, 0.1f };

   // Like the gizmo, only a pane of the selection's domain (desktop backdrop / 3D scene) transforms it: the fields still show its values, disabled
   const bool backdropView = m_pane.m_viewType == ViewMode::DesktopBackdrop;
   const bool inViewDomain = editor.IsSelectionInViewDomain(backdropView);
   if (!inViewDomain)
   {
      ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(EditorTheme::TextDim));
      ImGui::TextWrapped(backdropView ? "3D scene selection: edit it in a 3D view" : "Desktop backdrop selection: edit it in a backdrop view");
      ImGui::PopStyleColor();
   }
   const bool editable = hasTransform && inViewDomain && !editor.m_table->IsLocked() && !editor.IsInspectMode();

   ImGui::BeginDisabled(!editable);
   {
      bool changed = false;
      // One line per transform axis under a component header, the field spanning the row: it is
      // disabled with a hover hint when the selection does not support the component; the undo mark
      // lands once per widget gesture (on the first actual edit, re-armed on deactivation)
      static const char* const compName[3] = { "Location", "Rotation", "Scale" };
      static const char* const headerName[3] = { "Position:", "Rotation:", "Scale:" };
      const auto trsRow = [this, &changed](const int comp, const int axis, const char* fmt, const float step, const float stepFast, float* v, const bool sup)
      {
         const float labelW = 16.f * m_editor.m_liveUI.GetDPI();
         ImGui::BeginDisabled(!sup);
         ImGui::AlignTextToFramePadding();
         ImGui::TextUnformatted(axis == 0 ? "X:" : axis == 1 ? "Y:" : "Z:");
         ImGui::SameLine(labelW);
         ImGui::SetNextItemWidth(-FLT_MIN);
         ImGui::PushID(comp * 3 + axis);
         if (ImGui::InputFloat("##f", v, step, stepFast, fmt))
            changed = true;
         if (ImGui::IsItemDeactivated())
            m_undoMarked = false;
         ImGui::PopID();
         ImGui::EndDisabled();
         if (!sup && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s %c is not supported by this selection", compName[comp], "XYZ"[axis]);
      };
      float trs[3][3] = {
         { ToDisplayLen(pos.x), ToDisplayLen(pos.y), ToDisplayLen(pos.z) },
         { rot.x, rot.y, rot.z },
         { scale.x, scale.y, scale.z },
      };
      for (int c = 0; c < 3; ++c)
      {
         ImGui::TextUnformatted(headerName[c]);
         for (int a = 0; a < 3; ++a)
            trsRow(c, a, fmts[c], steps[c], fastSteps[c], &trs[c][a], supported[c][a]);
      }
      // Each keystroke applies, so a non-positive scale typed on the way to a value ("0." for 0.5, a leading "-") is
      // skipped: its degenerate or mirrored matrix would read back as a wrong rotation, kept when the typing ends
      if (changed && trs[2][0] > 0.f && trs[2][1] > 0.f && trs[2][2] > 0.f)
      {
         if (!m_undoMarked)
         {
            MarkSelectionUndo(); // Parts still hold the pre-edit state: the mark snapshots it
            m_undoMarked = true;
         }
         pos.Set(ToVpuLen(trs[0][0]), ToVpuLen(trs[0][1]), ToVpuLen(trs[0][2]));
         rot.Set(trs[1][0], trs[1][1], trs[1][2]);
         scale.Set(trs[2][0], trs[2][1], trs[2][2]);
         // Recompose the transform the way the parts do (S * Rz * Ry * Rx * T, ExtractTRS's inverse)
         const Matrix3D t = Matrix3D::MatrixScale(scale.x, scale.y, scale.z) //
            * Matrix3D::MatrixRotateZ(ANGTORAD(rot.z)) * Matrix3D::MatrixRotateY(ANGTORAD(rot.y)) * Matrix3D::MatrixRotateX(ANGTORAD(rot.x)) * Matrix3D::MatrixTranslate(pos.x, pos.y, pos.z);
         editor.SetSelectionTransform(t);
      }
   }
   ImGui::EndDisabled();

   // Dimensions of the selection's bounds, editable like Blender's Dimensions fields. They are
   // measured along the axes of the transform the Scale fields edit (the active part's local frame,
   // or the world frame in point edit) so they stay coherent for rotated parts: changing a
   // dimension applies the ratio as a scale on that local axis, about the bounds center. Offered
   // only where the scale component is supported, disabled with a hint otherwise.
   FRect3D bounds;
   const bool hasBounds = editor.GetSelectionBounds(bounds);
   ImGui::BeginDisabled(!hasBounds || !editable);
   {
      ImGui::TextUnformatted("Dimensions:");
      // The transform the scaling delta is composed against: SetSelectionTransform evaluates its
      // delta from the active part's transform — or in drag point edit mode, from the gesture base
      // (which is re-armed to the current transform when the tracked point set no longer matches)
      Matrix3D transformBase = transform;
      if (editor.m_pointEditPart != nullptr && !editor.m_pointEditCenter)
      {
         bool gestureValid = editor.m_pointTransformSel == editor.m_pointSel && editor.m_pointTransformCur.size() == editor.m_pointSel.size();
         for (size_t i = 0; gestureValid && i < editor.m_pointSel.size(); ++i)
            gestureValid = editor.m_pointSel[i]->GetVertex() == editor.m_pointTransformCur[i];
         if (gestureValid)
            transformBase = editor.m_pointTransformBase;
      }
      // The selection's bounds in the base transform's local frame
      float extent[3] = { 0.f, 0.f, 0.f }, center[3] = { 0.f, 0.f, 0.f };
      if (hasBounds)
      {
         Matrix3D invBase(transformBase);
         invBase.Invert();
         FRect3D localBounds;
         localBounds.Clear();
         for (int cx = 0; cx < 2; ++cx)
            for (int cy = 0; cy < 2; ++cy)
               for (int cz = 0; cz < 2; ++cz)
               {
                  const Vertex3Ds p = invBase * Vertex3Ds(cx ? bounds.right : bounds.left, cy ? bounds.bottom : bounds.top, cz ? bounds.zhigh : bounds.zlow);
                  localBounds.Extend(FRect3D(p.x, p.x, p.y, p.y, p.z, p.z));
               }
         extent[0] = localBounds.right - localBounds.left;
         extent[1] = localBounds.bottom - localBounds.top;
         extent[2] = localBounds.zhigh - localBounds.zlow;
         center[0] = (localBounds.left + localBounds.right) * 0.5f;
         center[1] = (localBounds.top + localBounds.bottom) * 0.5f;
         center[2] = (localBounds.zlow + localBounds.zhigh) * 0.5f;
      }
      for (int a = 0; a < 3; ++a)
      {
         const bool sup = supported[2][a] && extent[a] > 1e-4f;
         const float labelW = 16.f * dpi;
         ImGui::BeginDisabled(!sup);
         ImGui::AlignTextToFramePadding();
         ImGui::TextUnformatted(a == 0 ? "X:" : a == 1 ? "Y:" : "Z:");
         ImGui::SameLine(labelW);
         ImGui::SetNextItemWidth(-FLT_MIN);
         ImGui::PushID(9 + a); // The field ids continue after the 9 transform fields
         float dim = ToDisplayLen(extent[a]);
         if (ImGui::InputFloat("##f", &dim, steps[0], fastSteps[0], locFmtS.c_str()))
         {
            const float f = ToVpuLen(dim) / extent[a];
            if (f > 0.f) // Guards against empty bounds and non-positive/zero input
            {
               if (!m_undoMarked)
               {
                  MarkSelectionUndo(); // Parts still hold the pre-edit state: the mark snapshots it
                  m_undoMarked = true;
               }
               const float s[3] = { a == 0 ? f : 1.f, a == 1 ? f : 1.f, a == 2 ? f : 1.f };
               // The scaling is applied in the base's local frame (pre-multiplied), so it lands on
               // the axis the field displays even for a rotated part
               const Matrix3D delta
                  = Matrix3D::MatrixTranslate(-center[0], -center[1], -center[2]) * Matrix3D::MatrixScale(s[0], s[1], s[2]) * Matrix3D::MatrixTranslate(center[0], center[1], center[2]);
               editor.SetSelectionTransform(delta * transformBase);
            }
         }
         if (ImGui::IsItemDeactivated())
            m_undoMarked = false;
         ImGui::PopID();
         ImGui::EndDisabled();
         if (!sup && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Scale %c is not supported by this selection", "XYZ"[a]);
      }
   }
   ImGui::EndDisabled();
   // Reference space shared by the selection (same info as the status overlay)
   if (const std::optional<PartGroupData::SpaceReference> space = editor.GetSelectionSpace(); space.has_value())
   {
      const char* const spaceNames[] = { "Playfield", "Cabinet", "Cabinet Feet", "Room" };
      ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::V(EditorTheme::TextDim));
      ImGui::Text("Space: %s", spaceNames[clamp(static_cast<int>(*space), 0, 3)]);
      ImGui::PopStyleColor();
   }
}

void ViewportSidebar::RenderToolTab()
{
   EditorUI& editor = m_editor;
   Settings& settings = g_settingsService.GetAppSettings();

   // Edit modes (same actions as the tool shelf buttons and the G / R / S keys): not offered in an
   // inspection session, where the gizmo would write to the running table's live copy
   ImGui::SeparatorText("Edit Mode");
   ImGui::BeginDisabled(editor.IsInspectMode());
   static const struct
   {
      ImGuizmo::OPERATION operation;
      const char* icon;
      const char* title;
   } modes[] = {
      { (ImGuizmo::OPERATION)0, ICON_LC_MOUSE_POINTER_2, "Select" },
      { ImGuizmo::TRANSLATE, ICON_LC_MOVE, "Move" },
      { ImGuizmo::ROTATE, ICON_LC_ROTATE_3D, "Rotate" },
      { ImGuizmo::SCALE, ICON_LC_SCALE_3D, "Scale" },
   };
   for (const auto& mode : modes)
   {
      const bool active = m_pane.m_gizmoOperation == mode.operation && !m_pane.m_measureMode;
      if (ImGui::Selectable((string(mode.icon) + "  " + mode.title).c_str(), active))
         m_pane.SetGizmoOperation(mode.operation);
   }
   {
      const bool active = m_pane.m_measureMode;
      if (ImGui::Selectable(ICON_LC_RULER "  Measure", active))
         m_pane.SetMeasureMode(!active);
   }

   // Orientation of the gizmo (Global / Local), meaningful for Grab, Rotate and Scale
   ImGui::SeparatorText("Orientation");
   {
      const bool local = m_pane.m_gizmoMode == ImGuizmo::LOCAL;
      ImGui::BeginDisabled(m_pane.m_gizmoOperation == ImGuizmo::OPERATION(0) && m_pane.m_op.op == ImGuizmo::OPERATION(0));
      if (ImGui::Selectable(local ? ICON_LC_BOX "  Local" : ICON_LC_GLOBE "  Global"))
         m_pane.m_gizmoMode = local ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
      ImGui::EndDisabled();
   }
   ImGui::EndDisabled();

   // Snapping options (same actions and settings writes as the Edit menu's Snap Options)
   ImGui::SeparatorText("Snapping");
   if (ImGui::Checkbox("Enabled", &editor.m_snapEnabled))
   {
      settings.SetEditor_SnapEnabled(editor.m_snapEnabled, false);
      editor.Report(editor.m_snapEnabled ? "Snapping on" : "Snapping off");
   }
   ImGui::TextUnformatted("Target");
   ImGui::SameLine(64.f * m_editor.m_liveUI.GetDPI());
   if (ImGui::RadioButton("Increment", editor.m_snapTarget == 0))
   {
      editor.m_snapTarget = 0;
      settings.SetEditor_SnapTarget(0, false);
   }
   ImGui::SameLine();
   if (ImGui::RadioButton("Points", editor.m_snapTarget == 1))
   {
      editor.m_snapTarget = 1;
      settings.SetEditor_SnapTarget(1, false);
   }
   ImGui::SetNextItemWidth(-FLT_MIN);
   if (ImGui::DragFloat("Move Increment", &editor.m_snapMove, 0.1f, 0.01f, 1000.f, "%.2f vpu"))
      settings.SetEditor_SnapMove(editor.m_snapMove, false);
   ImGui::SetNextItemWidth(-FLT_MIN);
   if (ImGui::DragFloat("Rotate Increment", &editor.m_snapRotate, 0.5f, 0.1f, 180.f, "%.1f deg"))
      settings.SetEditor_SnapRotate(editor.m_snapRotate, false);
   ImGui::SetNextItemWidth(-FLT_MIN);
   if (ImGui::DragFloat("Scale Increment", &editor.m_snapScale, 0.005f, 0.001f, 10.f, "%.3f"))
      settings.SetEditor_SnapScale(editor.m_snapScale, false);
   if (ImGui::Checkbox("Affect Moves", &editor.m_snapAffectMove))
      settings.SetEditor_SnapAffectMove(editor.m_snapAffectMove, false);
   if (ImGui::Checkbox("Affect Rotations", &editor.m_snapAffectRotate))
      settings.SetEditor_SnapAffectRotate(editor.m_snapAffectRotate, false);
   if (ImGui::Checkbox("Affect Scales", &editor.m_snapAffectScale))
      settings.SetEditor_SnapAffectScale(editor.m_snapAffectScale, false);

   // Last measure (stays until a new drag or a mode change)
   if (m_pane.m_measureMode && m_pane.m_measureValid)
   {
      ImGui::SeparatorText("Measure");
      const Vertex3Ds delta(m_pane.m_measureTo.x - m_pane.m_measureFrom.x, m_pane.m_measureTo.y - m_pane.m_measureFrom.y, m_pane.m_measureTo.z - m_pane.m_measureFrom.z);
      const float len = sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
      ImGui::Text("%.1f%s", (double)ToDisplayLen(len), LenUnitLabel());
      if (m_pane.m_measureAxis >= 0)
         ImGui::TextDisabled("Axis: %c", "XYZ"[clamp(m_pane.m_measureAxis, 0, 2)]);
   }
}

void ViewportSidebar::RenderViewTab()
{
   EditorUI& editor = m_editor;
   Settings& settings = g_settingsService.GetAppSettings();

   // The pane's own camera (same actions as its header buttons and the View menu)
   ImGui::SeparatorText("Camera");
   {
      ImGui::TextUnformatted("Projection");
      ImGui::SameLine(64.f * editor.m_liveUI.GetDPI());
      const char* proj = m_pane.m_perspectiveCam ? "Perspective" : "Orthographic";
      if (ImGui::BeginCombo("##proj", proj))
      {
         if (ImGui::Selectable("Perspective", m_pane.m_perspectiveCam))
            m_pane.SetPerspectiveCam(true);
         if (ImGui::Selectable("Orthographic", !m_pane.m_perspectiveCam))
            m_pane.SetPerspectiveCam(false);
         ImGui::EndCombo();
      }
   }
   {
      const bool editable = m_pane.m_perspectiveCam && m_pane.m_viewType == ViewMode::EditorCam;
      ImGui::BeginDisabled(!editable);
      ImGui::SetNextItemWidth(-FLT_MIN);
      float fov = editor.m_navFovY;
      if (ImGui::SliderFloat("Field Of View", &fov, 15.f, 100.f, "%.1f deg"))
      {
         m_pane.SetFov(fov);
         settings.SetEditor_NavFovY(editor.m_navFovY, false);
      }
      ImGui::EndDisabled();
   }
   ImGui::Checkbox("Lock Orbit", &m_pane.m_orbitLock);
   ImGui::Checkbox("Fly Mode (F)", &m_pane.m_flyMode);
   if (m_pane.m_viewType == ViewMode::EditorCam)
      ImGui::Checkbox("Space Axes", &m_pane.m_showSpaceAxes);

   ImGui::SeparatorText("Shading");
   static const struct
   {
      Renderer::ShadeMode mode;
      const char* label;
   } shadeModes[] = {
      { Renderer::ShadeMode::NoDepthWireframe, "Wireframe" },
      { Renderer::ShadeMode::Wireframe, "Solid" },
      { Renderer::ShadeMode::Default, "Rendered" },
   };
   for (const auto& mode : shadeModes)
   {
      if (ImGui::RadioButton(mode.label, m_pane.m_shadeMode == mode.mode))
         m_pane.m_shadeMode = mode.mode;
   }

   ImGui::SeparatorText("Frame");
   ImGui::BeginDisabled(!editor.CanFrameSelection());
   if (ImGui::Button("Frame Selected", ImVec2(-FLT_MIN, 0.f)))
      m_pane.FrameSelection();
   ImGui::EndDisabled();
   if (ImGui::Button("Frame All", ImVec2(-FLT_MIN, 0.f)))
      m_pane.FrameAll();
}

}
