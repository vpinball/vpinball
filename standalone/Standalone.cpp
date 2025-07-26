#include "core/stdafx.h"

#include "Standalone.h"

#include "inc/b2s/plugin/PluginHost.h"
#include "inc/dof/DOFPlugin.h"
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
}

Standalone::~Standalone()
{
}

void Standalone::PreStartup()
{
   PLOGI.printf("Performing pre-startup standalone actions");

   Settings* const pSettings = &g_pplayer->m_ptable->m_settings;

   if (pSettings->LoadValueWithDefault(Settings::Standalone, "B2SPlugins"s, false)) {
      if (!pSettings->LoadValueWithDefault(pSettings->GetSection("Plugin.DOF"), "Enable"s, false)) {
         if (pSettings->LoadValueWithDefault(Settings::Standalone, "DOFPlugin"s, true))
            PluginHost::GetInstance()->RegisterPlugin(new DOFPlugin());
      }
   }
}

void Standalone::PostStartup()
{
   PLOGI.printf("Performing post-startup standalone actions");
}

void Standalone::Shutdown()
{
   PLOGI.printf("Performing shutdown standalone actions");

   PluginHost::GetInstance()->UnregisterAllPlugins();
}
