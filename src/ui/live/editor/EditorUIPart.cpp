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
         [](const DragPoint* p) { return Vertex2D(p->m_v.x, p->m_v.y); }, //
         [this](DragPoint* p, const Vertex2D& v)
         {
            p->m_v.x = v.x;
            p->m_v.y = v.y;
            GetDragPointCurve()->OnPointsModified();
         },
         PropertyPane::Unit::VPLength, 1);
      UpdatePointZField(props, point);
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
      if (point->m_smooth != smooth)
         point->ToggleSmooth(); // ToggleSmooth also maintains slingshot flag coherence
   m_pointEditCtx->EndPointEdit();
}

void EditorUIPart::FlipSelectedPoints(bool flipX)
{
   const vector<DragPoint*>& sel = m_pointEditCtx->GetSelectedPoints();
   float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
   for (const DragPoint* point : sel)
   {
      minX = min(minX, point->m_v.x);
      maxX = max(maxX, point->m_v.x);
      minY = min(minY, point->m_v.y);
      maxY = max(maxY, point->m_v.y);
   }
   const Vertex2D center(0.5f * (minX + maxX), 0.5f * (minY + maxY));
   m_pointEditCtx->BeginPointEdit();
   for (DragPoint* point : sel)
   {
      if (flipX)
         point->m_v.x = 2.f * center.x - point->m_v.x;
      else
         point->m_v.y = 2.f * center.y - point->m_v.y;
   }
   m_pointEditCtx->EndPointEdit();
}

void EditorUIPart::AlignSelectedPoints(bool onX, bool toMax)
{
   const vector<DragPoint*>& sel = m_pointEditCtx->GetSelectedPoints();
   float v = toMax ? -FLT_MAX : FLT_MAX;
   for (const DragPoint* point : sel)
      v = toMax ? max(v, onX ? point->m_v.x : point->m_v.y) : min(v, onX ? point->m_v.x : point->m_v.y);
   m_pointEditCtx->BeginPointEdit();
   for (DragPoint* point : sel)
   {
      if (onX)
         point->m_v.x = v;
      else
         point->m_v.y = v;
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
   for (CComObject<DragPoint>* point : curve->GetPoints())
   {
      const float dx = point->m_v.x - pos.x;
      const float dy = point->m_v.y - pos.y;
      const float dist = dx * dx + dy * dy;
      if (dist < nearestDist)
      {
         nearestDist = dist;
         nearest = point;
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
   CComObject<DragPoint>* pdp;
   CComObject<DragPoint>::CreateInstance(&pdp);
   if (pdp == nullptr)
      return;
   pdp->AddRef();
   pdp->Init(curve, vOut.x, vOut.y, 0.f, IsNewPointSmooth());
   curve->InsertPoint(icp, pdp);
}

}
