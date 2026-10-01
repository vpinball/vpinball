// license:GPLv3+

#pragma once

#define PLOG_OMIT_LOG_DEFINES 
#define PLOG_NO_DBG_OUT_INSTANCE_ID 1
#include <plog/Log.h>

// Log messages are UTF-8 (on Windows through a build definition, as it must match in all sources using plog)
static_assert(PLOG_CHAR_IS_UTF8, "PLOG_CHAR_IS_UTF8 must be defined to 1 for all sources");

class Logger final
{
public:
   ~Logger() {}

   static Logger* GetInstance();

   static void Init();
   static void SetupLogger(const bool enable);
   static void Truncate();

private:
   Logger() {}

   static Logger* m_pInstance;
};
