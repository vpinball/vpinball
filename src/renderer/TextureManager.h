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
   Sampler* LoadTexture(ITexManCacheable* const memtex, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb);
   void SetDirty(ITexManCacheable* memtex);
   void UnloadTexture(ITexManCacheable* memtex);
   void UnloadAll();

   vector<ITexManCacheable*> GetLoadedTextures() const;
   bool IsLinearRGB(ITexManCacheable* memtex) const;

private:
   struct MapEntry
   {
      Sampler* sampler = nullptr;
      bool forceLinearRGB = false;
      ITexManCacheable* tex = nullptr;
      bool isPlaceHolder = false;
      std::shared_ptr<class BaseTexture> pendingUpload;
   };
   RenderDevice& m_rd;
   ankerl::unordered_dense::map<unsigned long long, MapEntry> m_map;
   typedef ankerl::unordered_dense::map<unsigned long long, MapEntry>::iterator Iter;
};
