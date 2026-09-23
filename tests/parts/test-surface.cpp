// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/surface.h"
#include "parts/dragpoint.h"

#include "doctest.h"

TEST_CASE("Surface part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Surface* const surface = Surface::COMCreate();
      surface->Init(100.f, 200.f, false);
      surface->SetName(L"Wall1");
      surface->m_d.m_slingshot_threshold = 0.9f;
      surface->m_d.m_szSideImage = "side.png";
      surface->m_d.m_szImage = "top.png";
      surface->m_d.m_szTopMaterial = "topMat";
      surface->m_d.m_szSideMaterial = "sideMat";
      surface->m_d.m_szSlingShotMaterial = "slingMat";
      surface->m_d.m_szPhysicsMaterial = "physMat";
      surface->m_d.m_heightbottom = 5.f;
      surface->m_d.m_heighttop = 55.f;
      surface->m_d.m_slingshotforce = 4.f;
      surface->m_d.m_disableLightingTop = 0.3f;
      surface->m_d.m_disableLightingBelow = 0.7f;
      surface->m_d.m_elasticityFalloff = 0.25f;
      surface->m_d.m_droppable = true;
      surface->m_d.m_flipbook = true;
      surface->m_d.m_displayTexture = false;
      surface->m_d.m_sideVisible = false;
      // surface->m_d.m_enabled = ...; // TODO refactor unused property
      surface->m_d.m_isBottomSolid = false;
      surface->m_d.m_slingshotAnimation = false;
      surface->m_d.m_topBottomVisible = false;
      // surface->m_d.m_visible = ...; // TODO refactor unused property
      surface->m_d.m_collidable = false;
      surface->m_d.m_hitEvent = true;
      surface->m_d.m_overwritePhysics = false;
      surface->m_d.m_reflectionEnabled = false;
      table->AddPart(surface);
      surface->Release();

      const InMemStream saved = SavePartToStream(surface);

      Surface* const copy = Surface::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Wall1");
      CHECK(copy->m_d.m_slingshot_threshold == 0.9f);
      CHECK(copy->m_d.m_szSideImage == "side.png");
      CHECK(copy->m_d.m_szImage == "top.png");
      CHECK(copy->m_d.m_szTopMaterial == "topMat");
      CHECK(copy->m_d.m_szSideMaterial == "sideMat");
      CHECK(copy->m_d.m_szSlingShotMaterial == "slingMat");
      CHECK(copy->m_d.m_szPhysicsMaterial == "physMat");
      CHECK(copy->m_d.m_heightbottom == 5.f);
      CHECK(copy->m_d.m_heighttop == 55.f);
      CHECK(copy->m_d.m_slingshotforce == 4.f);
      CHECK(copy->m_d.m_disableLightingTop == 0.3f);
      CHECK(copy->m_d.m_disableLightingBelow == 0.7f);
      CHECK(copy->m_d.m_elasticityFalloff == 0.25f);
      CHECK(copy->m_d.m_droppable == true);
      CHECK(copy->m_d.m_flipbook == true);
      CHECK(copy->m_d.m_displayTexture == false);
      CHECK(copy->m_d.m_sideVisible == false);
      CHECK(copy->m_d.m_isBottomSolid == false);
      CHECK(copy->m_d.m_slingshotAnimation == false);
      CHECK(copy->m_d.m_topBottomVisible == false);
      CHECK(copy->m_d.m_collidable == false);
      CHECK(copy->m_d.m_hitEvent == true);
      CHECK(copy->m_d.m_overwritePhysics == false);
      CHECK(copy->m_d.m_reflectionEnabled == false);

      // The wall outline round-trips through the stream
      REQUIRE(copy->m_curve.GetPoints().size() == surface->m_curve.GetPoints().size());
      CHECK(copy->m_curve.GetPoints()[0]->m_v.x == doctest::Approx(surface->m_curve.GetPoints()[0]->m_v.x));
      CHECK(copy->m_curve.GetPoints()[0]->m_v.y == doctest::Approx(surface->m_curve.GetPoints()[0]->m_v.y));

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Surface* const surface = Surface::COMCreate();
      surface->Init(50.f, 60.f, false);
      surface->SetName(L"Wall2");
      surface->m_d.m_droppable = true;
      table->AddPart(surface);
      surface->Release();

      Surface* const liveCopy = surface->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(surface), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms move the curve")
   {
      Surface* const surface = Surface::COMCreate();
      surface->Init(100.f, 100.f, false);
      surface->SetName(L"Wall3");
      table->AddPart(surface);
      surface->Release();

      const Vertex2D center = surface->GetCenter();
      surface->Translate(Vertex2D(10.f, 20.f));
      CHECK(surface->GetCenter().x == doctest::Approx(center.x + 10.f));
      CHECK(surface->GetCenter().y == doctest::Approx(center.y + 20.f));
   }

   table->Release();
}
