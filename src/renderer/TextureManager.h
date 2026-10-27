// license:GPLv3+

#pragma once

#include <deque>
#include <functional>
#include <mutex>

#include "unordered_dense.h"

#include "Texture.h"
#include "Sampler.h"
#include "typedefs3D.h"

class RenderDevice;

class TextureManager final
{
public:
   TextureManager(RenderDevice& rd) : m_rd(rd)
   { }

   ~TextureManager()
   {
      UnloadAll();
   }

   void AddPlaceHolder(ITexManCacheable* memtex);
   void AddPendingUpload(ITexManCacheable* memtex, std::shared_ptr<const BaseTexture> bitmap = nullptr);
   std::shared_ptr<Sampler> LoadTexture(ITexManCacheable* const memtex, const bool force_linear_rgb);
   void SetDirty(ITexManCacheable* memtex);
   void UnloadTexture(ITexManCacheable* memtex);
   void UnloadAll();

   vector<ITexManCacheable*> GetLoadedTextures() const;
   bool IsLinearRGB(ITexManCacheable* memtex) const;

   // Notification sink, wired by the owner (Player routes it to LiveUI::PushNotification):
   // (message, lengthMs, reuseId) -> notification id
   std::function<unsigned int(const string&, int, unsigned int)> m_notify;

private:
   // A texture may be uploaded twice, once decoded as sRGB and once kept linear (see LoadTexture's force_linear_rgb).
   // Per variant state is indexed rather than paired up, so that state added later cannot silently apply to one variant only
   enum SamplerVariant : unsigned int { VARIANT_SRGB = 0, VARIANT_LINEAR = 1, VARIANT_COUNT = 2 };
   static constexpr SamplerVariant SamplerVariantOf(const bool force_linear_rgb) { return force_linear_rgb ? VARIANT_LINEAR : VARIANT_SRGB; }

   struct MapEntry
   {
      std::shared_ptr<Sampler> samplers[VARIANT_COUNT];
      bool dirty[VARIANT_COUNT] = { false, false };
      ITexManCacheable* tex = nullptr;
      bool isPlaceHolder = false;
      std::shared_ptr<const class BaseTexture> pendingUpload;
      uint32_t lastUsedFrame = 0; // Frame index of the last use, for LRU eviction
      size_t gpuBytes[VARIANT_COUNT] = { 0, 0 }; // Estimated resident bytes of each variant
      size_t fullBytes[VARIANT_COUNT] = { 0, 0 }; // Estimated bytes at full size of a variant uploaded downsized or as fallback, 0 if not degraded
      uint32_t upgradeRetryFrame = 0; // Frame index before which a degraded variant is not retried at full size
   };
   RenderDevice& m_rd;
   // Guards m_map, as loader threads register and upload textures while the main thread renders the loading UI.
   // Held for whole calls: code run under it (sampler creation, bitmap decode) must not call back into the manager.
   mutable std::mutex m_mapMutex;
   ankerl::unordered_dense::map<uint64_t, MapEntry> m_map;
   typedef ankerl::unordered_dense::map<uint64_t, MapEntry>::iterator Iter;
   typedef ankerl::unordered_dense::map<uint64_t, MapEntry>::const_iterator CIter;

#if defined(ENABLE_BGFX)
#pragma region GPU memory budget management (BGFX only)

   // Vulkan does not provide a driver managed fallback to system memory: when device memory is
   // exhausted, resource creation fails and the backend crashes. Other backends page GPU memory
   // instead, which avoids the crash but degrades to unusable framerates. To address both, the
   // resident texture set is kept inside a safety share of the budget reported through bgfx::Stats
   // (VK_EXT_memory_budget, QueryVideoMemoryInfo or NvAPI, depending on the backend and GPU): least
   // recently used samplers are evicted first (they are transparently re-uploaded on next use), then
   // images are downsized, and only then degraded to the fallback texture, instead of crashing.
   // On backends that do not report a budget (OpenGL, Metal, WebGPU, D3D11 on non NVIDIA GPUs),
   // gpuMemoryMax stays <= 0 and uploads remain unmanaged as before.

   size_t EstimateGpuBytes(const BaseTexture* tex) const;
   std::shared_ptr<Sampler> CreateSampler(MapEntry& entry, SamplerVariant variant, bool force_linear_rgb, std::shared_ptr<const BaseTexture> bitmap);

   void RefreshBudget(); // Samples bgfx::getStats(), at most once per frame
   int64_t BudgetCap() const { return (m_gpuMemBudget * kBudgetCapPct) / 100; }
   int64_t ProjectedUse(int64_t added) const; // Best effort estimate of the usage once deferred frees have reached the driver
   bool MakeRoom(size_t needed); // Evicts LRU samplers until 'needed' fits the budget cap
   void NotifyPressure();

   static constexpr int kBudgetCapPct = 85; // Headroom left for render targets, buffers and transient growth
   static constexpr int kEvictTargetPct = 70; // Hysteresis: evict down to this share of the budget
   static constexpr uint32_t kEvictGraceFrames = 3; // Do not evict samplers used this recently
   static constexpr uint32_t kFreeLatencyFrames = 4; // Frames for a bgfx::destroy to reach the driver
   static constexpr uint32_t kUpgradeRetryFrames = 60; // Delay between attempts to restore a degraded variant to full size

   int64_t m_gpuMemBudget = -1; // bgfx::Stats::gpuMemoryMax, <= 0 when unmanaged
   int64_t m_gpuMemUsed = -1; // bgfx::Stats::gpuMemoryUsed (whole process, including non texture memory)
   int64_t m_otherBytes = 0; // gpuMemUsed share neither owned by this manager nor pending free (render targets, buffers, staging, ...)
   int64_t m_managedBytes = 0; // Estimated bytes held by the samplers this manager uploaded
   uint32_t m_statsFrame = 0; // Frame index of the last stats sample, also used as the LRU clock
   std::deque<std::pair<uint32_t, int64_t>> m_pendingFree; // Evicted bytes not yet freed by the driver (frame, bytes)
   unsigned int m_pressureNotifId = 0;
   unsigned int m_downsizeNotifId = 0;
   uint32_t m_pressureNotifCooldown = 0;

#pragma endregion
#endif
};
