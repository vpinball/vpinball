// license:GPLv3+

#pragma once

#include "core/Settings.h"
#include <SDL3/SDL_audio.h>

struct ma_engine;
struct ma_context;
struct ma_device_ex;

namespace VPX
{

struct SoundSpec
{
   float lengthInSeconds;
   unsigned int sampleFrequency;
   unsigned int nChannels;
};

// Audio playback system, supporting:
// - Multiple streamed backglass audio (for PinMAME, PUP, AltSound, etc.)
// - A (single) backglass music
// - Multiple sound effects (backglass audio or playfield sounds) with dynamic effects (SSF, panning, ...)
class AudioPlayer
{
public:
   AudioPlayer(const Settings& settings);
   ~AudioPlayer();

   void SetMainVolume(float backglassVolume, float playfieldVolume); // Overall gain, directly applied to all sounds, including the ones being played
   void SetMirrored(bool mirrored) { m_mirrored = mirrored; } // Whether the table is mirrored, affects sound panning

   // Audio stream, directly forwarded to audio device, respecting channel assignment, applying backglass global volume
   typedef void* AudioStreamID; // opaque pointer as objects are always owned by AudioPlayer without any public API
   AudioStreamID OpenAudioStream(int frequency, int channels);
   void EnqueueStream(AudioStreamID stream, void* buffer, int length);
   int GetStreamQueueSize(AudioStreamID stream) const;
   void SetStreamVolume(AudioStreamID stream, const float volume);
   void CloseAudioStream(AudioStreamID stream);

   // Music streamed from a file to audio device, respecting channel assignment, applying backglass global volume
   bool PlayMusic(const string& filename);
   void PauseMusic();
   void UnpauseMusic();
   float GetMusicPosition() const;
   void SetMusicPosition(float seconds);
   void SetMusicVolume(float volume);
   bool IsMusicPlaying() const;

   // Sound, played from memory buffer to backglass or playfield device, applying 3D mode setup
   void PlaySound(Sound* sound, float volume, const float randompitch, const int pitch, float pan, float front_rear_fade, const int loopcount, const bool usesame, const bool restart);
   void StopSound(Sound* sound);
   SoundSpec GetSoundInformations(Sound* sound) const;

   SoundConfigTypes GetSoundMode3D() const { return m_soundMode3D; }

   struct AudioDevice
   {
      string name;
      unsigned int channels;
   };
   // initializes the SDL audio subsystem and retrieves a list of available audio playback devices.
   static vector<AudioDevice> EnumerateAudioDevices();

   ma_engine* GetEngine(SoundOutTypes out) const { return out == SoundOutTypes::SNDOUT_TABLE ? m_playfieldEngine.get() : m_backglassEngine.get(); }

private:
   float m_playfieldVolume = 1.f;
   float m_backglassVolume = 1.f;
   float m_musicVolume = 1.f;
   bool m_mirrored = false;

   int m_playfieldAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
   int m_backglassAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;

   mutable ankerl::unordered_dense::map<Sound*, vector<std::unique_ptr<class SoundPlayer>>> m_soundPlayers;

   vector<std::unique_ptr<class AudioStreamPlayer>> m_audioStreams;

   std::unique_ptr<class SoundPlayer> m_music;

   SoundConfigTypes m_soundMode3D = SNDCFG_SND3D2CH; // What 3Dsound Mode are we in from VPinball.ini "Sound3D" key.

   std::unique_ptr<ma_context> m_maContext;
   std::unique_ptr<ma_device_ex> m_backglassDevice;
   std::unique_ptr<ma_engine> m_backglassEngine;
   std::unique_ptr<ma_device_ex> m_playfieldDevice;
   std::unique_ptr<ma_engine> m_playfieldEngine;
};

}
