// license:GPLv3+

#include "PUPSound.h"
#include "PUPManager.h"
#include "LibAv.h"

#include <SDL3/SDL_audio.h>

#include <algorithm>
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

      if (name.starts_with("ballroll") && name.size() > 12)
      {
         const string digits = name.substr(8, name.size() - 12);
         if (std::ranges::all_of(digits, [](char c) { return isdigit(static_cast<unsigned char>(c)) != 0; }))
         {
            const int slot = string_to_int(digits, 0);
            if (slot < 29)
               m_ballRoll[slot].sample = sample;
         }
      }
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
   m_lastStream.clear();
}

bool PUPSoundBank::Queue(uint32_t streamId, const Sample& sample, int volume, int x, float rate)
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
   msg->sampleRate = static_cast<int>(std::lround(static_cast<float>(sample.sampleRate) * rate));
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
   Queue(roll.streamId, *roll.sample, roll.volume, roll.x, roll.rate);
   const double duration = static_cast<double>(roll.sample->frames) / (static_cast<double>(roll.sample->sampleRate) * roll.rate);
   m_pManager->RunLater(duration, [this, slot, generation]() { LoopBallRoll(slot, generation); });
}

void PUPSoundBank::PlayBallRoll(int slot, int x, int vol, double speed, bool pinup20)
{
   BallRoll& roll = m_ballRoll[slot];
   roll.volume = vol;
   roll.x = x;
   if (pinup20)
      roll.rate = static_cast<float>((std::abs(speed) / 100.0 * 0.75 + 0.25) * 44100.0 / roll.sample->sampleRate);
   if (speed == 0.0)
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
      LoopBallRoll(slot, roll.generation);
   }
}

void PUPSoundBank::Play(const string& name, int x, int y, int z, int vol, const string& special)
{
   const string key = lowerCase(name);
   const bool pinup20 = m_pManager->GetEngine() == PUPManager::Engine::PinUP20;

   if (pinup20)
   {
      if (std::abs(x) > 19)
         x /= 10;
      vol = std::clamp(vol * m_pManager->GetSSFVolume() / 100, 0, 100);
      if (key.starts_with("ballroll"))
      {
         string digits = key.substr(8);
         if (digits.ends_with(".wav"))
            digits.resize(digits.size() - 4);
         const int slot = string_to_int(digits, 0);
         if (slot >= 0 && slot < 30 && m_ballRoll[slot].sample)
            PlayBallRoll(slot, x, vol, std::atof(special.c_str()), true);
         return;
      }
   }
   else if (key.size() >= 9 && key.starts_with("ballroll") && isdigit(static_cast<unsigned char>(key[8])) && m_ballRoll[key[8] - '0'].sample)
   {
      PlayBallRoll(key[8] - '0', x, vol, string_to_int(special, 0), false);
      return;
   }

   auto it = m_samples.find(key);
   if (it == m_samples.end())
      return;
   if (pinup20)
   {
      if (const auto last = m_lastStream.find(key); last != m_lastStream.end())
         StopAudioStream(last->second);
   }
   const std::shared_ptr<Sample> sample = it->second;
   const uint32_t streamId = m_nextStreamId++;
   if (!Queue(streamId, *sample, vol, x))
      return;
   m_lastStream[key] = streamId;
   const double duration = static_cast<double>(sample->frames) / static_cast<double>(sample->sampleRate);
   m_pManager->RunLater(duration + 0.25, [streamId]() { StopAudioStream(streamId); });
}

}
