// license:GPLv3+

#include "PUPSound.h"
#include "PUPManager.h"
#include "LibAv.h"

#include <SDL3/SDL_audio.h>

#include <cmath>

namespace PUP {

PUPSoundBank::PUPSoundBank(PUPManager* manager)
   : m_pManager(manager)
{
}

PUPSoundBank::~PUPSoundBank()
{
   Unload();
}

void PUPSoundBank::Load(const std::filesystem::path& packPath)
{
   Unload();
   const std::filesystem::path folder = find_case_insensitive_directory_path(packPath / "PUPSOUNDS"sv);
   if (folder.empty() || !std::filesystem::is_directory(folder))
      return;

   for (const auto& entry : std::filesystem::directory_iterator(folder))
   {
      if (!entry.is_regular_file())
         continue;
      const string name = lowerCase(PluginStrings::PathToUTF8(entry.path().filename()));
      if (!name.ends_with(".wav"))
         continue;

      SDL_AudioSpec srcSpec;
      Uint8* srcData = nullptr;
      Uint32 srcLen = 0;
      if (!SDL_LoadWAV(PluginStrings::PathToUTF8(entry.path()).c_str(), &srcSpec, &srcData, &srcLen))
      {
         LOGE(std::format("Unable to load sound: file={}, error={}", PluginStrings::PathToUTF8(entry.path()), SDL_GetError()));
         continue;
      }
      const SDL_AudioSpec dstSpec { SDL_AUDIO_F32, 2, srcSpec.freq };
      Uint8* dstData = nullptr;
      int dstLen = 0;
      const bool converted = SDL_ConvertAudioSamples(&srcSpec, srcData, static_cast<int>(srcLen), &dstSpec, &dstData, &dstLen);
      SDL_free(srcData);
      if (!converted)
      {
         LOGE(std::format("Unable to convert sound: file={}, error={}", PluginStrings::PathToUTF8(entry.path()), SDL_GetError()));
         continue;
      }
      auto sample = std::make_shared<Sample>();
      sample->sampleRate = srcSpec.freq;
      sample->frames = dstLen / static_cast<int>(2 * sizeof(float));
      sample->samples.assign(reinterpret_cast<const float*>(dstData), reinterpret_cast<const float*>(dstData) + static_cast<size_t>(sample->frames) * 2);
      SDL_free(dstData);
      m_samples[name] = sample;

      if (name.size() == 13 && name.starts_with("ballroll") && isdigit(static_cast<unsigned char>(name[8])))
         m_ballRoll[name[8] - '0'].sample = sample;
   }
   LOGI(std::format("Sound bank loaded: folder={}, sounds={}", PluginStrings::PathToUTF8(folder), m_samples.size()));
}

void PUPSoundBank::Unload()
{
   for (BallRoll& slot : m_ballRoll)
   {
      if (slot.playing)
         StopAudioStream(slot.streamId);
      slot = BallRoll {};
   }
   m_samples.clear();
}

bool PUPSoundBank::Queue(uint32_t streamId, const Sample& sample, int volume, int x)
{
   const LibAV::LibAV& libAv = LibAV::LibAV::GetInstance();
   const float gain = std::clamp(static_cast<float>(volume) / 100.f, 0.f, 2.f);
   if (!libAv.isLoaded || sample.frames == 0 || gain <= 0.f)
      return false;

   // Listener at the origin, x runs -10 (left) .. 10 (right): constant power pan
   const float pan = std::clamp(static_cast<float>(x) / 10.f, -1.f, 1.f);
   const float angle = (pan + 1.f) * 0.78539816f;
   const float gainL = cosf(angle);
   const float gainR = sinf(angle);

   const size_t bufferSize = sample.samples.size() * sizeof(float);
   float* const buffer = static_cast<float*>(libAv._av_malloc(bufferSize));
   if (!buffer)
      return false;
   for (int i = 0; i < sample.frames; i++)
   {
      buffer[2 * i] = sample.samples[2 * i] * gainL;
      buffer[2 * i + 1] = sample.samples[2 * i + 1] * gainR;
   }

   AudioUpdateMsg* msg = new AudioUpdateMsg();
   msg->streamId.resId = streamId;
   msg->channelFormat = CTLPI_AUDIO_FORMAT_CHANNEL_STEREO;
   msg->sampleFormat = CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT;
   msg->sampleRate = sample.sampleRate;
   msg->bufferSize = static_cast<unsigned int>(bufferSize);
   msg->buffer = reinterpret_cast<uint8_t*>(buffer);
   msg->volume = gain;
   UpdateAudioStream(msg);
   return true;
}

void PUPSoundBank::LoopBallRoll(int slot, uint64_t generation)
{
   BallRoll& roll = m_ballRoll[slot];
   if (!roll.playing || roll.generation != generation || !roll.sample)
      return;
   Queue(roll.streamId, *roll.sample, roll.volume, roll.x);
   const double duration = static_cast<double>(roll.sample->frames) / static_cast<double>(roll.sample->sampleRate);
   m_pManager->RunLater(duration, [this, slot, generation]() { LoopBallRoll(slot, generation); });
}

void PUPSoundBank::Play(const string& name, int x, int y, int z, int vol, const string& special)
{
   const string key = lowerCase(name);

   if (key.size() >= 9 && key.starts_with("ballroll") && isdigit(static_cast<unsigned char>(key[8])) && m_ballRoll[key[8] - '0'].sample)
   {
      BallRoll& roll = m_ballRoll[key[8] - '0'];
      roll.volume = vol;
      roll.x = x;
      const int speed = string_to_int(special, 0);
      if (speed == 0)
      {
         if (roll.playing)
         {
            StopAudioStream(roll.streamId);
            roll.playing = false;
            roll.generation++;
         }
      }
      else if (!roll.playing)
      {
         roll.playing = true;
         roll.generation++;
         roll.streamId = m_nextStreamId++;
         LoopBallRoll(key[8] - '0', roll.generation);
      }
      return;
   }

   auto it = m_samples.find(key);
   if (it == m_samples.end())
      return;
   const std::shared_ptr<Sample> sample = it->second;
   const uint32_t streamId = m_nextStreamId++;
   if (!Queue(streamId, *sample, vol, x))
      return;
   const double duration = static_cast<double>(sample->frames) / static_cast<double>(sample->sampleRate);
   m_pManager->RunLater(duration + 0.25, [streamId]() { StopAudioStream(streamId); });
}

}
