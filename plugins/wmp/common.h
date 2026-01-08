// license:GPLv3+

#pragma once

#include "plugins/MsgPlugin.h"
#include "plugins/LoggingPlugin.h" 
#include "plugins/ScriptablePlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"

#include <string>
using namespace std::string_literals;
using std::string;

#include <memory>

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
#define LOGW(...) LPI_LOGW(__VA_ARGS__)
#define LOGE(...) LPI_LOGE(__VA_ARGS__)
#endif

PSC_USE_ERROR();

}
