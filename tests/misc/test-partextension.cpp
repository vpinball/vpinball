// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/flasher.h"
#include "parts/PartGroup.h"
#include "parts/pintable.h"
#include "utils/BiffReader.h"
#include "utils/BiffWriter.h"
#include "utils/JSONSerializer.h"

#include <nlohmann/json.hpp>

#include "pole/pole.h"

#include "doctest.h"

#include <filesystem>
#include <fstream>
#include <memory>

namespace
{
// Live extension data that counts how often it is asked for its persisted form
class TestExtData final : public IPartExtensionData
{
public:
   string m_payload;
   mutable int m_serializeCalls = 0;
   string Serialize() const override
   {
      ++m_serializeCalls;
      return m_payload;
   }
};

Flasher *AddFlasher(PinTable *table, const string &name)
{
   Flasher *const flasher = Flasher::COMCreate();
   flasher->Init(100.f, 100.f, false);
   flasher->SetName(name);
   table->AddPart(flasher);
   flasher->Release();
   return flasher;
}

vector<uint8_t> ReadStorageStream(const std::filesystem::path &file, const string &name)
{
   POLE::Storage storage(file.string().c_str());
   REQUIRE(storage.open());
   POLE::Stream stream(&storage, name);
   REQUIRE(!stream.fail());
   vector<uint8_t> data(static_cast<size_t>(stream.size()));
   if (!data.empty())
      CHECK(stream.read(data.data(), static_cast<uint64_t>(data.size())) == data.size());
   storage.close();
   return data;
}
}

TEST_CASE("Part extension store")
{
   PinTable *const table = CreateTestTable();
   Flasher *const flasher = AddFlasher(table, "ExtFlasher1");

   SUBCASE("get, create, clear and has")
   {
      CHECK(flasher->GetExtension("refimg") == nullptr);
      CHECK_FALSE(flasher->HasExtension("refimg"));

      PartExtension &entry = flasher->GetOrCreateExtension("refimg");
      CHECK(flasher->HasExtension("refimg"));
      CHECK(&flasher->GetOrCreateExtension("refimg") == &entry); // Get-or-create returns the same entry
      CHECK(entry.Serialized().empty()); // An empty payload is legal: the key's presence is the marker
      entry.SetRaw("{\"v\":1}");
      CHECK(entry.Serialized() == "{\"v\":1}"s);

      // Declaration order is stable and entry references survive appends
      PartExtension &other = flasher->GetOrCreateExtension("plastic");
      other.SetRaw("{}");
      REQUIRE(flasher->m_extensions.size() == 2);
      CHECK(&flasher->m_extensions[0] == &entry);
      CHECK(flasher->m_extensions[0].key == "refimg");
      CHECK(flasher->m_extensions[1].key == "plastic");

      flasher->ClearExtension("refimg");
      CHECK_FALSE(flasher->HasExtension("refimg"));
      CHECK(flasher->HasExtension("plastic"));
      REQUIRE(flasher->m_extensions.size() == 1);
      flasher->ClearExtension("no such key");
      CHECK(flasher->m_extensions.size() == 1);
      flasher->ClearExtension("plastic");
      CHECK(flasher->m_extensions.empty());
   }

   SUBCASE("live data serializes on demand and is never shared with copies")
   {
      PartExtension &entry = flasher->GetOrCreateExtension("plastic");
      entry.SetRaw("{\"v\":0}");
      const auto data = std::make_shared<TestExtData>();
      data->m_payload = "{\"v\":1,\"bevel_mm\":1.5}";
      entry.SetData(data);
      CHECK(entry.Data() == data.get());
      CHECK(entry.Serialized() == "{\"v\":1,\"bevel_mm\":1.5}"s); // The live data wins over the stored raw
      CHECK(data->m_serializeCalls >= 1);
      data->m_payload = "{\"v\":1,\"bevel_mm\":2}";
      CHECK(entry.Serialized() == "{\"v\":1,\"bevel_mm\":2}"s); // Re-asked on every serialization

      // An entry copy carries the synced string, not the live data
      const PartExtension copied = entry;
      CHECK(copied.key == "plastic");
      CHECK(copied.Serialized() == "{\"v\":1,\"bevel_mm\":2}"s);
      CHECK(copied.Data() == nullptr);

      // A raw write detaches the live data
      entry.SetRaw("{\"v\":2}");
      CHECK(entry.Data() == nullptr);
      CHECK(entry.Serialized() == "{\"v\":2}"s);
      CHECK(data.use_count() == 1);

      // CopyForPlay carries the extension, not the live data (no aliasing with the startup part)
      entry.SetData(data);
      Flasher *const live = flasher->CopyForPlay();
      const PartExtension *const liveEntry = live->GetExtension("plastic");
      REQUIRE(liveEntry != nullptr);
      CHECK(liveEntry->Serialized() == "{\"v\":1,\"bevel_mm\":2}"s);
      CHECK(liveEntry->Data() == nullptr);
      live->Release();
   }

   SUBCASE("ClearForOverwrite clears the store")
   {
      flasher->GetOrCreateExtension("refimg").SetRaw("x");
      flasher->ClearForOverwrite(); // Flasher overrides ClearForOverwrite (clears its curve too)
      CHECK(flasher->m_extensions.empty());

      PartGroup *const group = PartGroup::COMCreate();
      group->Init(0.f, 0.f, false);
      group->SetName("ExtGroup");
      table->AddPart(group);
      group->Release();
      group->GetOrCreateExtension("refimg").SetRaw("x");
      group->ClearForOverwrite(); // PartGroup relies on the base implementation
      CHECK(group->m_extensions.empty());
   }

   SUBCASE("BIFF save/load round-trip")
   {
      const auto data = std::make_shared<TestExtData>();
      data->m_payload = "{\"v\":1,\"src\":\"PF.png\"}";
      flasher->GetOrCreateExtension("refimg").SetRaw("{\"v\":1}");
      flasher->GetOrCreateExtension("plastic").SetData(data);
      flasher->GetOrCreateExtension("empty").SetRaw(""); // Marker with an empty payload

      const InMemStream saved = SavePartToStream(flasher);
      CHECK(data->m_serializeCalls >= 1); // The live data was asked for its persisted form once

      Flasher *const copy = Flasher::COMCreate();
      LoadPartFromStream(copy, saved);
      REQUIRE(copy->m_extensions.size() == 3);
      CHECK(copy->m_extensions[0].key == "refimg");
      CHECK(copy->m_extensions[0].Serialized() == "{\"v\":1}"s);
      CHECK(copy->m_extensions[1].key == "plastic");
      CHECK(copy->m_extensions[1].Serialized() == "{\"v\":1,\"src\":\"PF.png\"}"s);
      CHECK(copy->m_extensions[2].key == "empty");
      CHECK(copy->m_extensions[2].Serialized().empty());
      CHECK(copy->GetExtension("refimg")->Data() == nullptr); // Nothing is deserialized on load
      CHECK(copy->GetExtension("plastic")->Data() == nullptr);

      CHECK(StreamsEqual(saved, SavePartToStream(copy))); // Untouched entries re-save verbatim
      copy->Release();
   }

   SUBCASE("BIFF load tolerates unknown fields, duplicates and empty keys")
   {
      // Hand crafted part stream: an extension with an unknown extra field, a duplicated key
      // (last wins) and an entry without a key (dropped)
      InMemStream stream;
      const ItemTypeEnum type = eItemFlasher;
      stream.Write(&type, sizeof(int));
      BiffWriter writer(&stream, nullptr);
      writer.BeginObject(FID(EXTD), true, true);
      writer.WriteString(FID(XKEY), "dup");
      writer.WriteString(FID(XVAL), "first");
      writer.WriteInt(FID(ZZZZ), 42); // Unknown field inside the extension record
      writer.EndObject();
      writer.BeginObject(FID(EXTD), true, true);
      writer.WriteString(FID(XKEY), "dup");
      writer.WriteString(FID(XVAL), "last");
      writer.EndObject();
      writer.BeginObject(FID(EXTD), true, true);
      writer.WriteString(FID(XVAL), "no key");
      writer.EndObject();
      writer.EndObject(); // The part's own closing ENDB
      CHECK_FALSE(writer.HasError());

      Flasher *const copy = Flasher::COMCreate();
      LoadPartFromStream(copy, stream);
      REQUIRE(copy->m_extensions.size() == 1);
      CHECK(copy->m_extensions[0].key == "dup");
      CHECK(copy->m_extensions[0].Serialized() == "last"s);
      copy->Release();
   }

   table->Release();
}


TEST_CASE("Part extension table round-trips")
{
   std::error_code ec;

   CComObject<PinTable> *table;
   CComObject<PinTable>::CreateInstance(&table);
   table->AddRef();
   TestFileFeedback feedback;
   REQUIRE(SUCCEEDED(table->LoadGameFromFilename(GetAssetPath() / "test000-default-table.vpx", feedback)));

   Flasher *const flasher = AddFlasher(table, "ExtFlasher");
   flasher->GetOrCreateExtension("refimg").SetRaw("{\"v\":1}");
   flasher->GetOrCreateExtension("empty").SetRaw("");
   PartGroup *const group = PartGroup::COMCreate();
   group->Init(0.f, 0.f, false);
   group->SetName("ExtGroup");
   table->AddPart(group);
   group->Release();
   group->GetOrCreateExtension("refimg").SetRaw("{\"v\":1,\"role\":\"group\"}");

   const auto checkExtensions = [](const PinTable *reloaded)
   {
      const vector<IEditable *> marked = reloaded->GetPartsWithExtension("refimg");
      CHECK(marked.size() == 2);
      for (IEditable *const part : reloaded->GetParts())
      {
         if (part->GetName() == "ExtFlasher")
         {
            REQUIRE(part->m_extensions.size() == 2);
            CHECK(part->m_extensions[0].key == "refimg");
            CHECK(part->m_extensions[0].Serialized() == "{\"v\":1}"s);
            CHECK(part->m_extensions[1].key == "empty");
            CHECK(part->m_extensions[1].Serialized().empty());
            CHECK(part->m_extensions[0].Data() == nullptr); // Load stores strings, never parses
         }
         else if (part->GetName() == "ExtGroup")
         {
            REQUIRE(part->m_extensions.size() == 1);
            CHECK(part->m_extensions[0].key == "refimg");
            CHECK(part->m_extensions[0].Serialized() == "{\"v\":1,\"role\":\"group\"}"s);
         }
      }
   };

   SUBCASE("VPX file")
   {
      const std::filesystem::path file = GetTestTmpDir() / "partext.vpx";
      std::filesystem::remove(file, ec);
      table->m_filename = file;
      TestFileFeedback saveFeedback;
      REQUIRE(SUCCEEDED(table->Save(saveFeedback)));

      CComObject<PinTable> *reloaded;
      CComObject<PinTable>::CreateInstance(&reloaded);
      reloaded->AddRef();
      TestFileFeedback loadFeedback;
      REQUIRE(SUCCEEDED(reloaded->LoadGameFromFilename(file, loadFeedback)));
      checkExtensions(reloaded);
      reloaded->Release();
      std::filesystem::remove(file, ec);
   }

   SUBCASE("VPZ folder pack")
   {
      const std::filesystem::path packPath = GetTestTmpDir() / "partext.vpz";
      std::filesystem::remove_all(packPath, ec);
      REQUIRE(std::filesystem::create_directories(packPath, ec));
      TestFileFeedback saveFeedback;
      REQUIRE(SUCCEEDED(table->SaveToJSON(packPath, saveFeedback)));

      // The json shape: one "extensions" array of key/value objects per part document
      const nlohmann::ordered_json partDoc = nlohmann::ordered_json::parse(std::ifstream(packPath / "parts" / (JSONSerializer::SanitizeFileName("ExtFlasher"s) + ".json")));
      REQUIRE(partDoc["extensions"].is_array());
      REQUIRE(partDoc["extensions"].size() == 2);
      CHECK(partDoc["extensions"][0]["key"].get<string>() == "refimg");
      CHECK(partDoc["extensions"][0]["value"].get<string>() == "{\"v\":1}"s);
      CHECK(partDoc["extensions"][1]["key"].get<string>() == "empty");
      CHECK(partDoc["extensions"][1]["value"].get<string>().empty());

      CComObject<PinTable> *reloaded;
      CComObject<PinTable>::CreateInstance(&reloaded);
      reloaded->AddRef();
      TestFileFeedback loadFeedback;
      REQUIRE(SUCCEEDED(reloaded->LoadGameFromFilename(packPath, loadFeedback)));
      checkExtensions(reloaded);
      reloaded->Release();
      std::filesystem::remove_all(packPath, ec);
   }

   SUBCASE("extension data does not perturb the file hash")
   {
      // GameItem streams are not covered by the MAC: two saves that only differ in extension data must produce the same MAC
      const std::filesystem::path file1 = GetTestTmpDir() / "partext-mac-1.vpx";
      const std::filesystem::path file2 = GetTestTmpDir() / "partext-mac-2.vpx";
      std::filesystem::remove(file1, ec);
      std::filesystem::remove(file2, ec);
      table->m_filename = file1;
      TestFileFeedback saveFeedback;
      REQUIRE(SUCCEEDED(table->Save(saveFeedback)));
      flasher->GetOrCreateExtension("refimg").SetRaw("{\"v\":1,\"changed\":true}");
      flasher->GetOrCreateExtension("added").SetRaw("new key");
      table->m_filename = file2;
      REQUIRE(SUCCEEDED(table->Save(saveFeedback)));

      CHECK(ReadStorageStream(file1, "GameStg/MAC"s) == ReadStorageStream(file2, "GameStg/MAC"s));
      std::filesystem::remove(file1, ec);
      std::filesystem::remove(file2, ec);
   }

   table->Release();
}
