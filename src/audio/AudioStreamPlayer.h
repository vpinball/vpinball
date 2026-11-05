// license:GPLv3+

#pragma once

#include <atomic>
#include <cstdint>
#include <mutex>

struct ma_engine;
struct ma_sound;
struct vpx_node;

namespace VPX
{

class AudioPlayer;
class SpatialAudioSource;
struct vpx_stream_data_source;

// Plugin audio stream (PinMAME, PUP, AltSound, ...) rendered through the backglass
// miniaudio engine: the enqueued buffers are exposed as a custom data source
// feeding a sound, so the stream goes through the same mixing path as the other
// backglass sounds, including spatial (binaural) rendering in VR.
class AudioStreamPlayer
{
public:
   static std::unique_ptr<AudioStreamPlayer> Create(const AudioPlayer* audioPlayer, int frequency, int channels, bool isFloat);
   ~AudioStreamPlayer();

   void Enqueue(const uint8_t* buffer, int length);
   void FlushStream(); // Mark the stream as ended: queued data still plays out, then the sound stops
   // Detaches the stream's nodes from its engine while the engine is still alive. Must be called before
   // the engine is uninitialized, as external holders may keep this object alive past its engine's death.
   // The object stays valid afterwards but is silent and reports IsFinished().
   void DetachFromEngine();
   bool IsFinished() const; // Whether a flushed stream has played out (a live stream never ends, underruns play silence)
   int GetQueuedSize() const;
   void SetName(string name) { m_name = std::move(name); }
   void SetStreamVolume(const float volume);
   void SetMainVolume(const float volume);

private:
   friend struct vpx_stream_data_source;
   AudioStreamPlayer() = default;
   uint64_t ReadFrames(float* out, uint64_t frameCount); // Called by the embedded data source on the audio thread
   void ApplyVolume();

   std::unique_ptr<vpx_stream_data_source> m_dataSource;
   std::unique_ptr<ma_sound> m_sound;
   std::unique_ptr<SpatialAudioSource> m_spatialSource;
   std::unique_ptr<vpx_node> m_vpxMixNode; // Channel mixing node honoring the backglass output mode, used when spatial audio is disabled

   // Ring buffer of interleaved float frames, written by Enqueue (game/plugin
   // thread) and read by the audio thread through the data source
   mutable std::mutex m_mutex;
   std::vector<float> m_buffer;
   uint64_t m_writeIdx = 0;
   uint64_t m_readIdx = 0;
   size_t m_capFrames = 0;

   uint32_t m_frequency = 0;
   uint32_t m_channels = 0;
   bool m_isFloat = false;
   std::atomic<bool> m_eof { false };
   float m_mainVolume = 1.f;
   float m_streamVolume = 1.f;
   string m_name;
};

}
