// license:GPLv3+

#include "core/stdafx.h"
#include "SoundPlayer.h"

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#include "miniaudio/miniaudio.h"




// Simple custom node:
// - SSF processing: output according to table author supplied left/right & rear/front setup against user speaker layout setting
// - Backglass processing: expand channels

typedef struct
{
   ma_node_config nodeConfig;
   ma_uint32 inChannels;
   ma_uint32 outChannels;
} vpx_node_config;

typedef struct vpx_node vpx_node;
struct vpx_node
{
   ma_node_base baseNode;
   float volume;
   float pan;
   float rearFrontFade; // SSF only
   enum Mode
   {
      PF,          // Mix mono sound to playfield device channels
      PF_NO_FRONT, // Mix mono sound to playfield device channels, keeping front channels free
      BG           // Mix stereo sound to backglass channels
   } mode;
};

static void vpx_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
   const float* __restrict pFramesIn = ppFramesIn[0]; // Input bus @ index 0.
   float* __restrict pFramesOut = ppFramesOut[0]; // Output bus @ index 0.
   vpx_node* const pVPXNode = reinterpret_cast<vpx_node*>(pNode);
   const ma_uint32 nInChannels = ma_node_get_input_channels(pNode, 0);
   const ma_uint32 nOutChannels = ma_node_get_output_channels(pNode, 0);
   const unsigned int count = *pFrameCountOut;
   if (nOutChannels > 2)
      memset(pFramesOut, 0, count * nOutChannels * sizeof(float));

   // SDL audio channel layout is the following:
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
   // FL/FR = front speakers
   // FC = front center speaker
   // BL/BR = back speakers
   // SR/SL = surround speakers
   // BC = back center speaker
   // LFE = low-frequency speaker

   // Backglass: Stereo to mono
   if (pVPXNode->mode == vpx_node::Mode::BG && nOutChannels == 1)
   {
      assert(nInChannels == 2);
      const float weight = pVPXNode->volume * 0.5f;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
         pFramesOut[0] = weight * (pFramesIn[0] + pFramesIn[1]);
   }
   // Backglass: Panned stereo to front channels (other channels are not used for backglass)
   else if (pVPXNode->mode == vpx_node::Mode::BG)
   {
      assert(nInChannels == 2);
      // This is a balanced panning (lower the opposite channel but does not increase nor blend channels). Not sure if this should be true panning (but is this really used ?)
      const float left = pVPXNode->volume * (pVPXNode->pan < 0 ? 1.f : 1.f - pVPXNode->pan);
      const float right = pVPXNode->volume * (pVPXNode->pan > 0 ? 1.f : 1.f + pVPXNode->pan);
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
      {
         pFramesOut[0] = left * pFramesIn[0];
         pFramesOut[1] = right * pFramesIn[1];
      }
   }
   // Playfield: Mono mode (surprisingly this happens, for example with bluetooth LE devices)
   else if (nOutChannels == 1)
   {
      assert(nInChannels == 1);
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
         pFramesOut[0] = pVPXNode->volume * pFramesIn[0];
   }
   // Playfield: Stereo mode
   else if (nOutChannels < 4 || (pVPXNode-> mode == vpx_node::Mode::PF_NO_FRONT && nOutChannels < 8))
   {
      assert(nInChannels == 1);
      const float left = pVPXNode->volume * 0.5f * (1.f - pVPXNode->pan);
      const float right = pVPXNode->volume * 0.5f * (1.f + pVPXNode->pan);
      const int leftCh = nOutChannels < 4 ? 0 : nOutChannels - 2;
      const int rightCh = leftCh + 1;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
      {
         const float sample = *pFramesIn;
         pFramesOut[leftCh] = left * sample;
         pFramesOut[rightCh] = right * sample;
      }
   }
   // Playfield: 4 channels mode
   else if (nOutChannels < 8 || (pVPXNode->mode == vpx_node::Mode::PF_NO_FRONT && nOutChannels == 8))
   {
      assert(nInChannels == 1);
      const float left = pVPXNode->volume * 0.5f * (1.f - pVPXNode->pan);
      const float right = pVPXNode->volume * 0.5f * (1.f + pVPXNode->pan);
      const float front = 0.5f * (1.f + pVPXNode->rearFrontFade);
      const float rear = 1.f - front;
      const float fl = front * left;
      const float fr = front * right;
      const float rl = rear * left;
      const float rr = rear * right;
      const int flCh = nOutChannels == 8 ? 4 : 0;
      const int frCh = flCh + 1;
      const int rlCh = nOutChannels == 8 ? 6 : nOutChannels - 2;
      const int rrCh = rlCh + 1;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
      {
         const float sample = *pFramesIn;
         pFramesOut[flCh] = fl * sample;
         pFramesOut[frCh] = fr * sample;
         pFramesOut[rlCh] = rl * sample;
         pFramesOut[rrCh] = rr * sample;
      }
   }
   // Playfield: 6 channels mode
   else if (nOutChannels == 8)
   {
      assert(nInChannels == 1);
      const float left = pVPXNode->volume * 0.5f * (1.f - pVPXNode->pan);
      const float right = pVPXNode->volume * 0.5f * (1.f + pVPXNode->pan);
      const float front = max(0.f, pVPXNode->rearFrontFade);
      const float center = abs(pVPXNode->rearFrontFade);
      const float rear = max(0.f, -pVPXNode->rearFrontFade);
      const float fl = front * left;
      const float fr = front * right;
      const float cl = center * left;
      const float cr = center * right;
      const float rl = rear * left;
      const float rr = rear * right;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
      {
         const float sample = *pFramesIn;
         pFramesOut[0] = fl * sample;
         pFramesOut[1] = fr * sample;
         pFramesOut[4] = cl * sample;
         pFramesOut[5] = cr * sample;
         pFramesOut[6] = rl * sample;
         pFramesOut[7] = rr * sample;
      }
   }
}

static ma_node_vtable vpx_node_vtable = { vpx_node_process_pcm_frames, nullptr, 1, 1, 0 };

MA_API ma_result vpx_node_init(ma_node_graph* pNodeGraph, const vpx_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, vpx_node* pNode)
{
   ma_node_config baseConfig;
   if (pNode == nullptr)
      return MA_INVALID_ARGS;
   memset(pNode, 0, sizeof(vpx_node));
   baseConfig = pConfig->nodeConfig;
   baseConfig.vtable = &vpx_node_vtable;
   baseConfig.pInputChannels = &pConfig->inChannels;
   baseConfig.pOutputChannels = &pConfig->outChannels;
   return ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pNode->baseNode);
}

MA_API void vpx_node_uninit(ma_delay_node* pNode, const ma_allocation_callbacks* pAllocationCallbacks)
{
   if (pNode == nullptr)
      return;
   ma_node_uninit(pNode, pAllocationCallbacks);
}







namespace VPX
{

SoundPlayer* SoundPlayer::Create(const AudioPlayer* audioPlayer, Sound* sound)
{
   // Decode and resample the sound on the anciliary thread as this is fairly heavy
   return new SoundPlayer(audioPlayer, sound);
}

SoundPlayer* SoundPlayer::Create(const AudioPlayer* audioPlayer, const string& filename)
{
   // Decode and resample the sound on the anciliary thread as this is fairly heavy
   return new SoundPlayer(audioPlayer, filename);
}

SoundPlayer::SoundPlayer(const AudioPlayer* audioPlayer, const string& filename)
   : m_audioPlayer(audioPlayer)
   , m_outputTarget(SoundOutTypes::SNDOUT_BACKGLASS)
   , m_commandQueue(1)
{
   m_commandQueue.enqueue([this, filename]()
   {
      SetThreadName("VPX.SoundPlayer ["s.append(filename).append(1, ']'));

      ma_engine* engine = m_audioPlayer->GetEngine(m_outputTarget);

      // Add custom node for channel mixing
      m_vpxMixNode = std::make_unique<vpx_node>();
      vpx_node_config customNodeConfig;
      customNodeConfig.nodeConfig = ma_node_config_init();
      customNodeConfig.inChannels = 2;
      customNodeConfig.outChannels = ma_engine_get_channels(engine);
      const ma_result result = vpx_node_init(ma_engine_get_node_graph(engine), &customNodeConfig, nullptr, m_vpxMixNode.get());
      if (result != MA_SUCCESS)
      {
         PLOGE << "Failed to initialize custom mixer.";
         m_vpxMixNode = nullptr;
         return;
      }
      m_vpxMixNode->mode = m_outputTarget == SNDOUT_BACKGLASS                                                         ? vpx_node::Mode::BG          // Output to stereo
         : (m_audioPlayer->GetSoundMode3D() == SNDCFG_SND3D6CH || m_audioPlayer->GetSoundMode3D() == SNDCFG_SND3DSSF) ? vpx_node::Mode::PF_NO_FRONT // Output to multi channel, keeping front empty
                                                                                                                      : vpx_node::Mode::PF;         // Output to multi channel
      ma_node_attach_output_bus(m_vpxMixNode.get(), 0, ma_engine_get_endpoint(engine), 0);

      m_sound = std::make_unique<ma_sound>();
      ma_sound_config config = ma_sound_config_init_2(engine);
      config.pFilePath = filename.c_str();
      config.channelsOut = 2;
      config.monoExpansionMode = ma_mono_expansion_mode_stereo_only;
      config.endCallback = OnSoundEnd;
      config.pEndCallbackUserData = this;
      config.flags = MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_STREAM;
      config.pInitialAttachment = m_vpxMixNode.get();
      if (ma_sound_init_ex(engine, &config, m_sound.get()))
      {
         m_decoder = nullptr;
         m_sound = nullptr;
         return;
      }

      if (!IsPlaying())
         ma_sound_start(m_sound.get());
   });
}

SoundPlayer::SoundPlayer(const AudioPlayer* audioPlayer, Sound* sound)
   : m_audioPlayer(audioPlayer)
   , m_outputTarget(sound->GetOutputTarget())
   , m_commandQueue(1)
{
   m_commandQueue.enqueue([this, sound]()
   {
      SetThreadName("VPX.SoundPlayer ["s.append(sound->GetName()).append(1, ']'));

      ma_engine* engine = m_audioPlayer->GetEngine(m_outputTarget);

      // Add custom node for channel mixing
      m_vpxMixNode = std::make_unique<vpx_node>();
      vpx_node_config customNodeConfig;
      customNodeConfig.nodeConfig = ma_node_config_init();
      customNodeConfig.inChannels = m_outputTarget == SNDOUT_BACKGLASS ? 2 : 1;
      customNodeConfig.outChannels = ma_engine_get_channels(engine);
      const ma_result result = vpx_node_init(ma_engine_get_node_graph(engine), &customNodeConfig, nullptr, m_vpxMixNode.get());
      if (result != MA_SUCCESS)
      {
         PLOGE << "Failed to initialize custom mixer.";
         m_vpxMixNode = nullptr;
         return;
      }
      m_vpxMixNode->mode = m_outputTarget == SNDOUT_BACKGLASS                                                         ? vpx_node::Mode::BG          // Output to stereo
         : (m_audioPlayer->GetSoundMode3D() == SNDCFG_SND3D6CH || m_audioPlayer->GetSoundMode3D() == SNDCFG_SND3DSSF) ? vpx_node::Mode::PF_NO_FRONT // Output to multi channel, keeping front empty
                                                                                                                      : vpx_node::Mode::PF;         // Output to multi channel
      ma_node_attach_output_bus(m_vpxMixNode.get(), 0, ma_engine_get_endpoint(engine), 0);

      // Setup to:
      // . decode and convert playfield sounds to a mono channel
      // . decode and convert backglass sound to their native encoding with zeroed out additional channels
      // TODO we should convert mono backglass stream to stereo streams with zeroed out additional channels
      m_decoder = std::make_unique<ma_decoder>();
      ma_decoder_config decoderConfig = ma_decoder_config_init(ma_format_unknown, m_outputTarget == SNDOUT_BACKGLASS ? 2 : 1, 0);
      decoderConfig.channelMixMode = ma_channel_mix_mode_simple;
      if (ma_decoder_init_memory(sound->GetFileRaw(), sound->GetFileSize(), &decoderConfig, m_decoder.get()) != MA_SUCCESS)
      {
         m_decoder = nullptr;
         return;
      }
      m_monoCompensation = static_cast<float>(m_decoder->outputChannels); // When converting to mono, we average additional channel instead of summing them as this used to be done before, so multiply back

      m_sound = std::make_unique<ma_sound>();
      ma_sound_config config = ma_sound_config_init_2(engine);
      config.channelsOut = m_outputTarget == SNDOUT_BACKGLASS ? 2 : 1;
      config.pDataSource = m_decoder.get();
      config.monoExpansionMode = ma_mono_expansion_mode_duplicate;
      config.endCallback = OnSoundEnd;
      config.pEndCallbackUserData = this;
      config.flags = MA_SOUND_FLAG_NO_SPATIALIZATION;
      config.pInitialAttachment = m_vpxMixNode.get();

      if (ma_sound_init_ex(engine, &config, m_sound.get()))
      {
         m_decoder = nullptr;
         m_sound = nullptr;
         return;
      }

      if (!IsPlaying())
         ma_sound_start(m_sound.get());
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
   if (m_vpxMixNode)
      ma_node_uninit(m_vpxMixNode.get(), nullptr);
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
      m_vpxMixNode->volume = m_monoCompensation * sqrtf(totalvolume);
   }
}

void SoundPlayer::Play(float volume, const float randompitch, const int pitch, float pan, float frontRearFade, const int loopcount)
{
   m_commandQueue.enqueue([this, volume, randompitch, pitch, pan, frontRearFade, loopcount]()
   {
      if (m_sound == nullptr)
         return;

      // TODO implement spatialization (especially with binauralization for stereo / headset / VR play), using something like https://github.com/videolabs/libspatialaudio

      // This is designed to support existing tables which appends to apply x^10 to pan and front/rear fade, so we have to undo it
      m_vpxMixNode->pan = clamp(pan, -1.f, 1.f);
      m_vpxMixNode->pan = (m_vpxMixNode->pan < 0.0f) ? -powf(-m_vpxMixNode->pan, 0.1f) : powf(m_vpxMixNode->pan, 0.1f);
      if (m_outputTarget == SNDOUT_TABLE)
      {
         // Diffuse the (mono) playfield sound to 2 or 4 speakers spread on the playfield
         m_vpxMixNode->rearFrontFade = clamp(frontRearFade, -1.f, 1.f);
         m_vpxMixNode->rearFrontFade = (m_vpxMixNode->rearFrontFade < 0.0f) ? -powf(-m_vpxMixNode->rearFrontFade, 0.1f) : powf(m_vpxMixNode->rearFrontFade, 0.1f);
         switch (m_audioPlayer->GetSoundMode3D())
         {
         case SNDCFG_SND3D2CH: m_vpxMixNode->rearFrontFade = 0.f; break; // Stereo output to front channels
         case SNDCFG_SND3DALLREAR: m_vpxMixNode->rearFrontFade = 1.f; break; // Stereo output to rear channels
         case SNDCFG_SND3DFRONTISFRONT: break;
         case SNDCFG_SND3DFRONTISREAR: m_vpxMixNode->rearFrontFade = -m_vpxMixNode->rearFrontFade; break; // Reversed channel orientation
         case SNDCFG_SND3D6CH: break; // Keep front empty (performed when mixing) Not clear what would be the correct way of porting this (use a less effective pan & rearfade ?), so fallback to SSF
         case SNDCFG_SND3DSSF: break; // Keep front empty (performed when mixing)
         default: assert(false); return;
         }
         //Legacy hacked 3d audio, for reference:
         //case SNDCFG_SND3D2CH: ma_sound_set_pan(m_sound.get(), pan); break;
         //case SNDCFG_SND3DALLREAR: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -PanTo3D(1.0f)); break;
         //case SNDCFG_SND3DFRONTISFRONT: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, PanTo3D(frontRearFade)); break;
         //case SNDCFG_SND3DFRONTISREAR: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -PanTo3D(frontRearFade)); break;
         //case SNDCFG_SND3D6CH: ma_sound_set_position(m_sound.get(), PanTo3D(pan), 0.0f, -(PanTo3D(frontRearFade) + 3.f) / 2.f); break;
         //case SNDCFG_SND3DSSF: ma_sound_set_position(m_sound.get(), PanSSF(pan), 0.0f, -FadeSSF(frontRearFade)); break;
      }

      m_loopCount = loopcount <  0 ? -1 // Negative value is loop indefenitely
                  : loopcount <= 1 ?  0 // 0 is no loop, and 1 is play once (so no loop either)
                  : (loopcount - 1);    // >= 2 is the number of times to play the sound, so loop once less as we are already playing once

      ma_format format;
      ma_uint32 channels;
      ma_uint32 sampleRate;
      ma_sound_get_data_format(m_sound.get(), &format, &channels, &sampleRate, nullptr, 0);
      const float sampleFreq = static_cast<float>(sampleRate);
      float newFreq = sampleFreq + (float)pitch;
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
      {
         ma_sound_stop(m_sound.get());
         ma_sound_seek_to_pcm_frame(m_sound.get(), 0);
      }
   });
}

float SoundPlayer::GetPosition() const
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
