// license:GPLv3+

#include "core/stdafx.h"
#include "CurveEditOp.h"
#include "ui/live/editor/parts/EditorUIPart.h"
#include "ui/live/EditorUI.h"
#include "imgui/imgui_internal.h"

namespace VPX::EditorUI
{

vector<CurveEditOp::Tool *> &CurveEditOp::Tools()
{
   static vector<Tool *> tools; // Function scope: the tools register from static initializers of other files
   return tools;
}

bool CurveEditOp::Register(Tool *tool)
{
   vector<Tool *> &tools = Tools();
   // Sorted by name, the static initialization order of the registering files is unspecified
   tools.insert(std::ranges::upper_bound(tools, string(tool->GetName()), {}, [](const Tool *t) { return string(t->GetName()); }), tool);
   return true;
}

const vector<CurveEditOp::Tool *> &CurveEditOp::GetTools() { return Tools(); }

bool CurveEditOp::IsClosed(const EditorUIPart &part) { return part.GetEditable()->GetItemType() != eItemRamp; }

bool CurveEditOp::CanApply(const EditorUI &ed)
{
   if (ed.m_pointEditPart == nullptr || ed.m_pointEditCenter || ed.m_table->IsLocked() || ed.IsInspectMode() || ed.m_pointEditPart->GetEditable()->IsUILocked()
      || ed.m_pointEditPart->GetDragPointCurve() == nullptr || ed.IsDragInProgress())
      return false;
   return std::ranges::any_of(ed.m_pointSel, [](const DragPoint *point) { return !point->m_uiLocked; });
}

void CurveEditOp::Apply(EditorUI &ed, Tool &tool)
{
   if (!CanApply(ed))
   {
      if (ed.m_pointEditPart != nullptr && !ed.m_pointEditCenter && !ed.IsDragInProgress())
         ed.Report(string(tool.GetName()) + ": select unlocked points of an unlocked part (table not locked, not inspecting the game)");
      return;
   }
   const DragPointCurve *const curve = ed.m_pointEditPart->GetDragPointCurve();
   vector<PointState> points;
   vector<int> sel;
   for (const auto &point : curve->GetPoints())
   {
      if (ed.IsPointSelected(point.get()) && !point->m_uiLocked)
         sel.push_back(static_cast<int>(points.size()));
      points.push_back(
         { point->GetVertex(), point->GetTextureCoordinateU(), point->IsSmooth(), point->m_slingshot, point->IsAutoTextureCoordinate(), point->m_uiLocked, point->m_uiVisible });
   }
   const bool closed = IsClosed(*ed.m_pointEditPart);
   string error;
   vector<PointState> out;
   vector<int> outSel;
   if (!tool.Start(*ed.m_pointEditPart, points, sel, closed, error) || !tool.Generate(*ed.m_pointEditPart, points, sel, closed, out, outSel))
   {
      ed.Report(string(tool.GetName()) + ": " + (error.empty() ? "nothing to do on the selected points" : error));
      return;
   }
   ed.m_undo.BeginUndo();
   ed.m_undo.MarkForUndo(ed.m_pointEditPart->GetEditable());
   ed.m_undo.SetLabel(string(tool.GetName()) + ": " + ed.m_pointEditPart->GetEditable()->GetName());
   ed.m_undo.EndUndo();
   ed.SetLastOperation(EditorUI::LastOperation::Type::CurveEdit, Matrix3D::MatrixIdentity());
   EditorUI::LastOperation &op = ed.m_lastOp;
   op.tool = &tool;
   op.pointPart = ed.m_pointEditPart;
   op.startPoints = std::move(points);
   op.startSel = std::move(sel);
   Rebuild(ed);
}

void CurveEditOp::Reapply(EditorUI &ed)
{
   if (ed.m_lastOp.tool != nullptr && ed.m_pointEditPart != nullptr && ed.m_pointEditPart == ed.m_lastOp.pointPart)
      Rebuild(ed);
}

// Replaces the curve points by the tool's output on the start points (the undo record, marked before the first rebuild, holds the start state)
void CurveEditOp::Rebuild(EditorUI &ed)
{
   EditorUI::LastOperation &op = ed.m_lastOp;
   vector<PointState> out;
   vector<int> outSel;
   if (!op.tool->Generate(*op.pointPart, op.startPoints, op.startSel, IsClosed(*op.pointPart), out, outSel))
      out = op.startPoints; // e.g. adjusted to a no-op
   DragPointCurve *const curve = op.pointPart->GetDragPointCurve();
   curve->ClearPoints();
   for (const PointState &state : out)
   {
      auto point = std::make_unique<DragPoint>(curve, state.pos.x, state.pos.y, state.pos.z, state.smooth);
      point->SetTextureCoordinateU(state.texCoord);
      point->SetAutoTextureCoordinate(state.autoTexture);
      point->m_slingshot = state.slingshot;
      point->m_uiLocked = state.uiLocked;
      point->m_uiVisible = state.uiVisible;
      curve->PushPoint(std::move(point));
   }
   ed.m_pointSel.clear();
   for (const int i : outSel)
      ed.m_pointSel.push_back(curve->GetPoints()[i].get());
   op.pointSel = ed.m_pointSel;
   ed.EndPointEdit();
}

void CurveEditOp::RenderMenuItems(EditorUI &ed)
{
   const bool canApply = CanApply(ed);
   for (Tool *const tool : Tools())
      if (ImGui::MenuItem(tool->GetName(), tool->GetShortcut() != 0 ? ImGui::GetKeyChordName(tool->GetShortcut()) : nullptr, false, canApply))
         Apply(ed, *tool);
}

bool CurveEditOp::HandleShortcuts(EditorUI &ed)
{
   if (ed.m_pointEditPart == nullptr)
      return false;
   for (Tool *const tool : Tools())
      if (tool->GetShortcut() != 0 && ImGui::IsKeyChordPressed(tool->GetShortcut()))
      {
         Apply(ed, *tool);
         return true;
      }
   return false;
}

}
