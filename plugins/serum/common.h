#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>

#include <string>
using std::string;

// Shared logging
#include "LoggingPlugin.h"
LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

string find_case_insensitive_directory_path(const string& szPath);