// license:GPLv3+

#include "core/stdafx.h"
#include "CircleMath.h"

namespace VPX::EditorUI::CircleMath
{

float SignedArea(const vector<Vertex2D> &points)
{
   float area = 0.f;
   for (size_t i = 0, n = points.size(); i < n; i++)
   {
      const Vertex2D &a = points[i], &b = points[(i + 1) % n];
      area += a.x * b.y - b.x * a.y;
   }
   return 0.5f * area;
}

vector<Vertex2D> Circle(const Vertex2D &center, const float radius, const int faces, const float startAngle, const bool positive)
{
   const int n = clamp(faces, MIN_FACES, MAX_FACES);
   const float step = (positive ? 2.f : -2.f) * M_PIf / static_cast<float>(n);
   vector<Vertex2D> result;
   result.reserve(n);
   for (int i = 0; i < n; i++)
   {
      const float a = startAngle + step * static_cast<float>(i);
      result.emplace_back(center.x + radius * cosf(a), center.y + radius * sinf(a));
   }
   return result;
}

}
