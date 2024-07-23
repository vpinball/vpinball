#include "stdafx.h"
#include "Logger.h"

#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/RollingFileAppender.h>
#ifdef __STANDALONE__
#ifndef __ANDROID__
#include <plog/Appenders/ColorConsoleAppender.h>
#else
#include <plog/Appenders/AndroidAppender.h>
#endif
#endif

class DebugAppender : public plog::IAppender
{
public:
   virtual void write(const plog::Record &record) PLOG_OVERRIDE
   {
      if (g_pvp == nullptr || g_pplayer == nullptr)
         return;
      auto table = g_pvp->GetActiveTable();
      if (table == nullptr)
         return;
      #ifdef _WIN32
      // Convert from wchar* to char* on Win32
      auto msg = record.getMessage();
      const int len = (int)lstrlenW(msg);
      char *const szT = new char[len + 1];
      WideCharToMultiByteNull(CP_ACP, 0, msg, -1, szT, len + 1, nullptr, nullptr);
      table->m_pcv->AddToDebugOutput(szT);
      delete [] szT;
      #else
      table->m_pcv->AddToDebugOutput(record.getMessage());
      #endif
   }
};

Logger* Logger::m_pInstance = NULL;

Logger* Logger::GetInstance()
{
   if (!m_pInstance)
      m_pInstance = new Logger();

   return m_pInstance;
}

void Logger::SetupLogger(const bool enable)
{
   plog::Severity maxLogSeverity = plog::none;
   if (enable)
   {
      static bool initialized = false;
      if (!initialized)
      {
         initialized = true;
         string szLogPath = g_pvp->m_szMyPrefPath + "vpinball.log";
         static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(szLogPath.c_str(), 1024 * 1024 * 5, 1);
         static DebugAppender debugAppender;
         plog::Logger<PLOG_DEFAULT_INSTANCE_ID>::getInstance()->addAppender(&debugAppender);
         plog::Logger<PLOG_DEFAULT_INSTANCE_ID>::getInstance()->addAppender(&fileAppender);
         plog::Logger<PLOG_NO_DBG_OUT_INSTANCE_ID>::getInstance()->addAppender(&fileAppender);

#ifdef __STANDALONE__
#ifndef __ANDROID__
         static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
         plog::Logger<PLOG_DEFAULT_INSTANCE_ID>::getInstance()->addAppender(&consoleAppender);
         plog::Logger<PLOG_NO_DBG_OUT_INSTANCE_ID>::getInstance()->addAppender(&consoleAppender);
#else
         static plog::AndroidAppender<plog::TxtFormatter> androidAppender("vpinball");
         plog::Logger<PLOG_DEFAULT_INSTANCE_ID>::getInstance()->addAppender(&androidAppender);
         plog::Logger<PLOG_NO_DBG_OUT_INSTANCE_ID>::getInstance()->addAppender(&androidAppender);
#endif
#endif
      }
      #ifdef _DEBUG
      maxLogSeverity = plog::debug;
      #else
      maxLogSeverity = plog::info;
      #endif
   }
   plog::Logger<PLOG_DEFAULT_INSTANCE_ID>::getInstance()->setMaxSeverity(maxLogSeverity);
   plog::Logger<PLOG_NO_DBG_OUT_INSTANCE_ID>::getInstance()->setMaxSeverity(maxLogSeverity);
}

void Logger::Init()
{
   plog::init<PLOG_DEFAULT_INSTANCE_ID>();
   plog::init<PLOG_NO_DBG_OUT_INSTANCE_ID>(); // Logger that do not show in the debug window to avoid duplicated messages
}