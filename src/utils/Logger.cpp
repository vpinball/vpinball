// license:GPLv3+

#include <core/stdafx.h>
#include "Logger.h"

#include <sstream>
#include <iomanip>

#include <plog/Init.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Appenders/RollingFileAppender.h>
#ifdef __STANDALONE__
#ifndef __ANDROID__
#include <plog/Appenders/ColorConsoleAppender.h>
#else
#include <plog/Appenders/AndroidAppender.h>
#endif
#include "standalone/inc/webserver/WebServer.h"
#endif

class DebugAppender final : public plog::IAppender
{
public:
   DebugAppender()
   {
      m_uiThreadId = std::this_thread::get_id();
   }

   void write(const plog::Record &record) PLOG_OVERRIDE
   {
      if (g_pvp == nullptr || g_pplayer == nullptr)
         return;
      if (std::this_thread::get_id() != m_uiThreadId)
         return;
      auto table = g_pvp->GetActiveTable();
      if (table == nullptr)
         return;
      #ifdef _WIN32
      // Convert from wchar* to char* on Win32
      auto msg = record.getMessage();
      char * const szT = MakeChar(msg);
      table->m_pcv->AddToDebugOutput(szT);
      delete [] szT;
      #else
      table->m_pcv->AddToDebugOutput(record.getMessage());
      #endif
   }

private:
   std::thread::id m_uiThreadId;
};

#ifdef __LIBVPINBALL__
class WebServerAppender final : public plog::IAppender
{
public:
   void write(const plog::Record &record) PLOG_OVERRIDE
   {
      time_t rawTime = record.getTime().time;
      struct tm timeInfo;
      #ifdef _WIN32
      localtime_s(&timeInfo, &rawTime);
      #else
      localtime_r(&rawTime, &timeInfo);
      #endif

      std::string level;
      switch (record.getSeverity()) {
         case plog::fatal:   level = "FATAL"; break;
         case plog::error:   level = "ERROR"; break;
         case plog::warning: level = "WARN"; break;
         case plog::info:    level = "INFO"; break;
         case plog::debug:   level = "DEBUG"; break;
         case plog::verbose: level = "VERBOSE"; break;
         default:            level = "UNKNOWN"; break;
      }

      std::string message;
      #ifdef _WIN32
      auto msg = record.getMessage();
      const int len = (int)wcslen(msg) + 1;
      char *const szT = new char[len];
      WideCharToMultiByteNull(CP_UTF8, 0, msg, -1, szT, len, nullptr, nullptr);
      message = std::string(szT);
      delete [] szT;
      #else
      message = std::string(record.getMessage());
      #endif

      char timeBuffer[32];
      snprintf(timeBuffer, sizeof(timeBuffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
               timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
               timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec,
               static_cast<int>(record.getTime().millitm));

      std::stringstream ss;
      ss << timeBuffer << " " << std::left << std::setw(5) << level << " ";
      ss << "[" << record.getTid() << "] ";
      ss << "[" << record.getFunc() << "@" << record.getLine() << "] ";
      ss << message;

      WebServer::LogAppender(ss.str());
   }
};
#endif

Logger* Logger::m_pInstance = nullptr;

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
         const string szLogPath = g_pvp->m_myPrefPath + "vpinball.log";
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
#ifdef __LIBVPINBALL__
         static WebServerAppender webServerAppender;
         plog::Logger<PLOG_DEFAULT_INSTANCE_ID>::getInstance()->addAppender(&webServerAppender);
         plog::Logger<PLOG_NO_DBG_OUT_INSTANCE_ID>::getInstance()->addAppender(&webServerAppender);
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
   plog::init<PLOG_NO_DBG_OUT_INSTANCE_ID>(); // Logger that does not show in the debug window to avoid duplicated messages
}

void Logger::Truncate()
{
   std::string szLogPath = g_pvp->m_myPrefPath + "vpinball.log";
   std::ofstream ofs(szLogPath, std::ofstream::out | std::ofstream::trunc);
   ofs.close();
}
