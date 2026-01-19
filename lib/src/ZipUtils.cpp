// license:GPLv3+

#include "core/stdafx.h"
#include "ZipUtils.h"

#include <zip.h>
#include <fstream>

static bool IsExcludedPath(const string& path)
{
   return path.rfind("__MACOSX", 0) == 0 || path.find("/__MACOSX") != string::npos;
}

static int CountEntriesInDirectory(const std::filesystem::path& dirPath)
{
   int count = 0;
   for (const auto& entry : std::filesystem::recursive_directory_iterator(dirPath)) {
      const string relativePath = std::filesystem::relative(entry.path(), dirPath).string();
      if (!IsExcludedPath(relativePath))
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
   if (!std::filesystem::exists(sourcePath) || !std::filesystem::is_directory(sourcePath))
      return false;

   int error = 0;
   zip_t* archive = zip_open(destPath.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
   if (!archive)
      return false;

   const int totalEntries = CountEntriesInDirectory(sourcePath);
   int currentEntry = 0;

   for (const auto& entry : std::filesystem::recursive_directory_iterator(sourcePath)) {
      const string relativePath = std::filesystem::relative(entry.path(), sourcePath).string();

      if (IsExcludedPath(relativePath))
         continue;

      if (entry.is_directory()) {
         const string dirPath = relativePath + "/";
         zip_dir_add(archive, dirPath.c_str(), ZIP_FL_ENC_UTF_8);
      }
      else if (entry.is_regular_file()) {
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

   for (zip_uint64_t i = 0; i < (zip_uint64_t)totalEntries; ++i) {
      zip_stat_t fileStat;
      if (zip_stat_index(archive, i, ZIP_STAT_NAME, &fileStat) != 0)
         continue;

      const string filename = fileStat.name;

      if (IsExcludedPath(filename))
         continue;

      const std::filesystem::path destFilePath = destPath / filename;

      if (filename.back() == '/')
         std::filesystem::create_directories(destFilePath);
      else {
         std::filesystem::create_directories(destFilePath.parent_path());

         zip_file_t* zipFile = zip_fopen_index(archive, i, 0);
         if (!zipFile) {
            PLOGE.printf("Unable to extract file: %s", destFilePath.string().c_str());
            continue;
         }

         std::ofstream ofs(destFilePath, std::ios::binary);
         char buf[4096];
         zip_int64_t len;
         while ((len = zip_fread(zipFile, buf, sizeof(buf))) > 0)
            ofs.write(buf, len);
         zip_fclose(zipFile);
      }

      if (callback)
         callback((int)(i + 1), (int)totalEntries, filename.c_str());
   }

   zip_close(archive);
   return true;
}
