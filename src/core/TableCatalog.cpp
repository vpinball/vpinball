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

// Animated image detection is memoized per media path (sniffing opens the file); rescans clear it
static std::mutex s_animatedImageMutex;
static std::unordered_map<std::filesystem::path, bool> s_animatedImageFiles;

bool TableCatalog::IsAnimatedImageFile(const std::filesystem::path& path)
{
   std::lock_guard lock(s_animatedImageMutex);
   const auto it = s_animatedImageFiles.find(path);
   if (it != s_animatedImageFiles.end())
      return it->second;
   return s_animatedImageFiles.emplace(path, SniffApng(path)).first->second;
}

bool TableCatalog::IsAnimatedMediaFile(const std::filesystem::path& path) { return IsVideoFile(path) || IsAnimatedImageFile(path); }


void TableCatalog::Scan(const vector<std::filesystem::path>& roots)
{
   {
      std::lock_guard lock(s_animatedImageMutex);
      s_animatedImageFiles.clear();
   }
   m_entries.clear();
   for (const std::filesystem::path& root : roots)
   {
      std::error_code ec;
      if (std::filesystem::is_directory(root, ec))
         ScanFolder(root, m_entries);
   }
   std::sort(m_entries.begin(), m_entries.end(),
      [](const Entry& a, const Entry& b)
      {
         const int cmp = lowerCase(a.title).compare(lowerCase(b.title));
         return cmp != 0 ? cmp < 0 : a.tablePath < b.tablePath;
      });
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


void TableCatalog::ScanFolder(const std::filesystem::path& folder, vector<Entry>& entries)
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

      // The 'medias' folder is shared by a directory's loose tables: listed once here,
      // folder packs list their own inside the pack
      vector<std::filesystem::path> sharedMedias;
      if (!mediaDir.empty())
         sharedMedias = ListRegularFiles(mediaDir);
      for (const std::filesystem::path& table : tables)
      {
         try
         {
            entries.push_back(BuildEntry(table, lowerCase(PathToUTF8(table.extension())) == ".vpz"s, files, &sharedMedias));
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
            entries.push_back(BuildEntry(pack, true, files, nullptr));
         }
         catch (const std::exception& e)
         {
            PLOGW << "Frontend table scan skipped '" << pack << "': " << e.what();
         }
      }
   }
}


TableCatalog::Entry TableCatalog::BuildEntry(
   const std::filesystem::path& tablePath, bool isPack, const FolderFileIndex& folderFiles, const vector<std::filesystem::path>* sharedMedias)
{
   Entry entry;
   entry.tablePath = tablePath;
   entry.isPack = isPack;
   entry.title = PathToUTF8(tablePath.stem());
   const std::filesystem::path infoFile = FindInfoFile(tablePath, isPack, folderFiles);
   if (infoFile.empty())
      entry.infoPath = GetDefaultInfoPath(tablePath);
   else
   {
      entry.infoPath = infoFile;
      entry.hasInfoFile = true;
      ReadInfoFile(entry);
   }
   ScanMedias(entry, sharedMedias);
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


void TableCatalog::ScanMedias(Entry& entry, const vector<std::filesystem::path>* sharedMedias)
{
   // A directory's loose tables share its 'medias' folder, already listed by ScanFolder;
   // a folder pack lists its own, next to its manifest
   vector<std::filesystem::path> packMedias;
   if (sharedMedias == nullptr)
      packMedias = ListRegularFiles(entry.tablePath / "medias"s);
   const vector<std::filesystem::path>& files = sharedMedias != nullptr ? *sharedMedias : packMedias;
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
            if (entry.medias[type].empty())
               entry.medias[type] = path;
            else if (IsAnimatedMediaFile(path) && !IsAnimatedMediaFile(entry.medias[type]))
               entry.medias[type] = path;
         }
   }
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
