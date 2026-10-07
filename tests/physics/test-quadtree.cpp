// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "physics/collide.h"
#include "physics/quadtree.h"
#include "physics/kdtree.h"
#include "parts/ball.h"
#include "physics/hitball.h"
#include "physics-harness.h"

#include <chrono>
#include <random>

#include "doctest.h"

namespace
{

// Build n hit circles spread over the playfield, all owned by a single editable part
vector<std::unique_ptr<HitObject>> MakeCircles(IEditable* editable, const int n)
{
   vector<std::unique_ptr<HitObject>> objects;
   for (int i = 0; i < n; ++i)
      objects.push_back(std::make_unique<HitCircle>(editable, Vertex2D(50.f + 100.f * i, 50.f + 80.f * (i % 5)), 10.f, 0.f, 50.f));
   return objects;
}

// Build clusters of nearly coincident hit circles: the items of a cluster keep co-descending
// into a single quadrant level after level, so each cluster burns ~4 nodes per level (way more
// than the 2n+1 initial node pool estimate) until the level_empty bail out kicks in
vector<std::unique_ptr<HitObject>> MakeClusters(IEditable* editable, const int clusters)
{
   vector<std::unique_ptr<HitObject>> objects;
   for (int c = 0; c < clusters; ++c)
      for (int i = 0; i < 5; ++i)
         objects.push_back(std::make_unique<HitCircle>(editable, Vertex2D(133.f + 397.f * (c % 4) + 0.1f * i, 133.f + 397.f * (c / 4) + 0.1f * i), 1.f, 0.f, 50.f));
   return objects;
}

vector<HitObject*> ToPtrVector(const vector<std::unique_ptr<HitObject>>& objects)
{
   vector<HitObject*> result;
   result.reserve(objects.size());
   for (const auto& object : objects)
      result.push_back(object.get());
   return result;
}

} // namespace

TEST_CASE("Hit quadtree")
{
   Ball* const part = Ball::COMCreate();

   SUBCASE("empty tree")
   {
      HitQuadtree tree;
      vector<HitObject*> empty;
      tree.Reset(empty);
      CHECK(tree.GetObjectCount() == 0);
   }

   SUBCASE("reset, insert, remove and update")
   {
      const auto objects = MakeCircles(part, 16);
      vector<HitObject*> vho = ToPtrVector(objects);

      HitQuadtree tree;
      tree.Reset(vho);
      CHECK(tree.GetObjectCount() == 16);
      CHECK(tree.GetNLevels() >= 1); // more than 4 items: the tree subdivides
      CHECK(tree.GetHitObjects().size() == 16);

      HitCircle extra(part, Vertex2D(5000.f, 5000.f), 10.f, 0.f, 50.f);
      tree.Insert(&extra);
      CHECK(tree.GetObjectCount() == 17);

      tree.Remove(&extra);
      CHECK(tree.GetObjectCount() == 16);
      CHECK(std::ranges::find(tree.GetHitObjects(), &extra) == tree.GetHitObjects().end());

      // Move an object and update the tree
      objects[0]->m_hitBBox.Clear();
      tree.Update();
      CHECK(tree.GetObjectCount() == 16);

      tree.Finalize();
   }

   SUBCASE("begin/end reset rebuilds the object list")
   {
      const auto objects = MakeCircles(part, 4);
      HitCircle extra(part, Vertex2D(10.f, 10.f), 10.f, 0.f, 50.f);

      HitQuadtree tree;
      tree.Reset(ToPtrVector(objects));
      CHECK(tree.GetObjectCount() == 4);

      vector<HitObject*>& list = tree.BeginReset();
      list.clear();
      list.push_back(&extra);
      tree.EndReset();
      CHECK(tree.GetObjectCount() == 1);
      CHECK(tree.GetHitObjects()[0] == &extra);
   }

   SUBCASE("explicit bounds are accepted")
   {
      const auto objects = MakeCircles(part, 8);
      HitQuadtree tree;
      tree.SetBounds(FRect(0.f, 2000.f, 0.f, 2000.f));
      tree.Reset(ToPtrVector(objects));
      CHECK(tree.GetObjectCount() == 8);
      CHECK(tree.GetNLevels() >= 1);
   }

   part->Release();
}

// Intended behavior for the still-unfixed node-pool exhaustion defect:
// clustered items co-descend into a single quadrant level after level, burning
// ~4 nodes per level — far more than the 2n+1 initial pool estimate. Today
// AllocFourNodes silently returns nullptr and the tree stops subdividing early
// (correctness preserved, selectivity silently degraded).
TEST_CASE("Hit quadtree node pool grows on demand" * doctest::should_fail())
{
   Ball* const part = Ball::COMCreate();
   const auto objects = MakeClusters(part, 8);

   HitQuadtree tree;
   tree.SetBounds(FRect(0.f, 2000.f, 0.f, 2000.f));
   tree.Reset(ToPtrVector(objects));
   CHECK(tree.GetObjectCount() == 40);
   // The initial pool estimate is (2n+1) rounded to 4: 80 nodes for 40 items.
   // Natural subdivision needs far more; today the pool caps out and
   // subdivision is silently truncated instead of growing on demand.
   const size_t poolEstimate = (2 * objects.size() + 1) & ~size_t(3);
   CHECK(tree.GetNodeCount() > poolEstimate);
   tree.Finalize();

   part->Release();
}

TEST_CASE("Hit KD tree")
{
   SUBCASE("reset, insert, remove and update")
   {
      const auto objects = MakeCircles(nullptr, 16); // KD tree nodes do not dereference the editable
      vector<HitObject*> vho = ToPtrVector(objects);

      HitKD tree;
      tree.Reset(vho);
      CHECK(tree.GetObjectCount() == 16);
      CHECK(tree.GetNLevels() >= 1);
      CHECK(tree.GetHitObjects().size() == 16);

      HitCircle extra(nullptr, Vertex2D(5000.f, 5000.f), 10.f, 0.f, 50.f);
      extra.CalcHitBBox();
      tree.Insert(&extra);
      CHECK(tree.GetObjectCount() == 17);

      tree.Remove(&extra);
      CHECK(tree.GetObjectCount() == 16);
      CHECK(std::ranges::find(tree.GetHitObjects(), &extra) == tree.GetHitObjects().end());

      tree.Update();
      tree.Finalize();
   }
}

// ---------------------------------------------------------------------------
// Benchmark of the quadtree ball query, skipped by default. Run it with:
//   vpx-test --test-case="Benchmark: quadtree HitTestBall" --no-skip
// Table-like hit objects (circles and wall segments of varying sizes, owned by
// several parts so that nodes do not early out on a single unique part) queried
// by moving balls at random positions. Reports the best timing and a checksum
// of the results, which an optimization of the query must not change.
// ---------------------------------------------------------------------------

TEST_CASE("Benchmark: quadtree HitTestBall" * doctest::skip())
{
   PhysicsTestHarness harness;
   harness.Start();

   constexpr int nObjects = 3000, nBalls = 2000, nRounds = 200, nReps = 5;

   std::mt19937 rng(1234);
   auto uniform = [&rng](float lo, float hi) { return std::uniform_real_distribution<float>(lo, hi)(rng); };

   constexpr int nParts = 8;
   Ball *parts[nParts];
   for (Ball *&part : parts)
      part = Ball::COMCreate();

   vector<std::unique_ptr<HitObject>> objects;
   for (int i = 0; i < nObjects; ++i)
   {
      IEditable *const editable = parts[i % nParts];
      const Vertex2D p(uniform(0.f, 1000.f), uniform(0.f, 2000.f));
      if (i % 10 < 7)
         objects.push_back(std::make_unique<HitCircle>(editable, p, uniform(2.f, 25.f), 0.f, 50.f));
      else
      {
         const float angle = uniform(0.f, (float)(2.0 * M_PI)), length = uniform(20.f, 150.f);
         objects.push_back(std::make_unique<LineSeg>(editable, p, Vertex2D(p.x + length * cosf(angle), p.y + length * sinf(angle)), 0.f, 50.f));
      }
      objects.back()->m_physics = harness.GetEngine();
      objects.back()->CalcHitBBox();
   }

   HitQuadtree tree(harness.GetEngine());
   tree.SetBounds(FRect(0.f, 1000.f, 0.f, 2000.f));
   tree.Reset(ToPtrVector(objects));

   vector<HitBall> balls(nBalls);
   for (HitBall &ball : balls)
   {
      ball.m_d.m_pos = Vertex3Ds(uniform(0.f, 1000.f), uniform(0.f, 2000.f), 25.f);
      ball.m_d.m_vel = Vertex3Ds(uniform(-20.f, 20.f), uniform(-20.f, 20.f), 0.f);
      ball.CalcHitBBox();
   }

   double best = 1e30, checksum = 0.;
   int hits = 0;
   for (int rep = 0; rep < nReps; ++rep)
   {
      checksum = 0.;
      hits = 0;
      const auto t0 = std::chrono::steady_clock::now();
      for (int round = 0; round < nRounds; ++round)
         for (const HitBall &ball : balls)
         {
            CollisionEvent coll;
            coll.m_hittime = 1.f;
            coll.m_obj = nullptr;
            tree.HitTestBall(&ball, coll);
            if (coll.m_obj)
            {
               ++hits;
               checksum += coll.m_hittime;
            }
         }
      best = std::min(best, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count());
   }
   MESSAGE("quadtree HitTestBall: ", nBalls * nRounds, " queries, best of ", nReps, ": ", best, " ms, hits ", hits, ", checksum ", checksum);
   CHECK(hits > 0);

   tree.Finalize();
   for (Ball *part : parts)
      part->Release();
}
