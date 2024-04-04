#include "stdafx.h"

#include "Standalone.h"

#include "inc/b2s/plugin/PluginHost.h"
#include "inc/dof/DOFPlugin.h"
#include "inc/pup/PUPPlugin.h"

#include "DMDUtil/Config.h"

void OnDMDUtilLog(DMDUtil_LogLevel logLevel, const char* format, va_list args)
{
   char buffer[4096];
   vsnprintf(buffer, sizeof(buffer), format, args);

   PLOGI.printf("%s", buffer);
}

Standalone* Standalone::m_pInstance = NULL;

Standalone* Standalone::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new Standalone();

   return m_pInstance;
}

Standalone::Standalone()
{
}

Standalone::~Standalone()
{
}

void Standalone::Startup()
{
   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;
   
   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetLogCallback(OnDMDUtilLog);
   pConfig->SetZeDMD(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMD"s, true));
   pConfig->SetZeDMDDevice(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDDevice"s, ""s).c_str());
   pConfig->SetZeDMDDebug(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDDebug"s, false));
   pConfig->SetZeDMDRGBOrder(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDRGBOrder"s, -1));
   pConfig->SetZeDMDBrightness(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDBrightness"s, -1));
   pConfig->SetZeDMDSaveSettings(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDSaveSettings"s, false));
   pConfig->SetPixelcade(pSettings->LoadValueWithDefault(Settings::Standalone, "Pixelcade"s, true));
   pConfig->SetPixelcadeDevice(pSettings->LoadValueWithDefault(Settings::Standalone, "PixelcadeDevice"s, ""s).c_str());
   pConfig->SetDMDServer(pSettings->LoadValueWithDefault(Settings::Standalone, "DMDServer"s, false));
   pConfig->SetDMDServerAddr(pSettings->LoadValueWithDefault(Settings::Standalone, "DMDServerAddr"s, "localhost"s).c_str());
   pConfig->SetDMDServerPort(pSettings->LoadValueWithDefault(Settings::Standalone, "DMDServerPort"s, 6789));
   pConfig->SetPUPCapture(pSettings->LoadValueWithDefault(Settings::Standalone, "PUPCapture"s, false));

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "B2SPlugins"s, false)) {
      if (pSettings->LoadValueWithDefault(Settings::Standalone, "DOFPlugin"s, true))
         PluginHost::GetInstance()->RegisterPlugin(new DOFPlugin());
      if (pSettings->LoadValueWithDefault(Settings::Standalone, "PUPPlugin"s, true))
         PluginHost::GetInstance()->RegisterPlugin(new PUPPlugin());
   }
}
