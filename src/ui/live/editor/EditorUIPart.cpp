#include "core/stdafx.h"

#include "EditorUIPart.h"

namespace VPX::EditorUI
{

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// Drag point edit mode: shared implementation of the 'Curve' property pane section and of the
// point editing commands, used by all the parts defined by a DragPointCurve (ramps, surfaces,
// lights, flashers, rubbers, triggers). The editor provides the point selection and the edit
// side effects through the DragPointEditContext interface.
//

EditorUIPart::TransformMask EditorUIPart::GetCurveTransform(Matrix3D& transform, float z) const
{
   const Vertex2D pivot = GetEditable()->GetCenter();
   transform = Matrix3D::MatrixScale(m_curveScale.x, m_curveScale.y, 1.f) * Matrix3D::MatrixRotateZ(ANGTORAD(m_curveRot)) * Matrix3D::MatrixTranslate(pivot.x, pivot.y, z);
   return static_cast<TransformMask>(TM_TransAny | TM_RotZ | TM_ScaleX | TM_ScaleY | TM_ScaleAll);
}

void EditorUIPart::SetCurveTransform(const vec3& pos, const vec3& scale, const vec3& rot)
{
   IEditable* const part = GetEditable();
   const Vertex2D pivot = part->GetCenter();
   // The gizmo transform is absolute, so the delta of each component is applied to the points around the
   // part center (the reported transform carries the rotation & scale already applied to the points).
   if (rot.z != m_curveRot)
      part->Rotate(rot.z - m_curveRot, pivot, false);
   SetCurveScale(scale);
   part->Translate(Vertex2D(pos.x - pivot.x, pos.y - pivot.y));
   m_curveRot = rot.z;
}

void EditorUIPart::SetCurveScale(const vec3& scale)
{
   IEditable* const part = GetEditable();
   const Vertex2D pivot = part->GetCenter();
   if (scale.x != m_curveScale.x || scale.y != m_curveScale.y)
   {
      // The gizmo scales in the reported local frame: unrotate the points, scale, then rotate back
      if (m_curveRot != 0.f)
         part->Rotate(-m_curveRot, pivot, false);
      part->Scale(scale.x / m_curveScale.x, scale.y / m_curveScale.y, pivot, false);
      if (m_curveRot != 0.f)
         part->Rotate(m_curveRot, pivot, false);
   }
   m_curveScale.Set(max(scale.x, 1e-4f), max(scale.y, 1e-4f));
}

void EditorUIPart::UpdateCurveSection(PropertyPane& props)
{
   if (m_pointEditCtx == nullptr)
      return;
   DragPointCurve* const curve = GetDragPointCurve();
   if (curve == nullptr || !props.BeginSection("Curve"s))
      return;
   const vector<DragPoint*>& sel = m_pointEditCtx->GetSelectedPoints();

   ImGui::Text("%d of %d point(s) selected", (int)sel.size(), (int)curve->GetPoints().size());
   ImGui::BeginDisabled(sel.empty());
   if (ImGui::Button("Smooth"))
      SetSelectedPointsSmooth(true);
   ImGui::SameLine();
   if (ImGui::Button("Sharp"))
      SetSelectedPointsSmooth(false);
   ImGui::SameLine();
   if (ImGui::Button("Flip X"))
      FlipSelectedPoints(true);
   ImGui::SameLine();
   if (ImGui::Button("Flip Y"))
      FlipSelectedPoints(false);
   if (HasSlingshotSegments())
   {
      ImGui::SameLine();
      if (ImGui::Button("Slingshot"))
         ToggleSelectedPointsSlingshot();
   }
   ImGui::EndDisabled();

   if (sel.size() == 1)
   {
      DragPoint* const point = sel.front();
      ImGui::PushID(point);
      props.InputFloat2<DragPoint>(
         point, "Position"s, //
         [](const DragPoint* p) { return Vertex2D(p->GetX(), p->GetY()); }, //
         [this](DragPoint* p, const Vertex2D& v)
         {
            p->SetX(v.x);
            p->SetY(v.y);
            GetDragPointCurve()->OnPointsModified();
         },
         PropertyPane::Unit::VPLength, 1);
      UpdatePointZField(props, point);
      if (HasPointAutoTexture())
         props.Checkbox<DragPoint>(
            point, "Auto Texture Coord."s, //
            [](const DragPoint* p) { return p->IsAutoTextureCoordinate(); }, //
            [](DragPoint* p, bool v) { p->SetAutoTextureCoordinate(v); });
      if (HasPointTextureCoord())
         props.InputFloat<DragPoint>(
            point, "Texture Coord."s, //
            [](const DragPoint* p) { return p->GetTextureCoordinateU(); }, //
            [](DragPoint* p, float v) { p->SetTextureCoordinateU(v); }, PropertyPane::Unit::None, 2);
      ImGui::PopID();
   }
   else if (sel.size() > 1)
   {
      if (ImGui::Button("Align Left"))
         AlignSelectedPoints(true, false);
      ImGui::SameLine();
      if (ImGui::Button("Align Right"))
         AlignSelectedPoints(true, true);
      ImGui::SameLine();
      if (ImGui::Button("Align Top"))
         AlignSelectedPoints(false, false);
      ImGui::SameLine();
      if (ImGui::Button("Align Bottom"))
         AlignSelectedPoints(false, true);
   }

   ImGui::TextDisabled("Click or box select points, drag or use the gizmo (G/R/S) to move them in the playfield plane, Shift+A to add a point on the nearest segment, Del to "
                       "delete, Tab/Esc to exit");
   props.EndSection();
}

void EditorUIPart::UpdatePointZField(PropertyPane& props, DragPoint* point)
{
   ImGui::BeginDisabled();
   props.InputFloat<DragPoint>(
      point, "Z"s, //
      [this](const DragPoint* p) { return GetDragPointZ(p); }, //
      [](DragPoint*, float) {}, PropertyPane::Unit::VPLength, 1);
   ImGui::EndDisabled();
}

void EditorUIPart::SetSelectedPointsSmooth(bool smooth)
{
   m_pointEditCtx->BeginPointEdit();
   for (DragPoint* point : m_pointEditCtx->GetSelectedPoints())
      if (point->IsSmooth() != smooth)
         point->ToggleSmooth(); // ToggleSmooth also maintains slingshot flag coherence
   m_pointEditCtx->EndPointEdit();
}

void EditorUIPart::FlipSelectedPoints(bool flipX)
{
   const vector<DragPoint*>& sel = m_pointEditCtx->GetSelectedPoints();
   float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
   for (const DragPoint* point : sel)
   {
      minX = min(minX, point->GetX());
      maxX = max(maxX, point->GetX());
      minY = min(minY, point->GetY());
      maxY = max(maxY, point->GetY());
   }
   const Vertex2D center(0.5f * (minX + maxX), 0.5f * (minY + maxY));
   m_pointEditCtx->BeginPointEdit();
   for (DragPoint* point : sel)
   {
      if (flipX)
         point->SetX(2.f * center.x - point->GetX());
      else
         point->SetY(2.f * center.y - point->GetY());
   }
   m_pointEditCtx->EndPointEdit();
}

void EditorUIPart::AlignSelectedPoints(bool onX, bool toMax)
{
   const vector<DragPoint*>& sel = m_pointEditCtx->GetSelectedPoints();
   float v = toMax ? -FLT_MAX : FLT_MAX;
   for (const DragPoint* point : sel)
      v = toMax ? max(v, onX ? point->GetX() : point->GetY()) : min(v, onX ? point->GetX() : point->GetY());
   m_pointEditCtx->BeginPointEdit();
   for (DragPoint* point : sel)
   {
      if (onX)
         point->SetX(v);
      else
         point->SetY(v);
   }
   m_pointEditCtx->EndPointEdit();
}

void EditorUIPart::ToggleSelectedPointsSlingshot()
{
   m_pointEditCtx->BeginPointEdit();
   for (DragPoint* point : m_pointEditCtx->GetSelectedPoints())
      point->ToggleSlingshot(); // ToggleSlingshot also maintains smooth flag coherence
   m_pointEditCtx->EndPointEdit();
}

DragPoint* EditorUIPart::AddPointOnCurve(const Vertex2D& pos)
{
   DragPointCurve* const curve = GetDragPointCurve();
   if (curve == nullptr)
      return nullptr;
   const size_t prevPointCount = curve->GetPoints().size();
   InsertPointOnCurve(pos);
   if (curve->GetPoints().size() == prevPointCount)
      return nullptr;
   // Return the curve point nearest to the given position (the newly inserted one)
   DragPoint* nearest = nullptr;
   float nearestDist = FLT_MAX;
   for (const auto& point : curve->GetPoints())
   {
      const float dx = point->GetX() - pos.x;
      const float dy = point->GetY() - pos.y;
      const float dist = dx * dx + dy * dy;
      if (dist < nearestDist)
      {
         nearestDist = dist;
         nearest = point.get();
      }
   }
   return nearest;
}

void EditorUIPart::InsertPointOnCurve(const Vertex2D& pos)
{
   DragPointCurve* const curve = GetDragPointCurve();
   if (curve == nullptr)
      return;
   vector<RenderVertex> vvertex;
   curve->GetRgVertex(vvertex);
   Vertex2D vOut;
   int iSeg = -1;
   ClosestPointOnPolygon(vvertex, pos, vOut, iSeg, true);
   if (iSeg < 0)
      return;
   // Go through vertices (including iSeg itself) counting control points until iSeg
   int icp = 0;
   for (int i = 0; i < (iSeg + 1); i++)
      if (vvertex[i].controlPoint)
         icp++;
   curve->InsertPoint(icp, std::make_unique<DragPoint>(curve, vOut.x, vOut.y, 0.f, IsNewPointSmooth()));
}

}
