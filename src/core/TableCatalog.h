// license:GPLv3+

#pragma once

#include "unordered_dense.h"

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
      string version; // Table file version of the last run
      string lastRun; // ISO date of last game start
      int startCount = 0;
      int64_t runTime = 0; // Cumulated play duration (seconds)
   };

   struct Entry
   {
      std::filesystem::path tablePath; // .vpx file, or VPZ pack (a '.vpz' archive or a folder holding a 'manifest.json')
      std::filesystem::path infoPath; // Located '.info' companion file when found, else the default name to create it
      bool hasInfoFile = false;
      bool isPack = false;

      string title; // Display title: Info.Title when provided, else the table file name

      Info info;
      UserStats user;
      VpxStats vpx;

      std::filesystem::path medias[static_cast<int>(MediaType::Count)]; // Located media file per type (empty if none)
      bool HasMedia(MediaType type) const { return !medias[static_cast<int>(type)].empty(); }
      const std::filesystem::path& GetMedia(MediaType type) const { return medias[static_cast<int>(type)]; }
   };

   const vector<Entry>& GetEntries() const { return m_entries; }

   // Rebuilds the catalog by recursively scanning each root folder for tables.
   // Missing/unreadable roots are skipped.
   void Scan(const vector<std::filesystem::path>& roots);

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
   // convention but are decoded like videos through the shared media service. The result is
   // memoized per path and refreshed on the next catalog Scan.
   static bool IsAnimatedImageFile(const std::filesystem::path& path);

   // Whether a media file plays as an animation: a video, or an image carrying animation (APNG).
   // Animated medias are decoded through the shared media service on a per file session.
   static bool IsAnimatedMediaFile(const std::filesystem::path& path);

private:
   static void ScanFolder(const std::filesystem::path& folder, vector<Entry>& entries);
   static Entry BuildEntry(
      const std::filesystem::path& tablePath, bool isPack, const FolderFileIndex& folderFiles, const vector<std::filesystem::path>* sharedMedias);
   static void ScanMedias(Entry& entry, const vector<std::filesystem::path>* sharedMedias);

   vector<Entry> m_entries;
};
