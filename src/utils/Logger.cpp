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
      const string szT = MakeString(msg);
      table->m_pcv->AddToDebugOutput(szT);
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
         case plog::fatal:   level = "FATAL"s; break;
         case plog::error:   level = "ERROR"s; break;
         case plog::warning: level = "WARN"s; break;
         case plog::info:    level = "INFO"s; break;
         case plog::debug:   level = "DEBUG"s; break;
         case plog::verbose: level = "VERBOSE"s; break;
         default:            level = "UNKNOWN"s; break;
      }

      std::string message;
      #ifdef _WIN32
      auto msg = record.getMessage();
      const int len = WideCharToMultiByte(CP_UTF8, 0, msg, -1, nullptr, 0, nullptr, nullptr); //(int)wcslen(msg) + 1;
      if (len > 1)
      {
         message.resize(len - 1, '\0');
         WideCharToMultiByte(CP_UTF8, 0, msg, -1, message.data(), len, nullptr, nullptr);
      }
      #else
      message = std::string(record.getMessage());
      #endif

      char timeBuffer[32];
      snprintf(timeBuffer, sizeof(timeBuffer), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
               timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday,
               timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec,
               static_cast<int>(record.getTime().millitm));

      std::stringstream ss;
      ss << timeBuffer << ' ' << std::left << std::setw(5) << level << ' ';
      ss << '[' << record.getTid() << "] ";
      ss << '[' << record.getFunc() << '@' << record.getLine() << "] ";
      ss << message;

      WebServer::LogAppender(ss.str());
   }
};
#endif

template <bool useUtcTime> class ThreadAwareTxtFormatter
{
public:
   static plog::util::nstring header() { return plog::util::nstring(); }

   static plog::util::nstring format(const plog::Record& record)
   {
      tm t;
      useUtcTime ? plog::util::gmtime_s(&t, &record.getTime().time) : plog::util::localtime_s(&t, &record.getTime().time);

      plog::util::nostringstream ss;
      ss << t.tm_year + 1900 << "-" << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mon + 1 << PLOG_NSTR("-") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_mday
         << PLOG_NSTR(" ");
      ss << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_hour << PLOG_NSTR(":") << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_min << PLOG_NSTR(":")
         << std::setfill(PLOG_NSTR('0')) << std::setw(2) << t.tm_sec << PLOG_NSTR(".") << std::setfill(PLOG_NSTR('0')) << std::setw(3) << static_cast<int>(record.getTime().millitm)
         << PLOG_NSTR(" ");
      ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
      #ifdef _WIN32
         bool logged = false;
         HANDLE hThread = OpenThread(THREAD_QUERY_INFORMATION, FALSE, record.getTid());
         if (hThread != nullptr)
         {
            PWSTR data;
            HRESULT hr = GetThreadDescription(hThread, &data);
            if (SUCCEEDED(hr))
            {
               if (data[0] != 0)
               {
                  ss << PLOG_NSTR("[") << data << PLOG_NSTR("] ");
                  logged = true;
               }
               LocalFree(data);
            }
            CloseHandle(hThread);
         }
         if (!logged)
            ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
      #else
         ss << PLOG_NSTR("[") << record.getTid() << PLOG_NSTR("] ");
      #endif
      ss << PLOG_NSTR("[") << record.getFunc() << PLOG_NSTR("@") << record.getLine() << PLOG_NSTR("] ");
      ss << record.getMessage() << PLOG_NSTR("\n");

      return ss.str();
   }
};

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
         static plog::RollingFileAppender<ThreadAwareTxtFormatter<false>> fileAppender(szLogPath.c_str(), 1024 * 1024 * 5, 1);
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
