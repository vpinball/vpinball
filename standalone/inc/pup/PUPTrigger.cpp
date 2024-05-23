#include "core/stdafx.h"

#include "PUPTrigger.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"

/*
   triggers.pup: ID,Active,Descript,Trigger,ScreenNum,PlayList,PlayFile,Volume,Priority,Length,Counter,RestSeconds,Loop,Defaults
   PuP Pack Editor: Descript,Trigger,Screen,Playlist,PlayFile,Volume,RestSeconds,Priority,Counter,Length(s),PlayAction,Active

   mappings:

     ID = ?
     Active = Active
     Descript = Descript
     Trigger = Trigger
     ScreenNum = Screen
     PlayList = Playlist
     PlayFile = PlayFile
     Volume = Volume
     Priority = Priority
     Length = Length(s)
     Counter = Counter
     RestSeconds = RestSeconds
     Loop = PlayAction
     Defaults = ?

   trigger names:

     S = Solenoid
     W = Switches
     L = Lights
     G = GIs
     E = DOFs
     M = Mechs
     D = PupCap DMD Match
*/

PUPTrigger::PUPTrigger()
{
   m_playAction = PUP_TRIGGER_PLAY_ACTION_NORMAL;
}

PUPTrigger* PUPTrigger::CreateFromCSV(const string& line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 14)
      return nullptr;

   bool active = (string_to_int(parts[1], 0) == 1);
   if (!active)
      return nullptr;

   PUPScreen* pScreen = PUPManager::GetInstance()->GetScreen(string_to_int(parts[4], -1));
   if (!pScreen)
      return nullptr;

   PUPPlaylist* pPlaylist = PUPManager::GetInstance()->GetPlaylist(parts[5]);
   if (!pPlaylist)
      return nullptr;

   PUPTrigger* pTrigger = new PUPTrigger();

   pTrigger->m_active = active;
   pTrigger->m_descript = parts[2];
   pTrigger->m_trigger = parts[3];
   pTrigger->m_pScreen = pScreen;
   pTrigger->m_pPlaylist = pPlaylist;
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

   return pTrigger;
}

string PUPTrigger::ToString() const {
   return "active="s + ((m_active == true) ? "true" : "false") +
      ", descript=" + m_descript +
      ", trigger=" + m_trigger +
      ", screen={" + m_pScreen->ToString(false) + '}' +
      ", playlist={" + m_pPlaylist->ToString() + '}' +
      ", playFile=" + m_playFile +
      ", volume=" + std::to_string(m_volume) +
      ", priority=" + std::to_string(m_priority) +
      ", length=" + std::to_string(m_length) +
      ", count=" + std::to_string(m_counter) +
      ", restSeconds=" + std::to_string(m_restSeconds) +
      ", playAction=" + string(PUP_TRIGGER_PLAY_ACTION_TO_STRING(m_playAction));
}
