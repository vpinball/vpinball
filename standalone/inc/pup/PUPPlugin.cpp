#include "core/stdafx.h"

#include "PUPPlugin.h"
#include "PUPManager.h"

#include "DMDUtil/Config.h"

#include "standalone/Standalone.h"

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
   static const std::string name = "PinUpPlugin"s;
   return name;
}

void PUPPlugin::PluginInit(const string& szTableFilename, const string& szRomName)
{
   g_pStandalone->GetPUPManager()->LoadConfig(szRomName);

   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetPUPTriggerCallback(OnPUPCaptureTrigger, this);
}

void PUPPlugin::PluginFinish()
{
   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetPUPTriggerCallback(NULL, NULL);

   g_pStandalone->GetPUPManager()->Stop();
}

void PUPPlugin::DataReceive(char type, int number, int value)
{
   g_pStandalone->GetPUPManager()->QueueTriggerData({ type, number, value });
}