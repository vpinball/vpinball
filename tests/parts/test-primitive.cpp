// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/primitive.h"

#include "doctest.h"

TEST_CASE("Primitive part")
{
   PinTable* const table = CreateTestTable();

   SUBCASE("BIFF save/load round-trip")
   {
      Primitive* const prim = Primitive::COMCreate();
      prim->Init(100.f, 200.f, false);
      prim->SetName(L"Prim1");
      prim->m_d.m_Sides = 6;
      prim->m_d.m_vPosition.z = 10.f;
      prim->m_d.m_vSize = Vertex3Ds(120.f, 130.f, 140.f);
      for (int i = 0; i < 9; i++)
         prim->m_d.m_aRotAndTra[i] = static_cast<float>(i) * 0.5f;
      prim->m_d.m_szNormalMap = "normal.png";
      prim->m_d.m_SideColor = RGB(1, 2, 3);
      prim->m_d.m_elasticityFalloff = 0.11f;
      prim->m_d.m_depthBias = 7.f;
      prim->m_d.m_edgeFactorUI = 0.3f;
      prim->m_d.m_collision_reductionFactor = 0.5f;
      prim->m_d.m_disableLightingTop = 0.2f;
      prim->m_d.m_disableLightingBelow = 0.8f;
      prim->m_d.m_drawTexturesInside = true;
      prim->m_d.m_staticRendering = true;
      prim->m_d.m_useDepthMask = false;
      prim->m_d.m_addBlend = true;
      prim->m_d.m_color = RGB(9, 8, 7);
      prim->m_d.m_alpha = 0.5f;
      prim->m_d.m_szLightmap = "lm.png";
      prim->m_d.m_szReflectionProbe = "reflProbe";
      prim->m_d.m_reflectionStrength = 0.6f;
      prim->m_d.m_szRefractionProbe = "refrProbe";
      prim->m_d.m_refractionThickness = 15.f;
      prim->m_d.m_toy = true;
      prim->m_d.m_backfacesEnabled = true;
      prim->m_d.m_displayTexture = true;
      prim->m_d.m_objectSpaceNormalMap = true;
      prim->m_d.m_szImage = "prim.png";
      prim->m_d.m_szMaterial = "primMat";
      prim->m_d.m_szPhysicsMaterial = "physMat";
      prim->m_d.m_visible = false;
      prim->m_d.m_collidable = false;
      prim->m_d.m_hitEvent = true;
      prim->m_d.m_reflectionEnabled = false;
      table->AddPart(prim);
      prim->Release();

      const InMemStream saved = SavePartToStream(prim);

      Primitive* const copy = Primitive::COMCreate();
      LoadPartFromStream(copy, saved);

      CHECK(copy->GetWName() == L"Prim1");
      CHECK(copy->m_d.m_Sides == 6);
      CHECK(copy->m_d.m_vPosition.x == 100.f);
      CHECK(copy->m_d.m_vPosition.y == 200.f);
      CHECK(copy->m_d.m_vPosition.z == 10.f);
      CHECK(copy->m_d.m_vSize.x == 120.f);
      CHECK(copy->m_d.m_vSize.y == 130.f);
      CHECK(copy->m_d.m_vSize.z == 140.f);
      for (int i = 0; i < 9; i++)
         CHECK(copy->m_d.m_aRotAndTra[i] == static_cast<float>(i) * 0.5f);
      CHECK(copy->m_d.m_szNormalMap == "normal.png");
      CHECK(copy->m_d.m_SideColor == RGB(1, 2, 3));
      CHECK(copy->m_d.m_elasticityFalloff == 0.11f);
      CHECK(copy->m_d.m_depthBias == 7.f);
      CHECK(copy->m_d.m_edgeFactorUI == 0.3f);
      CHECK(copy->m_d.m_collision_reductionFactor == 0.5f);
      CHECK(copy->m_d.m_disableLightingTop == 0.2f);
      CHECK(copy->m_d.m_disableLightingBelow == 0.8f);
      CHECK(copy->m_d.m_drawTexturesInside == true);
      CHECK(copy->m_d.m_staticRendering == true);
      CHECK(copy->m_d.m_useDepthMask == false);
      CHECK(copy->m_d.m_addBlend == true);
      CHECK(copy->m_d.m_color == RGB(9, 8, 7));
      CHECK(copy->m_d.m_alpha == 0.5f);
      CHECK(copy->m_d.m_szLightmap == "lm.png");
      CHECK(copy->m_d.m_szReflectionProbe == "reflProbe");
      CHECK(copy->m_d.m_reflectionStrength == 0.6f);
      CHECK(copy->m_d.m_szRefractionProbe == "refrProbe");
      CHECK(copy->m_d.m_refractionThickness == 15.f);
      CHECK(copy->m_d.m_toy == true);
      CHECK(copy->m_d.m_backfacesEnabled == true);
      CHECK(copy->m_d.m_displayTexture == true);
      CHECK(copy->m_d.m_objectSpaceNormalMap == true);
      CHECK(copy->m_d.m_szImage == "prim.png");
      CHECK(copy->m_d.m_szMaterial == "primMat");
      CHECK(copy->m_d.m_szPhysicsMaterial == "physMat");
      CHECK(copy->m_d.m_visible == false);
      CHECK(copy->m_d.m_collidable == false);
      CHECK(copy->m_d.m_hitEvent == true);
      CHECK(copy->m_d.m_reflectionEnabled == false);

      CHECK(StreamsEqual(saved, SavePartToStream(copy)));

      copy->Release();
   }

   SUBCASE("CopyForPlay preserves the serialized state")
   {
      Primitive* const prim = Primitive::COMCreate();
      prim->Init(50.f, 60.f, false);
      prim->SetName(L"Prim2");
      prim->put_Sides(8);
      table->AddPart(prim);
      prim->Release();

      Primitive* const liveCopy = prim->CopyForPlay();
      CHECK(StreamsEqual(SavePartToStream(prim), SavePartToStream(liveCopy)));
      liveCopy->Release();
   }

   SUBCASE("builtin mesh generation")
   {
      Primitive* const prim = Primitive::COMCreate();
      prim->Init(0.f, 0.f, false);
      prim->SetName(L"Prim3");
      table->AddPart(prim);
      prim->Release();

      // Init builds the built-in mesh: an N-sided cylinder made of 4*N+2 vertices
      // (2 face centers + N top rim + N bottom rim + 2N side ring)
      const int sides = prim->m_d.m_Sides;
      REQUIRE(prim->m_mesh.m_vertices.size() == static_cast<size_t>(4 * sides + 2));
      // First vertex is the top face center
      CHECK(prim->m_mesh.m_vertices[0].x == 0.f);
      CHECK(prim->m_mesh.m_vertices[0].y == 0.f);
      CHECK(prim->m_mesh.m_vertices[0].z == 0.5f);
      CHECK(prim->m_mesh.m_vertices[0].nz == 1.f);
      // Second center vertex is the bottom face center
      CHECK(prim->m_mesh.m_vertices[sides + 1].z == -0.5f);
      CHECK(prim->m_mesh.m_vertices[sides + 1].nz == -1.f);
      // All top rim vertices sit at z=+0.5
      for (int i = 1; i <= sides; i++)
         CHECK(prim->m_mesh.m_vertices[i].z == 0.5f);

      // Changing the side count regenerates the built-in mesh through the COM property
      int newSides = 0;
      CHECK(prim->put_Sides(sides == 4 ? 8 : 4) == S_OK);
      CHECK(prim->get_Sides(&newSides) == S_OK);
      CHECK(newSides == (sides == 4 ? 8 : 4));
   }

   SUBCASE("editor transforms")
   {
      Primitive* const prim = Primitive::COMCreate();
      prim->Init(10.f, 20.f, false);
      prim->SetName(L"Prim4");
      table->AddPart(prim);
      prim->Release();

      CHECK(prim->GetCenter().x == 10.f);
      CHECK(prim->GetCenter().y == 20.f);
      prim->Translate(Vertex2D(2.f, -4.f));
      CHECK(prim->GetCenter().x == 12.f);
      CHECK(prim->GetCenter().y == 16.f);
   }

   table->Release();
}
