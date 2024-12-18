// license:GPLv3+

#include "core/stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"
#include "typedefs3D.h"

Sampler* TextureManager::LoadTexture(BaseTexture* const memtex, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   const Iter it = m_map.find(memtex);
   // During static part prerendering, trilinear/anisotropic filtering is disabled to get sharper results
   const bool isPreRender = g_pplayer->m_renderer && g_pplayer->m_renderer->IsRenderPass(Renderer::STATIC_ONLY);
   const SamplerFilter filter2 = (isPreRender && (filter == SamplerFilter::SF_ANISOTROPIC || filter == SamplerFilter::SF_TRILINEAR)) ? SamplerFilter::SF_BILINEAR : filter;
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.sampler = new Sampler(&m_rd, memtex, force_linear_rgb, clampU, clampV, filter2);
      #ifdef DEBUG
      if (g_pplayer->m_renderer && g_pplayer->m_renderer->m_envTexture != nullptr && g_pplayer->m_renderer->m_envTexture->m_pdsBuffer == memtex)
         entry.sampler->SetName("Env"s);
      else if (g_pplayer->m_renderer && g_pplayer->m_renderer->m_pinballEnvTexture.m_pdsBuffer == memtex)
         entry.sampler->SetName("Default Ball Env"s);
      else if (g_pplayer->m_dmdFrame == memtex)
         entry.sampler->SetName("Script DMD"s);
      else
      {
         for (Texture* image : g_pplayer->m_ptable->m_vimage)
         {
            if (image->m_pdsBuffer == memtex)
            {
               entry.name = image->m_szName;
               entry.sampler->SetName(image->m_szName);
               break;
            }
         }
      }
      #endif
      entry.sampler->m_dirty = false;
      entry.forceLinearRGB = force_linear_rgb;
      m_map[memtex] = entry;
      return entry.sampler;
   }
   else
   {
      MapEntry& entry = it->second;
      if (entry.sampler->m_dirty)
      {
         entry.sampler->UpdateTexture(memtex, force_linear_rgb);
         entry.sampler->m_dirty = false;
      }
      entry.sampler->SetClamp(clampU, clampV);
      entry.sampler->SetFilter(filter2);
      entry.forceLinearRGB = force_linear_rgb;
      return entry.sampler;
   }
}

vector<BaseTexture*> TextureManager::GetLoadedTextures() const
{
   std::vector<BaseTexture*> keys;
   for (auto it = m_map.begin(); it != m_map.end(); ++it)
      keys.push_back(it->first);
   return keys;
}

bool TextureManager::IsLinearRGB(BaseTexture* memtex) const
{
   auto it = m_map.find(memtex);
   return it == m_map.end() ? false : it->second.forceLinearRGB;
}

void TextureManager::SetDirty(BaseTexture* memtex)
{
   const Iter it = m_map.find(memtex);
   if (it != m_map.end())
      it->second.sampler->m_dirty = true;
}

void TextureManager::UnloadTexture(BaseTexture* memtex)
{
   const Iter it = m_map.find(memtex);
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
