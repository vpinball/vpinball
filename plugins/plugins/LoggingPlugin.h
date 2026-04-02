// license:GPLv3+

#pragma once

#include "MsgPlugin.h"

///////////////////////////////////////////////////////////////////////////////
// Shared logging support for plugins
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// This interface is part of a work in progress and will evolve likely a lot
// before being considered stable. Do not use it, or if you do, use it knowing
// that you're plugin will be broken by the upcoming updates.
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// This file defines an API and helpers that allows plugin to access shared login
//

#define LOGPI_NAMESPACE                 "Login"      // Namespace
#define LOGPI_MSG_GET_API               "GetAPI"     // Get the plugin API

#define LPI_LVL_DEBUG      0x00
#define LPI_LVL_INFO       0x10
#define LPI_LVL_WARN       0x20
#define LPI_LVL_ERROR      0x40

#ifdef _DEBUG
#define LOGPI_DEFAULT_LEVEL LPI_LVL_DEBUG
#else
#define LOGPI_DEFAULT_LEVEL LPI_LVL_INFO
#endif

typedef struct LoggingPluginAPI
{
   void (MSGPIAPI *Log)(unsigned int level, const char* message);
} LoggingPluginAPI;



///////////////////////////////////////////////////////////////////////////////
//
// Helper macros
//
// The plugin source file must include one LPI_IMPLEMENT() or LPI_IMPLEMENT_CPP() statement 
// in one .c(pp) file, and call LPISetup(unsigned int endpointId, MsgPluginAPI* msgApi) during its loading.
//
// Other source files in the same component need to add a LPI_USE() or LPI_USE_CPP() statement to
// be able to use LPI_LOGD(_CPP) / LPI_LOGI(_CPP) / LPI_LOGW(_CPP) / LPI_LOGE(_CPP)
//

#ifdef __cplusplus
#include <string>
#include <sstream>

#define LPI_USE_CPP() extern void LPILog_CPP(const unsigned int level, const std::string& s); extern void LPILog_CPP(const unsigned int level, const std::stringstream& s)
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_DEBUG
#define LPI_LOGD_CPP(x) LPILog_CPP(LPI_LVL_DEBUG, x)
#else
#define LPI_LOGD_CPP(x) LPILog_CPP(LPI_LVL_DEBUG, ""s)
#endif
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_INFO
#define LPI_LOGI_CPP(x) LPILog_CPP(LPI_LVL_INFO, x)
#else
#define LPI_LOGI_CPP(x) LPILog_CPP(LPI_LVL_INFO, ""s)
#endif
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_WARN
#define LPI_LOGW_CPP(x) LPILog_CPP(LPI_LVL_WARN, x)
#else
#define LPI_LOGW_CPP(x) LPILog_CPP(LPI_LVL_WARN, ""s)
#endif
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_ERROR
#define LPI_LOGE_CPP(x) LPILog_CPP(LPI_LVL_ERROR, x)
#else
#define LPI_LOGE_CPP(x) LPILog_CPP(LPI_LVL_ERROR, ""s)
#endif

#define LPI_IMPLEMENT_CPP \
   LoggingPluginAPI* loggingApi = nullptr; \
   void LPILog_CPP(const unsigned int level, const std::string& s) { \
      if (loggingApi != nullptr && !s.empty()) { \
         loggingApi->Log(level, s.c_str()); \
      } \
   } \
   void LPILog_CPP(const unsigned int level, const std::stringstream& s) { \
      if (loggingApi != nullptr && !s.str().empty()) { \
         loggingApi->Log(level, s.str().c_str()); \
      } \
   } \
   void LPISetup(unsigned int endpointId, const MsgPluginAPI* msgApi) { \
      const unsigned int getLoggingApiId = msgApi->GetMsgID(LOGPI_NAMESPACE, LOGPI_MSG_GET_API); \
      msgApi->BroadcastMsg(endpointId, getLoggingApiId, &loggingApi); \
      msgApi->ReleaseMsgID(getLoggingApiId); \
   }
#endif


#define LPI_USE() extern void LPILog(unsigned int level, const char* format, ...)
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_DEBUG 
#define LPI_LOGD(...) LPILog(LPI_LVL_DEBUG, __VA_ARGS__)
#else
#define LPI_LOGD(...)
#endif
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_INFO
#define LPI_LOGI(...) LPILog(LPI_LVL_INFO, __VA_ARGS__)
#else
#define LPI_LOGI(...)
#endif
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_WARN
#define LPI_LOGW(...) LPILog(LPI_LVL_WARN, __VA_ARGS__)
#else
#define LPI_LOGW(...)
#endif
#if LOGPI_DEFAULT_LEVEL <= LPI_LVL_ERROR
#define LPI_LOGE(...) LPILog(LPI_LVL_ERROR, __VA_ARGS__)
#else
#define LPI_LOGE(...)
#endif

#define LPI_IMPLEMENT \
   LoggingPluginAPI* loggingApi = nullptr; \
   void LPILog(unsigned int level, const char* format, ...) { \
      if (loggingApi != nullptr) { \
         va_list args; \
         va_start(args, format); \
         va_list args_copy; \
         va_copy(args_copy, args); \
         int size = vsnprintf(nullptr, 0, format, args_copy); \
         va_end(args_copy); \
         if (size > 0) { \
            char* buffer = static_cast<char*>(malloc(size + 1)); \
            vsnprintf(buffer, size + 1, format, args); \
            loggingApi->Log(level, buffer); \
            free(buffer); \
         } \
         va_end(args); \
      } \
   } \
   void LPISetup(unsigned int endpointId, const MsgPluginAPI* msgApi) { \
      const unsigned int getLoggingApiId = msgApi->GetMsgID(LOGPI_NAMESPACE, LOGPI_MSG_GET_API); \
      msgApi->BroadcastMsg(endpointId, getLoggingApiId, &loggingApi); \
      msgApi->ReleaseMsgID(getLoggingApiId); \
   }
