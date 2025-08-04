#pragma once

#include "MsgPlugin.h"
#include "LoggingPlugin.h" 
#include "ScriptablePlugin.h"
#include "VPXPlugin.h"
#include "ControllerPlugin.h"

#include <string>
#include <memory>

using std::string;

#ifdef _WIN32
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

namespace WMP {

string normalize_path_separators(const string& szPath);
string find_case_insensitive_file_path(const string& szPath);
bool StrCompareNoCase(const string& strA, const string& strB);

LPI_USE();
#ifndef LOGD
#define LOGD(...) LPI_LOGD(__VA_ARGS__)
#define LOGI(...) LPI_LOGI(__VA_ARGS__)
#define LOGW(...) LPI_LOGI(__VA_ARGS__)
#define LOGE(...) LPI_LOGE(__VA_ARGS__)
#endif

PSC_USE_ERROR();

}
