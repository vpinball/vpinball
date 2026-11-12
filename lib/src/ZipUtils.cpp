// license:GPLv3+

#include "core/stdafx.h"
#include "ZipUtils.h"

#include <miniz/miniz.h>

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

// Entry name as UTF-8 (see ZipEntryNameToUTF8 for names without the UTF-8 flag)
static string GetEntryName(mz_zip_archive& archive, mz_uint index, const mz_zip_archive_file_stat& stat)
{
   string name(mz_zip_reader_get_filename(&archive, index, nullptr, 0), '\0'); // Not stat.m_filename, which truncates longer names (MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE)
   name.resize(name.empty() ? 0 : mz_zip_reader_get_filename(&archive, index, name.data(), static_cast<mz_uint>(name.size())) - 1);
   return ZipEntryNameToUTF8(name.data(), name.size(), (stat.m_bit_flag & (1 << 11)) != 0); // Bit 11: MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_UTF8, only declared in miniz.c
}

bool ZipUtils::Zip(const std::filesystem::path& sourcePath, const std::filesystem::path& destPath, ProgressCallback callback)
{
   std::error_code ec;
   if (!std::filesystem::is_directory(sourcePath, ec))
      return false;

   // Written to a temporary file that replaces destPath on success, so that a failed zip never leaves a partial archive
   std::filesystem::path tmpPath = destPath;
   tmpPath += ".tmp";
   mz_zip_archive archive {};
   if (!mz_zip_writer_init_file(&archive, PathToUTF8(tmpPath).c_str(), 0))
   {
      PLOGE << "Unable to create zip file: " << PathToUTF8(tmpPath) << ", error=" << mz_zip_get_error_string(mz_zip_get_last_error(&archive));
      return false;
   }

   const int totalEntries = CountEntriesInDirectory(sourcePath);
   int currentEntry = 0;
   bool success = true;
   std::error_code iterError;
   for (auto it = std::filesystem::recursive_directory_iterator(sourcePath, iterError); success && !iterError && it != std::filesystem::recursive_directory_iterator();
      it.increment(iterError))
   {
      const auto& entry = *it;
      const std::filesystem::path relative = std::filesystem::relative(entry.path(), sourcePath, ec);

      if (IsExcludedFromZip(relative) || std::filesystem::equivalent(entry.path(), tmpPath, ec))
      {
         if (entry.is_directory(ec))
            it.disable_recursion_pending();
         continue;
      }

      const std::u8string u8Name = relative.generic_u8string(); // '/' separators, as required in archives
      const string name(u8Name.begin(), u8Name.end());

      if (entry.is_directory(ec))
         success = mz_zip_writer_add_mem(&archive, (name + '/').c_str(), nullptr, 0, MZ_NO_COMPRESSION);
      else if (entry.is_regular_file(ec))
         success = mz_zip_writer_add_file(&archive, name.c_str(), PathToUTF8(entry.path()).c_str(), nullptr, 0, MZ_DEFAULT_LEVEL);
      if (!success)
         PLOGE << "Failed to add to zip: " << name << ", error=" << mz_zip_get_error_string(mz_zip_get_last_error(&archive));

      currentEntry++;
      if (success && callback)
         callback(currentEntry, totalEntries, name.c_str());
   }
   if (iterError)
   {
      PLOGE << "Unable to list folder to zip: " << PathToUTF8(sourcePath) << ", error=" << iterError.message();
      success = false;
   }

   success = success && mz_zip_writer_finalize_archive(&archive);
   success = mz_zip_writer_end(&archive) && success;
   std::error_code renameError; // Not ec, which may hold an error from the per entry checks above
   if (success)
      std::filesystem::rename(tmpPath, destPath, renameError);
   if (!success || renameError)
   {
      std::filesystem::remove(tmpPath, ec);
      return false;
   }
   return true;
}

bool ZipUtils::Unzip(const std::filesystem::path& sourcePath, const std::filesystem::path& destPath, ProgressCallback callback)
{
   mz_zip_archive archive {};
   if (!mz_zip_reader_init_file(&archive, PathToUTF8(sourcePath).c_str(), 0)) {
      PLOGE << "Unable to unzip file: source=" << PathToUTF8(sourcePath) << ", error=" << mz_zip_get_error_string(mz_zip_get_last_error(&archive));
      return false;
   }

   const mz_uint totalEntries = mz_zip_reader_get_num_files(&archive);

   // Any failure fails the whole extraction, so that a partially extracted archive is never taken for a complete one.
   // Cleaning up what was already extracted is left to the caller, which owns the destination folder
   bool success = true;
   for (mz_uint i = 0; success && i < totalEntries; ++i) {
      mz_zip_archive_file_stat fileStat;
      if (!mz_zip_reader_file_stat(&archive, i, &fileStat)) {
         PLOGE << "Unable to read zip entry " << i << ": source=" << PathToUTF8(sourcePath);
         success = false;
         break;
      }

      const string filename = GetEntryName(archive, i, fileStat);

      if (!IsSafeEntryName(filename)) {
         PLOGE << "Unsafe zip entry name: " << filename << ", source=" << PathToUTF8(sourcePath);
         success = false;
         break;
      }

      if (IsExcludedPath(filename))
         continue;

      const std::filesystem::path destFilePath = destPath / PathFromUTF8(filename);

      std::error_code ec;
      // Directory as decided by miniz, or empty entry whose decoded name ends with a separator: miniz leaves the backslash of an unflagged non-ASCII name
      // (e.g. a name with umlauts from .NET given a code page encoding) alone, as it may be a DBCS trail byte, which in a non-empty entry is taken as part of a file name
      if (fileStat.m_is_directory || (filename.back() == '/' && fileStat.m_uncomp_size == 0)) {
         std::filesystem::create_directories(destFilePath, ec);
         if (ec) {
            PLOGE << "Unable to create directory: " << PathToUTF8(destFilePath) << ", error=" << ec.message();
            success = false;
            break;
         }
      }
      else {
         std::filesystem::create_directories(destFilePath.parent_path(), ec);
         if (ec) {
            PLOGE << "Unable to create directory: " << PathToUTF8(destFilePath.parent_path()) << ", error=" << ec.message();
            success = false;
            break;
         }

         // Streams the entry to the file and verifies its CRC
         if (!mz_zip_reader_extract_to_file(&archive, i, PathToUTF8(destFilePath).c_str(), 0)) {
            PLOGE << "Unable to extract file: " << PathToUTF8(destFilePath) << ", error=" << mz_zip_get_error_string(mz_zip_get_last_error(&archive));
            success = false;
            break;
         }
      }

      if (callback)
         callback(static_cast<int>(i + 1), static_cast<int>(totalEntries), filename.c_str());
   }

   mz_zip_reader_end(&archive);
   return success;
}
