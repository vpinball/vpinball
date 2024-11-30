// license:GPLv3+

#pragma once

#include "robin_hood.h"

#include "core/stdafx.h"
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

   Sampler* LoadTexture(BaseTexture* const memtex, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb);
   void SetDirty(BaseTexture* memtex);
   void UnloadTexture(BaseTexture* memtex);
   void UnloadAll();

   vector<BaseTexture*> GetLoadedTextures() const;
   bool IsLinearRGB(BaseTexture* memtex) const;

private:
   struct MapEntry
   {
      string name;
      Sampler* sampler;
      bool forceLinearRGB;
   };
   RenderDevice& m_rd;
   robin_hood::unordered_map<BaseTexture*, MapEntry> m_map;
   typedef robin_hood::unordered_map<BaseTexture*, MapEntry>::iterator Iter;
};
