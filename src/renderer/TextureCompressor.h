// license:GPLv3+

#pragma once

#if defined(ENABLE_BGFX)

#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_set>

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

// Compresses textures to GPU formats, eventually backed by a per table disk cache.
// An instance is scoped to a texture loading session: its statistics start clean on
// construction and can be reported at the end of the session with LogStats.
class TextureCompressor final
{
public:
   // cacheFolder is the folder holding the disk compressed texture cache (empty to disable caching)
   // maxTexDim is the maximum texture dimension applied when loading images (0 = no limit)
   TextureCompressor(std::filesystem::path cacheFolder, unsigned int maxTexDim);

   // Path of the disk cache file of an image (empty when disk caching is disabled)
   std::filesystem::path GetCacheFile(const Texture* image) const;

   // Loads an image, eventually resolving it from the compressed disk cache, otherwise decoding
   // it and compressing the result (saved to the disk cache when enabled). isCompressionDiscarded is
   // queried before and during compression; returning true skips or aborts the compression.
   std::shared_ptr<const BaseTexture> Load(Texture* image, bool resizeOnLowMem, const std::function<bool()>& isCompressionDiscarded);

   static bool IsSupported(const BaseTexture& tex);

   static bgfx::TextureFormat::Enum SelectFormatFor(BaseTexture::Format srcFormat, bool opaque, unsigned int width, unsigned int height);

   static const char* GetFormatName(bgfx::TextureFormat::Enum format);

   // Compresses a texture to the given format. Static since it is also used outside of loading
   // sessions (runtime texture updates), where it is not accounted in the session statistics.
   // isCompressionDiscarded is queried while compressing; returning true aborts the compression.
   static std::shared_ptr<const CompressedTexture> Compress(const BaseTexture& tex, bgfx::TextureFormat::Enum format, const std::function<bool()>& isCompressionDiscarded = nullptr);

   std::shared_ptr<const CompressedTexture> LoadOrCompress(const BaseTexture& tex, const std::filesystem::path& cacheFile, const std::function<bool()>& isCompressionDiscarded = nullptr);

   std::shared_ptr<BaseTexture> LoadCached(const std::filesystem::path& cacheFile);

   void LogStats() const;

   // Removes cache files that are not referenced by the images loaded during this session, as well as leftover temporary files
   void CleanCache();

private:
   const std::filesystem::path m_cacheFolder;
   const unsigned int m_maxTexDim;

   // Cache files referenced while loading this session (written by the loading threads), used by CleanCache to detect stale entries
   std::mutex m_usedCacheFilesMutex;
   std::unordered_set<std::filesystem::path> m_usedCacheFiles;

   std::atomic<uint64_t> m_nCompressed { 0 };
   std::atomic<uint64_t> m_nLoaded { 0 };
   std::atomic<uint64_t> m_compressMs { 0 };
   std::atomic<uint64_t> m_loadMs { 0 };
   std::atomic<uint64_t> m_rawBytes { 0 };
   std::atomic<uint64_t> m_compressedBytes { 0 };
};

#endif
