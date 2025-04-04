// license:GPLv3+

#pragma once

#define PLOG_OMIT_LOG_DEFINES 
#define PLOG_NO_DBG_OUT_INSTANCE_ID 1
#include <plog/Log.h>

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
