// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/hittarget.h"

#include "doctest.h"

TEST_CASE("HitTarget part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      HitTarget* const target = HitTarget::COMCreate();
      target->Init(100.f, 200.f, false);
      target->SetName(L"Target1");
      target->m_d.m_vPosition.z = 30.f;
      target->m_d.m_vSize = Vertex3Ds(40.f, 50.f, 60.f);
      target->m_d.m_rotZ = 45.f;
      target->m_d.m_targetType = HitTargetRound;
      target->m_d.m_elasticityFalloff = 0.2f;
      target->m_d.m_dropSpeed = 0.5f;
      target->m_d.m_raiseDelay = 100;
      target->m_d.m_depthBias = 3.f;
      target->m_d.m_disableLightingTop = 0.4f;
      target->m_d.m_disableLightingBelow = 0.6f;
      target->m_d.m_legacy = true;
      target->m_d.m_isDropped = true;
      target->m_d.m_szImage = "target.png";
      target->m_d.m_szMaterial = "targetMat";
      target->m_d.m_szPhysicsMaterial = "physMat";
      target->m_d.m_visible = false;
      target->m_d.m_collidable = false;
      target->m_d.m_hitEvent = true;
      target->m_d.m_overwritePhysics = false;
      target->m_d.m_reflectionEnabled = false;
      table->AddPart(target);
      target->Release();

      const InMemStream saved = SavePartToStream(target);

      HitTarget* const copy = HitTarget::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Target1");
      CHECK(copy->m_d.m_vPosition.x == 100.f);
      CHECK(copy->m_d.m_vPosition.y == 200.f);
      CHECK(copy->m_d.m_vPosition.z == 30.f);
      CHECK(copy->m_d.m_vSize.x == 40.f);
      CHECK(copy->m_d.m_vSize.y == 50.f);
      CHECK(copy->m_d.m_vSize.z == 60.f);
      CHECK(copy->m_d.m_rotZ == 45.f);
      CHECK(copy->m_d.m_targetType == HitTargetRound);
      CHECK(copy->m_d.m_elasticityFalloff == 0.2f);
      CHECK(copy->m_d.m_dropSpeed == 0.5f);
      CHECK(copy->m_d.m_raiseDelay == 100);
      CHECK(copy->m_d.m_depthBias == 3.f);
      CHECK(copy->m_d.m_disableLightingTop == 0.4f);
      CHECK(copy->m_d.m_disableLightingBelow == 0.6f);
      CHECK(copy->m_d.m_legacy == true);
      CHECK(copy->m_d.m_isDropped == true);
      CHECK(copy->m_d.m_szImage == "target.png");
      CHECK(copy->m_d.m_szMaterial == "targetMat");
      CHECK(copy->m_d.m_szPhysicsMaterial == "physMat");
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_collidable == false);
      CHECK(copy->m_d.m_hitEvent == true);
      CHECK(copy->m_d.m_overwritePhysics == false);
      CHECK(copy->m_d.m_reflectionEnabled == false);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      HitTarget* const target = HitTarget::COMCreate();
      target->Init(50.f, 60.f, false);
      target->SetName(L"Target2");
      target->m_d.m_targetType = HitTargetRectangle;
      table->AddPart(target);
      target->Release();

      HitTarget* const liveCopy = target->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(target), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("translation updates the transformed UI vertices")
   {
      HitTarget* const target = HitTarget::COMCreate();
      target->Init(10.f, 20.f, false);
      target->SetName(L"Target3");
      target->m_d.m_targetType = HitTargetRectangle;
      target->m_d.m_vSize = Vertex3Ds(10.f, 20.f, 30.f);
      target->m_d.m_rotZ = 0.f;
      target->TransformVertices();
      table->AddPart(target);
      target->Release();

      // The editor wireframe is built from the transformed vertices: translating
      // the target must shift the wireframe by the same offset
      vector<Vertex2D> before;
      target->GetEditorWireframe(before);
      REQUIRE_FALSE(before.empty());

      target->Translate(Vertex2D(5.f, -2.f));

      CHECK(target->m_d.m_vPosition.x == 15.f);
      CHECK(target->m_d.m_vPosition.y == 18.f);
      CHECK(target->GetCenter().x == 15.f);
      CHECK(target->GetCenter().y == 18.f);

      vector<Vertex2D> after;
      target->GetEditorWireframe(after);
      REQUIRE(after.size() == before.size());
      CHECK(after[0].x == doctest::Approx(before[0].x + 5.f));
      CHECK(after[0].y == doctest::Approx(before[0].y - 2.f));
   }

   table->Release();
}
