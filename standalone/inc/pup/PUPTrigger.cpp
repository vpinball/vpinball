#include "core/stdafx.h"
#include "PUPTrigger.h"

PUPTrigger::PUPTrigger()
{
   m_playAction = PUP_TRIGGER_PLAY_ACTION_NORMAL;
}

PUPTrigger::~PUPTrigger()
{
}

PUPTrigger* PUPTrigger::CreateFromCSVLine(string line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 14)
      return nullptr;

   bool active = (string_to_int(parts[1], 0) == 1);

   if (!active)
      return nullptr;

   PUPTrigger* pTrigger = new PUPTrigger();

   pTrigger->m_id = string_to_int(parts[0], 0);
   pTrigger->m_active = true;
   pTrigger->m_descript = parts[2];
   pTrigger->m_trigger = parts[3];
   pTrigger->m_screen = string_to_int(parts[4], 0);
   pTrigger->m_playlist = parts[5];
   pTrigger->m_playFile = parts[6];
   pTrigger->m_volume = string_to_int(parts[7], 0);
   pTrigger->m_priority = string_to_int(parts[8], 0);
   pTrigger->m_length = string_to_int(parts[9], 0);
   pTrigger->m_counter = string_to_int(parts[10], 0);
   pTrigger->m_restSeconds = string_to_int(parts[11], 0);

   string playAction = parts[12];

   if (string_compare_case_insensitive(playAction, "Loop"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_LOOP;
   else if (string_compare_case_insensitive(playAction, "SplashReset"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_SPLASH_RESET;
   else if (string_compare_case_insensitive(playAction, "SplashReturn"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_SPLASH_RESET;
   else if (string_compare_case_insensitive(playAction, "StopPlayer"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_STOP_PLAYER;
   else if (string_compare_case_insensitive(playAction, "StopFile"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_STOP_FILE;
   else if (string_compare_case_insensitive(playAction, "SetBG"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_SET_BG;
   else if (string_compare_case_insensitive(playAction, "PlaySSF"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_PLAY_SSF;
   else if (string_compare_case_insensitive(playAction, "SkipSamePrty"))
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_SKIP_SAME_PRTY;
   else
      pTrigger->m_playAction = PUP_TRIGGER_PLAY_ACTION_NORMAL;

   pTrigger->m_defaults = string_to_int(parts[13], 0);

   return pTrigger;
}

string PUPTrigger::ToString() const {
   return "id=" + std::to_string(m_id) +
      ", active=" + ((m_active) ? "true" : "false") +
      ", descript=" + m_descript +
      ", trigger=" + m_trigger +
      ", screen=" + std::to_string(m_screen) +
      ", playlist=" + m_playlist +
      ", playFile=" + m_playFile +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", length=" + std::to_string(m_length) +
      ", count=" + std::to_string(m_counter) +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", playAction=" + std::to_string(m_playAction) +
      ", defaults=" + std::to_string(m_defaults);
}
