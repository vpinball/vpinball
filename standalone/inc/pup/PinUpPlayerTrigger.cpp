#include "stdafx.h"
#include "PinUpPlayerTrigger.h"
#include "PinUpPlayerUtils.h"

PinUpPlayerTrigger::PinUpPlayerTrigger()
{
}

PinUpPlayerTrigger::~PinUpPlayerTrigger()
{
}

static PinUpPlayerTrigger::PlayAction fromString(string name) {
   std::transform(name.begin(), name.end(), name.begin(), ::tolower);

   if (name == "normal") return PinUpPlayerTrigger::PlayAction::Normal;
   if (name == "loop") return PinUpPlayerTrigger::PlayAction::Loop;
   if (name == "splashreset") return PinUpPlayerTrigger::PlayAction::SplashReset;
   if (name == "splashreturn") return PinUpPlayerTrigger::PlayAction::SplashReturn;
   if (name == "stopplayer") return PinUpPlayerTrigger::PlayAction::StopPlayer;
   if (name == "stopfile") return PinUpPlayerTrigger::PlayAction::StopFile;
   if (name == "setbg") return PinUpPlayerTrigger::PlayAction::SetBG;
   if (name == "​playssf") return PinUpPlayerTrigger::PlayAction::PlaySSF;
   if (name == "skipsamepri") return PinUpPlayerTrigger::PlayAction::SkipSamePri;

   PLOGW << "Unknown Play Action " << name;
   return PinUpPlayerTrigger::PlayAction::Normal;
}

PinUpPlayerTrigger* PinUpPlayerTrigger::CreateFromCSVLine(string line)
{
   vector<string> parts = PinUpPlayerUtils::ParseCSVLine(line);
   if (parts.size() != 14)
      return NULL;

   PinUpPlayerTrigger* trigger = new PinUpPlayerTrigger();

   trigger->m_id = string_to_int(parts[0], 0);
   trigger->m_active = string_to_int(parts[1], 0);
   trigger->m_descript = parts[2];
   trigger->m_trigger = parts[3];
   trigger->m_screenNum = string_to_int(parts[4], 0);
   trigger->m_playList = parts[5];
   trigger->m_playFile = parts[6];
   trigger->m_volume = string_to_int(parts[7], 0);
   trigger->m_priority = string_to_int(parts[8], 0);
   trigger->m_length = string_to_int(parts[9], 0);
   trigger->m_counter = string_to_int(parts[10], 0);
   trigger->m_restSeconds = string_to_int(parts[11], 0);
   trigger->m_loop = fromString(parts[12]);
   trigger->m_defaults = string_to_int(parts[13], 0);

   return trigger;
}