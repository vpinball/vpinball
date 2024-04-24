#pragma once

#include <plog/Log.h>
#include <plog/Formatters/TxtFormatter.h>

#define PLOG_OMIT_LOG_DEFINES 
#define PLOG_NO_DBG_OUT_INSTANCE_ID 1

void InitLogger();
void SetupLogger(const bool enable);
