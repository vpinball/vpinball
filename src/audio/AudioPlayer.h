// license:GPLv3+

#pragma once

#include "core/Settings.h"
#include <SDL3/SDL_audio.h>

namespace VPX
{

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
   double GetMusicPosition() const;
   void SetMusicPosition(double seconds);
   void SetMusicVolume(const float volume);
   bool IsMusicPlaying() const;

   // Sound, played from memory buffer to backglass or playfield device, applying 3D mode setup
   void PlaySound(Sound* sound, float volume, const float randompitch, const int pitch, float pan, float front_rear_fade, const int loopcount, const bool usesame, const bool restart);
   void StopSound(Sound* sound);

   const SDL_AudioSpec& GetAudioSpecOutput() const { return m_audioSpecOutput; }
   const SoundConfigTypes GetSoundMode3D() const { return m_SoundMode3D; }

   struct AudioDevice
   {
      int id;
      string name;
      unsigned int channels; //number of speakers in this case
   };
   static void EnumerateAudioDevices(vector<AudioDevice>& devices);

private:
   float m_playfieldVolume = 1.f;
   float m_backglassVolume = 1.f;
   float m_musicVolume = 1.f;
   bool m_mirrored = false;

   ankerl::unordered_dense::map<Sound*, vector<std::unique_ptr<class SoundPlayer>>> m_soundPlayers;

   vector<std::unique_ptr<class AudioStreamPlayer>> m_audioStreams;

   std::unique_ptr<class MusicPlayer> m_music;

   int m_tableAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
   int m_backglassAudioDevice = SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
   SoundConfigTypes m_SoundMode3D = SNDCFG_SND3D2CH; // What 3Dsound Mode are we in from VPinball.ini "Sound3D" key.
   SDL_AudioSpec m_audioSpecOutput { SDL_AUDIO_UNKNOWN, 0, 0 }; // The output devices audio spec
};

}