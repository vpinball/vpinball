// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "parts/Sound.h"
#include "core/vpversion.h"

#include "pole/pole.h"

#include "doctest.h"

using VPX::Sound;

namespace
{

// Writes an in-memory stream into a structured storage file, then reopens it for reading.
// Sound streams are stored in the "GameStg/SoundN" streams of a .vpx file.
void SaveSoundToFileStorage(const InMemStream& data, const std::filesystem::path& file)
{
   std::error_code ec;
   std::filesystem::remove(file, ec);
   POLE::Storage storage(file.string().c_str());
   REQUIRE(storage.open(true, true));
   {
      POLE::Stream stream(&storage, "GameStg/Sound0", true);
      REQUIRE(stream.write(const_cast<uint8_t*>(data.Data()), data.Size()) == data.Size());
      stream.flush();
   }
   storage.close();
}

Sound* LoadSoundFromFileStorage(const std::filesystem::path& file)
{
   POLE::Storage storage(file.string().c_str());
   REQUIRE(storage.open());
   POLE::Stream stream(&storage, "GameStg/Sound0");
   REQUIRE_FALSE(stream.fail());
   Sound* const sound = Sound::CreateFromStream(stream, CURRENT_FILE_FORMAT_VERSION);
   storage.close();
   return sound;
}

// Builds a minimal canonical PCM WAV file (44 byte header + samples)
vector<uint8_t> MakeWavFile(vector<uint8_t> samples)
{
   auto append32 = [](vector<uint8_t>& out, uint32_t v)
   {
      out.push_back(static_cast<uint8_t>(v & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
   };
   auto append16 = [](vector<uint8_t>& out, uint16_t v)
   {
      out.push_back(static_cast<uint8_t>(v & 0xFF));
      out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
   };

   vector<uint8_t> wav;
   wav.insert(wav.end(), { 'R', 'I', 'F', 'F' });
   append32(wav, static_cast<uint32_t>(36 + samples.size()));
   wav.insert(wav.end(), { 'W', 'A', 'V', 'E' });
   wav.insert(wav.end(), { 'f', 'm', 't', ' ' });
   append32(wav, 16); // fmt chunk size
   append16(wav, 1); // PCM
   append16(wav, 2); // stereo
   append32(wav, 44100);
   append32(wav, 44100 * 2 * 2);
   append16(wav, 4); // block align
   append16(wav, 16); // bits per sample
   wav.insert(wav.end(), { 'd', 'a', 't', 'a' });
   append32(wav, static_cast<uint32_t>(samples.size()));
   wav.insert(wav.end(), samples.begin(), samples.end());
   return wav;
}

} // namespace

TEST_CASE("Sound")
{
   SUBCASE("file import and export")
   {
      const std::filesystem::path file = GetTestTmpDir() / "testsound.bin";
      const vector<uint8_t> payload = { 0x11, 0x22, 0x33, 0x44, 0x55 };
      write_file(file.string(), payload, true);
      REQUIRE(read_file(file, true) == payload);

      std::unique_ptr<Sound> sound(Sound::CreateFromFile(file));
      REQUIRE(sound != nullptr);
      CHECK(sound->GetName() == "testsound");
      CHECK(sound->GetImportPath() == file);
      CHECK(sound->GetFileSize() == payload.size());
      CHECK(memcmp(sound->GetFileRaw(), payload.data(), payload.size()) == 0);

      const std::filesystem::path exported = GetTestTmpDir() / "testsound-export.bin";
      CHECK(sound->SaveToFile(exported));
      CHECK(read_file(exported, true) == payload);

      std::error_code ec;
      std::filesystem::remove(file, ec);
      std::filesystem::remove(exported, ec);
   }

   SUBCASE("stream round-trip of a non-WAV sound keeps raw data and settings")
   {
      const vector<uint8_t> payload = { 9, 8, 7, 6, 5, 4, 3, 2, 1 };
      Sound sound("MySound", "sounds/effect.mp3", payload);
      sound.SetOutputTarget(VPX::SNDOUT_BACKGLASS);
      sound.SetVolume(-50);
      sound.SetPan(25);
      sound.SetFrontRearFade(-10);

      InMemStream stream;
      sound.SaveToStream(&stream);

      const std::filesystem::path file = GetTestTmpDir() / "sound-stream.vpx";
      SaveSoundToFileStorage(stream, file);
      std::unique_ptr<Sound> loaded(LoadSoundFromFileStorage(file));
      REQUIRE(loaded != nullptr);

      CHECK(loaded->GetName() == "MySound");
      CHECK(loaded->GetImportPath() == std::filesystem::path("sounds/effect.mp3"));
      CHECK(loaded->GetFileSize() == payload.size());
      CHECK(memcmp(loaded->GetFileRaw(), payload.data(), payload.size()) == 0);
      CHECK(loaded->GetOutputTarget() == VPX::SNDOUT_BACKGLASS);
      CHECK(loaded->GetVolume() == -50);
      CHECK(loaded->GetPan() == 25);
      CHECK(loaded->GetFrontRearFade() == -10);
   }

   SUBCASE("stream round-trip of a WAV sound rebuilds the RIFF header")
   {
      // WAV files are stored as WAVEFORMATEX + raw samples; the RIFF header is
      // recreated on load, so a canonical WAV file must round-trip byte-exactly
      const vector<uint8_t> wav = MakeWavFile({ 1, 0, 2, 0, 3, 0, 4, 0 });
      Sound sound("WavSound", "fx/hit.wav", wav);

      InMemStream stream;
      sound.SaveToStream(&stream);

      const std::filesystem::path file = GetTestTmpDir() / "sound-wav.vpx";
      SaveSoundToFileStorage(stream, file);
      std::unique_ptr<Sound> loaded(LoadSoundFromFileStorage(file));
      REQUIRE(loaded != nullptr);

      CHECK(loaded->GetName() == "WavSound");
      CHECK(loaded->GetFileSize() == wav.size());
      CHECK(memcmp(loaded->GetFileRaw(), wav.data(), wav.size()) == 0);
   }
}
