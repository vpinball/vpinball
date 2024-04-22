#include "core/stdafx.h"

#include "PUPScreen.h"
#include "PUPTrigger.h"

PUPScreen::PUPScreen()
{
}

PUPScreen::~PUPScreen()
{
}

PUPScreen* PUPScreen::CreateFromCSVLine(string line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 8)
      return nullptr;

   string active = parts[5];
   if (string_compare_case_insensitive(active, "off"))
      return nullptr;

   PUPScreen* pScreen = new PUPScreen();

   pScreen->m_screenNum = string_to_int(parts[0], 0);
   pScreen->m_screenDes = parts[1];
   pScreen->m_playList = parts[2];
   pScreen->m_playFile = parts[3];
   pScreen->m_loopit = (parts[4] == "1");

   if (string_compare_case_insensitive(active, "Show"))
      pScreen->m_active = PUP_SCREEN_ACTIVE_SHOW;
   else if (string_compare_case_insensitive(active, "ForceON"))
      pScreen->m_active = PUP_SCREEN_ACTIVE_FORCE_ON;
   else if (string_compare_case_insensitive(active, "ForcePoP"))
      pScreen->m_active = PUP_SCREEN_ACTIVE_FORCE_ON;
   else if (string_compare_case_insensitive(active, "ForceBack"))
      pScreen->m_active = PUP_SCREEN_ACTIVE_FORCE_ON;
   else if (string_compare_case_insensitive(active, "ForcePopBack"))
      pScreen->m_active = PUP_SCREEN_ACTIVE_FORCE_ON;

   pScreen->m_priority = string_to_int(parts[6], 0);
   pScreen->m_customPos = parts[7];

   return pScreen;
}

PUPTrigger* PUPScreen::GetTrigger(const string& szTrigger)
{
   std::map<string, PUPTrigger*>::iterator it = m_triggerMap.find(szTrigger);
   return it != m_triggerMap.end() ? it->second : nullptr;
}

void PUPScreen::SetTrigger(PUPTrigger* pTrigger)
{
   if (GetTrigger(pTrigger->GetTrigger())) {
      PLOGW.printf("Duplicate trigger: %s", pTrigger->GetTrigger().c_str());
      return;
   }
   m_triggerMap[pTrigger->GetTrigger()] = pTrigger;
}

string PUPScreen::ToString() const
{
   return "screenNum=" + std::to_string(m_screenNum) +
      ", screenDes=" + m_screenDes +
      ", playlist=" + m_playList +
      ", playfile=" + m_playFile +
      ", loopit=" + (m_loopit ? "true" : "false") +
      ", active=" + std::to_string(m_active) +
      ", priority=" + std::to_string(m_priority) +
      ", m_customPos=" + m_customPos;
}
