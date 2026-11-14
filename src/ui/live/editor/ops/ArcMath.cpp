// license:GPLv3+

#include "core/stdafx.h"
#include "ArcMath.h"

namespace VPX::EditorUI::ArcMath
{

vector<int> SelectedEdges(const int n, const vector<int> &sel, const bool closed)
{
   vector<int> edges;
   const auto selected = [&sel](const int i) { return std::ranges::find(sel, i) != sel.end(); };
   const int count = closed && n >= 3 ? n : n - 1;
   for (int i = 0; i < count; i++)
      if (selected(i) && selected((i + 1) % n))
         edges.push_back(i);
   return edges;
}

float SignedArea2(const vector<Vertex2D> &points)
{
   double area = 0.;
   for (size_t i = 0, n = points.size(); i < n; i++)
   {
      const Vertex2D &a = points[i], &b = points[(i + 1) % n];
      area += static_cast<double>(a.x) * b.y - static_cast<double>(b.x) * a.y;
   }
   return static_cast<float>(area);
}

Result Arc(const vector<Vertex2D> &points, const vector<int> &edges, const bool closed, const float bulge, const int segments)
{
   Result result;
   const int n = static_cast<int>(points.size());
   const int nSegments = max(segments, 2);
   // Outward of a closed outline is on the right of its edges when it winds counter clockwise, on the left otherwise
   const double side = closed && SignedArea2(points) < 0.f ? -1. : 1.;
   for (int i = 0; i < n; i++)
   {
      result.points.push_back(points[i]);
      result.source.push_back(i);
      result.generated.push_back(false);
      result.along.push_back(0.f);
      if (std::ranges::find(edges, i) == edges.end())
         continue;
      const Vertex2D &a = points[i], &b = points[(i + 1) % n];
      const double dx = static_cast<double>(b.x) - a.x, dy = static_cast<double>(b.y) - a.y;
      const double length = sqrt(dx * dx + dy * dy);
      if (length < 1e-4)
         continue;
      const double h = 0.5 * length;
      if (fabs(bulge) > h)
         result.clamped = true;
      const double s = std::clamp(static_cast<double>(bulge), -h, h);
      if (fabs(s) < 1e-4)
         continue;
      // Frame of the edge: u along it, v its outward normal (right of u), the arc middle at mid + v * s
      const double ux = dx / length, uy = dy / length;
      const double vx = uy * side * (s > 0. ? 1. : -1.), vy = -ux * side * (s > 0. ? 1. : -1.);
      const double sag = fabs(s);
      const double radius = (h * h + sag * sag) / (2. * sag);
      const double halfAngle = 2. * atan(sag / h); // Half the angle of the arc seen from its center
      const double mx = 0.5 * (static_cast<double>(a.x) + b.x), my = 0.5 * (static_cast<double>(a.y) + b.y);
      for (int k = 1; k < nSegments; k++)
      {
         // Angle from the arc middle, from -halfAngle at a to halfAngle at b. Height above the chord: sag - radius (1 - cos(angle)),
         // written with sin^2 so that it stays accurate for a large radius (small bulge)
         const double angle = halfAngle * (2. * k / nSegments - 1.);
         const double along = radius * sin(angle);
         const double sh = sin(0.5 * angle);
         const double up = sag - 2. * radius * sh * sh;
         result.points.emplace_back(static_cast<float>(mx + ux * along + vx * up), static_cast<float>(my + uy * along + vy * up));
         result.source.push_back(i);
         result.generated.push_back(true);
         result.along.push_back(static_cast<float>(k) / static_cast<float>(nSegments));
      }
      result.arcs++;
   }
   return result;
}

}
