// license:GPLv3+

#include "core/stdafx.h"
#include "BevelOp.h"
#include "BevelMath.h"
#include "ui/live/editor/parts/EditorUIPart.h"
#include "ui/live/editor/EditorTheme.h"
#include "parts/ramp.h"

namespace VPX::EditorUI
{

namespace
{
   BevelOp bevelOp;
   [[maybe_unused]] const bool registered = CurveEditOp::Register(&bevelOp);
}

int BevelOp::GetShortcut() const { return ImGuiMod_Ctrl | ImGuiKey_B; }

bool BevelOp::Generate(
   const EditorUIPart &, const vector<CurveEditOp::PointState> &points, const vector<int> &sel, const bool closed, vector<CurveEditOp::PointState> &out, vector<int> &outSel)
{
   vector<Vertex2D> pos;
   vector<bool> slingshot;
   for (const CurveEditOp::PointState &point : points)
   {
      pos.emplace_back(point.pos.x, point.pos.y);
      slingshot.push_back(point.slingshot);
   }
   vector<int> corners; // Smooth points are skipped: the cut would not follow their curve
   std::ranges::copy_if(sel, std::back_inserter(corners), [&points](const int i) { return !points[i].smooth; });
   const BevelMath::Result bevel = BevelMath::Bevel(pos, corners, closed, m_width, m_steps);
   const vector<bool> segmentSlingshot = BevelMath::SegmentFlags(bevel, slingshot);
   m_minRadius = bevel.minRadius;
   out.clear();
   outSel.clear();
   for (size_t i = 0; i < bevel.points.size(); i++)
   {
      const int src = bevel.source[i];
      CurveEditOp::PointState point = points[src];
      if (bevel.generated[i])
      {
         point.pos.x = bevel.points[i].x;
         point.pos.y = bevel.points[i].y;
         point.smooth = false;
         outSel.push_back(static_cast<int>(out.size()));
      }
      point.slingshot = segmentSlingshot[i];
      out.push_back(point);
   }
   return !outSel.empty();
}

bool BevelOp::RenderParameters(const EditorUIPart &part, const std::function<void(const char *)> &row)
{
   bool changed = false;
   row("Width");
   changed |= ImGui::DragFloat("##BevelWidth", &m_width, 0.1f, 0.01f, 1000.f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
   ImGui::SetItemTooltip("Distance from the corner along each edge, in VP units (clamped to the middle of the edges).\nSmooth and straight points are not bevelled");
   row("Steps");
   changed |= ImGui::SliderInt("##BevelSteps", &m_steps, 1, 16, "%d", ImGuiSliderFlags_AlwaysClamp);
   ImGui::SetItemTooltip("1 for a flat cut, more for a round fillet of this many segments");
   if (part.GetEditable()->GetItemType() == eItemRamp)
   {
      const Ramp *const ramp = static_cast<const Ramp *>(part.GetEditable());
      const float halfWidth = 0.5f * max(ramp->m_d.m_widthbottom, ramp->m_d.m_widthtop);
      if (m_minRadius < halfWidth)
         ImGui::TextColored(EditorTheme::V(EditorTheme::Accent), "%s %.1f is below half the ramp width (%.1f):\nthe ramp folds on the inside of the turn",
            m_steps > 1 ? "Radius" : "Turn radius of the cut", m_minRadius, halfWidth);
   }
   return changed;
}

}
