// license:GPLv3+

#pragma once

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
   void AddPendingUpload(ITexManCacheable* memtex);
   std::shared_ptr<Sampler> LoadTexture(ITexManCacheable* const memtex, const bool force_linear_rgb);
   void SetDirty(ITexManCacheable* memtex);
   void UnloadTexture(ITexManCacheable* memtex);
   void UnloadAll();

   vector<ITexManCacheable*> GetLoadedTextures() const;
   bool IsLinearRGB(ITexManCacheable* memtex) const;

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
   };
   RenderDevice& m_rd;
   ankerl::unordered_dense::map<uint64_t, MapEntry> m_map;
   typedef ankerl::unordered_dense::map<uint64_t, MapEntry>::iterator Iter;
   typedef ankerl::unordered_dense::map<uint64_t, MapEntry>::const_iterator CIter;
};
