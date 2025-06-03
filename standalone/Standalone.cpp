#include "core/stdafx.h"

#include "Standalone.h"

#include "inc/b2s/plugin/PluginHost.h"
#include "inc/dof/DOFPlugin.h"
#include "inc/pup/PUPPlugin.h"
#include "inc/pup/PUPManager.h"

#include "DMDUtil/Config.h"

#include <csignal>

#ifdef __LIBVPINBALL__
#include "VPinballLib.h"
#endif

void OnDMDUtilLog(DMDUtil_LogLevel logLevel, const char* format, va_list args)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      if (logLevel == DMDUtil_LogLevel_INFO) {
         PLOGI << buffer;
      }
      else if (logLevel == DMDUtil_LogLevel_ERROR) {
         PLOGE << buffer;
      }
      free(buffer);
   }
}

void OnSignalHandler(int signum)
{
   PLOGI.printf("Exiting from signal: %d", signum);
   exit(-9999);
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
   struct sigaction sigIntHandler;
   sigIntHandler.sa_handler = OnSignalHandler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;
   sigaction(SIGINT, &sigIntHandler, nullptr);

   m_pPUPManager = nullptr;
}

Standalone::~Standalone()
{
}

void Standalone::PreStartup()
{
   PLOGI.printf("Performing pre-startup standalone actions");

#ifdef __LIBVPINBALL__
  VPinballLib::VPinball::GetInstance().LoadPlugins();
#endif

   m_pPUPManager = new PUPManager();

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetLogCallback(OnDMDUtilLog);

   if (!pSettings->LoadValueWithDefault(pSettings->GetSection("Plugin.DMDUtil"), "Enable"s, false)) {
      pConfig->SetZeDMD(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMD"s, true));
      pConfig->SetZeDMDDevice(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDDevice"s, string()).c_str());
      pConfig->SetZeDMDDebug(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDDebug"s, false));
      pConfig->SetZeDMDBrightness(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDBrightness"s, -1));
      pConfig->SetZeDMDWiFiEnabled(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDWiFi"s, false));
      pConfig->SetZeDMDWiFiAddr(pSettings->LoadValueWithDefault(Settings::Standalone, "ZeDMDWiFiAddr"s, "zedmd-wifi.local"s).c_str());
      pConfig->SetPixelcade(pSettings->LoadValueWithDefault(Settings::Standalone, "Pixelcade"s, true));
      pConfig->SetPixelcadeDevice(pSettings->LoadValueWithDefault(Settings::Standalone, "PixelcadeDevice"s, string()).c_str());
      pConfig->SetDMDServer(pSettings->LoadValueWithDefault(Settings::Standalone, "DMDServer"s, false));
      pConfig->SetDMDServerAddr(pSettings->LoadValueWithDefault(Settings::Standalone, "DMDServerAddr"s, "localhost"s).c_str());
      pConfig->SetDMDServerPort(pSettings->LoadValueWithDefault(Settings::Standalone, "DMDServerPort"s, 6789));
   }

   pConfig->SetPUPCapture(pSettings->LoadValueWithDefault(Settings::Standalone, "PUPCapture"s, false));

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "B2SPlugins"s, false)) {
      if (pSettings->LoadValueWithDefault(Settings::Standalone, "DOFPlugin"s, true))
         PluginHost::GetInstance()->RegisterPlugin(new DOFPlugin());
      if (pSettings->LoadValueWithDefault(Settings::Standalone, "PUPPlugin"s, true))
         PluginHost::GetInstance()->RegisterPlugin(new PUPPlugin());
   }
}

void Standalone::PostStartup()
{
   PLOGI.printf("Performing post-startup standalone actions");

   m_pPUPManager->Start();
}

void Standalone::Shutdown()
{
   PLOGI.printf("Performing shutdown standalone actions");

   PluginHost::GetInstance()->UnregisterAllPlugins();

   delete m_pPUPManager;
   m_pPUPManager = nullptr;

#ifdef __LIBVPINBALL__
   VPinballLib::VPinball::GetInstance().UnloadPlugins();
#endif
}
