#include "core/stdafx.h"
#include "PUPTrigger.h"
#include "PUPUtils.h"

PUPTrigger::PUPTrigger()
{
}

PUPTrigger::~PUPTrigger()
{
}

PUPTrigger* PUPTrigger::CreateFromCSVLine(string line)
{
   vector<string> parts = PUPUtils::ParseCSVLine(line);
   if (parts.size() != 14)
      return nullptr;

   PUPTrigger* trigger = new PUPTrigger();

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
   trigger->m_loop = parts[12];
   trigger->m_defaults = string_to_int(parts[13], 0);

   return trigger;
}