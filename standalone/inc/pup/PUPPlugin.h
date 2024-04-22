#pragma once

#include "../b2s/plugin/Plugin.h"

#include "PUPScreen.h"
#include "PUPPlaylist.h"
#include "PUPTrigger.h"

#include "PUPWindow.h"

#include <queue>
#include <mutex>

#define PUP_SETTINGS_X      320
#define PUP_SETTINGS_Y      30
#define PUP_SETTINGS_WIDTH  290
#define PUP_SETTINGS_HEIGHT 218
#define PUP_ZORDER          101

struct PUPTriggerData {
    char type;
    int number;
    int value;
};

class PUPManager;

class PUPPlugin : public Plugin
{
public:
   PUPPlugin();
   ~PUPPlugin();

   const std::string& GetName() const override;
   void PluginInit(const string& szTableFilename, const string& szRomName) override;
   void PluginFinish() override;
   void DataReceive(char type, int number, int value) override;

   void ProcessData();
   void QueueData(PUPTriggerData data);

private:
   PUPWindow* m_pWindow;
   PUPScreen* m_pBackglass;
   
   std::queue<PUPTriggerData> m_queue;
   std::mutex m_mutex;
   std::thread* m_pThread;
   bool m_running;

   PUPManager* m_pManager;
};