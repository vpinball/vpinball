// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/ramp.h"
#include "math/dragpoint.h"
#include "physics/PhysicsEngine.h"

#include "doctest.h"

TEST_CASE("Ramp part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Ramp* const ramp = Ramp::COMCreate();
      ramp->Init(100.f, 200.f, false);
      ramp->SetName("Ramp1");
      ramp->m_d.m_heightbottom = 10.f;
      ramp->m_d.m_heighttop = 90.f;
      ramp->m_d.m_widthbottom = 80.f;
      ramp->m_d.m_widthtop = 40.f;
      ramp->m_d.m_type = RampType4Wire;
      ramp->m_d.m_imagealignment = ImageModeWrap;
      ramp->m_d.m_leftwallheight = 55.f;
      ramp->m_d.m_rightwallheight = 65.f;
      ramp->m_d.m_leftwallheightvisible = 35.f;
      ramp->m_d.m_rightwallheightvisible = 45.f;
      ramp->m_d.m_wireDiameter = 6.f;
      ramp->m_d.m_wireDistanceX = 30.f;
      ramp->m_d.m_wireDistanceY = 60.f;
      ramp->m_d.m_depthBias = 2.f;
      ramp->m_d.m_imageWalls = false;
      ramp->m_d.m_szImage = "ramp.png";
      ramp->m_d.m_szMaterial = "rampMat";
      ramp->m_d.m_szPhysicsMaterial = "physMat";
      ramp->m_d.m_visible = false;
      ramp->m_d.m_collidable = false;
      ramp->m_d.m_hitEvent = true;
      ramp->m_d.m_reflectionEnabled = false;
      table->AddPart(ramp);
      ramp->Release();

      const InMemStream saved = SavePartToStream(ramp);

      Ramp* const copy = Ramp::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetName() == "Ramp1");
      CHECK(copy->m_d.m_heightbottom == 10.f);
      CHECK(copy->m_d.m_heighttop == 90.f);
      CHECK(copy->m_d.m_widthbottom == 80.f);
      CHECK(copy->m_d.m_widthtop == 40.f);
      CHECK(copy->m_d.m_type == RampType4Wire);
      CHECK(copy->m_d.m_imagealignment == ImageModeWrap);
      CHECK(copy->m_d.m_leftwallheight == 55.f);
      CHECK(copy->m_d.m_rightwallheight == 65.f);
      CHECK(copy->m_d.m_leftwallheightvisible == 35.f);
      CHECK(copy->m_d.m_rightwallheightvisible == 45.f);
      CHECK(copy->m_d.m_wireDiameter == 6.f);
      CHECK(copy->m_d.m_wireDistanceX == 30.f);
      CHECK(copy->m_d.m_wireDistanceY == 60.f);
      CHECK(copy->m_d.m_depthBias == 2.f);
      CHECK(copy->m_d.m_imageWalls == false);
      CHECK(copy->m_d.m_szImage == "ramp.png");
      CHECK(copy->m_d.m_szMaterial == "rampMat");
      CHECK(copy->m_d.m_szPhysicsMaterial == "physMat");
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_collidable == false);
      CHECK(copy->m_d.m_hitEvent == true);
      CHECK(copy->m_d.m_reflectionEnabled == false);

      // The default 2-point ramp curve round-trips through the stream
      REQUIRE(copy->m_curve.GetPoints().size() == ramp->m_curve.GetPoints().size());
      CHECK(copy->m_curve.GetPoints()[0]->GetX() == doctest::Approx(ramp->m_curve.GetPoints()[0]->GetX()));
      CHECK(copy->m_curve.GetPoints()[0]->GetY() == doctest::Approx(ramp->m_curve.GetPoints()[0]->GetY()));
      CHECK(copy->m_curve.GetPoints()[0]->GetCalcHeight() == doctest::Approx(ramp->m_curve.GetPoints()[0]->GetCalcHeight()));

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Ramp* const ramp = Ramp::COMCreate();
      ramp->Init(50.f, 60.f, false);
      ramp->SetName("Ramp2");
      ramp->m_d.m_type = RampType1Wire;
      table->AddPart(ramp);
      ramp->Release();

      Ramp* const liveCopy = ramp->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(ramp), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("surface height interpolates along the curve")
   {
      Ramp* const ramp = Ramp::COMCreate();
      ramp->Init(0.f, 0.f, false);
      ramp->SetName("Ramp3");
      table->AddPart(ramp);
      ramp->Release();

      // Default ramp is a straight 2-point path: height at each end is the bottom/top height
      ramp->m_d.m_heightbottom = 10.f;
      ramp->m_d.m_heighttop = 90.f;
      const auto& points = ramp->m_curve.GetPoints();
      REQUIRE(points.size() == 2);
      const Vertex2D start(points[0]->GetX(), points[0]->GetY());
      const Vertex2D end(points[1]->GetX(), points[1]->GetY());

      CHECK(ramp->GetSurfaceHeight(start.x, start.y) == doctest::Approx(10.f));
      CHECK(ramp->GetSurfaceHeight(end.x, end.y) == doctest::Approx(90.f));
      CHECK(ramp->GetSurfaceHeight((start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f) == doctest::Approx(50.f).epsilon(0.05));
   }

   SUBCASE("degenerate curve is handled safely")
   {
      Ramp* const ramp = Ramp::COMCreate();
      ramp->Init(0.f, 0.f, false);
      ramp->SetName("Ramp5");
      ramp->m_d.m_heightbottom = 10.f;
      ramp->m_d.m_heighttop = 90.f;
      table->AddPart(ramp);
      ramp->Release();

      // Empty curve: height queries report "not on ramp" and AddPoint appends a control point
      ramp->m_curve.ClearPoints();
      CHECK(ramp->GetSurfaceHeight(0.f, 0.f) == 0.f);
      ramp->AddPoint(Vertex2D(50.f, 50.f), true);
      REQUIRE(ramp->m_curve.GetPoints().size() == 1);
      CHECK(ramp->m_curve.GetPoints()[0]->GetX() == doctest::Approx(50.f));
      CHECK(ramp->m_curve.GetPoints()[0]->GetY() == doctest::Approx(50.f));
      CHECK(ramp->m_curve.GetPoints()[0]->GetZ() == doctest::Approx(10.f));

      // Physics setup over an empty curve adds no collider and does not crash
      ramp->m_curve.ClearPoints();
      PhysicsEngine physics(table);
      ramp->PhysicSetup(&physics, false);

      // Coincident control points produce a zero-length curve: generated heights stay finite
      ramp->m_curve.PushPoint(std::make_unique<DragPoint>(&ramp->m_curve, 10.f, 10.f, 0.f, true));
      ramp->m_curve.PushPoint(std::make_unique<DragPoint>(&ramp->m_curve, 10.f, 10.f, 0.f, true));
      int cvertex = -1;
      float* heights = nullptr;
      const Vertex2D* const verts = ramp->GetRampVertex(cvertex, &heights, nullptr, nullptr, nullptr, -1.f, true);
      for (int i = 0; i < cvertex; i++)
         CHECK(std::isfinite(heights[i]));
      delete[] heights;
      delete[] verts;
   }

   SUBCASE("editor transforms move the curve")
   {
      Ramp* const ramp = Ramp::COMCreate();
      ramp->Init(100.f, 100.f, false);
      ramp->SetName("Ramp4");
      table->AddPart(ramp);
      ramp->Release();

      const Vertex2D center = ramp->GetCenter();
      ramp->Translate(Vertex2D(10.f, -10.f));
      CHECK(ramp->GetCenter().x == doctest::Approx(center.x + 10.f));
      CHECK(ramp->GetCenter().y == doctest::Approx(center.y - 10.f));
   }

   table->Release();
}
