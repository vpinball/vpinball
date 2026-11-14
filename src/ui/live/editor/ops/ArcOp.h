// license:GPLv3+

#pragma once

#include "CurveEditOp.h"

namespace VPX::EditorUI
{

// Point edit mode > Arc Edges (Ctrl+E): turns each edge of the edited outline whose both end points are selected into a circular arc through
// them. The end points do not move, the arc is made of new plain (not smooth) drag points that copy the other properties of the start point
// of the edge (slingshot included), with height and texture coordinate interpolated along the edge. Smooth end points keep their flag. See
// ArcMath for the geometry.
class ArcOp final : public CurveEditOp::Tool
{
public:
   const char *GetName() const override { return "Arc Edges"; }
   int GetShortcut() const override;
   bool Start(const EditorUIPart &part, const vector<CurveEditOp::PointState> &points, const vector<int> &sel, bool closed, string &error) override;
   bool Generate(
      const EditorUIPart &part, const vector<CurveEditOp::PointState> &points, const vector<int> &sel, bool closed, vector<CurveEditOp::PointState> &out, vector<int> &outSel) override;
   bool RenderParameters(const EditorUIPart &part, const std::function<void(const char *)> &row) override;

private:
   float m_bulge = 0.f; // VP units, signed distance from the middle of the edge to the middle of the arc, positive outward (set by Start)
   int m_segments = 4;
   bool m_clamped = false; // The last generated arc was clamped to a half circle on at least one edge
   bool m_closed = true; // The last generated arc was on a closed outline (the bulge sign is outward), not on a ramp path
};

}
