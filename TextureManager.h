#pragma once

#include <inc/robin_hood.h>

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

private:
   RenderDevice& m_rd;
   robin_hood::unordered_map<BaseTexture*, Sampler*> m_map;
   typedef robin_hood::unordered_map<BaseTexture*, Sampler*>::iterator Iter;
};
