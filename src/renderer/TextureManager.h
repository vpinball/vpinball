#pragma once

#include "robin_hood.h"

#include "stdafx.h"
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

   Sampler* LoadTexture(BaseTexture* memtex, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb);
   void SetDirty(BaseTexture* memtex);
   void UnloadTexture(BaseTexture* memtex);
   void UnloadAll();

   vector<BaseTexture*> GetLoadedTextures() const;
   SamplerFilter GetFilter(BaseTexture* memtex) const;
   SamplerAddressMode GetClampU(BaseTexture* memtex) const;
   SamplerAddressMode GetClampV(BaseTexture* memtex) const;
   bool IsLinearRGB(BaseTexture* memtex) const;
   bool IsPreRenderOnly(BaseTexture* memtex) const;

private:
   struct MapEntry
   {
      Sampler* sampler;
      string name;
      SamplerFilter filter;
      SamplerAddressMode clampU, clampV;
      bool forceLinearRGB;
      bool preRenderOnly;
   };
   RenderDevice& m_rd;
   robin_hood::unordered_map<BaseTexture*, MapEntry> m_map;
   typedef robin_hood::unordered_map<BaseTexture*, MapEntry>::iterator Iter;
};
