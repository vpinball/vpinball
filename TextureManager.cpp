#include "stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"
#include "typedefs3D.h"

Sampler* TextureManager::LoadTexture(BaseTexture* memtex, const TextureFilter filter, const bool clampU, const bool clampV, const bool force_linear_rgb)
{
   const Iter it = m_map.find(memtex);
   if (it == m_map.end())
   {
      Sampler* sampler = new Sampler(&m_rd, memtex, force_linear_rgb);
      if (sampler)
      {
         sampler->m_dirty = false;
         m_map[memtex] = sampler;
      }
      return sampler;
   }
   else
   {
      if (it->second->m_dirty)
      {
         it->second->UpdateTexture(memtex, force_linear_rgb);
         it->second->m_dirty = false;
      }
      return it->second;
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
