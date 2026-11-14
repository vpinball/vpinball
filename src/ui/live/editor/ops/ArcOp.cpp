// license:GPLv3+

#include "core/stdafx.h"
#include "ArcOp.h"
#include "ArcMath.h"
#include "ui/live/editor/EditorTheme.h"

namespace VPX::EditorUI
{

namespace
{
   ArcOp arcOp;
   [[maybe_unused]] const bool registered = CurveEditOp::Register(&arcOp);
}

int ArcOp::GetShortcut() const { return ImGuiMod_Ctrl | ImGuiKey_E; }

bool ArcOp::Start(const EditorUIPart &, const vector<CurveEditOp::PointState> &points, const vector<int> &sel, const bool closed, string &error)
{
   // Default bulge: a quarter of the shortest selected edge
   const int n = static_cast<int>(points.size());
   float shortest = FLT_MAX;
   for (const int i : ArcMath::SelectedEdges(n, sel, closed))
   {
      const CurveEditOp::PointState &a = points[i], &b = points[(i + 1) % n];
      if (const float length = Vertex2D(b.pos.x - a.pos.x, b.pos.y - a.pos.y).Length(); length >= 1e-4f)
         shortest = min(shortest, length);
   }
   if (shortest == FLT_MAX)
   {
      error = "select both end points of an edge";
      return false;
   }
   m_bulge = 0.25f * shortest;
   return true;
}

bool ArcOp::Generate(
   const EditorUIPart &, const vector<CurveEditOp::PointState> &points, const vector<int> &sel, const bool closed, vector<CurveEditOp::PointState> &out, vector<int> &outSel)
{
   vector<Vertex2D> pos;
   for (const CurveEditOp::PointState &point : points)
      pos.emplace_back(point.pos.x, point.pos.y);
   const int n = static_cast<int>(points.size());
   const vector<int> edges = ArcMath::SelectedEdges(n, sel, closed);
   const ArcMath::Result arc = ArcMath::Arc(pos, edges, closed, m_bulge, m_segments);
   m_clamped = arc.clamped;
   m_closed = closed;
   out.clear();
   outSel.clear();
   for (size_t i = 0; i < arc.points.size(); i++)
   {
      const int src = arc.source[i];
      CurveEditOp::PointState point = points[src];
      if (arc.generated[i])
      {
         const CurveEditOp::PointState &end = points[(src + 1) % n];
         const float t = arc.along[i];
         point.pos.x = arc.points[i].x;
         point.pos.y = arc.points[i].y;
         point.pos.z += (end.pos.z - point.pos.z) * t;
         point.texCoord += (end.texCoord - point.texCoord) * t;
         point.smooth = false;
      }
      if (arc.generated[i] || std::ranges::find(sel, src) != sel.end())
         outSel.push_back(static_cast<int>(out.size())); // The arcs with their end points, and the other selected points
      out.push_back(point);
   }
   // A zero bulge (adjusted to it) keeps the curve as it was, still with an edge to arc
   return !edges.empty();
}

bool ArcOp::RenderParameters(const EditorUIPart &, const std::function<void(const char *)> &row)
{
   bool changed = false;
   row("Bulge");
   changed |= ImGui::DragFloat("##ArcBulge", &m_bulge, 0.25f, -10000.f, 10000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
   if (m_closed)
      ImGui::SetItemTooltip(
         "Distance from the middle of the edge to the middle of the arc, in VP units.\nPositive bulges outward of the wall, negative inward. At most half the edge length (a half circle)");
   else
      ImGui::SetItemTooltip("Distance from the middle of the edge to the middle of the arc, in VP units.\nPositive bulges to one side of the path, negative to the other. At most half the "
                            "edge length (a half circle)");
   row("Segments");
   changed |= ImGui::SliderInt("##ArcSegments", &m_segments, 2, 32, "%d", ImGuiSliderFlags_AlwaysClamp);
   ImGui::SetItemTooltip("Number of straight segments of each arc");
   if (m_clamped)
      ImGui::TextColored(EditorTheme::V(EditorTheme::Accent), "Clamped to a half circle (bulge above half the edge length)");
   return changed;
}

}
