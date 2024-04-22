#include "core/stdafx.h"

#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "PUPTrigger.h"

/*
   screens.pup: ScreenNum,ScreenDes,PlayList,PlayFile,Loopit,Active,Priority,CustomPos
   PuP Pack Editor: Mode,ScreenNum,ScreenDes,Background Playlist,Background Filename,Transparent,CustomPos,Volume %

   mappings:

     ScreenNum = ScreenNum
     ScreenDes = ScreenDes
     PlayList = Background Playlist
     PlayFile = Background Filename
     Loopit = Transparent
     Active = Mode
     Priority = Volume %
     CustomPos = CustomPos
*/

PUPScreen::PUPScreen()
{
   m_pCustomPos = nullptr;
}

PUPScreen::~PUPScreen()
{
   delete m_pCustomPos;
}

PUPScreen* PUPScreen::CreateFromCSVLine(string line)
{
   vector<string> parts = parse_csv_line(line);
   if (parts.size() != 8)
      return nullptr;

   PUPScreen* pScreen = new PUPScreen();

   string mode = parts[5];
   if (string_compare_case_insensitive(mode, "Show"))
      pScreen->m_mode = PUP_SCREEN_MODE_SHOW;
   else if (string_compare_case_insensitive(mode, "ForceON"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_ON;
   else if (string_compare_case_insensitive(mode, "ForcePoP"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_POP;
   else if (string_compare_case_insensitive(mode, "ForceBack"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_BACK;
   else if (string_compare_case_insensitive(mode, "ForcePopBack"))
      pScreen->m_mode = PUP_SCREEN_MODE_FORCE_POP_BACK;
   else
      pScreen->m_mode = PUP_SCREEN_MODE_OFF;

   pScreen->m_screenNum = string_to_int(parts[0], 0);
   pScreen->m_screenDes = parts[1];
   pScreen->m_backgroundPlaylist = parts[2];
   pScreen->m_backgroundFilename = parts[3];
   pScreen->m_transparent = (parts[4] == "1");
   pScreen->m_volume = string_to_int(parts[6], 0);
   pScreen->m_pCustomPos = PUPCustomPos::CreateFromCSVLine(parts[7]);

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
   return "mode=" + std::to_string(m_mode) +
      ", screenNum=" + std::to_string(m_screenNum) +
      ", screenDes=" + m_screenDes +
      ", backgroundPlaylist=" + m_backgroundPlaylist +
      ", backgroundFilename=" + m_backgroundFilename +
      ", transparent=" + (m_transparent ? "true" : "false") +
      ", volume=" + std::to_string(m_volume) +
      ", m_customPos={" + (m_pCustomPos ? m_pCustomPos->ToString() : "") + "}";
}
