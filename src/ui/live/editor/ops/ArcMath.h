// license:GPLv3+

#pragma once

#include "math/vector.h"

// Pure geometry of the arc tool (see ArcOp): edges of a drag point outline (its control polygon) replaced by circular arcs through both
// end points. Lengths are in VP units.
namespace VPX::EditorUI::ArcMath
{

struct Result
{
   vector<Vertex2D> points;
   vector<int> source; // Index of the input point each output point comes from (for an arc point, the start point of its edge)
   vector<bool> generated; // Whether the point was generated on an arc (the input points are all kept, unmoved)
   vector<float> along; // Position of a generated point between the end points of its edge, in ]0, 1[ (0 for the input points)
   int arcs = 0; // Number of edges turned into an arc
   bool clamped = false; // Whether the bulge was clamped to a half circle on at least one edge
};

// Edges whose both end points are listed in sel (indices in a curve of n points), as the index of their start point: edge i goes from
// point i to point i + 1, and on a closed curve (of at least 3 points) edge n - 1 goes from the last point to the first one
vector<int> SelectedEdges(int n, const vector<int> &sel, bool closed);

// Twice the signed area of the polygon (positive when counter clockwise in a y up frame)
float SignedArea2(const vector<Vertex2D> &points);

// Replaces each listed edge (see SelectedEdges) by a circular arc of segments segments (segments - 1 new points, evenly spaced on the arc)
// through both end points. bulge is the signed sagitta, the distance from the middle of the edge to the middle of the arc: positive is
// outward of a closed outline (away from its inside, whatever its winding), and to the right of the edge direction (in a y up frame) on an
// open curve. Outward comes from the winding of the whole outline (its signed area): it is arbitrary for a self intersecting outline (a figure
// eight) or one of (nearly) zero area. It is clamped per edge to half the edge length (a half circle). Zero length edges and a zero bulge are kept as they are.
Result Arc(const vector<Vertex2D> &points, const vector<int> &edges, bool closed, float bulge, int segments);

}
