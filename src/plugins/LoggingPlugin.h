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
#define LPI_LVL_ERROR      0x20

typedef struct LoggingPluginAPI
{
   void (MSGPIAPI *Log)(unsigned int level, const char* message);
} LoggingPluginAPI;



///////////////////////////////////////////////////////////////////////////////
//
// Helper macros
//
// The plugin source file must include a LPI_IMPLEMENT() statement and call
// LPISetup(unsigned int endpointId, MsgPluginAPI* msgApi) during its loading.
//
// Other source files in the same component need to add LPI_USE() statement to
// be able to use LPI_DEBUG / LPI_INFO / LPI_ERROR
//

#define LPI_USE() extern void LPILog(unsigned int level, const char* format, ...)
#define LPI_LOGD(...) LPILog(LPI_LVL_DEBUG, __VA_ARGS__)
#define LPI_LOGI(...) LPILog(LPI_LVL_INFO, __VA_ARGS__)
#define LPI_LOGE(...) LPILog(LPI_LVL_ERROR, __VA_ARGS__)

#define LPI_IMPLEMENT \
   LoggingPluginAPI* loggingApi = nullptr; \
   void LPILog(unsigned int level, const char* format, ...) { \
      if (loggingApi != nullptr) { \
         va_list args; \
         va_start(args, format); \
         int size = vsnprintf(NULL, 0, format, args); \
         if (size > 0) { \
            char* buffer = static_cast<char*>(malloc(size + 1)); \
            vsnprintf(buffer, size + 1, format, args); \
            loggingApi->Log(level, buffer); \
            free(buffer); \
         } \
         va_end(args); \
      } \
   } \
   void LPISetup(unsigned int endpointId, MsgPluginAPI* msgApi) { \
      const unsigned int getLoggingApiId = msgApi->GetMsgID(LOGPI_NAMESPACE, LOGPI_MSG_GET_API); \
      msgApi->BroadcastMsg(endpointId, getLoggingApiId, &loggingApi); \
      msgApi->ReleaseMsgID(getLoggingApiId); \
   }
