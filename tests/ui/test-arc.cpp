// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "ui/live/editor/ops/ArcMath.h"

#include "doctest.h"

using namespace VPX::EditorUI::ArcMath;

namespace
{
// Outline of a 100 x 100 wall, counter clockwise in a y up frame (inside at y > 0 of the first edge)
const vector<Vertex2D> square { { 0.f, 0.f }, { 100.f, 0.f }, { 100.f, 100.f }, { 0.f, 100.f } };

void CheckPoint(const Vertex2D &p, const float x, const float y)
{
   CHECK(p.x == doctest::Approx(x).epsilon(1e-4));
   CHECK(p.y == doctest::Approx(y).epsilon(1e-4));
}

// Center of the circle through a, b and c
Vertex2D Circumcenter(const Vertex2D &a, const Vertex2D &b, const Vertex2D &c)
{
   const double d = 2. * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y));
   const double a2 = a.x * a.x + a.y * a.y, b2 = b.x * b.x + b.y * b.y, c2 = c.x * c.x + c.y * c.y;
   return { static_cast<float>((a2 * (b.y - c.y) + b2 * (c.y - a.y) + c2 * (a.y - b.y)) / d), static_cast<float>((a2 * (c.x - b.x) + b2 * (a.x - c.x) + c2 * (b.x - a.x)) / d) };
}
}

TEST_CASE("Wall arc")
{
   SUBCASE("selected edges")
   {
      CHECK(SelectedEdges(4, { 1, 2 }, true) == vector<int> { 1 });
      CHECK(SelectedEdges(4, { 2, 1 }, true) == vector<int> { 1 });
      CHECK(SelectedEdges(4, { 0, 3 }, true) == vector<int> { 3 }); // Closing edge
      CHECK(SelectedEdges(4, { 0, 3 }, false).empty());
      CHECK(SelectedEdges(4, { 0, 2 }, true).empty()); // Not adjacent
      CHECK(SelectedEdges(4, { 1 }, true).empty());
      CHECK(SelectedEdges(4, { 0, 1, 2 }, true) == vector<int> { 0, 1 });
      CHECK(SelectedEdges(4, { 0, 1, 2, 3 }, true) == vector<int> { 0, 1, 2, 3 });
      CHECK(SelectedEdges(4, { 0, 1, 2, 3 }, false) == vector<int> { 0, 1, 2 });
   }

   SUBCASE("points are on the circle through both end points, end points untouched")
   {
      const Result r = Arc(square, { 0 }, true, 25.f, 4);
      REQUIRE(r.points.size() == 4 + 3);
      CHECK(r.arcs == 1);
      CHECK(!r.clamped);
      CheckPoint(r.points[0], 0.f, 0.f);
      CheckPoint(r.points[4], 100.f, 0.f);
      CheckPoint(r.points[5], 100.f, 100.f);
      CheckPoint(r.points[6], 0.f, 100.f);
      CHECK(!r.generated[0]);
      CHECK(!r.generated[4]);
      // Sagitta 25 on a chord of 100: radius (50^2 + 25^2) / 50 = 62.5, center 37.5 inside
      const Vertex2D center(50.f, 37.5f);
      for (int i = 0; i <= 4; i++)
         CHECK((r.points[i] - center).Length() == doctest::Approx(62.5).epsilon(1e-4));
      for (int i = 1; i <= 3; i++)
      {
         CHECK(r.generated[i]);
         CHECK(r.source[i] == 0);
         CHECK(r.along[i] == doctest::Approx(i / 4.));
      }
      CheckPoint(r.points[2], 50.f, -25.f); // Middle of the arc, outward
      // Evenly spaced along the arc
      const float chord = (r.points[1] - r.points[0]).Length();
      for (int i = 1; i < 4; i++)
         CHECK((r.points[i + 1] - r.points[i]).Length() == doctest::Approx(chord).epsilon(1e-4));
   }

   SUBCASE("segment count")
   {
      for (const int segments : { 2, 3, 8, 32 })
      {
         const Result r = Arc(square, { 1 }, true, 10.f, segments);
         CHECK(r.points.size() == 4 + segments - 1);
         const Vertex2D center = Circumcenter(r.points[1], r.points[2], r.points[2 + segments - 1]);
         const float radius = (r.points[1] - center).Length();
         for (int i = 1; i <= segments + 1; i++)
            CHECK((r.points[i] - center).Length() == doctest::Approx(radius).epsilon(1e-4));
      }
      CHECK(Arc(square, { 1 }, true, 10.f, 1).points.size() == 5); // At least 2 segments
   }

   SUBCASE("bulge sign is outward for both windings")
   {
      // Counter clockwise: the inside is above the first edge (y > 0), outward is y < 0
      CheckPoint(Arc(square, { 0 }, true, 20.f, 2).points[1], 50.f, -20.f);
      CheckPoint(Arc(square, { 0 }, true, -20.f, 2).points[1], 50.f, 20.f);
      // Clockwise: same square, reversed. Edge 2 goes from (100, 0) to (0, 0), the inside is still above it
      const vector<Vertex2D> cw { { 0.f, 100.f }, { 100.f, 100.f }, { 100.f, 0.f }, { 0.f, 0.f } };
      CHECK(SignedArea2(square) > 0.f);
      CHECK(SignedArea2(cw) < 0.f);
      const Result r = Arc(cw, { 2 }, true, 20.f, 2);
      REQUIRE(r.points.size() == 5);
      CheckPoint(r.points[3], 50.f, -20.f);
      CheckPoint(Arc(cw, { 2 }, true, -20.f, 2).points[3], 50.f, 20.f);
      // Top edge of the clockwise outline: outward is y > 100
      CheckPoint(Arc(cw, { 0 }, true, 20.f, 2).points[1], 50.f, 120.f);
      // Open curve: to the right of the edge direction (y up frame)
      const vector<Vertex2D> path { { 0.f, 0.f }, { 100.f, 0.f }, { 100.f, 100.f } };
      CheckPoint(Arc(path, { 0 }, false, 20.f, 2).points[1], 50.f, -20.f);
   }

   SUBCASE("bulge is clamped to a half circle")
   {
      const Result r = Arc(square, { 0 }, true, 80.f, 4);
      CHECK(r.clamped);
      REQUIRE(r.points.size() == 7);
      CheckPoint(r.points[2], 50.f, -50.f);
      for (int i = 0; i <= 4; i++)
         CHECK((r.points[i] - Vertex2D(50.f, 0.f)).Length() == doctest::Approx(50.).epsilon(1e-4));
      CHECK(Arc(square, { 0 }, true, -80.f, 4).clamped);
      CheckPoint(Arc(square, { 0 }, true, -80.f, 4).points[2], 50.f, 50.f);
      CHECK(!Arc(square, { 0 }, true, 50.f, 4).clamped);
   }

   SUBCASE("closing edge of a closed curve")
   {
      // Edge 3 from (0, 100) to (0, 0): outward is x < 0, the arc points are appended after the last point
      const Result r = Arc(square, { 3 }, true, 25.f, 4);
      REQUIRE(r.points.size() == 7);
      CheckPoint(r.points[3], 0.f, 100.f);
      CHECK(r.generated[4]);
      CHECK(r.source[4] == 3);
      CheckPoint(r.points[5], -25.f, 50.f);
      for (int i = 3; i < 7; i++)
         CHECK((r.points[i] - Vertex2D(37.5f, 50.f)).Length() == doctest::Approx(62.5).epsilon(1e-4));
      CHECK((r.points[0] - Vertex2D(37.5f, 50.f)).Length() == doctest::Approx(62.5).epsilon(1e-4));
   }

   SUBCASE("several edges")
   {
      const Result r = Arc(square, SelectedEdges(4, { 0, 1, 2, 3 }, true), true, 10.f, 2);
      REQUIRE(r.points.size() == 8);
      CHECK(r.arcs == 4);
      CheckPoint(r.points[1], 50.f, -10.f);
      CheckPoint(r.points[3], 110.f, 50.f);
      CheckPoint(r.points[5], 50.f, 110.f);
      CheckPoint(r.points[7], -10.f, 50.f);
   }

   SUBCASE("small bulge stays accurate")
   {
      const Result r = Arc(square, { 0 }, true, 0.01f, 4);
      REQUIRE(r.points.size() == 7);
      CHECK(r.points[2].x == doctest::Approx(50.).epsilon(1e-5));
      CHECK(r.points[2].y == doctest::Approx(-0.01).epsilon(1e-3));
      CHECK(r.points[1].y < 0.f);
      CHECK(r.points[1].y > r.points[2].y);
   }

   SUBCASE("zero bulge and zero length edges are kept")
   {
      const Result zero = Arc(square, { 0 }, true, 0.f, 4);
      CHECK(zero.points == square);
      CHECK(zero.arcs == 0);
      const vector<Vertex2D> degenerate { { 0.f, 0.f }, { 0.f, 0.f }, { 100.f, 100.f }, { 0.f, 100.f } };
      const Result r = Arc(degenerate, { 0 }, true, 10.f, 4);
      CHECK(r.points == degenerate);
      CHECK(r.arcs == 0);
   }
}
