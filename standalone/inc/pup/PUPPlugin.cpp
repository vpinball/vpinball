#include "core/stdafx.h"

#include "PUPPlugin.h"
#include "PUPManager.h"

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

   m_pManager = new PUPManager();
}

PUPPlugin::~PUPPlugin()
{
   delete m_pManager;
}

const std::string& PUPPlugin::GetName() const
{
   static std::string name = "PinUpPlugin";
   return name;
}

void PUPPlugin::PluginInit(const string& szTableFilename, const string& szRomName)
{
   if (!m_pManager->LoadConfig(szRomName)) {
      PLOGW.printf("Unable to load PUP config for %s", szRomName.c_str());
      return;
   }

   m_pBackglass = m_pManager->GetBackglass();
   if (!m_pBackglass) {
      PLOGW.printf("PUP backlass screen not found");
      return;
   }

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (!pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindow"s, true)) {
      PLOGI.printf("PUP window disabled");
      return;
   }

   m_pWindow = new PUPWindow("PUP",
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowX"s, PUP_SETTINGS_X),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowY"s, PUP_SETTINGS_Y),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowWidth"s, PUP_SETTINGS_WIDTH),
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowHeight"s, PUP_SETTINGS_HEIGHT),
      PUP_ZORDER,
      pSettings->LoadValueWithDefault(Settings::Standalone, "PUPWindowRotation"s, 0));

   m_running = true;

   m_pThread = new std::thread([this]() {
      PUPPlaylist* pPlaylist = m_pManager->GetPlaylist(m_pBackglass->GetPlayList());
      if (pPlaylist) {
         string szPath = m_pManager->GetPath(pPlaylist, m_pBackglass->GetPlayFile());
         if (!szPath.empty())
            m_pWindow->SetBG(szPath, pPlaylist->GetVolume(),
               m_pBackglass->IsLoopit() ? PUP_TRIGGER_PLAY_ACTION_LOOP : PUP_TRIGGER_PLAY_ACTION_NORMAL, 0);
      }
      
      QueueData({ 'D', 0, 1 });

      DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
      pConfig->SetPUPTriggerCallback(OnPUPCaptureTrigger, this);

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

      PUPTrigger* pTrigger = m_pBackglass->GetTrigger(item.type + std::to_string(item.number));
      if (pTrigger) {
         PLOGI.printf("Found pup trigger for type=%c, number=%d, value=%d, trigger={%s}", item.type, item.number, item.value, pTrigger->ToString().c_str());
         PUPPlaylist* pPlaylist = m_pManager->GetPlaylist(pTrigger->GetPlaylist());
         if (pPlaylist) {
            string szPlayfile = pTrigger->GetPlayFile();
            string szPath = m_pManager->GetPath(pPlaylist, szPlayfile);
            if (szPath.empty())
               continue;
            if (pPlaylist->GetFunction() == PUP_PLAYLIST_FUNCTION_OVERLAYS) {
               m_pWindow->SetOverlay(szPath);
            }
            else {
               m_pWindow->Play(szPath, (!szPlayfile.empty() ? pTrigger->GetVolume() : pPlaylist->GetVolume()),
                  pTrigger->GetPlayAction(), pTrigger->GetPriority());
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