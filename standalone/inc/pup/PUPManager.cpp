#include "core/stdafx.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPCustomPos.h"
#include "PUPWindow.h"

#include <filesystem>

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
   m_isRunning = false;
   m_szRootPath = find_case_insensitive_directory_path(g_pvp->m_currentTablePath + "pupvideos");
}

PUPManager::~PUPManager()
{
}

void PUPManager::LoadConfig(const string& szRomName)
{
   if (m_init) {
      PLOGW.printf("PUP already initialized");
      return;
   }

   if (m_szRootPath.empty())
   {
      PLOGI.printf("No pupvideos folder found, not initializing PUP");
      return;
   }

   m_szPath = find_case_insensitive_directory_path(m_szRootPath + szRomName);
   if (m_szPath.empty())
      return;

   PLOGI.printf("PUP path: %s", m_szPath.c_str());

   // Load playlists

   LoadPlaylists();

   // Load screens

   string szScreensPath = find_case_insensitive_file_path(m_szPath + "screens.pup");
   if (!szScreensPath.empty()) {
      std::ifstream screensFile;
      screensFile.open(szScreensPath, std::ifstream::in);
      if (screensFile.is_open()) {
         string line;
         int i = 0;
         while (std::getline(screensFile, line)) {
            if (++i == 1)
               continue;
            AddScreen(PUPScreen::CreateFromCSV(line, m_playlists));
         }
      }
      else {
         PLOGE.printf("Unable to load %s", szScreensPath.c_str());
      }
   }
   else {
      PLOGI.printf("No screens.pup file found");
   }

   // Determine child screens

   for (auto& [key, pScreen] : m_screenMap) {
      PUPCustomPos* pCustomPos = pScreen->GetCustomPos();
      if (pCustomPos) {
         std::map<int, PUPScreen*>::iterator it = m_screenMap.find(pCustomPos->GetSourceScreen());
         PUPScreen* pParentScreen = it != m_screenMap.end() ? it->second : nullptr;
         if (pParentScreen && pScreen != pParentScreen)
            pParentScreen->AddChild(pScreen);
      }
   }

   // Load Fonts

   string szFontsPath = find_case_insensitive_directory_path(m_szPath + "FONTS");
   if (!szFontsPath.empty()) {
      for (const auto& entry : std::filesystem::directory_iterator(szFontsPath)) {
         if (entry.is_regular_file()) {
            string szFontPath = entry.path().string();
            if (extension_from_path(szFontPath) == "ttf")
            {
               if (TTF_Font* pFont = TTF_OpenFont(szFontPath.c_str(), 8))
               {
                  AddFont(pFont, entry.path().filename());
               }else{
                  PLOGE.printf("Failed to load font: %s %s", szFontPath.c_str(), SDL_GetError());
               }
            }
         }
      }
   }
   else {
      PLOGI.printf("No FONTS folder found");
   }

   m_init = true;

   return;
}

void PUPManager::LoadPlaylists()
{
   string szPlaylistsPath = find_case_insensitive_file_path(GetPath() + "playlists.pup");
   std::ifstream playlistsFile;
   playlistsFile.open(szPlaylistsPath, std::ifstream::in);
   if (playlistsFile.is_open()) {
      ankerl::unordered_dense::set<string> lowerPlaylistNames;
      string line;
      int i = 0;
      while (std::getline(playlistsFile, line)) {
         if (++i == 1)
            continue;
         if (PUPPlaylist* pPlaylist = PUPPlaylist::CreateFromCSV(line)) {
            string folderNameLower = lowerCase(pPlaylist->GetFolder());
            if (lowerPlaylistNames.find(folderNameLower) == lowerPlaylistNames.end()) {
               m_playlists.push_back(pPlaylist);
               lowerPlaylistNames.insert(folderNameLower);
            }
            else {
               PLOGW.printf("Duplicate playlist: playlist=%s", pPlaylist->ToString().c_str());
            }
         }
      }
   }
}

const string& PUPManager::GetRootPath()
{
   return m_szRootPath;
}

bool PUPManager::AddScreen(PUPScreen* pScreen)
{
   if (!pScreen) {
      PLOGE.printf("Null screen argument");
      return false;
   }

   if (HasScreen(pScreen->GetScreenNum())) {
      PLOGW.printf("Duplicate screen: screen={%s}", pScreen->ToString(false).c_str());
      delete pScreen;
      return false;
   }

   m_screenMap[pScreen->GetScreenNum()] = pScreen;

   PLOGI.printf("Screen added: screen={%s}", pScreen->ToString().c_str());

   return true;
}

bool PUPManager::AddScreen(LONG lScreenNum)
{
   return AddScreen(PUPScreen::CreateDefault(lScreenNum, m_playlists));
}

bool PUPManager::HasScreen(int screenNum)
{
   std::map<int, PUPScreen*>::iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end();
}

PUPScreen* PUPManager::GetScreen(int screenNum)
{
   if (!m_init) {
      PLOGW.printf("Getting screen before initialization");
   }

   std::map<int, PUPScreen*>::iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end() ? it->second : nullptr;
}

bool PUPManager::AddFont(TTF_Font* pFont, const string& szFilename)
{
   if (!pFont)
      return false;

   m_fonts.push_back(pFont);

   const string szFamilyName = string(TTF_GetFontFamilyName(pFont));

   const string szNormalizedFamilyName = lowerCase(string_replace_all(szFamilyName, "  ", " "));
   m_fontMap[szNormalizedFamilyName] = pFont;

   string szStyleName = string(TTF_GetFontStyleName(pFont));
   if (szStyleName != "Regular")
   {
      const string szFullName = szFamilyName + ' ' + szStyleName;
      const string szNormalizedFullName = lowerCase(string_replace_all(szFullName, "  ", " "));
      m_fontMap[szNormalizedFullName] = pFont;
   }

   const string szNormalizedFilename = lowerCase(szFilename.substr(0, szFilename.length() - 4));
   m_fontFilenameMap[szNormalizedFilename] = pFont;

   PLOGI.printf("Font added: familyName=%s, styleName=%s, filename=%s", szFamilyName.c_str(), szStyleName.c_str(), szFilename.c_str());

   return true;
}

TTF_Font* PUPManager::GetFont(const string& szFont)
{
   string szNormalizedFamilyName = lowerCase(string_replace_all(szFont, "  ", " "));

   std::map<string, TTF_Font*>::iterator it = m_fontMap.find(szNormalizedFamilyName);
   if (it != m_fontMap.end())
      return it->second;
   it = m_fontFilenameMap.find(lowerCase(szFont));
   if (it != m_fontFilenameMap.end())
      return it->second;

   return nullptr;
}

void PUPManager::QueueTriggerData(PUPTriggerData data)
{
   {
      std::lock_guard<std::mutex> lock(m_queueMutex);
      m_triggerDataQueue.push({ data.type, data.number, data.value });
   }
   m_queueCondVar.notify_one();
}

void PUPManager::Start()
{
   if (!m_init)
      return;

   PLOGI.printf("PUP start");

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindows"s, true)) {
      AddWindow("Topper",
         PUP_SCREEN_TOPPER,
         PUP_SETTINGS_TOPPERX,
         PUP_SETTINGS_TOPPERY,
         PUP_SETTINGS_TOPPERWIDTH,
         PUP_SETTINGS_TOPPERHEIGHT,
         PUP_ZORDER_TOPPER);

      AddWindow("Backglass",
         PUP_SCREEN_BACKGLASS,
         PUP_SETTINGS_BACKGLASSX,
         PUP_SETTINGS_BACKGLASSY,
         PUP_SETTINGS_BACKGLASSWIDTH,
         PUP_SETTINGS_BACKGLASSHEIGHT,
         PUP_ZORDER_BACKGLASS);

      AddWindow("DMD",
         PUP_SCREEN_DMD,
         PUP_SETTINGS_DMDX,
         PUP_SETTINGS_DMDY,
         PUP_SETTINGS_DMDWIDTH,
         PUP_SETTINGS_DMDHEIGHT,
         PUP_ZORDER_DMD);

      AddWindow("Playfield",
         PUP_SCREEN_PLAYFIELD,
         PUP_SETTINGS_PLAYFIELDX,
         PUP_SETTINGS_PLAYFIELDY,
         PUP_SETTINGS_PLAYFIELDWIDTH,
         PUP_SETTINGS_PLAYFIELDHEIGHT,
         PUP_ZORDER_PLAYFIELD);

      AddWindow("FullDMD",
         PUP_SCREEN_FULLDMD,
         PUP_SETTINGS_FULLDMDX,
         PUP_SETTINGS_FULLDMDY,
         PUP_SETTINGS_FULLDMDWIDTH,
         PUP_SETTINGS_FULLDMDHEIGHT,
         PUP_ZORDER_FULLDMD);
   }
   else {
      PLOGI.printf("PUP windows disabled");
   }

   m_isRunning = true;
   m_thread = std::thread(&PUPManager::ProcessQueue, this);

   for (auto& [key, pScreen] : m_screenMap)
      pScreen->Start();
}

void PUPManager::AddWindow(const string& szWindowName, int screen, int x, int y, int width, int height, int zOrder)
{
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   string szPrefix = "PUP" + szWindowName;

   PUPScreen* pScreen = GetScreen(pSettings->LoadValueWithDefault(Settings::Standalone, szPrefix + "Screen"s, screen));
   if (!pScreen) {
      PLOGW.printf("PUP %s screen not found", szWindowName.c_str());
      return;
   }

   if (pScreen->HasParent()) {
      PLOGI.printf("PUP %s screen is a child screen, window disabled", szWindowName.c_str());
      return;
   }

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, szPrefix + "Window"s, true)) {
      PLOGI.printf("PUP %s window disabled", szWindowName.c_str());
      return;
   }

   PUPWindow* pWindow = new PUPWindow(pScreen, szPrefix,
      pSettings->LoadValueWithDefault(Settings::Standalone, szPrefix + "WindowX"s, x),
      pSettings->LoadValueWithDefault(Settings::Standalone, szPrefix + "WindowY"s, y),
      pSettings->LoadValueWithDefault(Settings::Standalone, szPrefix + "WindowWidth"s, width),
      pSettings->LoadValueWithDefault(Settings::Standalone, szPrefix + "WindowHeight"s, height),
      zOrder,
      pSettings->LoadValueWithDefault(Settings::Standalone, szPrefix + "WindowRotation"s, 0));

   if (pScreen->GetMode() != PUP_SCREEN_MODE_OFF)
      pWindow->Show();

   m_windows.push_back(pWindow);
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

      for (auto& [key, pScreen] : m_screenMap)
         pScreen->QueueTrigger(triggerData.type, triggerData.number, triggerData.value);
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

   for (auto pWindow : m_windows)
      delete pWindow;

   m_windows.clear();

   for (auto& [key, pScreen] : m_screenMap)
      delete pScreen;

   m_screenMap.clear();

   for (auto& pFont : m_fonts)
      TTF_CloseFont(pFont);

   m_fonts.clear();
   m_fontMap.clear();
   m_fontFilenameMap.clear();

   m_szPath.clear();
   m_init = false;
}
