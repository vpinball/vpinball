// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/rubber.h"
#include "math/dragpoint.h"

#include "doctest.h"

TEST_CASE("Rubber part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Rubber* const rubber = Rubber::COMCreate();
      rubber->Init(100.f, 200.f, false);
      rubber->SetName(L"Rubber1");
      rubber->m_d.m_height = 30.f;
      rubber->m_d.m_hitHeight = 25.f;
      rubber->m_d.m_thickness = 12;
      rubber->m_d.m_elasticityFalloff = 0.35f;
      rubber->m_d.m_rotX = 5.f;
      rubber->m_d.m_rotY = 10.f;
      rubber->m_d.m_rotZ = 15.f;
      rubber->m_d.m_staticRendering = true;
      rubber->m_d.m_showInEditor = false;
      rubber->m_d.m_szImage = "rubber.png";
      rubber->m_d.m_szMaterial = "rubberMat";
      rubber->m_d.m_szPhysicsMaterial = "physMat";
      rubber->m_d.m_visible = false;
      rubber->m_d.m_collidable = false;
      rubber->m_d.m_hitEvent = true;
      rubber->m_d.m_overwritePhysics = false;
      rubber->m_d.m_reflectionEnabled = false;
      table->AddPart(rubber);
      rubber->Release();

      const InMemStream saved = SavePartToStream(rubber);

      Rubber* const copy = Rubber::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Rubber1");
      CHECK(copy->m_d.m_height == 30.f);
      CHECK(copy->m_d.m_hitHeight == 25.f);
      CHECK(copy->m_d.m_thickness == 12);
      CHECK(copy->m_d.m_elasticityFalloff == 0.35f);
      CHECK(copy->m_d.m_rotX == 5.f);
      CHECK(copy->m_d.m_rotY == 10.f);
      CHECK(copy->m_d.m_rotZ == 15.f);
      CHECK(copy->m_d.m_staticRendering == true);
      CHECK(copy->m_d.m_showInEditor == false);
      CHECK(copy->m_d.m_szImage == "rubber.png");
      CHECK(copy->m_d.m_szMaterial == "rubberMat");
      CHECK(copy->m_d.m_szPhysicsMaterial == "physMat");
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_collidable == false);
      CHECK(copy->m_d.m_hitEvent == true);
      CHECK(copy->m_d.m_overwritePhysics == false);
      CHECK(copy->m_d.m_reflectionEnabled == false);

      // The default 8-point rubber ring round-trips through the stream
      REQUIRE(copy->m_curve.GetPoints().size() == rubber->m_curve.GetPoints().size());
      CHECK(copy->m_curve.GetPoints()[0]->GetX() == doctest::Approx(rubber->m_curve.GetPoints()[0]->GetX()));
      CHECK(copy->m_curve.GetPoints()[0]->GetY() == doctest::Approx(rubber->m_curve.GetPoints()[0]->GetY()));

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Rubber* const rubber = Rubber::COMCreate();
      rubber->Init(50.f, 60.f, false);
      rubber->SetName(L"Rubber2");
      rubber->m_d.m_thickness = 4;
      table->AddPart(rubber);
      rubber->Release();

      Rubber* const liveCopy = rubber->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(rubber), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms move the curve")
   {
      Rubber* const rubber = Rubber::COMCreate();
      rubber->Init(100.f, 100.f, false);
      rubber->SetName(L"Rubber3");
      table->AddPart(rubber);
      rubber->Release();

      CHECK(rubber->GetCenter().x == doctest::Approx(100.f));
      CHECK(rubber->GetCenter().y == doctest::Approx(100.f));
      rubber->Translate(Vertex2D(-50.f, 50.f));
      CHECK(rubber->GetCenter().x == doctest::Approx(50.f));
      CHECK(rubber->GetCenter().y == doctest::Approx(150.f));
   }

   table->Release();
}
