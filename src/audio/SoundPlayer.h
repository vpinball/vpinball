// license:GPLv3+

#pragma once

#include "AudioPlayer.h"
#include "ThreadPool.h"

struct ma_decoder;
struct ma_sound;
struct vpx_node;

namespace VPX
{

// Play a sound load from a Sound object, either untouched to backglass, or applying playfield channel setup with adjusted pitch and volume control.
class SoundPlayer
{
public:
   static SoundPlayer* Create(const AudioPlayer* audioPlayer, Sound* sound);
   static SoundPlayer* Create(const AudioPlayer* audioPlayer, const string& filename);
   ~SoundPlayer();

   void Play(float volume, const float randompitch, const int pitch, float pan, float frontRearFade, const int loopcount);
   void Pause();
   void Unpause();
   void Stop();

   float GetPosition() const;
   void SetPosition(float pos);

   bool IsPlaying() const;

   void SetMainVolume(float backglassVolume, float playfieldVolume);
   void SetVolume(float volume);

private:
   SoundPlayer(const AudioPlayer* audioPlayer, Sound* sound);
   SoundPlayer(const AudioPlayer* audioPlayer, const string& filename);

   const class AudioPlayer* const m_audioPlayer;
   const SoundOutTypes m_outputTarget;

   float m_monoCompensation = 1.f;
   float m_soundVolume = 1.f;
   float m_mainVolume = 1.f;
   int m_loopCount = 0;

   void ApplyVolume();

   std::unique_ptr<ma_decoder> m_decoder;
   std::unique_ptr<ma_sound> m_sound;
   std::unique_ptr<vpx_node> m_vpxMixNode;

   mutable ThreadPool m_commandQueue; // Worker thread on which all commands are dispatched

   static void OnSoundEnd(void* pUserData, ma_sound* pSound);
};

}
