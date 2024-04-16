#include "core/stdafx.h"

#include "PUPPlugin.h"

#include "DMDUtil/Config.h"


void OnPUPCaptureTrigger(uint16_t id, void* pUserData)
{
   PUPPlugin* pPlugin = (PUPPlugin*)pUserData;
   pPlugin->QueueData({ 'D', id, 1 });
}


PUPPlugin::PUPPlugin() : Plugin()
{
   m_pWindow = nullptr;
   m_pThread = nullptr;
}

PUPPlugin::~PUPPlugin()
{
}

const std::string& PUPPlugin::GetName() const
{
   static std::string name = "PinUpPlugin";
   return name;
}

void PUPPlugin::PluginInit(const string& szTableFilename, const string& szRomName)
{
   m_szPath = g_pvp->m_currentTablePath + "PupVideos" +
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

         if (pScreen)
            m_screens.push_back(pScreen);
      }
      PLOGI.printf("Screens loaded: file=%s, size=%d", screensPath.c_str(), m_screens.size());
   }
   else {
      PLOGE.printf("Unable to load %s", screensPath.c_str());

      return;
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
            m_playlists.push_back(pPlaylist);
      }
      PLOGI.printf("Playlists loaded: file=%s, size=%d", playlistsPath.c_str(), m_playlists.size());
   }
   else {
      PLOGE.printf("Unable to load %s", playlistsPath.c_str());

      return;
   }

   // Load Triggers

   string triggersPath = m_szPath + "triggers.pup";
   std::ifstream triggersFile;
   triggersFile.open(triggersPath, std::ifstream::in);
   if (triggersFile.is_open()) {
      string line;
      int i = 0;
      while (std::getline(triggersFile, line)) {
         if (++i == 1)
            continue;

         PUPTrigger* pTrigger = PUPTrigger::CreateFromCSVLine(line, m_screens, m_playlists);

         if (pTrigger)
            m_triggers.push_back(pTrigger);
      }
      PLOGI.printf("Triggers loaded: file=%s, size=%d", triggersPath.c_str(), m_triggers.size());
   }
   else {
      PLOGE.printf("Unable to load %s", triggersPath.c_str());

      return;
   }

   //

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindow"s, true)) {
      PLOGI.printf("PUP window disabled");
      return;
   }

   m_pWindow = new VP::VideoWindow("PUP",
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowX"s, PUP_SETTINGS_X),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowY"s, PUP_SETTINGS_Y),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowWidth"s, PUP_SETTINGS_WIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowHeight"s, PUP_SETTINGS_HEIGHT),
      PUP_ZORDER,
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowRotation"s, 0));

   QueueData({ 'D', 0, 1 });

   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetPUPTriggerCallback(OnPUPCaptureTrigger, this);

   m_running = true;

   m_pThread = new std::thread([this]() {
      while (m_running) {
         ProcessData();
         SDL_Delay(10);
      }
   });
}

void PUPPlugin::ProcessData()
{
   while (!m_queue.empty()) {
      std::unique_lock<std::mutex> guard(m_mutex);
      auto item = m_queue.front();
      m_queue.pop();
      guard.unlock();

      if (item.value == 0)
         continue;

      for (PUPTrigger* pTrigger : m_triggers) {
         if ((pTrigger->GetScreen()->GetScreenNum() == 2) && pTrigger->IsActive()) {
            if (pTrigger->GetTrigger() == item.type + std::to_string(item.number)) {
               PLOGI.printf("Found pup trigger for type=%c, number=%d, value=%d, trigger={%s}", item.type, item.number, item.value, pTrigger->ToString().c_str());

               if (pTrigger->GetPlaylist()->GetFolder() == "PuPOverlays") {
                  string szPlayFile = pTrigger->GetPlayFile();
                  string szPath = m_szPath + pTrigger->GetPlaylist()->GetFolder() + PATH_SEPARATOR_CHAR + szPlayFile;
                  m_pWindow->SetOverlay(szPath);
               }
               else {
                  string szPlayFile = pTrigger->GetPlayFile();
                  if (!szPlayFile.empty()) {
                     string szPath = m_szPath + pTrigger->GetPlaylist()->GetFolder() + PATH_SEPARATOR_CHAR + szPlayFile;
                     m_pWindow->Play(szPath, pTrigger->GetVolume());
                  }
               }

               break;
            }
         }
      }
   }
}

void PUPPlugin::PluginFinish()
{
   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetPUPTriggerCallback(NULL, NULL);

   m_running = false;

   if (m_pThread) {
      m_pThread->join();
      delete m_pThread;
      m_pThread = NULL;
   }
}

void PUPPlugin::QueueData(PUPTriggerData data)
{
   std::lock_guard<std::mutex> guard(m_mutex);
   m_queue.push({ data.type, data.number, data.value });
}

void PUPPlugin::DataReceive(char type, int number, int value)
{
   QueueData({ type, number, value });
}