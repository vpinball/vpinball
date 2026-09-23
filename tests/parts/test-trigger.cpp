// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/trigger.h"
#include "math/dragpoint.h"

#include "doctest.h"

TEST_CASE("Trigger part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Trigger* const trigger = Trigger::COMCreate();
      trigger->Init(100.f, 200.f, false);
      trigger->SetName(L"Trigger1");
      trigger->m_d.m_radius = 60.f;
      trigger->m_d.m_szSurface = "surface1";
      trigger->m_d.m_shape = TriggerStar;
      trigger->m_d.m_rotation = 15.f;
      trigger->m_d.m_scaleX = 1.5f;
      trigger->m_d.m_scaleY = 0.75f;
      trigger->m_d.m_hit_height = 60.f;
      trigger->m_d.m_animSpeed = 2.f;
      trigger->m_d.m_wireThickness = 3.f;
      trigger->m_d.m_enabled = false;
      trigger->m_d.m_szMaterial = "triggerMat";
      trigger->m_d.m_visible = false;
      trigger->m_d.m_reflectionEnabled = false;
      table->AddPart(trigger);
      trigger->Release();

      const InMemStream saved = SavePartToStream(trigger);

      Trigger* const copy = Trigger::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Trigger1");
      CHECK(copy->m_d.m_vCenter.x == 100.f);
      CHECK(copy->m_d.m_vCenter.y == 200.f);
      CHECK(copy->m_d.m_radius == 60.f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_shape == TriggerStar);
      CHECK(copy->m_d.m_rotation == 15.f);
      CHECK(copy->m_d.m_scaleX == 1.5f);
      CHECK(copy->m_d.m_scaleY == 0.75f);
      CHECK(copy->m_d.m_hit_height == 60.f);
      CHECK(copy->m_d.m_animSpeed == 2.f);
      CHECK(copy->m_d.m_wireThickness == 3.f);
      CHECK(copy->m_d.m_enabled == false);
      CHECK(copy->m_d.m_szMaterial == "triggerMat");
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_reflectionEnabled == false);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Trigger* const trigger = Trigger::COMCreate();
      trigger->Init(50.f, 60.f, false);
      trigger->SetName(L"Trigger2");
      trigger->m_d.m_shape = TriggerButton;
      table->AddPart(trigger);
      trigger->Release();

      Trigger* const liveCopy = trigger->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(trigger), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("shape changes rebuild the trigger outline")
   {
      Trigger* const trigger = Trigger::COMCreate();
      trigger->Init(0.f, 0.f, false);
      trigger->SetName(L"Trigger3");
      table->AddPart(trigger);
      trigger->Release();

      // Switching to a custom shape keeps the drag points that define the outline
      TriggerShape shape = TriggerNone;
      CHECK(trigger->get_TriggerShape(&shape) == S_OK);
      CHECK(trigger->put_TriggerShape(TriggerStar) == S_OK);
      CHECK(trigger->get_TriggerShape(&shape) == S_OK);
      CHECK(shape == TriggerStar);
   }

   SUBCASE("editor transforms move the curve")
   {
      Trigger* const trigger = Trigger::COMCreate();
      trigger->Init(10.f, 20.f, false);
      trigger->SetName(L"Trigger4");
      table->AddPart(trigger);
      trigger->Release();

      CHECK(trigger->GetCenter().x == 10.f);
      CHECK(trigger->GetCenter().y == 20.f);
      trigger->Translate(Vertex2D(-10.f, -20.f));
      CHECK(trigger->GetCenter().x == 0.f);
      CHECK(trigger->GetCenter().y == 0.f);
   }

   table->Release();
}
