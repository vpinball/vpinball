// license:GPLv3+

#include "core/stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"

#if defined(ENABLE_BGFX)
#include "TextureCompressor.h"
#endif

std::shared_ptr<const BaseTexture> TextureManager::LoadBitmapCC(ITexManCacheable* memtex, const bool resizeOnLowMem, const bool allowCompression,
   const std::function<bool()>& isCompressionDiscarded) const
{
#if defined(ENABLE_BGFX)
   // Table images are compressed (and cached) while loading the table. Later loads (after an eviction, for the other sampler variant,
   // or of an image that was not preloaded) read the disk cache: faster than decoding, and as small on the GPU. They do not compress an
   // uncached image, as it may take seconds
   if (m_compressor && m_rd.m_compressTextures)
      if (Texture* const image = dynamic_cast<Texture*>(memtex); image != nullptr)
      {
         if (allowCompression)
            return m_compressor->LoadOrCompress(image, m_maxTexDimension, resizeOnLowMem, isCompressionDiscarded);
         if (std::shared_ptr<const BaseTexture> cached = m_compressor->LoadCached(image, m_maxTexDimension); cached != nullptr)
            return cached;
      }
#endif
   return memtex->GetRawBitmap(resizeOnLowMem, m_maxTexDimension);
}

void TextureManager::EndLoading(const vector<Texture*>& tableImages) const
{
#if defined(ENABLE_BGFX)
   if (m_compressor)
   {
      m_compressor->LogStats();
      m_compressor->CleanCache(tableImages, m_maxTexDimension);
   }
#endif
}

std::shared_ptr<Sampler> TextureManager::LoadTexture(ITexManCacheable* const memtex, const bool force_linear_rgb, const bool markUsed)
{
   const std::lock_guard lock(m_mapMutex);
   const uint64_t hash = memtex->GetLiveHash();
   const SamplerVariant variant = SamplerVariantOf(force_linear_rgb);
   Iter it = m_map.find(hash);
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.tex = memtex;
      it = m_map.emplace(hash, std::move(entry)).first;
   }

   MapEntry& entry = it->second;
   entry.used |= markUsed;
   std::shared_ptr<Sampler>& sampler = entry.samplers[variant];
#if defined(ENABLE_BGFX)
   RefreshBudget();
   entry.lastUsedFrame = m_statsFrame;
#endif

   if (entry.pendingUpload)
   {
      // Consumed by whichever variant asks first, dropping the strong reference that kept the bitmap alive. A second variant
      // requested later falls in the branch below (LoadBitmapCC: from the disk cache, or decoded again on the render thread)
#if defined(ENABLE_BGFX)
      sampler = CreateSampler(entry, variant, force_linear_rgb, std::move(entry.pendingUpload));
#else
      sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), m_rd.OrFallback(std::move(entry.pendingUpload)), force_linear_rgb);
#endif
   }
   else if (sampler == nullptr)
   {
#if defined(ENABLE_BGFX)
      sampler = CreateSampler(entry, variant, force_linear_rgb, LoadBitmapCC(memtex));
#else
      sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), m_rd.OrFallback(LoadBitmapCC(memtex)), force_linear_rgb);
#endif
   }
   else if (entry.dirty[variant])
   {
      entry.dirty[variant] = false; // Only the variant actually refreshed below, the other keeps waiting its turn
      std::shared_ptr<const BaseTexture> bitmap = m_rd.OrFallback(memtex->GetRawBitmap(false, m_maxTexDimension));
      sampler->UpdateTexture(bitmap, force_linear_rgb);
#if defined(ENABLE_BGFX)
      const size_t est = bitmap->GetEstimatedGPUSize();
      m_managedBytes += static_cast<int64_t>(est) - static_cast<int64_t>(entry.gpuBytes[variant]);
      entry.gpuBytes[variant] = est;
      entry.fullBytes[variant] = 0; // Now holds the full size content
   }
   else if (entry.fullBytes[variant] > 0 && m_statsFrame >= entry.upgradeRetryFrame)
   {
      // Degraded (downsized or fallback) under memory pressure: re-upload at full size once it fits below the
      // eviction target, not just the cap, so that it does not immediately evict other textures
      entry.upgradeRetryFrame = m_statsFrame + kUpgradeRetryFrames;
      if (m_gpuMemBudget > 0 && ProjectedUse(static_cast<int64_t>(entry.fullBytes[variant]) - static_cast<int64_t>(entry.gpuBytes[variant])) <= (m_gpuMemBudget * kEvictTargetPct) / 100)
      {
         m_managedBytes -= static_cast<int64_t>(entry.gpuBytes[variant]);
         m_pendingFree.emplace_back(m_statsFrame, static_cast<int64_t>(entry.gpuBytes[variant]));
         entry.gpuBytes[variant] = 0;
         sampler = CreateSampler(entry, variant, force_linear_rgb, LoadBitmapCC(memtex));
      }
#endif
   }
   return sampler;
}

void TextureManager::AddPendingUpload(ITexManCacheable* memtex, std::shared_ptr<const BaseTexture> bitmap)
{
   const std::lock_guard lock(m_mapMutex);
   const CIter it = m_map.find(memtex->GetLiveHash());
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.pendingUpload = bitmap ? std::move(bitmap) : memtex->GetRawBitmap(false, m_maxTexDimension);
      entry.tex = memtex;
      m_map[memtex->GetLiveHash()] = entry;
   }
}

void TextureManager::AddPlaceHolder(ITexManCacheable* memtex)
{
   const std::lock_guard lock(m_mapMutex);
   const CIter it = m_map.find(memtex->GetLiveHash());
   if (it == m_map.end())
   {
      MapEntry entry;
      // For an image that failed to load (even downsized on low memory): the shared fallback tex, so a failed load and an uncreatable
      // texture look alike, with one sampler for both variants. Neither evicted nor reloaded at full size: SetDirty drops it for a regular reload
      entry.samplers[VARIANT_SRGB]   = std::make_shared<Sampler>(&m_rd, memtex->GetName(), m_rd.m_fallbackTexture, false);
      entry.samplers[VARIANT_LINEAR] = entry.samplers[VARIANT_SRGB];
      entry.isPlaceHolder = true;
      entry.tex = memtex;
      m_map[memtex->GetLiveHash()] = entry;
   }
}

vector<ITexManCacheable*> TextureManager::GetUsedTextures() const
{
   const std::lock_guard lock(m_mapMutex);
   std::vector<ITexManCacheable*> keys;
   for (const auto& it : m_map)
      if (it.second.used && !it.second.isPlaceHolder)
         keys.push_back(it.second.tex);
   return keys;
}

bool TextureManager::IsLinearRGB(ITexManCacheable* memtex) const
{
   const std::lock_guard lock(m_mapMutex);
   const CIter it = m_map.find(memtex->GetLiveHash());
   // A placeholder aliases both variants onto a single sRGB sampler, so it is not a linear upload
   return it != m_map.end() && !it->second.isPlaceHolder && it->second.samplers[VARIANT_LINEAR] != nullptr;
}

void TextureManager::SetDirty(ITexManCacheable* memtex)
{
   const std::lock_guard lock(m_mapMutex);
   const Iter it = m_map.find(memtex->GetLiveHash());
   if (it == m_map.end())
      return;
   if (it->second.isPlaceHolder)
      m_map.erase(it); // Reloaded on next use as a regular entry: a refreshed placeholder would never be evicted nor listed as used
   else
      for (bool& dirty : it->second.dirty) // Every live variant needs refreshing, not just the next one drawn
         dirty = true;
}

void TextureManager::UnloadTexture(ITexManCacheable* memtex)
{
   const std::lock_guard lock(m_mapMutex);
   const CIter it = m_map.find(memtex->GetLiveHash());
   if (it != m_map.end())
   {
#if defined(ENABLE_BGFX)
      for (const size_t bytes : it->second.gpuBytes)
         if (bytes > 0)
         {
            m_managedBytes -= static_cast<int64_t>(bytes);
            m_pendingFree.emplace_back(m_statsFrame, static_cast<int64_t>(bytes));
         }
#endif
      m_map.erase(it);
   }
}

void TextureManager::UnloadAll()
{
   const std::lock_guard lock(m_mapMutex);
   m_map.clear();
#if defined(ENABLE_BGFX)
   m_compressor = nullptr;
   m_managedBytes = 0; // m_pendingFree is kept as the deferred destroys still reach the driver
#endif
}

#if defined(ENABLE_BGFX)
#pragma region GPU memory budget management (BGFX only)

void TextureManager::OnFrameStats(const bgfx::Stats* stats)
{
   if (stats == nullptr)
      return;
   // Memory first: a reader seeing the new frame number gets memory figures of that frame or a later one
   m_statsGpuMemMax.store(stats->gpuMemoryMax, std::memory_order_relaxed);
   m_statsGpuMemUsed.store(stats->gpuMemoryUsed, std::memory_order_relaxed);
   m_statsGpuFrameNum.store(stats->gpuFrameNum, std::memory_order_release);
}

void TextureManager::RefreshBudget()
{
   const uint32_t gpuFrameNum = m_statsGpuFrameNum.load(std::memory_order_acquire);
   if (gpuFrameNum == m_statsFrame)
      return;
   m_statsFrame = gpuFrameNum;
   // The budget management is only enabled on the Vulkan backend: it is the only backend without a driver managed
   // fallback to system memory, so resource creation fails (and the backend crashes) once device memory is
   // exhausted. All the other backends (D3D11, D3D12, OpenGL, Metal, WebGPU) page GPU memory to system RAM under
   // pressure instead of failing: they slow down but neither crash nor lose content, so keeping the management on
   // for them would only evict/downsize textures needlessly (the reported usage is not even a per-process figure
   // on some drivers, e.g. NvAPI reports adapter-wide usage on D3D11).
   m_gpuMemBudget = bgfx::getRendererType() == bgfx::RendererType::Vulkan ? m_statsGpuMemMax.load(std::memory_order_relaxed) : -1;
   m_gpuMemUsed = m_statsGpuMemUsed.load(std::memory_order_relaxed);
   // Destroyed samplers take a few frames before their memory is actually freed by the driver
   while (!m_pendingFree.empty() && m_statsFrame - m_pendingFree.front().first >= kFreeLatencyFrames)
      m_pendingFree.pop_front();
   int64_t pendingFree = 0;
   for (const auto& freed : m_pendingFree)
      pendingFree += freed.second;
   // gpuMemoryUsed covers the whole process: the share neither owned by this manager nor about to be freed
   // (render targets, buffers, staging, ...) acts as a baseline for projected usage between stats updates
   m_otherBytes = std::max<int64_t>(0, m_gpuMemUsed - m_managedBytes - pendingFree);
}

int64_t TextureManager::ProjectedUse(const int64_t added) const
{
   // Evicted bytes leave m_managedBytes right away, so that eviction actually makes room. They are still resident
   // for a few frames (kFreeLatencyFrames), a transient overshoot covered by the headroom above BudgetCap()
   return m_otherBytes + m_managedBytes + added;
}

bool TextureManager::MakeRoom(const size_t needed)
{
   if (m_gpuMemBudget <= 0 || ProjectedUse(static_cast<int64_t>(needed)) <= BudgetCap())
      return true;

   NotifyPressure();

   // Evict least recently used samplers, excluding placeholders (tiny shared fallback) and recently
   // used ones which are likely referenced by in-flight draws
   vector<std::pair<uint32_t, Iter>> victims;
   for (Iter it = m_map.begin(); it != m_map.end(); ++it)
   {
      const MapEntry& entry = it->second;
      if (!entry.isPlaceHolder && entry.lastUsedFrame + kEvictGraceFrames < m_statsFrame && (entry.samplers[VARIANT_SRGB] || entry.samplers[VARIANT_LINEAR]))
         victims.emplace_back(entry.lastUsedFrame, it);
   }
   std::sort(victims.begin(), victims.end(), [](const auto& a, const auto& b) { return a.first < b.first; }); // Least recently used first

   const int64_t target = (m_gpuMemBudget * kEvictTargetPct) / 100;
   for (const auto& victim : victims)
   {
      if (ProjectedUse(static_cast<int64_t>(needed)) <= target)
         break;
      MapEntry& entry = victim.second->second;
      for (unsigned int v = 0; v < VARIANT_COUNT; ++v)
         if (entry.samplers[v])
         {
            PLOGI << "GPU memory limit: evicting texture '" << entry.tex->GetName() << '\'';
            entry.samplers[v].reset();
            m_managedBytes -= static_cast<int64_t>(entry.gpuBytes[v]);
            m_pendingFree.emplace_back(m_statsFrame, static_cast<int64_t>(entry.gpuBytes[v]));
            entry.gpuBytes[v] = 0;
            entry.fullBytes[v] = 0;
            entry.dirty[v] = false;
         }
      // The entry stays registered: the texture is transparently re-uploaded on next use
   }

   return ProjectedUse(static_cast<int64_t>(needed)) <= BudgetCap();
}

std::shared_ptr<Sampler> TextureManager::CreateSampler(MapEntry& entry, const SamplerVariant variant, const bool force_linear_rgb, std::shared_ptr<const BaseTexture> bitmap)
{
   bitmap = m_rd.OrFallback(std::move(bitmap));
   size_t est = bitmap->GetEstimatedGPUSize();
   entry.fullBytes[variant] = 0;

   if (!MakeRoom(est))
   {
      // Out of budget even after evicting: try a downsized decode before degrading to the fallback texture.
      // Half of the loaded size, not of the declared size, which is larger when capped by the max texture dimension setting
      entry.fullBytes[variant] = est;
      entry.upgradeRetryFrame = m_statsFrame + kUpgradeRetryFrames;
      const unsigned int halfDim = std::max(std::max(bitmap->width(), bitmap->height()), 2 * MIN_TEXTURE_SIZE) / 2;
      if (std::shared_ptr<const BaseTexture> downsized = entry.tex->GetRawBitmap(true, halfDim);
         downsized != nullptr && (downsized->width() < bitmap->width() || downsized->height() < bitmap->height()) && MakeRoom(downsized->GetEstimatedGPUSize()))
      {
         PLOGW << "GPU memory limit: image '" << entry.tex->GetName() << "' was downsized to " << downsized->width() << 'x' << downsized->height();
         if (m_notify)
            m_downsizeNotifId = m_notify("Image '" + entry.tex->GetName() + "' was downsized due to low GPU memory"s, 5000, m_downsizeNotifId);
         bitmap = std::move(downsized);
      }
      else
      {
         PLOGW << "GPU memory limit: image '" << entry.tex->GetName() << "' skipped (out of GPU memory)";
         if (m_notify)
            m_downsizeNotifId = m_notify("Image '" + entry.tex->GetName() + "' could not be uploaded, out of GPU memory"s, 5000, m_downsizeNotifId);
         bitmap = m_rd.m_fallbackTexture;
      }
      est = bitmap->GetEstimatedGPUSize();
   }

   m_managedBytes += static_cast<int64_t>(est) - static_cast<int64_t>(entry.gpuBytes[variant]);
   entry.gpuBytes[variant] = est;
   return std::make_shared<Sampler>(&m_rd, entry.tex->GetName(), std::move(bitmap), force_linear_rgb);
}

void TextureManager::NotifyPressure()
{
   PLOGW << "GPU memory budget reached: " << (m_gpuMemUsed >> 20) << " / " << (m_gpuMemBudget >> 20) << " MB used, " << (m_managedBytes >> 20) << " MB of managed textures";
   const uint32_t now = msec();
   if (now < m_pressureNotifCooldown || !m_notify)
      return;
   m_pressureNotifCooldown = now + 30 * 1000; // Rate limit the notifications
   m_pressureNotifId = m_rd.m_compressTextures
      ? m_notify("GPU memory limit reached, releasing least recently used textures"s, 8000, m_pressureNotifId)
      : m_notify("GPU memory limit reached and 'Compress Textures' is disabled. Enabling it in the graphic settings reduces texture memory ~4x."s, 10000, m_pressureNotifId);
}

#pragma endregion
#endif
