// license:GPLv3+

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

#include "core/TableCatalog.h"

class BaseTexture;
class RenderDevice;
class RenderTarget;
class Sampler;

namespace VPX::Frontend
{

// Lazy cache of the frontend media files. Still images are decoded asynchronously on a worker
// thread, then uploaded to GPU samplers on the render thread during BeginFrame; Get returns nullptr
// while an image is being decoded. Videos and animated images (decoded through the shared
// VPX::MediaPlayer service) run one session per file on a dedicated thread, publishing frames paced
// by their presentation timestamps; the render thread uploads them into a persistent sampler.
// Blurred variants are produced by the GPU gaussian blur shaders into a render target. Least
// recently used entries are evicted when the cache grows past its memory budget.
class FrontendMedia final
{
public:
   explicit FrontendMedia(RenderDevice *renderDevice);
   ~FrontendMedia();

   // The media of a catalog entry, if it exists and is decoded, else nullptr (keeps loading)
   std::shared_ptr<Sampler> Get(const TableCatalog::Entry &entry, TableCatalog::MediaType type);

   // An arbitrary image file (theme underlay/overlay) or video, if decoded, else nullptr.
   // Videos decode at a bounded resolution (upload cost scales with the frame size): the
   // bound defaults to a media cell size, callers presenting a media larger (e.g. a theme
   // underlay/overlay covering the display) pass its on screen size
   std::shared_ptr<Sampler> Get(const std::filesystem::path &path, unsigned int targetW = 0, unsigned int targetH = 0);

   // A GPU blurred version of the media of a catalog entry (for blurred backgrounds)
   std::shared_ptr<Sampler> GetBlurred(const TableCatalog::Entry &entry, TableCatalog::MediaType type);

   // The latest decoded frame of a video media (auxiliary window textures), nullptr until the
   // first frame; seq is bumped on each new frame so callers can skip unchanged updates
   std::shared_ptr<const BaseTexture> GetVideoFrame(const std::filesystem::path &path, uint64_t *seq);

   // Whether this media is backed by an animation decode session. Video extensions are known
   // immediately; animated images are classified by the decode worker, off the render thread.
   bool IsVideoBacked(const std::filesystem::path &path) const;

   // The time at which the sampler of an image became ready (for fade ins), 0 if not ready yet
   uint32_t GetReadyTime(const std::filesystem::path &key) const;

   // To be called once per rendered frame: updates usage timestamps, uploads decoded images and
   // computes pending GPU blurs, evicts old entries
   void BeginFrame();

private:
   struct VideoSession; // Decoding worker of a video file, defined in the .cpp

   struct Item
   {
      std::shared_ptr<BaseTexture> bitmap; // Decoded bitmap, kept to unload the sampler on eviction
      std::shared_ptr<Sampler> sampler; // Ready to use sampler (blurred one for blurred entries)
      std::shared_ptr<Sampler> rawSampler; // Blurred entries only: decoded image pending its blur
      std::unique_ptr<RenderTarget> blurRT; // Blurred entries only: target the blur is rendered to
      bool failed = false;
      bool blurred = false;
      bool needsBlur = false;
      size_t bytes = 0;
      uint64_t lastUsed = 0;
      uint32_t readyTime = 0; // msec() when the sampler became ready (fade in)
      std::shared_ptr<VideoSession> video; // Video medias only: the decode session for this file
      std::shared_ptr<const BaseTexture> videoBitmap; // Video medias only: last uploaded frame
      uint64_t videoSeq = 0; // Video medias only: sequence of videoBitmap
      unsigned int decodeW = 0, decodeH = 0; // Video medias only: requested decode bound (0 = cell sized)
      unsigned int imageMaxDimension = 0; // Still images only: largest requested decode bound
   };

   std::shared_ptr<Sampler> Load(const std::filesystem::path &key, const std::filesystem::path &file, bool blurred, unsigned int decodeW = 0, unsigned int decodeH = 0);
   std::shared_ptr<VideoSession> GetVideoSession(const std::filesystem::path &file);
   void TouchVideo(Item &item, const std::filesystem::path &file);
   static void DecodeVideo(VideoSession *session);
   void StopVideoSession(const std::shared_ptr<VideoSession> &session);
   void Pump();
   void Prune();
   void Blur(Item &item);
   void Evict();

   RenderDevice *const m_rd;
   std::unordered_map<std::filesystem::path, Item> m_items;
   uint64_t m_tick = 0;
   size_t m_bytes = 0;
   static constexpr size_t ByteBudget = 256ull << 20; // 256 MB of decoded/uploaded stills

   // Image decoding on a single worker thread (FreeImage is not guaranteed to be thread safe)
   struct Job
   {
      std::filesystem::path key;
      std::filesystem::path file;
      unsigned int maxDimension = 0;
   };
   std::thread m_worker;
   std::mutex m_mutex;
   std::condition_variable m_cv;
   std::deque<Job> m_jobs;
   struct CompletedJob
   {
      std::filesystem::path key;
      std::filesystem::path file;
      std::shared_ptr<BaseTexture> bitmap;
      bool animated = false;
   };
   vector<CompletedJob> m_completed;
   bool m_quit = false;

   // Shared render targets of the GPU blur (the destination target is per item)
   std::unique_ptr<RenderTarget> m_blurSrcRT;
   std::unique_ptr<RenderTarget> m_blurTmpRT;

   // Video decode sessions, one per video file: a worker thread decodes and paces frames, the
   // render thread uploads the latest published frame to each referencing item's sampler
   std::unordered_map<std::filesystem::path, std::shared_ptr<VideoSession>> m_videos;
   vector<std::shared_ptr<VideoSession>> m_stopping; // Sessions signaled to quit, joined once finished
};

}
