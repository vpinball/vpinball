// license:GPLv3+

#pragma once

struct ma_engine;

namespace VPX
{

// Streams plugin-provided raw buffer audio into a miniaudio engine: the plugin pushes PCM
// via Enqueue into a ring buffer that the engine pulls from, mixed unclamped in the same
// node graph as VPX's own sounds and routed through a shared soft limiter.
class AudioStreamPlayer
{
public:
   // attachTo is the node to attach the stream's output to (the shared limiter), as void*
   // (= ma_node) to keep miniaudio.h out of this header; null falls back to the engine endpoint.
   static std::unique_ptr<AudioStreamPlayer> Create(ma_engine* engine, void* attachTo, int frequency, int channels, bool isFloat);
   explicit AudioStreamPlayer(ma_engine* engine, void* attachTo, int frequency, int channels, bool isFloat);
   ~AudioStreamPlayer();

   bool IsValid() const { return m_valid; }
   void Enqueue(const uint8_t* buffer, int length);
   void FlushStream();
   int GetQueuedSize() const;
   void SetName(string name) { m_name = std::move(name); }
   void SetStreamVolume(const float volume);
   void SetMainVolume(const float volume);

private:
   void ApplyVolume();

   // Opaque miniaudio state (held via pimpl to keep miniaudio.h out of this header).
   struct Impl;
   std::unique_ptr<Impl> m_impl;

   int m_frequency = 0;
   int m_channels = 0;
   bool m_isFloat = false;
   bool m_valid = false;
   float m_mainVolume = 1.f;
   float m_streamVolume = 1.f;
   string m_name;
};

}
