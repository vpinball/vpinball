// license:GPLv3+

#pragma once

#include "plugins/MsgPlugin.h"
#include "plugins/LoggingPlugin.h" 
#include "plugins/ScriptablePlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"

#include <string>
using namespace std::string_literals;
using namespace std::string_view_literals;
using std::string;
#include <format>

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

LPI_USE_CPP();
#ifndef LOGD
#define LOGD WMP::LPI_LOGD_CPP
#define LOGI WMP::LPI_LOGI_CPP
#define LOGW WMP::LPI_LOGW_CPP
#define LOGE WMP::LPI_LOGE_CPP
#endif

PSC_USE_ERROR();

}
