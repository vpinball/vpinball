// license:GPLv3+

#pragma once

#if defined(ENABLE_BGFX)

class BaseTexture;

struct CompressedTexture final
{
   bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Unknown;
   unsigned int width = 0;
   unsigned int height = 0;
   uint8_t numMips = 0;
   vector<uint8_t> data;

   size_t GetMipSize(unsigned int mip) const;
};

namespace TextureCompressor
{
   bool IsSupported(const BaseTexture& tex);
   bgfx::TextureFormat::Enum SelectFormat(bool hasAlpha);
   const char* GetFormatName(bgfx::TextureFormat::Enum format);
   std::shared_ptr<const CompressedTexture> Compress(const BaseTexture& tex, bgfx::TextureFormat::Enum format);

   std::shared_ptr<const CompressedTexture> LoadOrCompress(const BaseTexture& tex, const std::filesystem::path& cacheFile);

   std::shared_ptr<BaseTexture> LoadCached(const std::filesystem::path& cacheFile);

   void ResetStats();
   void LogStats();
}

#endif
