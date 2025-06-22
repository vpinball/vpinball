// license:GPLv3+

#include "core/stdafx.h"
#include "SoundPlayer.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#include "miniaudio/miniaudio.h"




// Simple SSF processing: output according to table author supplied left/right & rear/front setup against user speaker layout setting

typedef struct
{
   ma_node_config nodeConfig;
   ma_uint32 channels;
} ssf_node_config;

typedef struct ssf_node ssf_node;
struct ssf_node
{
   ma_node_base baseNode;
   float volume;
   float pan;
   float rearFrontFade;
};

static void ssf_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
   const float* pFramesIn = ppFramesIn[0]; // Input bus @ index 0.
   float* pFramesOut = ppFramesOut[0]; // Output bus @ index 0.
   ssf_node* const pSSFNode = (ssf_node*)pNode;
   const ma_uint32 nInChannels = ma_node_get_input_channels(pNode, 0);
   const ma_uint32 nOutChannels = ma_node_get_output_channels(pNode, 0);
   assert(nInChannels == 1);

   // Supposed channel layout (taken from SDL docs):
   // 1 channel (mono) layout: FRONT
   // 2 channels (stereo) layout: FL, FR
   // 3 channels (2.1) layout: FL, FR, LFE
   // 4 channels (quad) layout: FL, FR, BL, BR
   // 5 channels (4.1) layout: FL, FR, LFE, BL, BR
   // 6 channels (5.1) layout: FL, FR, FC, LFE, BL, BR (last two can also be SL, SR)
   // 7 channels (6.1) layout: FL, FR, FC, LFE, BC, SL, SR
   // 8 channels (7.1) layout: FL, FR, FC, LFE, BL, BR, SL, SR
   //
   // Where:
   // FRONT = single mono speaker
   // FL = front left speaker
   // FR = front right speaker
   // FC = front center speaker
   // BL = back left speaker
   // BR = back right speaker
   // SR = surround right speaker
   // SL = surround left speaker
   // BC = back center speaker
   // LFE = low-frequency speaker
   //
   // FL/FR are always backglass only (except for 2 channel layouts obviously)

   const float left = pSSFNode->volume * 0.5f * (1.f - pSSFNode->pan);
   const float right = pSSFNode->volume * 0.5f * (1.f + pSSFNode->pan);
   const float front = 0.5f * (1.f + pSSFNode->rearFrontFade);
   const float rear = 1.f - front;
   const float fl = front * left;
   const float fr = front * right;
   const float rl = rear * left;
   const float rr = rear * right;

   const unsigned int count = *pFrameCountOut;

   for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
   {
      const float sample = *pFramesIn;
      switch (nOutChannels)
      {
      case 2: // Stereo output: just mono to left/right pan
         pFramesOut[0] = left  * sample; // FL
         pFramesOut[1] = right * sample; // FR
         break;
      case 3:
         pFramesOut[0] = left  * sample; // FL
         pFramesOut[1] = right * sample; // FR
         pFramesOut[2] = 0.f;            // LFE
         break;
      case 4:
         pFramesOut[0] = 0.f;            // FL
         pFramesOut[1] = 0.f;            // FR
         pFramesOut[2] = left  * sample; // BL
         pFramesOut[3] = right * sample; // BR
         break;
      case 5:
         pFramesOut[0] = 0.f;            // FL
         pFramesOut[1] = 0.f;            // FR
         pFramesOut[2] = 0.f;            // LFE
         pFramesOut[3] = left  * sample; // BL
         pFramesOut[4] = right * sample; // BR
         break;
      case 6:
         pFramesOut[0] = 0.f;            // FL
         pFramesOut[1] = 0.f;            // FR
         pFramesOut[2] = 0.f;            // FC
         pFramesOut[3] = 0.f;            // LFE
         pFramesOut[4] = left  * sample; // BL or SL
         pFramesOut[5] = right * sample; // BR or SR
         break;
      case 7:
         pFramesOut[0] = 0.f;            // FL
         pFramesOut[1] = 0.f;            // FR
         pFramesOut[2] = 0.f;            // FC
         pFramesOut[3] = 0.f;            // LFE
         pFramesOut[5] = 0.f;            // BC
         pFramesOut[6] = left  * sample; // SL
         pFramesOut[7] = right * sample; // SR
         break;
      case 8:
         pFramesOut[0] = 0.f;            // FL
         pFramesOut[1] = 0.f;            // FR
         pFramesOut[2] = 0.f;            // FC
         pFramesOut[3] = 0.f;            // LFE
         pFramesOut[4] = fl * sample;    // BL
         pFramesOut[5] = fr * sample;    // BR
         pFramesOut[6] = rl * sample;    // SL
         pFramesOut[7] = rr * sample;    // SR
         break;
      default: assert(false);
      }
   }
}

static ma_node_vtable ssf_node_vtable = { ssf_node_process_pcm_frames, NULL, 1, 1, 0 };

MA_API ma_result ssf_node_init(ma_node_graph* pNodeGraph, const ssf_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ssf_node* pNode)
{
   ma_result result;
   ma_node_config baseConfig;
   if (pNode == NULL)
   {
      return MA_INVALID_ARGS;
   }
   memset(pNode, 0, sizeof(ssf_node));
   baseConfig = pConfig->nodeConfig;
   baseConfig.vtable = &ssf_node_vtable;
   static ma_uint32 nInputChannel[] = { 1 };
   baseConfig.pInputChannels = nInputChannel;
   baseConfig.pOutputChannels = &pConfig->channels;
   result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pNode->baseNode);
   if (result != MA_SUCCESS)
   {
      return result;
   }
   return result;
}

MA_API void ssf_node_uninit(ma_delay_node* pNode, const ma_allocation_callbacks* pAllocationCallbacks)
{
   if (pNode == NULL)
   {
      return;
   }
   ma_node_uninit(pNode, pAllocationCallbacks);
}







namespace VPX
{

SoundPlayer* SoundPlayer::Create(const AudioPlayer* audioPlayer, Sound* sound)
{
   // Decode and resample the sound on the anciliary thread as this is fairly heavy
   return new SoundPlayer(audioPlayer, sound);
}

SoundPlayer* SoundPlayer::Create(const AudioPlayer* audioPlayer, string filename)
{
   // Decode and resample the sound on the anciliary thread as this is fairly heavy
   return new SoundPlayer(audioPlayer, filename);
}

SoundPlayer::SoundPlayer(const AudioPlayer* audioPlayer, string filename)
   : m_audioPlayer(audioPlayer)
   , m_outputTarget(SoundOutTypes::SNDOUT_BACKGLASS)
   , m_commandQueue(1)
{
   m_commandQueue.enqueue([this, filename]()
   {
      m_sound = std::make_unique<ma_sound>();
      if (ma_sound_init_from_file(m_audioPlayer->GetEngine(), filename.c_str(), MA_SOUND_FLAG_STREAM, nullptr, nullptr, m_sound.get()))
      {
         m_decoder = nullptr;
         m_sound = nullptr;
         return;
      }
      ma_sound_set_spatialization_enabled(m_sound.get(), (m_outputTarget != SNDOUT_BACKGLASS && m_audioPlayer->GetSoundMode3D() != SNDCFG_SND3D2CH) ? MA_TRUE : MA_FALSE);
      ma_sound_set_end_callback(m_sound.get(), OnSoundEnd, this);
   });
}

SoundPlayer::SoundPlayer(const AudioPlayer* audioPlayer, Sound* sound)
   : m_audioPlayer(audioPlayer)
   , m_outputTarget(sound->GetOutputTarget())
   , m_commandQueue(1)
{
   m_commandQueue.enqueue([this, sound]()
   {
      ma_engine* engine = m_audioPlayer->GetEngine();
      ma_result result;

      // Playfield sound goes through a custom effect to dispatch the sound across the playfield speaker channels (otherwise, just expand to stereo and play on front speakers)
      if (m_outputTarget != SNDOUT_BACKGLASS)
      {
         m_ssfEffect = std::make_unique<ssf_node>();
         ssf_node_config customNodeConfig;
         customNodeConfig.nodeConfig = ma_node_config_init();
         customNodeConfig.channels = ma_engine_get_channels(engine);
         result = ssf_node_init(ma_engine_get_node_graph(engine), &customNodeConfig, nullptr, m_ssfEffect.get());
         if (result != MA_SUCCESS)
         {
            PLOGE << "Failed to initialize SSF effect.";
            m_ssfEffect = nullptr;
         }
         else
         {
            ma_node_attach_output_bus(m_ssfEffect.get(), 0, ma_engine_get_endpoint(engine), 0);
         }
      }

      // Setup to:
      // . decode and convert playfield sounds to a mono channel
      // . decode and convert backglass sound to their native encoding with zeroed out additional channels
      // TODO we should convert mono backglass stream to stereo streams with zeroed out additional channels

      m_decoder = std::make_unique<ma_decoder>();
      ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_unknown,  m_ssfEffect ? 1 : 0, 0);
      decoderConfig.channelMixMode = ma_channel_mix_mode_simple;
      if (ma_decoder_init_memory(sound->GetFileRaw(), sound->GetFileSize(), &decoderConfig, m_decoder.get()) != MA_SUCCESS)
      {
         m_decoder = nullptr;
         return;
      }
      m_sound = std::make_unique<ma_sound>();
      ma_sound_config config = ma_sound_config_init_2(engine);
      config.channelsOut = m_ssfEffect ? 1 : 0;
      config.pDataSource = m_decoder.get();
      config.monoExpansionMode = ma_mono_expansion_mode_stereo_only;
      config.endCallback = OnSoundEnd;
      config.pEndCallbackUserData = this;
      config.flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
      config.pInitialAttachment = m_ssfEffect.get();

      if (ma_sound_init_ex(engine, &config, m_sound.get()))
      {
         m_decoder = nullptr;
         m_sound = nullptr;
         return;
      }
   });
}

SoundPlayer::~SoundPlayer()
{
   m_commandQueue.wait_until_empty();
   m_commandQueue.wait_until_nothing_in_flight();
   if (m_sound)
   {
      ma_sound_stop(m_sound.get());
      ma_sound_uninit(m_sound.get());
   }
   if (m_decoder)
      ma_decoder_uninit(m_decoder.get());
   if (m_ssfEffect)
      ma_node_uninit(m_ssfEffect.get(), nullptr);
}

void SoundPlayer::SetMainVolume(float backglassVolume, float playfieldVolume)
{
   m_mainVolume = m_outputTarget == SNDOUT_BACKGLASS ? backglassVolume : playfieldVolume;
   m_commandQueue.enqueue([this]() { ApplyVolume(); });
}

void SoundPlayer::SetVolume(float volume) {
   m_soundVolume = volume;
   m_commandQueue.enqueue([this]() { ApplyVolume(); });
}


void SoundPlayer::ApplyVolume()
{
   if (m_sound)
   {
      const float totalvolume = clamp(m_soundVolume * m_mainVolume, 0.0f, 1.0f);
      // VP legacy conversion:
      // const float decibelvolume = (totalvolume == 0.0f) ? -100.f : max(logf(totalvolume) * (float)(10.0 / log(10.0)) - 20.0f, -100.f);
      // const float decibelvolume = logf(totalvolume) * (float)(10.0 / log(10.0)) - 20.0f; // as we don't need to handle silence separately with linear volume
      // const float decibelvolume = 10.f * log10f(totalvolume) - 20.f;
      // const float linearvolume = powf(10.f, 10.f * log10f(totalvolume) / 20.f - 1.f); // since linear = powf(10.f, decibel gain / 20.f)
      // const float linearvolume = powf(10.f, log10f(sqrt(totalvolume)) - 1.f);
      // const float linearvolume = sqrt(totalvolume) / 10.f; // we don't keep the 1/10 factor as this is better placed as part of the main volume mixer setup (this create a setup regression when updating from 10.8 to later version)
      if (m_ssfEffect)
         m_ssfEffect->volume = sqrtf(totalvolume);
      else
         ma_sound_set_volume(m_sound.get(), sqrtf(totalvolume));
   }
}

void SoundPlayer::Play(float volume, const float randompitch, const int pitch, float pan, float frontRearFade, const int loopcount)
{
   m_commandQueue.enqueue([this, volume, randompitch, pitch, pan, frontRearFade, loopcount]()
   {
      if (m_sound == nullptr)
         return;

      if (m_outputTarget == SNDOUT_BACKGLASS)
      {
         // Simply output to the 2 first channels (front left/right) for backglass or no spatial sound mode
         ma_sound_set_pan(m_sound.get(), pan);
      }
      else if (m_ssfEffect)
      {
         // Diffuse the (mono) playfield sound to 2 or 4 speakers spread on the playfield
         // This is designed to support existing tables which appends to apply x^10 to pan and front/rear fade, so we have to undo it
         m_ssfEffect->pan = clamp(pan, -1.f, 1.f);
         m_ssfEffect->pan = (m_ssfEffect->pan < 0.0f) ? -powf(-m_ssfEffect->pan, 0.1f) : powf(m_ssfEffect->pan, 0.1f);

         switch (m_audioPlayer->GetSoundMode3D())
         {
         case SNDCFG_SND3D2CH: break;
         case SNDCFG_SND3DALLREAR: m_ssfEffect->rearFrontFade = 1.f; break;
         case SNDCFG_SND3DFRONTISFRONT: // Needs more explanation on what it is supposed to do, so fallback to SSF
         case SNDCFG_SND3DFRONTISREAR: // Needs more explanation on what it is supposed to do, so fallback to SSF
         case SNDCFG_SND3D6CH: // Not clear what would be the correct way of porting this (use a less effective pan & rearfade ?), so fallback to SSF
         case SNDCFG_SND3DSSF:
            m_ssfEffect->rearFrontFade = clamp(frontRearFade, -1.f, 1.f);
            m_ssfEffect->rearFrontFade = (m_ssfEffect->rearFrontFade < 0.0f) ? -powf(-m_ssfEffect->rearFrontFade, 0.1f) : powf(m_ssfEffect->rearFrontFade, 0.1f);
            break;
         default: assert(false); return;
         }
         //case SNDCFG_SND3D2CH: ma_sound_set_pan(m_sound.get(), pan); break;
         //case SNDCFG_SND3DALLREAR: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -PanTo3D(1.0f)); break;
         //case SNDCFG_SND3DFRONTISFRONT: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, PanTo3D(frontRearFade)); break;
         //case SNDCFG_SND3DFRONTISREAR: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -PanTo3D(frontRearFade)); break;
         //case SNDCFG_SND3D6CH: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -(PanTo3D(frontRearFade) + 3.f) / 2.f); break;
         //case SNDCFG_SND3DSSF: ma_sound_set_position(m_sound.get(), PanSSF(pan), 0.0f, -FadeSSF(frontRearFade)); break;
      }
      else
      {
         // TODO implement spatialization (especially with binauralization for stereo / headset / VR play), using something like https://github.com/videolabs/libspatialaudio
         assert(false);
      }

      m_loopCount = loopcount;

      ma_format format;
      ma_uint32 channels;
      ma_uint32 sampleRate;
      ma_sound_get_data_format(m_sound.get(), &format, &channels, &sampleRate, nullptr, 0);
      const float sampleFreq = static_cast<float>(sampleRate);
      float newFreq = sampleFreq + pitch;
      if (randompitch > 0.f)
      {
         const float rndh = rand_mt_01();
         const float rndl = rand_mt_01();
         newFreq *= 1.f + (randompitch * rndh * rndh) - (randompitch * rndl * rndl * 0.5f);
      }
      ma_sound_set_pitch(m_sound.get(), newFreq / sampleFreq);

      m_soundVolume = volume;
      ApplyVolume();

      if (!IsPlaying())
         ma_sound_start(m_sound.get());
   });
}

void SoundPlayer::Pause()
{
   m_commandQueue.enqueue([this]()
   {
      if (m_sound)
         ma_sound_stop(m_sound.get());
   });
}

void SoundPlayer::Unpause()
{
   m_commandQueue.enqueue([this]()
   {
      if (m_sound)
         ma_sound_start(m_sound.get());
   });
}

void SoundPlayer::Stop()
{
   m_commandQueue.enqueue([this]()
   {
      if (m_sound)
         ma_sound_stop(m_sound.get());
   });
}

float SoundPlayer::GetPosition()
{
   float pos = 0.f;
   if (ma_sound_get_cursor_in_seconds(m_sound.get(), &pos) == MA_SUCCESS)
      return pos;
   return 0.f;
}

void SoundPlayer::SetPosition(float pos)
{
   m_commandQueue.enqueue([this, pos]()
   {
      if (m_sound)
         ma_sound_seek_to_second(m_sound.get(), pos);
   });
}

bool SoundPlayer::IsPlaying() const
{
   return m_sound && ma_sound_is_playing(m_sound.get());
}

SoundSpec SoundPlayer::GetInformations() const
{
   SoundSpec specs { 0 };
   m_commandQueue.wait_until_empty();
   m_commandQueue.wait_until_nothing_in_flight();
   if (m_sound == nullptr)
      return specs;
   ma_format format;
   ma_uint32 channels;
   ma_uint32 sampleRate;
   float length;
   ma_sound_get_length_in_seconds(m_sound.get(), &length);
   ma_sound_get_data_format(m_sound.get(), &format, &channels, &sampleRate, nullptr, 0);
   specs.lengthInSeconds = length;
   specs.nChannels = channels;
   specs.sampleFrequency = sampleRate;
   return specs;
}

void SoundPlayer::OnSoundEnd(void* pUserData, ma_sound* pSound)
{
   SoundPlayer* me = static_cast<SoundPlayer*>(pUserData);
   if (me->m_loopCount == 0)
      return;
   if (me->m_loopCount > 0)
      me->m_loopCount--;
   // Dispatch through the command queue since we can not restart the sound from the callback as the sound is still playing and command would be discarded
   me->m_commandQueue.enqueue([pSound]() { ma_sound_start(pSound); });
}

}
