#include "core/stdafx.h"

#include "PUPPlaylist.h"

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

PUPPlaylist::PUPPlaylist()
{
   m_lastIndex = 0;
}

PUPPlaylist::~PUPPlaylist()
{
   m_files.clear();
}

PUPPlaylist* PUPPlaylist::CreateFromCSV(const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 7)
      return nullptr;

   string szFolder = parts[1];
   string szFolderPath = find_directory_case_insensitive(PUPManager::GetInstance()->GetPath(), szFolder);

   if (szFolderPath.empty()) {
      PLOGW.printf("Playlist folder not found: %s", szFolder.c_str());
      return nullptr;
   }

   PUPPlaylist* pPlaylist = new PUPPlaylist();

   pPlaylist->m_szFolder = szFolder;
   pPlaylist->m_szDescription = parts[2];
   pPlaylist->m_randomize = (string_to_int(parts[3], 0) == 0);
   pPlaylist->m_restSeconds = string_to_int(parts[4], 0);
   pPlaylist->m_volume = string_to_int(parts[5], 0);
   pPlaylist->m_priority = string_to_int(parts[6], 0);

   if (string_compare_case_insensitive(pPlaylist->m_szFolder, "PUPOverlays"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_OVERLAYS;
   else if (string_compare_case_insensitive(pPlaylist->m_szFolder, "PUPFrames"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_FRAMES;
   else if (string_compare_case_insensitive(pPlaylist->m_szFolder, "PUPAlphas"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_ALPHAS;
   else if (string_compare_case_insensitive(pPlaylist->m_szFolder, "PuPShapes"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_SHAPES;
   else
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_DEFAULT;

   pPlaylist->m_szFolderPath = szFolderPath;
   for (const auto& entry : std::filesystem::directory_iterator(pPlaylist->m_szFolderPath)) {
      if (entry.is_regular_file()) {
         string szFilename = entry.path().filename();
         if (!szFilename.empty() && szFilename[0] != '.')
            pPlaylist->m_files.push_back(szFilename);
      }
   }
   std::sort(pPlaylist->m_files.begin(), pPlaylist->m_files.end());

   return pPlaylist;
}

string PUPPlaylist::GetPlayFile(const string& szPlayFile)
{
   static const std::string szEmptyString = "";

   if (!szPlayFile.empty())
      return find_path_case_insensitive(m_szFolderPath + szPlayFile);

   if (m_files.empty())
      return szEmptyString;

   if (!m_randomize) {
      std::lock_guard<std::mutex> lock(m_mutex);
      if (++m_lastIndex >= m_files.size())
         m_lastIndex = 0;
      return m_szFolderPath + m_files[m_lastIndex];
   }

   return m_szFolderPath + m_files[rand() % m_files.size()];
}

string PUPPlaylist::ToString() const {
   return "folder=" + m_szFolder +
      ", description=" + m_szDescription +
      ", randomize=" + (m_randomize ? "true" : "false") +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", function=" + string(PUP_PLAYLIST_FUNCTION_TO_STRING(m_function)) +
      ", folderPath=" + m_szFolderPath;
}
