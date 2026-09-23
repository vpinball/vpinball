// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/flasher.h"

#include "doctest.h"

TEST_CASE("Flasher part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Flasher* const flasher = Flasher::COMCreate();
      flasher->Init(100.f, 200.f, false);
      flasher->SetName(L"Flasher1");
      flasher->m_d.m_color = RGB(255, 64, 32);
      flasher->m_d.m_height = 60.f;
      flasher->m_d.m_rotX = 10.f;
      flasher->m_d.m_rotY = 20.f;
      flasher->m_d.m_rotZ = 30.f;
      flasher->m_d.m_szImageA = "flashA.png";
      flasher->m_d.m_szImageB = "flashB.png";
      flasher->m_d.m_alpha = 50;
      flasher->m_d.m_intensity_scale = 1.5f;
      flasher->m_d.m_modulate_vs_add = 0.7f;
      flasher->m_d.m_addBlend = FlasherData::AB_ADD;
      flasher->m_d.m_renderMode = FlasherData::DMD;
      flasher->m_d.m_renderStyle = 2;
      flasher->m_d.m_imageSrcLink = "src:display";
      flasher->m_d.m_glassRoughness = 0.4f;
      flasher->m_d.m_glassAmbient = RGB(20, 30, 40);
      flasher->m_d.m_glassPadTop = 1.f;
      flasher->m_d.m_glassPadBottom = 2.f;
      flasher->m_d.m_glassPadLeft = 3.f;
      flasher->m_d.m_glassPadRight = 4.f;
      flasher->m_d.m_filterAmount = 100;
      flasher->m_d.m_filter = Filter_Multiply;
      flasher->m_d.m_imagealignment = ImageModeWrap;
      flasher->m_d.m_displayTexture = true;
      flasher->m_d.m_isVisible = false;
      flasher->m_d.m_depthBias = 5.f;
      flasher->m_d.m_szLightmap = "lm.png";
      table->AddPart(flasher);
      flasher->Release();

      const InMemStream saved = SavePartToStream(flasher);

      Flasher* const copy = Flasher::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Flasher1");
      CHECK(copy->m_d.m_color == RGB(255, 64, 32));
      CHECK(copy->m_d.m_height == 60.f);
      CHECK(copy->m_d.m_rotX == 10.f);
      CHECK(copy->m_d.m_rotY == 20.f);
      CHECK(copy->m_d.m_rotZ == 30.f);
      CHECK(copy->m_d.m_szImageA == "flashA.png");
      CHECK(copy->m_d.m_szImageB == "flashB.png");
      CHECK(copy->m_d.m_alpha == 50);
      // CHECK(copy->m_d.m_intensity_scale == 1.5f); // TODO m_intensity_scale should not be part of the object data model as this is only a live render state
      CHECK(copy->m_d.m_modulate_vs_add == 0.7f);
      CHECK(copy->m_d.m_addBlend == FlasherData::AB_ADD);
      CHECK(copy->m_d.m_renderMode == FlasherData::DMD);
      CHECK(copy->m_d.m_renderStyle == 2);
      CHECK(copy->m_d.m_imageSrcLink == "src:display");
      CHECK(copy->m_d.m_glassRoughness == 0.4f);
      CHECK(copy->m_d.m_glassAmbient == RGB(20, 30, 40));
      CHECK(copy->m_d.m_glassPadTop == 1.f);
      CHECK(copy->m_d.m_glassPadBottom == 2.f);
      CHECK(copy->m_d.m_glassPadLeft == 3.f);
      CHECK(copy->m_d.m_glassPadRight == 4.f);
      CHECK(copy->m_d.m_filterAmount == 100);
      CHECK(copy->m_d.m_filter == Filter_Multiply);
      CHECK(copy->m_d.m_imagealignment == ImageModeWrap);
      CHECK(copy->m_d.m_displayTexture == true);
      CHECK(copy->m_d.m_isVisible == false);
      CHECK(copy->m_d.m_depthBias == 5.f);
      CHECK(copy->m_d.m_szLightmap == "lm.png");

      // The default flasher shape is a 100x100 quad centered on the init position
      REQUIRE(copy->m_curve.GetPoints().size() == flasher->m_curve.GetPoints().size());
      CHECK(copy->m_curve.GetPoints()[0]->GetX() == flasher->m_curve.GetPoints()[0]->GetX());
      CHECK(copy->m_curve.GetPoints()[0]->GetY() == flasher->m_curve.GetPoints()[0]->GetY());

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Flasher* const flasher = Flasher::COMCreate();
      flasher->Init(50.f, 60.f, false);
      flasher->SetName(L"Flasher2");
      flasher->m_d.m_alpha = 10;
      table->AddPart(flasher);
      flasher->Release();

      Flasher* const liveCopy = flasher->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(flasher), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("editor transforms move the curve")
   {
      Flasher* const flasher = Flasher::COMCreate();
      flasher->Init(100.f, 100.f, false);
      flasher->SetName(L"Flasher3");
      table->AddPart(flasher);
      flasher->Release();

      CHECK(flasher->GetCenter().x == doctest::Approx(100.f));
      CHECK(flasher->GetCenter().y == doctest::Approx(100.f));
      flasher->Translate(Vertex2D(-100.f, -100.f));
      CHECK(flasher->GetCenter().x == doctest::Approx(0.f));
      CHECK(flasher->GetCenter().y == doctest::Approx(0.f));
   }

   table->Release();
}
