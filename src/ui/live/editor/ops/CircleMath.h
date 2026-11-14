// license:GPLv3+

#pragma once

#include "math/vector.h"

// Pure geometry of Round Wall (see CircleWallOp): the outline of a round wall, a regular polygon.
// Lengths are in VP units, angles in radians.
namespace VPX::EditorUI::CircleMath
{

constexpr int MIN_FACES = 3;
constexpr int MAX_FACES = 64;

// Signed area of the closed outline (shoelace), positive when it turns towards increasing angles (atan2 of y over x)
float SignedArea(const vector<Vertex2D> &points);

// Regular polygon of faces points (clamped to MIN_FACES..MAX_FACES) on the circle, the first at startAngle, turning towards increasing angles
// if positive is set, decreasing otherwise
vector<Vertex2D> Circle(const Vertex2D &center, float radius, int faces, float startAngle, bool positive);

}
