#pragma once

//#include "plugins/MsgPlugin.h"

// Shared logging
#include "plugins/LoggingPlugin.h"

// Scriptable API
#include "plugins/ScriptablePlugin.h"

// VPX main API
#include "plugins/VPXPlugin.h"

// Controller plugin message support
#include "plugins/ControllerPlugin.h"

#include <string>
#include <memory>
#include <cstdarg>

using std::string;

namespace AltSound {

string normalize_path_separators(const string& szPath);
string find_case_insensitive_file_path(const string& szPath);
bool StrCompareNoCase(const string& strA, const string& strB);

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGW LPI_LOGW
#define LOGE LPI_LOGE

PSC_USE_ERROR();

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

}