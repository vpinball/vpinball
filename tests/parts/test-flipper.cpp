// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/flipper.h"

#include "doctest.h"

TEST_CASE("Flipper part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Flipper* const flipper = Flipper::COMCreate();
      flipper->Init(100.f, 200.f, false);
      flipper->SetName(L"Flipper1");
      flipper->m_d.m_BaseRadius = 20.f;
      flipper->m_d.m_EndRadius = 12.f;
      flipper->m_d.m_FlipperRadiusMin = 100.f;
      flipper->m_d.m_FlipperRadiusMax = 140.f;
      flipper->m_d.m_FlipperRadius = 120.f;
      flipper->m_d.m_StartAngle = 30.f;
      flipper->m_d.m_EndAngle = -30.f;
      flipper->m_d.m_height = 55.f;
      flipper->m_d.m_szSurface = "surface1";
      // flipper->m_d.m_color = ...; // TODO refactor unused property
      // flipper->m_d.m_rubbercolor = ...; // TODO refactor unused property
      flipper->m_d.m_szRubberMaterial = "rubberMat";
      flipper->m_d.m_rubberthickness = 8.f;
      flipper->m_d.m_rubberheight = 19.f;
      flipper->m_d.m_rubberwidth = 24.f;
      flipper->m_d.m_mass = 1.5f;
      flipper->m_d.m_strength = 2200.f;
      flipper->m_d.m_elasticityFalloff = 0.43f;
      flipper->m_d.m_return = 0.058f;
      flipper->m_d.m_rampUp = 3.f;
      flipper->m_d.m_torqueDamping = 0.75f;
      flipper->m_d.m_torqueDampingAngle = 6.f;
      flipper->m_d.m_OverridePhysics = 2;
      flipper->m_d.m_OverrideMass = 2.f;
      flipper->m_d.m_OverrideStrength = 3000.f;
      flipper->m_d.m_OverrideElasticity = 0.9f;
      flipper->m_d.m_OverrideElasticityFalloff = 0.2f;
      flipper->m_d.m_OverrideFriction = 0.8f;
      flipper->m_d.m_OverrideReturnStrength = 0.1f;
      flipper->m_d.m_OverrideCoilRampUp = 4.f;
      flipper->m_d.m_OverrideTorqueDamping = 0.6f;
      flipper->m_d.m_OverrideTorqueDampingAngle = 5.f;
      flipper->m_d.m_OverrideScatterAngle = 1.f;
      flipper->m_d.m_enabled = false;
      flipper->m_d.m_visible = false;
      flipper->m_d.m_szImage = "flipper.png";
      flipper->m_d.m_szMaterial = "flipperMat";
      // flipper->m_d.m_szPhysicsMaterial = ...; // TODO refactor unused property
      // flipper->m_d.m_overwritePhysics = ...; // TODO refactor unused property
      table->AddPart(flipper);
      flipper->Release();

      const InMemStream saved = SavePartToStream(flipper);

      Flipper* const copy = Flipper::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Flipper1");
      CHECK(copy->m_d.m_Center.x == 100.f);
      CHECK(copy->m_d.m_Center.y == 200.f);
      CHECK(copy->m_d.m_BaseRadius == 20.f);
      CHECK(copy->m_d.m_EndRadius == 12.f);
      CHECK(copy->m_d.m_FlipperRadiusMin == 100.f);
      CHECK(copy->m_d.m_FlipperRadiusMax == 140.f);
      // CHECK(copy->m_d.m_FlipperRadius == 120.f); // TODO this is not a part defined value but a computed live property, derived from table difficulty
      CHECK(copy->m_d.m_StartAngle == 30.f);
      CHECK(copy->m_d.m_EndAngle == -30.f);
      CHECK(copy->m_d.m_height == 55.f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_szRubberMaterial == "rubberMat");
      CHECK(copy->m_d.m_rubberthickness == 8.f);
      CHECK(copy->m_d.m_rubberheight == 19.f);
      CHECK(copy->m_d.m_rubberwidth == 24.f);
      CHECK(copy->m_d.m_mass == 1.5f);
      CHECK(copy->m_d.m_strength == 2200.f);
      CHECK(copy->m_d.m_elasticityFalloff == 0.43f);
      CHECK(copy->m_d.m_return == 0.058f);
      CHECK(copy->m_d.m_rampUp == 3.f);
      CHECK(copy->m_d.m_torqueDamping == 0.75f);
      CHECK(copy->m_d.m_torqueDampingAngle == 6.f);
      // CHECK(copy->m_d.m_OverridePhysics == 2); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideMass == 2.f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideStrength == 3000.f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideElasticity == 0.9f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideElasticityFalloff == 0.2f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideFriction == 0.8f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideReturnStrength == 0.1f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideCoilRampUp == 4.f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideTorqueDamping == 0.6f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideTorqueDampingAngle == 5.f); // TODO this is not a part defined value but a computed live property, derived from table settings
      // CHECK(copy->m_d.m_OverrideScatterAngle == 1.f); // TODO this is not a part defined value but a computed live property, derived from table settings
      CHECK(copy->m_d.m_enabled == false);
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_szImage == "flipper.png");
      CHECK(copy->m_d.m_szMaterial == "flipperMat");

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Flipper* const flipper = Flipper::COMCreate();
      flipper->Init(50.f, 60.f, false);
      flipper->SetName(L"Flipper2");
      flipper->m_d.m_StartAngle = 121.f;
      table->AddPart(flipper);
      flipper->Release();

      Flipper* const liveCopy = flipper->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(flipper), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Flipper* const flipper = Flipper::COMCreate();
      flipper->Init(10.f, 20.f, false);
      flipper->SetName(L"Flipper3");
      table->AddPart(flipper);
      flipper->Release();

      CHECK(flipper->GetCenter().x == 10.f);
      CHECK(flipper->GetCenter().y == 20.f);
      flipper->Translate(Vertex2D(7.f, -3.f));
      CHECK(flipper->GetCenter().x == 17.f);
      CHECK(flipper->GetCenter().y == 17.f);
   }

   table->Release();
}
