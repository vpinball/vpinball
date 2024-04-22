#include "core/stdafx.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"
#include "PUPTrigger.h"

bool PUPManager::LoadConfig(const string& szRomName)
{
   m_szPath = g_pvp->m_currentTablePath + "pupvideos" +
      PATH_SEPARATOR_CHAR + szRomName + PATH_SEPARATOR_CHAR;

   // Load screens

   string screensPath = m_szPath + "screens.pup";
   std::ifstream screensFile;
   screensFile.open(screensPath, std::ifstream::in);
   if (screensFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(screensFile, line)) {
         if (++i == 1)
            continue;

         PUPScreen* pScreen = PUPScreen::CreateFromCSVLine(line);

         if (pScreen) {
            m_screenMap[pScreen->GetScreenNum()] = pScreen;
            m_screenDesMap[pScreen->GetScreenDes()] = pScreen;
         }
      }
      PLOGI.printf("Screens loaded: file=%s, size=%d", screensPath.c_str(), m_screenMap.size());
   }
   else {
      PLOGE.printf("Unable to load %s", screensPath.c_str());

      return false;
   }

   // Load Playlists

   string playlistsPath = m_szPath + "playlists.pup";
   std::ifstream playlistsFile;
   playlistsFile.open(playlistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;

         PUPPlaylist* pPlaylist = PUPPlaylist::CreateFromCSVLine(m_szPath, line);

         if (pPlaylist)
            m_playlistMap[pPlaylist->GetFolder()] = pPlaylist;
      }
      PLOGI.printf("Playlists loaded: file=%s, size=%d", playlistsPath.c_str(), m_playlistMap.size());
   }
   else {
      PLOGE.printf("Unable to load %s", playlistsPath.c_str());

      return false;
   }

   // Load Triggers

   string triggersPath = m_szPath + "triggers.pup";
   std::ifstream triggersFile;
   triggersFile.open(triggersPath, std::ifstream::in);
   if (triggersFile.is_open()) {
      int triggers = 0;

      string line;
      int i = 0;
      while (std::getline(triggersFile, line)) {
         if (++i == 1)
            continue;

         PUPTrigger* pTrigger = PUPTrigger::CreateFromCSVLine(line);

         if (pTrigger) {
            PUPScreen* pScreen = GetScreen(pTrigger->GetScreen());
            if (pScreen) {
                pScreen->SetTrigger(pTrigger);
                triggers++;
            }
            else {
               PLOGW.printf("Screen not found: %d", pTrigger->GetScreen());
               delete pTrigger;
            }
         }
      }
      PLOGI.printf("Triggers loaded: file=%s, size=%d", triggersPath.c_str(), triggers);
   }
   else {
      PLOGE.printf("Unable to load %s", triggersPath.c_str());

      return false;
   }

   return true;
}

PUPScreen* PUPManager::GetScreen(int screenNum)
{
   std::map<int, PUPScreen*>::iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end() ? it->second : nullptr;
}

PUPScreen* PUPManager::GetScreen(const string& szScreen)
{
   std::map<string, PUPScreen*>::iterator it = m_screenDesMap.find(szScreen);
   return it != m_screenDesMap.end() ? it->second : nullptr;
}

PUPPlaylist* PUPManager::GetPlaylist(const string& szFolder)
{
   std::map<string, PUPPlaylist*>::iterator it = m_playlistMap.find(szFolder);
   return it != m_playlistMap.end() ? it->second : nullptr;
}

string PUPManager::GetPath(PUPPlaylist* pPlaylist, const string& szPlayFile)
{
   if (!szPlayFile.empty())
      return m_szPath + pPlaylist->GetFolder() + PATH_SEPARATOR_CHAR + szPlayFile;

   return m_szPath + pPlaylist->GetFolder() + PATH_SEPARATOR_CHAR + pPlaylist->GetPlayFile();
}

PUPScreen* PUPManager::GetBackglass()
{
   return GetScreen("Backglass");
}