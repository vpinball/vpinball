// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "audio/AudioPlayer.h"
#include "audio/AudioStreamPlayer.h"
#include "parts/Sound.h"

#include "doctest.h"

using namespace VPX;

namespace
{

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

// Builds a stereo 16 bit 44.1kHz WAV file holding durationS seconds of a sine wave
vector<uint8_t> MakeSineWav(float durationS)
{
   const int nFrames = static_cast<int>(44100.f * durationS);
   vector<uint8_t> samples;
   samples.reserve(nFrames * 4);
   for (int i = 0; i < nFrames; ++i)
   {
      const int16_t sample = static_cast<int16_t>(8000.0 * sin(2.0 * M_PI * 440.0 * i / 44100.0));
      samples.push_back(static_cast<uint8_t>(sample & 0xFF));
      samples.push_back(static_cast<uint8_t>((sample >> 8) & 0xFF));
      samples.push_back(static_cast<uint8_t>(sample & 0xFF));
      samples.push_back(static_cast<uint8_t>((sample >> 8) & 0xFF));
   }
   return MakeWavFile(std::move(samples));
}

} // namespace

TEST_CASE("Audio player")
{
   // Use the SDL dummy audio driver so the tests do not depend on audio hardware
   SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");

   SUBCASE("device enumeration")
   {
      const vector<AudioPlayer::AudioDevice> devices = AudioPlayer::EnumerateAudioDevices();
      REQUIRE(!devices.empty()); // the dummy driver always exposes a playback device
      for (const auto& device : devices)
      {
         CHECK(!device.name.empty());
         CHECK(device.channels > 0);
      }
   }

   SUBCASE("player construction and stream lifecycle")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH);
      CHECK(player.GetSoundMode3D() == SNDCFG_SND3D2CH);
      CHECK(player.GetBackglassDeviceName() != "Error"s);
      CHECK(player.GetPlayfieldDeviceName() != "Error"s);

      AudioPlayer::AudioStreamID stream = player.OpenAudioStream("test-stream", 44100, 2, true);
      REQUIRE(stream != nullptr);
      CHECK(player.IsOpened(stream));

      // Enqueue half a second of float32 stereo silence
      vector<uint8_t> pcm(44100 * 2 * sizeof(float) / 2, 0);
      player.EnqueueStream(stream, pcm.data(), static_cast<int>(pcm.size()));
      CHECK(stream->GetQueuedSize() > 0); // the dummy driver drains the queue in real time

      player.SetStreamVolume(stream, 0.5f);
      player.CloseAudioStream(stream, false);
      CHECK(!player.IsOpened(stream));
   }

   SUBCASE("closing an unknown stream keeps the opened streams")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH);
      AudioPlayer::AudioStreamID stream = player.OpenAudioStream("test-stream", 44100, 2, true);
      AudioPlayer::AudioStreamID unknown;
      player.CloseAudioStream(unknown, false); // logs an error, must not remove the valid stream
      CHECK(player.IsOpened(stream));
   }

   SUBCASE("sound information decoding")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH);
      VPX::Sound sound("TestSound", "test.wav", MakeSineWav(0.1f));

      const SoundSpec spec = player.GetSoundInformations(&sound);
      CHECK(spec.nChannels == 2);
      CHECK(spec.sampleFrequency == 44100);
      CHECK(spec.lengthInSeconds == doctest::Approx(0.1).epsilon(0.2));
   }

   SUBCASE("play and stop a sound")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH);
      VPX::Sound sound("TestSound", "test.wav", MakeSineWav(0.5f));

      player.SetMainVolume(0.5f, 0.5f);
      player.PlaySound(&sound, 0.f, 0.f, 0, 0.f, 0.f, 1, false, true);
      player.StopSound(&sound);
   }

   SUBCASE("music playback commands")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH);

      const std::filesystem::path musicFile = GetTestTmpDir() / "test-music.wav";
      write_file(musicFile.string(), MakeSineWav(0.5f), true);

      CHECK(player.PlayMusic(musicFile.string()));
      CHECK(player.GetMusicPosition() >= 0.f);
      player.PauseMusic();
      player.UnpauseMusic();
      player.SetMusicVolume(0.5f);
      player.SetMusicPosition(0.f);
      CHECK(player.GetMusicPosition() >= 0.f);

      std::error_code ec;
      std::filesystem::remove(musicFile, ec);
   }
}
