// license:GPLv3+

#include "core/stdafx.h"
#include "BevelMath.h"

namespace VPX::EditorUI::BevelMath
{

float CornerAngle(const Vertex2D &prev, const Vertex2D &corner, const Vertex2D &next)
{
   const Vertex2D e1 = prev - corner, e2 = next - corner;
   return atan2f(fabsf(e1.x * e2.y - e1.y * e2.x), e1.Dot(e2));
}

float FilletRadius(const float width, const float cornerAngle) { return width * tanf(0.5f * cornerAngle); }

Result Bevel(const vector<Vertex2D> &points, const vector<int> &corners, const bool closed, const float width, const int steps)
{
   Result result;
   const int n = static_cast<int>(points.size());
   const auto keep = [&result, &points](const int i)
   {
      result.points.push_back(points[i]);
      result.source.push_back(i);
      result.generated.push_back(false);
      result.cutOf.push_back(-1);
   };
   for (int i = 0; i < n; i++)
   {
      if (std::ranges::find(corners, i) == corners.end() || n < 3 || (!closed && (i == 0 || i == n - 1)))
      {
         keep(i);
         continue;
      }
      const Vertex2D &p = points[i];
      Vertex2D d1 = points[(i + n - 1) % n] - p, d2 = points[(i + 1) % n] - p;
      const float l1 = d1.Length(), l2 = d2.Length();
      if (l1 < 1e-4f || l2 < 1e-4f)
      {
         keep(i);
         continue;
      }
      d1 /= l1;
      d2 /= l2;
      const float angle = CornerAngle(points[(i + n - 1) % n], p, points[(i + 1) % n]);
      if (angle > M_PIf - ANGTORAD(0.5f) || angle < ANGTORAD(0.5f)) // (Nearly) straight or folded back: nothing to cut
      {
         keep(i);
         continue;
      }
      const float w = min(width, 0.5f * min(l1, l2));
      if (w <= 1e-4f)
      {
         keep(i);
         continue;
      }
      const float radius = FilletRadius(w, angle);
      result.minRadius = min(result.minRadius, radius);
      const Vertex2D a = p + d1 * w, b = p + d2 * w; // Tangent points on the edges to the previous and next point
      const int segments = max(steps, 1);
      // Fillet: its center is on the bisector, at w / cos(angle / 2) from the corner
      Vertex2D bisector = d1 + d2;
      bisector.Normalize();
      const Vertex2D center = p + bisector * (w / cosf(0.5f * angle));
      const float a0 = atan2f(a.y - center.y, a.x - center.x);
      float sweep = atan2f(b.y - center.y, b.x - center.x) - a0;
      if (sweep > M_PIf)
         sweep -= 2.f * M_PIf;
      else if (sweep < -M_PIf)
         sweep += 2.f * M_PIf;
      for (int k = 0; k <= segments; k++)
      {
         if (k == 0 && !result.points.empty() && result.generated.back() && (result.points.back() - a).LengthSquared() < 1e-6f)
         {
            result.cutOf.back() = i; // Shared with the cut of the previous corner (both clamped to the middle of the edge): its segment to the next point is this cut
            continue;
         }
         const float t = a0 + sweep * static_cast<float>(k) / static_cast<float>(segments);
         result.points.push_back(k == 0 ? a : k == segments ? b : center + Vertex2D(cosf(t), sinf(t)) * radius);
         result.source.push_back(i);
         result.generated.push_back(true);
         result.cutOf.push_back(k < segments ? i : -1);
      }
   }
   if (closed && result.points.size() > 1 && result.generated.front() && result.generated.back() && (result.points.front() - result.points.back()).LengthSquared() < 1e-6f)
   {
      result.points.pop_back(); // Same, across the closing edge
      result.source.pop_back();
      result.generated.pop_back();
      result.cutOf.pop_back();
   }
   return result;
}

vector<bool> SegmentFlags(const Result &bevel, const vector<bool> &flags)
{
   const int n = static_cast<int>(flags.size());
   vector<bool> out(bevel.points.size());
   for (size_t i = 0; i < out.size(); i++)
   {
      const int c = bevel.cutOf[i];
      out[i] = c >= 0 ? flags[(c + n - 1) % n] && flags[c] : flags[bevel.source[i]];
   }
   return out;
}

}
