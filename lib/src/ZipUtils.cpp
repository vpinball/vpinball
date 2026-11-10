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

// Entry name as UTF-8. Names without the UTF-8 flag are code page 437 per the ZIP specification (older Windows archivers).
// As libzip did, such names are kept if they are valid UTF-8 anyway, and converted from code page 437 otherwise
static string GetEntryName(mz_zip_archive& archive, mz_uint index, const mz_zip_archive_file_stat& stat)
{
   string name(mz_zip_reader_get_filename(&archive, index, nullptr, 0), '\0'); // Not stat.m_filename, which is truncated to 511 bytes
   name.resize(name.empty() ? 0 : mz_zip_reader_get_filename(&archive, index, name.data(), static_cast<mz_uint>(name.size())) - 1);
   if ((stat.m_bit_flag & (1 << 11)) || IsValidUTF8(name.data(), name.size()))
      return name;
   static constexpr char16_t cp437[128] = {
      0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7, 0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
      0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9, 0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
      0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA, 0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
      0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556, 0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
      0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F, 0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
      0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B, 0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
      0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4, 0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
      0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248, 0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0 };
   string utf8;
   utf8.reserve(name.size() * 2);
   for (const char c : name) {
      const uint8_t b = static_cast<uint8_t>(c);
      const char16_t cp = b < 0x80 ? b : cp437[b - 0x80];
      if (cp < 0x80)
         utf8 += static_cast<char>(cp);
      else if (cp < 0x800) {
         utf8 += static_cast<char>(0xC0 | (cp >> 6));
         utf8 += static_cast<char>(0x80 | (cp & 0x3F));
      }
      else {
         utf8 += static_cast<char>(0xE0 | (cp >> 12));
         utf8 += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
         utf8 += static_cast<char>(0x80 | (cp & 0x3F));
      }
   }
   return utf8;
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
   if (!mz_zip_writer_init_file(&archive, PathToUTF8(tmpPath).c_str(), 0)) {
      PLOGE << "Unable to create zip file: " << PathToUTF8(tmpPath) << ", error=" << mz_zip_get_error_string(mz_zip_get_last_error(&archive));
      return false;
   }

   const int totalEntries = CountEntriesInDirectory(sourcePath);
   int currentEntry = 0;
   bool success = true;
   std::error_code iterError;
   for (auto it = std::filesystem::recursive_directory_iterator(sourcePath, iterError); success && !iterError && it != std::filesystem::recursive_directory_iterator(); it.increment(iterError)) {
      const auto& entry = *it;
      const std::filesystem::path relative = std::filesystem::relative(entry.path(), sourcePath, ec);

      if (IsExcludedFromZip(relative) || std::filesystem::equivalent(entry.path(), tmpPath, ec)) {
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
   if (iterError) {
      PLOGE << "Unable to list folder to zip: " << PathToUTF8(sourcePath) << ", error=" << iterError.message();
      success = false;
   }

   success = success && mz_zip_writer_finalize_archive(&archive);
   success = mz_zip_writer_end(&archive) && success;
   std::error_code renameError; // Not ec, which may hold an error from the per entry checks above
   if (success)
      std::filesystem::rename(tmpPath, destPath, renameError);
   if (!success || renameError) {
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
      if (filename.back() == '/') {
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
