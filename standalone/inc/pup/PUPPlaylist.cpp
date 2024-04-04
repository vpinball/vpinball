#include "stdafx.h"

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

PUPPlaylist* PUPPlaylist::CreateFromCSVLine(const string& szBasePath, const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 7)
      return nullptr;

   PUPPlaylist* pPlaylist = new PUPPlaylist();

   pPlaylist->m_folder = parts[1];
   pPlaylist->m_description = parts[2];
   pPlaylist->m_randomize = (string_to_int(parts[3], 0) == 0);
   pPlaylist->m_restSeconds = string_to_int(parts[4], 0);
   pPlaylist->m_volume = string_to_int(parts[5], 0);
   pPlaylist->m_priority = string_to_int(parts[6], 0);

   if (string_compare_case_insensitive(pPlaylist->m_folder, "PUPOverlays"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_OVERLAYS;
   else if (string_compare_case_insensitive(pPlaylist->m_folder, "PUPFrames"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_FRAMES;
   else if (string_compare_case_insensitive(pPlaylist->m_folder, "PUPAlphas"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_ALPHAS;
   else if (string_compare_case_insensitive(pPlaylist->m_folder, "PuPShapes"))
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_SHAPES;
   else
      pPlaylist->m_function = PUP_PLAYLIST_FUNCTION_DEFAULT;

   vector<string> files;

   try {
      string szPath = szBasePath + pPlaylist->m_folder + PATH_SEPARATOR_CHAR;

      for (const auto& entry : std::filesystem::directory_iterator(szPath)) {
         if (entry.is_regular_file()) {
            string szFilename = entry.path().filename();
            if (!szFilename.empty() && szFilename[0] != '.')
               files.push_back(szFilename);
         }
      }
   }

   catch(...) {
   }

   std::sort(files.begin(), files.end());
   pPlaylist->m_files = files;

   return pPlaylist;
}

const string& PUPPlaylist::GetPlayFile()
{
   static const std::string szEmptyString;

   if (m_files.empty())
      return szEmptyString;

   if (!m_randomize) {
      if (++m_lastIndex >= m_files.size())
         m_lastIndex = 0;

      return m_files[m_lastIndex];
   }

   return m_files[rand() % m_files.size()];
}

string PUPPlaylist::ToString() const {
   return "folder=" + m_folder +
      ", description=" + m_description +
      ", randomize=" + (m_randomize ? "true" : "false") +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", function=" + std::to_string(m_function);
}
