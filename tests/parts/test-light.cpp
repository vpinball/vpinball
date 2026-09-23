// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/light.h"

#include "doctest.h"

TEST_CASE("Light part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Light* const light = Light::COMCreate();
      light->Init(100.f, 200.f, false);
      light->SetName(L"Light1");
      light->m_d.m_height = 40.f;
      light->m_d.m_szSurface = "surface1";
      light->m_d.m_falloff = 60.f;
      light->m_d.m_falloff_power = 2.5f;
      light->m_d.m_intensity = 3.f;
      light->m_d.m_fadeSpeedUp = 0.2f;
      light->m_d.m_fadeSpeedDown = 0.3f;
      light->m_d.m_state = 1.f;
      light->m_d.m_rgblinkpattern = "pattern01";
      light->m_d.m_blinkinterval = 125;
      light->m_d.m_color = RGB(255, 200, 100);
      light->m_d.m_color2 = RGB(10, 20, 30);
      light->m_d.m_shape = ShapeCustom;
      light->m_d.m_depthBias = 4.f;
      light->m_d.m_showReflectionOnBall = false;
      light->m_d.m_imageMode = true;
      light->m_d.m_BulbLight = true;
      light->m_d.m_bulbHaloHeight = 35.f;
      light->m_d.m_transmissionScale = 0.8f;
      light->m_d.m_modulate_vs_add = 0.99f;
      light->m_d.m_showBulbMesh = true;
      light->m_d.m_staticBulbMesh = true;
      light->m_d.m_meshRadius = 22.f;
      light->m_d.m_shadows = RAYTRACED_BALL_SHADOWS;
      light->m_d.m_fader = FADER_INCANDESCENT;
      light->m_d.m_szImage = "light.png";
      light->m_d.m_visible = false;
      light->m_timerEnabled = true;
      light->m_timerInterval = 55;
      table->AddPart(light);
      light->Release();

      const InMemStream saved = SavePartToStream(light);

      Light* const copy = Light::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Light1");
      CHECK(copy->m_d.m_vCenter.x == 100.f);
      CHECK(copy->m_d.m_vCenter.y == 200.f);
      CHECK(copy->m_d.m_height == 40.f);
      CHECK(copy->m_d.m_szSurface == "surface1");
      CHECK(copy->m_d.m_falloff == 60.f);
      CHECK(copy->m_d.m_falloff_power == 2.5f);
      CHECK(copy->m_d.m_intensity == 3.f);
      CHECK(copy->m_d.m_fadeSpeedUp == 0.2f);
      CHECK(copy->m_d.m_fadeSpeedDown == 0.3f);
      CHECK(copy->m_d.m_state == 1.f);
      CHECK(copy->m_d.m_rgblinkpattern == "pattern01");
      CHECK(copy->m_d.m_blinkinterval == 125);
      CHECK(copy->m_d.m_color == RGB(255, 200, 100));
      CHECK(copy->m_d.m_color2 == RGB(10, 20, 30));
      CHECK(copy->m_d.m_shape == ShapeCustom);
      CHECK(copy->m_d.m_depthBias == 4.f);
      CHECK(copy->m_d.m_showReflectionOnBall == false);
      CHECK(copy->m_d.m_imageMode == true);
      CHECK(copy->m_d.m_BulbLight == true);
      CHECK(copy->m_d.m_bulbHaloHeight == 35.f);
      CHECK(copy->m_d.m_transmissionScale == 0.8f);
      CHECK(copy->m_d.m_modulate_vs_add == 0.99f);
      CHECK(copy->m_d.m_showBulbMesh == true);
      CHECK(copy->m_d.m_staticBulbMesh == true);
      CHECK(copy->m_d.m_meshRadius == 22.f);
      CHECK(copy->m_d.m_shadows == RAYTRACED_BALL_SHADOWS);
      CHECK(copy->m_d.m_fader == FADER_INCANDESCENT);
      CHECK(copy->m_d.m_szImage == "light.png");
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_timerEnabled == true);
      CHECK(copy->m_timerInterval == 55);

      // The custom shape's drag points round-trip through the stream
      REQUIRE(copy->m_curve.GetPoints().size() == light->m_curve.GetPoints().size());
      CHECK(copy->m_curve.GetPoints()[0]->m_v.x == doctest::Approx(light->m_curve.GetPoints()[0]->m_v.x));
      CHECK(copy->m_curve.GetPoints()[0]->m_v.y == doctest::Approx(light->m_curve.GetPoints()[0]->m_v.y));

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Light* const light = Light::COMCreate();
      light->Init(50.f, 60.f, false);
      light->SetName(L"Light2");
      light->m_d.m_intensity = 7.f;
      table->AddPart(light);
      light->Release();

      Light* const liveCopy = light->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(light), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms")
   {
      Light* const light = Light::COMCreate();
      light->Init(10.f, 20.f, false);
      light->SetName(L"Light3");
      table->AddPart(light);
      light->Release();

      CHECK(light->GetCenter().x == 10.f);
      CHECK(light->GetCenter().y == 20.f);
      light->Translate(Vertex2D(1.f, 1.f));
      CHECK(light->GetCenter().x == 11.f);
      CHECK(light->GetCenter().y == 21.f);
   }

   table->Release();
}
