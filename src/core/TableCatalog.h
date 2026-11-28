// license:GPLv3+

#pragma once

#include "unordered_dense.h"

#include <array>
#include <unordered_map>

// Discovers the tables of a set of root folders, along their frontend companion
// datas ('.info' file, 'medias' folder), following the conventions defined in
// docs/FileLayout.md. This is a pure file/metadata layer used by the mini
// frontend: it never loads the table files themselves.
class TableCatalog
{
public:
   // Media file types stored as 'medias/(<Type>) <name>.<ext>' along each table
   enum class MediaType
   {
      Playfield,
      Backglass,
      DMD,
      FullDMD,
      Topper,
      Other,
      RealDMD,
      RealColorDMD,
      Wheel,
      GameHelp,
      GameInfo,
      Loading,
      Audio,
      AudioLaunch,
      Count
   };
   static const char* GetMediaTypeName(MediaType type);

   // 'Info' section of the companion file (static, frontend maintained datas)
   struct Info
   {
      bool operator==(const Info&) const = default;

      string title;
      string description;
      vector<string> authors;
      string manufacturer;
      int year = 0;
      string vpsId;
      string ipdbId;
   };

   // 'User' section of the companion file (user/frontend maintained stats)
   struct UserStats
   {
      bool operator==(const UserStats&) const = default;

      int rating = 0; // 0..10
      bool favorite = false;
      string lastRun; // ISO date of last game start
      int startCount = 0;
      int64_t runTime = 0; // Cumulated play duration (seconds)
      vector<string> tags;
   };

   // 'VPX' section of the companion file (stats maintained by the engine)
   struct VpxStats
   {
      bool operator==(const VpxStats&) const = default;

      string version; // Table file version of the last run
      string lastRun; // ISO date of last game start
      int startCount = 0;
      int64_t runTime = 0; // Cumulated play duration (seconds)
   };

   struct Entry
   {
      bool operator==(const Entry&) const = default;

      std::filesystem::path tablePath; // .vpx file, or VPZ pack (a '.vpz' archive or a folder holding a 'manifest.json')
      std::filesystem::path infoPath; // Located '.info' companion file when found, else the default name to create it
      bool hasInfoFile = false;
      bool isPack = false;

      string title; // Display title: Info.Title when provided, else the table file name
      string sortTitle; // Stable ordering key while details are still deferred (updated when all details load)
      std::filesystem::path mediaFolder; // Folder scanned on demand for this table's media
      bool detailsLoaded = false;

      // Filesystem stamps used to decide whether cached details may be reused. They are
      // implementation-defined filesystem clock values, only meaningful inside one cache version.
      int64_t tableWriteTime = 0;
      int64_t tableSize = 0;
      int64_t infoWriteTime = 0;
      int64_t infoSize = 0;
      int64_t mediaFolderWriteTime = 0;

      Info info;
      UserStats user;
      VpxStats vpx;

      std::filesystem::path medias[static_cast<int>(MediaType::Count)]; // Located media file per type (empty if none)
      bool HasMedia(MediaType type) const { return !medias[static_cast<int>(type)].empty(); }
      const std::filesystem::path& GetMedia(MediaType type) const { return medias[static_cast<int>(type)]; }
   };

   const vector<Entry>& GetEntries() const { return m_entries; }

   // Rebuilds the catalog by recursively scanning each root folder for tables, then loading
   // all companion information and media paths. Missing/unreadable roots are skipped.
   void Scan(const vector<std::filesystem::path>& roots);

   // Rebuilds only the table path catalog: no companion file is read and no media folder is
   // listed. This is the fast pass used to populate the frontend before details arrive. The
   // optional progress callback receives the number of entries discovered so far after each folder.
   void ScanPaths(const vector<std::filesystem::path>& roots, const std::function<void(size_t)>& progress = {}, vector<std::filesystem::path>* cacheFolders = nullptr);

   // Loads a discovered entry's companion information and media paths on demand.
   void EnsureEntryDetails(size_t index);

   // Loads every entry's deferred details then restores ordering by display title.
   void LoadAllDetails();

   // Inserts or updates scanned entries, preserving already loaded details when the table,
   // companion file and media folder stamps are unchanged. Entries missing from the scan are kept.
   void UpsertEntries(const vector<Entry>& entries);

   // Reconciles the catalog with a complete path scan: missing entries are removed, new entries
   // are added, and valid cached details are preserved for entries whose stamps are unchanged.
   void ReconcileEntries(const vector<Entry>& scannedEntries);

   // Binary scan cache. The cache intentionally does not validate entry existence when loading:
   // callers must treat it as a possibly stale snapshot until a scan reconciles it.
   bool SaveCacheFile(const std::filesystem::path& path, const vector<std::filesystem::path>& roots) const;
   bool LoadCacheFile(const std::filesystem::path& path, const vector<std::filesystem::path>& roots);

   // Whether the entry's table file/pack still exists (the catalog itself may be a stale snapshot).
   static bool TableExists(const Entry& entry);

   struct TextureCacheCleanupResult
   {
      size_t staleRemoved = 0;
      size_t overflowRemoved = 0;
      size_t failedRemovals = 0;
   };

   // Maintains the per-table cache folders below the scanned roots. maxRecentTables is negative
   // for unlimited, or the number of most recently used tables whose cache folders are kept.
   static TextureCacheCleanupResult CleanupTextureCaches(
      const vector<std::filesystem::path>& roots, const vector<Entry>& entries, bool discardStale, int maxRecentTables, const vector<std::filesystem::path>* scannedCacheFolders = nullptr);

   // A table file is either a '.vpx' file or a VPZ pack ('.vpz' archive file, or
   // a folder holding a 'manifest.json' file), detected by name only.
   static bool IsTablePath(const std::filesystem::path& path, bool* isPack = nullptr);

   // Index of a scanned folder's files by lower case file name, built from the single
   // enumeration ScanFolder performs, so that companion files are resolved from it
   // without re-listing the folder
   using FolderFileIndex = ankerl::unordered_dense::map<string, std::filesystem::path>;

   // Locates the '.info' companion file of a table inside the given listing of the folder
   // holding it, following the same naming scheme as PinTable::GetCompanionFileName: file
   // named after the table file, then file named after the containing folder. For a folder
   // pack, the in-pack file (not part of the parent's listing) is searched directly.
   // Returns an empty path when none exists.
   static std::filesystem::path FindInfoFile(const std::filesystem::path& tablePath, bool isFolderPack, const FolderFileIndex& folderFiles);

   // The default '.info' file name to create when none exists (alongside the table file)
   static std::filesystem::path GetDefaultInfoPath(const std::filesystem::path& tablePath);

   // Reads the '.info' companion file into the entry's Info/User/Vpx sections.
   static void ReadInfoFile(Entry& entry);

   // Writes the Info and User sections of the entry back to its '.info' companion file, preserving
   // the VPX section and any unknown keys already present. Creates the file when missing.
   // Returns false when the file could not be written.
   static bool SaveInfoFile(const Entry& entry);

   // Whether a file has a still image extension (decoded through BaseTexture, unless animated)
   static bool IsImageFile(const std::filesystem::path& path);

   // Whether a media file is a video (decoded through the shared media service, not BaseTexture)
   static bool IsVideoFile(const std::filesystem::path& path);

   // Whether an image media file is animated (APNG). Animated images keep the still image naming
   // convention but are decoded like videos through the shared media service.
   static bool IsAnimatedImageFile(const std::filesystem::path& path);

   // Whether a media file plays as an animation: a video, or an image carrying animation (APNG).
   // Animated medias are decoded through the shared media service on a per file session.
   static bool IsAnimatedMediaFile(const std::filesystem::path& path);

private:
   using MediaSet = std::array<std::filesystem::path, static_cast<size_t>(MediaType::Count)>;

   static void ScanFolder(const std::filesystem::path& folder, vector<Entry>& entries, const std::function<void(size_t)>& progress, vector<std::filesystem::path>* cacheFolders);
   static Entry BuildEntry(const std::filesystem::path& tablePath, bool isPack, const FolderFileIndex& folderFiles, const std::filesystem::path& sharedMediaFolder);
   static MediaSet BuildMediaSet(const vector<std::filesystem::path>& files);
   const MediaSet& GetMediaSet(const std::filesystem::path& folder);
   void SortEntries();

   vector<Entry> m_entries;
   std::unordered_map<std::filesystem::path, MediaSet> m_mediaSets;
};
