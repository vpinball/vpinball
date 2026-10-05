// license:GPLv3+

#pragma once

#include <atomic>
#include <memory>
#include <vector>

struct ma_engine;

namespace VPX
{

// Spatial audio support, based on libspatialaudio:
// - SpatialAudioListener holds the listener pose in table space, shared between
//   all sources and mixers of a play session (atomics as it is updated from the
//   render/VR thread and read from the audio thread).
// - SpatialAudioMixer is a per audio engine node mixing all spatial sources in
//   a second order B-format bus, then decoding to the device loudspeaker layout
//   or binauralizing (headphones / VR).
// - SpatialAudioSource is a per sound encoder node, converting a mono or stereo
//   source to B-format according to its position relative to the listener.
//
// Positions are expressed in table units (VPU): x to the right (from player
// point of view), y toward the lockbar (front of the cabinet, where the player
// stands), z above the playfield surface. The listener faces the backglass.

class SpatialAudioListener
{
public:
   void SetPose(float x, float y, float z, float yaw, float pitch, float roll);
   void GetPose(float& x, float& y, float& z, float& yaw, float& pitch, float& roll) const;

private:
   std::atomic<float> m_x { 0.f };
   std::atomic<float> m_y { 0.f };
   std::atomic<float> m_z { 0.f };
   std::atomic<float> m_yaw { 0.f }; // rotation around the up axis, 0 = facing the backglass, positive = looking left
   std::atomic<float> m_pitch { 0.f }; // positive = looking up
   std::atomic<float> m_roll { 0.f }; // positive = tilting head to the right
};

// Loudspeaker position for the decoder layout, in spherical coordinates
// relative to the listener (degrees for angles, meters for the distance).
struct SpatialSpeakerPosition
{
   float azimuth; // positive = left
   float elevation; // positive = up
   float distance;
   bool active; // inactive channels are kept silent (e.g. front pair in 6CH/SSF mode, LFE)
};

class SpatialAudioMixer
{
public:
   // Builds a mixer node attached to the engine endpoint. speakerLayout gives
   // the polar position of each output channel of the audio device (in SDL
   // channel order). May return a mixer even on failure, the node then simply
   // stays silent.
   SpatialAudioMixer(ma_engine* engine, const std::vector<SpatialSpeakerPosition>& speakerLayout);
   ~SpatialAudioMixer();

   // Node to attach SpatialAudioSource outputs to (B-format input bus).
   // Returns a ma_node* (declared as void* to avoid including miniaudio.h here)
   void* GetMixBus() const;
   SpatialAudioListener* GetListener() { return &m_listener; }

   // Switch between loudspeaker decoding and binaural rendering (headphones, VR).
   void SetBinauralEnabled(bool enabled);

   // Loudspeaker layouts for the playfield output, according to the selected
   // SNDCFG_* mode and the device channel count (SDL channel order).
   static std::vector<SpatialSpeakerPosition> GetPlayfieldSpeakerLayout(int soundConfigMode, unsigned int nChannels);

   // Loudspeaker layout for the backglass output (speakers located in the backbox).
   static std::vector<SpatialSpeakerPosition> GetBackglassSpeakerLayout(unsigned int nChannels);

   // Opaque implementation, defined in the .cpp (publicly visible for the node struct helpers)
   struct Impl;

private:
   SpatialAudioListener m_listener;
   std::unique_ptr<Impl> m_impl;
};

class SpatialAudioSource
{
public:
   // inChannels is 1 for playfield sources (mono) or 2 for backglass sources (stereo).
   SpatialAudioSource(ma_engine* engine, SpatialAudioMixer* mixer, unsigned int inChannels);
   ~SpatialAudioSource();

   // Returns a ma_node* (declared as void* to avoid including miniaudio.h here)
   void* GetNode() const { return m_node; }

   // Source position in table units (playfield sounds).
   void SetPosition(float x, float y, float z);
   // Source position in listener relative spherical coordinates (backglass sounds), degrees and meters.
   void SetPositionPolar(float azimuth, float elevation, float distance);
   void SetVolume(float volume);
   // Balance between left and right channels for stereo sources, -1..1 (legacy backglass pan).
   void SetChannelBalance(float balance);
   // Distance at which no attenuation is applied (closer = louder, farther = quieter).
   void SetReferenceDistance(float distance);

   // Opaque implementation, defined in the .cpp (publicly visible for the node struct helpers)
   struct Impl;

private:
   std::unique_ptr<Impl> m_impl;
   void* m_node = nullptr;
};

}
