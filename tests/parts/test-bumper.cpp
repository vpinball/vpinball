// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/bumper.h"

#include "doctest.h"

TEST_CASE("Bumper part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(100.f, 200.f, false);
      bumper->SetName(L"Bumper1");
      bumper->m_d.m_radius = 45.f;
      bumper->m_d.m_force = 12.5f;
      bumper->m_d.m_heightScale = 90.f;
      bumper->m_d.m_orientation = 30.f;
      bumper->m_d.m_ringSpeed = 0.5f;
      bumper->m_d.m_ringDropOffset = 15.f;
      bumper->m_d.m_szCapMaterial = "capMat";
      bumper->m_d.m_szBaseMaterial = "baseMat";
      bumper->m_d.m_szSkirtMaterial = "skirtMat";
      bumper->m_d.m_szRingMaterial = "ringMat";
      bumper->m_d.m_szSurface = "surface1";
      bumper->m_d.m_capVisible = false;
      bumper->m_d.m_baseVisible = false;
      bumper->m_d.m_ringVisible = false;
      bumper->m_d.m_skirtVisible = false;
      bumper->m_d.m_collidable = false;
      bumper->m_d.m_reflectionEnabled = false;
      bumper->m_d.m_hitEvent = true;
      bumper->m_d.m_threshold = 2.5f;
      bumper->m_timerEnabled = true;
      bumper->m_timerInterval = 123;
      // bumper->m_d.m_szImage = ...; // TODO refactor unused property
      // bumper->m_d.m_visible = ...; // TODO refactor unused property
      table->AddPart(bumper);
      bumper->Release();

      const InMemStream saved = SavePartToStream(bumper);

      Bumper* const copy = Bumper::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Bumper1");
      CHECK(copy->m_d.m_vCenter.x == 100.f);
      CHECK(copy->m_d.m_vCenter.y == 200.f);
      CHECK(copy->m_d.m_radius == 45.f);
      CHECK(copy->m_d.m_force == 12.5f);
      CHECK(copy->m_d.m_heightScale == 90.f);
      CHECK(copy->m_d.m_orientation == 30.f);
      CHECK(copy->m_d.m_ringSpeed == 0.5f);
      CHECK(copy->m_d.m_ringDropOffset == 15.f);
      CHECK(copy->m_d.m_szCapMaterial == "capMat");
      CHECK(copy->m_d.m_szBaseMaterial == "baseMat");
      CHECK(copy->m_d.m_szSkirtMaterial == "skirtMat");
      CHECK(copy->m_d.m_szRingMaterial == "ringMat");
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_capVisible == false);
      CHECK(copy->m_d.m_baseVisible == false);
      CHECK(copy->m_d.m_ringVisible == false);
      CHECK(copy->m_d.m_skirtVisible == false);
      CHECK(copy->m_d.m_collidable == false);
      CHECK(copy->m_d.m_reflectionEnabled == false);
      CHECK(copy->m_d.m_hitEvent == true);
      CHECK(copy->m_d.m_threshold == 2.5f);
      CHECK(copy->m_timerEnabled == true);
      CHECK(copy->m_timerInterval == 123);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(50.f, 60.f, false);
      bumper->SetName(L"Bumper2");
      bumper->m_d.m_radius = 25.f;
      bumper->m_d.m_ringVisible = false;
      table->AddPart(bumper);
      bumper->Release();

      Bumper* const liveCopy = bumper->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(bumper), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Bumper* const bumper = Bumper::COMCreate();
      bumper->Init(10.f, 20.f, false);
      bumper->SetName(L"Bumper3");
      table->AddPart(bumper);
      bumper->Release();

      CHECK(bumper->GetCenter().x == 10.f);
      CHECK(bumper->GetCenter().y == 20.f);
      bumper->Translate(Vertex2D(1.f, -2.f));
      CHECK(bumper->GetCenter().x == 11.f);
      CHECK(bumper->GetCenter().y == 18.f);
   }

   table->Release();
}
