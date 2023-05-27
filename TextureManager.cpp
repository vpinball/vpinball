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
      Sampler* sampler = new Sampler(&m_rd, memtex, force_linear_rgb, clampU, clampV, filter);
      if (g_pplayer->m_pin3d.m_envTexture != nullptr && g_pplayer->m_pin3d.m_envTexture->m_pdsBuffer == memtex)
         sampler->SetName("Env"s);
      else
      {
         for (Texture* image : g_pplayer->m_ptable->m_vimage)
         {
            if (image->m_pdsBuffer == memtex)
            {
               sampler->SetName(image->m_szName);
               break;
            }
         }
      }
      sampler->m_dirty = false;
      m_map[memtex] = sampler;
      return sampler;
   }
   else
   {
      Sampler* sampler = it->second;
      if (sampler->m_dirty)
      {
         sampler->UpdateTexture(memtex, force_linear_rgb);
         sampler->m_dirty = false;
      }
      sampler->SetClamp(clampU, clampV);
      sampler->SetFilter(filter);
      return sampler;
   }
}

void TextureManager::SetDirty(BaseTexture* memtex)
{
   const Iter it = m_map.find(memtex);
   if (it != m_map.end())
      it->second->m_dirty = true;
}

void TextureManager::UnloadTexture(BaseTexture* memtex)
{
   const Iter it = m_map.find(memtex);
   if (it != m_map.end())
   {
      delete it->second;
      m_map.erase(it);
   }
}

void TextureManager::UnloadAll()
{
   for (Iter it = m_map.begin(); it != m_map.end(); ++it)
      delete it->second;
   m_map.clear();
}
