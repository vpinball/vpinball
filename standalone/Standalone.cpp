#include "core/stdafx.h"

#include "Standalone.h"
#include <csignal>

#ifdef __LIBVPINBALL__
#include "VPinballLib.h"
#endif

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
}

void Standalone::PostStartup()
{
   PLOGI.printf("Performing post-startup standalone actions");
}

void Standalone::Shutdown()
{
   PLOGI.printf("Performing shutdown standalone actions");
}
