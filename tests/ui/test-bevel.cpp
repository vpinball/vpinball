// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "ui/live/editor/ops/BevelMath.h"

#include "doctest.h"

using namespace VPX::EditorUI::BevelMath;

namespace
{
// Outline of a 100 x 100 wall
const vector<Vertex2D> square { { 0.f, 0.f }, { 100.f, 0.f }, { 100.f, 100.f }, { 0.f, 100.f } };

void CheckPoint(const Vertex2D &p, const float x, const float y)
{
   CHECK(p.x == doctest::Approx(x).epsilon(1e-4));
   CHECK(p.y == doctest::Approx(y).epsilon(1e-4));
}
}

TEST_CASE("Wall bevel")
{
   SUBCASE("flat cut of a square corner")
   {
      const Result r = Bevel(square, { 1 }, true, 1.f, 1);
      REQUIRE(r.points.size() == 5);
      CheckPoint(r.points[0], 0.f, 0.f);
      CheckPoint(r.points[1], 99.f, 0.f);
      CheckPoint(r.points[2], 100.f, 1.f);
      CheckPoint(r.points[3], 100.f, 100.f);
      CHECK(!r.generated[0]);
      CHECK(r.generated[1]);
      CHECK(r.generated[2]);
      CHECK(r.source[1] == 1);
      CHECK(r.source[2] == 1);
      CHECK(r.source[3] == 2);
   }

   SUBCASE("the flat cut of a square corner makes two 135 degree corners")
   {
      const Result r = Bevel(square, { 1 }, true, 1.f, 1);
      REQUIRE(r.points.size() == 5);
      CHECK(CornerAngle(r.points[0], r.points[1], r.points[2]) == doctest::Approx(0.75 * M_PI));
      CHECK(CornerAngle(r.points[1], r.points[2], r.points[3]) == doctest::Approx(0.75 * M_PI));
   }

   SUBCASE("fillet with 4 steps is on a circle tangent to both edges")
   {
      const Result r = Bevel(square, { 1 }, true, 10.f, 4);
      REQUIRE(r.points.size() == 3 + 5);
      // Square corner: radius = width, center at (90, 10)
      CHECK(r.minRadius == doctest::Approx(10.));
      CheckPoint(r.points[1], 90.f, 0.f);
      CheckPoint(r.points[5], 100.f, 10.f);
      for (int i = 1; i <= 5; i++)
      {
         CHECK(r.generated[i]);
         CHECK((r.points[i] - Vertex2D(90.f, 10.f)).Length() == doctest::Approx(10.).epsilon(1e-4));
      }
      // Evenly spaced along the quarter circle
      const float chord = (r.points[2] - r.points[1]).Length();
      for (int i = 2; i <= 4; i++)
         CHECK((r.points[i + 1] - r.points[i]).Length() == doctest::Approx(chord).epsilon(1e-4));
   }

   SUBCASE("width is clamped to the middle of the adjacent edges")
   {
      const vector<Vertex2D> rect { { 0.f, 0.f }, { 100.f, 0.f }, { 100.f, 20.f }, { 0.f, 20.f } };
      const Result r = Bevel(rect, { 1 }, true, 50.f, 1);
      REQUIRE(r.points.size() == 5);
      CheckPoint(r.points[1], 90.f, 0.f); // Clamped by the short edge
      CheckPoint(r.points[2], 100.f, 10.f);

      // Two neighbour corners share the middle point of their common edge
      const Result both = Bevel(rect, { 1, 2 }, true, 50.f, 1);
      REQUIRE(both.points.size() == 5);
      CheckPoint(both.points[1], 90.f, 0.f);
      CheckPoint(both.points[2], 100.f, 10.f);
      CheckPoint(both.points[3], 90.f, 20.f);
   }

   SUBCASE("endpoints of an open curve are kept")
   {
      const vector<Vertex2D> path { { 0.f, 0.f }, { 100.f, 0.f }, { 100.f, 100.f } };
      CHECK(Bevel(path, { 0, 2 }, false, 1.f, 1).points.size() == 3);
      const Result r = Bevel(path, { 0, 1, 2 }, false, 1.f, 1);
      REQUIRE(r.points.size() == 4);
      CheckPoint(r.points[0], 0.f, 0.f);
      CheckPoint(r.points[3], 100.f, 100.f);
      // The same points as a closed outline: all corners bevelled
      CHECK(Bevel(path, { 0, 1, 2 }, true, 1.f, 1).points.size() == 6);
   }

   SUBCASE("nearly straight corners are kept")
   {
      const vector<Vertex2D> outline { { 0.f, 0.f }, { 50.f, 0.f }, { 100.f, 0.3f }, { 100.f, 100.f }, { 0.f, 100.f } }; // 179.66 degrees at (50, 0)
      CHECK(Bevel(outline, { 1 }, true, 1.f, 1).points.size() == 5);
      const vector<Vertex2D> bent { { 0.f, 0.f }, { 50.f, 0.f }, { 100.f, 1.f }, { 100.f, 100.f }, { 0.f, 100.f } }; // 178.85 degrees
      CHECK(Bevel(bent, { 1 }, true, 1.f, 1).points.size() == 6);
   }

   SUBCASE("a cut is a slingshot only if both edges of its corner are")
   {
      // Edge 1-2 is a slingshot, edge 2-3 is not; both corners clamped to the middle of edge 1-2, sharing its middle point
      const vector<Vertex2D> rect { { 0.f, 0.f }, { 100.f, 0.f }, { 100.f, 20.f }, { 0.f, 20.f } };
      const Result r = Bevel(rect, { 1, 2 }, true, 50.f, 1);
      REQUIRE(r.points.size() == 5);
      const vector<bool> flags = SegmentFlags(r, { true, true, false, false });
      CHECK(flags[0]); // Rest of edge 0-1
      CHECK(flags[1]); // Cut of corner 1 (edges 0-1 and 1-2)
      CHECK(!flags[2]); // Cut of corner 2 (edges 1-2 and 2-3), from the shared middle point
      CHECK(!flags[3]); // Rest of edge 2-3
      CHECK(!flags[4]);
   }

   SUBCASE("slingshot flags across the closing edge")
   {
      // All corners clamped to the middle of the edges: the last cut wraps to the first point
      const Result r = Bevel(square, { 0, 1, 2, 3 }, true, 60.f, 1);
      REQUIRE(r.points.size() == 4);
      CHECK(r.cutOf[3] == 3);
      const vector<bool> flags = SegmentFlags(r, { false, false, true, true }); // Edges 2-3 and 3-0
      CHECK(!flags[0]); // Cut of corner 0: edges 3-0 and 0-1
      CHECK(!flags[1]);
      CHECK(!flags[2]);
      CHECK(flags[3]); // Cut of corner 3: edges 2-3 and 3-0
   }

   SUBCASE("colinear points are kept")
   {
      const vector<Vertex2D> outline { { 0.f, 0.f }, { 50.f, 0.f }, { 100.f, 0.f }, { 100.f, 100.f }, { 0.f, 100.f } };
      const Result r = Bevel(outline, { 1 }, true, 1.f, 1);
      REQUIRE(r.points.size() == 5);
      CHECK(!r.generated[1]);
      CHECK(r.minRadius == FLT_MAX);
   }
}
