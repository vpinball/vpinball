// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/pintable.h"
#include "parts/timer.h"
#include "parts/bumper.h"
#include "parts/flasher.h"
#include "parts/Collection.h"
#include "parts/Material.h"
#include "parts/surface.h"

#include "doctest.h"

TEST_CASE("PinTable part")
{
   SUBCASE("part add/remove lifecycle and name lookup")
   {
      PinTable* const table = CreateTestTable();
      CHECK(table->GetParts().empty());

      Timer* const timer = Timer::COMCreate();
      timer->Init(10.f, 20.f, false);
      timer->SetName("Part1");

      // AddPart takes its own reference on the part and registers the script name
      table->AddPart(timer);
      timer->Release();
      CHECK(table->HasPart(timer));
      CHECK(table->GetParts().size() == 1);
      CHECK(table->GetElementByName("Part1") == timer);
      CHECK(timer->GetPTable() == table);

      // Removing the part releases the table's reference and clears the name
      table->RemovePart(timer);
      CHECK_FALSE(table->HasPart(timer));
      CHECK(table->GetParts().empty());
      CHECK(table->GetElementByName("Part1") == nullptr);
      // 'timer' was deleted by RemovePart: do not touch it anymore

      table->Release();
   }

   SUBCASE("parts keep their creation order in the table list")
   {
      PinTable* const table = CreateTestTable();

      Timer* const t1 = Timer::COMCreate();
      t1->Init(0.f, 0.f, false);
      t1->SetName("A");
      table->AddPart(t1);
      t1->Release();

      Bumper* const b1 = Bumper::COMCreate();
      b1->Init(0.f, 0.f, false);
      b1->SetName("B");
      table->AddPart(b1);
      b1->Release();

      REQUIRE(table->GetParts().size() == 2);
      CHECK(table->GetParts()[0] == t1);
      CHECK(table->GetParts()[1] == b1);
      CHECK(table->GetParts()[0]->GetItemType() == eItemTimer);
      CHECK(table->GetParts()[1]->GetItemType() == eItemBumper);

      table->Release();
   }

   SUBCASE("part names are UTF-8, length limited and unique")
   {
      PinTable* const table = CreateTestTable();

      Timer* const t1 = Timer::COMCreate();
      t1->Init(0.f, 0.f, false);
      t1->SetName("Caf\xC3\xA9Timer"s);
      table->AddPart(t1);
      t1->Release();
      CHECK(t1->GetName() == "Caf\xC3\xA9Timer");
      CHECK(table->GetElementByName("Caf\xC3\xA9Timer") == t1);
      CHECK_FALSE(table->IsNameUnique("CAF\xC3\xA9TIMER"s)); // Case insensitive (ASCII only)

      // Changing only the letter case keeps the name (no unique suffix), and keeps it registered
      t1->SetName("CAF\xC3\xA9TIMER"s);
      CHECK(t1->GetName() == "CAF\xC3\xA9TIMER");
      CHECK(table->GetElementByName("CAF\xC3\xA9TIMER") == t1);
      CHECK_FALSE(table->IsNameUnique("caf\xC3\xA9timer"s));
      t1->SetName("Caf\xC3\xA9Timer"s);

      // Limited to MAXNAMEBUFFER - 1 UTF-16 units, cut on a character boundary
      Timer* const t2 = Timer::COMCreate();
      t2->Init(0.f, 0.f, false);
      t2->SetName(string(MAXNAMEBUFFER - 2, 'x') + "\xC3\xA9\xC3\xA9");
      CHECK(t2->GetName() == string(MAXNAMEBUFFER - 2, 'x') + "\xC3\xA9");
      t2->Release();

      // The 3 digit suffix of unique names stays within the limit
      CHECK(table->GetUniqueName(string(40, 'y')) == string(MAXNAMEBUFFER - 4, 'y') + "001");
      CHECK(table->GetUniqueName("Caf\xC3\xA9Timer"s) == "Caf\xC3\xA9Timer001");

      table->Release();
   }

   SUBCASE("name lookups ignore case, like VBScript")
   {
      PinTable* const table = CreateTestTable();

      Surface* const wall = Surface::COMCreate();
      wall->Init(0.f, 0.f, false);
      wall->SetName("Wall1"s);
      wall->m_d.m_heighttop = 42.f;
      table->AddPart(wall);
      wall->Release();

      CHECK(table->GetElementByName("wall1") == wall);
      CHECK(table->GetElementByName("WALL1") == wall);
      CHECK(table->GetElementByName("Wall2") == nullptr);
      CHECK(table->GetSurfaceHeight("wALL1"s, 0.f, 0.f) == 42.f);
      CHECK(table->GetSurfaceHeight("Wall2"s, 0.f, 0.f) == 0.f);

      table->Release();
   }

   SUBCASE("material add, lookup and name uniquification")
   {
      PinTable* const table = CreateTestTable();
      CHECK(table->GetMaterialList().empty());

      Material* const wood = new Material();
      wood->m_name = "Wood";
      wood->m_fRoughness = 0.9f;
      table->AddMaterial(wood);

      CHECK(table->GetMaterial("Wood") == wood);
      // Lookup is case insensitive
      CHECK(table->GetMaterial("wood") == wood);
      // Missing names resolve to the dummy material
      CHECK(table->IsDummyMaterial(table->GetMaterial("DoesNotExist")));
      CHECK(table->IsDummyMaterial(table->GetMaterial("")));

      // Adding a material with a colliding name renames the new one
      Material* const dupe = new Material();
      dupe->m_name = "Wood";
      table->AddMaterial(dupe);
      CHECK(dupe->m_name == "Wood1");
      CHECK(table->GetMaterial("Wood1") == dupe);
      CHECK(table->GetMaterialList().size() == 2);

      // RemoveMaterial takes ownership back and deletes the material
      table->RemoveMaterial(dupe);
      CHECK(table->GetMaterialList().size() == 1);
      CHECK(table->IsDummyMaterial(table->GetMaterial("Wood1")));

      table->Release();
   }

   SUBCASE("ON/OFF toggle option round-trips in its declared domain")
   {
      // Stars registers Ambience/Background Music as Option(name, 1, 2, 1, 2, 0,
      // Array("ON","OFF")) and branches on the declared value (1 = ON, 2 = OFF), so the
      // option must round-trip in that 1..2 domain, not the stored bool {0,1} domain.
      PinTable* const table = CreateTestTable();

      // Array("ON","OFF") as a VT_ARRAY|VT_VARIANT of BSTRs.
      SAFEARRAY* psa = SafeArrayCreateVector(VT_VARIANT, 0, 2);
      REQUIRE(psa != nullptr);
      {
         LONG idx = 0;
         CComVariant on(L"ON");
         SafeArrayPutElement(psa, &idx, &on);
         idx = 1;
         CComVariant off(L"OFF");
         SafeArrayPutElement(psa, &idx, &off);
      }
      VARIANT values;
      VariantInit(&values);
      V_VT(&values) = VT_ARRAY | VT_VARIANT;
      V_ARRAY(&values) = psa;

      BSTR name = SysAllocString(L"Ambience");

      // Unset: the script receives the declared default (2 = OFF).
      float v = -1.f;
      REQUIRE(table->get_Option(name, 1.f, 2.f, 1.f, 2.f, 0, values, &v) == S_OK);
      CHECK(v == 2.f);

      // ON round-trips through the ini: set ON, save, reload (as on relaunch), re-seed the
      // live value as RegisterOption does, read back. Proves the key persists and decodes
      // to the declared ON value.
      const auto id = table->GetOptions().front().id;
      const std::filesystem::path iniPath = GetTestTmpDir() / "pintable-toggle-roundtrip.ini";
      std::filesystem::remove(iniPath);

      REQUIRE(table->put_Option(name, 1.f, 2.f, 1.f, 2.f, 0, values, 1.f) == S_OK);
      table->GetSettings().SetIniPath(iniPath);
      table->GetSettings().Save();
      REQUIRE(FileExists(iniPath)); // the override was written, not stripped
      table->GetSettings().Load(false); // relaunch: reload from the ini we just wrote
      table->SetOptionLiveValue(id, table->GetSettings().GetBool(id) ? 1.f : 0.f);
      v = -1.f;
      REQUIRE(table->get_Option(name, 1.f, 2.f, 1.f, 2.f, 0, values, &v) == S_OK);
      CHECK(v == 1.f); // ON
      // The stored logical value drives the UI checkbox (checked = value != 0), so ON must
      // store true for the checkbox and the script value to agree.
      CHECK(table->GetSettings().GetBool(id));

      // OFF round-trips the same way.
      REQUIRE(table->put_Option(name, 1.f, 2.f, 1.f, 2.f, 0, values, 2.f) == S_OK);
      table->GetSettings().Save();
      table->GetSettings().Load(false);
      table->SetOptionLiveValue(id, table->GetSettings().GetBool(id) ? 1.f : 0.f);
      v = -1.f;
      REQUIRE(table->get_Option(name, 1.f, 2.f, 1.f, 2.f, 0, values, &v) == S_OK);
      CHECK(v == 2.f); // OFF
      CHECK_FALSE(table->GetSettings().GetBool(id)); // checkbox unchecked, agrees with OFF

      std::filesystem::remove(iniPath);
      SysFreeString(name);
      VariantClear(&values);
      table->Release();
   }

   SUBCASE("collections are owned by the table")
   {
      PinTable* const table = CreateTestTable();
      CHECK(table->GetCollections().empty());

      CComObject<Collection>* col;
      CComObject<Collection>::CreateInstance(&col);
      col->AddRef();
      col->m_name = "Col1";
      table->AddCollection(col);
      col->Release(); // the table now owns the only reference

      REQUIRE(table->GetCollections().size() == 1);
      CHECK(table->GetCollections()[0]->m_name == "Col1");

      table->Release();
   }

   SUBCASE("audit reports deprecated flasher VideoCap API use")
   {
      PinTable* const table = CreateTestTable();

      Flasher* const flasher = Flasher::COMCreate();
      flasher->Init(100.f, 200.f, false);
      flasher->SetName("Flasher1");
      table->AddPart(flasher);
      flasher->Release();

      table->m_script_text = "Sub Table1_Init\nFlasher1.VideoCapWidth = 320\nFlasher1.VideoCapUpdate \"cap\"\nEnd Sub\n";
      const string audit = table->AuditTable(false);
      CHECK(audit.find("deprecated") != string::npos);
      CHECK(audit.find("Flasher1") != string::npos);

      // Indirect use (e.g. through a With block or an expression) still reports a generic error
      table->m_script_text = "Sub Table1_Init\nVideoCapUpdate \"cap\"\nEnd Sub\n";
      CHECK(table->AuditTable(false).find("VideoCap") != string::npos);

      table->m_script_text = "Sub Table1_Init\nEnd Sub\n";
      CHECK(table->AuditTable(false).find("VideoCap") == string::npos);

      table->Release();
   }
}
