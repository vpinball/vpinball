#include "core/stdafx.h"
#include "PUPPlaylist.h"
#include "PUPUtils.h"

PUPPlaylist::PUPPlaylist()
{
}

PUPPlaylist::~PUPPlaylist()
{
}

PUPPlaylist* PUPPlaylist::CreateFromCSVLine(string line)
{
   vector<string> parts = PUPUtils::ParseCSVLine(line);
   if (parts.size() != 7)
      return nullptr;

   PUPPlaylist* playlist = new PUPPlaylist();

   playlist->m_screenNum = string_to_int(parts[0], 0);
   playlist->m_folder = parts[1];
   playlist->m_des = parts[2];
   playlist->m_alphaSort = string_to_int(parts[3], 0);
   playlist->m_restSeconds = string_to_int(parts[4], 0);
   playlist->m_volume = string_to_int(parts[5], 0);
   playlist->m_priority = string_to_int(parts[6], 0);

   return playlist;
}