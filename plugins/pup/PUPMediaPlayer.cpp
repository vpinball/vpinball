/*
 * Portions of this code was derived from SDL and FFMPEG:
 *
 * https://github.com/libsdl-org/SDL/blob/ed6381b68d4053aba65c08857d62b852f0db6832/test/testffmpeg.c
 * https://github.com/FFmpeg/FFmpeg/blob/e38092ef9395d7049f871ef4d5411eb410e283e0/fftools/ffplay.c
 *
 * The actual media decoding is performed by the VPX core and accessed through
 * the MediaAPI (see plugins/MediaPlugin.h).
 */

#include "PUPMediaPlayer.h"

#include <cfloat>
#include <climits>

namespace PUP {

std::atomic<uint32_t> PUPMediaPlayer::m_nextAudioStreamId = 0;

PUPMediaPlayer::PUPMediaPlayer(const string& name)
   : m_name(name)
   , m_frames(3)
   , m_scaledMask(nullptr, &SDL_DestroySurface)
   , m_commandQueue(1)
{
   assert(mediaApi != nullptr);
   m_audioStreamId = m_nextAudioStreamId.fetch_add(1);
   SetName(name);
}

PUPMediaPlayer::~PUPMediaPlayer()
{
   Stop();
}

bool PUPMediaPlayer::IsPlaying()
{
   return m_running.load(std::memory_order_relaxed) || (m_pendingPlay.load(std::memory_order_relaxed) > m_pendingStop.load(std::memory_order_relaxed));
}

void PUPMediaPlayer::SetName(const string& name)
{
   m_commandQueue.enqueue(
      [this, name]()
      {
         SetThreadName(name + ".CmdQueue");
         std::lock_guard lock(m_mutex);
         m_name = name;
      });
}

void PUPMediaPlayer::SetBounds(const SDL_Rect& rect)
{
   m_commandQueue.enqueue(
      [this, rect]()
      {
         {
            std::lock_guard lock(m_mutex);
            m_bounds = rect;
         }
         if (m_media)
            mediaApi->SetVideoFrameSize(m_media, rect.w > 0 ? static_cast<unsigned int>(rect.w) : 0, rect.h > 0 ? static_cast<unsigned int>(rect.h) : 0);
      });
}

void PUPMediaPlayer::SetMask(std::shared_ptr<SDL_Surface> mask)
{
   m_commandQueue.enqueue([this, mask]()
   {
      std::lock_guard lock(m_mutex);
      m_mask = mask;
      m_scaledMask.reset();
   });
}

void PUPMediaPlayer::SetGameTime(double gameTime)
{
   const double now = static_cast<double>(SDL_GetTicks()) / 1000.0;
   if (m_lastGameTime < 0.0 || gameTime > m_lastGameTime)
   {
      m_lastGameTime = gameTime;
      m_lastGameTimeWall = now;
      m_gameTimeOffsetBase = m_gameTimeOffset;
   }
   else if (now - m_lastGameTimeWall > 0.1)
      m_gameTimeOffset = m_gameTimeOffsetBase + (now - m_lastGameTimeWall);
   m_gameTime = gameTime + m_gameTimeOffset;
}

double PUPMediaPlayer::GetPlayTime() const
{
   assert(m_gameTime >= 0.0 || !m_syncOnGameTime);
   return (m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0)) - m_startTimestamp;
}

void PUPMediaPlayer::Play(const std::filesystem::path& filename, float volume, double startPosition, double stillDuration)
{
   m_pendingPlay++;
   m_commandQueue.enqueue(
      [this, filename, volume, startPosition, stillDuration]()
   {
      LOGD("> Playing filename=" + PluginStrings::PathToUTF8(filename));

      // Suppress end callback during play-to-play transition — the previous video
      // is being replaced, not ending naturally.
      auto savedCallback = m_onEndCallback;
      m_onEndCallback = [](){};
      StopBlocking();
      m_onEndCallback = savedCallback;

      Media media = mediaApi->Open(PluginStrings::PathToUTF8(filename).c_str(), MEDIA_OPEN_DEFAULT);
      if (media == nullptr)
      {
         LOGE("Unable to open: filename=" + PluginStrings::PathToUTF8(filename));
         if (--m_pendingPlay == 0)
            ReleaseHoldTexture();
         return;
      }

      MediaInfo info {};
      mediaApi->GetInfo(media, &info);
      if (info.videoWidth > 0)
         LOGD(std::format("Video stream: {}x{} file={}", info.videoWidth, info.videoHeight, PluginStrings::PathToUTF8(filename)));
      if (info.audioSampleRate > 0)
         LOGD(std::format("Audio stream: {} channels, {} Hz", info.audioChannels, info.audioSampleRate));

      {
         std::lock_guard lock(m_mutex);
         if (m_bounds.w > 0 && m_bounds.h > 0)
            mediaApi->SetVideoFrameSize(media, static_cast<unsigned int>(m_bounds.w), static_cast<unsigned int>(m_bounds.h));
      }

      if (startPosition > 0.0 && mediaApi->Seek(media, startPosition) < 0)
         LOGE("Unable to resume at position: filename=" + PluginStrings::PathToUTF8(filename));

      {
         std::lock_guard lock(m_mutex);
         m_filename = filename;
         m_volume = volume;
         m_loop = false;
         m_stillDuration = stillDuration;
         m_paused = false;
         m_syncOnGameTime = m_gameTime >= 0.0;
         m_startTimestamp = m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0);
         m_media = media;
         m_hasVideo = info.videoWidth > 0;
         m_hasAudio = info.audioSampleRate > 0;
         m_streamStartTime = info.startTime;
         m_videoDuration = info.videoDuration;
         m_videoPtsBaseValid = false;
         m_videoPtsBase = 0.0;
         m_audioPtsBaseValid = false;
         m_audioPtsBase = 0.0;
         m_position = startPosition;
         m_duration = info.duration;
         m_seekRequest = -1.0;
         m_safeLoop = false;
         m_lastAudioPass = -1;
         m_firstRenderPending = m_hasVideo;
         // Audio only media: no video frame will replace the held one
         if (!m_hasVideo && m_holdTexture)
         {
            DeleteTexture(m_holdTexture);
            m_holdTexture = nullptr;
         }
         m_lastDecodedPts = 0.0;
         m_lastRenderTicks = 0;
         m_playTicks = SDL_GetTicks();
         m_running = true;
         m_thread = std::thread(&PUPMediaPlayer::Run, this);
      }
      m_pendingPlay--;
   });
}

void PUPMediaPlayer::Pause(bool pause)
{
   m_commandQueue.enqueue([this, pause]()
   {
      if (m_paused != pause)
      {
         std::lock_guard lock(m_mutex);
         m_paused = pause;
         if (m_paused)
            m_pauseTimestamp = GetPlayTime(); // Freeze at the current playing time
         else
            m_startTimestamp = m_startTimestamp + GetPlayTime() - m_pauseTimestamp; // Adjust start time to restart from freeze point
      }
   });
}

void PUPMediaPlayer::Stop()
{
   m_pendingStop++;
   m_commandQueue.enqueue(
      [this]()
      {
         StopBlocking();
         m_pendingStop--;
      });
}

// Drops the frame held from the previous media (see StopBlocking) when no new media replaces it (failed play)
void PUPMediaPlayer::ReleaseHoldTexture()
{
   std::lock_guard lock(m_mutex);
   if (m_holdTexture)
   {
      DeleteTexture(m_holdTexture);
      m_holdTexture = nullptr;
   }
}

void PUPMediaPlayer::StopBlocking()
{
   // Stop decoder thread and flush queue
   std::unique_lock lock(m_mutex);
   m_running = false;

   lock.unlock();
   if (m_thread.joinable())
      m_thread.join();
   lock.lock();

   int holdSlot = -1;
   for (int i = 0; i < static_cast<int>(m_frames.size()); i++)
      if (m_frames[i].valid && m_frames[i].texture && (holdSlot < 0 || m_frames[i].age < m_frames[holdSlot].age))
         holdSlot = i;
   if (holdSlot >= 0)
   {
      if (m_holdTexture)
         DeleteTexture(m_holdTexture);
      m_holdTexture = m_frames[holdSlot].texture;
      m_frames[holdSlot].texture = nullptr;
   }

   for (auto& frame : m_frames)
   {
      if (frame.frame)
         mediaApi->ReleaseFrame(m_media, frame.frame);
      if (frame.pixels)
         free(frame.pixels);
      if (frame.texture)
         DeleteTexture(frame.texture);
      frame = FrameInfo {};
   }

   if (m_pendingPlay == 0 && m_holdTexture)
   {
      DeleteTexture(m_holdTexture);
      m_holdTexture = nullptr;
   }

   if (m_media)
   {
      mediaApi->Close(m_media);
      m_media = nullptr;
   }
   m_hasVideo = false;
   m_hasAudio = false;
   m_streamStartTime = 0.0;
   m_videoDuration = 0.0;

   StopAudioStream(m_audioStreamId);
}

void PUPMediaPlayer::SetLoop(bool loop, double minLoopDuration)
{
   m_commandQueue.enqueue([this, loop, minLoopDuration]() mutable
   {
      std::lock_guard lock(m_mutex);
      if (loop && minLoopDuration > 0.0)
      {
         if (m_duration < minLoopDuration)
            loop = false;
      }
      if (m_loop != loop)
      {
         LOGD("Setting loop: loop=" + std::to_string(loop));
         m_loop = loop;
      }
   });
}

void PUPMediaPlayer::SetSafeLoop()
{
   m_commandQueue.enqueue([this]()
   {
      std::lock_guard lock(m_mutex);
      m_safeLoop = true;
   });
}

void PUPMediaPlayer::Seek(double position)
{
   m_commandQueue.enqueue([this, position]()
   {
      std::lock_guard lock(m_mutex);
      if (m_running)
         m_seekRequest = std::max(position, 0.0);
   });
}

void PUPMediaPlayer::SetVolume(float volume)
{
   m_commandQueue.enqueue([this, volume]()
   {
      std::lock_guard lock(m_mutex);
      if (m_volume != volume)
      {
         LOGD(std::format("Setting volume: volume={:.1f}%%", volume));
         m_volume = volume;
      }
   });
}

void PUPMediaPlayer::RenderHold(VPXRenderContext2D* const ctx, const SDL_Rect& destRect, float alpha)
{
   if (m_holdTexture == nullptr)
      return;
   const VPXTextureInfo* texInfo = GetTextureInfo(m_holdTexture);
   ctx->DrawImage(ctx, m_holdTexture, 1.f, 1.f, 1.f, alpha, 0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), 0.f, 0.f, 0.f,
      static_cast<float>(destRect.x), static_cast<float>(destRect.y), static_cast<float>(destRect.w), static_cast<float>(destRect.h));
}

void PUPMediaPlayer::RenderLastFrame(VPXRenderContext2D* const ctx, const SDL_Rect& destRect, float alpha)
{
   int slot = -1;
   for (int i = 0; i < static_cast<int>(m_frames.size()); i++)
      if (m_frames[i].valid && (slot < 0 || m_frames[i].pts > m_frames[slot].pts))
         slot = i;
   if (slot < 0)
      return;
   FrameInfo& frame = m_frames[slot];
   if (!frame.uploaded)
   {
      frame.uploaded = true;
      if (frame.frame)
         UpdateTextureFromFrame(m_media, frame.frame, &frame.texture);
      else if (frame.pixels)
         UpdateTexture(&frame.texture, static_cast<int>(frame.width), static_cast<int>(frame.height), VPXTextureFormat::VPXTEXFMT_sRGBA8, frame.pixels);
   }
   if (frame.texture == nullptr)
      return;
   const VPXTextureInfo* texInfo = GetTextureInfo(frame.texture);
   ctx->DrawImage(ctx, frame.texture, 1.f, 1.f, 1.f, alpha, 0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), 0.f, 0.f, 0.f,
      static_cast<float>(destRect.x), static_cast<float>(destRect.y), static_cast<float>(destRect.w), static_cast<float>(destRect.h));
}

void PUPMediaPlayer::Render(VPXRenderContext2D* const ctx, const SDL_Rect& destRect, float alpha)
{
   if (!m_running && m_holdTexture == nullptr && !IsPlaying() && (SDL_GetTicks() - m_endTicks) >= 100)
      return;

   std::lock_guard lock(m_mutex);

   if (!m_running)
   {
      if (m_holdTexture)
         RenderHold(ctx, destRect, alpha);
      else if (IsPlaying() || (SDL_GetTicks() - m_endTicks) < 100)
         RenderLastFrame(ctx, destRect, alpha);
      return;
   }

   const uint64_t nowTicks = SDL_GetTicks();
   const uint64_t renderGap = m_lastRenderTicks != 0 ? (nowTicks - m_lastRenderTicks) : 0;
   m_lastRenderTicks = nowTicks;

   // The playback clock only advances while the screen is actually being rendered: start the
   // timeline at the first render, and shift it across render stalls (table load, ...) so the
   // video holds and resumes instead of skipping ahead
   if (!m_paused)
   {
      if (m_firstRenderPending.exchange(false))
      {
         const double late = GetPlayTime();
         if (late > 0.0)
            m_startTimestamp += late;
      }
      else if (renderGap > 250)
      {
         m_startTimestamp += static_cast<double>(renderGap) / 1000.0;
      }
   }

   const double playPts = m_paused ? m_pauseTimestamp : GetPlayTime();

   // Search for the best frame to display (the one nearest to our play time), also ageing all slots to let the decoder select the eldest used slot
   int selectedFrameSlot = -1;
   double bestDelta = DBL_MAX;
   for (int i = 0; i < static_cast<int>(m_frames.size()); i++)
   {
      m_frames[i].age++;
      if (const double delta = std::fabs(playPts - m_frames[i].pts); m_frames[i].valid && (delta < bestDelta))
      {
         bestDelta = delta;
         selectedFrameSlot = i;
      }
   }
   if (selectedFrameSlot == -1)
   {
      RenderHold(ctx, destRect, alpha);
      return;
   }
   FrameInfo& selectedFrame = m_frames[selectedFrameSlot];

   if (!selectedFrame.uploaded)
   {
      // UpdateTexture updates the texture in place, or transparently substitutes it by a fresh one
      // when it is still referenced by a pending GPU upload
      selectedFrame.uploaded = true;
      if (selectedFrame.frame)
         UpdateTextureFromFrame(m_media, selectedFrame.frame, &selectedFrame.texture);
      else if (selectedFrame.pixels)
         UpdateTexture(&selectedFrame.texture, static_cast<int>(selectedFrame.width), static_cast<int>(selectedFrame.height), VPXTextureFormat::VPXTEXFMT_sRGBA8, selectedFrame.pixels);
   }
   if (selectedFrame.texture == nullptr)
   {
      RenderHold(ctx, destRect, alpha);
      return;
   }
   if (m_holdTexture)
   {
      DeleteTexture(m_holdTexture);
      m_holdTexture = nullptr;
   }

   const VPXTextureInfo* texInfo = GetTextureInfo(selectedFrame.texture);
   ctx->DrawImage(ctx, selectedFrame.texture, 1.f, 1.f, 1.f, alpha, 0.f, 0.f, static_cast<float>(texInfo->width), static_cast<float>(texInfo->height), 0.f, 0.f, 0.f,
      static_cast<float>(destRect.x), static_cast<float>(destRect.y), static_cast<float>(destRect.w), static_cast<float>(destRect.h));

   selectedFrame.age = 0;
   m_position = selectedFrame.pts + m_videoPtsBase;
}

void PUPMediaPlayer::Run()
{
// Main loop which loops over read/decode and handles video seeking/looping
#ifdef _DEBUG
   string name;
   bool paused = false;
#endif
   while (true)
   {
      // Interact with main thread
      bool loop;
      bool safeLoop;
      double seek;
      {
         std::unique_lock<std::mutex> lock(m_mutex);
         #ifdef _DEBUG
            if ((name != m_name) || (paused != m_paused))
            {
               name = m_name;
               paused = m_paused;
               SetThreadName(m_name + (paused ? ".Paused" : ".Play"));
            }
         #endif
         if (!m_running)
            break;
         if (m_paused)
         {
            lock.unlock();
            SDL_Delay(100);
            continue;
         }
         loop = m_loop;
         safeLoop = m_safeLoop;
         seek = m_seekRequest.exchange(-1.0);
      }

      if (seek >= 0.0)
      {
         if (mediaApi->Seek(m_media, seek) < 0)
            LOGE("Unable to seek: " + PluginStrings::PathToUTF8(m_filename.filename()));
         m_playIndex++;
         StopAudioStream(m_audioStreamId);
         std::lock_guard lock(m_mutex);
         m_startTimestamp = m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0);
         m_videoPtsBaseValid = false;
         m_videoPtsBase = 0.0;
         m_audioPtsBaseValid = false;
         m_audioPtsBase = 0.0;
         m_lastAudioPass = -1;
         m_lastDecodedPts = 0.0;
         m_position = seek;
         for (auto& frame : m_frames)
            frame.valid = false;
         continue;
      }

      // Read next decoded unit from the session
      MediaOutput output {};
      const int rdRet = mediaApi->Read(m_media, &output);
      if (rdRet < 0 || output.type == MEDIA_OUT_ERROR)
      {
         LOGE(std::format("Error reading frame: name={}, file={}, ret={}", m_name, PluginStrings::PathToUTF8(m_filename.filename()), rdRet));
         break;
      }

      bool endOfPass = (output.type == MEDIA_OUT_EOF);
      double safeLoopEnd = 0.0;
      if (!endOfPass && safeLoop && loop && m_duration > 1.0)
      {
         const int clockOut = m_hasVideo ? MEDIA_OUT_VIDEO : MEDIA_OUT_AUDIO;
         if (output.type == clockOut && output.pts - m_streamStartTime >= m_duration - 1.0)
         {
            if (output.type == MEDIA_OUT_VIDEO)
               mediaApi->ReleaseFrame(m_media, output.videoFrame);
            endOfPass = true;
            safeLoopEnd = output.pts;
         }
      }

      if (endOfPass)
      {
         // The session has already drained both decoders before reporting EOF, so the
         // last buffered frames have been queued and handled in stream order
         while (m_running && m_stillDuration > 0.0 && (m_paused || GetPlayTime() < m_stillDuration))
            SDL_Delay(8);
         if (!loop)
         {
            while (m_running && !m_paused && m_hasVideo && GetPlayTime() < m_lastDecodedPts + 0.05)
               SDL_Delay(8);
            break;
         }
         if (mediaApi->Seek(m_media, 0.0) < 0)
         {
            LOGE("Unable to loop: " + PluginStrings::PathToUTF8(m_filename.filename()));
            break;
         }
         m_playIndex++;
         if (safeLoopEnd == 0.0)
            m_loopCount++;
         double duration = safeLoopEnd;
         if (duration == 0.0 && m_hasVideo)
            duration = m_videoDuration;
         {
            std::lock_guard lock(m_mutex);
            if (m_stillDuration > 0.0)
               m_startTimestamp = m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0);
            else if (duration > 0.0)
            {
               // Keep the timeline continuous across the loop: the tail frames stay valid at
               // negative pts and play out while the next iteration's frames (pts 0..) decode.
               // A pass resumed mid file only lasted from its start position to the end
               const double pass = duration - m_videoPtsBase;
               m_videoPtsBase = 0.0;
               m_startTimestamp += pass;
               m_lastDecodedPts -= pass;
               for (auto& frame : m_frames)
                  frame.pts -= pass;
            }
            else
            {
               m_startTimestamp = m_syncOnGameTime ? m_gameTime : (static_cast<double>(SDL_GetTicks()) / 1000.0);
               for (auto& frame : m_frames)
                  frame.valid = false;
            }
            m_audioPtsBaseValid = false;
         }
         continue;
      }

      // Process decoded unit
      // TODO This should be done on ancillary threads to improve synchronization and better balance the load between CPU cores
      // Here, the synchronization entirely rely on the fact that HandleVideoFrame will block, waiting for a free slot in the circular frame buffer
      // This can cause audio overflow if there are no video (the stream is directly fully decoded) or audio glitches if the muxing is not great
      // (video waiting for frame while audio buffer is exhausted, this one being unlikely).
      if (output.type == MEDIA_OUT_AUDIO)
         HandleAudioOutput(output, !m_hasVideo);
      else if (output.type == MEDIA_OUT_VIDEO)
         HandleVideoFrame(output.videoFrame, output.pts);
   }

   {
      std::lock_guard lock(m_mutex);
      LOGD(std::format("Player stopped: name={}, file={}", m_name, PluginStrings::PathToUTF8(m_filename.filename())));
      m_running = false;
      m_endTicks = SDL_GetTicks();
      StopAudioStream(m_audioStreamId);
      m_onEndCallback();
   }
}

void PUPMediaPlayer::HandleVideoFrame(MediaFrame mediaFrame, double framePts)
{
   // Select the best buffer slot, eventually waiting for a free slot
   int selectedFrameSlot = -1;
   while (m_running)
   {
      const double playPTS = GetPlayTime();
      int maxAge = INT_MIN;
      for (int i = 0; i < static_cast<int>(m_frames.size()); i++)
      {
         if (!m_frames[i].valid)
         { // Unused slot => directly select
            maxAge = 2;
            selectedFrameSlot = i;
            break;
         }
         else if (m_frames[i].pts + 0.5 < playPTS)
         { // Far behind play time => reclaim regardless of age (screens that are never rendered do not age their slots)
            maxAge = 2;
            selectedFrameSlot = i;
            break;
         }
         else if ((m_frames[i].age > maxAge) && (m_frames[i].pts < playPTS))
         { // Search for the oldest slot with a frame behind play time
            maxAge = m_frames[i].age;
            selectedFrameSlot = i;
         }
      }
      if (maxAge >= 2)
         break;
      // This could be change by a condition variable, waiting on changes of either m_frames or m_running
      SDL_Delay(8);
   }
   if (!m_running)
   {
      mediaApi->ReleaseFrame(m_media, mediaFrame);
      return;
   }

   // Take ownership of the slot
   FrameInfo& selectedFrame = m_frames[selectedFrameSlot];
   std::shared_ptr<SDL_Surface> mask;
   {
      std::lock_guard lock(m_mutex);
      selectedFrame.valid = false;
      mask = m_mask;
   }

   if (selectedFrame.frame)
      mediaApi->ReleaseFrame(m_media, selectedFrame.frame);
   selectedFrame.frame = nullptr;

   if (mask)
   {
      // Materialize the frame to apply the transparency mask on CPU pixels before upload
      const uint8_t* src = nullptr;
      unsigned int width = 0;
      unsigned int height = 0;
      if (mediaApi->GetFramePixels(m_media, mediaFrame, &src, &width, &height) != 0)
      {
         mediaApi->ReleaseFrame(m_media, mediaFrame);
         return;
      }
      if (selectedFrame.pixels && (selectedFrame.width != width || selectedFrame.height != height))
      {
         free(selectedFrame.pixels);
         selectedFrame.pixels = nullptr;
      }
      if (selectedFrame.pixels == nullptr)
      {
         selectedFrame.pixels = static_cast<uint8_t*>(malloc(width * height * 4));
         if (selectedFrame.pixels == nullptr)
         {
            LOGE("Failed to allocate RGB buffer"s);
            mediaApi->ReleaseFrame(m_media, mediaFrame);
            m_running = false;
            return;
         }
      }
      memcpy(selectedFrame.pixels, src, static_cast<size_t>(width) * height * 4);
      selectedFrame.width = width;
      selectedFrame.height = height;
      mediaApi->ReleaseFrame(m_media, mediaFrame);

      // Apply the transparency mask
      SDL_Surface* sdlMask = mask.get();
      if (static_cast<int>(width) != sdlMask->w || static_cast<int>(height) != sdlMask->h)
      {
         if (m_scaledMask == nullptr || static_cast<int>(width) != m_scaledMask->w || static_cast<int>(height) != m_scaledMask->h)
            m_scaledMask = std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)>(
               SDL_ScaleSurface(mask.get(), static_cast<int>(width), static_cast<int>(height), SDL_ScaleMode::SDL_SCALEMODE_LINEAR), SDL_DestroySurface);
         sdlMask = m_scaledMask.get();
      }
      if (sdlMask)
      {
         SDL_LockSurface(sdlMask);
         const uint8_t* __restrict maskRow = static_cast<const uint8_t*>(sdlMask->pixels);
         uint8_t* __restrict frameRow = selectedFrame.pixels;
         for (int i = 0; i < sdlMask->h; i++)
         {
            const uint32_t* __restrict maskPtr = reinterpret_cast<const uint32_t*>(maskRow);
            uint32_t* __restrict frame2 = reinterpret_cast<uint32_t*>(frameRow);
            for (int j = 0; j < sdlMask->w; j++, maskPtr++, frame2++)
               if (!*maskPtr)
                  *frame2 = 0x00000000u;
            maskRow += sdlMask->pitch;
            frameRow += width * 4;
         }
         SDL_UnlockSurface(sdlMask);
      }
   }
   else
   {
      // Keep the lazy frame handle: pixels will be materialized on the render thread,
      // directly into the texture by UpdateTextureFromFrame
      selectedFrame.frame = mediaFrame;
   }

   // Update frame information & mark it as valid for selection by the renderer thread
   {
      std::lock_guard lock(m_mutex);
      selectedFrame.age = 0;
      if (!m_videoPtsBaseValid)
      {
         m_videoPtsBase = framePts;
         m_videoPtsBaseValid = true;
      }
      selectedFrame.pts = framePts - m_videoPtsBase;
      selectedFrame.uploaded = false;
      selectedFrame.valid = true;
      m_lastDecodedPts = selectedFrame.pts;
      // If playback ran ahead of decoding, resync the clock to the freshest frame instead of
      // fast-forwarding through the deficit. Only when rendering is current, or during the first
      // second before the first render (start delayed by a load stall); a screen that is never
      // displayed plays on its own clock. Render stalls are compensated at render resume so
      // correcting them here too would double count
      if (m_firstRenderPending && SDL_GetTicks() - m_playTicks > 1000)
         m_firstRenderPending = false;
      const double deficit = GetPlayTime() - selectedFrame.pts;
      if (deficit > 0.25 && (m_lastRenderTicks == 0 ? m_firstRenderPending.load() : SDL_GetTicks() - m_lastRenderTicks < 250))
         m_startTimestamp += deficit;
   }
}

void PUPMediaPlayer::HandleAudioOutput(const MediaOutput& output, bool sync)
{
   // If we have decoded enough data and we did not loop, then wait (if requested). Our aim is to enqueue up to our playing position + a reasonnable buffer
   if (sync && output.pts >= 0.0)
   {
      if (!m_audioPtsBaseValid)
      {
         m_audioPtsBase = output.pts;
         m_audioPtsBaseValid = true;
      }
      m_position = output.pts;
   }
   if (sync && m_lastAudioPass == m_playIndex && output.pts >= 0.0)
   {
      const double framePTS = output.pts - m_audioPtsBase;
      const double decodeTS = GetPlayTime() + 0.5; // Target + 500ms buffer
      if (framePTS > decodeTS)
         SDL_Delay(static_cast<uint32_t>(1000.0 * (framePTS - decodeTS)));
      if (!m_running)
         return;
   }
   m_lastAudioPass = m_playIndex;

   if (output.audioSize > 0 && m_volume > 0.0f)
   {
      uint8_t* buffer = static_cast<uint8_t*>(malloc(output.audioSize));
      if (!buffer)
         return;
      memcpy(buffer, output.audioData, output.audioSize);
      AudioUpdateMsg* audioUpdate = new AudioUpdateMsg();
      audioUpdate->streamId.resId = m_audioStreamId;
      audioUpdate->channelFormat = CTLPI_AUDIO_FORMAT_CHANNEL_STEREO;
      audioUpdate->sampleFormat = output.audioSampleFormat;
      audioUpdate->sampleRate = output.audioSampleRate;
      audioUpdate->bufferSize = output.audioSize;
      audioUpdate->buffer = buffer;
      audioUpdate->volume = m_volume / 100.0f;
      UpdateAudioStream(audioUpdate);
   }
}

}
