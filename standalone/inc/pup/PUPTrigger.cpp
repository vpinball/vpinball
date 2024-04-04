#include "core/stdafx.h"
#include "PUPTrigger.h"
#include "PUPUtils.h"

PUPTrigger::PUPTrigger()
{
   m_pScreen = nullptr;
   m_pPlaylist = nullptr;
}

PUPTrigger::~PUPTrigger()
{
}

PUPTrigger* PUPTrigger::CreateFromCSVLine(string line, vector<PUPScreen*>& screens, vector<PUPPlaylist*>& playlists)
{
   vector<string> parts = PUPUtils::ParseCSVLine(line);
   if (parts.size() != 14)
      return nullptr;

   PUPTrigger* trigger = new PUPTrigger();

   trigger->m_id = string_to_int(parts[0], 0);
   trigger->m_active = (string_to_int(parts[1], 0) == 1);
   trigger->m_descript = parts[2];
   trigger->m_trigger = parts[3];

   int screenNum = string_to_int(parts[4], 0);
   for (PUPScreen* pScreen : screens) {
      if (pScreen->GetScreenNum() == screenNum) {
         trigger->m_pScreen = pScreen;
         break;
      }
   }

   string playlist = parts[5];
   for (PUPPlaylist* pPlaylist : playlists) {
      if (pPlaylist->GetFolder() == playlist) {
         trigger->m_pPlaylist = pPlaylist;
         break;
      }
   }

   trigger->m_playFile = parts[6];
   trigger->m_volume = string_to_int(parts[7], 0);
   trigger->m_priority = string_to_int(parts[8], 0);
   trigger->m_length = string_to_int(parts[9], 0);
   trigger->m_counter = string_to_int(parts[10], 0);
   trigger->m_restSeconds = string_to_int(parts[11], 0);
   trigger->m_loop = parts[12];
   trigger->m_defaults = string_to_int(parts[13], 0);

   return trigger;
}

const string& PUPTrigger::GetPlayFile()
{
   static const std::string szEmptyString;
   return !m_playFile.empty() ? m_playFile : (m_pPlaylist ? m_pPlaylist->GetPlayFile() : szEmptyString);
}

string PUPTrigger::ToString() const {
   return "id=" + std::to_string(m_id) +
      ", active=" + ((m_active) ? "true" : "false") +
      ", descript=" + m_descript +
      ", trigger=" + m_trigger +
      ", screen=" + ((m_pScreen) ? ("{" + m_pScreen->ToString() + "}") : "NULL") +
      ", playlist=" + ((m_pPlaylist) ? ("{" + m_pPlaylist->ToString() + "}") : "NULL") +
      ", playfile=" + m_playFile +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", length=" + std::to_string(m_length) +
      ", count=" + std::to_string(m_counter) +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", loop=" + m_loop +
      ", defaults=" + std::to_string(m_defaults);
}
