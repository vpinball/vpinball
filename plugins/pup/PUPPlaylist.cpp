// license:GPLv3+

#include "PUPPlaylist.h"

#include <filesystem>
#include <algorithm>

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

     AlphaSort=0 is Randomize checked
*/

static const std::filesystem::path emptyPath;

PUPPlaylist::PUPPlaylist(PUPManager* manager, const string& szFolder, const string& szDescription, bool randomize, int restSeconds, float volume, int priority)
{
   m_szFolder = szFolder;
   m_szDescription = szDescription;
   m_randomize = randomize;
   m_restSeconds = restSeconds;
   m_volume = volume;
   m_priority = priority;
   m_lastIndex = 0;

   if (StrCompareNoCase(szFolder, "PUPOverlays"s))
      m_function = PUPPlaylist::Function::Overlays;
   else if (StrCompareNoCase(szFolder, "PUPFrames"s))
      m_function = PUPPlaylist::Function::Frames;
   else if (StrCompareNoCase(szFolder, "PUPAlphas"s))
      m_function = PUPPlaylist::Function::Alphas;
   else if (StrCompareNoCase(szFolder, "PuPShapes"s))
      m_function = PUPPlaylist::Function::Shapes;
   else
      m_function = PUPPlaylist::Function::Default;

   m_szBasePath = find_case_insensitive_directory_path(manager->GetPath() / szFolder);
   if (m_szBasePath.empty()) {
      LOGE("Playlist folder not found: %s", szFolder.c_str());
      return;
   }

   for (const auto& entry : std::filesystem::directory_iterator(m_szBasePath)) {
      if (entry.is_regular_file()) {
         std::filesystem::path szFilename = entry.path().filename();
         if (!szFilename.empty() && szFilename != ".") {
            m_files.push_back(szFilename);
            m_fileMap[lowerCase(szFilename)] = szFilename;
         }
      }
   }
   std::ranges::sort(m_files.begin(), m_files.end());
}

PUPPlaylist::~PUPPlaylist()
{
   m_files.clear();
}

PUPPlaylist* PUPPlaylist::CreateFromCSV(PUPManager* manager, const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 7) {
      LOGE("Invalid playlist: %s", line.c_str());
      return nullptr;
   }

   std::filesystem::path szFolderPath = find_case_insensitive_directory_path(manager->GetPath() / parts[1]);
   if (szFolderPath.empty()) {
      LOGE("Playlist folder not found: %s", parts[1].c_str());
      return nullptr;
   }

   bool hasFiles = false;
   for (const auto& entry : std::filesystem::directory_iterator(szFolderPath)) {
      if (entry.is_regular_file()) {
         std::filesystem::path szFilename = entry.path().filename();
         if (!szFilename.empty() && szFilename != ".") {
            hasFiles = true;
            break;
         }
      }
   }

   if (!hasFiles) {
      // TODO add to a pup pack audit, we log as info as not a big deal.
      LOGI("Playlist folder %s is empty",szFolderPath.string().c_str());
   }

   PUPPlaylist* pPlaylist = new PUPPlaylist(
      manager, //
      szFolderPath.filename().string(), // Subfolder
      parts[2], // Description
      (string_to_int(parts[3], 0) == 1), // Randomize
      string_to_int(parts[4], 0), // Rest seconds
      static_cast<float>(string_to_int(parts[5], 0)), // Volume
      string_to_int(parts[6], 0)); // Priority

   return pPlaylist;
}

std::filesystem::path PUPPlaylist::GetPlayFile(const std::filesystem::path& szFilename)
{
   ankerl::unordered_dense::map<std::filesystem::path, std::filesystem::path>::const_iterator it = m_fileMap.find(lowerCase(szFilename));
   return it != m_fileMap.end() ? it->second : emptyPath;
}

const std::filesystem::path& PUPPlaylist::GetNextPlayFile()
{
   if (!m_randomize) {
      const std::filesystem::path& file = m_files[m_lastIndex];
      if (++m_lastIndex >= (int)m_files.size())
         m_lastIndex = 0;
      return file;
   }
   return m_files[rand() % m_files.size()];
}

std::filesystem::path PUPPlaylist::GetPlayFilePath(const std::filesystem::path& szFilename)
{
   if (m_files.empty())
      return emptyPath;

   if (!szFilename.empty()) {
      ankerl::unordered_dense::map<std::filesystem::path, std::filesystem::path>::const_iterator it = m_fileMap.find(lowerCase(szFilename));
      if (it != m_fileMap.end())
         return m_szBasePath / it->second;
      else
         return emptyPath;
   }
   else
      return m_szBasePath / GetNextPlayFile();
}

string PUPPlaylist::ToString() const {
   return "folder=" + m_szFolder +
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
