// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "core/ScriptInterpreter.h"
#include "parts/pintable.h"
#include "core/pinundo.h"
#include "core/TableAudit.h"
#include "audio/AudioPlayer.h"
#include "parts/light.h"
#include "parts/timer.h"
#include "parts/bumper.h"
#include "parts/flasher.h"
#include "parts/Collection.h"
#include "parts/Material.h"
#include "parts/Sound.h"
#include "parts/surface.h"
#include "renderer/Texture.h"

#include <fstream>

#ifdef __STANDALONE__
#define _WINDOWS_ // FreeImage then reuses the libwinevbs windows.h types (as in TableAudit.cpp)
#endif
#include "FreeImage.h"

#include "doctest.h"

namespace
{

// Builds a stereo 16 bit PCM WAV file holding durationS seconds of a sine wave
vector<uint8_t> MakeStereoSineWav(float durationS, int sampleRate = 44100)
{
   const auto append32 = [](vector<uint8_t>& out, uint32_t v)
   {
      out.push_back(static_cast<uint8_t>(v & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
   };
   const auto append16 = [](vector<uint8_t>& out, uint16_t v)
   {
      out.push_back(static_cast<uint8_t>(v & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
   };
   vector<uint8_t> samples;
   const int nFrames = static_cast<int>(sampleRate * durationS);
   samples.reserve(nFrames * 4);
   for (int i = 0; i < nFrames; ++i)
   {
      const int16_t sample = static_cast<int16_t>(8000.0 * sin(2.0 * M_PI * 440.0 * i / sampleRate));
      append16(samples, static_cast<uint16_t>(sample));
      append16(samples, static_cast<uint16_t>(sample));
   }
   vector<uint8_t> wav;
   wav.insert(wav.end(), { 'R', 'I', 'F', 'F' });
   append32(wav, static_cast<uint32_t>(36 + samples.size()));
   wav.insert(wav.end(), { 'W', 'A', 'V', 'E', 'f', 'm', 't', ' ' });
   append32(wav, 16); // fmt chunk size
   append16(wav, 1); // PCM
   append16(wav, 2); // Stereo
   append32(wav, static_cast<uint32_t>(sampleRate));
   append32(wav, static_cast<uint32_t>(sampleRate * 2 * 2));
   append16(wav, 4); // Block align
   append16(wav, 16); // Bits per sample
   wav.insert(wav.end(), { 'd', 'a', 't', 'a' });
   append32(wav, static_cast<uint32_t>(samples.size()));
   wav.insert(wav.end(), samples.begin(), samples.end());
   return wav;
}

}

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

   SUBCASE("degree unit option displays unscaled with a degree sign suffix")
   {
      PinTable* const table = CreateTestTable();
      VARIANT values;
      VariantInit(&values);

      // Float option (e.g. a playfield slope): value is shown and stored unscaled, with the degree sign suffix
      BSTR name = SysAllocString(L"Slope");
      float v = -1.f;
      REQUIRE(table->get_Option(name, 0.f, 10.f, 0.1f, 6.f, 2, values, &v) == S_OK);
      CHECK(v == 6.f);
      REQUIRE(table->GetOptions().size() == 1);
      CHECK(table->GetOptions().back().displayScale == 1.f);
      CHECK(table->GetOptions().back().format == "%4.1f\xC2\xB0");
      SysFreeString(name);

      // Int option likewise
      name = SysAllocString(L"Rotation");
      REQUIRE(table->get_Option(name, 0.f, 360.f, 1.f, 90.f, 2, values, &v) == S_OK);
      CHECK(v == 90.f);
      REQUIRE(table->GetOptions().size() == 2);
      CHECK(table->GetOptions().back().displayScale == 1.f);
      CHECK(table->GetOptions().back().format == "%d\xC2\xB0");
      SysFreeString(name);

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

   SUBCASE("audit items carry subjects and undoable fixes")
   {
      PinTable* const table = CreateTestTable();
      PinUndo undo(table);

      // Fixable error: negative light intensity clamps back to 0
      Light* const light = Light::COMCreate();
      light->Init(0.f, 0.f, false);
      light->SetName("Light1");
      light->m_d.m_intensity = -5.f;
      table->AddPart(light);
      light->Release();

      // Fixable warning: glass bottom above glass top
      table->m_glassBottomHeight = 300.f;
      table->m_glassTopHeight = 200.f;

      const TableAudit audit = TableAudit::Run(*table);
      const auto findItem = [&audit](const string& needle)
      {
         const auto it = std::ranges::find_if(audit.items, [&needle](const TableAuditItem& item) { return item.summary.find(needle) != string::npos; });
         return it != audit.items.end() ? &*it : nullptr;
      };

      const TableAuditItem* const lightItem = findItem("negative intensity");
      REQUIRE(lightItem != nullptr);
      CHECK(lightItem->severity == TableAuditItem::Severity::Error);
      CHECK(lightItem->part == light);
      CHECK(lightItem->details.length() >= lightItem->summary.length());
      REQUIRE(lightItem->fix != nullptr);

      const TableAuditItem* const glassItem = findItem("Glass height");
      REQUIRE(glassItem != nullptr);
      CHECK(glassItem->severity == TableAuditItem::Severity::Warning);
      CHECK(glassItem->part == static_cast<IEditable*>(table));
      REQUIRE(glassItem->fix != nullptr);

      undo.BeginUndo();
      lightItem->fix(*table, undo);
      undo.EndUndo();
      CHECK(light->m_d.m_intensity == doctest::Approx(0.f));

      undo.BeginUndo();
      glassItem->fix(*table, undo);
      undo.EndUndo();
      CHECK(table->m_glassBottomHeight == doctest::Approx(table->m_glassTopHeight));

      undo.Undo();
      CHECK(table->m_glassBottomHeight == doctest::Approx(300.f));
      undo.Undo();
      CHECK(light->m_d.m_intensity == doctest::Approx(-5.f));

      table->Release();
   }

   SUBCASE("audit creates a missing PinMAMETimer timer")
   {
      PinTable* const table = CreateTestTable();
      PinUndo undo(table);

      table->m_script_text = "Sub Table1_Init\nLoadVPM \"x\", \"y\", 0\nvpmInit Me\nEnd Sub\n";
      const TableAudit audit = TableAudit::Run(*table);
      const auto it = std::ranges::find_if(audit.items, [](const TableAuditItem& item) { return item.summary.find("PinMAMETimer") != string::npos; });
      REQUIRE(it != audit.items.end());
      REQUIRE(it->fix != nullptr); // The name is free: the fix is offered

      undo.BeginUndo();
      IEditable* const created = it->fix(*table, undo);
      undo.EndUndo();
      REQUIRE(created != nullptr);
      CHECK(created->GetItemType() == eItemTimer);
      CHECK(table->GetElementByName("PinMAMETimer") == created);

      undo.Undo(); // The creation is undone: the timer is removed from the table
      CHECK(table->GetElementByName("PinMAMETimer") == nullptr);

      table->Release();
   }

   SUBCASE("audit re-encodes a stereo playfield sound to mono Ogg Opus")
   {
      PinTable* const table = CreateTestTable();
      PinUndo undo(table);

      // 22kHz stereo: the Opus fix encodes at 24kHz, the codec rate covering the source bandwidth
      VPX::Sound* const sound = new VPX::Sound("Snd1", "snd1.wav", MakeStereoSineWav(0.1f, 22050));
      sound->SetOutputTarget(VPX::SNDOUT_TABLE);
      table->m_vsound.push_back(sound);
      const size_t wavSize = sound->GetFileSize();

      const TableAudit audit = TableAudit::Run(*table);
      const auto findItem = [&audit](const string& needle)
      {
         const auto it = std::ranges::find_if(audit.items, [&needle](const TableAuditItem& item) { return item.summary.find(needle) != string::npos; });
         return it != audit.items.end() ? &*it : nullptr;
      };
      const TableAuditItem* const monoItem = findItem("multiple channels");
      REQUIRE(monoItem != nullptr);
      CHECK(monoItem->severity == TableAuditItem::Severity::Error);
      CHECK(monoItem->sound == sound);
      REQUIRE(monoItem->fix != nullptr);
      REQUIRE(findItem("Ogg Opus") != nullptr); // The WAV is also reported as not optimally encoded

      undo.BeginUndo();
      monoItem->fix(*table, undo);
      undo.EndUndo();

      CHECK(sound->GetImportPath().extension() == ".opus");
      CHECK(memcmp(sound->GetFileRaw(), "OggS", 4) == 0);
      VPX::AudioPlayer player(""s, ""s, VPX::SNDCFG_SND3D2CH, VPX::SNDCFG_BG2CH, ""s, VPX::SNDCFG_SND3D2CH);
      const VPX::SoundSpec specs = player.GetSoundInformations(sound);
      CHECK(specs.nChannels == 1);
      CHECK(specs.lengthInSeconds == doctest::Approx(0.1).epsilon(0.3));

      undo.Undo(); // The original WAV data and path are restored
      CHECK(sound->GetFileSize() == wavSize);
      CHECK(memcmp(sound->GetFileRaw(), "RIFF", 4) == 0);
      CHECK(sound->GetImportPath() == "snd1.wav");

      table->Release();
   }

   SUBCASE("audit re-encodes a lossy sound to Ogg Opus only well above the Opus bitrate")
   {
      PinTable* const table = CreateTestTable();
      PinUndo undo(table);

      // The default table's 'ballrelease' sound: mono 44.1kHz Ogg Vorbis
      vector<uint8_t> oggData;
      {
         std::ifstream file(GetAssetPath() / "ballrelease.ogg", std::ios::binary);
         REQUIRE(file.is_open());
         oggData = vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
      }
      VPX::Sound* const sound = new VPX::Sound("ballrelease", "ballrelease.ogg", oggData);
      sound->SetOutputTarget(VPX::SNDOUT_TABLE);
      table->m_vsound.push_back(sound);
      const size_t oggSize = sound->GetFileSize();

      // The source bitrate counts the audio pages only (the Vorbis setup header alone is several KB)
      VPX::AudioPlayer player(""s, ""s, VPX::SNDCFG_SND3D2CH, VPX::SNDCFG_BG2CH, ""s, VPX::SNDCFG_SND3D2CH);
      const VPX::SoundSpec sourceSpecs = player.GetSoundInformations(sound);
      const TableAudit::SoundFileInfo source = TableAudit::AnalyzeSoundFile(*sound, sourceSpecs.lengthInSeconds);
      CHECK(source.codec == TableAudit::SoundFileCodec::OggLossy);
      const int sourceBitrate = source.bitrate;
      CHECK(sourceBitrate > 16000);
      CHECK(sourceBitrate < static_cast<int>(static_cast<float>(oggSize) * 8.f / sourceSpecs.lengthInSeconds));

      // About 85 kbit/s, below 1.5x the 96 kbit/s mono Opus bitrate: a re-encode would barely save anything, it is not offered
      REQUIRE(sourceBitrate < 144000);
      const TableAudit audit = TableAudit::Run(*table);
      CHECK(std::ranges::none_of(audit.items, [](const TableAuditItem& item) { return item.summary.find("Ogg Opus") != string::npos; }));

      // The same sound seen as twice that bitrate (half its length): offered, encoded at 96 kbit/s
      VPX::SoundSpec highBitrateSpecs = sourceSpecs;
      highBitrateSpecs.lengthInSeconds *= 0.5f;
      REQUIRE(TableAudit::AnalyzeSoundFile(*sound, highBitrateSpecs.lengthInSeconds).bitrate >= 144000);
      const vector<TableAuditItem> fixes = TableAudit::SoundFixes(*sound, highBitrateSpecs);
      REQUIRE(fixes.size() == 1);
      CHECK(fixes[0].severity == TableAuditItem::Severity::Info);
      CHECK(fixes[0].sound == sound);
      CHECK(fixes[0].details.find("at 96 kbit/s") != string::npos);
      REQUIRE(fixes[0].fix != nullptr);

      undo.BeginUndo();
      fixes[0].fix(*table, undo);
      undo.EndUndo();
      CHECK(fixes[0].fixFailure->empty());

      CHECK(sound->GetImportPath().extension() == ".opus");
      CHECK(memcmp(sound->GetFileRaw(), "OggS", 4) == 0);
      CHECK(sound->GetFileSize() < oggSize);
      const VPX::SoundSpec specs = player.GetSoundInformations(sound);
      CHECK(specs.nChannels == 1);
      CHECK(specs.lengthInSeconds == doctest::Approx(0.53).epsilon(0.3));
      // Opus' variable bitrate may exceed its target a bit
      CHECK(TableAudit::AnalyzeSoundFile(*sound, specs.lengthInSeconds).bitrate < 96000 * 5 / 4);

      // A lossy file of unknown length: no bitrate, no space saving re-encode
      VPX::SoundSpec unknownLengthSpecs = sourceSpecs;
      unknownLengthSpecs.lengthInSeconds = 0.f;
      CHECK(TableAudit::AnalyzeSoundFile(*sound, 0.f).bitrate == 0);
      CHECK(TableAudit::SoundFixes(*sound, unknownLengthSpecs).empty());

      undo.Undo(); // The original Vorbis data and path are restored
      CHECK(sound->GetFileSize() == oggSize);
      CHECK(sound->GetImportPath() == "ballrelease.ogg");

      table->Release();
   }

   SUBCASE("audit re-encodes a non-WebP image lossless, only when exact and smaller")
   {
      PinTable* const table = CreateTestTable();
      PinUndo undo(table);

      // PNG file data of a generated image, stored uncompressed so that WebP is smaller. With 'hiddenColors', the
      // right half is fully transparent but holds varied colors (an encoder not in 'exact' mode clears them)
      const auto makePng = [](const bool withAlpha, const bool hiddenColors)
      {
         FIBITMAP* const bitmap = FreeImage_Allocate(64, 64, withAlpha ? 32 : 24);
         for (unsigned int y = 0; y < 64; y++)
         {
            BYTE* line = FreeImage_GetScanLine(bitmap, y);
            for (unsigned int x = 0; x < 64; x++, line += withAlpha ? 4 : 3)
            {
               line[FI_RGBA_RED] = static_cast<BYTE>(x * 4);
               line[FI_RGBA_GREEN] = static_cast<BYTE>(y * 4);
               line[FI_RGBA_BLUE] = static_cast<BYTE>((x * y) & 0xFF);
               if (withAlpha)
                  line[FI_RGBA_ALPHA] = hiddenColors && x >= 32 ? 0 : 255;
            }
         }
         FIMEMORY* const mem = FreeImage_OpenMemory();
         FreeImage_SaveToMemory(FIF_PNG, bitmap, mem, PNG_Z_NO_COMPRESSION);
         FreeImage_Unload(bitmap);
         BYTE* data = nullptr;
         DWORD size = 0;
         FreeImage_AcquireMemory(mem, &data, &size);
         vector<uint8_t> png(data, data + size);
         FreeImage_CloseMemory(mem);
         return png;
      };
      const auto decode = [](const uint8_t* data, size_t size)
      {
         FIMEMORY* const mem = FreeImage_OpenMemory(const_cast<BYTE*>(data), static_cast<DWORD>(size));
         FIBITMAP* const bitmap = FreeImage_LoadFromMemory(FreeImage_GetFileTypeFromMemory(mem), mem, 0);
         FreeImage_CloseMemory(mem);
         vector<uint8_t> pixels;
         if (bitmap != nullptr)
         {
            FIBITMAP* const bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
            for (unsigned int y = 0; y < FreeImage_GetHeight(bitmap32); y++)
               pixels.insert(pixels.end(), FreeImage_GetScanLine(bitmap32, y), FreeImage_GetScanLine(bitmap32, y) + FreeImage_GetLine(bitmap32));
            FreeImage_Unload(bitmap32);
            FreeImage_Unload(bitmap);
         }
         return pixels;
      };

      // The texture object comes from an import, its file data is then replaced by the generated PNG
      Texture* const image = table->ImportImage(GetAssetPath() / "test000-default-table-Ref.webp", "auditimg"s);
      REQUIRE(image != nullptr);
      const auto findFix = [&table]()
      {
         TableAudit audit = TableAudit::Run(*table);
         const auto it = std::ranges::find_if(audit.items, [](const TableAuditItem& item) { return item.summary.find("'WebP'") != string::npos; });
         return it != audit.items.end() ? std::optional<TableAuditItem>(*it) : std::nullopt;
      };

      // Opaque PNG: lossless re-encode, same pixels, smaller
      {
         const vector<uint8_t> png = makePng(false, false);
         image->SetFromFileData("auditimg.png", png);
         const std::optional<TableAuditItem> fix = findFix();
         REQUIRE(fix.has_value());
         CHECK(fix->severity == TableAuditItem::Severity::Info);
         CHECK(fix->image == image);
         CHECK(fix->fixLabel == "Re-encode as WebP (lossless)");
         REQUIRE(fix->fix != nullptr);
         undo.BeginUndo();
         fix->fix(*table, undo);
         undo.EndUndo();
         REQUIRE(fix->fixFailure != nullptr);
         CHECK(fix->fixFailure->empty());
         CHECK(image->GetFilePath().extension() == ".webp");
         CHECK(memcmp(image->GetFileRaw(), "RIFF", 4) == 0); // WebP is RIFF containered
         CHECK(memcmp(image->GetFileRaw() + 8, "WEBP", 4) == 0);
         CHECK(image->GetFileSize() < png.size());
         CHECK(decode(image->GetFileRaw(), image->GetFileSize()) == decode(png.data(), png.size()));

         undo.Undo(); // The original blob and path are restored
         CHECK(image->GetFilePath().extension() == ".png");
         CHECK(image->GetFileSize() == png.size());
      }

      // Colors under fully transparent pixels would be altered by the lossless encoder: the image is kept as is
      {
         const vector<uint8_t> png = makePng(true, true);
         image->SetFromFileData("auditimg.png", png);
         const std::optional<TableAuditItem> fix = findFix();
         REQUIRE(fix.has_value());
         undo.BeginUndo();
         fix->fix(*table, undo);
         undo.EndUndo();
         undo.Discard();
         REQUIRE(fix->fixFailure != nullptr);
         CHECK(fix->fixFailure->find("alter pixels") != string::npos);
         CHECK(image->GetFilePath().extension() == ".png");
         CHECK(image->GetFileSize() == png.size());
      }

      // A JPEG is offered a lossy re-encode, at its quality estimated from its quantization tables
      {
         const auto makeJpeg = [](const int flags)
         {
            FIBITMAP* const bitmap = FreeImage_Allocate(64, 64, 24);
            FIMEMORY* const mem = FreeImage_OpenMemory();
            FreeImage_SaveToMemory(FIF_JPEG, bitmap, mem, flags);
            FreeImage_Unload(bitmap);
            BYTE* data = nullptr;
            DWORD size = 0;
            FreeImage_AcquireMemory(mem, &data, &size);
            vector<uint8_t> jpeg(data, data + size);
            FreeImage_CloseMemory(mem);
            return jpeg;
         };
         bool exact = false;
         for (const int quality : { 100, 90, 75, 30, 5 })
         {
            const vector<uint8_t> jpeg = makeJpeg(quality); // FreeImage takes the JPEG quality in the low bits of the flags
            CHECK(TableAudit::EstimateJpegQuality(jpeg.data(), jpeg.size(), exact) == quality);
            CHECK(exact);
         }

         // Tables that are not scaled standard ones (one coefficient changed): an equivalent quality from their average scale
         vector<uint8_t> jpeg = makeJpeg(90);
         const auto dqt = std::ranges::search(jpeg, std::array<uint8_t, 2> { 0xFF, 0xDB });
         REQUIRE(!dqt.empty());
         jpeg[dqt.begin() - jpeg.begin() + 5 + 63]++; // Last coefficient of the first table (2 bytes marker, 2 bytes length, 1 byte id)
         const int estimate = TableAudit::EstimateJpegQuality(jpeg.data(), jpeg.size(), exact);
         CHECK(!exact);
         CHECK(estimate >= 88);
         CHECK(estimate <= 92);

         // Not a JPEG
         const vector<uint8_t> png = makePng(false, false);
         CHECK(TableAudit::EstimateJpegQuality(png.data(), png.size(), exact) == 0);

         image->SetFromFileData("auditimg.jpg", makeJpeg(90));
         const std::optional<TableAuditItem> fix = findFix();
         REQUIRE(fix.has_value());
         CHECK(fix->fixLabel == "Re-encode as WebP (lossy)");
         CHECK(fix->details.find("quality 90, the quality of the JPEG") != string::npos);
         CHECK(fix->details.find("4:4:4") == string::npos); // FreeImage's default is 4:2:0

         // Full color resolution (4:4:4): the details tell that lossy WebP halves it
         const vector<uint8_t> jpeg444 = makeJpeg(90 | JPEG_SUBSAMPLING_444);
         CHECK(TableAudit::JpegHasFullColorResolution(jpeg444.data(), jpeg444.size()));
         const vector<uint8_t> jpeg420 = makeJpeg(90 | JPEG_SUBSAMPLING_420);
         CHECK(!TableAudit::JpegHasFullColorResolution(jpeg420.data(), jpeg420.size()));
         image->SetFromFileData("auditimg.jpg", jpeg444);
         const std::optional<TableAuditItem> fix444 = findFix();
         REQUIRE(fix444.has_value());
         CHECK(fix444->details.find("4:4:4") != string::npos);
      }

      // A 16 bit per channel PNG is reduced to the 8 bits VPX renders, then stored lossless
      {
         FIBITMAP* const bitmap = FreeImage_AllocateT(FIT_RGB16, 64, 64);
         for (unsigned int y = 0; y < 64; y++)
         {
            FIRGB16* const line = reinterpret_cast<FIRGB16*>(FreeImage_GetScanLine(bitmap, y));
            for (unsigned int x = 0; x < 64; x++)
               line[x] = { static_cast<WORD>(x * 1000 + 7), static_cast<WORD>(y * 1000 + 3), static_cast<WORD>((x * y * 13) & 0xFFFF) };
         }
         FIMEMORY* const mem = FreeImage_OpenMemory();
         FreeImage_SaveToMemory(FIF_PNG, bitmap, mem, PNG_Z_NO_COMPRESSION);
         BYTE* data = nullptr;
         DWORD size = 0;
         FreeImage_AcquireMemory(mem, &data, &size);
         const vector<uint8_t> png16(data, data + size);
         FreeImage_CloseMemory(mem);
         FIBITMAP* const rendered = FreeImage_ConvertTo24Bits(bitmap); // As VPX's loader converts it
         FreeImage_Unload(bitmap);
         FIMEMORY* const renderedMem = FreeImage_OpenMemory();
         FreeImage_SaveToMemory(FIF_PNG, rendered, renderedMem, 0);
         FreeImage_Unload(rendered);
         FreeImage_AcquireMemory(renderedMem, &data, &size);
         const vector<uint8_t> renderedPng(data, data + size);
         FreeImage_CloseMemory(renderedMem);

         image->SetFromFileData("auditimg.png", png16);
         const std::optional<TableAuditItem> fix = findFix();
         REQUIRE(fix.has_value());
         CHECK(fix->fixLabel == "Re-encode as WebP (as rendered, 8 bit)");
         undo.BeginUndo();
         fix->fix(*table, undo);
         undo.EndUndo();
         CHECK(fix->fixFailure->empty());
         CHECK(image->GetFilePath().extension() == ".webp");
         CHECK(decode(image->GetFileRaw(), image->GetFileSize()) == decode(renderedPng.data(), renderedPng.size()));
      }

      table->Release();
   }

   SUBCASE("audit re-encodes an HDR image to half float EXR only when no value needs clamping")
   {
      PinTable* const table = CreateTestTable();
      PinUndo undo(table);

      // Uncompressed 32 bit float TIFF file data of a smooth gradient, with an optional value beyond the half float range
      const auto makeTiff = [](const float peak)
      {
         FIBITMAP* const bitmap = FreeImage_AllocateT(FIT_RGBF, 32, 32);
         for (unsigned int y = 0; y < 32; y++)
         {
            FIRGBF* const line = reinterpret_cast<FIRGBF*>(FreeImage_GetScanLine(bitmap, y));
            for (unsigned int x = 0; x < 32; x++)
               line[x] = { static_cast<float>(x) * 31.3f + 0.1f, static_cast<float>(y) * 0.0137f, 1000.f / static_cast<float>(1 + x + y) };
         }
         reinterpret_cast<FIRGBF*>(FreeImage_GetScanLine(bitmap, 5))[7].red = peak;
         FIMEMORY* const mem = FreeImage_OpenMemory();
         FreeImage_SaveToMemory(FIF_TIFF, bitmap, mem, TIFF_NONE);
         FreeImage_Unload(bitmap);
         BYTE* data = nullptr;
         DWORD size = 0;
         FreeImage_AcquireMemory(mem, &data, &size);
         vector<uint8_t> tiff(data, data + size);
         FreeImage_CloseMemory(mem);
         return tiff;
      };
      // Decoded values as RGB floats
      const auto decode = [](const uint8_t* data, size_t size)
      {
         FIMEMORY* const mem = FreeImage_OpenMemory(const_cast<BYTE*>(data), static_cast<DWORD>(size));
         FIBITMAP* const bitmap = FreeImage_LoadFromMemory(FreeImage_GetFileTypeFromMemory(mem), mem, 0);
         FreeImage_CloseMemory(mem);
         vector<float> values;
         if (bitmap != nullptr)
         {
            FIBITMAP* const rgbf = FreeImage_ConvertToRGBF(bitmap);
            for (unsigned int y = 0; y < FreeImage_GetHeight(rgbf); y++)
            {
               const float* const line = reinterpret_cast<const float*>(FreeImage_GetScanLine(rgbf, y));
               values.insert(values.end(), line, line + FreeImage_GetWidth(rgbf) * 3);
            }
            FreeImage_Unload(rgbf);
            FreeImage_Unload(bitmap);
         }
         return values;
      };

      // Without decoded data, a texture is HDR by its file extension: the TIFF data is labeled as a Radiance '.hdr' file
      const std::filesystem::path dir = GetAssetPath() / "tmp";
      std::filesystem::create_directories(dir);
      const vector<uint8_t> inRange = makeTiff(60000.f);
      {
         std::ofstream file(dir / "audit-hdr.tif", std::ios::binary);
         file.write(reinterpret_cast<const char*>(inRange.data()), static_cast<std::streamsize>(inRange.size()));
      }
      Texture* const image = table->ImportImage(dir / "audit-hdr.tif", "audithdr"s);
      REQUIRE(image != nullptr);
      image->SetFromFileData(dir / "audit-hdr.hdr", inRange);
      REQUIRE(image->IsHDR());

      const auto applyFix = [&table, &undo]()
      {
         TableAudit audit = TableAudit::Run(*table);
         const auto it = std::ranges::find_if(audit.items, [](const TableAuditItem& item) { return item.summary.find("'EXR'") != string::npos; });
         REQUIRE(it != audit.items.end());
         CHECK(it->fixLabel == "Re-encode as EXR");
         undo.BeginUndo();
         it->fix(*table, undo);
         undo.EndUndo();
         REQUIRE(it->fixFailure != nullptr);
         CHECK(it->fixFailure->empty());
      };

      // All values in the half float range: stored as half floats (close, not exact)
      {
         applyFix();
         CHECK(image->GetFilePath().extension() == ".exr");
         const vector<float> source = decode(inRange.data(), inRange.size());
         const vector<float> stored = decode(image->GetFileRaw(), image->GetFileSize());
         REQUIRE(stored.size() == source.size());
         CHECK(stored != source); // Half float precision
         for (size_t i = 0; i < source.size(); i++)
            CHECK(stored[i] == doctest::Approx(source[i]).epsilon(1e-3));
         undo.Undo();
         CHECK(image->GetFilePath().extension() == ".hdr");
      }

      // A value beyond the half float range would be clamped: stored as 32 bit floats, exactly
      {
         const vector<uint8_t> outOfRange = makeTiff(1.0e6f);
         image->SetFromFileData(dir / "audit-hdr.hdr", outOfRange);
         applyFix();
         CHECK(image->GetFilePath().extension() == ".exr");
         CHECK(decode(image->GetFileRaw(), image->GetFileSize()) == decode(outOfRange.data(), outOfRange.size()));
      }

      table->Release();
   }

   SUBCASE("sound re-encode bitrate follows the sample rate, low quality sounds are not re-encoded to save space")
   {
      // PCM WAV of a sine wave
      const auto makeWav = [](const int sampleRate, const int bits, const int channels)
      {
         vector<uint8_t> wav = MakeStereoSineWav(0.2f, sampleRate);
         if (bits == 16 && channels == 2)
            return wav;
         // Rebuild the data for another layout: 'MakeStereoSineWav' holds 16 bit stereo frames after a 44 byte header
         vector<uint8_t> samples;
         for (size_t i = 44; i + 4 <= wav.size(); i += 4)
            for (int c = 0; c < channels; c++)
            {
               const int16_t sample = static_cast<int16_t>(wav[i] | (wav[i + 1] << 8));
               if (bits == 8)
                  samples.push_back(static_cast<uint8_t>((sample >> 8) + 128));
               else
               {
                  samples.push_back(wav[i]);
                  samples.push_back(wav[i + 1]);
               }
            }
         const auto put32 = [&wav](size_t pos, uint32_t v) { for (int b = 0; b < 4; b++) wav[pos + b] = static_cast<uint8_t>(v >> (8 * b)); };
         const auto put16 = [&wav](size_t pos, uint32_t v) { for (int b = 0; b < 2; b++) wav[pos + b] = static_cast<uint8_t>(v >> (8 * b)); };
         wav.resize(44);
         put32(4, static_cast<uint32_t>(36 + samples.size()));
         put16(22, static_cast<uint32_t>(channels));
         put32(28, static_cast<uint32_t>(sampleRate * channels * bits / 8));
         put16(32, static_cast<uint32_t>(channels * bits / 8));
         put16(34, static_cast<uint32_t>(bits));
         put32(40, static_cast<uint32_t>(samples.size()));
         wav.insert(wav.end(), samples.begin(), samples.end());
         return wav;
      };
      VPX::AudioPlayer player(""s, ""s, VPX::SNDCFG_SND3D2CH, VPX::SNDCFG_BG2CH, ""s, VPX::SNDCFG_SND3D2CH);
      const auto fixesFor = [&player](const vector<uint8_t>& wav, const VPX::SoundOutTypes target)
      {
         VPX::Sound sound("Snd"s, "snd.wav", wav);
         sound.SetOutputTarget(target);
         return TableAudit::SoundFixes(sound, player.GetSoundInformations(&sound));
      };

      // Bit depth from the WAV header
      {
         VPX::Sound sound("Snd"s, "snd.wav", makeWav(22050, 8, 1));
         CHECK(TableAudit::AnalyzeSoundFile(sound, 0.2f).bitsPerSample == 8);
      }

      // Space saving re-encode: full bitrate at 44.1kHz, 3/4 at 22kHz (encoded at 24kHz)
      vector<TableAuditItem> fixes = fixesFor(makeWav(44100, 16, 2), VPX::SNDOUT_BACKGLASS);
      REQUIRE(fixes.size() == 1);
      CHECK(fixes[0].details.find("at 128 kbit/s") != string::npos);
      fixes = fixesFor(makeWav(22050, 16, 2), VPX::SNDOUT_BACKGLASS);
      REQUIRE(fixes.size() == 1);
      CHECK(fixes[0].details.find("at 96 kbit/s") != string::npos);
      fixes = fixesFor(makeWav(22050, 16, 1), VPX::SNDOUT_BACKGLASS);
      REQUIRE(fixes.size() == 1);
      CHECK(fixes[0].details.find("at 72 kbit/s") != string::npos);

      // Low quality (11kHz or less, 8 bit): not re-encoded to save space
      CHECK(fixesFor(makeWav(11025, 16, 2), VPX::SNDOUT_BACKGLASS).empty());
      CHECK(fixesFor(makeWav(22050, 8, 1), VPX::SNDOUT_BACKGLASS).empty());

      // ... but a stereo playfield sound still gets the mono fix, at a bitrate for its rate (12kHz: half of 96 kbit/s)
      fixes = fixesFor(makeWav(11025, 16, 2), VPX::SNDOUT_TABLE);
      REQUIRE(fixes.size() == 1);
      CHECK(fixes[0].severity == TableAuditItem::Severity::Error);
      CHECK(fixes[0].details.find("at 48 kbit/s") != string::npos);
   }

   SUBCASE("undo of a re-encode skips a deleted asset")
   {
      PinTable* const table = CreateTestTable();
      PinUndo undo(table);

      VPX::Sound* const sound = new VPX::Sound("Snd1", "snd1.wav", MakeStereoSineWav(0.1f, 22050));
      table->m_vsound.push_back(sound);
      VPX::AudioPlayer player(""s, ""s, VPX::SNDCFG_SND3D2CH, VPX::SNDCFG_BG2CH, ""s, VPX::SNDCFG_SND3D2CH);
      const TableAudit::SoundFileInfo source = TableAudit::AnalyzeSoundFile(*sound, 0.1f);
      CHECK(source.codec == TableAudit::SoundFileCodec::Lossless); // PCM WAV
      CHECK(source.bitrate == 0);
      const vector<TableAuditItem> fixes = TableAudit::SoundFixes(*sound, player.GetSoundInformations(sound));
      REQUIRE(!fixes.empty());

      undo.BeginUndo();
      fixes.front().fix(*table, undo);
      undo.EndUndo();
      CHECK(sound->GetImportPath().extension() == ".opus");

      table->RemoveSound(sound); // Not undoable: the re-encode record now refers to a deleted sound
      CHECK(table->m_vsound.empty());
      undo.Undo(); // Must neither restore into the freed sound nor crash
      CHECK(table->m_vsound.empty());
      undo.Redo();
      CHECK(table->m_vsound.empty());

      table->Release();
   }

   SUBCASE("default playfield to cabinet transform applies the authored slope")
   {
      PinTable* const table = CreateTestTable();
      table->m_bottom = 2000.f;
      table->m_glassBottomHeight = 60.f;
      table->m_groundToLockbarHeight = 900.f;
      const float h = table->m_groundToLockbarHeight - table->m_glassBottomHeight;

      // A level table keeps the legacy translation-only transform (ground below the playfield)
      table->m_angletiltMin = table->m_angletiltMax = 0.f;
      const Matrix3D flat = table->GetSceneLayout().GetSpaceToPlayfieldTransform(VPX::SpaceReference::Room).m_transform;
      const Vertex3Ds flatOrigin = flat * Vertex3Ds(0.f, 0.f, 0.f);
      CHECK(flatOrigin.x == doctest::Approx(0.f).epsilon(1e-5));
      CHECK(flatOrigin.y == doctest::Approx(0.f).epsilon(1e-5));
      CHECK(flatOrigin.z == doctest::Approx(-h).epsilon(1e-5));
      CHECK(flat.MultiplyVectorNoTranslate(Vertex3Ds(0.f, 0.f, 1.f)).z == doctest::Approx(1.f).epsilon(1e-5));

      table->m_angletiltMin = table->m_angletiltMax = 6.f;
      const float slope = ANGTORAD(table->GetPlayfieldSlope());
      const Matrix3D toPf = table->GetSceneLayout().GetSpaceToPlayfieldTransform(VPX::SpaceReference::Room).m_transform;
      const Matrix3D toRoom = Matrix3D::MatrixInverse(toPf);

      // The lockbar line is the pivot: it stays at the authored lockbar height above the ground, the glass
      // distance being perpendicular to the playfield (the front edge sits below it, slightly toward the player)
      const Vertex3Ds lockbar = toPf * Vertex3Ds(0.f, table->m_bottom, table->m_groundToLockbarHeight);
      CHECK(lockbar.y == doctest::Approx(table->m_bottom).epsilon(1e-5));
      CHECK(lockbar.z == doctest::Approx(table->m_glassBottomHeight).epsilon(1e-5));
      const Vertex3Ds front = toRoom * Vertex3Ds(0.f, table->m_bottom, 0.f);
      CHECK(front.y == doctest::Approx(table->m_bottom - table->m_glassBottomHeight * sinf(slope)).epsilon(1e-5));
      CHECK(front.z == doctest::Approx(table->m_groundToLockbarHeight - table->m_glassBottomHeight * cosf(slope)).epsilon(1e-5));
      const Vertex3Ds back = toRoom * Vertex3Ds(0.f, 0.f, 0.f);
      CHECK(back.z == doctest::Approx(front.z + table->m_bottom * sinf(slope)).epsilon(1e-5));

      // The room axes are inclined by the slope in playfield space: X is shared (pivot axis),
      // the room floor rises toward the front of the table, the up axis leans toward the back
      const Vertex3Ds xAxis = toPf.MultiplyVectorNoTranslate(Vertex3Ds(1.f, 0.f, 0.f));
      CHECK(xAxis.y == doctest::Approx(0.f).epsilon(1e-5));
      CHECK(xAxis.z == doctest::Approx(0.f).epsilon(1e-5));
      const Vertex3Ds yAxis = toPf.MultiplyVectorNoTranslate(Vertex3Ds(0.f, 1.f, 0.f));
      CHECK(yAxis.y == doctest::Approx(cosf(slope)).epsilon(1e-5));
      CHECK(yAxis.z == doctest::Approx(sinf(slope)).epsilon(1e-5));
      const Vertex3Ds zAxis = toPf.MultiplyVectorNoTranslate(Vertex3Ds(0.f, 0.f, 1.f));
      CHECK(zAxis.y == doctest::Approx(-sinf(slope)).epsilon(1e-5));
      CHECK(zAxis.z == doctest::Approx(cosf(slope)).epsilon(1e-5));

      // Round trip
      const Vertex3Ds p = toPf * (toRoom * Vertex3Ds(3.f, -7.f, 11.f));
      CHECK(p.x == doctest::Approx(3.f).epsilon(1e-4));
      CHECK(p.y == doctest::Approx(-7.f).epsilon(1e-4));
      CHECK(p.z == doctest::Approx(11.f).epsilon(1e-4));

      table->Release();
   }

   SUBCASE("scene layout room to playfield transform matches the authored cabinet geometry")
   {
      PinTable* const table = CreateTestTable();
      table->m_bottom = 2000.f;
      table->m_glassBottomHeight = 60.f;
      table->m_groundToLockbarHeight = 900.f;
      table->m_angletiltMin = table->m_angletiltMax = 6.f;

      // Authored layout, written independently of SceneLayout: the playfield is inclined by the authored slope around the
      // lockbar line, at the authored lockbar height above the ground, and lies the glass distance below it, perpendicular
      // to the playfield
      const Matrix3D authoredRoomToPf = Matrix3D::MatrixTranslate(0.f, -table->m_bottom, -table->m_groundToLockbarHeight)
         * Matrix3D::MatrixRotateX(ANGTORAD(table->GetPlayfieldSlope())) * Matrix3D::MatrixTranslate(0.f, table->m_bottom, table->m_glassBottomHeight);
      const Matrix3D toPf = table->GetSceneLayout().GetSpaceToPlayfieldTransform(VPX::SpaceReference::Room).m_transform;
      for (const Vertex3Ds& p : { Vertex3Ds(0.f, 0.f, 0.f), Vertex3Ds(100.f, table->m_bottom, table->m_groundToLockbarHeight), Vertex3Ds(-300.f, 500.f, 1200.f) })
      {
         const Vertex3Ds a = authoredRoomToPf * p;
         const Vertex3Ds b = toPf * p;
         CHECK(a.x == doctest::Approx(b.x).epsilon(1e-3));
         CHECK(a.y == doctest::Approx(b.y).epsilon(1e-3));
         CHECK(a.z == doctest::Approx(b.z).epsilon(1e-3));
      }

      table->Release();
   }
}

TEST_CASE("PinTable image import and removal with the lookup table in use")
{
   // Once the lookup table is set up (play, live editor), GetImage only looks there: an imported image must be found right away, and a
   // reimport under the same name must not leave the table pointing at the deleted image
   PinTable* const table = CreateTestTable();
   table->SetupLookUpTables(true);
   const std::filesystem::path file = GetAssetPath() / "test000-default-table-Ref.webp";
   Texture* const image = table->ImportImage(file, "imported"s);
   REQUIRE(image != nullptr);
   CHECK(table->GetImage("imported"s) == image);
   Texture* const reimported = table->ImportImage(file, "imported"s); // Deletes 'image'
   REQUIRE(reimported != nullptr);
   CHECK(table->GetImage("imported"s) == reimported);
   CHECK(table->GetImageList().size() == 1);
   // An import without a name takes the file title, and a second one with the same title does not take over the lookup entry
   Texture* const titled = table->ImportImage(file, ""s);
   REQUIRE(titled != nullptr);
   CHECK(table->GetImage(titled->m_name) == titled);
   Texture* const duplicate = table->ImportImage(file, ""s);
   REQUIRE(duplicate != nullptr);
   CHECK(table->GetImage(titled->m_name) == titled);
   // Removing an image hands its lookup entry to another image of the same name, or drops it: never a dangling pointer
   const string title = titled->m_name;
   table->RemoveImage(titled); // Deletes 'titled'
   CHECK(table->GetImage(title) == duplicate);
   table->RemoveImage(duplicate);
   CHECK(table->GetImage(title) == nullptr);
   CHECK(table->GetImage("imported"s) == reimported);
   table->Release();
}

TEST_CASE("ScriptInterpreter syntax check parses the table script without running it")
{
   PinTable* const table = CreateTestTable();
   // Script check of the live editor (F7): the number of errors reported for a script
   const auto check = [table](const string& script)
   {
      int errors = 0;
      CComObject<ScriptInterpreter>* interpreter;
      CComObject<ScriptInterpreter>::CreateInstance(&interpreter);
      interpreter->AddRef();
      interpreter->Start(table);
      interpreter->SetScriptErrorHandler([&errors](ScriptInterpreter::ErrorType, int, int, const string&, const vector<string>&) { errors++; });
      interpreter->Parse(script);
      interpreter->Stop(table);
      interpreter->Release();
      return errors;
   };
   CHECK(check("Sub Foo()\nEnd Sub\n") == 0);
   CHECK(check("Sub Foo(\nEnd Sub\n") == 1); // Syntax error
   CHECK(check("Err.Raise 5\n") == 0); // Top-level code that would fail if it ran
   table->Release();
}
