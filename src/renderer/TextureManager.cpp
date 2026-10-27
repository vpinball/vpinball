// license:GPLv3+

#include "core/stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"

#if defined(ENABLE_BGFX)
#include "TextureCompressor.h" // CompressedTexture
#endif

std::shared_ptr<Sampler> TextureManager::LoadTexture(ITexManCacheable* const memtex, const bool force_linear_rgb)
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
   std::shared_ptr<Sampler>& sampler = entry.samplers[variant];
#if defined(ENABLE_BGFX)
   RefreshBudget();
   entry.lastUsedFrame = m_statsFrame;
#endif

   if (entry.pendingUpload)
   {
      // Consumed by whichever variant asks first, dropping the strong reference that was keeping the decoded
      // bitmap alive. A second variant requested later therefore falls in the branch below and calls
      // GetRawBitmap(), which only holds the bitmap weakly and so re-decodes the image from its compressed
      // data. Content is still correct, but will be a decode on the render thread then
#if defined(ENABLE_BGFX)
      sampler = CreateSampler(entry, variant, force_linear_rgb, std::move(entry.pendingUpload));
#else
      sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), m_rd.OrFallback(std::move(entry.pendingUpload)), force_linear_rgb);
#endif
   }
   else if (sampler == nullptr)
   {
#if defined(ENABLE_BGFX)
      sampler = CreateSampler(entry, variant, force_linear_rgb, memtex->GetRawBitmap(false, 0));
#else
      sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), m_rd.OrFallback(memtex->GetRawBitmap(false, 0)), force_linear_rgb);
#endif
   }
   else if (entry.dirty[variant])
   {
      entry.dirty[variant] = false; // Only the variant actually refreshed below, the other keeps waiting its turn
      std::shared_ptr<const BaseTexture> bitmap = m_rd.OrFallback(memtex->GetRawBitmap(false, 0));
      sampler->UpdateTexture(bitmap, force_linear_rgb);
#if defined(ENABLE_BGFX)
      const size_t est = EstimateGpuBytes(bitmap.get());
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
         sampler = CreateSampler(entry, variant, force_linear_rgb, memtex->GetRawBitmap(false, 0));
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
      entry.pendingUpload = bitmap ? std::move(bitmap) : memtex->GetRawBitmap(false, 0);
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
      // Uses the shared fallback tex, so a failed load and an uncreatable texture look alike.
      // Both variants share the one sampler, and it is replaced as soon as the real texture loads
      entry.samplers[VARIANT_SRGB]   = std::make_shared<Sampler>(&m_rd, memtex->GetName(), m_rd.m_fallbackTexture, false);
      entry.samplers[VARIANT_LINEAR] = entry.samplers[VARIANT_SRGB];
      entry.isPlaceHolder = true;
      entry.tex = memtex;
      m_map[memtex->GetLiveHash()] = entry;
   }
}

vector<ITexManCacheable*> TextureManager::GetLoadedTextures() const
{
   const std::lock_guard lock(m_mapMutex);
   std::vector<ITexManCacheable*> keys;
   for (const auto& it : m_map)
      if (!it.second.isPlaceHolder && !it.second.pendingUpload)
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
   if (it != m_map.end())
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
   m_managedBytes = 0; // m_pendingFree is kept as the deferred destroys still reach the driver
#endif
}

#if defined(ENABLE_BGFX)
#pragma region GPU memory budget management (BGFX only)

size_t TextureManager::EstimateGpuBytes(const BaseTexture* tex) const
{
   if (tex == nullptr)
      return 0;
   if (tex->m_compressed && m_rd.m_compressTextures) // Same condition as Sampler's precompressed upload
      return tex->m_compressed->data.size() + tex->m_compressed->GetMipSize(0); // Mip chain plus the no-mips alias
   // Bytes per texel of the GPU format (UncompressedBgfxFormat in Sampler.cpp), not of the bitmap: SRGB and SRGB565
   // upload as RGBA8, RGB float as RGBA float, and RGB8 is padded to 4 bytes by most drivers
   size_t texelBytes;
   switch (tex->m_format)
   {
   case BaseTexture::BW: texelBytes = 1; break;
   case BaseTexture::RGB_FP16:
   case BaseTexture::RGBA_FP16: texelBytes = 8; break;
   case BaseTexture::RGB_FP32:
   case BaseTexture::RGBA_FP32: texelBytes = 16; break;
   default: texelBytes = 4; break; // BW_FP32, RGB, SRGB, RGBA, SRGBA, SRGB565
   }
   return (texelBytes * tex->width() * tex->height() * 4) / 3; // Base level plus ~1/3 for the mip chain
}

void TextureManager::RefreshBudget()
{
   const bgfx::Stats* stats = bgfx::getStats();
   if (stats == nullptr || stats->gpuFrameNum == m_statsFrame)
      return;
   m_statsFrame = stats->gpuFrameNum;
   m_gpuMemBudget = stats->gpuMemoryMax;
   m_gpuMemUsed = stats->gpuMemoryUsed;
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
   size_t est = EstimateGpuBytes(bitmap.get());
   entry.fullBytes[variant] = 0;

   if (!MakeRoom(est))
   {
      // Out of budget even after evicting: try a downsized decode before degrading to the fallback texture.
      // Half of the loaded size, not of the declared size, which is larger when capped by the max texture dimension setting
      entry.fullBytes[variant] = est;
      entry.upgradeRetryFrame = m_statsFrame + kUpgradeRetryFrames;
      const unsigned int halfDim = std::max(std::max(bitmap->width(), bitmap->height()), 2 * MIN_TEXTURE_SIZE) / 2;
      if (std::shared_ptr<const BaseTexture> downsized = entry.tex->GetRawBitmap(true, halfDim);
         downsized != nullptr && (downsized->width() < bitmap->width() || downsized->height() < bitmap->height()) && MakeRoom(EstimateGpuBytes(downsized.get())))
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
      est = EstimateGpuBytes(bitmap.get());
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
