// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/Material.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"
#include "core/vpversion.h"

#include "doctest.h"

TEST_CASE("Material")
{
   SUBCASE("BIFF save/load round-trip")
   {
      Material mat;
      mat.m_name = "TestMaterial";
      mat.m_type = Material::METAL;
      mat.m_fWrapLighting = 0.25f;
      mat.m_fRoughness = 0.75f;
      mat.m_fGlossyImageLerp = 0.5f;
      mat.m_fThickness = 0.8f;
      mat.m_fEdge = 0.9f;
      mat.m_fEdgeAlpha = 0.6f;
      mat.m_fOpacity = 0.4f;
      mat.m_cBase = RGB(10, 20, 30);
      mat.m_cGlossy = RGB(40, 50, 60);
      mat.m_cClearcoat = RGB(70, 80, 90);
      mat.m_bOpacityActive = true;
      mat.m_fElasticity = 0.3f;
      mat.m_fElasticityFalloff = 0.1f;
      mat.m_fFriction = 0.2f;
      mat.m_fScatterAngle = 5.f;
      mat.m_cRefractionTint = RGB(100, 110, 120);

      // Materials are saved as a MATR sub-object inside the table's GameData stream
      InMemStream stream;
      {
         BiffWriter writer(&stream, nullptr);
         mat.Save(writer, false);
         writer.EndObject();
         CHECK_FALSE(writer.HasError());
      }

      Material loaded;
      BiffReader reader(stream.Data(), static_cast<uint32_t>(stream.Size()), CURRENT_FILE_FORMAT_VERSION, nullptr, 0);
      reader.AsObject(
         [&loaded](int tag, IObjectReader& fieldReader)
         {
            if (tag == FID(MATR))
               loaded.Load(fieldReader);
            return true;
         });
      CHECK_FALSE(reader.HasError());

      CHECK(loaded.m_name == "TestMaterial");
      CHECK(loaded.m_type == Material::METAL);
      CHECK(loaded.m_fWrapLighting == 0.25f);
      CHECK(loaded.m_fRoughness == 0.75f);
      CHECK(loaded.m_fGlossyImageLerp == 0.5f);
      CHECK(loaded.m_fThickness == 0.8f);
      CHECK(loaded.m_fEdge == 0.9f);
      CHECK(loaded.m_fEdgeAlpha == 0.6f);
      CHECK(loaded.m_fOpacity == 0.4f);
      CHECK(loaded.m_cBase == RGB(10, 20, 30));
      CHECK(loaded.m_cGlossy == RGB(40, 50, 60));
      CHECK(loaded.m_cClearcoat == RGB(70, 80, 90));
      CHECK(loaded.m_bOpacityActive == true);
      CHECK(loaded.m_fElasticity == 0.3f);
      CHECK(loaded.m_fElasticityFalloff == 0.1f);
      CHECK(loaded.m_fFriction == 0.2f);
      CHECK(loaded.m_fScatterAngle == 5.f);
      CHECK(loaded.m_cRefractionTint == RGB(100, 110, 120));
   }

   SUBCASE("copy constructor duplicates all fields")
   {
      Material mat;
      mat.m_name = "Source";
      mat.m_type = Material::METAL;
      mat.m_fRoughness = 0.42f;
      mat.m_cBase = RGB(1, 2, 3);
      mat.m_bOpacityActive = true;
      mat.m_fOpacity = 0.7f;
      mat.m_fElasticity = 0.15f;
      mat.m_fScatterAngle = 2.5f;
      mat.m_cRefractionTint = RGB(9, 8, 7);

      const Material copy(&mat);
      CHECK(copy.m_name == mat.m_name);
      CHECK(copy.m_type == mat.m_type);
      CHECK(copy.m_fRoughness == mat.m_fRoughness);
      CHECK(copy.m_cBase == mat.m_cBase);
      CHECK(copy.m_bOpacityActive == mat.m_bOpacityActive);
      CHECK(copy.m_fOpacity == mat.m_fOpacity);
      CHECK(copy.m_fElasticity == mat.m_fElasticity);
      CHECK(copy.m_fScatterAngle == mat.m_fScatterAngle);
      CHECK(copy.m_cRefractionTint == mat.m_cRefractionTint);
   }

   SUBCASE("hash is stable and field dependent")
   {
      Material mat;
      mat.m_fRoughness = 0.5f;
      const uint64_t h1 = mat.hash();
      CHECK(mat.hash() == h1);
      mat.m_fRoughness = 0.9f;
      CHECK(mat.hash() != h1);
   }

   SUBCASE("legacy save structs are zero initialized")
   {
      // SaveMaterial/SavePhysicsMaterial are the pre-10.8 on-disk formats: their
      // constructors must leave no uninitialized data that could leak into files
      const SaveMaterial mat;
      CHECK(mat.szName[0] == '\0');
      CHECK(mat.cBase == 0);
      CHECK(mat.fRoughness == 0.f);
      CHECK(mat.bIsMetal == false);

      const SavePhysicsMaterial pmat;
      CHECK(pmat.szName[0] == '\0');
      CHECK(pmat.fElasticity == 0.f);
      CHECK(pmat.fElasticityFallOff == 0.f);
      CHECK(pmat.fFriction == 0.f);
      CHECK(pmat.fScatterAngle == 0.f);
   }
}
