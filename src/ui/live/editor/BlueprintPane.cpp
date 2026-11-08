// license:GPLv3+

#include "core/stdafx.h"

#include "BlueprintPane.h"

#include "../EditorUI.h"
#include "core/Settings.h"
#include "core/player.h"
#include "physics/PhysicsEngine.h"
#include "ui/live/LiveUI.h"
#include "EditorTheme.h"
#include "IconsLucide.h"
#include "fonts/IconsForkAwesome.h"
#include "OutlinerPanel.h"
#include "Selection.h"
#include "core/ieditable.h"
#include "math/matrix.h"
#include "parts/pintable.h"
#include "renderer/RenderDevice.h"
#include "utils/color.h"

namespace VPX::EditorUI
{

BlueprintPane::BlueprintPane(EditorUI& editor)
   : EditorAreaView(editor)
{
}

void BlueprintPane::OnUnhosted()
{
   if (m_editor.m_hoverBp == this)
      m_editor.m_hoverBp = nullptr;
   if (m_editor.m_captureBp == this)
      m_editor.m_captureBp = nullptr;
}

ImVec2 BlueprintPane::Project(const Vertex2D& v) const
{
   return ImVec2(0.5f * (m_imageRect.x0 + m_imageRect.x1) + (v.x - m_offset.x) * m_zoom, //
      0.5f * (m_imageRect.y0 + m_imageRect.y1) + (v.y - m_offset.y) * m_zoom);
}

Vertex2D BlueprintPane::Unproject(const ImVec2& pos) const
{
   const float invZoom = (m_zoom != 0.f) ? (1.f / m_zoom) : 0.f;
   return Vertex2D(m_offset.x + (pos.x - 0.5f * (m_imageRect.x0 + m_imageRect.x1)) * invZoom, //
      m_offset.y + (pos.y - 0.5f * (m_imageRect.y0 + m_imageRect.y1)) * invZoom);
}

void BlueprintPane::ZoomAt(const ImVec2& pos, const float factor)
{
   // Zoom toward the point under the mouse (it keeps its screen position), like the win32 view
   const Vertex2D anchor = Unproject(pos);
   m_zoom = clamp(m_zoom * factor, 0.1f, 64.f);
   m_offset.x = anchor.x - (pos.x - 0.5f * (m_imageRect.x0 + m_imageRect.x1)) / m_zoom;
   m_offset.y = anchor.y - (pos.y - 0.5f * (m_imageRect.y0 + m_imageRect.y1)) / m_zoom;
}

void BlueprintPane::FitTable()
{
   const PinTable* const table = m_editor.m_table;
   m_offset = Vertex2D(0.5f * (table->m_left + table->m_right), 0.5f * (table->m_top + table->m_bottom));
   const float w = table->m_right - table->m_left, h = table->m_bottom - table->m_top;
   if (m_imageRect.Width() > 0.f && m_imageRect.Height() > 0.f && w > 0.f && h > 0.f)
      m_zoom = 0.9f * min(m_imageRect.Width() / w, m_imageRect.Height() / h);
   m_setupValid = true;
}

void BlueprintPane::FrameSelection()
{
   if (m_editor.m_multiSel.empty())
      return;
   Vertex2D lo(FLT_MAX, FLT_MAX), hi(-FLT_MAX, -FLT_MAX);
   for (const auto& part : m_editor.m_multiSel)
   {
      Matrix3D transform;
      if (part->GetTransform(transform) == EditorUIPart::TM_None)
         continue;
      const Vertex3Ds pos = transform.GetOrthoNormalPos();
      lo.x = min(lo.x, pos.x);
      lo.y = min(lo.y, pos.y);
      hi.x = max(hi.x, pos.x);
      hi.y = max(hi.y, pos.y);
   }
   if (lo.x > hi.x)
      return;
   m_offset = Vertex2D(0.5f * (lo.x + hi.x), 0.5f * (lo.y + hi.y));
   const float w = max(hi.x - lo.x, 50.f), h = max(hi.y - lo.y, 50.f);
   if (m_imageRect.Width() > 0.f && m_imageRect.Height() > 0.f)
      m_zoom = clamp(0.8f * min(m_imageRect.Width() / w, m_imageRect.Height() / h), 0.1f, 64.f);
}

nlohmann::json BlueprintPane::SaveSetup() const
{
   nlohmann::json j;
   j["zoom"] = m_zoom;
   j["offx"] = m_offset.x;
   j["offy"] = m_offset.y;
   return j;
}

void BlueprintPane::LoadSetup(const nlohmann::json& j)
{
   if (!j.is_object())
      return;
   const float zoom = j.value("zoom", 0.f), offx = j.value("offx", 0.f), offy = j.value("offy", 0.f);
   if (!std::isfinite(zoom) || zoom <= 0.f || !std::isfinite(offx) || !std::isfinite(offy))
      return;
   m_zoom = zoom;
   m_offset = Vertex2D(offx, offy);
   m_setupValid = true;
}

void BlueprintPane::SetGizmoOperation(const ImGuizmo::OPERATION operation)
{
   if (operation == ImGuizmo::OPERATION(0))
   {
      m_gizmoOperation = operation;
      return;
   }
   // Repeating the same operation toggles between world and local coordinates (same as the viewport)
   m_gizmoMode = m_gizmoOperation == operation ? (m_gizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL) : ImGuizmo::WORLD;
   m_gizmoOperation = operation;
}

std::shared_ptr<EditorUIPart> BlueprintPane::Resolve(const EditorUIPart* const part) const
{
   for (const auto& uiPart : m_editor.m_editables)
      if (uiPart.get() == part)
         return uiPart;
   return nullptr;
}

void BlueprintPane::RenderPart(BlueprintRenderContext& ctx, const std::shared_ptr<EditorUIPart>& part) const
{
   ctx.m_partSelected = m_editor.IsPartSelected(part);
   ctx.m_partActive = m_editor.m_selection.GetType() == Selection::S_EDITABLE && m_editor.m_selection.GetPart() == part;
   ctx.m_partLocked = part->GetEditable()->IsUILocked();
   part->RenderBlueprint(ctx);
   ctx.SetObject(nullptr);
}

void BlueprintPane::RenderScene(ImDrawList* const drawList)
{
   if (m_imageRect.Width() <= 0.f || m_imageRect.Height() <= 0.f)
      return;
   PinTable* const table = m_editor.m_table;
   Settings& settings = g_settingsService.GetAppSettings();
   const float dpi = m_editor.m_liveUI.GetDPI();

   drawList->PushClipRect(ImVec2(m_imageRect.x0, m_imageRect.y0), ImVec2(m_imageRect.x1, m_imageRect.y1), true);

   BlueprintDrawContext ctx(m_editor.m_renderer->m_renderDevice, drawList);
   ctx.Setup(m_imageRect, m_zoom, m_offset, dpi);
   ctx.m_pointEditPart = m_editor.m_pointEditPart.get();
   ctx.m_selectedPoints = m_editor.m_pointEditPart ? &m_editor.m_pointSel : nullptr;
   ctx.m_hotPoint = m_dragPoint;

   // 1. Editor background over the whole canvas
   ctx.SetFillColor((int)settings.GetEditor_BackGroundColor());
   ctx.SetBorderColor(-1, false, 0);
   ctx.SetLineColor(-1, false, 0);
   ctx.Rectangle2(m_imageRect.x0, m_imageRect.y0, m_imageRect.x1, m_imageRect.y1);

   // 2. Optional backdrop image over the playfield bounds (same option as the win32 view)
   if (table->m_winEditorBackdrop)
   {
      if (Texture* const image = table->GetImage(table->m_image))
         ctx.Image(table->m_left, table->m_top, table->m_right, table->m_bottom, image);
   }

   // 3. Fill pass (win32 UIRenderPass1 of all parts, in table z-order)
   ctx.SetPass(BlueprintRenderContext::Pass::Fill);
   for (IEditable* const editable : table->GetParts())
   {
      if (editable->m_desktopBackdrop || !editable->IsUIVisible(false))
         continue;
      const auto it = m_editor.m_editableMap.find(editable);
      if (it != m_editor.m_editableMap.end())
         RenderPart(ctx, it->second);
   }
   RenderGhostParts(ctx);

   // 4. Grid (below the outlines, clipped to the playfield bounds)
   const float gridSize = settings.GetEditor_GridSize();
   if (table->m_winEditorGrid && gridSize > 0.f)
   {
      // Visible table rect clipped to the playfield bounds (same as the win32 view)
      const Vertex2D tl = Unproject(ImVec2(m_imageRect.x0, m_imageRect.y0));
      const Vertex2D br = Unproject(ImVec2(m_imageRect.x1, m_imageRect.y1));
      const Vertex2D lo(max(tl.x, table->m_left), max(tl.y, table->m_top));
      const Vertex2D hi(min(br.x, table->m_right), min(br.y, table->m_bottom));
      if (lo.x <= hi.x && lo.y <= hi.y)
      {
         ctx.SetObject(nullptr);
         ctx.SetLineColor(RGB(190, 220, 240), false, 0);
         for (float x = floorf(lo.x / gridSize) * gridSize; x <= hi.x; x += gridSize)
            ctx.Line(x, lo.y, x, hi.y);
         for (float y = floorf(lo.y / gridSize) * gridSize; y <= hi.y; y += gridSize)
            ctx.Line(lo.x, y, hi.x, y);
      }
   }

   // 5. Outline pass (win32 UIRenderPass2 of all parts)
   ctx.SetPass(BlueprintRenderContext::Pass::Outline);
   for (IEditable* const editable : table->GetParts())
   {
      if (editable->m_desktopBackdrop || !editable->IsUIVisible(false))
         continue;
      const auto it = m_editor.m_editableMap.find(editable);
      if (it != m_editor.m_editableMap.end())
         RenderPart(ctx, it->second);
   }
   RenderGhostParts(ctx);

   drawList->PopClipRect();
}

void BlueprintPane::RenderGhostParts(BlueprintRenderContext& ctx) const
{
   // Ghost preview of the armed part library placement (the part shapes in selection styling)
   if (!m_editor.m_partLibrary.IsPreviewVisible())
      return;
   for (const std::shared_ptr<EditorUIPart>& uiPart : m_editor.m_partLibrary.GetPreviewParts())
   {
      ctx.m_partSelected = true;
      ctx.m_partActive = false;
      ctx.m_partLocked = false;
      uiPart->RenderBlueprint(ctx);
   }
   ctx.SetObject(nullptr);
}

void BlueprintPane::HitTestParts(const ImVec2& pos, vector<BlueprintHitContext::Hit>& hits) const
{
   // Replay both passes through the hit recorder (same order as the win32 HitSur picking)
   BlueprintHitContext ctx;
   ctx.Setup(m_imageRect, m_zoom, m_offset, m_editor.m_liveUI.GetDPI());
   ctx.SetHitPoint(pos);
   ctx.m_pointEditPart = m_editor.m_pointEditPart.get();
   ctx.m_selectedPoints = m_editor.m_pointEditPart ? &m_editor.m_pointSel : nullptr;
   for (const BlueprintRenderContext::Pass pass : { BlueprintRenderContext::Pass::Fill, BlueprintRenderContext::Pass::Outline })
   {
      ctx.SetPass(pass);
      for (IEditable* const editable : m_editor.m_table->GetParts())
      {
         if (editable->m_desktopBackdrop || !editable->IsUIVisible(false) || !m_editor.IsEditablePickable(editable))
            continue;
         const auto it = m_editor.m_editableMap.find(editable);
         if (it != m_editor.m_editableMap.end())
            RenderPart(ctx, it->second);
      }
   }
   hits = ctx.GetHits();
}

void BlueprintPane::BoxSelectParts(const ImVec2& cornerA, const ImVec2& cornerB, const bool add)
{
   // Parts whose drawn shapes are fully inside the box (win32 HitRectSur semantics). The hit replay
   // runs before the selection is cleared, so the drag point handles of selected parts are recorded
   // too (the win32 band select picks the sub parts when the box holds no main part)
   BlueprintHitContext ctx;
   ctx.Setup(m_imageRect, m_zoom, m_offset, m_editor.m_liveUI.GetDPI());
   ctx.SetHitRect(AreaRect { min(cornerA.x, cornerB.x), min(cornerA.y, cornerB.y), max(cornerA.x, cornerB.x), max(cornerA.y, cornerB.y) });
   for (const BlueprintRenderContext::Pass pass : { BlueprintRenderContext::Pass::Fill, BlueprintRenderContext::Pass::Outline })
   {
      ctx.SetPass(pass);
      for (IEditable* const editable : m_editor.m_table->GetParts())
      {
         if (editable->m_desktopBackdrop || !editable->IsUIVisible(false) || !m_editor.IsEditablePickable(editable))
            continue;
         const auto it = m_editor.m_editableMap.find(editable);
         if (it != m_editor.m_editableMap.end())
            RenderPart(ctx, it->second);
      }
   }
   const vector<const EditorUIPart*>& parts = ctx.GetRectParts();
   if (parts.empty())
   {
      // No main part in the box: when it holds the drag point handles of a single curve part, its drag
      // point edit mode opens with the boxed points selected (the win32 sub part band select)
      const vector<const DragPoint*>& points = ctx.GetRectPoints();
      if (!points.empty() && !m_editor.IsInspectMode() && !m_editor.m_table->IsLocked())
      {
         std::shared_ptr<EditorUIPart> owner;
         bool singleOwner = true;
         for (const DragPoint* point : points)
         {
            const auto it = m_editor.m_editableMap.find(point->GetIEditable());
            if (it == m_editor.m_editableMap.end() || (owner != nullptr && owner != it->second))
            {
               singleOwner = false;
               break;
            }
            owner = it->second;
         }
         if (singleOwner && owner != nullptr && owner->GetDragPointCurve() != nullptr)
         {
            m_editor.SetSelection(Selection(owner));
            m_editor.EnterPointEditMode();
            if (m_editor.m_pointEditPart == owner)
               for (const DragPoint* point : points)
                  m_editor.m_pointSel.push_back(const_cast<DragPoint*>(point));
         }
      }
      return;
   }
   if (!add)
   {
      m_editor.m_multiSel.clear();
      m_editor.m_selection = Selection();
   }
   for (const EditorUIPart* const part : parts)
   {
      const std::shared_ptr<EditorUIPart> uiPart = Resolve(part);
      if (uiPart != nullptr && !m_editor.IsPartSelected(uiPart))
         m_editor.m_multiSel.push_back(uiPart);
   }
   if (!m_editor.m_multiSel.empty() && (m_editor.m_selection.GetType() != Selection::S_EDITABLE || !m_editor.IsPartSelected(m_editor.m_selection.GetPart())))
      m_editor.m_selection = Selection(m_editor.m_multiSel.back());
}

void BlueprintPane::BoxSelectPoints(const ImVec2& cornerA, const ImVec2& cornerB, const bool add)
{
   // Drag point handles of the edited curve fully inside the box (point edit mode)
   if (!add)
      m_editor.m_pointSel.clear();
   BlueprintHitContext ctx;
   ctx.Setup(m_imageRect, m_zoom, m_offset, m_editor.m_liveUI.GetDPI());
   ctx.SetHitRect(AreaRect { min(cornerA.x, cornerB.x), min(cornerA.y, cornerB.y), max(cornerA.x, cornerB.x), max(cornerA.y, cornerB.y) });
   ctx.m_pointEditPart = m_editor.m_pointEditPart.get();
   ctx.m_selectedPoints = &m_editor.m_pointSel;
   ctx.SetPass(BlueprintRenderContext::Pass::Outline);
   RenderPart(ctx, m_editor.m_pointEditPart);
   for (const DragPoint* const point : ctx.GetRectPoints())
      if (!m_editor.IsPointSelected(point))
         m_editor.m_pointSel.push_back(const_cast<DragPoint*>(point));
}

void BlueprintPane::UpdateDraggedPoint(const ImVec2& pos, const bool start)
{
   const Vertex2D newPos = Unproject(pos);
   const Vertex2D delta(newPos.x - m_dragPos.x, newPos.y - m_dragPos.y);
   if (start)
   {
      m_editor.m_undo.BeginUndo();
      m_editor.m_undo.MarkForUndo(m_dragPart->GetEditable());
      m_editor.m_undo.EndUndo();
   }
   if (delta.x == 0.f && delta.y == 0.f)
      return;
   if (m_dragCenter)
   {
      const Vertex3Ds center = m_dragPart->GetEditCenter();
      m_dragPart->SetEditCenter(Vertex2D(center.x + delta.x, center.y + delta.y));
   }
   else if (m_dragPoint != nullptr)
   {
      m_dragPoint->Translate(delta);
      if (DragPointCurve* const curve = m_dragPart->GetDragPointCurve())
         curve->OnPointsModified();
   }
   m_editor.m_renderer->ReinitRenderable(m_dragPart->GetEditable()->GetIRenderable());
   m_editor.m_player->m_physics->Update(m_dragPart->GetEditable());
   m_dragPos = newPos;
}

void BlueprintPane::RenderPane(const AreaRect& rect, const int leafId)
{
   EditorUI& editor = m_editor;
   m_leafId = leafId;

   char name[40];
   snprintf(name, sizeof(name), "##Pane%d", leafId);
   ImGui::SetNextWindowPos(ImVec2(rect.x0, rect.y0));
   ImGui::SetNextWindowSize(ImVec2(rect.Width(), rect.Height()));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
   ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
   ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
   // NoBackground like the 3D panes (the 2D canvas is drawn opaquely by RenderScene)
   ImGui::Begin(name, nullptr,
      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus
         | ImGuiWindowFlags_NoNavFocus);
   ImGui::PushID(leafId);
   m_window = ImGui::GetCurrentWindow();
   const float dpi = editor.m_liveUI.GetDPI();

   // Header bar: type switcher, edit modes, add part; right side: view options & fit (see ViewportPane)
   const float pad = 2.f * dpi;
   const ImVec2 winPos = ImGui::GetWindowPos();
   ImDrawList* const drawList = ImGui::GetWindowDrawList();
   drawList->ChannelsSplit(2);
   drawList->ChannelsSetCurrent(1);
   ImGui::SetCursorPos(ImVec2(pad, pad));

   editor.RenderAreaTypeButton(leafId);
   ImGui::SameLine();

   if (!editor.IsInspectMode())
   {
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

      // Add part button (same as the Shift+A shortcut)
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

   // Right side of the header: view options popover and fit view button
   const ImGuiStyle& style = ImGui::GetStyle();
   const float rightW = ImGui::CalcTextSize(ICON_FK_STICKY_NOTE, nullptr, true).x + style.FramePadding.x * 2.0f //
      + ImGui::CalcTextSize("Fit", nullptr, true).x + style.FramePadding.x * 2.0f //
      + style.ItemSpacing.x * 2.0f;
   const float rightX = rect.Width() - pad - rightW;
   ImGui::SameLine();
   if (rightX > ImGui::GetCursorPosX())
      ImGui::SetCursorPosX(rightX);
   if (ImGui::Button(ICON_FK_STICKY_NOTE)) // View options popover (shared with the win32 editor view)
      ImGui::OpenPopup("Overlay Popup");
   if (ImGui::BeginPopup("Overlay Popup"))
   {
      ImGui::TextUnformatted("Blueprint options:");
      ImGui::Separator();
      bool grid = editor.m_table->m_winEditorGrid;
      if (ImGui::Checkbox("Display grid", &grid))
         editor.m_table->m_winEditorGrid = grid;
      bool solid = editor.m_table->m_renderSolid;
      if (ImGui::Checkbox("Render solid", &solid))
      {
         editor.m_table->m_renderSolid = solid;
         g_settingsService.GetAppSettings().SetEditor_RenderSolid(solid, false);
      }
      bool backdrop = editor.m_table->m_winEditorBackdrop;
      if (ImGui::Checkbox("Display backdrop", &backdrop))
         editor.m_table->m_winEditorBackdrop = backdrop;
      ImGui::EndPopup();
   }
   ImGui::SameLine();
   if (ImGui::Button("Fit"))
      FitTable();
   if (ImGui::IsItemHovered())
      ImGui::SetTooltip("Fit view on the playfield\n[Home]");

   const float headerBottom = ImGui::GetItemRectMax().y + pad;
   drawList->ChannelsSetCurrent(0);
   drawList->AddRectFilled(winPos, ImVec2(winPos.x + rect.Width(), headerBottom), EditorTheme::Bar);
   drawList->AddLine(ImVec2(winPos.x, headerBottom - 0.5f), ImVec2(winPos.x + rect.Width(), headerBottom - 0.5f), EditorTheme::Border);
   drawList->ChannelsMerge();
   ImGui::SetCursorPosY(headerBottom - winPos.y);

   // Part type picker popup (header button and Shift+A shortcut)
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

   // Right mouse context menu & numeric transform / drawing order dialogs: opened one frame after the
   // request recorded by RenderUI or a menu command (popups must live in this window's scope)
   if (m_contextMenuPending)
   {
      m_contextMenuPending = false;
      ImGui::SetNextWindowPos(m_menuPos, ImGuiCond_Appearing);
      ImGui::OpenPopup("Blueprint Context");
   }
   RenderContextMenu();
   if (m_dialogRequest)
   {
      m_dialogRequest = false;
      m_dialogApplyCount = 0;
      m_dialogInit = false;
      const char* const popupName = m_dialog == DialogKind::Rotate ? "Rotate###BlueprintDialog"
         : m_dialog == DialogKind::Scale                           ? "Scale###BlueprintDialog"
         : m_dialog == DialogKind::Translate                       ? "Translate###BlueprintDialog"
                                                                   : "Drawing Order###BlueprintDialog";
      ImGui::OpenPopup(popupName);
   }
   RenderDialogs();

   // Canvas: the 2D schematic is drawn into the pane's window draw list
   const ImVec2 avail = ImGui::GetContentRegionAvail();
   const ImVec2 imgPos = ImGui::GetCursorScreenPos();
   m_imageRect = AreaRect { imgPos.x, imgPos.y, imgPos.x + avail.x, imgPos.y + avail.y };
   ImGui::InvisibleButton("##view", ImVec2(max(1.f, avail.x), max(1.f, avail.y)));
   m_viewItemId = ImGui::GetItemID();
   m_hovered = ImGui::IsItemHovered();
   m_active = ImGui::IsItemActive();

   RenderScene(drawList);

   // Status overlay text (view name, edit mode) at the top left of the canvas
   if (m_imageRect.Width() > 0.f && m_imageRect.Height() > 0.f)
   {
      const ImVec2 overlayPos(m_imageRect.x0 + 5.f * dpi, m_imageRect.y0 + 5.f * dpi);
      ImGui::SetNextWindowSize(ImVec2(max(1.f, m_imageRect.x1 - overlayPos.x), max(1.f, m_imageRect.y1 - overlayPos.y)));
      ImGui::SetNextWindowPos(overlayPos);
      ImGui::Begin("text overlay", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav);
      ImGui::Text("Blueprint (x%.1f)", m_zoom);
      if (m_hovered)
      {
         // Cursor position in table coordinates (the win32 status bar position readout)
         const Vertex2D cur = Unproject(ImGui::GetMousePos());
         ImGui::Text("%.1f, %.1f", cur.x, cur.y);
      }
      if (m_gizmoOperation == ImGuizmo::TRANSLATE)
         ImGui::TextUnformatted(ICON_FK_ARROWS " Grab");
      else if (m_gizmoOperation == ImGuizmo::SCALE)
         ImGui::TextUnformatted(ICON_FK_EXPAND " Scale");
      else if (m_gizmoOperation == ImGuizmo::ROTATE)
         ImGui::TextUnformatted(ICON_FK_REPEAT " Rotate");
      if (editor.GetTargetBlueprint() == this && editor.m_pointEditPart)
         ImGui::TextUnformatted(editor.m_pointEditCenter ? "Center Edit (Tab to exit)" : "Drag Point Edit (Tab to exit)");
      ImGui::End();
   }

   ImGui::GetWindowDrawList()->AddRect(m_window->Pos, m_window->Pos + m_window->Size, EditorTheme::Border, 3.f * dpi);
   ImGui::PopID();
   ImGui::End();
   ImGui::PopStyleVar(3);
}

void BlueprintPane::RenderUI(ImDrawList* const overlayDrawList)
{
   if (!m_setupValid)
      FitTable();
   if (m_imageRect.Width() <= 0.f || m_imageRect.Height() <= 0.f)
      return;

   EditorUI& editor = m_editor;
   const ImGuiIO& io = ImGui::GetIO();
   const float dpi = editor.m_liveUI.GetDPI();

   // Selection gizmo over the pane's 2D orthographic mapping (XY plane ops only)
   ImGuizmo::SetOrthographic(true);
   ImGuizmo::SetRect(m_imageRect.x0, m_imageRect.y0, m_imageRect.Width(), m_imageRect.Height());
   ImGuizmo::SetAlternativeWindow(m_window);

   Matrix3D transform;
   const bool isSelectionTransformValid = editor.GetSelectionTransform(transform);
   if (isSelectionTransformValid && !editor.m_table->IsLocked())
   {
      const float w2 = m_imageRect.Width() * 0.5f / m_zoom, h2 = m_imageRect.Height() * 0.5f / m_zoom;
      Matrix3D view = Matrix3D::MatrixLookAtRH(vec3(m_offset.x, m_offset.y, -1000.f), vec3(m_offset.x, m_offset.y, 0.f), vec3(0.f, -1.f, 0.f));
      Matrix3D proj = Matrix3D::MatrixOrthoOffCenterRH(-w2, w2, -h2, h2, 0.5f, -50000.f);
      ImGuizmo::OPERATION gizmoOperation = m_gizmoOperation;
      if (gizmoOperation == ImGuizmo::TRANSLATE)
         gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::TRANSLATE_X | ImGuizmo::TRANSLATE_Y);
      else if (gizmoOperation == ImGuizmo::ROTATE)
         gizmoOperation = ImGuizmo::ROTATE_Z;
      else if (gizmoOperation == ImGuizmo::SCALE)
         gizmoOperation = static_cast<ImGuizmo::OPERATION>(ImGuizmo::SCALE_X | ImGuizmo::SCALE_Y);
      const bool gizmoLost = m_gizmoDragActive && editor.m_multiSel != m_gizmoDragSelection;
      if (gizmoLost)
      {
         ImGuizmo::Enable(false);
         ImGuizmo::Enable(true);
      }
      else
      {
         const Matrix3D prevTransform(transform);
         ImGuizmo::Manipulate((float*)(view.m), (float*)(proj.m), gizmoOperation, m_gizmoMode, (float*)(transform.m));
         if (memcmp(transform.m, prevTransform.m, 16 * sizeof(float)) != 0)
         {
            if (!m_gizmoDragActive && editor.m_table->m_liveBaseTable == nullptr)
            {
               m_gizmoDragActive = true;
               m_gizmoDragSelection = editor.m_multiSel;
               editor.m_undo.BeginUndo();
               for (const auto& part : editor.m_multiSel)
                  editor.m_undo.MarkForUndo(part->GetEditable());
               editor.m_undo.EndUndo();
               editor.m_lastUndoPart = nullptr;
               editor.m_lastUndoId = 0x1000;
            }
            editor.SetSelectionTransform(transform);
         }
      }
   }
   else if (ImGuizmo::IsUsingAny())
   {
      ImGuizmo::Enable(false);
      ImGuizmo::Enable(true);
   }
   if (editor.m_lastUndoId == 0x1000 && !ImGuizmo::IsUsing())
   {
      m_gizmoDragActive = false;
      m_gizmoDragSelection.clear();
      editor.m_lastUndoPart = nullptr;
      editor.m_lastUndoId = 0;
   }

   // Overlays: selection pivot dot + the box select rect (the rest is inside the blueprint passes)
   overlayDrawList->PushClipRect(ImVec2(m_imageRect.x0, m_imageRect.y0), ImVec2(m_imageRect.x1, m_imageRect.y1), true);
   if (isSelectionTransformValid)
   {
      const Vertex3Ds pivot = transform.GetOrthoNormalPos();
      overlayDrawList->AddCircleFilled(Project(Vertex2D(pivot.x, pivot.y)), 3.f * dpi, IM_COL32(255, 255, 255, 255), 16);
   }
   if (m_boxSelectActive && ImGui::IsMouseDown(ImGuiMouseButton_Left))
   {
      const ImVec2 end = ImGui::GetMousePos();
      if (fabsf(end.x - m_boxSelectStart.x) > 2.f || fabsf(end.y - m_boxSelectStart.y) > 2.f)
      {
         overlayDrawList->AddRectFilled(m_boxSelectStart, end, IM_COL32(255, 128, 0, 32));
         overlayDrawList->AddRect(m_boxSelectStart, end, IM_COL32(255, 128, 0, 255));
      }
   }
   overlayDrawList->PopClipRect();

   // Scene input (the pane under the mouse, or capturing a drag, owns it)
   const bool input = m_hovered || m_active;
   if (input && editor.m_fileOp == nullptr)
   {
      const LiveUI& liveUI = editor.m_liveUI;
      if (io.MouseWheel != 0.f && (!liveUI.IsTrackpadScroll() || io.KeyCtrl))
         ZoomAt(ImGui::GetMousePos(), powf(1.1f, io.MouseWheel));
      else if (liveUI.IsTrackpadScroll())
      {
         m_offset.x -= io.MouseWheelH * 10.f / m_zoom;
         m_offset.y -= io.MouseWheel * 10.f / m_zoom;
      }
      if (liveUI.GetPinchScale() != 1.f && liveUI.GetPinchScale() > 0.f)
         ZoomAt(ImGui::GetMousePos(), liveUI.GetPinchScale());

      if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
      {
         const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
         ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
         m_offset.x -= drag.x / m_zoom;
         m_offset.y -= drag.y / m_zoom;
      }

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver())
      {
         m_boxSelectStart = ImGui::GetMousePos();
         if (editor.m_partLibrary.IsPlacementArmed())
            editor.m_partLibrary.CommitPlacement();
         else if (editor.m_addPartType != eItemInvalid)
         {
            const ItemTypeEnum type = editor.m_addPartType;
            editor.m_addPartType = eItemInvalid;
            editor.CreatePart(type, Unproject(m_boxSelectStart));
         }
         else if (editor.m_pointEditPart != nullptr && editor.m_pointEditCenter)
         {
            // Center point edit mode: click on the center selects it and arms a drag
            const ImVec2 center = Project(Vertex2D(editor.m_pointEditPart->GetEditCenter().x, editor.m_pointEditPart->GetEditCenter().y));
            const float dx = m_boxSelectStart.x - center.x, dy = m_boxSelectStart.y - center.y;
            if (dx * dx + dy * dy <= 100.f * dpi * dpi)
            {
               editor.m_centerSelected = io.KeyShift ? !editor.m_centerSelected : true;
               editor.m_pointDragPending = editor.m_centerSelected;
               editor.m_pointDragZ = editor.m_pointEditPart->GetEditCenter().z;
               editor.m_pointDragPos = Unproject(m_boxSelectStart);
            }
            else
               m_boxSelectActive = true;
         }
         else
         {
            vector<BlueprintHitContext::Hit> hits;
            HitTestParts(m_boxSelectStart, hits);
            if (editor.m_pointEditPart != nullptr)
            {
               // Drag point edit mode: hits are restricted to the edited part's curve points
               DragPoint* hitPoint = nullptr;
               for (const auto& hit : hits)
                  if (hit.point != nullptr && hit.part == editor.m_pointEditPart.get())
                     hitPoint = const_cast<DragPoint*>(hit.point);
               if (hitPoint != nullptr)
               {
                  if (io.KeyShift)
                     editor.TogglePointSelection(hitPoint);
                  else if (!editor.IsPointSelected(hitPoint))
                  {
                     editor.m_pointSel.clear();
                     editor.m_pointSel.push_back(hitPoint);
                  }
                  editor.m_pointDragPending = editor.IsPointSelected(hitPoint) && !hitPoint->m_uiLocked;
                  editor.m_pointDragZ = editor.m_pointEditPart->GetDragPointZ(hitPoint);
                  editor.m_pointDragPos = Unproject(m_boxSelectStart);
               }
               else
                  m_boxSelectActive = true;
            }
            else if (!hits.empty())
            {
               const BlueprintHitContext::Hit& top = hits.back();
               if (top.point != nullptr && !top.point->m_uiLocked)
               {
                  // Direct drag of a drag point handle of a selected part (win32 behavior)
                  m_dragPart = Resolve(top.part);
                  m_dragPoint = const_cast<DragPoint*>(top.point);
                  m_dragCenter = false;
                  m_dragPos = Unproject(m_boxSelectStart);
                  m_pointDragPending = (m_dragPart != nullptr);
                  m_pointDragActive = false;
               }
               else
               {
                  // Parts: the click selection is applied on mouse down (win32 semantics), so a
                  // press-drag on a part selects & moves it in one gesture. Repeated clicks cycle
                  // through the stacked parts while the selection holds a single part of the stack.
                  vector<std::shared_ptr<EditorUIPart>> hitParts; // Front to back
                  for (auto it = hits.rbegin(); it != hits.rend(); ++it)
                     if (it->point == nullptr)
                        if (const std::shared_ptr<EditorUIPart> uiPart = Resolve(it->part); uiPart != nullptr && std::ranges::find(hitParts, uiPart) == hitParts.end())
                           hitParts.push_back(uiPart);
                  if (hitParts.empty())
                     m_boxSelectActive = true;
                  else
                  {
                     const std::shared_ptr<EditorUIPart>& frontPart = hitParts.front();
                     if (io.KeyShift)
                     {
                        editor.TogglePartSelection(frontPart);
                        OutlinerPanel::m_anchor = frontPart;
                     }
                     else if (editor.m_multiSel.size() <= 1)
                     {
                        // Single selection: selects the topmost part, cycling to the parts behind it
                        // on repeated clicks while the active part is in the stack (win32 behavior)
                        size_t selectionIndex = 0;
                        const auto selIt = std::ranges::find(hitParts, editor.m_selection.GetPart());
                        if (selIt != hitParts.end())
                           selectionIndex = (static_cast<size_t>(selIt - hitParts.begin()) + 1) % hitParts.size();
                        editor.SetSelection(Selection(hitParts[selectionIndex]));
                        OutlinerPanel::m_anchor = hitParts[selectionIndex];
                     }
                     else if (editor.IsPartSelected(frontPart))
                     {
                        // Multi selection: a press on a member promotes it to the active part
                        editor.m_selection = Selection(frontPart);
                        OutlinerPanel::m_anchor = frontPart;
                     }
                     else
                     {
                        editor.SetSelection(Selection(frontPart));
                        OutlinerPanel::m_anchor = frontPart;
                     }
                     // The resulting selection may start a move drag (locked parts only select)
                     const std::shared_ptr<EditorUIPart>& sel = editor.m_selection.GetPart();
                     if (sel != nullptr && !sel->GetEditable()->IsUILocked())
                     {
                        m_moveDragPending = true;
                        m_moveDragActive = false;
                        m_moveDragPos = Unproject(m_boxSelectStart);
                     }
                  }
               }
            }
            else
               m_boxSelectActive = true;
         }
      }

      // In-progress drags: move selection, drag point/center, box select
      if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
      {
         const ImVec2 mouse = ImGui::GetMousePos();
         if (m_moveDragPending || m_moveDragActive)
         {
            if (m_moveDragPending)
            {
               const ImVec2 dd = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
               if (fabsf(dd.x) > 2.f || fabsf(dd.y) > 2.f)
               {
                  m_moveDragPending = false;
                  m_moveDragActive = true;
                  editor.m_undo.BeginUndo();
                  for (const auto& part : editor.m_multiSel)
                     editor.m_undo.MarkForUndo(part->GetEditable());
                  editor.m_undo.EndUndo();
               }
            }
            if (m_moveDragActive)
            {
               const Vertex2D pos = Unproject(mouse);
               const Vertex2D delta(pos.x - m_moveDragPos.x, pos.y - m_moveDragPos.y);
               if (delta.x != 0.f || delta.y != 0.f)
               {
                  for (const auto& part : editor.m_multiSel)
                  {
                     IEditable* const editable = part->GetEditable();
                     if (editable->IsUILocked())
                        continue;
                     editable->Translate(delta);
                     editor.m_renderer->ReinitRenderable(editable->GetIRenderable());
                     editor.m_player->m_physics->Update(editable);
                  }
                  m_moveDragPos = pos;
               }
            }
         }
         else if (m_pointDragPending || m_pointDragActive)
         {
            // Direct drag of a drag point handle or an edit center (outside point edit mode)
            if (m_pointDragPending)
            {
               const ImVec2 dd = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
               if (fabsf(dd.x) > 2.f || fabsf(dd.y) > 2.f)
               {
                  m_pointDragPending = false;
                  m_pointDragActive = (m_dragPart != nullptr);
                  if (m_pointDragActive)
                     UpdateDraggedPoint(mouse, true);
               }
            }
            if (m_pointDragActive && m_dragPart != nullptr)
               UpdateDraggedPoint(mouse, false);
         }
         else if (editor.m_pointDragPending || editor.m_pointDragActive)
         {
            // Point edit mode drags (the editor owns m_pointSel / the center point)
            const Vertex2D pos = Unproject(mouse);
            const Vertex2D delta(pos.x - editor.m_pointDragPos.x, pos.y - editor.m_pointDragPos.y);
            if (editor.m_pointDragPending)
            {
               const ImVec2 dd = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
               if (fabsf(dd.x) > 2.f || fabsf(dd.y) > 2.f)
               {
                  editor.m_pointDragPending = false;
                  editor.m_pointDragActive = editor.m_pointEditCenter ? editor.m_centerSelected : !editor.m_pointSel.empty();
                  if (editor.m_pointDragActive)
                  {
                     editor.m_undo.BeginUndo();
                     editor.m_undo.MarkForUndo(editor.m_pointEditPart->GetEditable());
                     editor.m_undo.EndUndo();
                  }
               }
            }
            if (editor.m_pointDragActive && (delta.x != 0.f || delta.y != 0.f))
            {
               if (editor.m_pointEditCenter)
               {
                  const Vertex3Ds center = editor.m_pointEditPart->GetEditCenter();
                  editor.m_pointEditPart->SetEditCenter(Vertex2D(center.x + delta.x, center.y + delta.y));
               }
               else
               {
                  DragPointCurve* const curve = editor.m_pointEditPart->GetDragPointCurve();
                  for (DragPoint* point : editor.m_pointSel)
                     point->Translate(delta);
                  curve->OnPointsModified();
               }
               editor.m_renderer->ReinitRenderable(editor.m_pointEditPart->GetEditable()->GetIRenderable());
               editor.m_player->m_physics->Update(editor.m_pointEditPart->GetEditable());
            }
            editor.m_pointDragPos = pos;
         }
      }
      else
      {
         // Button released: commit the box selection or apply the click selection
         if (m_moveDragPending || m_moveDragActive || m_pointDragPending || m_pointDragActive)
         {
            m_moveDragPending = m_moveDragActive = m_pointDragPending = m_pointDragActive = false;
            m_dragPart.reset();
            m_dragPoint = nullptr;
         }
         else if (m_boxSelectActive)
         {
            m_boxSelectActive = false;
            const ImVec2 end = ImGui::GetMousePos();
            if (editor.m_pointEditPart)
            {
               if (fabsf(end.x - m_boxSelectStart.x) > 4.f || fabsf(end.y - m_boxSelectStart.y) > 4.f)
                  BoxSelectPoints(m_boxSelectStart, end, io.KeyShift);
               else if (!io.KeyShift)
               {
                  editor.m_pointSel.clear();
                  editor.m_centerSelected = false;
               }
            }
            else if (fabsf(end.x - m_boxSelectStart.x) > 4.f || fabsf(end.y - m_boxSelectStart.y) > 4.f)
               BoxSelectParts(m_boxSelectStart, end, io.KeyShift);
            else if (!io.KeyShift)
               editor.ClearSelection(); // The click selection was applied on press (win32 semantics): an empty click clears
         }
      }

      // Right mouse button: completes pending left drags (win32 unstick), applies the win32 right
      // click selection, and records the context menu target (the popup is opened on release)
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      {
         m_moveDragPending = m_moveDragActive = m_pointDragPending = m_pointDragActive = false;
         m_dragPart.reset();
         m_dragPoint = nullptr;
         m_boxSelectActive = false;
         editor.m_pointDragPending = false;
         editor.m_pointDragActive = false;
         m_menuArmed = true;
         m_menuPos = ImGui::GetMousePos();
         HitTestParts(m_menuPos, m_menuHits);
         m_menuPoint = nullptr;
         m_menuPointPart = nullptr;
         if (!m_menuHits.empty() && m_menuHits.back().point != nullptr) // Topmost hit: drag point handle
         {
            m_menuPoint = m_menuHits.back().point;
            m_menuPointPart = m_menuHits.back().part;
         }
         else
         {
            // Win32 right-down selection: a selected part under the cursor keeps the multi selection
            // and becomes the active one, otherwise the topmost hit part is selected
            const vector<std::shared_ptr<EditorUIPart>> hitParts = MenuHitParts();
            bool promoted = false;
            for (const std::shared_ptr<EditorUIPart>& part : hitParts)
               if (editor.IsPartSelected(part))
               {
                  editor.m_selection = Selection(part);
                  OutlinerPanel::m_anchor = part;
                  promoted = true;
                  break;
               }
            if (!promoted)
            {
               if (!hitParts.empty())
               {
                  editor.SetSelection(Selection(hitParts.front()));
                  OutlinerPanel::m_anchor = hitParts.front();
               }
               else
                  editor.ClearSelection();
            }
         }
      }
      if (m_menuArmed && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
      {
         m_menuArmed = false;
         m_contextMenuPending = true;
      }
   }

   // Released outside of the pane (drags would otherwise stay armed until the mouse comes back)
   if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
      m_menuArmed = false;
   if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
   {
      m_boxSelectActive = false;
      if (!m_pointDragActive && !m_pointDragPending)
      {
         m_dragPart.reset();
         m_dragPoint = nullptr;
      }
      m_moveDragPending = false;
      m_pointDragPending = false;
      editor.m_pointDragPending = false;
      editor.m_pointDragActive = false;
   }

   // Pane keyboard actions: gizmo ops, drag point edit, mode cancels
   if (editor.GetTargetBlueprint() == this && !io.WantCaptureKeyboard && editor.m_fileOp == nullptr)
   {
      if (ImGui::IsKeyReleased(ImGuiKey_Escape))
      {
         if (editor.m_suppressNextEscape)
            editor.m_suppressNextEscape = false;
         else if (editor.ConsumedAreaEscape())
         {
         }
         else if (m_gizmoOperation != ImGuizmo::OPERATION(0))
            m_gizmoOperation = ImGuizmo::OPERATION(0);
         else if (m_boxSelectActive)
            m_boxSelectActive = false;
         else if (editor.m_addPartType != eItemInvalid)
            editor.m_addPartType = eItemInvalid;
         else if (editor.m_partLibrary.IsPlacementArmed())
            editor.m_partLibrary.CancelPlacement();
         else if (editor.m_pointEditPart)
            editor.ExitPointEditMode(true);
         else if (editor.m_selection.GetType() != Selection::S_NONE)
            editor.ClearSelection();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_Tab, false) && !io.KeyCtrl && !io.KeyAlt && !io.KeyShift)
      {
         if (editor.m_pointEditPart)
         {
            if (!editor.m_pointEditCenter && editor.m_pointEditPart->HasEditCenter())
            {
               editor.m_pointEditCenter = true;
               editor.m_centerSelected = true;
               editor.m_pointSel.clear();
               editor.m_pointDragPending = false;
               editor.m_pointDragActive = false;
            }
            else
               editor.ExitPointEditMode(true);
         }
         else
            editor.EnterPointEditMode();
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_A) && io.KeyShift && !io.KeyCtrl && !io.KeyAlt)
      {
         if (editor.m_pointEditPart && !editor.m_pointEditCenter && !io.WantCaptureMouse)
            editor.AddPointOnNearestSegment();
         else if (!editor.m_pointEditPart && !editor.IsInspectMode() && !editor.m_table->IsLocked())
            editor.RequestAddPartPopup(ImGui::GetMousePos());
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_G))
      {
         if (io.KeyAlt && !editor.m_pointEditPart)
         {
            Matrix3D tmp;
            if (editor.GetSelectionTransform(tmp))
               editor.SetSelectionTransform(tmp, true, false, false);
         }
         else if (!io.KeyCtrl)
            SetGizmoOperation(ImGuizmo::TRANSLATE);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_S))
      {
         if (!io.KeyCtrl)
         {
            if (io.KeyAlt && !editor.m_pointEditPart)
            {
               Matrix3D tmp;
               if (editor.GetSelectionTransform(tmp))
                  editor.SetSelectionTransform(tmp, false, true, false);
            }
            else
               SetGizmoOperation(ImGuizmo::SCALE);
         }
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_R))
      {
         if (io.KeyAlt && !editor.m_pointEditPart)
         {
            Matrix3D tmp;
            if (editor.GetSelectionTransform(tmp))
               editor.SetSelectionTransform(tmp, false, false, true);
         }
         else if (!io.KeyCtrl)
            SetGizmoOperation(ImGuizmo::ROTATE);
      }
      else if (ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal))
         FrameSelection();
      else if (ImGui::IsKeyPressed(ImGuiKey_Home))
         FitTable();
      else if (!io.WantTextInput
         && (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, true) || ImGui::IsKeyPressed(ImGuiKey_RightArrow, true) || ImGui::IsKeyPressed(ImGuiKey_UpArrow, true)
            || ImGui::IsKeyPressed(ImGuiKey_DownArrow, true)))
      {
         // Nudge the selection by 1 display px (10 with Shift), like the win32 arrow keys
         const float distance = (io.KeyShift ? 10.f : 1.f) / m_zoom;
         const Vertex2D delta(ImGui::IsKeyDown(ImGuiKey_RightArrow) ? distance : (ImGui::IsKeyDown(ImGuiKey_LeftArrow) ? -distance : 0.f),
            ImGui::IsKeyDown(ImGuiKey_DownArrow) ? distance : (ImGui::IsKeyDown(ImGuiKey_UpArrow) ? -distance : 0.f));
         editor.TranslateMultiSel(delta);
      }
   }
}

void BlueprintPane::AppendStatusHints(vector<StatusHint>& hints) const
{
   if (m_moveDragActive)
      hints = { { "Drag", "Move" }, { "Release " ICON_LC_MOUSE_LEFT, "Confirm" } };
   else if (m_pointDragActive)
      hints = { { "Drag", "Move Point" }, { "Release " ICON_LC_MOUSE_LEFT, "Confirm" } };
   else if (m_gizmoDragActive)
      hints = { { "Drag", "Transform" }, { "Release " ICON_LC_MOUSE_LEFT, "Confirm" } };
   else if (m_boxSelectActive)
      hints = { { "Release " ICON_LC_MOUSE_LEFT, "Select" }, { "Shift", "Add to Selection" }, { "Esc", "Cancel" } };
   else if (m_menuArmed)
      hints = { { "Release " ICON_LC_MOUSE_RIGHT, "Context Menu" } };
   else if (m_editor.m_addPartType != eItemInvalid || m_editor.m_partLibrary.IsPlacementArmed())
      hints = { { ICON_LC_MOUSE_LEFT, "Place Part" }, { "Esc", "Cancel" } };
   else if (m_editor.m_pointEditPart)
   {
      if (m_editor.m_pointEditCenter)
         hints = { { ICON_LC_MOUSE_LEFT, "Select Center" }, { "Shift+" ICON_LC_MOUSE_LEFT, "Toggle" }, { "G", "Move" }, { "Tab", "Exit" } };
      else
         hints = { { ICON_LC_MOUSE_LEFT, "Select Point" }, { "Shift+" ICON_LC_MOUSE_LEFT, "Toggle" }, { "G", "Move" }, { "Shift+A", "Add Point" }, { "Del", "Delete" }, { "Tab", "Exit" } };
   }
   else
   {
      hints = { { ICON_LC_MOUSE_LEFT, "Select" }, { "Drag " ICON_LC_MOUSE_LEFT, "Move" }, { ICON_LC_MOUSE_RIGHT, "Menu" }, { ICON_LC_MOUSE, "Pan" }, { "Wheel", "Zoom" },
         { "Arrows", "Nudge" } };
      if (!m_editor.m_table->IsLocked())
      {
         hints.push_back({ "G/S/R", "Gizmo Op" });
         if (!m_editor.IsInspectMode())
         {
            hints.push_back({ "Shift+A", "Add Part" });
            const std::shared_ptr<EditorUIPart>& part = m_editor.m_selection.GetPart();
            if (m_editor.m_selection.GetType() == Selection::S_EDITABLE && part != nullptr && (part->GetDragPointCurve() != nullptr || part->HasEditCenter()))
               hints.push_back({ "Tab", "Edit Points" });
         }
      }
      hints.push_back({ "Numpad .", "Frame" });
      hints.push_back({ "Home", "Fit" });
   }
}
}
