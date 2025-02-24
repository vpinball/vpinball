#pragma once

#include "libpinmame.h"

#include <string>
using std::string;

#include <vector>
using std::vector;

// Shared logging
#include "LoggingPlugin.h"
LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

// Scriptable API
#include "ScriptablePlugin.h"
PSC_USE_ERROR();

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_WCHAR L'\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_WCHAR L'/'
#endif

string find_directory_case_insensitive(const std::string& szParentPath, const std::string& szDirName);
