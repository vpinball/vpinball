// license:GPLv3+

#pragma once

#define MA_ENABLE_ONLY_SPECIFIC_BACKENDS
#define MA_ENABLE_CUSTOM
#include "miniaudio/miniaudio.h"

namespace VPX
{

// Output routing applied by non spatial channel mixing node, derived from the playfield and
// backglass output mode settings (see SoundConfigTypes and BackglassConfigTypes)
enum class SoundMixMode
{
   Playfield, // Mix mono sound to the playfield device channels
   PlayfieldNoFront, // Mix mono sound to the playfield device channels, keeping the front pair free
   PlayfieldFrontRight, // Mix mono sound to the front right channel only
   Backglass, // Mix stereo sound to the front channels of the backglass device
   BackglassFrontLeft // Mix sound downmixed to the front left channel only
};

// Per-frame interleaved mixing applied by the vpx_node custom node, exposed for testing.
// framesOut is fully overwritten (silence on channels the mode does not drive).
// pan (-1..1) balances left/right, rearFrontFade (-1..1, playfield modes only) balances rear/front.
void MixToDeviceChannels(
   SoundMixMode mode, float volume, float pan, float rearFrontFade, const float* framesIn, unsigned int inChannels, float* framesOut, unsigned int outChannels, unsigned int frameCount);

}

// Simple custom node (used when spatial audio is disabled in the settings):
// - SSF processing: output according to table author supplied left/right & rear/front setup against user speaker layout setting
// - Backglass processing: expand channels
struct vpx_node
{
   ma_node_base baseNode;
   float volume;
   float pan;
   float rearFrontFade; // SSF only
   VPX::SoundMixMode mode;
};

ma_result vpx_node_init(ma_node_graph* pNodeGraph, unsigned int inChannels, unsigned int outChannels, const ma_allocation_callbacks* pAllocationCallbacks, vpx_node* pNode);
void vpx_node_uninit(vpx_node* pNode, const ma_allocation_callbacks* pAllocationCallbacks);
