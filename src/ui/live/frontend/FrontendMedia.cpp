// license:GPLv3+

#include "core/stdafx.h"
#include "FrontendMedia.h"

#include "media/MediaPlayer.h"
#include "utils/wintimer.h"
#include "renderer/RenderDevice.h"
#include "renderer/RenderPass.h"
#include "renderer/RenderTarget.h"
#include "renderer/Sampler.h"
#include "renderer/Shader.h"
#include "renderer/Texture.h"
#include "renderer/TextureManager.h"


namespace VPX::Frontend
{

// One decoding session per video file: a worker thread reads the media through the shared
// VPX::MediaPlayer service (synchronous single threaded session), materializes each video frame
// when its presentation timestamp is due on the frontend clock and publishes it for the render
// thread to upload. Audio outputs are dropped: the frontend plays media videos silent. The stream
// loops back to its start at end of file.
struct FrontendMedia::VideoSession
{
   std::filesystem::path file;
   std::thread thread;
   std::mutex mutex;
   std::condition_variable cv;
   std::atomic<bool> quit { false }; // Set to end the session
   std::atomic<bool> suspend { false }; // Set to pause decoding while unused
   bool stopping = false; // Render thread only: already moved to the stopping list
   std::atomic<bool> failed { false }; // Set on the worker thread when decoding could not run
   std::atomic<bool> finished { false };
   std::shared_ptr<BaseTexture> recycled; // Render thread to worker: a frame buffer free for reuse
   std::shared_ptr<const BaseTexture> pending; // Worker to render thread: latest materialized frame
   uint64_t seq = 0; // Publication counter of pending
   uint64_t lastUsed = 0; // Render thread only: m_tick of the last request on this file
   std::atomic<unsigned int> decodeW { 0 }; // Largest decode bound requested by referencing items
   std::atomic<unsigned int> decodeH { 0 }; // (0 = default media cell bound)
};

FrontendMedia::FrontendMedia(RenderDevice *renderDevice)
   : m_rd(renderDevice)
{
   m_worker = std::thread(
      [this]()
      {
         while (true)
         {
            Job job;
            {
               std::unique_lock lock(m_mutex);
               m_cv.wait(lock, [this] { return m_quit || !m_jobs.empty(); });
               if (m_quit)
                  return;
               job = m_jobs.front();
               m_jobs.pop_front();
            }
            CompletedJob completed;
            completed.key = job.key;
            completed.file = job.file;
            // APNG sniffing opens the media file: do it here, never on the render thread. A
            // decodable first frame is still used as a poster while the animated session starts.
            completed.animated = TableCatalog::IsAnimatedImageFile(job.file);
            completed.bitmap = BaseTexture::CreateFromFile(job.file, job.maxDimension);
            {
               std::lock_guard lock(m_mutex);
               m_completed.push_back(std::move(completed));
            }
         }
      });
}

FrontendMedia::~FrontendMedia()
{
   for (auto &[path, session] : m_videos)
      StopVideoSession(session);
   m_videos.clear();
   for (auto &[key, item] : m_items)
      if (item.video != nullptr)
         StopVideoSession(item.video); // Already stopped sessions are skipped
   for (auto &session : m_stopping)
      session->thread.join();
   m_stopping.clear();
   {
      std::lock_guard lock(m_mutex);
      m_quit = true;
   }
   m_cv.notify_all();
   m_worker.join();
}

void FrontendMedia::DecodeVideo(VideoSession *const session)
{
   SetThreadName("Frontend Video");
   std::unique_ptr<VPX::MediaPlayer> player(VPX::MediaPlayer::TryOpen(PathToUTF8(session->file).c_str()));
   if (player == nullptr || !player->IsOpen())
   {
      PLOGW << "FrontendMedia: failed to open video '" << session->file << "'";
      session->failed = true;
      session->finished = true;
      return;
   }
   MediaInfo info {};
   player->GetInfo(&info);
   if (info.videoWidth == 0)
   {
      session->failed = true;
      session->finished = true;
      return;
   }
   // Decode at a bounded resolution: frontend medias are mostly shown at cell or pane size and
   // the upload cost scales with the frame size. Items presenting a media larger (e.g. a theme
   // underlay/overlay covering the display) raise the bound, applied live on the next frame
   unsigned int capW = 1280, capH = 720;
   const auto applyDecodeBound = [&]()
   {
      unsigned int w = 0, h = 0; // 0: decode at the video's native resolution
      if (info.videoWidth > capW || info.videoHeight > capH)
      {
         const float s = std::min(capW / static_cast<float>(info.videoWidth), capH / static_cast<float>(info.videoHeight));
         w = static_cast<unsigned int>(info.videoWidth * s) & ~1u;
         h = static_cast<unsigned int>(info.videoHeight * s) & ~1u;
      }
      player->SetVideoFrameSize(w, h);
   };
   applyDecodeBound();
   double clockBase = -1.; // Wall clock time at which pts 0 is presented, -1 until the first frame
   while (true)
   {
      {
         std::unique_lock lock(session->mutex);
         if (session->quit)
            break;
         if (session->suspend)
         {
            session->cv.wait(lock, [session] { return session->quit || !session->suspend; });
            clockBase = -1.; // Rebase the presentation clock on resume
            if (session->quit)
               break;
         }
      }
      const unsigned int reqW = session->decodeW.load(), reqH = session->decodeH.load();
      if (reqW > capW || reqH > capH)
      {
         capW = std::max(capW, reqW);
         capH = std::max(capH, reqH);
         applyDecodeBound();
      }
      MediaOutput output {};
      if (player->Read(&output) != 0 || output.type == MEDIA_OUT_ERROR)
      {
         PLOGW << "FrontendMedia: failed to decode video '" << session->file << "'";
         session->failed = true;
         break;
      }
      if (output.type == MEDIA_OUT_AUDIO)
         continue; // Silent media videos: audio samples are dropped
      if (output.type == MEDIA_OUT_EOF)
      {
         if (player->Seek(0.) != 0)
         {
            session->failed = true;
            break;
         }
         clockBase = -1.;
         continue;
      }
      if (output.type != MEDIA_OUT_VIDEO)
         continue;

      // Present each frame when its pts is due on the session clock; frames late by more than a
      // few frame periods are dropped to catch the pace back
      const double now = static_cast<double>(SDL_GetTicksNS() / 1000) / 1e6;
      if (clockBase < 0.)
         clockBase = now - output.pts;
      const double late = now - (clockBase + output.pts);
      if (late < -0.001)
      {
         std::unique_lock lock(session->mutex);
         session->cv.wait_for(lock, std::chrono::duration<double>(-late), [session] { return session->quit || session->suspend; });
      }
      if (session->quit || session->suspend || late > 0.150)
      {
         player->ReleaseFrame(output.videoFrame);
         continue;
      }
      const uint8_t *pixels = nullptr;
      unsigned int width = 0, height = 0;
      if (player->GetFramePixels(output.videoFrame, &pixels, &width, &height) == 0)
      {
         std::shared_ptr<BaseTexture> tex;
         {
            std::lock_guard lock(session->mutex);
            tex = std::move(session->recycled);
         }
         // The recycled buffer may still be referenced by a pending GPU upload: writing it would
         // corrupt the upload, so it is only reused once fully released
         if (tex != nullptr && (tex.use_count() > 1 || tex->width() != width || tex->height() != height))
            tex = nullptr;
         if (tex == nullptr)
            tex = BaseTexture::Create(width, height, BaseTexture::SRGBA);
         if (tex != nullptr)
         {
            memcpy(tex->data(), pixels, static_cast<size_t>(width) * height * 4);
            std::lock_guard lock(session->mutex);
            session->pending = tex;
            session->seq++;
         }
      }
      player->ReleaseFrame(output.videoFrame);
   }
   session->finished = true;
}

std::shared_ptr<FrontendMedia::VideoSession> FrontendMedia::GetVideoSession(const std::filesystem::path &file)
{
   const auto it = m_videos.find(file);
   if (it != m_videos.end() && !it->second->finished && !it->second->quit)
      return it->second;
   std::shared_ptr<VideoSession> session = std::make_shared<VideoSession>();
   session->file = file;
   session->thread = std::thread(&FrontendMedia::DecodeVideo, session.get());
   if (it == m_videos.end())
      return m_videos.emplace(file, std::move(session)).first->second;
   // The previous session was stopped but is still being joined in the stopping list: it keeps
   // owning its lifetime there while the map hands out the fresh session
   it->second = session;
   return session;
}

void FrontendMedia::StopVideoSession(const std::shared_ptr<VideoSession> &session)
{
   if (session->stopping)
      return; // The same session may be referenced by several items (media and its blurred variant)
   session->stopping = true;
   {
      std::lock_guard lock(session->mutex);
      session->quit = true;
      session->suspend = false;
   }
   session->cv.notify_all();
   m_stopping.push_back(session);
}

std::shared_ptr<Sampler> FrontendMedia::Get(const TableCatalog::Entry &entry, const TableCatalog::MediaType type)
{
   if (!entry.HasMedia(type))
      return nullptr;
   return Get(entry.GetMedia(type));
}

std::shared_ptr<Sampler> FrontendMedia::Get(const std::filesystem::path &path, const unsigned int targetW, const unsigned int targetH)
{
   return Load(path, path, false, targetW, targetH);
}

std::shared_ptr<Sampler> FrontendMedia::GetBlurred(const TableCatalog::Entry &entry, const TableCatalog::MediaType type)
{
   if (!entry.HasMedia(type))
      return nullptr;
   const std::filesystem::path &file = entry.GetMedia(type);
   return Load(file.wstring() + L".blur", file, true, 256, 256);
}

bool FrontendMedia::IsVideoBacked(const std::filesystem::path &path) const
{
   if (TableCatalog::IsVideoFile(path))
      return true;
   const auto it = m_items.find(path);
   return it != m_items.end() && it->second.video != nullptr;
}

uint32_t FrontendMedia::GetReadyTime(const std::filesystem::path &key) const
{
   const auto it = m_items.find(key);
   return (it != m_items.end() && !it->second.failed) ? it->second.readyTime : 0;
}

static unsigned int ImageDecodeBound(const unsigned int decodeW, const unsigned int decodeH)
{
   const unsigned int requested = std::max(decodeW, decodeH);
   return requested != 0 ? requested : 1024; // Bounded preview quality when the caller has no on screen size
}

std::shared_ptr<Sampler> FrontendMedia::Load(const std::filesystem::path &key, const std::filesystem::path &file, const bool blurred, const unsigned int decodeW, const unsigned int decodeH)
{
   // Only the extension is tested here. Sniffing image files for animation is filesystem IO and
   // is deferred to the image worker: media classification must never stall rendering.
   const bool video = TableCatalog::IsVideoFile(file);
   auto it = m_items.find(key);
   if (it != m_items.end())
   {
      it->second.lastUsed = m_tick;
      if (video || it->second.video != nullptr)
      {
         it->second.decodeW = std::max(it->second.decodeW, decodeW);
         it->second.decodeH = std::max(it->second.decodeH, decodeH);
         TouchVideo(it->second, file);
      }
      else if (!it->second.failed)
      {
         const unsigned int bound = ImageDecodeBound(decodeW, decodeH);
         if (bound > it->second.imageMaxDimension)
         {
            it->second.imageMaxDimension = bound;
            std::lock_guard lock(m_mutex);
            m_jobs.push_back({ key, file, bound });
            m_cv.notify_one();
         }
      }
      return it->second.failed ? nullptr : it->second.sampler;
   }

   Item item;
   item.blurred = blurred;
   item.lastUsed = m_tick;
   item.decodeW = decodeW;
   item.decodeH = decodeH;
   item.imageMaxDimension = ImageDecodeBound(decodeW, decodeH);
   it = m_items.emplace(key, std::move(item)).first;
   if (video)
   {
      TouchVideo(it->second, file);
      return nullptr;
   }

   // Queue the decode on the worker thread; the sampler will appear in a later frame
   {
      std::lock_guard lock(m_mutex);
      m_jobs.push_back({ key, file });
   }
   m_cv.notify_one();
   return nullptr;
}

// Attaches the item to the decode session of its file, restarting it when the previous session
// has been stopped (a media and its blurred variant share the same session)
void FrontendMedia::TouchVideo(Item &item, const std::filesystem::path &file)
{
   if (item.video == nullptr || item.video->finished || item.video->quit)
      item.video = GetVideoSession(file);
   item.video->lastUsed = m_tick;
   // Raise the session decode bound to this item's need (render thread only writes, the worker reads)
   item.video->decodeW = std::max(item.video->decodeW.load(), item.decodeW);
   item.video->decodeH = std::max(item.video->decodeH.load(), item.decodeH);
   {
      std::lock_guard lock(item.video->mutex);
      if (item.video->suspend)
      {
         item.video->suspend = false;
         item.video->cv.notify_one();
      }
   }
}

std::shared_ptr<const BaseTexture> FrontendMedia::GetVideoFrame(const std::filesystem::path &file, uint64_t *const seq)
{
   Load(file, file, false); // Ensures the item exists; animated images become video backed after worker classification
   const auto it = m_items.find(file);
   if (it == m_items.end())
      return nullptr;
   if (seq != nullptr)
      *seq = it->second.videoSeq;
   return it->second.videoBitmap;
}

void FrontendMedia::BeginFrame()
{
   m_tick++;
   Pump();
   Prune();
   Evict();

   // Suspend the decode of video sessions no longer requested, stop the long stale ones and join
   // the finished threads of the stopping list
   for (auto it = m_videos.begin(); it != m_videos.end();)
   {
      VideoSession *const session = it->second.get();
      if (session->lastUsed + 600 <= m_tick)
      {
         StopVideoSession(it->second);
         it = m_videos.erase(it);
         continue;
      }
      const bool suspend = session->lastUsed + 30 <= m_tick;
      {
         std::lock_guard lock(session->mutex);
         if (session->suspend != suspend)
         {
            session->suspend = suspend;
            session->cv.notify_one();
         }
      }
      ++it;
   }
   for (auto it = m_stopping.begin(); it != m_stopping.end();)
      if ((*it)->finished)
      {
         (*it)->thread.join();
         it = m_stopping.erase(it);
      }
      else
         ++it;
}

void FrontendMedia::Pump()
{
   // Upload the images decoded by the worker thread (must run on the render thread), bounding the
   // per frame GPU upload cost so a burst of decodes does not stall navigation
   vector<CompletedJob> completed;
   {
      std::lock_guard lock(m_mutex);
      completed.swap(m_completed);
   }
   int uploadBudget = 4;
   vector<CompletedJob> deferred;
   for (CompletedJob &completedJob : completed)
   {
      const std::filesystem::path &key = completedJob.key;
      const std::shared_ptr<BaseTexture> &bitmap = completedJob.bitmap;
      auto it = m_items.find(key);
      if (it == m_items.end())
         continue; // evicted while decoding
      if (uploadBudget-- <= 0)
      {
         deferred.push_back(std::move(completedJob));
         continue;
      }
      Item &item = it->second;
      if (completedJob.animated && item.video == nullptr)
         TouchVideo(item, completedJob.file);
      if (bitmap == nullptr)
      {
         if (!completedJob.animated)
         {
            PLOGW << "FrontendMedia: failed to decode '" << key << "'";
            item.failed = true;
         }
         continue;
      }
      const std::shared_ptr<Sampler> sampler = m_rd->m_texMan.LoadTexture(bitmap.get(), false);
      if (sampler == nullptr)
      {
         PLOGW << "FrontendMedia: failed to upload '" << key << "'";
         item.failed = true;
         continue;
      }
      if (item.bitmap != nullptr && item.bitmap != bitmap)
         m_rd->m_texMan.UnloadTexture(item.bitmap.get());
      m_bytes -= item.bytes;
      item.bitmap = bitmap;
      item.bytes = static_cast<size_t>(sampler->GetWidth()) * sampler->GetHeight() * 4;
      m_bytes += item.bytes;
      if (item.blurred)
      {
         item.rawSampler = sampler;
         item.needsBlur = true;
      }
      else
      {
         item.sampler = sampler;
         if (item.readyTime == 0)
            item.readyTime = static_cast<uint32_t>(SDL_GetTicks());
      }
   }
   if (!deferred.empty())
   {
      std::lock_guard lock(m_mutex);
      m_completed.insert(m_completed.end(), std::make_move_iterator(deferred.begin()), std::make_move_iterator(deferred.end()));
   }

   // Upload the frames materialized by the video decode sessions to each referencing item's
   // sampler; a blurred item keeps the live frame in rawSampler and is re-blurred below
   for (auto &[key, item] : m_items)
   {
      VideoSession *const session = item.video.get();
      if (session == nullptr)
         continue;
      if (session->failed)
      {
         item.failed = true;
         continue;
      }
      std::shared_ptr<const BaseTexture> frame;
      {
         std::lock_guard lock(session->mutex);
         if (item.videoSeq != session->seq)
         {
            frame = session->pending;
            item.videoSeq = session->seq;
         }
      }
      if (frame == nullptr)
         continue;
      // Hand the previously uploaded frame back to the worker once nothing references it anymore
      if (item.videoBitmap != nullptr && item.videoBitmap.use_count() == 1)
      {
         std::lock_guard lock(session->mutex);
         session->recycled = std::const_pointer_cast<BaseTexture>(item.videoBitmap);
      }
      item.videoBitmap = frame;
      std::shared_ptr<Sampler> &target = item.blurred ? item.rawSampler : item.sampler;
      // The decode bound can change mid session (a fullscreen consumer upgrading a stream that
      // was opened for a cell): reallocate the sampler when the frame size changes
      if (target == nullptr || target->GetWidth() != static_cast<int>(frame->width()) || target->GetHeight() != static_cast<int>(frame->height()))
         target = std::make_shared<Sampler>(m_rd, "Frontend Video"s, frame, false);
      else
         target->UpdateTexture(frame, false);
      const size_t frameBytes = static_cast<size_t>(frame->width()) * frame->height() * 4;
      if (item.bytes != frameBytes)
      {
         m_bytes -= item.bytes;
         item.bytes = frameBytes;
         m_bytes += item.bytes;
      }
      if (item.blurred)
         item.needsBlur = true;
      if (item.readyTime == 0)
         item.readyTime = static_cast<uint32_t>(SDL_GetTicks());
   }

   // Compute pending blurs through the GPU gaussian blur shaders (bounded per frame)
   int blurBudget = 2;
   for (auto &[key, item] : m_items)
      if (item.needsBlur && blurBudget-- > 0)
         Blur(item);
}

void FrontendMedia::Prune()
{
   // Drop queued decodes whose image has not been requested in the last frames: scrolling fast
   // through the wheel piles up jobs for items that moved out of view before being started
   std::lock_guard lock(m_mutex);
   for (auto it = m_jobs.begin(); it != m_jobs.end();)
   {
      const auto itemIt = m_items.find(it->key);
      if (itemIt != m_items.end() && itemIt->second.lastUsed + 2 <= m_tick)
      {
         m_items.erase(itemIt); // erase so a later request re-enqueues the decode
         it = m_jobs.erase(it);
      }
      else
         ++it;
   }
}

void FrontendMedia::Blur(Item &item)
{
   item.needsBlur = false;
   RenderDevice *const rd = m_rd;
   constexpr int size = 256;
   if (m_blurSrcRT == nullptr)
   {
      m_blurSrcRT = std::make_unique<RenderTarget>(rd, SurfaceType::RT_DEFAULT, "Frontend Blur Source"s, size, size, colorFormat::RGBA8, false, 1,
         "Failed to create frontend blur source render target");
      m_blurTmpRT = std::make_unique<RenderTarget>(rd, SurfaceType::RT_DEFAULT, "Frontend Blur Temp"s, size, size, colorFormat::RGBA8, false, 1,
         "Failed to create frontend blur temp render target");
   }
   if (item.blurRT == nullptr)
      item.blurRT = std::make_unique<RenderTarget>(rd, SurfaceType::RT_DEFAULT, "Frontend Blurred Media"s, size, size, colorFormat::RGBA8, false, 1,
         "Failed to create frontend blur render target");

   // Blit the decoded image into the small source target, then run the gaussian blur passes
   RenderPass *const prevPass = rd->GetCurrentPass();
   RenderState prevState;
   rd->CopyRenderStates(true, prevState);
   rd->ResetRenderState();
   rd->SetRenderState(RenderState::ALPHABLENDENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::CULLMODE, RenderState::CULL_NONE);
   rd->SetRenderState(RenderState::ZWRITEENABLE, RenderState::RS_FALSE);
   rd->SetRenderState(RenderState::ZENABLE, RenderState::RS_FALSE);
   rd->SetRenderTarget("Frontend Blur Source"s, m_blurSrcRT.get(), false);
   rd->Clear(clearType::TARGET, 0x00000000);
   rd->m_FBShader->SetTexture(ShaderUniform::tex_fb_filtered, item.rawSampler);
   rd->m_FBShader->SetTechnique(ShaderTechnique::fb_copy);
   rd->DrawFullscreenTexturedQuad(rd->m_FBShader);
   rd->m_FBShader->SetTextureNull(ShaderUniform::tex_fb_filtered);
   rd->DrawGaussianBlur(m_blurSrcRT.get(), m_blurTmpRT.get(), item.blurRT.get(), 19.f);
   rd->CopyRenderStates(false, prevState);
   if (prevPass != nullptr)
   {
      rd->SetRenderTarget(prevPass->m_name, prevPass->m_rt, true);
      prevPass->MarkInterrupted();
      // The blurred target is consumed through a shader texture binding which the pass scheduler cannot
      // see: declare the dependency or the blur passes are unreachable and dropped from the frame
      rd->AddRenderTargetDependency(item.blurRT.get());
   }

   item.sampler = item.blurRT->GetColorSampler();
   if (item.video == nullptr)
      item.rawSampler.reset(); // Still images blur once; video items keep their live source
   if (item.readyTime == 0)
      item.readyTime = static_cast<uint32_t>(SDL_GetTicks());
}

void FrontendMedia::Evict()
{
   if (m_bytes <= ByteBudget)
      return;

   // Evict least recently used entries until back under budget (this also unloads the GPU samplers)
   vector<std::filesystem::path> lru;
   lru.reserve(m_items.size());
   for (const auto &[path, item] : m_items)
      lru.push_back(path);
   std::ranges::sort(lru, [&](const std::filesystem::path &a, const std::filesystem::path &b) { return m_items[a].lastUsed < m_items[b].lastUsed; });
   for (const auto &path : lru)
   {
      if (m_bytes <= ByteBudget)
         break;
      auto it = m_items.find(path);
      if (it == m_items.end() || it->second.lastUsed == m_tick)
         continue; // keep media used by the current frame
      if (it->second.bitmap != nullptr)
         m_rd->m_texMan.UnloadTexture(it->second.bitmap.get());
      m_bytes -= it->second.bytes;
      m_items.erase(it);
   }
}

}
