#include "core/stdafx.h"

#include "PUPManager.h"
#include "PUPScreen.h"
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
   m_pBackglassWindow = nullptr;
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

   m_szRootPath = find_directory_case_insensitive(g_pvp->m_currentTablePath, "pupvideos");
   if (m_szRootPath.empty())
      return false;

   m_szPath = find_directory_case_insensitive(m_szRootPath, szRomName);
   if (m_szPath.empty())
      return false;

   PLOGI.printf("PUP path: %s", m_szPath.c_str());

   TTF_Init();

   // Load screens

   string szScreensPath = find_path_case_insensitive(m_szPath + "screens.pup");
   std::ifstream screensFile;
   screensFile.open(szScreensPath, std::ifstream::in);
   if (screensFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(screensFile, line)) {
         if (++i == 1)
            continue;
         AddScreen(PUPScreen::CreateFromCSV(line));
      }
   }
   else {
      PLOGE.printf("Unable to load %s", szScreensPath.c_str());
      return false;
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

   // Load Fonts

   string szFontsPath = find_directory_case_insensitive(m_szPath, "FONTS");
   if (!szFontsPath.empty()) {
      for (const auto& entry : std::filesystem::directory_iterator(szFontsPath)) {
         if (entry.is_regular_file()) {
            string szFontPath = entry.path().string();
            if (extension_from_path(szFontPath) == "ttf")
               AddFont(TTF_OpenFont(szFontPath.c_str(), 8), entry.path().filename());
         }
      }
   }
   else {
      PLOGI.printf("No FONTS folder found");
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

   PLOGI.printf("Screen added: screen={%s}", pScreen->ToString().c_str());

   return true;
}

PUPScreen* PUPManager::GetScreen(int screenNum)
{
   std::map<int, PUPScreen*>::iterator it = m_screenMap.find(screenNum);
   return it != m_screenMap.end() ? it->second : nullptr;
}

bool PUPManager::AddFont(TTF_Font* pFont, const string& szFilename)
{
   if (!pFont)
      return false;

   string szFamilyName = string(TTF_FontFaceFamilyName(pFont));
   string szNormalizedFamilyName = string_to_lower(string_replace_all(szFamilyName, "  ", " "));

   m_fontMap[szNormalizedFamilyName] = pFont;
   m_fontFilenameMap[string_to_lower(szFilename.substr(0, szFilename.length() - 4))] = pFont;

   PLOGI.printf("Font added: familyName=%s, filename=%s", szFamilyName.c_str(), szFilename.c_str());

   return true;
}

TTF_Font* PUPManager::GetFont(const string& szFont)
{
   string szNormalizedFamilyName = string_to_lower(string_replace_all(szFont, "  ", " "));

   std::map<string, TTF_Font*>::iterator it = m_fontMap.find(szNormalizedFamilyName);
   if (it != m_fontMap.end())
      return it->second;
   it = m_fontFilenameMap.find(string_to_lower(szFont));
   if (it != m_fontFilenameMap.end())
      return it->second;

   return nullptr;
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
      PUPScreen* pScreen = GetScreen(pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowScreen"s, 2));
      if (pScreen) {
         m_pBackglassWindow = new PUPWindow(pScreen, "PUPBackglass",
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowX"s, PUP_SETTINGS_BACKGLASSX),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowY"s, PUP_SETTINGS_BACKGLASSY),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowWidth"s, PUP_SETTINGS_BACKGLASSWIDTH),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowHeight"s, PUP_SETTINGS_BACKGLASSHEIGHT),
            PUP_ZORDER_BACKGLASS,
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPBackglassWindowRotation"s, 0));
         if (pScreen->GetMode() != PUP_SCREEN_MODE_OFF)
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
      PUPScreen* pScreen = GetScreen(pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowScreen"s, 0));
      if (pScreen) {
         m_pTopperWindow = new PUPWindow(pScreen, "PUPTopper",
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowX"s, PUP_SETTINGS_TOPPERX),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowY"s, PUP_SETTINGS_TOPPERY),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowWidth"s, PUP_SETTINGS_TOPPERWIDTH),
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowHeight"s, PUP_SETTINGS_TOPPERHEIGHT),
            PUP_ZORDER_TOPPER,
            pSettings->LoadValueWithDefault(Settings::Standalone, "PUPTopperWindowRotation"s, 0));
         if (pScreen->GetMode() != PUP_SCREEN_MODE_OFF)
            m_pTopperWindow->Show();
      }
      else {
         PLOGW.printf("PUP Topper screen not found");
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

   delete m_pBackglassWindow;
   m_pBackglassWindow = nullptr;

   delete m_pTopperWindow;
   m_pTopperWindow = nullptr;;

   for (auto& [key, pScreen] : m_screenMap)
      delete pScreen;

   m_screenMap.clear();

   for (auto& [key, pFont] : m_fontMap)
      TTF_CloseFont(pFont);

   m_fontMap.clear();

   m_szPath.clear();
   m_init = false;

   TTF_Quit();
}