// license:GPLv3+

#include "PUPPlaylist.h"

#include <filesystem>
#include <algorithm>
#include <random>

namespace PUP {

/*
   playlists.pup: ScreenNum,Folder,Des,AlphaSort,RestSeconds,Volume,Priority
   PuP Pack Editor: Folder (Playlist),Description,Randomize,RestSeconds,Volume,Priority

   mappings:

     ScreenNum = ?
     Folder = Folder (Playlist)
     Des = Description
     AlphaSort = Randomize
     RestSeconds = RestSeconds
     Volume = Volume
     Priority = Priority

   notes:

     AlphaSort=1 is Randomize checked
*/

static const std::filesystem::path emptyPath;

static bool IsPlaylistMedia(const std::filesystem::path& filename)
{
   static const ankerl::unordered_dense::set<string> extensions { "mp4", "mp3", "avi", "mkv", "mov", "asf", "vob", "ogg", "flv", "wav", "png", "gif", "jpg", "jpeg", "m4v",
      "webm", "wmv", "mpg", "mpeg", "m4a", "aac", "flac", "bmp", "webp", "apng" };
   const string name = PluginStrings::PathToUTF8(filename);
   return !name.starts_with('.') && extensions.contains(extension_from_path(name));
}

static std::filesystem::path FileKey(const std::filesystem::path& filename)
{
   return lowerCase(PluginStrings::PathFromUTF8(normalize_path_separators(PluginStrings::PathToUTF8(filename))));
}

PUPPlaylist::PUPPlaylist(PUPManager* manager, const std::filesystem::path& szFolder, const string& szDescription, bool randomize, int restSeconds, float volume, int priority)
{
   m_szFolder = szFolder;
   m_szDescription = szDescription;
   m_randomize = randomize;
   m_restSeconds = restSeconds;
   m_volume = volume;
   m_priority = priority;
   m_lastIndex = 0;

   const string szFolderName = PluginStrings::PathToUTF8(szFolder);
   if (StrCompareNoCase(szFolderName, "PUPOverlays"s))
      m_function = PUPPlaylist::Function::Overlays;
   else if (StrCompareNoCase(szFolderName, "PUPFrames"s))
      m_function = PUPPlaylist::Function::Frames;
   else if (StrCompareNoCase(szFolderName, "PUPAlphas"s))
      m_function = PUPPlaylist::Function::Alphas;
   else if (StrCompareNoCase(szFolderName, "PuPShapes"s))
      m_function = PUPPlaylist::Function::Shapes;
   else
      m_function = PUPPlaylist::Function::Default;

   m_szBasePath = find_case_insensitive_directory_path(manager->GetPath() / szFolder);
   if (m_szBasePath.empty()) {
      LOGE("Playlist folder not found: " + szFolderName);
      return;
   }

   for (const auto& entry : std::filesystem::recursive_directory_iterator(m_szBasePath)) {
      if (entry.is_regular_file()) {
         std::filesystem::path szFilename = entry.path().lexically_relative(m_szBasePath);
         if (!szFilename.empty() && szFilename != ".") {
            // Only top level files take part in the playlist rotation, files in subfolders
            // can still be requested explicitly (e.g. LabelSet with playlist\subdir\file.png)
            if (szFilename.parent_path().empty() && IsPlaylistMedia(szFilename))
               m_files.push_back(szFilename);
            m_fileMap[lowerCase(szFilename)] = szFilename;
         }
      }
   }
   std::ranges::sort(m_files.begin(), m_files.end());
   if (m_randomize)
      std::shuffle(m_files.begin(), m_files.end(), std::mt19937(std::random_device {}()));
}

PUPPlaylist* PUPPlaylist::CreateFromCSV(PUPManager* manager, const string& line)
{
   if (trim_string(line).empty())
      return nullptr;
   vector<string> parts = parse_csv_line(line);
   if (parts.size() < 7) {
      LOGE("Invalid playlist: " + line);
      return nullptr;
   }

   std::filesystem::path szFolderPath = find_case_insensitive_directory_path(manager->GetPath() / PluginStrings::PathFromUTF8(parts[1]));
   if (szFolderPath.empty()) {
      LOGE("Playlist folder not found: " + parts[1]);
      return nullptr;
   }

   PUPPlaylist* pPlaylist = new PUPPlaylist(
      manager, //
      szFolderPath.filename(), // Subfolder
      parts[2], // Description
      (string_to_int(parts[3], 0) == 1), // Randomize
      string_to_int(parts[4], 30), // Rest seconds
      static_cast<float>(string_to_int(parts[5], 100)), // Volume
      string_to_int(parts[6], 0)); // Priority

   if (!pPlaylist->HasFiles()) {
      // TODO add to a pup pack audit, we log as info as not a big deal.
      LOGW("Playlist folder " + PluginStrings::PathToUTF8(szFolderPath) + " is empty");
   }

   return pPlaylist;
}

std::filesystem::path PUPPlaylist::GetPlayFile(const std::filesystem::path& szFilename)
{
   ankerl::unordered_dense::map<std::filesystem::path, std::filesystem::path>::const_iterator it = m_fileMap.find(FileKey(szFilename));
   return it != m_fileMap.end() ? it->second : emptyPath;
}

const std::filesystem::path& PUPPlaylist::GetNextPlayFile()
{
   const std::filesystem::path& file = m_files[m_lastIndex];
   if (++m_lastIndex >= (int)m_files.size())
      m_lastIndex = 0;
   return file;
}

const std::filesystem::path& PUPPlaylist::PeekNextPlayFile() const
{
   return m_files.empty() ? emptyPath : m_files[m_lastIndex];
}

std::filesystem::path PUPPlaylist::GetPlayFilePath(const std::filesystem::path& szFilename)
{
   if (!szFilename.empty()) {
      ankerl::unordered_dense::map<std::filesystem::path, std::filesystem::path>::const_iterator it = m_fileMap.find(FileKey(szFilename));
      if (it != m_fileMap.end())
         return m_szBasePath / it->second;
      else
         return emptyPath;
   }

   if (m_files.empty())
      return emptyPath;

   return m_szBasePath / GetNextPlayFile();
}

bool PUPPlaylist::IsResting() const
{
   if (m_restSeconds <= 0 || m_lastPlayed == 0)
      return false;
   return (SDL_GetTicks() - m_lastPlayed) < (uint64_t)m_restSeconds * 1000;
}

void PUPPlaylist::MarkPlayed()
{
   m_lastPlayed = SDL_GetTicks();
}

string PUPPlaylist::ToString() const {
   return "folder=" + PluginStrings::PathToUTF8(m_szFolder) +
      ", description=" + m_szDescription +
      ", randomize=" + (m_randomize ? "true" : "false") +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", function=" + ToString(m_function);
}

const string& PUPPlaylist::ToString(PUPPlaylist::Function value)
{
   static const string PUP_PLAYLIST_FUNCTION_STRINGS[] = { "Default"s, "Overlays"s, "Frames"s, "Alphas"s, "Shapes"s };
   static const string error = "Unknown"s;
   if ((int)value < 0 || (size_t)value >= std::size(PUP_PLAYLIST_FUNCTION_STRINGS))
      return error;
   return PUP_PLAYLIST_FUNCTION_STRINGS[(size_t)value];
}

}
