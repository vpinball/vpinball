// license:GPLv3+

#pragma once

#include "AudioPlayer.h"
#include "ThreadPool.h"
#include <SDL3_mixer/SDL_mixer.h>

namespace VPX
{

// Play a sound load from a Sound object, either untouched to backglass, or applying playfield channel setup with adjusted pitch and volume control.
// This class expects SDL_audio & SDL_mixer library to be fully initialized before use
class SoundPlayer
{
public:
   static SoundPlayer* Create(AudioPlayer* AudioPlayer, Sound* sound);
   ~SoundPlayer();

   void Play(float volume, const float randompitch, const int pitch, float pan, float frontRearFade, const int loopcount);
   void Stop();

   bool IsPlaying() const { return m_assignedChannel != -1 && Mix_Playing(m_assignedChannel); }

   void SetMainVolume(float backglassVolume, float playfieldVolume);

private:
   SoundPlayer(AudioPlayer* AudioPlayer, Sound* sound);
   SoundPlayer(AudioPlayer* AudioPlayer, Mix_Chunk* mixChunk, SoundOutTypes outputTarget);

   class AudioPlayer* const m_audioPlayer;
   Mix_Chunk* m_pMixChunkOrg = nullptr; // the original unmodified loaded sound
   const SoundOutTypes m_outputTarget;

   float m_soundVolume = 1.f;
   float m_mainVolume = 1.f;

   void ApplyVolume();

   Mix_Chunk* m_pMixChunk = nullptr; // we use this one when we resample for pitch changes
   int m_assignedChannel = -1; // dynamically assigned mixer channel when the sound is playing

   ThreadPool m_commandQueue; // Worker thread on which all commands are dispatched

   // Resample the original sound to match the pitch settings sent from the table each time.
   // This function can be very slow (a few ms) and may not be called on the main thread.
   void AdjustPitch(int pitch, float randompitch);

   // Callback to release assigned channel
   static void OnPlayFinished(int channel, void* udata);

   // Mixer effect that blend sound according to the channels weights
   float m_mixFL = 0.f;
   float m_mixFR = 0.f;
   float m_mixSL = 0.f;
   float m_mixSR = 0.f;
   float m_mixBL = 0.f;
   float m_mixBR = 0.f;
   static void MixStereoToChannelsEffect(int chan, void* stream, int len, void* udata);

   // Null mixer effect, needed for dynamic channel management
   static void NullEffect(int chan, void* stream, int len, void* udata) { }

   // MixEffects support funcs
   static void CalcPan(float& leftPanRatio, float& rightPanRatio, float adjustedVolRatio, float pan);
   static void CalcFade(float leftPanRatio, float rightPanRatio, float fadeRatio, float& frontLeft, float& frontRight, float& rearLeft, float& rearRight);
   static float PanSSF(float pan);
   static float PanTo3D(float input);
   static float FadeSSF(float front_rear_fade);
};

}
