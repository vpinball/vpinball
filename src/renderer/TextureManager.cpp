#include "stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"
#include "typedefs3D.h"

Sampler* TextureManager::LoadTexture(BaseTexture* memtex, const SamplerFilter filter, const SamplerAddressMode clampU, const SamplerAddressMode clampV, const bool force_linear_rgb)
{
   const Iter it = m_map.find(memtex);
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.sampler = new Sampler(&m_rd, memtex, force_linear_rgb, clampU, clampV, filter);
      if (g_pplayer->m_renderer && g_pplayer->m_renderer->m_envTexture != nullptr && g_pplayer->m_renderer->m_envTexture->m_pdsBuffer == memtex)
         entry.sampler->SetName("Env"s);
      else if (g_pplayer->m_renderer && g_pplayer->m_renderer->m_pinballEnvTexture.m_pdsBuffer == memtex)
         entry.sampler->SetName("Default Ball Env"s);
      else if (g_pplayer->m_texdmd == memtex)
         entry.sampler->SetName("DMD"s);
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
      entry.sampler->m_dirty = false;
      entry.clampU = clampU;
      entry.clampV = clampV;
      entry.filter = filter;
      entry.forceLinearRGB = force_linear_rgb;
      entry.preRenderOnly = g_pplayer->m_renderer && g_pplayer->m_renderer->IsRenderPass(Renderer::STATIC_ONLY);
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
      entry.sampler->SetFilter(filter);
      entry.clampU = clampU;
      entry.clampV = clampV;
      entry.filter = filter;
      entry.forceLinearRGB = force_linear_rgb;
      entry.preRenderOnly &= g_pplayer->m_renderer && g_pplayer->m_renderer->IsRenderPass(Renderer::STATIC_ONLY);
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

SamplerFilter TextureManager::GetFilter(BaseTexture* memtex) const
{
   auto it = m_map.find(memtex);
   return it == m_map.end() ? SamplerFilter::SF_UNDEFINED : it->second.filter;
}

SamplerAddressMode TextureManager::GetClampU(BaseTexture* memtex) const
{
   auto it = m_map.find(memtex);
   return it == m_map.end() ? SamplerAddressMode::SA_UNDEFINED : it->second.clampU;
}

SamplerAddressMode TextureManager::GetClampV(BaseTexture* memtex) const
{
   auto it = m_map.find(memtex);
   return it == m_map.end() ? SamplerAddressMode::SA_UNDEFINED : it->second.clampV;
}

bool TextureManager::IsLinearRGB(BaseTexture* memtex) const
{
   auto it = m_map.find(memtex);
   return it == m_map.end() ? false : it->second.forceLinearRGB;
}

bool TextureManager::IsPreRenderOnly(BaseTexture* memtex) const
{
   auto it = m_map.find(memtex);
   return it == m_map.end() ? false : it->second.preRenderOnly;
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
