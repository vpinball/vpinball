// license:GPLv3+

#pragma once

#include "plugins/VPXPlugin.h"
#include "LibAv.h"

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

struct AVFormatContext;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;
struct SwsContext;
struct SwrContext;

namespace VPX
{

// Decoded video frame, exposed to plugins as the opaque MediaFrame handle.
// The frame is a lazy token: the decoded source frame is held until the pixels
// are materialized by GetFramePixels/UpdateTextureFromFrame, then released.
struct DecodedFrame
{
   AVFrame* frame = nullptr; // Decoded source frame (owned), freed once materialized or released
   double pts = 0.; // Presentation timestamp [s]
   uint8_t* pixels = nullptr; // Materialized RGBA pixels, from the session pool
   unsigned int width = 0; // Materialized size
   unsigned int height = 0;
};

// Synchronous media (video & audio) decoding session, implementing the
// plugin-facing MediaAPI. All calls are expected to be made from a
// single thread (the one the plugin decodes on), except ReleaseFrame which
// may also be called from the frame consuming thread as long as calls are
// serialized with the session's use.
class MediaPlayer
{
public:
   // Open a session, returning nullptr on failure (also when the FFmpeg runtime
   // libraries are not available, lazily loaded on Windows)
   static MediaPlayer* TryOpen(const char* path);

   explicit MediaPlayer(const char* path);
   ~MediaPlayer();

   bool IsOpen() const { return m_formatContext != nullptr; }

   void GetInfo(MediaInfo* info) const;
   int Read(MediaOutput* output);
   int Seek(double position);
   void SetVideoFrameSize(unsigned int width, unsigned int height)
   {
      m_targetWidth = width;
      m_targetHeight = height;
   }
   void ReleaseFrame(MediaFrame frame);
   int GetFramePixels(MediaFrame frame, const uint8_t** data, unsigned int* width, unsigned int* height);

private:
   AVCodecContext* OpenStream(int streamIndex);
   void ReceiveFrames(AVCodecContext* codecContext);
   void EmitAudioFrame(AVFrame* frame);
   double FramePtsSeconds(const AVFrame* frame, int streamIndex);
   bool Materialize(DecodedFrame* mediaFrame);
   uint8_t* AcquirePixels(unsigned int width, unsigned int height);

   AVFormatContext* m_formatContext = nullptr;
   AVPacket* m_packet = nullptr;

   int m_videoStream = -1;
   AVCodecContext* m_videoContext = nullptr;
   double m_videoTimeBaseSec = 1.; // Stream time base as a pts to seconds factor

   int m_audioStream = -1;
   AVCodecContext* m_audioContext = nullptr;
   double m_audioTimeBaseSec = 1.;

   struct SwsContext* m_swsContext = nullptr;
   std::atomic<unsigned int> m_targetWidth { 0 }; // Decoded output size override (0 = native)
   std::atomic<unsigned int> m_targetHeight { 0 };

   struct SwrContext* m_audioConversionContext = nullptr;
   int m_audioFormat = -1; // AVSampleFormat of the current conversion, -1 = none
   int m_audioFreq = 0;

   // A decoded unit pending emission. Audio chunks own their data so a queued
   // output stays valid until it is consumed by Read (the returned pointer is
   // then kept alive by m_lastOutput until the next Read, per the API contract).
   struct QueuedOutput
   {
      MediaOutput output {};
      std::unique_ptr<uint8_t[]> ownedAudio;
   };
   std::deque<QueuedOutput> m_outputs; // Decoded units pending emission by Read
   void DropQueuedOutputs(); // Clears m_outputs, releasing the video frames it owns
   QueuedOutput m_lastOutput; // Last emitted output, keeps audio data alive until next Read
   bool m_inputEof = false; // The container reported end of file (decoders may still drain)
   double m_frameDuration = 0.; // [s], for timestamp-less frames
   double m_lastVideoPts = 0.;
   double m_lastAudioPts = 0.;

   // Pool of materialized RGBA buffers, recycled through ReleaseFrame
   struct PooledPixels
   {
      uint8_t* data;
      unsigned int width;
      unsigned int height;
   };
   std::mutex m_pixelsMutex; // Serializes materialization: sws context and pixel pool
   std::vector<PooledPixels> m_pixelsPool;
   std::atomic<int> m_liveFrames { 0 }; // Frames currently handed out, for leak checking

   const LibAV::LibAV& m_libAV = LibAV::LibAV::GetInstance(); // Lazily loads the FFmpeg runtime on first session
};

}
