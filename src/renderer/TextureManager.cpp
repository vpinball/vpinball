// license:GPLv3+

#include "core/stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"
#include "typedefs3D.h"

Sampler* TextureManager::LoadTexture(ITexManCacheable* const memtex, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   const Iter it = m_map.find(memtex->GetLiveHash());
   // During static part prerendering, trilinear/anisotropic filtering is disabled to get sharper results
   const bool isPreRender = g_pplayer->m_renderer && g_pplayer->m_renderer->IsRenderPass(Renderer::STATIC_ONLY);
   const SamplerFilter filter2 = (isPreRender && (filter == SamplerFilter::SF_ANISOTROPIC || filter == SamplerFilter::SF_TRILINEAR)) ? SamplerFilter::SF_BILINEAR : filter;
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.sampler = new Sampler(&m_rd, memtex->GetRawBitmap(), force_linear_rgb, clampU, clampV, filter2);
      entry.sampler->SetName(memtex->GetName());
      entry.sampler->m_dirty = false;
      entry.forceLinearRGB = force_linear_rgb;
      entry.tex = memtex;
      m_map[memtex->GetLiveHash()] = entry;
      return entry.sampler;
   }
   else
   {
      MapEntry& entry = it->second;
      if (entry.sampler->m_dirty)
      {
         entry.sampler->UpdateTexture(memtex->GetRawBitmap(), force_linear_rgb);
         entry.sampler->m_dirty = false;
      }
      entry.sampler->SetClamp(clampU, clampV);
      entry.sampler->SetFilter(filter2);
      entry.forceLinearRGB = force_linear_rgb;
      return entry.sampler;
   }
}

vector<ITexManCacheable*> TextureManager::GetLoadedTextures() const
{
   std::vector<ITexManCacheable*> keys;
   for (auto it = m_map.begin(); it != m_map.end(); ++it)
      keys.push_back(it->second.tex);
   return keys;
}

bool TextureManager::IsLinearRGB(ITexManCacheable* memtex) const
{
   const auto it = m_map.find(memtex->GetLiveHash());
   return it == m_map.end() ? false : it->second.forceLinearRGB;
}

void TextureManager::SetDirty(ITexManCacheable* memtex)
{
   const Iter it = m_map.find(memtex->GetLiveHash());
   if (it != m_map.end())
      it->second.sampler->m_dirty = true;
}

void TextureManager::UnloadTexture(ITexManCacheable* memtex)
{
   const Iter it = m_map.find(memtex->GetLiveHash());
   if (it != m_map.end())
   {
      delete it->second.sampler;
      m_map.erase(it);
   }
}

void TextureManager::UnloadAll()
{
   for (Iter it = m_map.begin(); it != m_map.end(); ++it)
      delete it->second.sampler;
   m_map.clear();
}
