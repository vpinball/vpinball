// license:GPLv3+

#pragma once

#include "math/vector.h"

#include <array>

// Pure helpers of the reference image tools (see ReferenceImageOp): sizes in real world units, image aspect checks and the rectangle
// of the flasher showing the image. Lengths are in VP units unless stated otherwise.
namespace VPX::EditorUI::ReferenceImageMath
{

enum class Unit
{
   Centimeters,
   Inches
};

float ToVPU(float value, Unit unit);
float FromVPU(float vpu, Unit unit);

// Height that keeps the pixel aspect of an image for the given width (0 when the image size is not valid)
float HeightForWidth(float width, unsigned int pxWidth, unsigned int pxHeight);

// Relative difference between the aspect of the entered size and the pixel aspect of the image (0 when they match)
float AspectMismatch(float width, float height, unsigned int pxWidth, unsigned int pxHeight);
constexpr float AspectTolerance = 0.01f; // Below 1%, the image is not visibly stretched
bool IsAspectMismatch(float width, float height, unsigned int pxWidth, unsigned int pxHeight);

// Size (VPU) of a side of pxCount pixels of an image scanned at the given resolution (dots per inch)
float SizeFromDpi(unsigned int pxCount, float dpi);

// Axis aligned rectangle
struct Rect
{
   Vertex2D center;
   float width;
   float height;
};

// Bounding rectangle of a point list (a zero rectangle at the origin for an empty list)
template <class Points> Rect BoundsOf(const Points &points)
{
   if (points.empty())
      return { Vertex2D(0.f, 0.f), 0.f, 0.f };
   Vertex2D lo(FLT_MAX, FLT_MAX), hi(-FLT_MAX, -FLT_MAX);
   for (const Vertex2D &p : points)
   {
      lo.x = min(lo.x, p.x);
      lo.y = min(lo.y, p.y);
      hi.x = max(hi.x, p.x);
      hi.y = max(hi.y, p.y);
   }
   return { Vertex2D(0.5f * (lo.x + hi.x), 0.5f * (lo.y + hi.y)), hi.x - lo.x, hi.y - lo.y };
}

// Rectangle at the pixel aspect of the image, with the centre and width of the given one (unchanged when the image size is not valid)
Rect FitToAspect(const Rect &rect, unsigned int pxWidth, unsigned int pxHeight);

// Uniform version of the X/Y scale a transform tool asks for (gizmo axis handle, S with an axis constraint), so that the image keeps its
// aspect: the axis the tool drives sets both. The tools are absolute (each frame asks for the scale from the start of the drag), so the
// driving axis is the one that changed since the last request (lastRequested) while the other did not, else the one that differs the most
// from the current scale. A request that keeps the current X/Y ratio is returned as is (uniform scale, cancel, undo of the drag), and the
// same request as the last one keeps the current scale.
Vertex2D UniformScale(const Vertex2D &requested, const Vertex2D &current, const Vertex2D &lastRequested);

// Corners of a rectangle, in the order of the points of a new flasher (Flasher::InitShape): (-x,-y) (-x,+y) (+x,+y) (+x,-y).
// The image is mapped over the bounds of the points (Wrap mode) with its first row along -y, so it shows upright in the top view.
std::array<Vertex2D, 4> RectCorners(const Rect &rect);

}
