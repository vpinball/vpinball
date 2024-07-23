#pragma once

#define PLOG_OMIT_LOG_DEFINES 
#define PLOG_NO_DBG_OUT_INSTANCE_ID 1
#include <plog/Log.h>

class Logger final
{
public:
   ~Logger() {}

   static Logger* GetInstance();

   void Init();
   void SetupLogger(const bool enable);

private:
   Logger() {}

   static Logger* m_pInstance;
};