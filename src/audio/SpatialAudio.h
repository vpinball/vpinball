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
//   a second order B-format bus, then binauralizing them to the 2 first output
//   channels (VR headset).
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

class SpatialAudioMixer
{
public:
   // Builds a mixer node attached to the engine endpoint, binauralizing all its
   // sources to the 2 first output channels. May return a mixer even on
   // failure, the node then simply stays silent.
   SpatialAudioMixer(ma_engine* engine);
   ~SpatialAudioMixer();

   // Node to attach SpatialAudioSource outputs to (B-format input bus).
   // Returns a ma_node* (declared as void* to avoid including miniaudio.h here)
   void* GetMixBus() const;
   SpatialAudioListener* GetListener() { return &m_listener; }
   // Canonical listener pose (the fixed fake pose the tables' loudness is
   // authored for), used as the reference for distance attenuation
   SpatialAudioListener* GetReferenceListener() { return &m_referenceListener; }

   // Opaque implementation, defined in the .cpp (publicly visible for the node struct helpers)
   struct Impl;

private:
   SpatialAudioListener m_listener;
   SpatialAudioListener m_referenceListener;
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

   // Source position in table units: the emitter point for mono sources; for
   // stereo sources both channels emit from that point.
   void SetPosition(float x, float y, float z);
   // Explicit emitter position of each channel in table units (stereo sources
   // only, e.g. the 2 backglass speakers). Overrides SetPosition.
   void SetChannelPositions(float xLeft, float yLeft, float zLeft, float xRight, float yRight, float zRight);
   void SetVolume(float volume);
   // Balance between left and right channels for stereo sources, -1..1 (legacy backglass pan).
   void SetChannelBalance(float balance);

   // Opaque implementation, defined in the .cpp (publicly visible for the node struct helpers)
   struct Impl;

private:
   std::unique_ptr<Impl> m_impl;
   void* m_node = nullptr;
};

}
