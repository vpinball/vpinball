#include "core/stdafx.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPPlaylist.h"
#include "PUPTrigger.h"
#include "PUPCustomPos.h"
#include "PUPWindow.h"

const char* PUP_TRIGGER_PLAY_ACTION_STRINGS[] = {
   "PUP_TRIGGER_PLAY_ACTION_NORMAL",
   "PUP_TRIGGER_PLAY_ACTION_LOOP",
   "PUP_TRIGGER_PLAY_ACTION_SPLASH_RESET",
   "PUP_TRIGGER_PLAY_ACTION_SPLASH_RETURN",
   "PUP_TRIGGER_PLAY_ACTION_STOP_PLAYER",
   "PUP_TRIGGER_PLAY_ACTION_STOP_FILE",
   "PUP_TRIGGER_PLAY_ACTION_SET_BG",
   "PUP_TRIGGER_PLAY_ACTION_PLAY_SSF",
   "PUP_TRIGGER_PLAY_ACTION_SKIP_SAME_PRTY"
};

const char* PUP_PLAYLIST_FUNCTION_STRINGS[] = {
   "PUP_PLAYLIST_FUNCTION_DEFAULT",
   "PUP_PLAYLIST_FUNCTION_OVERLAYS",
   "PUP_PLAYLIST_FUNCTION_FRAMES",
   "PUP_PLAYLIST_FUNCTION_ALPHAS",
   "PUP_PLAYLIST_FUNCTION_SHAPES"
};

const char* PUP_SCREEN_MODE_STRINGS[] = {
   "PUP_SCREEN_MODE_OFF",
   "PUP_SCREEN_MODE_SHOW",
   "PUP_SCREEN_MODE_FORCE_ON",
   "PUP_SCREEN_MODE_FORCE_POP",
   "PUP_SCREEN_MODE_FORCE_BACK",
   "PUP_SCREEN_MODE_FORCE_POP_BACK",
   "PUP_SCREEN_MODE_MUSIC_ONLY"
};

const char* PUP_TRIGGER_PLAY_ACTION_TO_STRING(PUP_TRIGGER_PLAY_ACTION value)
{
    if (value < 0 || value >= sizeof(PUP_TRIGGER_PLAY_ACTION_STRINGS) / sizeof(PUP_TRIGGER_PLAY_ACTION_STRINGS[0]))
        return "UNKNOWN";
    return PUP_TRIGGER_PLAY_ACTION_STRINGS[value];
}

const char* PUP_PLAYLIST_FUNCTION_TO_STRING(PUP_PLAYLIST_FUNCTION value)
{
    if (value < 0 || value >= sizeof(PUP_PLAYLIST_FUNCTION_STRINGS) / sizeof(PUP_PLAYLIST_FUNCTION_STRINGS[0]))
        return "UNKNOWN";
    return PUP_PLAYLIST_FUNCTION_STRINGS[value];
}

const char* PUP_SCREEN_MODE_TO_STRING(PUP_SCREEN_MODE value)
{
    if (value < 0 || value >= sizeof(PUP_SCREEN_MODE_STRINGS) / sizeof(PUP_SCREEN_MODE_STRINGS[0]))
        return "UNKNOWN";
    return PUP_SCREEN_MODE_STRINGS[value];
}

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
   m_isRunning = false;
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

   m_szPath = find_directory_case_insensitive(g_pvp->m_currentTablePath, "pupvideos");
   if (m_szPath.empty())
      return false;

   m_szPath = find_directory_case_insensitive(m_szPath, szRomName);
   if (m_szPath.empty())
      return false;

   PLOGI.printf("PUP path: %s", m_szPath.c_str());

   TTF_Init();

   // Load screens

   string szScreensPath = find_path_case_insensitive(m_szPath + "screens.pup");
   std::ifstream screensFile;
   screensFile.open(szScreensPath, std::ifstream::in);
   if (screensFile.is_open()) {
      int count = 0;
      string line;
      int i = 0;
      while (std::getline(screensFile, line)) {
         if (++i == 1)
            continue;
         if (AddScreen(PUPScreen::CreateFromCSV(line)))
            count++;
      }
      PLOGI.printf("Screens loaded: file=%s, size=%d", szScreensPath.c_str(), count);
   }
   else {
      PLOGE.printf("Unable to load %s", szScreensPath.c_str());
      return false;
   }

   // Load Playlists

   string szPlaylistsPath = find_path_case_insensitive(m_szPath + "playlists.pup");
   std::ifstream playlistsFile;
   playlistsFile.open(szPlaylistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      int count = 0;
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;
         if (AddPlaylist(PUPPlaylist::CreateFromCSV(line)))
            count++;
      }
      PLOGI.printf("Playlists loaded: file=%s, size=%d", szPlaylistsPath.c_str(), count);
   }
   else {
      PLOGE.printf("Unable to load %s", szPlaylistsPath.c_str());
      return false;
   }

   // Load Triggers

   string szTriggersPath = find_path_case_insensitive(m_szPath + "triggers.pup");
   std::ifstream triggersFile;
   triggersFile.open(szTriggersPath, std::ifstream::in);
   if (triggersFile.is_open()) {
      int count = 0;
      string line;
      int i = 0;
      while (std::getline(triggersFile, line)) {
         if (++i == 1)
            continue;
         if (AddTrigger(PUPTrigger::CreateFromCSV(line)))
            count++;
      }
      PLOGI.printf("Triggers loaded: file=%s, size=%d", szTriggersPath.c_str(), count);
   }
   else {
      PLOGE.printf("Unable to load %s", szTriggersPath.c_str());
      return false;
   }

   // Load Fonts

   string szFontsPath = find_directory_case_insensitive(m_szPath, "FONTS");
   if (!szFontsPath.empty()) {
      int count = 0;
      for (const auto& entry : std::filesystem::directory_iterator(szFontsPath)) {
         if (entry.is_regular_file()) {
            string szFontPath = entry.path().string();
            if (extension_from_path(szFontPath) == "ttf") {
               if (AddFont(TTF_OpenFont(szFontPath.c_str(), 8)))
                  count++;
               else {
                  PLOGE.printf("Unable to load %s", szFontPath.c_str());
               }
            }
         }
      }
      PLOGI.printf("Fonts loaded: path=%s, size=%d", szFontsPath.c_str(), count);
   }

   // Determine child screens

   for (auto& [key, pScreen] : m_screenMap) {
      PUPCustomPos* pCustomPos = pScreen->GetCustomPos();
      if (pCustomPos) {
         PUPScreen* pParentScreen = GetScreen(pCustomPos->GetSourceScreen());
         if (pParentScreen && pScreen != pParentScreen)
            pParentScreen->AddChild(pScreen);
      }
   }

   m_init = true;

   return true;
}

bool PUPManager::AddScreen(PUPScreen* pScreen)
{
   if (!pScreen)
      return false;

   if (GetScreen(pScreen->GetScreenNum())) {
      PLOGW.printf("Duplicate screen: screen={%s}", pScreen->ToString(false).c_str());
      delete pScreen;
      return false;
   }

   m_screenMap[pScreen->GetScreenNum()] = pScreen;
   return true;
}

PUPScreen* PUPManager::GetScreen(int screenNum)
{
   std::map<int, PUPScreen*>::iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end() ? it->second : nullptr;
}

bool PUPManager::AddPlaylist(PUPPlaylist* pPlaylist)
{
   if (!pPlaylist)
      return false;

   if (GetPlaylist(pPlaylist->GetFolder())) {
      PLOGW.printf("Duplicate playlist: folder=%s", pPlaylist->GetFolder().c_str());
      delete pPlaylist;
      return false;
   }

   m_playlistMap[string_to_lower(pPlaylist->GetFolder())] = pPlaylist;
   return true;
}

PUPPlaylist* PUPManager::GetPlaylist(const string& szFolder)
{
   std::map<string, PUPPlaylist*>::iterator it = m_playlistMap.find(string_to_lower(szFolder));
   return it != m_playlistMap.end() ? it->second : nullptr;
}

bool PUPManager::AddTrigger(PUPTrigger* pTrigger)
{
   if (!pTrigger)
      return false;

   m_triggerMap[pTrigger->GetTrigger()].push_back(pTrigger);
   return true;
}

vector<PUPTrigger*>* PUPManager::GetTriggers(const string& szTrigger)
{
   std::map<string, vector<PUPTrigger*>>::iterator it = m_triggerMap.find(szTrigger);
   return it != m_triggerMap.end() ? &it->second : nullptr;
}

bool PUPManager::AddFont(TTF_Font* pFont)
{
   if (!pFont)
      return false;

   m_fontMap[string_to_lower(string(TTF_FontFaceFamilyName(pFont)))] = pFont;
   return true;
}

TTF_Font* PUPManager::GetFont(const string& szFamilyName)
{
   std::map<string, TTF_Font*>::iterator it = m_fontMap.find(string_to_lower(szFamilyName));
   return it != m_fontMap.end() ? it->second : nullptr;
}

void PUPManager::QueueTriggerData(PUPTriggerData data)
{
   if (data.value == 0)
      return;

   {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_triggerDataQueue.push({ data.type, data.number, data.value });
   }

   m_queueCondVar.notify_one();
}

void PUPManager::Start()
{
   m_isRunning = true;
   m_thread = std::thread(&PUPManager::ProcessQueue, this);

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindows"s, true)) {
      PLOGI.printf("PUP windows disabled");
      return;
   }

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindow"s, true)) {
      m_pBackglassScreen = GetScreen(pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowScreen"s, 2));
      if (m_pBackglassScreen && m_pBackglassScreen->GetMode() != PUP_SCREEN_MODE_OFF) {
         m_screenMap.erase(m_pBackglassScreen->GetScreenNum());
         m_pBackglassWindow = new PUPWindow(m_pBackglassScreen, "PUPBackglass",
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowX"s, PUP_SETTINGS_BACKGLASSX),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowY"s, PUP_SETTINGS_BACKGLASSY),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowWidth"s, PUP_SETTINGS_BACKGLASSWIDTH),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowHeight"s, PUP_SETTINGS_BACKGLASSHEIGHT),
            PUP_ZORDER_BACKGLASS,
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowRotation"s, 0));
         if (m_pBackglassScreen->GetMode() != PUP_SCREEN_MODE_OFF)
            m_pBackglassWindow->Show();
      }
      else {
         PLOGW.printf("PUP Backglass screen not found");
      }
   }
   else {
      PLOGI.printf("PUP Backglass window disabled");
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
            if (m_pTopperScreen->GetMode() != PUP_SCREEN_MODE_OFF)
               m_pTopperWindow->Show();
      }
      else {
         PLOGW.printf("PUP topper screen not found");
      }
   }
   else {
      PLOGI.printf("PUP Topper window disabled");
   }
}

void PUPManager::ProcessQueue()
{
   while (true) {
      std::unique_lock<std::mutex> lock(m_queueMutex);
      m_queueCondVar.wait(lock, [this] { return !m_triggerDataQueue.empty() || !m_isRunning; });

      if (!m_isRunning) {
         while (!m_triggerDataQueue.empty())
            m_triggerDataQueue.pop();
         break;
      }

      PUPTriggerData triggerData = m_triggerDataQueue.front();
      m_triggerDataQueue.pop();
      lock.unlock();

      vector<PUPTrigger*>* pTriggers = GetTriggers(triggerData.type + std::to_string(triggerData.number));
      if (pTriggers) {
         for (PUPTrigger* pTrigger : *pTriggers)
            pTrigger->GetScreen()->ProcessTrigger(pTrigger);
      }
   }
}

void PUPManager::Stop()
{
   {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_isRunning = false;
   }

   m_queueCondVar.notify_all();
   if (m_thread.joinable())
      m_thread.join();

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

   for (auto& [key, triggers] : m_triggerMap) {
      for (PUPTrigger* pTrigger : triggers)
         delete pTrigger;
   }

   m_triggerMap.clear();

   for (auto& [key, pFont] : m_fontMap)
      TTF_CloseFont(pFont);

   m_fontMap.clear();

   m_szPath.clear();
   m_init = false;

   TTF_Quit();
}