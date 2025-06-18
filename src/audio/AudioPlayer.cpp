// license:GPLv3+

#include "core/stdafx.h"
#include "AudioPlayer.h"
#include "AudioStreamPlayer.h"
#include "MusicPlayer.h"
#include "SoundPlayer.h"

#include <SDL3_mixer/SDL_mixer.h>

namespace VPX
{

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

   // Initialize for Wav and Ogg Vorbis formats
   MIX_InitFlags initialized = Mix_Init(MIX_INIT_OGG);
   if ((initialized & MIX_INIT_OGG) == 0)
   {
      PLOGE << "Failed to initialize SDL Mixer for OGG decoding";
   }

   // Initialize the SDL mixer library on the playfield audio device
   SDL_AudioSpec spec;
   SDL_GetAudioDeviceFormat(m_tableAudioDevice, &spec, nullptr);
   SDL_AudioSpec* reqSpec = ((spec.format != SDL_AUDIO_S16LE) && (spec.format != SDL_AUDIO_F32LE) && (spec.format != SDL_AUDIO_S32LE)) ? &spec : nullptr;
   spec.format = SDL_AUDIO_F32LE; // If we don't support device's native format, request one we support
   if (!Mix_OpenAudio(m_tableAudioDevice, reqSpec))
   {
      PLOGE << "Failed to initialize SDL Mixer: " << SDL_GetError();
      return;
   }

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

   Mix_QuerySpec(&m_audioSpecOutput.freq, &m_audioSpecOutput.format, &m_audioSpecOutput.channels);

   // Default is 8 channels which is usually enough but add some margin anyway for tables with lots of sounds playing simultaneously
   Mix_AllocateChannels(32);

   const char* pdriverName = SDL_GetCurrentAudioDriver();
   PLOGI << "Output Device Settings: " << "Freq: " << m_audioSpecOutput.freq << " Format (SDL_AudioFormat): " << m_audioSpecOutput.format
         << " channels: " << m_audioSpecOutput.channels << ", driver: " << (pdriverName ? pdriverName : "NULL") ;
}

AudioPlayer::~AudioPlayer()
{
   m_soundPlayers.clear();
   m_audioStreams.clear();
   m_music = nullptr;
   Mix_CloseAudio();
   Mix_Quit();
   SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

void AudioPlayer::SetMainVolume(float backglassVolume, float playfieldVolume)
{
   m_backglassVolume = backglassVolume;
   m_playfieldVolume = playfieldVolume;
   if (m_music)
      m_music->SetMainVolume(backglassVolume);
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
   m_music = std::unique_ptr<MusicPlayer>(MusicPlayer::Create(filename));
   if (m_music)
   {
      m_music->SetMusicVolume(m_musicVolume);
      m_music->SetMainVolume(m_backglassVolume);
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

double AudioPlayer::GetMusicPosition() const
{
   return m_music ? m_music->GetPosition() : 0.0;
}
   
void AudioPlayer::SetMusicPosition(double seconds)
{
   if (m_music) m_music->SetPosition(seconds);
}

void AudioPlayer::SetMusicVolume(const float volume)
{
   m_musicVolume = volume;
   if (m_music) m_music->SetMusicVolume(volume);
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