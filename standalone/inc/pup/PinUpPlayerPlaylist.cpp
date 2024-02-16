#include "stdafx.h"
#include "PinUpPlayerPlaylist.h"
#include "PinUpPlayerUtils.h"

PinUpPlayerPlaylist::PinUpPlayerPlaylist()
{
}

PinUpPlayerPlaylist::~PinUpPlayerPlaylist()
{
}

PinUpPlayerPlaylist* PinUpPlayerPlaylist::CreateFromCSVLine(string line)
{
   vector<string> parts = PinUpPlayerUtils::ParseCSVLine(line);
   if (parts.size() != 7)
      return NULL;

   PinUpPlayerPlaylist* playlist = new PinUpPlayerPlaylist();

   playlist->m_screenNum = string_to_int(parts[0], 0);
   playlist->m_folder = parts[1];
   playlist->m_des = parts[2];
   playlist->m_alphaSort = string_to_int(parts[3], 0);
   playlist->m_restSeconds = string_to_int(parts[4], 0);
   playlist->m_volume = string_to_int(parts[5], 0);
   playlist->m_priority = string_to_int(parts[6], 0);

   return playlist;
}