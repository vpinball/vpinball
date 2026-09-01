// license:GPLv3+

#include "core/stdafx.h"

#include "TextureManager.h"
#include "RenderDevice.h"
#include "Texture.h"

std::shared_ptr<Sampler> TextureManager::LoadTexture(ITexManCacheable* const memtex, const bool force_linear_rgb)
{
   const uint64_t hash = memtex->GetLiveHash();
   const SamplerVariant variant = SamplerVariantOf(force_linear_rgb);
   const Iter it = m_map.find(hash);
   if (it == m_map.end())
   {
      MapEntry entry;
      entry.tex = memtex;
      std::shared_ptr<Sampler>& sampler = entry.samplers[variant];
      sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), memtex->GetRawBitmap(false, 0), force_linear_rgb);
      m_map[hash] = entry;
      return sampler;
   }
   else
   {
      MapEntry& entry = it->second;
      std::shared_ptr<Sampler>& sampler = entry.samplers[variant];
      if (entry.pendingUpload)
      {
         // Consumed by whichever variant asks first, dropping the strong reference that was keeping the decoded
         // bitmap alive. A second variant requested later therefore falls in the branch below and calls
         // GetRawBitmap(), which only holds the bitmap weakly and so re-decodes the image from its compressed
         // data. Content is still correct, but will be a decode on the render thread then
         sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), entry.pendingUpload, force_linear_rgb);
         entry.pendingUpload = nullptr;
      }
      else if (sampler == nullptr)
      {
         sampler = std::make_shared<Sampler>(&m_rd, memtex->GetName(), memtex->GetRawBitmap(false, 0), force_linear_rgb);
      }
      else if (entry.dirty[variant])
      {
         entry.dirty[variant] = false; // Only the variant actually refreshed below, the other keeps waiting its turn
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
      // Both variants share the one sampler: a 1x1 magenta marker looks the same either way, and it is replaced as soon as the real texture loads
      entry.samplers[VARIANT_SRGB]   = std::make_shared<Sampler>(&m_rd, memtex->GetName(), placeHolder, false);
      entry.samplers[VARIANT_LINEAR] = entry.samplers[VARIANT_SRGB];
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
   // A placeholder aliases both variants onto a single sRGB sampler, so it is not a linear upload
   return it != m_map.end() && !it->second.isPlaceHolder && it->second.samplers[VARIANT_LINEAR] != nullptr;
}

void TextureManager::SetDirty(ITexManCacheable* memtex)
{
   const Iter it = m_map.find(memtex->GetLiveHash());
   if (it != m_map.end())
      for (bool& dirty : it->second.dirty) // Every live variant needs refreshing, not just the next one drawn
         dirty = true;
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
