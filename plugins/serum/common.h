#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>

#include <string>
using std::string;

// Shared logging
#include "LoggingPlugin.h"

// VPX main API
#include "VPXPlugin.h"

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

namespace Serum {

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGW LPI_LOGW
#define LOGE LPI_LOGE

void SetThreadName(const std::string& name);
string find_case_insensitive_directory_path(const string& szPath);

}