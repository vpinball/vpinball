// license:GPLv3+

#pragma once

#include "core/Settings.h"
#include <SDL3/SDL_audio.h>

struct ma_engine;
struct ma_context;
struct ma_device_ex;

namespace VPX
{

// Playfield sound playback mode
// =============================
// This option select how the playfield table sounds are rendered. While default backglass audio and streamed audio (VPinMAME, 
// AltSound, PuP, FlexDMD, ...) always plays from the front channels of the selected audio device (which may be a
// different device or not)
enum SoundConfigTypes : int
{
   // 2CH: Render playfield sounds as 2 channels to the front channel of the selected audio device (which should be a different 
   // one than the audio device playing backglass audio)
   SNDCFG_SND3D2CH = 0,

   // ALLREAR: Render playfield sounds as 2 channels to the rear channel of the audio card
   // This can replace the need to use two sound cards to move table audio inside the cab.
   SNDCFG_SND3DALLREAR = 1,

   // FRONTISREAR: Render playfield sounds as up to 6 channels to the front/side/rear channels of the audio card (depending 
   // on the audio card type), with the rear channels near the front of the cab (where the player stands).
   // Table effects are mapped such that the front of the cab is the rear surround channels. If you were to play
   // VPX in a home theater system with the TV in front of you, this would produce an appropriate result with the ball coming
   // from the rear channels as it gets closer to you.
   SNDCFG_SND3DFRONTISREAR = 2,

   // FRONTISFRONT: Render playfield sounds as up to 6 channels to the front/side/rear channels of the audio card (depending 
   // on the audio card type), with the front channels near the front of the cab (where the player stands).
   // Recommended mapping for a dedicated sound card attached to the playfield. Front channel maps to the front
   // of the cab. We "flip" the rear to the standard 2 channels, so older versions of VP still play sounds on the front most
   // channels of the cab. This mapping could also be used to place 6 channels on the playfield.
   SNDCFG_SND3DFRONTISFRONT = 3,

   // 6CH: Render playfield sounds to 4 channels: side (back of the cab) & rear (front of the cab) channels of the audio 
   // card, leaving the front channels for backglass audio (similar to SNDCFG_SND3DFRONTISFRONT but leaving front channels empty)
   // Rear of playfield shifted to the sides, and front of playfield shifted to the far rear. Leaves front channels open
   // for default backglass and VPinMAME.
   SNDCFG_SND3D6CH = 4,

   // SSF: Same as 6CH but with a different sound horizontal panning and vertical fading are enhanced for a more realistic experience.
   SNDCFG_SND3DSSF = 5
};

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
   explicit AudioPlayer(const Settings& settings);
   ~AudioPlayer();

   void SetMainVolume(float backglassVolume, float playfieldVolume); // Overall gain, directly applied to all sounds, including the ones being played
   void SetMirrored(bool mirrored) { m_mirrored = mirrored; } // Whether the table is mirrored, affects sound panning

   // Audio stream, directly forwarded to audio device, respecting channel assignment, applying backglass global volume
   using AudioStreamID = std::shared_ptr<class AudioStreamPlayer>; // opaque pointer as objects are always owned by AudioPlayer without any public API
   AudioStreamID OpenAudioStream(const string& name, int frequency, int channels, bool isFloat);
   void EnqueueStream(AudioStreamID stream, uint8_t* buffer, int length) const;
   void SetStreamVolume(AudioStreamID stream, const float volume) const;
   void CloseAudioStream(AudioStreamID stream, bool afterEndOfStream);

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
   SoundSpec GetSoundInformations(const Sound* const sound) const;

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
   SDL_AudioDeviceID m_backglassSDLDevice = 0;

   mutable ankerl::unordered_dense::map<Sound*, vector<std::unique_ptr<class SoundPlayer>>> m_soundPlayers;

   vector<AudioStreamID> m_audioStreams;
   vector<AudioStreamID> m_pendingDeleteAudioStreams;

   std::unique_ptr<class SoundPlayer> m_music;

   SoundConfigTypes m_soundMode3D = SNDCFG_SND3D2CH; // What 3Dsound Mode are we in from VPinball.ini "Sound3D" key.

   std::unique_ptr<ma_context> m_maContext;
   std::unique_ptr<ma_device_ex> m_backglassDevice;
   std::unique_ptr<ma_engine> m_backglassEngine;
   std::unique_ptr<ma_device_ex> m_playfieldDevice;
   std::unique_ptr<ma_engine> m_playfieldEngine;
};

}
