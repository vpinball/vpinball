// license:GPLv3+

#pragma once

#include "AudioPlayer.h"
#include "ThreadPool.h"

#include <chrono>

struct ma_decoder;
struct ma_sound;
struct vpx_node;

namespace VPX
{
class SpatialAudioSource;
}

namespace VPX
{

// Play a sound load from a Sound object, either untouched to backglass, or applying playfield channel setup with adjusted pitch and volume control.
class SoundPlayer
{
public:
   static SoundPlayer* Create(const AudioPlayer* audioPlayer, Sound* sound);
   static SoundPlayer* Create(const AudioPlayer* audioPlayer, const std::filesystem::path& filename);
   ~SoundPlayer();

   // Plays the sound. Position is either given explicitly (hasPos, table units, z above the playfield)
   // or derived from the legacy pan/frontRearFade parameters.
   void Play(float volume, const float randompitch, const int pitch, float pan, float frontRearFade, const int loopcount, bool hasPos, float posX, float posY, float posZ);
   void Pause();
   void Unpause();
   void Stop();

   float GetPosition() const;
   void SetPosition(float pos);

   bool IsPlaying() const;

   void SetMainVolume(float backglassVolume, float playfieldVolume);
   void SetVolume(float volume);

   SoundOutTypes GetOutputTarget() const { return m_outputTarget; }

private:
   SoundPlayer(const AudioPlayer* audioPlayer, Sound* sound);
   SoundPlayer(const AudioPlayer* audioPlayer, const std::filesystem::path& filename);

   const class AudioPlayer* const m_audioPlayer;
   const SoundOutTypes m_outputTarget;

   float m_monoCompensation = 1.f;
   float m_soundVolume = 1.f;
   float m_mainVolume = 1.f;
   int m_loopCount = 0;

   void ApplyVolume();
   void UpdateDoppler(float x, float y, float z);

   std::unique_ptr<ma_decoder> m_decoder;
   std::unique_ptr<ma_sound> m_sound;
   std::unique_ptr<SpatialAudioSource> m_spatialSource;
   std::unique_ptr<vpx_node> m_vpxMixNode; // Legacy channel mixing node, used when spatial audio is disabled

   // Doppler effect, estimated from successive position updates
   float m_pitchFactor = 1.f;
   float m_dopplerFactor = 1.f;
   float m_lastDistance = 0.f;
   bool m_hasLastDistance = false;
   std::chrono::steady_clock::time_point m_lastPosTime;

   mutable ThreadPool m_commandQueue; // Worker thread on which all commands are dispatched

   const string m_callbackId;
   static void OnSoundEnd(void* pUserData, ma_sound* pSound);
};

}
