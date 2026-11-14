// license:GPLv3+

#pragma once

#include "CurveEditOp.h"

namespace VPX::EditorUI
{

// Point edit mode > Bevel Corners (Ctrl+B, Blender's bevel): cuts the selected corners of the edited outline back along both adjacent edges,
// with a flat cut (default 1 VP unit, so no wall collider corner is sharper than 90 degrees from a square one) or a round fillet. Smooth
// points are kept (the cut would not follow their curve). The generated points are plain (not smooth) drag points that copy the other properties of the bevelled corner. See BevelMath for the geometry.
class BevelOp final : public CurveEditOp::Tool
{
public:
   const char *GetName() const override { return "Bevel Corners"; }
   int GetShortcut() const override;
   bool Generate(
      const EditorUIPart &part, const vector<CurveEditOp::PointState> &points, const vector<int> &sel, bool closed, vector<CurveEditOp::PointState> &out, vector<int> &outSel) override;
   bool RenderParameters(const EditorUIPart &part, const std::function<void(const char *)> &row) override;

private:
   float m_width = 1.f; // VP units, from the corner along each edge
   int m_steps = 1; // 1 for a flat cut, more for a round fillet
   float m_minRadius = FLT_MAX; // Smallest fillet radius of the last generated bevel (a ramp folds on the inside of a tighter turn than its half width)
};

}
