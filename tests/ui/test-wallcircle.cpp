// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "ui/live/editor/ops/CircleMath.h"

#include "doctest.h"

using namespace VPX::EditorUI::CircleMath;

namespace
{
// Outline of a 100 x 100 wall (positive winding), and the same outline the other way round
const vector<Vertex2D> square { { 0.f, 0.f }, { 100.f, 0.f }, { 100.f, 100.f }, { 0.f, 100.f } };
const vector<Vertex2D> squareReversed { { 0.f, 0.f }, { 0.f, 100.f }, { 100.f, 100.f }, { 100.f, 0.f } };

void CheckPoint(const Vertex2D &p, const float x, const float y)
{
   CHECK(p.x == doctest::Approx(x).epsilon(1e-4));
   CHECK(p.y == doctest::Approx(y).epsilon(1e-4));
}
}

TEST_CASE("Wall circle")
{
   SUBCASE("regular spacing")
   {
      const vector<Vertex2D> circle = Circle({ 10.f, -20.f }, 30.f, 8, 0.f, true);
      REQUIRE(circle.size() == 8);
      CheckPoint(circle[0], 40.f, -20.f);
      CheckPoint(circle[2], 10.f, 10.f);
      for (size_t i = 0; i < circle.size(); i++)
         CHECK((circle[(i + 1) % circle.size()] - circle[i]).Length() == doctest::Approx(2.f * 30.f * sinf(M_PIf / 8.f)).epsilon(1e-4));
   }

   SUBCASE("winding")
   {
      CHECK(SignedArea(square) == doctest::Approx(10000.f));
      CHECK(SignedArea(squareReversed) == doctest::Approx(-10000.f));
      for (const bool positive : { true, false })
      {
         const vector<Vertex2D> circle = Circle({ 50.f, 50.f }, 40.f, 12, 0.f, positive);
         CHECK((SignedArea(circle) > 0.f) == positive);
         // Area of the regular dodecagon: 3 r^2
         CHECK(fabsf(SignedArea(circle)) == doctest::Approx(3.f * 40.f * 40.f).epsilon(1e-4));
      }
      // 4 faces through the square's corners, either way round
      const vector<Vertex2D> four = Circle({ 50.f, 50.f }, 50.f * sqrtf(2.f), 4, -0.75f * M_PIf, true);
      for (size_t i = 0; i < 4; i++)
         CheckPoint(four[i], square[i].x, square[i].y);
      const vector<Vertex2D> fourReversed = Circle({ 50.f, 50.f }, 50.f * sqrtf(2.f), 4, -0.75f * M_PIf, false);
      for (size_t i = 0; i < 4; i++)
         CheckPoint(fourReversed[i], squareReversed[i].x, squareReversed[i].y);
   }

   SUBCASE("face count is clamped")
   {
      CHECK(Circle({ 0.f, 0.f }, 10.f, 2, 0.f, true).size() == MIN_FACES);
      CHECK(Circle({ 0.f, 0.f }, 10.f, 0, 0.f, true).size() == MIN_FACES);
      CHECK(Circle({ 0.f, 0.f }, 10.f, 3, 0.f, true).size() == 3);
      CHECK(Circle({ 0.f, 0.f }, 10.f, 1000, 0.f, true).size() == MAX_FACES);
   }
}
