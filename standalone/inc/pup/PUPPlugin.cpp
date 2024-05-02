#include "stdafx.h"

#include "PUPPlugin.h"
#include "PUPManager.h"

#include "DMDUtil/Config.h"

void OnPUPCaptureTrigger(uint16_t id, void* pUserData)
{
   PUPPlugin* pPlugin = (PUPPlugin*)pUserData;
   pPlugin->DataReceive('D', id, 1);
}

PUPPlugin::PUPPlugin() : Plugin()
{
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
   m_pManager = PUPManager::GetInstance();

   if (m_pManager->IsInit()) {
      PLOGW.printf("PUP already initialized");
      return;
   }

   if (m_pManager->LoadConfig(szRomName)) {
      DataReceive( 'D', 0, 1 );
      m_pManager->Start();
   }

   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetPUPTriggerCallback(OnPUPCaptureTrigger, this);
}

void PUPPlugin::PluginFinish()
{
   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetPUPTriggerCallback(NULL, NULL);

   m_pManager->Stop();
}

void PUPPlugin::DataReceive(char type, int number, int value)
{
   m_pManager->QueueTriggerData({ type, number, value });
}