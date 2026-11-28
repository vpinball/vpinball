// license:GPLv3+

#include "core/stdafx.h"
#include "core/TableCatalog.h"

#include <nlohmann/json.hpp>

#include <cstring>
#include <fstream>
#include <mutex>
#include <sstream>
#include <unordered_map>


const char* TableCatalog::GetMediaTypeName(MediaType type)
{
   static const char* const names[]
      = { "Playfield", "Backglass", "DMD", "FullDMD", "Topper", "Other", "RealDMD", "RealColorDMD", "Wheel", "GameHelp", "GameInfo", "Loading", "Audio", "AudioLaunch" };
   const int index = static_cast<int>(type);
   return (index >= 0 && index < static_cast<int>(MediaType::Count)) ? names[index] : "";
}


namespace
{

string TextureCachePathKey(const std::filesystem::path& path) { return lowerCase(PathToUTF8(path.lexically_normal())); }

std::filesystem::file_time_type TextureCacheTime(const std::filesystem::path& path)
{
   using Clock = std::filesystem::file_time_type;
   Clock newest = Clock::min();
   const auto addFileTime = [&newest](const std::filesystem::path& file)
   {
      std::error_code ec;
      const Clock time = std::filesystem::last_write_time(file, ec);
      if (!ec && time > newest)
         newest = time;
   };
   addFileTime(path / "last_played"sv);
   addFileTime(path / "used_textures.xml"sv);
   addFileTime(path);
   return newest;
}

bool RemoveTextureCacheFolder(const std::filesystem::path& path)
{
   std::error_code ec;
   std::filesystem::remove_all(path, ec);
   return !ec && !std::filesystem::exists(path, ec);
}

vector<std::filesystem::path> FindTextureCacheContainers(const vector<std::filesystem::path>& roots)
{
   vector<std::filesystem::path> containers;
   static const ankerl::unordered_dense::set<string> assetFolders
      = { "medias"s, "cache"s, "user"s, "music"s, "altsound"s, "altcolor"s, "pinmame"s, "pupvideos"s, "serum"s, "vni"s, "scripts"s, "dmd"s };
   for (const std::filesystem::path& root : roots)
   {
      std::error_code ec;
      if (!std::filesystem::is_directory(root, ec))
         continue;
      const auto options = std::filesystem::directory_options::skip_permission_denied;
      for (std::filesystem::recursive_directory_iterator it(root, options, ec), end; !ec && it != end; it.increment(ec))
      {
         std::error_code typeEc;
         if (!it->is_directory(typeEc) || typeEc)
            continue;
         const string folderName = lowerCase(PathToUTF8(it->path().filename()));
         if (!assetFolders.contains(folderName))
            continue;
         it.disable_recursion_pending();
         if (folderName == "cache"s)
            containers.push_back(it->path());
      }
   }
   return containers;
}
}

TableCatalog::TextureCacheCleanupResult TableCatalog::CleanupTextureCaches(
   const vector<std::filesystem::path>& roots, const vector<Entry>& entries, const bool discardStale, const int maxRecentTables, const vector<std::filesystem::path>* scannedCacheFolders)
{
   TextureCacheCleanupResult result;
   ankerl::unordered_dense::set<string> validCacheFolders;
   for (const Entry& entry : entries)
      validCacheFolders.insert(TextureCachePathKey(entry.tablePath.parent_path() / "cache"sv / entry.tablePath.stem()));

   vector<std::filesystem::path> discoveredContainers;
   const vector<std::filesystem::path>* containers = scannedCacheFolders;
   if (containers == nullptr)
   {
      discoveredContainers = FindTextureCacheContainers(roots);
      containers = &discoveredContainers;
   }

   struct CacheItem
   {
      std::filesystem::path path;
      std::filesystem::file_time_type lastUsed;
   };
   vector<CacheItem> validItems;
   ankerl::unordered_dense::set<string> processed;

   for (const std::filesystem::path& container : *containers)
   {
      std::error_code childEc;
      for (std::filesystem::directory_iterator child(container, childEc), childEnd; !childEc && child != childEnd; child.increment(childEc))
      {
         std::error_code childTypeEc;
         if (!child->is_directory(childTypeEc) || childTypeEc)
            continue;
         const std::filesystem::path cacheFolder = child->path();
         const string key = TextureCachePathKey(cacheFolder);
         if (!processed.insert(key).second)
            continue;
         if (!validCacheFolders.contains(key))
         {
            if (discardStale)
            {
               if (RemoveTextureCacheFolder(cacheFolder))
                  result.staleRemoved++;
               else
                  result.failedRemovals++;
            }
            continue;
         }
         validItems.push_back({ cacheFolder, TextureCacheTime(cacheFolder) });
      }
   }

   if (maxRecentTables >= 0 && validItems.size() > static_cast<size_t>(maxRecentTables))
   {
      std::ranges::sort(validItems,
         [](const CacheItem& a, const CacheItem& b)
         {
            if (a.lastUsed != b.lastUsed)
               return a.lastUsed > b.lastUsed;
            return TextureCachePathKey(a.path) < TextureCachePathKey(b.path);
         });
      for (size_t i = static_cast<size_t>(maxRecentTables); i < validItems.size(); ++i)
      {
         if (RemoveTextureCacheFolder(validItems[i].path))
            result.overflowRemoved++;
         else
            result.failedRemovals++;
      }
   }
   return result;
}

bool TableCatalog::IsTablePath(const std::filesystem::path& path, bool* isPack)
{
   std::error_code ec;
   if (std::filesystem::is_directory(path, ec))
   {
      if (isPack != nullptr)
         *isPack = true;
      return FileExists(path / "manifest.json"s);
   }
   const string extension = lowerCase(PathToUTF8(path.extension()));
   if (extension == ".vpx"s)
   {
      if (isPack != nullptr)
         *isPack = false;
      return true;
   }
   if (extension == ".vpz"s)
   {
      if (isPack != nullptr)
         *isPack = true;
      return true;
   }
   return false;
}


std::filesystem::path TableCatalog::FindInfoFile(
   const std::filesystem::path& tablePath, const bool isFolderPack, const FolderFileIndex& folderFiles)
{
   // File named after the table file/pack
   std::filesystem::path tableFile = tablePath;
   tableFile.replace_extension(".info"s);
   if (const auto it = folderFiles.find(lowerCase(PathToUTF8(tableFile.filename()))); it != folderFiles.end())
      return it->second;

   // File named after the folder containing the table (inside the folder for a folder pack)
   const std::filesystem::path folder = isFolderPack ? tablePath : tablePath.parent_path();
   std::filesystem::path fn = folder.filename();
   fn += ".info"s;
   if (const auto it = folderFiles.find(lowerCase(PathToUTF8(fn))); it != folderFiles.end())
      return it->second;
   if (isFolderPack)
      return find_case_insensitive_file_path(folder / fn);

   return std::filesystem::path();
}


std::filesystem::path TableCatalog::GetDefaultInfoPath(const std::filesystem::path& tablePath)
{
   std::error_code ec;
   if (std::filesystem::is_directory(tablePath, ec))
   {
      std::filesystem::path fn = tablePath.filename();
      fn += ".info"s;
      return tablePath / fn;
   }
   std::filesystem::path infoPath = tablePath;
   infoPath.replace_extension(".info"s);
   return infoPath;
}


// Sniffs the chunk headers of a PNG file for the 'acTL' animation control chunk, mandatory and
// located before the first 'IDAT' in a valid APNG, so scanning stops at whichever is met first
static bool SniffApng(const std::filesystem::path& path)
{
   const string extension = lowerCase(PathToUTF8(path.extension()));
   if (extension != ".png"s && extension != ".apng"s)
      return false;
   std::ifstream file(path, std::ios::binary);
   if (!file.is_open())
      return false;
   static constexpr uint8_t signature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
   uint8_t header[8];
   if (!file.read(reinterpret_cast<char*>(header), 8) || memcmp(header, signature, 8) != 0)
      return false;
   while (file.read(reinterpret_cast<char*>(header), 8))
   {
      if (memcmp(header + 4, "acTL", 4) == 0)
         return true;
      if (memcmp(header + 4, "IDAT", 4) == 0 || memcmp(header + 4, "fdAT", 4) == 0 || memcmp(header + 4, "IEND", 4) == 0)
         return false;
      const uint32_t length = (static_cast<uint32_t>(header[0]) << 24) | (static_cast<uint32_t>(header[1]) << 16) | (static_cast<uint32_t>(header[2]) << 8) | header[3];
      file.seekg(static_cast<std::streamoff>(length) + 4, std::ios::cur); // Skip chunk data and CRC
   }
   return false;
}

struct AnimatedImageCacheKey
{
   std::filesystem::path path;
   int64_t writeTime = 0;
   int64_t size = 0;
   bool operator==(const AnimatedImageCacheKey&) const = default;
};

struct AnimatedImageCacheKeyHash
{
   size_t operator()(const AnimatedImageCacheKey& key) const
   {
      size_t h = std::hash<std::filesystem::path>()(key.path);
      h = h * 31 + std::hash<int64_t>()(key.writeTime);
      return h * 31 + std::hash<int64_t>()(key.size);
   }
};

// Sniffing opens the file and is also needed while rendering (to choose still/video decoding), so
// memoize it. The stamp participates in the key: replacing a file at the same path invalidates it.
static std::mutex s_animatedImageMutex;
static std::unordered_map<AnimatedImageCacheKey, bool, AnimatedImageCacheKeyHash> s_animatedImageFiles;

bool TableCatalog::IsAnimatedImageFile(const std::filesystem::path& path)
{
   AnimatedImageCacheKey key { path };
   std::error_code ec;
   const auto writeTime = std::filesystem::last_write_time(path, ec);
   if (!ec)
      key.writeTime = writeTime.time_since_epoch().count();
   if (std::filesystem::is_regular_file(path, ec))
   {
      const auto fileSize = std::filesystem::file_size(path, ec);
      if (!ec)
         key.size = static_cast<int64_t>(fileSize);
   }
   {
      std::lock_guard lock(s_animatedImageMutex);
      if (const auto it = s_animatedImageFiles.find(key); it != s_animatedImageFiles.end())
         return it->second;
   }
   const bool animated = SniffApng(path); // Keep file IO outside the mutex: scans must not stall rendering
   std::lock_guard lock(s_animatedImageMutex);
   return s_animatedImageFiles.emplace(key, animated).first->second;
}

bool TableCatalog::IsAnimatedMediaFile(const std::filesystem::path& path) { return IsVideoFile(path) || IsAnimatedImageFile(path); }


void TableCatalog::ScanPaths(const vector<std::filesystem::path>& roots, const std::function<void(size_t)>& progress, vector<std::filesystem::path>* cacheFolders)
{
   m_entries.clear();
   m_mediaSets.clear();
   if (cacheFolders != nullptr)
      cacheFolders->clear();
   for (const std::filesystem::path& root : roots)
   {
      std::error_code ec;
      if (std::filesystem::is_directory(root, ec))
         ScanFolder(root, m_entries, progress, cacheFolders);
   }
   if (progress)
      progress(m_entries.size());
   SortEntries();
}

void TableCatalog::Scan(const vector<std::filesystem::path>& roots)
{
   ScanPaths(roots);
   LoadAllDetails();
}

void TableCatalog::EnsureEntryDetails(const size_t index, const bool reposition)
{
   if (index >= m_entries.size())
      return;
   Entry& entry = m_entries[index];
   if (entry.detailsLoaded)
      return;
   const string previousSortTitle = entry.sortTitle;
   if (entry.hasInfoFile)
      ReadInfoFile(entry);
   const MediaSet& medias = GetMediaSet(entry.mediaFolder);
   for (size_t i = 0; i < medias.size(); ++i)
      entry.medias[i] = medias[i];
   entry.detailsLoaded = true;
   entry.sortTitle = lowerCase(entry.title);

   // Background scans load thousands of entries and order them as a batch. A foreground on demand
   // load instead keeps the sorted catalog valid immediately by moving only this entry.
   if (reposition && entry.sortTitle != previousSortTitle)
   {
      Entry moved = std::move(m_entries[index]);
      m_entries.erase(m_entries.begin() + static_cast<ptrdiff_t>(index));
      const auto insertAt = std::lower_bound(
         m_entries.begin(), m_entries.end(), moved, [](const Entry& a, const Entry& b) { return a.sortTitle != b.sortTitle ? a.sortTitle < b.sortTitle : a.tablePath < b.tablePath; });
      m_entries.insert(insertAt, std::move(moved));
      RebuildPositions();
   }
}

void TableCatalog::LoadAllDetails()
{
   for (size_t i = 0; i < m_entries.size(); ++i)
      EnsureEntryDetails(i);
   for (Entry& entry : m_entries)
      entry.sortTitle = lowerCase(entry.title);
   SortEntries();
}

static bool EntrySortLess(const TableCatalog::Entry& a, const TableCatalog::Entry& b) { return a.sortTitle != b.sortTitle ? a.sortTitle < b.sortTitle : a.tablePath < b.tablePath; }

void TableCatalog::SortEntries()
{
   std::sort(m_entries.begin(), m_entries.end(), EntrySortLess);
   RebuildPositions();
}

void TableCatalog::RebuildPositions()
{
   m_positions.clear();
   m_positions.reserve(m_entries.size());
   for (size_t i = 0; i < m_entries.size(); ++i)
      m_positions[m_entries[i].tablePath] = i;
}


// Regular files directly inside a folder, empty when the folder cannot be listed
static vector<std::filesystem::path> ListRegularFiles(const std::filesystem::path& folder)
{
   vector<std::filesystem::path> files;
   std::error_code ec;
   for (std::filesystem::directory_iterator it(folder, std::filesystem::directory_options::skip_permission_denied, ec), end; it != end; it.increment(ec))
   {
      if (ec)
      {
         ec.clear();
         continue;
      }
      if (it->is_regular_file(ec))
         files.push_back(it->path());
   }
   return files;
}


void TableCatalog::ScanFolder(const std::filesystem::path& folder, vector<Entry>& entries, const std::function<void(size_t)>& progress, vector<std::filesystem::path>* cacheFolders)
{
   // Companion/asset subfolders which never hold tables (see docs/FileLayout.md)
   static const ankerl::unordered_dense::set<string> assetFolders
      = { "medias"s, "cache"s, "user"s, "music"s, "altsound"s, "altcolor"s, "pinmame"s, "pupvideos"s, "serum"s, "vni"s, "scripts"s, "dmd"s };

   // Iterate one directory level at a time, collecting subfolders to visit: a single
   // failing entry (broken link, unreadable stat, ...) only aborts its own directory
   // listing — a recursive iterator is left at the end on error, ending the whole walk.
   // Each folder is enumerated exactly once: table files, folder packs, companion '.info'
   // files and the shared 'medias' folder are all resolved from this single listing, the
   // per-table work below performing no further enumeration of the folder
   vector<std::filesystem::path> pending { folder };
   while (!pending.empty())
   {
      const std::filesystem::path dir = pending.back();
      pending.pop_back();
      FolderFileIndex files;
      vector<std::filesystem::path> tables, packs;
      std::filesystem::path mediaDir;
      std::error_code ec;
      for (std::filesystem::directory_iterator it(dir, std::filesystem::directory_options::skip_permission_denied, ec), end; it != end; it.increment(ec))
      {
         if (ec)
         {
            ec.clear();
            continue;
         }
         const std::filesystem::path& path = it->path();
         try
         {
            const string name = PathToUTF8(path.filename());
            if (it->is_directory(ec))
            {
               const string lname = lowerCase(name);
               if (name.starts_with('.') || assetFolders.contains(lname))
               {
                  if (lname == "cache"s && cacheFolders != nullptr)
                     cacheFolders->push_back(path);
                  if (lname == "medias"s && mediaDir.empty())
                     mediaDir = path;
               }
               else if (FileExists(path / "manifest.json"s))
                  packs.push_back(path); // Folder pack: its files are not scanned as standalone tables
               else
                  pending.push_back(path);
            }
            else if (it->is_regular_file(ec))
            {
               const string extension = lowerCase(PathToUTF8(path.extension()));
               if (extension == ".vpx"s || extension == ".vpz"s)
                  tables.push_back(path);
               else if (extension == ".info"s)
                  files[lowerCase(name)] = path;
            }
         }
         catch (const std::exception& e)
         {
            PLOGW << "Frontend table scan skipped '" << path << "': " << e.what();
         }
      }

      // The 'medias' folder is shared by a directory's loose tables. Only its path is kept here:
      // its content is indexed on demand, once, the first time an entry's details are needed.
      for (const std::filesystem::path& table : tables)
      {
         try
         {
            entries.push_back(BuildEntry(table, lowerCase(PathToUTF8(table.extension())) == ".vpz"s, files, mediaDir));
         }
         catch (const std::exception& e)
         {
            PLOGW << "Frontend table scan skipped '" << table << "': " << e.what();
         }
      }
      for (const std::filesystem::path& pack : packs)
      {
         try
         {
            entries.push_back(BuildEntry(pack, true, files, std::filesystem::path()));
         }
         catch (const std::exception& e)
         {
            PLOGW << "Frontend table scan skipped '" << pack << "': " << e.what();
         }
      }
      if (progress)
         progress(entries.size());
   }
}


static void FillFileStamp(const std::filesystem::path& path, int64_t& writeTime, int64_t* size = nullptr)
{
   writeTime = 0;
   if (size != nullptr)
      *size = 0;
   std::error_code ec;
   const auto time = std::filesystem::last_write_time(path, ec);
   if (!ec)
      writeTime = time.time_since_epoch().count();
   if (size != nullptr && std::filesystem::is_regular_file(path, ec))
   {
      const auto fileSize = std::filesystem::file_size(path, ec);
      if (!ec)
         *size = static_cast<int64_t>(fileSize);
   }
}

static bool CanReuseDetails(const TableCatalog::Entry& cached, const TableCatalog::Entry& scanned)
{
   return cached.detailsLoaded && cached.tableWriteTime == scanned.tableWriteTime && cached.tableSize == scanned.tableSize && cached.infoWriteTime == scanned.infoWriteTime
      && cached.infoSize == scanned.infoSize && cached.mediaFolderWriteTime == scanned.mediaFolderWriteTime;
}

static TableCatalog::Entry MergeScannedEntry(const TableCatalog::Entry& cached, const TableCatalog::Entry& scanned)
{
   if (!CanReuseDetails(cached, scanned))
      return scanned;
   TableCatalog::Entry merged = scanned;
   merged.title = cached.title;
   merged.sortTitle = lowerCase(cached.sortTitle);
   merged.detailsLoaded = true;
   merged.info = cached.info;
   merged.user = cached.user;
   merged.vpx = cached.vpx;
   for (int type = 0; type < static_cast<int>(TableCatalog::MediaType::Count); ++type)
      merged.medias[type] = cached.medias[type];
   return merged;
}

void TableCatalog::UpsertEntries(const vector<Entry>& entries)
{
   if (entries.empty())
      return;

   // The catalog is kept sorted for the wheel. Applying every asynchronous batch with a full
   // sort is quadratic over a scan and stalls navigation on large libraries, so only entries
   // whose ordering key changed (plus genuinely new entries) are merged back into the sorted vector.
   std::unordered_map<std::filesystem::path, Entry> uniqueUpdates;
   for (const Entry& scanned : entries)
      if (!scanned.tablePath.empty())
         uniqueUpdates[scanned.tablePath] = scanned;

   vector<Entry> additions;
   additions.reserve(uniqueUpdates.size());
   ankerl::unordered_dense::set<std::filesystem::path> movedPaths;
   for (auto& [path, scanned] : uniqueUpdates)
   {
      const auto position = m_positions.find(path);
      if (position == m_positions.end())
      {
         additions.push_back(std::move(scanned));
         continue;
      }

      Entry& existing = m_entries[position->second];
      if (!CanReuseDetails(existing, scanned))
         m_mediaSets.erase(scanned.mediaFolder);
      Entry merged = MergeScannedEntry(existing, scanned);
      if (existing.sortTitle == merged.sortTitle)
         existing = std::move(merged); // Same ordering key: no reordering work needed
      else
      {
         movedPaths.insert(path);
         additions.push_back(std::move(merged));
      }
   }
   if (additions.empty())
      return;

   if (!movedPaths.empty())
      m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(), [&movedPaths](const Entry& entry) { return movedPaths.contains(entry.tablePath); }), m_entries.end());
   std::sort(additions.begin(), additions.end(), EntrySortLess);
   vector<Entry> mergedEntries;
   mergedEntries.reserve(m_entries.size() + additions.size());
   std::merge(m_entries.begin(), m_entries.end(), additions.begin(), additions.end(), std::back_inserter(mergedEntries), EntrySortLess);
   m_entries = std::move(mergedEntries);
   RebuildPositions();
}

void TableCatalog::ReconcileEntries(const vector<Entry>& scannedEntries)
{
   vector<Entry> reconciled;
   reconciled.reserve(scannedEntries.size());
   for (const Entry& scanned : scannedEntries)
   {
      if (scanned.tablePath.empty())
         continue;
      if (const auto it = m_positions.find(scanned.tablePath); it != m_positions.end())
      {
         const Entry& cached = m_entries[it->second];
         if (!CanReuseDetails(cached, scanned))
            m_mediaSets.erase(scanned.mediaFolder);
         reconciled.push_back(MergeScannedEntry(cached, scanned));
      }
      else
         reconciled.push_back(scanned);
   }
   m_entries = std::move(reconciled);
   SortEntries();
}

bool TableCatalog::TableExists(const Entry& entry)
{
   std::error_code ec;
   if (!entry.isPack)
      return std::filesystem::is_regular_file(entry.tablePath, ec);
   if (std::filesystem::is_directory(entry.tablePath, ec))
      return std::filesystem::is_regular_file(entry.tablePath / "manifest.json"s, ec);
   return std::filesystem::is_regular_file(entry.tablePath, ec);
}

TableCatalog::Entry TableCatalog::BuildEntry(const std::filesystem::path& tablePath, bool isPack, const FolderFileIndex& folderFiles, const std::filesystem::path& sharedMediaFolder)
{
   Entry entry;
   entry.tablePath = tablePath;
   entry.isPack = isPack;
   entry.title = PathToUTF8(tablePath.stem());
   entry.sortTitle = lowerCase(entry.title);
   std::error_code ec;
   const bool isFolderPack = std::filesystem::is_directory(tablePath, ec);
   entry.mediaFolder = isFolderPack ? tablePath / "medias"s : sharedMediaFolder;
   const std::filesystem::path infoFile = FindInfoFile(tablePath, isFolderPack, folderFiles);
   entry.infoPath = infoFile.empty() ? GetDefaultInfoPath(tablePath) : infoFile;
   entry.hasInfoFile = !infoFile.empty();
   FillFileStamp(entry.tablePath, entry.tableWriteTime, &entry.tableSize);
   if (entry.hasInfoFile)
      FillFileStamp(entry.infoPath, entry.infoWriteTime, &entry.infoSize);
   FillFileStamp(entry.mediaFolder, entry.mediaFolderWriteTime);
   return entry;
}


// Media files other than still images and videos (audio, documents, ...) are dropped while scanning.
// Stills are decoded through BaseTexture (stb plus FreeImage), videos and animated images (APNG)
// through the shared VPX::MediaPlayer decoding service.
static bool IsSupportedMediaFile(const std::filesystem::path& path) { return TableCatalog::IsImageFile(path) || TableCatalog::IsVideoFile(path); }

bool TableCatalog::IsImageFile(const std::filesystem::path& path)
{
   static const char* const extensions[] = { ".png", ".apng", ".jpg", ".jpeg", ".bmp", ".gif", ".tga", ".webp", ".exr", ".hdr", ".dds", ".tif", ".tiff" };
   const string extension = lowerCase(PathToUTF8(path.extension()));
   for (const char* const e : extensions)
      if (extension == e)
         return true;
   return false;
}

// Videos follow the same naming convention as stills ((Type) name.ext) but are decoded and
// displayed by the frontend through the shared media decoding service instead of BaseTexture
bool TableCatalog::IsVideoFile(const std::filesystem::path& path)
{
   static const char* const extensions[] = { ".mp4", ".mkv", ".webm", ".avi", ".mov", ".mpg", ".mpeg", ".m4v", ".f4v", ".wmv", ".flv" };
   const string extension = lowerCase(PathToUTF8(path.extension()));
   for (const char* const e : extensions)
      if (extension == e)
         return true;
   return false;
}


TableCatalog::MediaSet TableCatalog::BuildMediaSet(const vector<std::filesystem::path>& files)
{
   MediaSet medias;
   for (const std::filesystem::path& path : files)
   {
      if (!IsSupportedMediaFile(path))
         continue;
      const string filename = PathToUTF8(path.filename());
      if (!filename.starts_with('('))
         continue;
      const size_t close = filename.find(')');
      if (close == string::npos)
         continue;
      const string typeName = filename.substr(1, close - 1);
      for (int type = 0; type < static_cast<int>(MediaType::Count); ++type)
         if (StrCompareNoCase(typeName, GetMediaTypeName(static_cast<MediaType>(type))))
         {
            // Animated medias (videos, animated images) are favored over stills: the
            // animation check opens the file, so it only runs when several candidates
            // compete for a same media type
            if (medias[type].empty())
               medias[type] = path;
            else if (IsAnimatedMediaFile(path) && !IsAnimatedMediaFile(medias[type]))
               medias[type] = path;
         }
   }
   return medias;
}

const TableCatalog::MediaSet& TableCatalog::GetMediaSet(const std::filesystem::path& folder)
{
   static const MediaSet empty;
   if (folder.empty())
      return empty;
   const auto [it, inserted] = m_mediaSets.try_emplace(folder);
   if (inserted)
      it->second = BuildMediaSet(ListRegularFiles(folder));
   return it->second;
}


// Binary scan cache. This is intentionally a private, versioned format: it is not user data,
// only a fast snapshot of the last scan, and may be invalidated freely when the layout changes.
static constexpr char kCatalogCacheMagic[8] = { 'V', 'P', 'X', 'T', 'C', 'A', 'T', '1' };
static constexpr uint32_t kCatalogCacheVersion = 1;

static vector<string> CatalogCacheRootKeys(const vector<std::filesystem::path>& roots)
{
   vector<string> keys;
   keys.reserve(roots.size());
   for (const std::filesystem::path& root : roots)
      keys.push_back(PathToUTF8(root));
   std::ranges::sort(keys);
   return keys;
}

static void WriteU32(std::ostream& out, const uint32_t v)
{
   for (int shift = 0; shift < 32; shift += 8)
      out.put(static_cast<char>((v >> shift) & 0xff));
}

static void WriteU64(std::ostream& out, const uint64_t v)
{
   for (int shift = 0; shift < 64; shift += 8)
      out.put(static_cast<char>((v >> shift) & 0xff));
}

static void WriteI64(std::ostream& out, const int64_t v) { WriteU64(out, static_cast<uint64_t>(v)); }
static void WriteBool(std::ostream& out, const bool v) { out.put(v ? 1 : 0); }

static void WriteString(std::ostream& out, const string& v)
{
   WriteU32(out, static_cast<uint32_t>(v.size()));
   out.write(v.data(), static_cast<std::streamsize>(v.size()));
}

static void WritePath(std::ostream& out, const std::filesystem::path& v) { WriteString(out, PathToUTF8(v)); }

static void WriteStringList(std::ostream& out, const vector<string>& values)
{
   WriteU32(out, static_cast<uint32_t>(values.size()));
   for (const string& value : values)
      WriteString(out, value);
}

class CatalogCacheReader
{
public:
   explicit CatalogCacheReader(const std::filesystem::path& path)
   {
      std::ifstream file(path, std::ios::binary | std::ios::ate);
      if (!file.is_open())
         return;
      const std::streamsize size = file.tellg();
      if (size <= 0)
         return;
      m_data.resize(static_cast<size_t>(size));
      file.seekg(0);
      if (!file.read(reinterpret_cast<char*>(m_data.data()), size))
         m_data.clear();
   }

   bool IsOpen() const { return !m_data.empty(); }
   bool AtEnd() const { return m_pos == m_data.size(); }

   bool ReadU32(uint32_t& v)
   {
      uint64_t value = 0;
      if (!ReadBits(value, 32))
         return false;
      v = static_cast<uint32_t>(value);
      return true;
   }

   bool ReadU64(uint64_t& v) { return ReadBits(v, 64); }

   bool ReadI64(int64_t& v)
   {
      uint64_t value = 0;
      if (!ReadU64(value))
         return false;
      v = static_cast<int64_t>(value);
      return true;
   }

   bool ReadBool(bool& v)
   {
      uint8_t value = 0;
      if (!ReadBytes(&value, 1))
         return false;
      v = value != 0;
      return true;
   }

   bool ReadString(string& v)
   {
      uint32_t length = 0;
      if (!ReadU32(length) || length > 16 * 1024 * 1024 || length > m_data.size() - m_pos)
         return false;
      v.assign(reinterpret_cast<const char*>(m_data.data() + m_pos), length);
      m_pos += length;
      return true;
   }

   bool ReadPath(std::filesystem::path& v)
   {
      string utf8;
      if (!ReadString(utf8))
         return false;
      v = PathFromUTF8(utf8);
      return true;
   }

   bool ReadStringList(vector<string>& values)
   {
      uint32_t count = 0;
      if (!ReadU32(count) || count > 100000)
         return false;
      values.clear();
      values.reserve(count);
      for (uint32_t i = 0; i < count; ++i)
      {
         string value;
         if (!ReadString(value))
            return false;
         values.push_back(std::move(value));
      }
      return true;
   }

   bool ReadBytes(void* dst, const size_t count)
   {
      if (count > m_data.size() - m_pos)
         return false;
      memcpy(dst, m_data.data() + m_pos, count);
      m_pos += count;
      return true;
   }

private:
   bool ReadBits(uint64_t& v, const int bits)
   {
      if (m_data.size() - m_pos < static_cast<size_t>(bits / 8))
         return false;
      v = 0;
      for (int shift = 0; shift < bits; shift += 8)
         v |= static_cast<uint64_t>(m_data[m_pos++]) << shift;
      return true;
   }

   vector<uint8_t> m_data;
   size_t m_pos = 0;
};

static void WriteCacheEntry(std::ostream& out, const TableCatalog::Entry& entry)
{
   WritePath(out, entry.tablePath);
   WritePath(out, entry.infoPath);
   WriteBool(out, entry.hasInfoFile);
   WriteBool(out, entry.isPack);
   WriteString(out, entry.title);
   WriteString(out, entry.sortTitle);
   WritePath(out, entry.mediaFolder);
   WriteBool(out, entry.detailsLoaded);
   WriteI64(out, entry.tableWriteTime);
   WriteI64(out, entry.tableSize);
   WriteI64(out, entry.infoWriteTime);
   WriteI64(out, entry.infoSize);
   WriteI64(out, entry.mediaFolderWriteTime);
   WriteString(out, entry.info.title);
   WriteString(out, entry.info.description);
   WriteStringList(out, entry.info.authors);
   WriteString(out, entry.info.manufacturer);
   WriteI64(out, entry.info.year);
   WriteString(out, entry.info.vpsId);
   WriteString(out, entry.info.ipdbId);
   WriteI64(out, entry.user.rating);
   WriteBool(out, entry.user.favorite);
   WriteString(out, entry.user.lastRun);
   WriteI64(out, entry.user.startCount);
   WriteI64(out, entry.user.runTime);
   WriteStringList(out, entry.user.tags);
   WriteString(out, entry.vpx.version);
   WriteString(out, entry.vpx.lastRun);
   WriteI64(out, entry.vpx.startCount);
   WriteI64(out, entry.vpx.runTime);
   for (const std::filesystem::path& media : entry.medias)
      WritePath(out, media);
}

static bool ReadCacheEntry(CatalogCacheReader& reader, TableCatalog::Entry& entry)
{
   int64_t year = 0, rating = 0, userStartCount = 0, userRunTime = 0, vpxStartCount = 0, vpxRunTime = 0;
   if (!reader.ReadPath(entry.tablePath) || !reader.ReadPath(entry.infoPath) || !reader.ReadBool(entry.hasInfoFile) || !reader.ReadBool(entry.isPack) || !reader.ReadString(entry.title)
      || !reader.ReadString(entry.sortTitle) || !reader.ReadPath(entry.mediaFolder) || !reader.ReadBool(entry.detailsLoaded) || !reader.ReadI64(entry.tableWriteTime)
      || !reader.ReadI64(entry.tableSize) || !reader.ReadI64(entry.infoWriteTime) || !reader.ReadI64(entry.infoSize) || !reader.ReadI64(entry.mediaFolderWriteTime)
      || !reader.ReadString(entry.info.title) || !reader.ReadString(entry.info.description) || !reader.ReadStringList(entry.info.authors) || !reader.ReadString(entry.info.manufacturer)
      || !reader.ReadI64(year) || !reader.ReadString(entry.info.vpsId) || !reader.ReadString(entry.info.ipdbId) || !reader.ReadI64(rating) || !reader.ReadBool(entry.user.favorite)
      || !reader.ReadString(entry.user.lastRun) || !reader.ReadI64(userStartCount) || !reader.ReadI64(userRunTime) || !reader.ReadStringList(entry.user.tags)
      || !reader.ReadString(entry.vpx.version) || !reader.ReadString(entry.vpx.lastRun) || !reader.ReadI64(vpxStartCount) || !reader.ReadI64(vpxRunTime))
      return false;
   entry.info.year = static_cast<int>(year);
   entry.user.rating = static_cast<int>(rating);
   entry.user.startCount = static_cast<int>(userStartCount);
   entry.user.runTime = userRunTime;
   entry.vpx.startCount = static_cast<int>(vpxStartCount);
   entry.vpx.runTime = vpxRunTime;
   for (std::filesystem::path& media : entry.medias)
      if (!reader.ReadPath(media))
         return false;
   return !entry.tablePath.empty();
}

bool TableCatalog::SaveCacheFile(const std::filesystem::path& path, const vector<std::filesystem::path>& roots) const
{
   std::error_code ec;
   if (!path.parent_path().empty())
      std::filesystem::create_directories(path.parent_path(), ec);
   std::filesystem::path tempPath = path;
   tempPath += ".tmp";
   {
      std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
      if (!out.is_open())
         return false;
      out.write(kCatalogCacheMagic, sizeof(kCatalogCacheMagic));
      WriteU32(out, kCatalogCacheVersion);
      const vector<string> rootKeys = CatalogCacheRootKeys(roots);
      WriteU32(out, static_cast<uint32_t>(rootKeys.size()));
      for (const string& root : rootKeys)
         WriteString(out, root);
      WriteU32(out, static_cast<uint32_t>(m_entries.size()));
      for (const Entry& entry : m_entries)
         WriteCacheEntry(out, entry);
      if (!out.good())
         return false;
   }
   std::filesystem::rename(tempPath, path, ec);
   if (ec)
   {
      ec.clear();
      std::filesystem::remove(path, ec);
      ec.clear();
      std::filesystem::rename(tempPath, path, ec);
   }
   return !ec;
}

bool TableCatalog::LoadCacheFile(const std::filesystem::path& path, const vector<std::filesystem::path>& roots)
{
   CatalogCacheReader reader(path);
   if (!reader.IsOpen())
      return false;
   char magic[sizeof(kCatalogCacheMagic)];
   uint32_t version = 0, rootCount = 0, entryCount = 0;
   if (!reader.ReadBytes(magic, sizeof(magic)) || memcmp(magic, kCatalogCacheMagic, sizeof(magic)) != 0 || !reader.ReadU32(version) || version != kCatalogCacheVersion
      || !reader.ReadU32(rootCount) || rootCount > 10000)
      return false;
   vector<string> cachedRoots;
   for (uint32_t i = 0; i < rootCount; ++i)
   {
      string root;
      if (!reader.ReadString(root))
         return false;
      cachedRoots.push_back(root);
   }
   if (cachedRoots != CatalogCacheRootKeys(roots) || !reader.ReadU32(entryCount) || entryCount > 1000000)
      return false;
   vector<Entry> entries;
   entries.reserve(entryCount);
   for (uint32_t i = 0; i < entryCount; ++i)
   {
      Entry entry;
      if (!ReadCacheEntry(reader, entry))
         return false;
      entry.sortTitle = lowerCase(entry.sortTitle);
      entries.push_back(std::move(entry));
   }
   if (!reader.AtEnd())
      return false;
   m_entries = std::move(entries);
   m_mediaSets.clear();
   SortEntries();
   return true;
}


void TableCatalog::ReadInfoFile(Entry& entry)
{
   std::stringstream buffer;
   buffer << std::ifstream(entry.infoPath, std::ios::binary).rdbuf();
   nlohmann::ordered_json doc = nlohmann::ordered_json::parse(buffer.str(), nullptr, false);
   if (doc.is_discarded())
   {
      PLOGW << "Failed to parse table info file " << entry.infoPath;
      return;
   }

   const auto asString = [](const nlohmann::ordered_json& value, const string& fallback = ""s) -> string
   {
      if (value.is_string())
         return value.get<string>();
      if (value.is_number())
         return std::to_string(value.get<int64_t>());
      return fallback;
   };
   const auto asInt = [](const nlohmann::ordered_json& value, int64_t fallback = 0) -> int64_t
   {
      if (value.is_number_integer() || value.is_number_unsigned())
         return value.get<int64_t>();
      if (value.is_number_float())
         return static_cast<int64_t>(value.get<double>());
      if (value.is_string())
      {
         int64_t parsed = fallback;
         const string str = value.get<string>();
         std::from_chars(str.c_str(), str.c_str() + str.length(), parsed);
         return parsed;
      }
      return fallback;
   };
   const auto asStringList = [&asString](const nlohmann::ordered_json& value) -> vector<string>
   {
      vector<string> list;
      if (value.is_array())
      {
         for (const auto& item : value)
            if (item.is_string() || item.is_number())
               list.push_back(asString(item));
      }
      else if (value.is_string())
         list.push_back(value.get<string>());
      return list;
   };

   if (const auto info = doc.find("Info"s); info != doc.end() && info->is_object())
   {
      entry.info.title = asString(info->value("Title"s, nlohmann::ordered_json()));
      entry.info.description = asString(info->value("Description"s, nlohmann::ordered_json()));
      entry.info.authors = asStringList(info->value("Authors"s, nlohmann::ordered_json()));
      entry.info.manufacturer = asString(info->value("Manufacturer"s, nlohmann::ordered_json()));
      entry.info.year = static_cast<int>(asInt(info->value("Year"s, nlohmann::ordered_json())));
      entry.info.vpsId = asString(info->value("VPSId"s, nlohmann::ordered_json()));
      entry.info.ipdbId = asString(info->value("IPDBId"s, nlohmann::ordered_json()));
      if (!entry.info.title.empty())
         entry.title = entry.info.title;
   }

   if (const auto user = doc.find("User"s); user != doc.end() && user->is_object())
   {
      entry.user.rating = static_cast<int>(asInt(user->value("Rating"s, nlohmann::ordered_json())));
      const auto favorite = user->value("Favorite"s, nlohmann::ordered_json());
      entry.user.favorite = favorite.is_boolean() ? favorite.get<bool>() : (asInt(favorite) != 0);
      entry.user.lastRun = asString(user->value("LastRun"s, nlohmann::ordered_json()));
      entry.user.startCount = static_cast<int>(asInt(user->value("StartCount"s, nlohmann::ordered_json())));
      entry.user.runTime = asInt(user->value("RunTime"s, nlohmann::ordered_json()));
      entry.user.tags = asStringList(user->value("Tags"s, nlohmann::ordered_json()));
   }

   if (const auto vpx = doc.find("VPX"s); vpx != doc.end() && vpx->is_object())
   {
      entry.vpx.version = asString(vpx->value("Version"s, nlohmann::ordered_json()));
      entry.vpx.lastRun = asString(vpx->value("LastRun"s, nlohmann::ordered_json()));
      entry.vpx.startCount = static_cast<int>(asInt(vpx->value("StartCount"s, nlohmann::ordered_json())));
      entry.vpx.runTime = asInt(vpx->value("RunTime"s, nlohmann::ordered_json()));
   }
}

bool TableCatalog::SaveInfoFile(const Entry& entry)
{
   // Parse the existing document to preserve the VPX section and unknown keys
   nlohmann::ordered_json doc;
   if (entry.hasInfoFile)
   {
      std::stringstream buffer;
      buffer << std::ifstream(entry.infoPath, std::ios::binary).rdbuf();
      doc = nlohmann::ordered_json::parse(buffer.str(), nullptr, false);
      if (doc.is_discarded() || !doc.is_object())
         doc = nlohmann::ordered_json::object();
   }
   else
      doc = nlohmann::ordered_json::object();

   auto& info = doc["Info"s];
   if (!info.is_object())
      info = nlohmann::ordered_json::object();
   info["Title"s] = entry.info.title;
   info["Description"s] = entry.info.description;
   info["Authors"s] = entry.info.authors;
   info["Manufacturer"s] = entry.info.manufacturer;
   info["Year"s] = entry.info.year;
   info["VPSId"s] = entry.info.vpsId;
   info["IPDBId"s] = entry.info.ipdbId;

   auto& user = doc["User"s];
   if (!user.is_object())
      user = nlohmann::ordered_json::object();
   user["Rating"s] = entry.user.rating;
   user["Favorite"s] = entry.user.favorite;
   user["Tags"s] = entry.user.tags;

   std::error_code ec;
   std::filesystem::create_directories(entry.infoPath.parent_path(), ec);
   std::ofstream file(entry.infoPath, std::ios::binary | std::ios::trunc);
   if (!file.is_open())
   {
      PLOGE << "Failed to write table info file " << entry.infoPath;
      return false;
   }
   file << doc.dump(2);
   return file.good();
}
