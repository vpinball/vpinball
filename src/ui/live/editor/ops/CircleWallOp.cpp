// license:GPLv3+

#include "core/stdafx.h"
#include "CircleWallOp.h"
#include "CircleMath.h"
#include "ui/live/editor/parts/EditorUIPart.h"
#include "ui/live/EditorUI.h"
#include "physics/PhysicsEngine.h"
#include "renderer/Renderer.h"

namespace VPX::EditorUI
{

namespace
{
   struct State
   {
      bool armed = false;
      bool dragging = false; // Placing button still held: the mouse sets the radius
      double clickTime = 0.0; // Time of the placing click (a later press is not a radius drag, e.g. after a release outside the panes)
      std::shared_ptr<EditorUIPart> part; // The created wall
      Vertex2D center;
      float radius = 25.f; // VP units, the default for a click without drag
      int faces = 12;
      bool positive = true; // Winding of the default wall outline
   } state;
}

void CircleWallOp::Arm(EditorUI &ed)
{
   ed.m_addPartType = eItemSurface;
   state.armed = true;
}

void CircleWallOp::Disarm() { state.armed = false; }

void CircleWallOp::OnCreated(EditorUI &ed, IEditable *const part, const Vertex2D &center)
{
   if (!state.armed)
      return;
   state.armed = false;
   const auto it = ed.m_editableMap.find(part);
   if (it == ed.m_editableMap.end() || it->second->GetDragPointCurve() == nullptr)
      return;
   vector<Vertex2D> outline;
   for (const auto &point : it->second->GetDragPointCurve()->GetPoints())
      outline.emplace_back(point->GetX(), point->GetY());
   state.positive = CircleMath::SignedArea(outline) >= 0.f;
   state.part = it->second;
   state.center = center;
   state.radius = 25.f;
   ed.SetLastOperation(EditorUI::LastOperation::Type::CircleWall, Matrix3D::MatrixIdentity());
   state.dragging = ImGui::IsMouseDown(ImGuiMouseButton_Left);
   state.clickTime = ImGui::GetIO().MouseClickedTime[ImGuiMouseButton_Left];
   Rebuild(ed);
}

void CircleWallOp::Drag(EditorUI &ed, const Vertex2D &pos)
{
   if (state.armed && ed.m_addPartType != eItemSurface) // Add part mode canceled
      state.armed = false;
   if (!state.dragging)
      return;
   if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::GetIO().MouseClickedTime[ImGuiMouseButton_Left] != state.clickTime || !IsLive(ed))
   {
      state.dragging = false;
      return;
   }
   if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left))
      return;
   state.radius = max(1.f, (pos - state.center).Length());
   Rebuild(ed);
}

void CircleWallOp::Reapply(EditorUI &ed) { Rebuild(ed); }

// The wall is still the one created by the last operation, which is still adjustable (the creation is the last undo step)
bool CircleWallOp::IsLive(const EditorUI &ed)
{
   const bool live = state.part != nullptr && ed.m_lastOp.type == EditorUI::LastOperation::Type::CircleWall && ed.m_undo.TopRecordId() == ed.m_lastOp.undoId
      && ed.m_lastOp.selection.size() == 1 && ed.m_lastOp.selection[0] == state.part && !ed.m_table->IsLocked() && !state.part->GetEditable()->IsUILocked();
   if (!live)
   {
      state.part.reset(); // Release the part (it may have been deleted)
      state.dragging = false;
   }
   return live;
}

void CircleWallOp::Rebuild(EditorUI &ed)
{
   if (!IsLive(ed))
      return;
   IEditable *const editable = state.part->GetEditable();
   DragPointCurve *const curve = state.part->GetDragPointCurve();
   curve->ClearPoints();
   for (const Vertex2D &p : CircleMath::Circle(state.center, state.radius, state.faces, 0.f, state.positive))
      curve->PushPoint(std::make_unique<DragPoint>(curve, p.x, p.y, 0.f, false));
   curve->OnPointsModified();
   ed.m_renderer->ReinitRenderable(editable->GetIRenderable());
   ed.m_player->m_physics->Update(editable);
}

bool CircleWallOp::RenderParameters(const std::function<void(const char *)> &row)
{
   bool changed = false;
   row("Radius");
   changed |= ImGui::DragFloat("##RoundWallRadius", &state.radius, 0.1f, 1.f, 10000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
   ImGui::SetItemTooltip("Distance from the center to the points, in VP units");
   row("Faces");
   changed |= ImGui::SliderInt("##RoundWallFaces", &state.faces, CircleMath::MIN_FACES, CircleMath::MAX_FACES, "%d", ImGuiSliderFlags_AlwaysClamp);
   ImGui::SetItemTooltip("Number of points of the wall outline: more faces give a rounder bounce and cost more physics work");
   row("Center");
   changed |= ImGui::DragFloat2("##RoundWallCenter", &state.center.x, 1.f, 0.f, 0.f, "%.2f");
   ImGui::SetItemTooltip("X, Y in VP units");
   return changed;
}

}
