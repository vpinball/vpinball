#include "core/stdafx.h"
#include "PUPPlaylist.h"
#include "PUPUtils.h"

PUPPlaylist::PUPPlaylist()
{
    m_lastIndex = 0;
}

PUPPlaylist::~PUPPlaylist()
{
}

PUPPlaylist* PUPPlaylist::CreateFromCSVLine(const string& szBasePath, const string& line)
{
   vector<string> parts = PUPUtils::ParseCSVLine(line);
   if (parts.size() != 7)
      return nullptr;

   PUPPlaylist* pPlaylist = new PUPPlaylist();

   pPlaylist->m_folder = parts[1];
   pPlaylist->m_des = parts[2];
   pPlaylist->m_alphaSort = string_to_int(parts[3], 0);
   pPlaylist->m_restSeconds = string_to_int(parts[4], 0);
   pPlaylist->m_volume = string_to_int(parts[5], 0);
   pPlaylist->m_priority = string_to_int(parts[6], 0);

   vector<string> files;

   try {
      string szPath = szBasePath + pPlaylist->m_folder + PATH_SEPARATOR_CHAR;

      for (const auto& entry : std::filesystem::directory_iterator(szPath)) {
         if (entry.is_regular_file()) {
            files.push_back(entry.path().filename().string());
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

   if (m_alphaSort) {
      if (++m_lastIndex >= m_files.size())
         m_lastIndex = 0;

      return m_files[m_lastIndex];
   }

   return m_files[rand() % m_files.size()];
}

string PUPPlaylist::ToString() const {
   return "folder=" + m_folder +
      ", des=" + m_des +
      ", alphaSort=" + std::to_string(m_alphaSort) +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority);
}
