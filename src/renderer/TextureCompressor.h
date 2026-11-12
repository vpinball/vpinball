// license:GPLv3+

#pragma once

#if defined(ENABLE_BGFX)

#include <atomic>
#include <functional>

#include "Texture.h"

struct CompressedTexture final
{
   bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Unknown;
   unsigned int width = 0;
   unsigned int height = 0;
   uint8_t numMips = 0;
   vector<uint8_t> data;

   size_t GetMipSize(unsigned int mip) const;
};

// Compresses textures to GPU formats, backed by a per table disk cache. Owned by the texture manager for the player's lifetime
// (see TextureManager::LoadBitmapCC): images are compressed while loading the table, and later reloaded from the disk cache.
class TextureCompressor final
{
public:
   // cacheFolder is the folder holding the disk compressed texture cache (empty to disable caching)
   explicit TextureCompressor(std::filesystem::path cacheFolder);

   // Compressed image: from the disk cache, otherwise decoded (within maxTexDim, 0 for no limit), compressed, and cached.
   // isCompressionDiscarded is queried before and during compression: true skips or aborts it. Returns the decoded image if not
   // compressed (unsupported format, discarded compression), nullptr if it can not be decoded
   std::shared_ptr<const BaseTexture> LoadOrCompress(Texture* image, unsigned int maxTexDim, bool resizeOnLowMem, const std::function<bool()>& isCompressionDiscarded);

   // Compressed image from the disk cache only (nullptr if not cached)
   std::shared_ptr<BaseTexture> LoadCached(const Texture* image, unsigned int maxTexDim);

   static bool IsSupported(const BaseTexture& tex);

   static bgfx::TextureFormat::Enum SelectFormatFor(BaseTexture::Format srcFormat, bool opaque, unsigned int width, unsigned int height);

   static const char* GetFormatName(bgfx::TextureFormat::Enum format);

   // Compresses a texture to the given format. Static since it is also used outside of loading
   // sessions (runtime texture updates), where it is not accounted in the session statistics.
   // isCompressionDiscarded is queried while compressing; returning true aborts the compression.
   static std::shared_ptr<const CompressedTexture> Compress(const BaseTexture& tex, bgfx::TextureFormat::Enum format, const std::function<bool()>& isCompressionDiscarded = nullptr);

   void LogStats() const; // Statistics since the compressor was created

   // Removes the cache files of the images not in 'images' (all the table's ones, not only the loaded ones), and leftover temporary files
   void CleanCache(const vector<Texture*>& images, unsigned int maxTexDim) const;

private:
   // Path of the disk cache file of an image (empty when disk caching is disabled)
   std::filesystem::path GetCacheFile(const Texture* image, unsigned int maxTexDim) const;
   // Reads a cache file (nullptr if missing, or of a format not selected for this GPU)
   std::shared_ptr<BaseTexture> LoadCacheFile(const std::filesystem::path& cacheFile);
   // Compresses a decoded texture, and saves the result to the cache file (if any). The caller already missed the cache (LoadCached)
   std::shared_ptr<const CompressedTexture> CompressToCache(const BaseTexture& tex, const std::filesystem::path& cacheFile, const std::function<bool()>& isCompressionDiscarded);

   const std::filesystem::path m_cacheFolder;

   std::atomic<uint64_t> m_nCompressed { 0 };
   std::atomic<uint64_t> m_nLoaded { 0 };
   std::atomic<uint64_t> m_compressMs { 0 };
   std::atomic<uint64_t> m_loadMs { 0 };
   std::atomic<uint64_t> m_rawBytes { 0 };
   std::atomic<uint64_t> m_compressedBytes { 0 };
};

#endif
