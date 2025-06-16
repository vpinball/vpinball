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
   struct MapEntry
   {
      std::shared_ptr<Sampler> sampler = nullptr;
      std::shared_ptr<Sampler> linearSampler = nullptr;
      ITexManCacheable* tex = nullptr;
      bool isPlaceHolder = false;
      bool dirty = false;
      std::shared_ptr<const class BaseTexture> pendingUpload;
   };
   RenderDevice& m_rd;
   ankerl::unordered_dense::map<uint64_t, MapEntry> m_map;
   typedef ankerl::unordered_dense::map<uint64_t, MapEntry>::iterator Iter;
   typedef ankerl::unordered_dense::map<uint64_t, MapEntry>::const_iterator CIter;
};
