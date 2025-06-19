// license:GPLv3+

#include "core/stdafx.h"
#include "SoundPlayer.h"

namespace VPX
{

SoundPlayer* SoundPlayer::Create(AudioPlayer* audioPlayer, Sound* sound)
{
   /*SDL_IOStream* const stream = SDL_IOFromMem((void*)sound->GetFileRaw(), sound->GetFileSize());
   if (stream == nullptr)
   {
      PLOGE << "SDL_IOFromMem error: " << SDL_GetError();
      return nullptr;
   }

   Mix_Chunk* decodedData = Mix_LoadWAV_IO(stream, true);
   if (decodedData == nullptr)
   {
      PLOGE << "Failed to load sound via Mix_LoadWAV_IO: " << SDL_GetError();
      return nullptr;
   }

   return new SoundPlayer(audioPlayer, decodedData, sound->GetOutputTarget());*/
   
   // Decode and resample the sound on the anciliary thread as this is fairly heavy
   return new SoundPlayer(audioPlayer, sound);
}

SoundPlayer::SoundPlayer(AudioPlayer* audioPlayer, Mix_Chunk* mixChunk, SoundOutTypes outputTarget)
   : m_audioPlayer(audioPlayer)
   , m_pMixChunkOrg(mixChunk)
   , m_outputTarget(outputTarget)
   , m_commandQueue(1)
{
}

SoundPlayer::SoundPlayer(AudioPlayer* audioPlayer, Sound* sound)
   : m_audioPlayer(audioPlayer)
   , m_outputTarget(sound->GetOutputTarget())
   , m_commandQueue(1)
{
   m_commandQueue.enqueue([this, sound]() {
      SDL_IOStream* const stream = SDL_IOFromMem((void*)sound->GetFileRaw(), sound->GetFileSize());
      if (stream == nullptr)
      {
         PLOGE << "SDL_IOFromMem error: " << SDL_GetError();
         return;
      }

      m_pMixChunkOrg = Mix_LoadWAV_IO(stream, true);
      if (m_pMixChunkOrg == nullptr)
      {
         PLOGE << "Failed to load sound via Mix_LoadWAV_IO: " << SDL_GetError();
         return;
      }
   });
}

SoundPlayer::~SoundPlayer()
{
   m_commandQueue.wait_until_empty();
   m_commandQueue.wait_until_nothing_in_flight();
   if (m_assignedChannel != -1)
      Mix_HaltChannel(m_assignedChannel);
   if (m_pMixChunkOrg != nullptr)
      Mix_FreeChunk(m_pMixChunkOrg);
   if (m_pMixChunk != nullptr)
      Mix_FreeChunk(m_pMixChunk);
}

void SoundPlayer::SetMainVolume(float backglassVolume, float playfieldVolume)
{
   m_mainVolume = m_outputTarget == SNDOUT_BACKGLASS ? backglassVolume : playfieldVolume;
   m_commandQueue.enqueue([this]() { ApplyVolume(); });
}

void SoundPlayer::ApplyVolume()
{
   if (m_pMixChunk == nullptr)
      return;
   float volume = clamp(m_soundVolume * m_mainVolume, 0.0f, 1.0f);
   Mix_VolumeChunk(m_pMixChunk, static_cast<int>(volume * static_cast<float>(MIX_MAX_VOLUME)));
}

void SoundPlayer::Play(float volume, const float randompitch, const int pitch, float pan, float frontRearFade, const int loopcount)
{
   m_commandQueue.enqueue([this, volume, randompitch, pitch, pan, frontRearFade, loopcount]()
   {
      if (m_pMixChunkOrg == nullptr)
         return;

      const bool playing = IsPlaying();
      
      float leftPanRatio, rightPanRatio;
      m_mixFL = m_mixFR = 0.f;
      m_mixSL = m_mixSR = 0.f;
      m_mixBL = m_mixBR = 0.f;
      switch (m_outputTarget == SNDOUT_BACKGLASS ? SNDCFG_SND3D2CH : m_audioPlayer->GetSoundMode3D())
      {
      case SNDCFG_SND3D2CH:
         CalcPan(m_mixFL, m_mixFR, 1.f, pan * 3.0f);
         break;

      case SNDCFG_SND3DALLREAR:
         CalcPan(m_mixBL, m_mixBR, 1.f, PanTo3D(pan));
         break;

      case SNDCFG_SND3D6CH: // we just fall through to SSF. This mode is same but it used two different pan and fade algos. No need to have two different ones now.
      case SNDCFG_SND3DSSF:
         CalcPan(leftPanRatio, rightPanRatio, 1.f, PanSSF(pan));
         CalcFade(leftPanRatio, rightPanRatio, FadeSSF(frontRearFade), m_mixBL, m_mixBR, m_mixSL, m_mixSR);
         break;

      default: assert(false); return;
      }

      if (playing)
      {
         // FIXME apply pitch changes
         // This is not easy as SDL_mixer does not offer anyway to change sample length during play, and sadly this is really needed to get good ball rolling sounds
         // One option would be to move away from SDL_mixer and use another library like SoLoud (using SDL as the audio backend)

         m_soundVolume = volume;
         ApplyVolume();
      }
      else
      {
         // Resample sound to the expected pitch (lengthy operation)
         AdjustPitch(pitch, randompitch);

         m_soundVolume = volume;
         ApplyVolume();

         m_assignedChannel = Mix_PlayChannel(m_assignedChannel, m_pMixChunk, loopcount > 0 ? loopcount - 1 : loopcount);
         Mix_RegisterEffect(m_assignedChannel, MixStereoToChannelsEffect, OnPlayFinished, this);
      }
   });
}

void SoundPlayer::Stop()
{
   m_commandQueue.enqueue([&]()
   {
      if (m_assignedChannel != -1)
         Mix_HaltChannel(m_assignedChannel);
   });
}

void SoundPlayer::OnPlayFinished(int channel, void* udata)
{
   static_cast<SoundPlayer*>(udata)->m_assignedChannel = -1; // Free channel for dynamic reuse
}

/**
 * @brief Sets the pitch of the sound, optionally with random variation.
 *
 * This function adjusts the pitch of the sound associated with the `Sound` object. If a non-zero pitch or random 
 * pitch variation is provided, the function resamples the sound to the new pitch. If the pitch remains unchanged
 * (both pitch and randompitch are zero), the original sound is used.
 * 
 * The function first frees any previously allocated `Mix_Chunk` to ensure the memory is managed correctly. 
 * If the pitch change is needed, the function calculates the new frequency (taking into account any random variation)
 * and performs the resampling process to convert the original audio sample to the new pitch. It then converts the
 * audio back to the original format, ensuring the sound is ready for playback at the correct pitch.
 * 
 * @note Since mixer doesn't support adjusting the frequency or the stream buffer length via a MixEffect
 *       we must resample the audio each time and replace it.
 *
 * @param pitch The target pitch to apply to the sound. A positive or negative integer alters the frequency.
 * @param randompitch The random variation to apply to the pitch, influencing the final frequency. A value greater than 0 introduces a random factor, while 0 means no random variation.
 */
void SoundPlayer::AdjustPitch(int pitch, float randompitch)
{
   if (m_pMixChunk != nullptr) {
      Mix_FreeChunk(m_pMixChunk);
      m_pMixChunk = nullptr;
   }

   // FIXME the pitch/randompitch are supposed to be applied relatively from the original sample frequency
   // since we do not have this information we guess it was a 'standard' 44100Hz
   // Use Mix_LoadSndFile_IO (not part of the public API) ?
   const float sampleFreq = 44100.f;
   float newFreq = sampleFreq + pitch;
   if (randompitch > 0.f) {
      const float rndh = rand_mt_01();
      const float rndl = rand_mt_01();
      newFreq *= 1.f + (randompitch * rndh * rndh) - (randompitch * rndl * rndl * 0.5f);
   }

   m_pMixChunk = static_cast<Mix_Chunk*>(SDL_malloc(sizeof(Mix_Chunk))); // need to use SDL_malloc as SDL_free will be used when freeing the chunk
   m_pMixChunk->volume = MIX_MAX_VOLUME;

   if (abs(newFreq - sampleFreq) > 100.f)
   {
      m_pMixChunk->allocated = 1; // allocated, Mix_FreeChunk will free the buffer allocated by SDL_ConvertAudioSamples
      const float outFreq = static_cast<float>(m_audioPlayer->GetAudioSpecOutput().freq); // Frequency of the output and of the samples as all loaded samples are automatically resampled to the output spec
      SDL_AudioSpec audioSpecConvert;
      audioSpecConvert.format = m_audioPlayer->GetAudioSpecOutput().format;
      audioSpecConvert.channels = m_audioPlayer->GetAudioSpecOutput().channels;
      audioSpecConvert.freq = static_cast<int>(outFreq * sampleFreq / newFreq);
      SDL_ConvertAudioSamples(&m_audioPlayer->GetAudioSpecOutput(), m_pMixChunkOrg->abuf, (int)m_pMixChunkOrg->alen,
                              &audioSpecConvert, &m_pMixChunk->abuf, (int*)&m_pMixChunk->alen);
   }
   else
   {
      m_pMixChunk->allocated = 0; // not allocated, just a reference to the original data
      m_pMixChunk->abuf = m_pMixChunkOrg->abuf;
      m_pMixChunk->alen = m_pMixChunkOrg->alen;
   }
}


/**
 * (Static)
 * @brief Calculates the left and right channel volume ratios based on the pan value.
 * 
 * This function takes an input pan value (ranging from -3 to 3) and normalizes it to a 
 * standard stereo pan range of [-1, 1]. It then computes the left and right channel 
 * volume ratios using a linear panning formula.
 * 
 * @param[out] leftPanRatio  Reference to a float where the computed left channel ratio will be stored.
 * @param[out] rightPanRatio Reference to a float where the computed right channel ratio will be stored.
 * @param[in] adjustedVolRatio The overall volume adjustment ratio (typically in range [0, 1]).
 * @param[in] pan The panning position, where -3 is full left, 0 is center, and 3 is full right.
 * 
 * @note The function ensures that the output volume ratios are clamped between 0 and 1 to prevent overflow.
 */
void SoundPlayer::CalcPan(float& leftPanRatio, float& rightPanRatio, float adjustedVolRatio, float pan)
{
   // Normalize pan from range [-3, 3] to [-1, 1]
   pan = pan / 3.0f;

   // Ensure pan is within -1 to 1 (in case of floating-point errors)
   pan = clamp(pan, -1.0f, 1.0f);

   // Use a more standard panning formula that keeps values within range
   float leftFactor  = 0.5f * (1.0f - pan); // Left decreases as pan increases
   float rightFactor = 0.5f * (1.0f + pan); // Right increases as pan increases

   leftPanRatio = adjustedVolRatio * leftFactor;
   rightPanRatio = adjustedVolRatio * rightFactor;

   // Ensure the values are properly clamped
   leftPanRatio  = clamp(leftPanRatio,  0.0f, 1.0f);
   rightPanRatio = clamp(rightPanRatio, 0.0f, 1.0f);

   //PLOGI << "Pan: " << pan << " AdjustedVolRatio: " << adjustedVolRatio << " Left: " << leftPanRatio << " Right: " << rightPanRatio;
}

/**
 * (Static)
 * @brief Calculates the fade distribution between front and rear channels based on the given fade ratio and panning ratios.
 *
 * This function computes the gain values for front-left, front-right, rear-left, and rear-right channels
 * based on the provided left/right panning ratios and a fade ratio. The fade ratio determines the balance
 * between front and rear channels, where:
 *  - A fadeRatio of 0.0 fully favors the rear speakers.
 *  - A fadeRatio of 2.5 fully favors the front speakers.
 *  - A fadeRatio between 0.0 and 2.5 interpolates linearly between rear and front.
 *
 * @param leftPanRatio  The left channel's panning ratio (typically in the range [0.0, 1.0]).
 * @param rightPanRatio The right channel's panning ratio (typically in the range [0.0, 1.0]).
 * @param fadeRatio     The fade control value determining the balance between front and rear channels.
 *                      It is clamped to the range [0.0, 2.5].
 * @param frontLeft     Output parameter for the front-left channel gain.
 * @param frontRight    Output parameter for the front-right channel gain.
 * @param rearLeft      Output parameter for the rear-left channel gain.
 * @param rearRight     Output parameter for the rear-right channel gain.
 */
void SoundPlayer::CalcFade(float leftPanRatio, float rightPanRatio, float fadeRatio, float& frontLeft, float& frontRight, float& rearLeft, float& rearRight)
{
   // Clamp fadeRatio to the range [0.0, 2.5]
   fadeRatio = clamp(fadeRatio, 0.0f, 2.5f);

   // Calculate front and rear weights (linear fade)
   //float rearWeight = max(0.0f, 2.5f - fadeRatio) / 2.5f; // 1 at fadeRatio=0, 0 at fadeRatio=2.5
   //float frontWeight = min(2.5f, fadeRatio) / 2.5f;       // 0 at fadeRatio=0, 1 at fadeRatio=2.5
   float rearWeight = (2.5f - fadeRatio) / 2.5f;
   float frontWeight = fadeRatio / 2.5f;

   // Apply panning ratios
   frontLeft  = frontWeight * leftPanRatio;
   frontRight = frontWeight * rightPanRatio;
   rearLeft   = rearWeight * leftPanRatio;
   rearRight  = rearWeight * rightPanRatio;

   //PLOGI << "FadeRatio: " << fadeRatio << " FrontLeft: " << frontLeft << " FrontRight: " << frontRight << " RearLeft: " << rearLeft << " RearRight: " << rearRight;
}

// The existing pan value in PlaySound function takes a -1 to 1 value, however it's extremely non-linear.
// -0.1 is very obviously to the left.  Table scripts like the ball rolling script seem to use x^10 to map
// linear positions, so we'll use that and reverse it.   Also multiplying by 3 since that seems to be the
// total distance necessary to fully pan away from one side at the center of the room.
// static
float SoundPlayer::PanTo3D(float input)
{
   // DirectSound's position command does weird things at exactly 0. 
   if (fabsf(input) < 0.0001f)
      input = (input < 0.0f) ? -0.0001f : 0.0001f;
   if (input < 0.0f)
      return -powf(-max(input, -1.0f), (float)(1.0 / 10.0)) * 3.0f;
   else
      return powf(min(input, 1.0f), (float)(1.0 / 10.0)) * 3.0f;
}

// This is a replacement function for PanTo3D() for sound effect panning (audio x-axis).
// It performs the same calculations but maps the resulting values to an area of the 3D 
// sound stage that has the expected panning effect for this application. It is written 
// in a long form to facilitate tweaking the formulas.  *njk*
// static
float SoundPlayer::PanSSF(float pan)
{
   // This math could probably be simplified but it is kept in long form
   // to aid in fine tuning and clarity of function.

   // Clip the pan input range to -1.0 to 1.0
   float x = clamp(pan, -1.f, 1.f);

   // Rescale pan range from an exponential [-1,0] and [0,1] to a linear [-1.0, 1.0]
   // Do not avoid values close to zero like PanTo3D() does as that
   // prevents the middle range of the exponential curves converting back to 
   // a linear scale (which would leave a gap in the center of the range).
   // This basically undoes the Pan() fading function in the table scripts.

   x = (x < 0.0f) ? -powf(-x, 0.1f) : powf(x, 0.1f);

   // Increase the pan range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

   x *= 3.0f;

   // BASS pan effect is much better than VPX 10.6/DirectSound3d but it
   // could still stand a little enhancement to exaggerate the effect.
   // The effect goal is to place slingshot effects almost entirely left/right
   // and flipper effects in the cross fade region (louder on their corresponding
   // sides but still audible on the opposite side..)

   // Rescale [-3.0,0.0) to [-3.00,-2.00] and [0,3.0] to [2.00,3.00]

   // Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
   // x' = ( (x - o1) / (o2 - o1) ) * (n2 - n1) + n1
   //
   // We retain the full formulas below to make it easier to tweak the values.
   // The compiler will optimize away the excess math.

   if (x >= 0.0f)
      x = ((x -  0.0f) / (3.0f -  0.0f)) * ( 3.0f -  2.0f) +  2.0f;
   else
      x = ((x - -3.0f) / (0.0f - -3.0f)) * (-2.0f - -3.0f) + -2.0f;

   // Clip the pan output range to 3.0 to -3.0
   //
   // This probably can never happen but is here in case the formulas above
   // change or there is a rounding issue.

   if (x > 3.0f)
      x = 3.0f;
   else if (x < -3.0f)
      x = -3.0f;

   // If the final value is sufficiently close to zero it causes sound to come from
   // all speakers and lose it's positional effect. We scale well away from zero
   // above but will keep this check to document the effect or catch the condition
   // if the formula above is later changed to one that can result in x = 0.0.

   // NOTE: This no longer seems to be the case with VPX 10.7/BASS

   // HOWEVER: Weird things still happen NEAR 0.0 or if both x and z are at 0.0.
   //          So we keep the fix here with wider margins to prevent that case.
   //          The current formula won't produce values in this weird range.

   if (fabsf(x) < 0.1f)
      x = (x < 0.0f) ? -0.1f : 0.1f;

   return x;
}

// This is a replacement function for PanTo3D() for sound effect fading (audio z-axis).
// It performs the same calculations but maps the resulting values to 
// an area of the 3D sound stage that has the expected fading
// effect for this application. It is written in a long form to facilitate tweaking the 
// values (which turned out to be more straightforward than originally coded). *njk*
// static
float SoundPlayer::FadeSSF(float front_rear_fade)
{
   float z;

   // Clip the fade input range to -1.0 to 1.0

   if (front_rear_fade < -1.0f)
      z = -1.0f;
   else if (front_rear_fade > 1.0f)
      z = 1.0f;
   else
      z = front_rear_fade;

   // Rescale fade range from an exponential [0,-1] and [0,1] to a linear [-1.0, 1.0]
   // Do not avoid values close to zero like PanTo3D() does at this point as that
   // prevents the middle range of the exponential curves converting back to 
   // a linear scale (which would leave a gap in the center of the range).
   // This basically undoes the AudioFade() fading function in the table scripts.   

   z = (z < 0.0f) ? -powf(-z, 0.1f) : powf(z, 0.1f);

   // Increase the fade range from [-1.0, 1.0] to [-3.0, 3.0] to improve the surround sound fade effect

   z *= 3.0f;

   // Rescale fade range from [-3.0,3.0] to [0.0,-2.5] in an attempt to remove all sound from
   // the surround sound front (backbox) speakers and place them close to the surround sound
   // side (cabinet rear) speakers.
   //
   // Reminder: Linear Conversion Formula [o1,o2] to [n1,n2]
   // z' = ( (z - o1) / (o2 - o1) ) * (n2 - n1) + n1
   //
   // We retain the full formulas below to make it easier to tweak the values.
   // The compiler will optimize away the excess math.

   // Rescale to -2.5 instead of -3.0 to further push sound away from rear channels
   z = ((z - -3.0f) / (3.0f - -3.0f)) * (-2.5f - 0.0f) + 0.0f;

   // With BASS the above scaling is sufficient to keep the playfield sounds out of 
   // the backbox. However playfield sounds are heavily weighted to the rear channels. 
   // For BASS we do a simple scale of the top third [0,-1.0] BY 0.10 to favor
   // the side channels. This is better than we could do in VPX 10.6 where z just
   // had to be set to 0.0 as there was no fade range that didn't leak to the backbox
   // as well.
   
   if (z > -1.0f)
      z = z / 10.0f;

   // Clip the fade output range to 0.0 to -3.0
   //
   // This probably can never happen but is here in case the formulas above
   // change or there is a rounding issue. A result even slightly greater
   // than zero can bleed to the backbox speakers.

   if (z > 0.0f)
      z = 0.0f;
   else if (z < -3.0f)
      z = -3.0f;

   // If the final value is sufficiently close to zero it causes sound to come from
   // all speakers on some systems and lose it's positional effect. We do use 0.0 
   // above and could set the safe value there. Instead will keep this check to document 
   // the effect or catch the condition if the formula/conditions above are later changed

   // NOTE: This no longer seems to be the case with VPX 10.7/BASS

   // HOWEVER: Weird things still happen near 0.0 or if both x and z are at 0.0.
   //          So we keep the fix here to prevent that case. This does push a tiny bit 
   //          of audio to the rear channels but that is perfectly ok.

   if (fabsf(z) < 0.0001f)
      z = -0.0001f;

   return fabsf(z); // I changed this to get a positive range from 0 to 2.5. not sure why before they returned negative
}

void SoundPlayer::MixStereoToChannelsEffect(int chan, void* stream, int len, void* udata)
{
   SoundPlayer* const me = static_cast<SoundPlayer*>(udata);
   const float levelFL = me->m_mixFL;
   const float levelFR = me->m_mixFR;
   const float levelBL = me->m_mixBL;
   const float levelBR = me->m_mixBR;
   const float levelSL = me->m_mixSL;
   const float levelSR = me->m_mixSR;
   // 4 channels (quad) layout: FL, FR, BL, BR
   // 5 channels (4.1)  layout: FL, FR, LFE, BL, BR
   // 6 channels (5.1)  layout: FL, FR, FC, LFE, BL, BR (last two can also be SL, SR)
   // 7 channels (6.1)  layout: FL, FR, FC, LFE, BC, SL, SR
   // 8 channels (7.1)  layout: FL, FR, FC, LFE, BL, BR, SL, SR
   const int channels = me->m_audioPlayer->GetAudioSpecOutput().channels;
   const int backOffset = channels <= 2 ? 0 : channels <= 6 ? 2 : (channels - 4);
   const int sideOffset = channels - 2;
   switch (me->m_audioPlayer->GetAudioSpecOutput().format)
   {
   case SDL_AUDIO_S16LE:
   {
      int16_t* const samples = static_cast<int16_t*>(stream);
      const int total_samples = len / (int)sizeof(int16_t);
      for (int index = 0; index < total_samples; index += channels)
      {
         const float leftSample  = static_cast<float>(samples[index    ]); // FL
         const float rightSample = static_cast<float>(samples[index + 1]); // FR
         samples[index + sideOffset    ] = static_cast<int16_t>(leftSample  * levelSL);
         samples[index + sideOffset + 1] = static_cast<int16_t>(rightSample * levelSR);
         samples[index + backOffset    ] = static_cast<int16_t>(leftSample  * levelBL);
         samples[index + backOffset + 1] = static_cast<int16_t>(rightSample * levelBR);
         samples[index                 ] = static_cast<int16_t>(leftSample  * levelFL);
         samples[index              + 1] = static_cast<int16_t>(rightSample * levelFR);
      }
      break;
   }
   case SDL_AUDIO_F32LE:
   {
      float* const samples = static_cast<float*>(stream);
      const int total_samples = len / (int)sizeof(float);
      for (int index = 0; index < total_samples; index += channels)
      {
         const float leftSample  = samples[index    ]; // FL
         const float rightSample = samples[index + 1]; // FR
         samples[index + sideOffset    ] = leftSample  * levelSL;
         samples[index + sideOffset + 1] = rightSample * levelSR;
         samples[index + backOffset    ] = leftSample  * levelBL;
         samples[index + backOffset + 1] = rightSample * levelBR;
         samples[index                 ] = leftSample  * levelFL;
         samples[index              + 1] = rightSample * levelFR;
      }
      break;
   }
   case SDL_AUDIO_S32LE:
   {
      int32_t* const samples = static_cast<int32_t*>(stream);
      const int total_samples = len / (int)sizeof(int32_t);
      for (int index = 0; index < total_samples; index += channels)
      {
         const float leftSample  = static_cast<float>(samples[index    ]); // FL
         const float rightSample = static_cast<float>(samples[index + 1]); // FR
         samples[index + sideOffset    ] = static_cast<int32_t>(leftSample  * levelSL);
         samples[index + sideOffset + 1] = static_cast<int32_t>(rightSample * levelSR);
         samples[index + backOffset    ] = static_cast<int32_t>(leftSample  * levelBL);
         samples[index + backOffset + 1] = static_cast<int32_t>(rightSample * levelBR);
         samples[index                 ] = static_cast<int32_t>(leftSample  * levelFL);
         samples[index              + 1] = static_cast<int32_t>(rightSample * levelFR);
      }
      break;
   }
   default:
      assert(false);
      PLOGE << "Unsupported audio format: " << me->m_audioPlayer->GetAudioSpecOutput().format;
      return;
   }
}

}
