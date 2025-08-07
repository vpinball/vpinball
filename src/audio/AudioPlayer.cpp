// license:GPLv3+

#include "core/stdafx.h"
#include "AudioPlayer.h"
#include "AudioStreamPlayer.h"
#include "SoundPlayer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#include "miniaudio/extras/stb_vorbis.c"
#include "miniaudio/miniaudio.h"
#include "miniaudio/miniaudio.c"

// Simple SDL3 backend for miniaudio, derived from miniaudio's backend example

struct ma_device_ex
{
   ma_device device; // Make this the first member so we can cast between ma_device and ma_device_ex.
   SDL_AudioDeviceID deviceID;
   SDL_AudioStream* stream;
   vector<uint8_t> buffer;
};

static ma_result ma_context_enumerate_devices__sdl(ma_context* pContext, ma_enum_devices_callback_proc callback, void* pUserData)
{
   int count;
   auto pAudioList = SDL_GetAudioPlaybackDevices(&count);
   if (pAudioList == nullptr)
      return MA_ERROR;
   for (int i = 0; i < count; ++i)
   {
      ma_device_info deviceInfo;
      MA_ZERO_OBJECT(&deviceInfo);
      deviceInfo.id.custom.i = pAudioList[i];
      ma_strncpy_s(deviceInfo.name, sizeof(deviceInfo.name), SDL_GetAudioDeviceName(pAudioList[i]), (size_t)-1);
      ma_bool32 cbResult = callback(pContext, ma_device_type_playback, &deviceInfo, pUserData);
      if (cbResult == MA_FALSE)
         break;
   }
   SDL_free(pAudioList);
   return MA_SUCCESS;
}

static ma_result ma_context_get_device_info__sdl(ma_context* pContext, ma_device_type deviceType, const ma_device_id* pDeviceID, ma_device_info* pDeviceInfo)
{
   if (deviceType != ma_device_type_playback)
      return MA_DEVICE_TYPE_NOT_SUPPORTED;

   if (pDeviceID == nullptr)
   {
      pDeviceInfo->id.custom.i = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
      ma_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), MA_DEFAULT_PLAYBACK_DEVICE_NAME, (size_t)-1);
   }
   else
   {
      pDeviceInfo->id.custom.i = pDeviceID->custom.i;
      ma_strncpy_s(pDeviceInfo->name, sizeof(pDeviceInfo->name), SDL_GetAudioDeviceName(pDeviceID->custom.i), (size_t)-1);
   }
   if (pDeviceInfo->id.custom.i == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK)
      pDeviceInfo->isDefault = MA_TRUE;

   SDL_AudioSpec specs;
   if (pDeviceInfo->isDefault)
   {
      SDL_AudioDeviceID tempDeviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
      if (tempDeviceID == 0)
      {
         PLOGE << "Failed to open default SDL device.";
         return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
      }
      SDL_GetAudioDeviceFormat(tempDeviceID, &specs, nullptr);
      SDL_CloseAudioDevice(tempDeviceID);
   }
   else
   {
      SDL_GetAudioDeviceFormat(pDeviceInfo->id.custom.i, &specs, nullptr);
   }

   pDeviceInfo->nativeDataFormatCount = 1;
   pDeviceInfo->nativeDataFormats[0].format = ma_format_f32;
   pDeviceInfo->nativeDataFormats[0].channels = specs.channels;
   pDeviceInfo->nativeDataFormats[0].sampleRate = specs.freq;
   pDeviceInfo->nativeDataFormats[0].flags = 0;

   return MA_SUCCESS;
}

void ma_audio_callback_playback__sdl(void* pUserData, SDL_AudioStream* stream, int additional_amount, const int total_amount)
{
   auto pDevice = static_cast<ma_device_ex*>(pUserData);
   if ((int)pDevice->buffer.size() < total_amount)
      pDevice->buffer.resize(total_amount);
   const int sizePerMAFrame = ma_get_bytes_per_frame(pDevice->device.playback.internalFormat, pDevice->device.playback.internalChannels);
   const int nFrames = total_amount / sizePerMAFrame;
   ma_device__read_frames_from_client(&pDevice->device, nFrames, pDevice->buffer.data());
   SDL_PutAudioStreamData(stream, pDevice->buffer.data(), nFrames * sizePerMAFrame);
}

static ma_result ma_device_init__sdl(ma_device* pDevice, const ma_device_config* pConfig, ma_device_descriptor* pDescriptorPlayback, ma_device_descriptor* pDescriptorCapture)
{
   if (pConfig->deviceType != ma_device_type_playback)
      return MA_DEVICE_TYPE_NOT_SUPPORTED;

   auto pDeviceEx = reinterpret_cast<ma_device_ex*>(pDevice);

   auto requestedDeviceId = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
   if (pConfig->playback.pDeviceID)
      requestedDeviceId = pConfig->playback.pDeviceID->custom.i;

   pDeviceEx->stream = SDL_OpenAudioDeviceStream(requestedDeviceId, nullptr, ma_audio_callback_playback__sdl, pDeviceEx);
   if (pDeviceEx->stream == nullptr)
   {
      PLOGE << "Failed to open SDL audio device (Error: " << SDL_GetError() << ")";
      return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
   }
   pDeviceEx->deviceID = SDL_GetAudioStreamDevice(pDeviceEx->stream);
   int periodSizeInFrames;
   SDL_AudioSpec specs;
   SDL_GetAudioDeviceFormat(pDeviceEx->deviceID, &specs, &periodSizeInFrames);
   
   // Convert SDL format to miniaudio format
   ma_format deviceFormat;
   switch (specs.format)
   {
      case SDL_AUDIO_U8: deviceFormat = ma_format_u8; break;
      case SDL_AUDIO_S16: deviceFormat = ma_format_s16; break;
      case SDL_AUDIO_S32: deviceFormat = ma_format_s32; break;
      case SDL_AUDIO_F32: deviceFormat = ma_format_f32; break;
      default:
         PLOGI << "Unsupported SDL audio format " << SDL_GetAudioFormatName(specs.format) << " (0x" << std::hex << specs.format << std::dec << "), forcing to F32";
         specs.format = SDL_AUDIO_F32;
         if (!SDL_SetAudioStreamFormat(pDeviceEx->stream, nullptr, &specs))
         {
            PLOGE << "Failed to set audio stream format to F32";
            SDL_DestroyAudioStream(pDeviceEx->stream);
            return MA_FAILED_TO_OPEN_BACKEND_DEVICE;
         }
         deviceFormat = ma_format_f32;
         break;
   }

   // Update miniaudio descriptor with actual device settings
   pDescriptorPlayback->format = deviceFormat;
   pDescriptorPlayback->channels = specs.channels;
   pDescriptorPlayback->sampleRate = static_cast<ma_uint32>(specs.freq);
   pDescriptorPlayback->periodSizeInFrames = periodSizeInFrames;
   pDescriptorPlayback->periodCount = 1; // SDL doesn't use the notion of period counts, so just set to 1.

   // TODO check that the default channel map matches SDL channel map
   ma_channel_map_init_standard(ma_standard_channel_map_default, pDescriptorPlayback->channelMap, std::size(pDescriptorPlayback->channelMap), pDescriptorPlayback->channels);
   
   PLOGI << "Audio device initialized. Device: '" << SDL_GetAudioDeviceName(pDeviceEx->deviceID) << "', Freq : " << specs.freq << ", Format: " << SDL_GetAudioFormatName(specs.format) << ", Channels: " << specs.channels << ", Driver: " << SDL_GetCurrentAudioDriver();
   return MA_SUCCESS;
}

static ma_result ma_device_uninit__sdl(ma_device* pDevice)
{
   if (auto pDeviceEx = reinterpret_cast<ma_device_ex*>(pDevice); pDeviceEx->stream)
      SDL_DestroyAudioStream(pDeviceEx->stream);
   return MA_SUCCESS;
}

static ma_result ma_device_start__sdl(ma_device* pDevice)
{
   if (auto pDeviceEx = reinterpret_cast<ma_device_ex*>(pDevice); pDeviceEx->stream)
      SDL_ResumeAudioStreamDevice(pDeviceEx->stream);
   return MA_SUCCESS;
}

static ma_result ma_device_stop__sdl(ma_device* pDevice)
{
   if (auto pDeviceEx = reinterpret_cast<ma_device_ex*>(pDevice); pDeviceEx->stream)
      SDL_PauseAudioStreamDevice(pDeviceEx->stream);
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



namespace VPX
{

AudioPlayer::AudioPlayer(const Settings& settings)
   : m_music(nullptr)
{
   if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
      return;

   string soundDeviceName, soundDeviceBGName;
   const bool hasTableSoundDevice = settings.LoadValue(Settings::Player, "SoundDevice"s, soundDeviceName);
   const bool hasBackglassSOundDevice = settings.LoadValue(Settings::Player, "SoundDeviceBG"s, soundDeviceBGName);
   {
      int count;
      const SDL_AudioDeviceID* const pAudioList = SDL_GetAudioPlaybackDevices(&count);
      for (int i = 0; i < count; ++i)
      { // We identify by name as this is the only stable property (see https://github.com/libsdl-org/SDL/issues/12278)
         string name = SDL_GetAudioDeviceName(pAudioList[i]);
         if (hasTableSoundDevice && name == soundDeviceName)
            m_playfieldAudioDevice = pAudioList[i];
         if (hasBackglassSOundDevice && name == soundDeviceBGName)
            m_backglassAudioDevice = pAudioList[i];
      }
      if (m_playfieldAudioDevice == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK)
      {
         PLOGE << "Table sound device was not found (" << soundDeviceName << "), using default.";
         m_playfieldAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
      }
      if (m_backglassAudioDevice == SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK)
      {
         PLOGE << "Backglass sound device was not found (" << soundDeviceBGName << "), using default.";
         m_backglassAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
      }
      SDL_free(const_cast<SDL_AudioDeviceID*>(pAudioList));
   }

   m_soundMode3D = static_cast<SoundConfigTypes>(settings.LoadValueUInt(Settings::Player, "Sound3D"s));

   ma_result result;
   ma_context_config contextConfig;
   contextConfig = ma_context_config_init();
   contextConfig.custom.onContextInit = ma_context_init__sdl;

   m_maContext = std::make_unique<ma_context>();
   constexpr ma_backend backends[] = { ma_backend_custom };
   ma_context_init(backends, std::size(backends), &contextConfig, m_maContext.get());
   m_maContext->pUserData = this;

   struct SDLDeviceInfo
   {
      int id;
      ma_device_info dev;
   };
   auto selectDevice = [](ma_context* pContext, ma_device_type deviceType, const ma_device_info* pInfo, void* pUserData) {
      if (auto info = static_cast<SDLDeviceInfo*>(pUserData); pInfo->id.custom.i == info->id)
      {
         info->dev = *pInfo;
         return (ma_bool32)MA_FALSE;
      }
      return (ma_bool32)MA_TRUE;
   };

   {
      SDLDeviceInfo deviceInfo { m_backglassAudioDevice, { 0 } };
      ma_context_get_device_info(m_maContext.get(), ma_device_type_playback, nullptr, &deviceInfo.dev);
      ma_context_enumerate_devices(m_maContext.get(), selectDevice, &deviceInfo);

      m_backglassDevice = std::make_unique<ma_device_ex>();
      ma_device_config deviceConfig;
      deviceConfig = ma_device_config_init(ma_device_type_playback);
      deviceConfig.playback.pDeviceID = &deviceInfo.dev.id;
      deviceConfig.playback.format = ma_format_f32;
      deviceConfig.noPreSilencedOutputBuffer = MA_TRUE; // We'll always be outputting to every frame in the callback so there's no need for a pre-silenced buffer.
      deviceConfig.noClip = MA_TRUE; // The engine will do clipping itself.
      result = ma_device_init(m_maContext.get(), &deviceConfig, reinterpret_cast<ma_device*>(m_backglassDevice.get()));

      if (result == MA_SUCCESS)
      {
         ma_engine_config engineConfig;
         engineConfig = ma_engine_config_init();
         engineConfig.pContext = m_maContext.get();
         engineConfig.pDevice = &m_backglassDevice->device;
         engineConfig.noAutoStart = MA_TRUE;
         m_backglassEngine = std::make_unique<ma_engine>();
         result = ma_engine_init(&engineConfig, m_backglassEngine.get());
         m_backglassDevice->device.onData = ma_engine_data_callback_internal;
         m_backglassDevice->device.pUserData = m_backglassEngine.get();
         ma_engine_start(m_backglassEngine.get());
      }
      else
      {
         PLOGE << "Failed to initialize miniaudio for backglass sounds";
         m_backglassDevice = nullptr;
      }
   }

   {
      SDLDeviceInfo deviceInfo { m_playfieldAudioDevice, { 0 } };
      ma_context_get_device_info(m_maContext.get(), ma_device_type_playback, nullptr, &deviceInfo.dev);
      ma_context_enumerate_devices(m_maContext.get(), selectDevice, &deviceInfo);

      m_playfieldDevice = std::make_unique<ma_device_ex>();
      ma_device_config deviceConfig;
      deviceConfig = ma_device_config_init(ma_device_type_playback);
      deviceConfig.playback.pDeviceID = &deviceInfo.dev.id;
      deviceConfig.playback.format = ma_format_f32;
      deviceConfig.noPreSilencedOutputBuffer = MA_TRUE; // We'll always be outputting to every frame in the callback so there's no need for a pre-silenced buffer.
      deviceConfig.noClip = MA_TRUE; // The engine will do clipping itself.
      result = ma_device_init(m_maContext.get(), &deviceConfig, reinterpret_cast<ma_device*>(m_playfieldDevice.get()));

      if (result == MA_SUCCESS)
      {
         ma_engine_config engineConfig;
         engineConfig = ma_engine_config_init();
         engineConfig.pContext = m_maContext.get();
         engineConfig.pDevice = &m_playfieldDevice->device;
         engineConfig.noAutoStart = MA_TRUE;
         m_playfieldEngine = std::make_unique<ma_engine>();
         result = ma_engine_init(&engineConfig, m_playfieldEngine.get());
         m_playfieldDevice->device.onData = ma_engine_data_callback_internal;
         m_playfieldDevice->device.pUserData = m_playfieldEngine.get();
         ma_engine_start(m_playfieldEngine.get());
      }
      else
      {
         PLOGE << "Failed to initialize miniaudio for playfield sounds";
         m_playfieldDevice = nullptr;
      }
   }
}

AudioPlayer::~AudioPlayer()
{
   m_soundPlayers.clear();
   m_audioStreams.clear();
   m_pendingDeleteAudioStreams.clear();
   m_music = nullptr;
   if (m_backglassEngine)
      ma_engine_uninit(m_backglassEngine.get());
   if (m_playfieldEngine)
      ma_engine_uninit(m_playfieldEngine.get());
   if (m_playfieldDevice)
      ma_device_uninit(&m_playfieldDevice->device);
   if (m_backglassDevice)
      ma_device_uninit(&m_backglassDevice->device);
   if (m_maContext)
      ma_context_uninit(m_maContext.get());
   if (m_backglassSDLDevice != 0)
      SDL_CloseAudioDevice(m_backglassSDLDevice);
   SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioPlayer::SetMainVolume(float backglassVolume, float playfieldVolume)
{
   m_backglassVolume = backglassVolume;
   m_playfieldVolume = playfieldVolume;
   if (m_music)
      m_music->SetMainVolume(backglassVolume, playfieldVolume);
   for (const auto& [sound, players] : m_soundPlayers)
      for (auto& player : players)
         player->SetMainVolume(backglassVolume, playfieldVolume);
   for (const auto& player : m_audioStreams)
      player->SetMainVolume(backglassVolume);
}

AudioPlayer::AudioStreamID AudioPlayer::OpenAudioStream(const string& name, int frequency, int channels, bool isFloat)
{
   if (m_backglassSDLDevice == 0)
   {
      SDL_AudioSpec deviceSpec;
      const bool hasDeviceSpec = SDL_GetAudioDeviceFormat(m_backglassAudioDevice, &deviceSpec, nullptr);
      m_backglassSDLDevice = SDL_OpenAudioDevice(m_backglassAudioDevice, hasDeviceSpec ? & deviceSpec : nullptr);
   }
   std::unique_ptr<AudioStreamPlayer> audioStream = AudioStreamPlayer::Create(m_backglassSDLDevice, frequency, channels, isFloat);
   if (audioStream == nullptr)
      return nullptr;
   AudioStreamID stream = std::move(audioStream);
   stream->SetMainVolume(m_backglassVolume);
   stream->SetName(name);
   m_audioStreams.push_back(stream);
   return stream;
}

void AudioPlayer::EnqueueStream(AudioStreamID stream, uint8_t* buffer, int length) const {
   stream->Enqueue(buffer, length);
}

void AudioPlayer::SetStreamVolume(AudioStreamID stream, const float volume) const {
   stream->SetStreamVolume(volume);
}

void AudioPlayer::CloseAudioStream(AudioStreamID stream, bool afterEndOfStream)
{
   auto item = std::ranges::find_if(m_audioStreams, [stream](std::shared_ptr<AudioStreamPlayer> player) { return player == stream; });
   if (item != m_audioStreams.end())
   {
      // Keep a reference until enqueued data has been played
      if (afterEndOfStream && (*item)->GetQueuedSize() != 0)
      {
         m_pendingDeleteAudioStreams.push_back(*item);
         (*item)->FlushStream();
      }
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
   for (const auto& soundPlayer : players)
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
   const vector<std::unique_ptr<SoundPlayer>>& players = m_soundPlayers[sound];
   for (const auto& player : players)
      player->Stop();
}

SoundSpec AudioPlayer::GetSoundInformations(const Sound* const sound) const
{
   SoundSpec specs { 0 };
   ma_decoder decoder;
   if (ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_unknown, 0, 0);
      ma_decoder_init_memory(sound->GetFileRaw(), sound->GetFileSize(), &decoderConfig, &decoder) != MA_SUCCESS)
      return specs;
   specs.nChannels = decoder.outputChannels;
   specs.sampleFrequency = decoder.outputSampleRate;

   ma_sound maSound;
   ma_sound_config config = ma_sound_config_init_2(m_backglassEngine.get());
   config.pDataSource = &decoder;
   if (ma_sound_init_ex(m_backglassEngine.get(), &config, &maSound))
      return specs;
   float length;
   ma_sound_get_length_in_seconds(&maSound, &length);
   specs.lengthInSeconds = length;
   ma_sound_uninit(&maSound);

   ma_decoder_uninit(&decoder);
   return specs;
}

vector<AudioPlayer::AudioDevice> AudioPlayer::EnumerateAudioDevices()
{
   if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
      PLOGE << "SDL Init Audio failed: " << SDL_GetError();
      return vector<AudioDevice>();
   }
   int count;
   auto pAudioList = SDL_GetAudioPlaybackDevices(&count);
   vector<AudioDevice> audioDevices;
   for (int i = 0; i < count; ++i)
   {
      SDL_AudioSpec spec;
      SDL_GetAudioDeviceFormat(pAudioList[i], &spec, nullptr);
      const AudioDevice audioDevice = { SDL_GetAudioDeviceName(pAudioList[i]), static_cast<unsigned int>(spec.channels) };
      audioDevices.push_back(audioDevice);
   }
   SDL_QuitSubSystem(SDL_INIT_AUDIO);
   return audioDevices;
}

}
