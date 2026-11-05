// license:GPLv3+

#include "core/stdafx.h"
#include "MixNode.h"

namespace VPX
{

void MixToDeviceChannels(const SoundMixMode mode, const float volume, const float pan, const float rearFrontFade, const float* __restrict pFramesIn, const unsigned int nInChannels,
   float* __restrict pFramesOut, const unsigned int nOutChannels, const unsigned int count)
{
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

   // Backglass: downmix to the front left channel only
   if (mode == SoundMixMode::BackglassFrontLeft)
   {
      assert(nInChannels <= 2);
      const float weight = nInChannels > 1 ? volume * 0.5f : volume;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
         pFramesOut[0] = weight * (nInChannels > 1 ? pFramesIn[0] + pFramesIn[1] : pFramesIn[0]);
   }
   // Backglass: Stereo to mono
   else if (mode == SoundMixMode::Backglass && nOutChannels == 1)
   {
      assert(nInChannels <= 2);
      const float weight = nInChannels > 1 ? volume * 0.5f : volume;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
         pFramesOut[0] = weight * (nInChannels > 1 ? pFramesIn[0] + pFramesIn[1] : pFramesIn[0]);
   }
   // Backglass: Panned stereo to front channels (other channels are not used for backglass)
   else if (mode == SoundMixMode::Backglass)
   {
      assert(nInChannels <= 2);
      // This is a balanced panning (lower the opposite channel but does not increase nor blend channels). Not sure if this should be true panning (but is this really used ?)
      const float left = volume * (pan < 0 ? 1.f : 1.f - pan);
      const float right = volume * (pan > 0 ? 1.f : 1.f + pan);
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
      {
         if (nInChannels > 1)
         {
            pFramesOut[0] = left * pFramesIn[0];
            pFramesOut[1] = right * pFramesIn[1];
         }
         else
         {
            pFramesOut[0] = volume * pFramesIn[0];
            pFramesOut[1] = volume * pFramesIn[0];
         }
      }
   }
   // Playfield: front right channel only
   else if (mode == SoundMixMode::PlayfieldFrontRight)
   {
      assert(nInChannels == 1);
      const int rightCh = nOutChannels > 1 ? 1 : 0;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
         pFramesOut[rightCh] = volume * pFramesIn[0];
   }
   // Playfield: Mono mode (surprisingly this happens, for example with bluetooth LE devices)
   else if (nOutChannels == 1)
   {
      assert(nInChannels == 1);
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
         pFramesOut[0] = volume * pFramesIn[0];
   }
   // Playfield: Stereo mode
   else if (nOutChannels < 4 || (mode == SoundMixMode::PlayfieldNoFront && nOutChannels < 8))
   {
      assert(nInChannels == 1);
      const float left = volume * 0.5f * (1.f - pan);
      const float right = volume * 0.5f * (1.f + pan);
      const int leftCh = nOutChannels < 4 ? 0 : static_cast<int>(nOutChannels) - 2;
      const int rightCh = leftCh + 1;
      for (unsigned int i = 0; i < count; i++, pFramesIn += nInChannels, pFramesOut += nOutChannels)
      {
         const float sample = *pFramesIn;
         pFramesOut[leftCh] = left * sample;
         pFramesOut[rightCh] = right * sample;
      }
   }
   // Playfield: 4 channels mode
   else if (nOutChannels < 8 || (mode == SoundMixMode::PlayfieldNoFront && nOutChannels == 8))
   {
      assert(nInChannels == 1);
      const float left = volume * 0.5f * (1.f - pan);
      const float right = volume * 0.5f * (1.f + pan);
      const float front = 0.5f * (1.f + rearFrontFade);
      const float rear = 1.f - front;
      const float fl = front * left;
      const float fr = front * right;
      const float rl = rear * left;
      const float rr = rear * right;
      const int flCh = nOutChannels == 8 ? 4 : 0;
      const int frCh = flCh + 1;
      const int rlCh = nOutChannels == 8 ? 6 : static_cast<int>(nOutChannels) - 2;
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
      const float left = volume * 0.5f * (1.f - pan);
      const float right = volume * 0.5f * (1.f + pan);
      const float front = max(0.f, rearFrontFade);
      const float center = 1.f - fabsf(rearFrontFade);
      const float rear = max(0.f, -rearFrontFade);
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

}

static void vpx_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
   vpx_node* const pVPXNode = reinterpret_cast<vpx_node*>(pNode);
   VPX::MixToDeviceChannels(pVPXNode->mode, pVPXNode->volume, pVPXNode->pan, pVPXNode->rearFrontFade, ppFramesIn[0], ma_node_get_input_channels(pNode, 0), ppFramesOut[0],
      ma_node_get_output_channels(pNode, 0), *pFrameCountOut);
}

static ma_node_vtable vpx_node_vtable = { vpx_node_process_pcm_frames, nullptr, 1, 1, 0 };

ma_result vpx_node_init(ma_node_graph* pNodeGraph, unsigned int inChannels, unsigned int outChannels, const ma_allocation_callbacks* pAllocationCallbacks, vpx_node* pNode)
{
   if (pNode == nullptr)
      return MA_INVALID_ARGS;
   memset(pNode, 0, sizeof(vpx_node));
   ma_node_config baseConfig = ma_node_config_init();
   baseConfig.vtable = &vpx_node_vtable;
   baseConfig.pInputChannels = &inChannels;
   baseConfig.pOutputChannels = &outChannels;
   return ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pNode->baseNode);
}

void vpx_node_uninit(vpx_node* pNode, const ma_allocation_callbacks* pAllocationCallbacks)
{
   if (pNode == nullptr)
      return;
   ma_node_uninit(pNode, pAllocationCallbacks);
}
