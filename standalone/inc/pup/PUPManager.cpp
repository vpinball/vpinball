#include "stdafx.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"
#include "PUPTrigger.h"
#include "PUPCustomPos.h"
#include "PUPWindow.h"

PUPManager* PUPManager::m_pInstance = NULL;

PUPManager* PUPManager::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new PUPManager();

   return m_pInstance;
}

PUPManager::PUPManager()
{
   m_init = false;
   m_pBackglassScreen = nullptr;
   m_pBackglassWindow = nullptr;
   m_pTopperScreen = nullptr;
   m_pTopperWindow = nullptr;
   m_pThread = nullptr;
   m_running = false;
}

PUPManager::~PUPManager()
{
}

bool PUPManager::LoadConfig(const string& szRomName)
{
   if (m_init) {
      PLOGW.printf("PUP already initialized");
      return false;
   }

   TTF_Init();

   string szPath = g_pvp->m_currentTablePath + "pupvideos" +
      PATH_SEPARATOR_CHAR + szRomName + PATH_SEPARATOR_CHAR;

   if (!std::filesystem::is_directory(szPath))
      return false;

   // Load screens

   string szScreensPath = szPath + "screens.pup";
   std::ifstream screensFile;
   screensFile.open(szScreensPath, std::ifstream::in);
   if (screensFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(screensFile, line)) {
         if (++i == 1)
            continue;

         PUPScreen* pScreen = PUPScreen::CreateFromCSVLine(this, line);
         if (pScreen)
            m_screenMap[pScreen->GetScreenNum()] = pScreen;
      }
      PLOGI.printf("Screens loaded: file=%s, size=%d", szScreensPath.c_str(), m_screenMap.size());
   }
   else {
      PLOGW.printf("Unable to load %s", szScreensPath.c_str());

      return false;
   }

   // Load Playlists

   string szPlaylistsPath = szPath + "playlists.pup";
   std::ifstream playlistsFile;
   playlistsFile.open(szPlaylistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;

         PUPPlaylist* pPlaylist = PUPPlaylist::CreateFromCSVLine(szPath, line);
         if (pPlaylist)
            m_playlistMap[pPlaylist->GetFolder()] = pPlaylist;
      }
      PLOGI.printf("Playlists loaded: file=%s, size=%d", szPlaylistsPath.c_str(), m_playlistMap.size());
   }
   else {
      PLOGW.printf("Unable to load %s", szPlaylistsPath.c_str());

      return false;
   }

   // Load Triggers

   string szTriggersPath = szPath + "triggers.pup";
   std::ifstream triggersFile;
   triggersFile.open(szTriggersPath, std::ifstream::in);
   if (triggersFile.is_open()) {
      int triggers = 0;
      string line;
      int i = 0;
      while (std::getline(triggersFile, line)) {
         if (++i == 1)
            continue;

         PUPTrigger* pTrigger = PUPTrigger::CreateFromCSVLine(line);
         if (pTrigger) {
            PUPScreen* pScreen = GetScreen(pTrigger->GetScreenNum());
            if (pScreen) {
                pScreen->AddTrigger(pTrigger);
                triggers++;
            }
            else {
               PLOGW.printf("Screen not found: %d", pTrigger->GetScreenNum());
               delete pTrigger;
            }
         }
      }
      PLOGI.printf("Triggers loaded: file=%s, size=%d", szTriggersPath.c_str(), triggers);
   }
   else {
      PLOGW.printf("Unable to load %s", szTriggersPath.c_str());

      return false;
   }

   // Load Fonts

   string szFontsPath = szPath + "FONTS" + PATH_SEPARATOR_CHAR;
   if (std::filesystem::is_directory(szFontsPath)) {
      for (const auto& entry : std::filesystem::directory_iterator(szFontsPath)) {
         if (entry.is_regular_file()) {
            string szFontPath = entry.path().string();
            if (extension_from_path(szFontPath) == "ttf") {
               TTF_Font* pFont = TTF_OpenFont(szFontPath.c_str(), 8);
               if (pFont) {
                  string szFamilyName = std::string(TTF_FontFaceFamilyName(pFont));
                  m_fontMap[szFamilyName] = pFont;

                  PLOGI.printf("Font loaded: file=%s, family=%s", szFontPath.c_str(), szFamilyName.c_str());
               }
               else {
                  PLOGW.printf("Error loading font: file=%s", szFontPath.c_str());
               }
            }
         }
      }
   }
   else {
      PLOGI.printf("No PUP FONTS directory");
   }

   // Determine child screens

   for (auto& [key, pScreen] : m_screenMap) {
      PUPCustomPos* pCustomPos = pScreen->GetCustomPos();
      if (pCustomPos) {
         PUPScreen* pParentScreen = GetScreen(pCustomPos->GetSourceScreen());
         if (pParentScreen) {
            pParentScreen->AddChild(pScreen);
         }
      }
   }

   m_szPath = szPath;
   m_init = true;

   return true;
}

void PUPManager::Start()
{
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindows"s, true)) {
      PLOGI.printf("PUP windows disabled");
      return;
   }

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindow"s, true)) {
      m_pBackglassScreen = GetScreen(pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowScreen"s, 2));
      if (m_pBackglassScreen) {
         m_screenMap.erase(m_pBackglassScreen->GetScreenNum());
         m_pBackglassWindow = new PUPWindow(m_pBackglassScreen, "PUPBackglass",
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowX"s, PUP_SETTINGS_BACKGLASSX),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowY"s, PUP_SETTINGS_BACKGLASSY),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowWidth"s, PUP_SETTINGS_BACKGLASSWIDTH),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowHeight"s, PUP_SETTINGS_BACKGLASSHEIGHT),
            PUP_ZORDER_BACKGLASS,
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowRotation"s, 0));
      }
      else {
         PLOGW.printf("PUP Backglass screen not found");
      }
   }
   else {
      PLOGI.printf("PUP Backglass window disabled");
      return;
   }

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindow"s, true)) {
      m_pTopperScreen = GetScreen(pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowScreen"s, 0));
      if (m_pTopperScreen) {
         m_screenMap.erase(m_pTopperScreen->GetScreenNum());
         m_pTopperWindow = new PUPWindow(m_pTopperScreen, "PUPTopper",
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowX"s, PUP_SETTINGS_TOPPERX),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowY"s, PUP_SETTINGS_TOPPERY),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowWidth"s, PUP_SETTINGS_TOPPERWIDTH),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowHeight"s, PUP_SETTINGS_TOPPERHEIGHT),
            PUP_ZORDER_TOPPER,
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowRotation"s, 0));
      }
      else {
         PLOGW.printf("PUP topper screen not found");
      }
   }
   else {
      PLOGI.printf("PUP Topper window disabled");
      return;
   }

   if (m_pBackglassWindow || m_pTopperWindow)
      RunLoop();
}

void PUPManager::Stop()
{
   m_running = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
      m_pThread = NULL;
   }

   delete m_pBackglassWindow;
   m_pBackglassWindow = nullptr;
   delete m_pBackglassScreen;
   m_pBackglassScreen = nullptr;

   delete m_pTopperWindow;
   m_pTopperWindow = nullptr;
   delete m_pTopperScreen;
   m_pTopperScreen = nullptr;

   for (auto& [key, pScreen] : m_screenMap)
      delete pScreen;

   m_screenMap.clear();

   for (auto& [key, pPlaylist] : m_playlistMap)
      delete pPlaylist;

   m_playlistMap.clear();

   for (auto& [key, pFont] : m_fontMap)
      TTF_CloseFont(pFont);

   m_fontMap.clear();

   m_szPath.clear();
   m_init = false;

   TTF_Quit();
}

void PUPManager::RunLoop()
{
   m_running = true;

   m_pThread = new std::thread([this]() {
      while (m_running) {
         while (!m_triggerQueue.empty()) {
            std::unique_lock<std::mutex> lock(m_triggerMutex);
            auto item = m_triggerQueue.front();
            m_triggerQueue.pop();
            lock.unlock();

            if (item.value) {
               string szTrigger = item.type + std::to_string(item.number);

               if (m_pBackglassScreen)
                  m_pBackglassScreen->Trigger(szTrigger);

               if (m_pTopperScreen)
                  m_pTopperScreen->Trigger(szTrigger);
            }
         }
         SDL_Delay(10);
      }
   });
}

void PUPManager::QueueTriggerData(PUPTriggerData data)
{
   std::lock_guard<std::mutex> guard(m_triggerMutex);
   m_triggerQueue.push({ data.type, data.number, data.value });
}

PUPScreen* PUPManager::GetScreen(int screenNum)
{
   std::map<int, PUPScreen*>::iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end() ? it->second : nullptr;
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

TTF_Font* PUPManager::GetFont(const string& szFamilyName)
{
   std::map<string, TTF_Font*>::iterator it = m_fontMap.find(szFamilyName);
   return it != m_fontMap.end() ? it->second : nullptr;
}