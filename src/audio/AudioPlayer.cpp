// license:GPLv3+

#include "core/stdafx.h"
#include "AudioPlayer.h"
#include "AudioStreamPlayer.h"
#include "SoundPlayer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS 1
#define MA_ENABLE_CUSTOM 1
#include "miniaudio/extras/stb_vorbis.c"
#include "miniaudio/miniaudio.h"
#include "miniaudio/miniaudio.c"

namespace VPX
{

// Simple SDL3 backend for miniaudio, derived from miniaudio's backend example

SDL_AudioFormat ma_format_to_sdl(ma_format format)
{
   switch (format)
   {
   case ma_format_unknown: return SDL_AudioFormat::SDL_AUDIO_UNKNOWN;
   case ma_format_u8: return SDL_AudioFormat::SDL_AUDIO_U8;
   case ma_format_s16: return SDL_AudioFormat::SDL_AUDIO_S16;
   case ma_format_s24: return SDL_AudioFormat::SDL_AUDIO_S32; /* Closest match. */
   case ma_format_s32: return SDL_AudioFormat::SDL_AUDIO_S32;
   case ma_format_f32: return SDL_AudioFormat::SDL_AUDIO_F32;
   default: return SDL_AudioFormat::SDL_AUDIO_UNKNOWN;
   }
}

ma_format ma_format_from_sdl(SDL_AudioFormat format)
{
   switch (format)
   {
   case SDL_AudioFormat::SDL_AUDIO_U8: return ma_format_u8;
   case SDL_AudioFormat::SDL_AUDIO_S16: return ma_format_s16;
   case SDL_AudioFormat::SDL_AUDIO_S32: return ma_format_s32;
   case SDL_AudioFormat::SDL_AUDIO_F32: return ma_format_f32;
   default: return ma_format_unknown;
   }
}

static ma_result ma_context_enumerate_devices__sdl(ma_context* pContext, ma_enum_devices_callback_proc callback, void* pUserData)
{
   ma_device_info deviceInfo; // Single virtual device for VPX that maps to VPX config outputs on SDL devices
   MA_ZERO_OBJECT(&deviceInfo);
   static const char* name = "VPX Audio Device";
   ma_strncpy_s(deviceInfo.name, sizeof(deviceInfo.name), name, (size_t)-1);
   deviceInfo.isDefault = MA_TRUE;
   callback(pContext, ma_device_type_playback, &deviceInfo, pUserData);
   return MA_SUCCESS;
}

static ma_result ma_context_get_device_info__sdl(ma_context* pContext, ma_device_type deviceType, const ma_device_id* pDeviceID, ma_device_info* pDeviceInfo)
{
   static const char* name = "VPX Audio Device"; // Single virtual device for VPX that maps to VPX config outputs on SDL devices
   ma_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), name, (size_t)-1);
   pDeviceInfo->isDefault = MA_TRUE;

   AudioPlayer* player = static_cast<AudioPlayer*>(pContext->pUserData);

   SDL_AudioSpec desiredSpec;
   SDL_AudioDeviceID tempDeviceID = SDL_OpenAudioDevice(player->m_tableAudioDevice, &desiredSpec);
   if (tempDeviceID == 0)
   {
      ma_log_postf(ma_context_get_log(pContext), MA_LOG_LEVEL_ERROR, "Failed to open SDL device.");
      return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
   }
   SDL_AudioSpec obtainedSpec;
   SDL_GetAudioDeviceFormat(tempDeviceID, &obtainedSpec, nullptr);
   SDL_CloseAudioDevice(tempDeviceID);

   // Only reporting a single native data format. It'll be whatever SDL decides is the best.
   pDeviceInfo->nativeDataFormatCount = 1;
   pDeviceInfo->nativeDataFormats[0].format = ma_format_from_sdl(obtainedSpec.format);
   pDeviceInfo->nativeDataFormats[0].channels = obtainedSpec.channels;
   pDeviceInfo->nativeDataFormats[0].sampleRate = obtainedSpec.freq;
   pDeviceInfo->nativeDataFormats[0].flags = 0;

   // If miniaudio does not support the format, just use f32 as the native format (SDL will do the necessary conversions for us).
   if (pDeviceInfo->nativeDataFormats[0].format == ma_format_unknown)
      pDeviceInfo->nativeDataFormats[0].format = ma_format_f32;

   return MA_SUCCESS;
}

void ma_audio_callback_playback__sdl(void* pUserData, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
   ma_device* pDevice = static_cast<ma_device*>(pUserData);
   AudioPlayer* player = static_cast<AudioPlayer*>(pDevice->pContext->pUserData);
   if (player->m_streamBuffer.size() < total_amount)
      player->m_streamBuffer.resize(total_amount);
   const int sizePerFrame = ma_get_bytes_per_frame(pDevice->playback.internalFormat, pDevice->playback.internalChannels);
   const int nFrames = total_amount / sizePerFrame;
   ma_device_handle_backend_data_callback(pDevice, player->m_streamBuffer.data(), nullptr, nFrames);
   SDL_PutAudioStreamData(stream, player->m_streamBuffer.data(), nFrames * sizePerFrame);
}

static ma_result ma_device_init__sdl(ma_device* pDevice, const ma_device_config* pConfig, ma_device_descriptor* pDescriptorPlayback, ma_device_descriptor* pDescriptorCapture)
{
   if (pConfig->deviceType != ma_device_type_playback)
      return MA_DEVICE_TYPE_NOT_SUPPORTED;

   AudioPlayer* player = static_cast<AudioPlayer*>(pDevice->pContext->pUserData);
   
   SDL_AudioSpec desiredSpec;
   int deviceId = SDL_OpenAudioDevice(player->m_tableAudioDevice, nullptr);
   if (deviceId == 0)
      return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
   SDL_GetAudioDeviceFormat(deviceId, &desiredSpec, nullptr);
   SDL_CloseAudioDevice(deviceId);

   if (pDescriptorPlayback->sampleRate)
      desiredSpec.freq = pDescriptorPlayback->sampleRate;
   if (ma_format_to_sdl(pDescriptorPlayback->format) != 0)
      desiredSpec.format = ma_format_to_sdl(pDescriptorPlayback->format);
   if (pDescriptorPlayback->channels)
      desiredSpec.channels = pDescriptorPlayback->channels;

   player->m_outStream = SDL_OpenAudioDeviceStream(player->m_tableAudioDevice, &desiredSpec, ma_audio_callback_playback__sdl, pDevice);
   if (player->m_outStream == nullptr)
   {
      const char* error = SDL_GetError();
      ma_log_postf(ma_device_get_log((ma_device*)pDevice), MA_LOG_LEVEL_ERROR, "Failed to open SDL device.");
      return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
   }

   /* The descriptor needs to be updated with our actual settings. */
   SDL_AudioSpec obtainedSpec;
   int periodSizeInFrames;
   deviceId = SDL_GetAudioStreamDevice(player->m_outStream);
   SDL_GetAudioDeviceFormat(deviceId, &obtainedSpec, &periodSizeInFrames);
   pDescriptorPlayback->format = ma_format_from_sdl(obtainedSpec.format);
   pDescriptorPlayback->channels = obtainedSpec.channels;
   pDescriptorPlayback->sampleRate = (ma_uint32)obtainedSpec.freq;
   ma_channel_map_init_standard(ma_standard_channel_map_default, pDescriptorPlayback->channelMap, ma_countof(pDescriptorPlayback->channelMap), pDescriptorPlayback->channels);
   pDescriptorPlayback->periodSizeInFrames = periodSizeInFrames;
   pDescriptorPlayback->periodCount = 1; /* SDL doesn't use the notion of period counts, so just set to 1. */

   return MA_SUCCESS;
}

static ma_result ma_device_uninit__sdl(ma_device* pDevice)
{
   AudioPlayer* player = static_cast<AudioPlayer*>(pDevice->pContext->pUserData);
   SDL_DestroyAudioStream(player->m_outStream);
   player->m_outStream = nullptr;
   return MA_SUCCESS;
}

static ma_result ma_device_start__sdl(ma_device* pDevice)
{
   AudioPlayer* player = static_cast<AudioPlayer*>(pDevice->pContext->pUserData);
   SDL_ResumeAudioStreamDevice(player->m_outStream);
   return MA_SUCCESS;
}

static ma_result ma_device_stop__sdl(ma_device* pDevice)
{
   AudioPlayer* player = static_cast<AudioPlayer*>(pDevice->pContext->pUserData);
   SDL_PauseAudioStreamDevice(player->m_outStream);
   return MA_SUCCESS;
}

static ma_result ma_context_uninit__sdl(ma_context* pContext)
{
   SDL_QuitSubSystem(SDL_INIT_AUDIO);
   return MA_SUCCESS;
}

static ma_result ma_context_init__sdl(ma_context* pContext, const ma_context_config* pConfig, ma_backend_callbacks* pCallbacks)
{
   (void)pConfig;

   if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
      return MA_ERROR;

   pCallbacks->onContextInit = ma_context_init__sdl;
   pCallbacks->onContextUninit = ma_context_uninit__sdl;
   pCallbacks->onContextEnumerateDevices = ma_context_enumerate_devices__sdl;
   pCallbacks->onContextGetDeviceInfo = ma_context_get_device_info__sdl;
   pCallbacks->onDeviceInit = ma_device_init__sdl;
   pCallbacks->onDeviceUninit = ma_device_uninit__sdl;
   pCallbacks->onDeviceStart = ma_device_start__sdl;
   pCallbacks->onDeviceStop = ma_device_stop__sdl;

   return MA_SUCCESS;
}



AudioPlayer::AudioPlayer(const Settings& settings)
   : m_music(nullptr)
{
   if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
   {
      PLOGE << "Failed to initialize SDL Audio: " << SDL_GetError();
      return;
   }

   string soundDeviceName;
   string soundDeviceBGName;
   const bool good = settings.LoadValue(Settings::Player, "SoundDevice"s, soundDeviceName);
   const bool good2 = settings.LoadValue(Settings::Player, "SoundDeviceBG"s, soundDeviceBGName);
   if (!good && !good2)
   { // use the default SDL audio device
      PLOGI << "Sound Device not set.  Using default";
      m_tableAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
      m_backglassAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
   }
   else
   { // this is all because the device id's are random: https://github.com/libsdl-org/SDL/issues/12278
      vector<AudioDevice> allAudioDevices;
      AudioPlayer::EnumerateAudioDevices(allAudioDevices);
      for (size_t i = 0; i < allAudioDevices.size(); ++i)
      {
         const AudioDevice& audioDevice = allAudioDevices[i];
         if (audioDevice.name == soundDeviceName)
            m_tableAudioDevice = audioDevice.id;
         if (audioDevice.name == soundDeviceBGName)
            m_backglassAudioDevice = audioDevice.id;
      }

      if (m_tableAudioDevice == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK)
      { // we didn't find a matching name
         PLOGE << "No sound device by that name found in VPinball.ini. " << "SoundDevice:\"" << soundDeviceName << "\" SoundDeviceBG:\"" << soundDeviceBGName << "\" Using default.";
         m_tableAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
         m_backglassAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
      }
   }

   // Initialize the SDL mixer library on the playfield audio device
   SDL_AudioSpec spec;
   SDL_GetAudioDeviceFormat(m_tableAudioDevice, &spec, nullptr);
   SDL_AudioSpec* reqSpec = ((spec.format != SDL_AUDIO_S16LE) && (spec.format != SDL_AUDIO_F32LE) && (spec.format != SDL_AUDIO_S32LE)) ? &spec : nullptr;
   spec.format = SDL_AUDIO_F32LE; // If we don't support device's native format, request one we support

   m_soundMode3D = static_cast<SoundConfigTypes>(settings.LoadValueUInt(Settings::Player, "Sound3D"s));
   if ((m_soundMode3D == SNDCFG_SND3DALLREAR) && (m_audioSpecOutput.channels < 4))
   {
      PLOGE << "Your sound device does not have the required number of channels (4+) to support this mode. <SND3DALLREAR>";
      m_soundMode3D = SNDCFG_SND3D2CH;
   }
   else if (((m_soundMode3D == SNDCFG_SND3D6CH) || (m_soundMode3D == SNDCFG_SND3DSSF)) && (m_audioSpecOutput.channels != 8))
   {
      PLOGE << "Your sound device does not have the required number of channels (8) to support this mode. <SNDCFG_SND3D6CH/SNDCFG_SND3DSSF>";
      m_soundMode3D = SNDCFG_SND3D2CH;
   }
   else if (m_soundMode3D == SNDCFG_SND3DFRONTISREAR)
   {
      PLOGI << "Sound Mode SNDCFG_SND3DFRONTISREAR not implemented yet."; return;
      m_soundMode3D = SNDCFG_SND3D2CH;
   }
   else if (m_soundMode3D == SNDCFG_SND3DFRONTISFRONT)
   {
      PLOGI << "Sound Mode SNDCFG_SND3DFRONTISFRONT not implemented yet.";
      m_soundMode3D = SNDCFG_SND3D2CH;
   }

   const char* pdriverName = SDL_GetCurrentAudioDriver();
   PLOGI << "Output Device Settings: " << "Freq: " << m_audioSpecOutput.freq << " Format (SDL_AudioFormat): " << m_audioSpecOutput.format
         << " channels: " << m_audioSpecOutput.channels << ", driver: " << (pdriverName ? pdriverName : "NULL") ;

   ma_context_config contextConfig;
   contextConfig = ma_context_config_init();
   contextConfig.custom.onContextInit = ma_context_init__sdl;

   m_maContext = std::make_unique<ma_context>();
   ma_backend backends[] = { ma_backend_custom };
   ma_context_init(backends, sizeof(backends) / sizeof(backends[0]), &contextConfig, m_maContext.get());
   m_maContext->pUserData = this;

   ma_engine_config config;
   config = ma_engine_config_init();
   config.pContext = m_maContext.get();

   ma_result result;
   m_maEngine = std::make_unique<ma_engine>();
   result = ma_engine_init(&config, m_maEngine.get());
   if (result != MA_SUCCESS)
   {
      PLOGE << "Failed to initialize miniaudio";
   }
}

AudioPlayer::~AudioPlayer()
{
   m_soundPlayers.clear();
   m_audioStreams.clear();
   m_music = nullptr;
   ma_engine_uninit(m_maEngine.get());
   ma_context_uninit(m_maContext.get());
   SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioPlayer::SetMainVolume(float backglassVolume, float playfieldVolume)
{
   m_backglassVolume = backglassVolume;
   m_playfieldVolume = playfieldVolume;
   if (m_music)
      m_music->SetMainVolume(backglassVolume, playfieldVolume);
   for (auto& players : m_soundPlayers)
      for (auto& player : players.second)
         player->SetMainVolume(backglassVolume, playfieldVolume);
   for (auto& player : m_audioStreams)
      player->SetMainVolume(backglassVolume);
}

AudioPlayer::AudioStreamID AudioPlayer::OpenAudioStream(int frequency, int channels)
{
   AudioStreamPlayer* audioStream = AudioStreamPlayer::Create(m_backglassAudioDevice, frequency, channels);
   if (audioStream)
   {
      audioStream->SetMainVolume(m_backglassVolume);
      m_audioStreams.push_back(std::unique_ptr<AudioStreamPlayer>(audioStream));
   }
   return audioStream;
}

void AudioPlayer::EnqueueStream(AudioStreamID stream, void* buffer, int length)
{
   static_cast<AudioStreamPlayer*>(stream)->Enqueue(buffer, length);
}

int AudioPlayer::GetStreamQueueSize(AudioStreamID stream) const
{
   return static_cast<AudioStreamPlayer*>(stream)->GetQueued();
}

void AudioPlayer::SetStreamVolume(AudioStreamID stream, const float volume)
{
   static_cast<AudioStreamPlayer*>(stream)->SetStreamVolume(volume);
}

void AudioPlayer::CloseAudioStream(AudioStreamID stream)
{
   auto item = std::ranges::find_if(m_audioStreams, [stream](std::unique_ptr<AudioStreamPlayer>& player) { return player.get() == stream; });
   if (item != m_audioStreams.end())
   {
      m_audioStreams.erase(item);
   }
   else
   {
      PLOGE << "AudioStream not found in AudioPlayer::CloseAudioStream()";
   }
}

bool AudioPlayer::PlayMusic(const string& filename)
{
   m_music = std::unique_ptr<SoundPlayer>(SoundPlayer::Create(this, filename));
   if (m_music)
   {
      m_music->SetVolume(m_musicVolume);
      m_music->SetMainVolume(m_backglassVolume, m_playfieldVolume);
   }
   return m_music != nullptr;
}

void AudioPlayer::PauseMusic()
{
   if (m_music) m_music->Pause();
}

void AudioPlayer::UnpauseMusic()
{
   if (m_music) m_music->Unpause();
}

float AudioPlayer::GetMusicPosition() const
{
   return m_music ? m_music->GetPosition() : 0.f;
}
   
void AudioPlayer::SetMusicPosition(float seconds)
{
   if (m_music) m_music->SetPosition(seconds);
}

void AudioPlayer::SetMusicVolume(const float volume)
{
   m_musicVolume = volume;
   if (m_music) m_music->SetVolume(volume);
}

bool AudioPlayer::IsMusicPlaying() const
{
   return m_music && m_music->IsPlaying();
}

void AudioPlayer::PlaySound(Sound* sound, float volumeOffset, const float randomPitch, const int pitch, float panOffset, float frontRearFadeOffset, const int loopcount, const bool useSame, const bool restart)
{
   SoundPlayer* player = nullptr;
   vector<std::unique_ptr<SoundPlayer>>& players = m_soundPlayers[sound];

   // Until 10.8, implementation would:
   // - for some reason, 'usesame' would only be processed for wav file:
   //   - if 'usesame' is true, search for the first player for the given sound and reuse it if any (even is it is playing), create a new one otherwise
   //   - if 'usesame' is false, always create a new player for the given sound
   // - if restart is false and selected sound player was already playing, settings would be applied without restarting the sound
   for (auto& soundPlayer : players)
   {
      if (useSame || !soundPlayer->IsPlaying())
      {
         player = soundPlayer.get();
         break;
      }
   }

   if (player == nullptr)
   {
      player = SoundPlayer::Create(this, sound);
      if (player == nullptr)
         return;
      player->SetMainVolume(m_backglassVolume, m_playfieldVolume);
      players.push_back(std::unique_ptr<SoundPlayer>(player));
   }

   float pan = dequantizeSignedPercent(sound->GetPan()) + panOffset;

   if (restart)
      player->Stop();
   player->Play(
      dequantizeSignedPercent(sound->GetVolume()) + volumeOffset,
      randomPitch,
      pitch,
      m_mirrored ? -pan : pan,
      dequantizeSignedPercent(sound->GetFrontRearFade()) + frontRearFadeOffset,
      loopcount);
}

void AudioPlayer::StopSound(Sound* sound)
{
   vector<std::unique_ptr<SoundPlayer>>& players = m_soundPlayers[sound];
   for (auto& player : players)
      player->Stop();
}

SoundSpec AudioPlayer::GetSoundInformations(Sound* sound) const
{
   vector<std::unique_ptr<SoundPlayer>>& players = m_soundPlayers[sound];
   if (players.empty())
   {
      SoundPlayer* player = SoundPlayer::Create(this, sound);
      if (player == nullptr)
         return { 0 };
      player->SetMainVolume(m_backglassVolume, m_playfieldVolume);
      players.push_back(std::unique_ptr<SoundPlayer>(player));
   }
   return players.back()->GetInformations();
}

/**
 * (Static)
 * @brief Enumerates available audio playback devices and stores their details.
 *
 * This function initializes the SDL audio subsystem and retrieves a list of available
 * audio playback devices. It populates the provided vector with information about each device,
 * including its ID, name, and channel count.
 *
 * @param[out] audioDevices A vector to store the discovered audio playback devices.
 *
 * @note This function clears the provided vector before populating it with device information.
 * @note SDL must be properly initialized before calling this function.
 */ 
void AudioPlayer::EnumerateAudioDevices(vector<AudioDevice>& audioDevices)
{
   if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
      PLOGE << "SDL Init Audio failed: " << SDL_GetError();
      return;
   }

   //output name of audio driver
   const char *pdriverName;
   if ((pdriverName = SDL_GetCurrentAudioDriver()) != nullptr) {
      PLOGI << "Current Audio Driver: " << pdriverName;
   }
   else {
      PLOGE << "SDL Get Audio Driver failed: " << SDL_GetError();
   }

   // log default audio device
   SDL_AudioDeviceID devid = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
   const char *pDefaultDeviceName = SDL_GetAudioDeviceName(devid);
   if (pDefaultDeviceName) {
      PLOGI << "Default Audio Device: " << pDefaultDeviceName;
   }
   else {
      const char* pError = SDL_GetError();
      // workaround for https://github.com/libsdl-org/SDL/issues/12977
      if (pError && pError[0] != '\0') {
         PLOGE << "Failed to get name for Default Audio Device: " << pError;
      }
   }
   SDL_CloseAudioDevice(devid);

   audioDevices.clear();
   int count;
   SDL_AudioDeviceID* pAudioList = SDL_GetAudioPlaybackDevices(&count);
   for (int i = 0; i < count; ++i) {
      AudioDevice audioDevice = {};
      audioDevice.id = pAudioList[i];
      audioDevice.name = SDL_GetAudioDeviceName(pAudioList[i]);
      SDL_AudioSpec spec;
      SDL_GetAudioDeviceFormat(pAudioList[i], &spec, nullptr);
      audioDevice.channels = spec.channels;
      SDL_CloseAudioDevice(pAudioList[i]);
      audioDevices.push_back(audioDevice);
   }

   SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

}