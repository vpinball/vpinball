// license:GPLv3+

#include "core/stdafx.h"
#include "../vpx-test.h"

#include "audio/AudioPlayer.h"
#include "audio/AudioStreamPlayer.h"
#include "audio/MixNode.h"
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

// 0.1 s of a 440 Hz sine, stereo 48 kHz, as an Ogg Opus file (ffmpeg -c:a libopus -b:a 24k)
const vector<uint8_t> opusSine {
   0x4F, 0x67, 0x67, 0x53, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0xD5, 0xC5, 0xF7, 0x01, 0x13, 0x4F, 0x70, 0x75, 0x73,
   0x48, 0x65, 0x61, 0x64, 0x01, 0x02, 0x38, 0x01, 0x80, 0xBB, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4F,
   0x67, 0x67, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x01, 0x00, 0x00, 0x00, 0x49, 0x95, 0xBE, 0x54, 0x01, 0x2E, 0x4F, 0x70, 0x75, 0x73, 0x54,
   0x61, 0x67, 0x73, 0x06, 0x00, 0x00, 0x00, 0x66, 0x66, 0x6D, 0x70, 0x65, 0x67, 0x01, 0x00, 0x00,
   0x00, 0x14, 0x00, 0x00, 0x00, 0x65, 0x6E, 0x63, 0x6F, 0x64, 0x65, 0x72, 0x3D, 0x4C, 0x61, 0x76,
   0x63, 0x20, 0x6C, 0x69, 0x62, 0x6F, 0x70, 0x75, 0x73, 0x4F, 0x67, 0x67, 0x53, 0x00, 0x04, 0xF8,
   0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x8A,
   0x39, 0x87, 0xF0, 0x06, 0x4E, 0x3F, 0x3D, 0x3B, 0x3F, 0x2C, 0x7C, 0x87, 0xFC, 0xB8, 0x4B, 0x84,
   0x83, 0xFD, 0x32, 0x68, 0x00, 0x29, 0xA8, 0xA7, 0xAA, 0xB0, 0xF4, 0xF8, 0x68, 0xA5, 0x32, 0x81,
   0x7B, 0x1B, 0xF0, 0x9C, 0xD1, 0xF1, 0x51, 0x1A, 0xDD, 0x37, 0x9C, 0x3E, 0x00, 0xBA, 0xF4, 0xA7,
   0x97, 0x6B, 0x32, 0x67, 0x03, 0xAC, 0xB5, 0x6B, 0x1A, 0xF8, 0x11, 0x79, 0x2A, 0xA5, 0x86, 0xAB,
   0xF2, 0xB0, 0xC0, 0x07, 0x26, 0xDE, 0x12, 0x82, 0x5D, 0x25, 0x6D, 0xBB, 0xA8, 0xFD, 0xDF, 0x92,
   0x15, 0xB1, 0x66, 0x97, 0xCE, 0x76, 0xCE, 0x40, 0x7C, 0x87, 0xFF, 0x32, 0xC3, 0xCA, 0xB0, 0x36,
   0x9A, 0x33, 0x48, 0xD4, 0x86, 0xAE, 0x7C, 0x9D, 0x2B, 0x95, 0x5D, 0xF6, 0xAC, 0x6C, 0xBB, 0x0D,
   0xE7, 0x01, 0x9B, 0x4A, 0x06, 0x63, 0xCA, 0xD7, 0x2A, 0x74, 0x4B, 0x85, 0x06, 0xC2, 0x48, 0xDE,
   0x57, 0x3F, 0x0F, 0x59, 0xD9, 0x73, 0xEB, 0x34, 0xE8, 0xF0, 0xE9, 0x72, 0x2D, 0x5B, 0x1B, 0x14,
   0x42, 0xE6, 0x99, 0xD3, 0x0F, 0x4F, 0xA0, 0x7C, 0x88, 0x02, 0x64, 0x01, 0x63, 0xB8, 0x6A, 0x90,
   0xB1, 0xC5, 0x82, 0x6F, 0x2D, 0x00, 0x88, 0x04, 0x36, 0xD1, 0xA8, 0x50, 0xC6, 0x2D, 0x28, 0x03,
   0xDA, 0x05, 0x4E, 0xE2, 0xC8, 0x4B, 0x1C, 0x7B, 0x0B, 0x0E, 0xA1, 0x69, 0xC8, 0x69, 0xAA, 0xFE,
   0xE2, 0x84, 0x29, 0x37, 0x62, 0xC9, 0x5F, 0x81, 0xE9, 0x60, 0x55, 0x17, 0xBD, 0x84, 0x61, 0x8E,
   0xD1, 0x1F, 0x74, 0xCA, 0x7C, 0x88, 0x02, 0x64, 0x01, 0x63, 0xB8, 0x6A, 0x90, 0x52, 0x67, 0xAC,
   0x8D, 0x91, 0xC0, 0x5D, 0x50, 0xCA, 0xE9, 0x99, 0x64, 0xDF, 0x82, 0xD1, 0xF6, 0x77, 0xD7, 0xCC,
   0xE6, 0xFE, 0x5E, 0xC2, 0x95, 0x22, 0xEE, 0xE1, 0xFA, 0x8A, 0x77, 0xFF, 0x60, 0x29, 0x49, 0x89,
   0x04, 0xAA, 0x8D, 0xE8, 0xB7, 0xF7, 0x5F, 0x97, 0x85, 0x41, 0x8B, 0x1E, 0x6D, 0xF0, 0x0F, 0x7C,
   0x88, 0x02, 0x64, 0x01, 0x63, 0xB8, 0x8A, 0xBD, 0x21, 0x05, 0xAB, 0x41, 0xEF, 0x0F, 0x85, 0x80,
   0xF1, 0xEB, 0x80, 0x71, 0x11, 0x4E, 0x70, 0xB6, 0x3B, 0x86, 0xEC, 0xEB, 0x13, 0x94, 0xF9, 0x32,
   0xA1, 0x7A, 0xBB, 0x37, 0x86, 0x46, 0x69, 0x82, 0x90, 0x35, 0x68, 0xC0, 0x0B, 0x88, 0xC6, 0x82,
   0x9D, 0x0C, 0xB0, 0x8E, 0xDC, 0xA8, 0xB1, 0x90, 0xFA, 0x14, 0x2F, 0x83, 0xBC, 0xCA, 0x7C, 0x08,
   0x01, 0xDC, 0x05, 0x01, 0x89, 0xBC, 0x08, 0x6A, 0x9B, 0x60, 0x47, 0x58, 0xAD, 0xB7, 0x58, 0x2F,
   0x8C, 0xE9, 0xF5, 0x0F, 0x44, 0x10, 0xF9, 0x4C, 0x43, 0x82, 0x55, 0x6F, 0xBE, 0x8E, 0xD9, 0x7E,
   0x92, 0xA4, 0x1B, 0x9F, 0x17, 0x88, 0x5F, 0x60, 0xFE, 0x4F,
};

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
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      CHECK(player.GetSoundMode3D() == SNDCFG_SND3D2CH);
      CHECK(player.GetBackglassMode() == SNDCFG_BG2CH);
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

   SUBCASE("mono and stereo streams open in backglass front left mode")
   {
      // The mixing node declares the stream channel count: the sound must output the same
      // number of channels for the node attachment to succeed (node attachments are strict)
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BGFRONTLEFT, ""s, SNDCFG_SND3D2CH);
      CHECK(player.GetBackglassMode() == SNDCFG_BGFRONTLEFT);

      AudioPlayer::AudioStreamID monoStream = player.OpenAudioStream("mono-stream", 44100, 1, true);
      CHECK(monoStream != nullptr);
      player.CloseAudioStream(monoStream, false);

      AudioPlayer::AudioStreamID stereoStream = player.OpenAudioStream("stereo-stream", 44100, 2, true);
      CHECK(stereoStream != nullptr);
      player.CloseAudioStream(stereoStream, false);
   }

   SUBCASE("mono and stereo streams open in spatial backglass mode")
   {
      // The spatial source node declares the stream channel count: the sound must output the
      // same number of channels for the node attachment to succeed (node attachments are strict)
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      player.SetSpatialMode(true, false);
      CHECK(player.IsSpatialAudioEnabled(SoundOutTypes::SNDOUT_BACKGLASS));

      AudioPlayer::AudioStreamID monoStream = player.OpenAudioStream("mono-stream", 44100, 1, true);
      CHECK(monoStream != nullptr);
      player.CloseAudioStream(monoStream, false);

      AudioPlayer::AudioStreamID stereoStream = player.OpenAudioStream("stereo-stream", 44100, 2, true);
      CHECK(stereoStream != nullptr);
      player.CloseAudioStream(stereoStream, false);
   }

   SUBCASE("streams outliving the audio player are safely detached")
   {
      // Plugin audio lanes keep streams alive through shared_ptr: recreating the audio player
      // (e.g. after an output mode change) must not leave them bound to a dead engine
      AudioPlayer::AudioStreamID stream;
      {
         AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
         stream = player.OpenAudioStream("kept-stream", 44100, 2, true);
         REQUIRE(stream != nullptr);
      } // player destroyed: the stream is detached from its engine
      CHECK(stream->IsFinished());
   }

   SUBCASE("streams in front left mode outliving the audio player are safely detached")
   {
      // Same scenario through the mixing node path
      AudioPlayer::AudioStreamID stream;
      {
         AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BGFRONTLEFT, ""s, SNDCFG_SND3D2CH);
         stream = player.OpenAudioStream("kept-stream", 44100, 1, true);
         REQUIRE(stream != nullptr);
      }
      CHECK(stream->IsFinished());
   }

   SUBCASE("closing an unknown stream keeps the opened streams")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      AudioPlayer::AudioStreamID stream = player.OpenAudioStream("test-stream", 44100, 2, true);
      AudioPlayer::AudioStreamID unknown;
      player.CloseAudioStream(unknown, false); // logs an error, must not remove the valid stream
      CHECK(player.IsOpened(stream));
   }

   SUBCASE("sound information decoding")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      VPX::Sound sound("TestSound", "test.wav", MakeSineWav(0.1f));

      const SoundSpec spec = player.GetSoundInformations(&sound);
      CHECK(spec.nChannels == 2);
      CHECK(spec.sampleFrequency == 44100);
      CHECK(spec.lengthInSeconds == doctest::Approx(0.1).epsilon(0.2));
   }

   SUBCASE("opus sound information decoding")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      VPX::Sound sound("TestSound", "test.opus", opusSine);
      const SoundSpec spec = player.GetSoundInformations(&sound);
      CHECK(spec.nChannels == 2);
      CHECK(spec.sampleFrequency == 48000);
      CHECK(spec.lengthInSeconds == doctest::Approx(0.1).epsilon(0.2));
   }

   SUBCASE("play and stop an opus sound")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      VPX::Sound sound("TestSound", "test.opus", opusSine);
      player.SetMainVolume(0.5f, 0.5f, 0.5f);
      player.PlaySound(&sound, 0.f, 0.f, 0, 0.f, 0.f, 1, false, true);
      player.StopSound(&sound);
   }

   SUBCASE("play and stop a sound")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      VPX::Sound sound("TestSound", "test.wav", MakeSineWav(0.5f));

      player.SetMainVolume(0.5f, 0.5f, 0.5f);
      player.PlaySound(&sound, 0.f, 0.f, 0, 0.f, 0.f, 1, false, true);
      player.StopSound(&sound);
   }

   SUBCASE("haptic output requires a configured device")
   {
      // No device configured: the haptic output is disabled (no default device fallback)
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);
      CHECK(!player.IsHapticEnabled());
      CHECK(player.GetHapticDeviceName().empty());

      VPX::Sound sound("TestSound", "test.wav", MakeSineWav(0.5f));
      player.PlaySound(&sound, 0.f, 0.f, 0, 0.f, 0.f, 1, false, true);
      player.StopSound(&sound);
   }

   SUBCASE("haptic output mirrors playfield sounds to its device")
   {
      const vector<AudioPlayer::AudioDevice> devices = AudioPlayer::EnumerateAudioDevices();
      REQUIRE(!devices.empty());

      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, devices.front().name, SNDCFG_SND3DFRONTISREAR);
      CHECK(player.IsHapticEnabled());
      CHECK(player.GetHapticDeviceName() == devices.front().name);
      CHECK(player.GetHapticMode() == SNDCFG_SND3DFRONTISREAR);

      // Mirroring is exercised on the playfield sound path (both engines run on the dummy driver)
      VPX::Sound sound("TestSound", "test.wav", MakeSineWav(0.5f));
      player.SetMainVolume(0.5f, 0.5f, 0.5f);
      player.PlaySound(&sound, 0.f, 0.f, 0, 0.f, 0.f, 1, false, true);
      player.StopSound(&sound);
   }

   SUBCASE("haptic output is disabled when its device is missing")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, "No such audio device"s, SNDCFG_SND3D2CH);
      CHECK(!player.IsHapticEnabled());
      CHECK(player.GetHapticDeviceName().empty());
   }

   SUBCASE("music playback commands")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);

      const std::filesystem::path musicFile = GetTestTmpDir() / "test-music.wav";
      write_file(musicFile, MakeSineWav(0.5f), true);

      CHECK(player.PlayMusic(musicFile));
      CHECK(player.GetMusicPosition() >= 0.f);
      player.PauseMusic();
      player.UnpauseMusic();
      player.SetMusicVolume(0.5f);
      player.SetMusicPosition(0.f);
      CHECK(player.GetMusicPosition() >= 0.f);

      std::error_code ec;
      std::filesystem::remove(musicFile, ec);
   }

   SUBCASE("opus music playback")
   {
      AudioPlayer player(""s, ""s, SNDCFG_SND3D2CH, SNDCFG_BG2CH, ""s, SNDCFG_SND3D2CH);

      const std::filesystem::path musicFile = GetTestTmpDir() / "test-music.opus";
      write_file(musicFile, opusSine, true);

      REQUIRE(player.PlayMusic(musicFile));
      // The music is opened on the player's thread: wait for it to be decoded and to start moving
      bool started = false;
      for (int i = 0; i < 200 && !started; ++i)
      {
         SDL_Delay(10);
         started = player.GetMusicPosition() > 0.f;
      }
      CHECK(started);

      std::error_code ec;
      std::filesystem::remove(musicFile, ec);
   }
}

TEST_CASE("Channel mixing")
{
   // Direct validation of the legacy (non spatial) channel mixing applied by the vpx_node custom node
   constexpr unsigned int nFrames = 4;
   const float monoIn[nFrames] = { 0.25f, -0.5f, 0.75f, -1.f };
   const float stereoIn[nFrames * 2] = { 0.4f, 0.2f, -0.6f, 0.8f, 1.f, -1.f, 0.f, 0.5f };

   SUBCASE("backglass stereo to 2 front channels")
   {
      float out[nFrames * 2];
      MixToDeviceChannels(SoundMixMode::Backglass, 0.5f, 0.f, 0.f, stereoIn, 2, out, 2, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(out[i * 2] == doctest::Approx(0.5f * stereoIn[i * 2]));
         CHECK(out[i * 2 + 1] == doctest::Approx(0.5f * stereoIn[i * 2 + 1]));
      }
   }

   SUBCASE("backglass stereo balanced pan")
   {
      // Pan attenuates the opposite channel but never boosts nor blends
      float out[nFrames * 2];
      MixToDeviceChannels(SoundMixMode::Backglass, 1.f, 0.5f, 0.f, stereoIn, 2, out, 2, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(out[i * 2] == doctest::Approx(0.5f * stereoIn[i * 2]));
         CHECK(out[i * 2 + 1] == doctest::Approx(stereoIn[i * 2 + 1]));
      }
   }

   SUBCASE("backglass front left only downmixes to a single channel")
   {
      float out[nFrames * 8];
      MixToDeviceChannels(SoundMixMode::BackglassFrontLeft, 0.5f, 0.f, 0.f, stereoIn, 2, out, 8, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(out[i * 8] == doctest::Approx(0.5f * 0.5f * (stereoIn[i * 2] + stereoIn[i * 2 + 1])));
         for (unsigned int ch = 1; ch < 8; ++ch)
            CHECK(out[i * 8 + ch] == 0.f);
      }
   }

   SUBCASE("backglass front left only on a stereo device keeps the right channel silent")
   {
      float out[nFrames * 2];
      MixToDeviceChannels(SoundMixMode::BackglassFrontLeft, 1.f, 0.f, 0.f, stereoIn, 2, out, 2, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(out[i * 2] == doctest::Approx(0.5f * (stereoIn[i * 2] + stereoIn[i * 2 + 1])));
         CHECK(out[i * 2 + 1] == 0.f);
      }
   }

   SUBCASE("backglass front left only passes a mono source through")
   {
      // Mono input happens for plugin audio streams
      float out[nFrames * 2];
      MixToDeviceChannels(SoundMixMode::BackglassFrontLeft, 0.5f, 0.f, 0.f, monoIn, 1, out, 2, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(out[i * 2] == doctest::Approx(0.5f * monoIn[i]));
         CHECK(out[i * 2 + 1] == 0.f);
      }
   }

   SUBCASE("backglass stereo downmix to a mono device")
   {
      float out[nFrames];
      MixToDeviceChannels(SoundMixMode::Backglass, 0.5f, 0.f, 0.f, stereoIn, 2, out, 1, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
         CHECK(out[i] == doctest::Approx(0.5f * 0.5f * (stereoIn[i * 2] + stereoIn[i * 2 + 1])));
   }

   SUBCASE("backglass stereo to a multichannel device only drives the front pair")
   {
      // Plugin audio streams rely on this: attached straight to the endpoint, miniaudio's channel
      // conversion would also map stereo FL/FR onto SL/SR, leaking on the playfield channels
      for (const unsigned int nOut : { 6u, 8u })
      {
         float out[nFrames * 8];
         MixToDeviceChannels(SoundMixMode::Backglass, 1.f, 0.f, 0.f, stereoIn, 2, out, nOut, nFrames);
         for (unsigned int i = 0; i < nFrames; ++i)
            for (unsigned int ch = 0; ch < nOut; ++ch)
               CHECK(out[i * nOut + ch] == doctest::Approx(ch < 2 ? stereoIn[i * 2 + ch] : 0.f));
      }
   }

   SUBCASE("playfield stereo pan to 2 front channels")
   {
      float out[nFrames * 2];
      MixToDeviceChannels(SoundMixMode::Playfield, 1.f, 0.f, 0.f, monoIn, 1, out, 2, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(out[i * 2] == doctest::Approx(0.5f * monoIn[i]));
         CHECK(out[i * 2 + 1] == doctest::Approx(0.5f * monoIn[i]));
      }
   }

   SUBCASE("playfield front right only on a stereo device")
   {
      float out[nFrames * 2];
      MixToDeviceChannels(SoundMixMode::PlayfieldFrontRight, 1.f, 0.f, 0.f, monoIn, 1, out, 2, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(out[i * 2] == 0.f);
         CHECK(out[i * 2 + 1] == doctest::Approx(monoIn[i]));
      }
   }

   SUBCASE("playfield front right only on a multichannel device")
   {
      float out[nFrames * 8];
      MixToDeviceChannels(SoundMixMode::PlayfieldFrontRight, 0.5f, 0.f, 0.f, monoIn, 1, out, 8, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
         for (unsigned int ch = 0; ch < 8; ++ch)
            CHECK(out[i * 8 + ch] == doctest::Approx(ch == 1 ? 0.5f * monoIn[i] : 0.f));
   }

   SUBCASE("backglass on front left and playfield on front right share a stereo device")
   {
      // The combined setup the two output modes are designed for: a single stereo
      // device with the backglass on its left channel and the playfield on its right one
      float bgOut[nFrames * 2], pfOut[nFrames * 2];
      MixToDeviceChannels(SoundMixMode::BackglassFrontLeft, 1.f, 0.f, 0.f, stereoIn, 2, bgOut, 2, nFrames);
      MixToDeviceChannels(SoundMixMode::PlayfieldFrontRight, 1.f, 0.f, 0.f, monoIn, 1, pfOut, 2, nFrames);
      for (unsigned int i = 0; i < nFrames; ++i)
      {
         CHECK(bgOut[i * 2] == doctest::Approx(0.5f * (stereoIn[i * 2] + stereoIn[i * 2 + 1])));
         CHECK(bgOut[i * 2 + 1] == 0.f);
         CHECK(pfOut[i * 2] == 0.f);
         CHECK(pfOut[i * 2 + 1] == doctest::Approx(monoIn[i]));
      }
   }
}
