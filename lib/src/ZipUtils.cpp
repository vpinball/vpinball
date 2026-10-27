// license:GPLv3+

#include "core/stdafx.h"
#include "ZipUtils.h"

#include <zip.h>
#include <fstream>

static bool IsExcludedPath(const string& path)
{
   return path.rfind("__MACOSX", 0) == 0 || path.find("/__MACOSX") != string::npos;
}

// Entry names come from the archive, so they must stay inside the destination folder: no absolute path,
// no drive letter, no '..' component (zip slip). Both separators are checked, as archives made on Windows may use '\'
static bool IsSafeEntryName(const string& name)
{
   if (name.empty() || name[0] == '/' || name[0] == '\\' || name.find(':') != string::npos)
      return false;
   size_t start = 0;
   while (start <= name.size())
   {
      const size_t end = std::min(name.find('/', start), name.find('\\', start));
      if (name.compare(start, (end == string::npos ? name.size() : end) - start, "..") == 0)
         return false;
      if (end == string::npos)
         break;
      start = end + 1;
   }
   return true;
}

static bool IsExcludedFromZip(const std::filesystem::path& relativePath)
{
   return IsExcludedPath(relativePath.string()) || (!relativePath.empty() && *relativePath.begin() == "cache");
}

static int CountEntriesInDirectory(const std::filesystem::path& dirPath)
{
   int count = 0;
   std::error_code ec;
   for (auto it = std::filesystem::recursive_directory_iterator(dirPath, ec); !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
      if (!IsExcludedFromZip(std::filesystem::relative(it->path(), dirPath, ec)))
         count++;
   }
   return count;
}

struct ZipProgressContext {
   ZipUtils::ProgressCallback callback;
   int totalEntries;
};

static void ZipProgressCallback(zip_t* archive, double progress, void* userdata)
{
   auto* ctx = static_cast<ZipProgressContext*>(userdata);
   if (ctx && ctx->callback)
      ctx->callback(static_cast<int>(progress * ctx->totalEntries), ctx->totalEntries, "");
}

bool ZipUtils::Zip(const std::filesystem::path& sourcePath, const std::filesystem::path& destPath, ProgressCallback callback)
{
   std::error_code ec;
   if (!std::filesystem::is_directory(sourcePath, ec))
      return false;

   int error = 0;
   zip_t* archive = zip_open(destPath.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
   if (!archive)
      return false;

   const int totalEntries = CountEntriesInDirectory(sourcePath);
   int currentEntry = 0;

   for (auto it = std::filesystem::recursive_directory_iterator(sourcePath, ec); !ec && it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
      const auto& entry = *it;
      const std::filesystem::path relative = std::filesystem::relative(entry.path(), sourcePath, ec);

      if (IsExcludedFromZip(relative)) {
         if (entry.is_directory(ec))
            it.disable_recursion_pending();
         continue;
      }

      const string relativePath = relative.string();

      if (entry.is_directory(ec)) {
         const string dirPath = relativePath + "/";
         zip_dir_add(archive, dirPath.c_str(), ZIP_FL_ENC_UTF_8);
      }
      else if (entry.is_regular_file(ec)) {
         zip_source_t* fileSource = zip_source_file(archive, entry.path().string().c_str(), 0, -1);
         if (fileSource) {
            const zip_int64_t index = zip_file_add(archive, relativePath.c_str(), fileSource, ZIP_FL_ENC_UTF_8);
            if (index < 0) {
               zip_source_free(fileSource);
               PLOGE.printf("Failed to add file to zip: %s", relativePath.c_str());
            }
         }
      }

      currentEntry++;
   }

   ZipProgressContext progressCtx { callback, totalEntries };
   if (callback)
      zip_register_progress_callback_with_state(archive, 0.01, ZipProgressCallback, nullptr, &progressCtx);

   if (zip_close(archive) < 0)
      return false;

   return true;
}

bool ZipUtils::Unzip(const std::filesystem::path& sourcePath, const std::filesystem::path& destPath, ProgressCallback callback)
{
   int error = 0;
   zip_t* archive = zip_open(sourcePath.string().c_str(), ZIP_RDONLY, &error);
   if (!archive) {
      PLOGE.printf("Unable to unzip file: source=%s", sourcePath.string().c_str());
      return false;
   }

   const zip_int64_t totalEntries = zip_get_num_entries(archive, 0);

   // Any failure fails the whole extraction, so that a partially extracted archive is never taken for a complete one.
   // Cleaning up what was already extracted is left to the caller, which owns the destination folder
   bool success = totalEntries >= 0;
   for (zip_uint64_t i = 0; success && i < (zip_uint64_t)totalEntries; ++i) {
      zip_stat_t fileStat;
      if (zip_stat_index(archive, i, ZIP_STAT_NAME, &fileStat) != 0) {
         PLOGE.printf("Unable to read zip entry %d: source=%s", (int)i, sourcePath.string().c_str());
         success = false;
         break;
      }

      const string filename = fileStat.name;

      if (!IsSafeEntryName(filename)) {
         PLOGE.printf("Unsafe zip entry name: %s, source=%s", filename.c_str(), sourcePath.string().c_str());
         success = false;
         break;
      }

      if (IsExcludedPath(filename))
         continue;

      const std::filesystem::path destFilePath = destPath / filename;

      std::error_code ec;
      if (filename.back() == '/') {
         std::filesystem::create_directories(destFilePath, ec);
         if (ec) {
            PLOGE.printf("Unable to create directory: %s, error=%s", destFilePath.string().c_str(), ec.message().c_str());
            success = false;
            break;
         }
      }
      else {
         std::filesystem::create_directories(destFilePath.parent_path(), ec);
         if (ec) {
            PLOGE.printf("Unable to create directory: %s, error=%s", destFilePath.parent_path().string().c_str(), ec.message().c_str());
            success = false;
            break;
         }

         zip_file_t* zipFile = zip_fopen_index(archive, i, 0);
         if (!zipFile) {
            PLOGE.printf("Unable to extract file: %s", destFilePath.string().c_str());
            success = false;
            break;
         }

         std::ofstream ofs(destFilePath, std::ios::binary);
         char buf[4096];
         zip_int64_t len = 0;
         while (ofs && (len = zip_fread(zipFile, buf, sizeof(buf))) > 0)
            ofs.write(buf, len);
         zip_fclose(zipFile);
         ofs.close();
         if (!ofs || len < 0) {
            PLOGE.printf("Unable to extract file (read or write error): %s", destFilePath.string().c_str());
            success = false;
            break;
         }
      }

      if (callback)
         callback((int)(i + 1), (int)totalEntries, filename.c_str());
   }

   zip_close(archive);
   return success;
}
