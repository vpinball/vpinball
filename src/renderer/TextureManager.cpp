// license:GPLv3+

#include "core/stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"

std::shared_ptr<Sampler> TextureManager::LoadTexture(ITexManCacheable* const memtex, const bool force_linear_rgb)
{
   const unsigned long long hash = memtex->GetLiveHash();
   const Iter it = m_map.find(hash);
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.tex = memtex;
      std::shared_ptr<Sampler>& sampler = force_linear_rgb ? entry.linearSampler : entry.sampler;
      sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), memtex->GetRawBitmap(false, 0), force_linear_rgb);
      m_map[hash] = entry;
      return sampler;
   }
   else
   {
      MapEntry& entry = it->second;
      std::shared_ptr<Sampler>& sampler = force_linear_rgb ? entry.linearSampler : entry.sampler;
      if (entry.pendingUpload)
      {
         sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), entry.pendingUpload, force_linear_rgb);
         entry.pendingUpload = nullptr;
      }
      else if (sampler == nullptr)
      {
         sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), memtex->GetRawBitmap(false, 0), force_linear_rgb);
      }
      else if (entry.dirty)
      {
         entry.dirty = false;
         sampler->UpdateTexture(memtex->GetRawBitmap(false, 0), force_linear_rgb);
      }
      return sampler;
   }
}

void TextureManager::AddPendingUpload(ITexManCacheable* memtex)
{
   const CIter it = m_map.find(memtex->GetLiveHash());
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.pendingUpload = memtex->GetRawBitmap(false, 0);
      entry.tex = memtex;
      m_map[memtex->GetLiveHash()] = entry;
   }
}

void TextureManager::AddPlaceHolder(ITexManCacheable* memtex)
{
   const CIter it = m_map.find(memtex->GetLiveHash());
   if (it == m_map.end())
   {
      std::shared_ptr<BaseTexture> placeHolder = std::shared_ptr<BaseTexture> (BaseTexture::Create(1, 1, BaseTexture::SRGBA));
      *reinterpret_cast<uint32_t*>(placeHolder->data()) = 0xFFFF00FFu;
      MapEntry entry;
      entry.sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), placeHolder, false);
      entry.linearSampler = entry.sampler;
      entry.isPlaceHolder = true;
      entry.tex = memtex;
      m_map[memtex->GetLiveHash()] = entry;
   }
}

vector<ITexManCacheable*> TextureManager::GetLoadedTextures() const
{
   std::vector<ITexManCacheable*> keys;
   for (const auto& it : m_map)
      if (!it.second.isPlaceHolder && !it.second.pendingUpload)
         keys.push_back(it.second.tex);
   return keys;
}

bool TextureManager::IsLinearRGB(ITexManCacheable* memtex) const
{
   const CIter it = m_map.find(memtex->GetLiveHash());
   return it == m_map.end() ? false : (it->second.linearSampler != nullptr);
}

void TextureManager::SetDirty(ITexManCacheable* memtex)
{
   const Iter it = m_map.find(memtex->GetLiveHash());
   if (it != m_map.end())
      it->second.dirty = true;
}

void TextureManager::UnloadTexture(ITexManCacheable* memtex)
{
   const CIter it = m_map.find(memtex->GetLiveHash());
   if (it != m_map.end())
      m_map.erase(it);
}

void TextureManager::UnloadAll()
{
   m_map.clear();
}
