#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "libpinmame.h"

// Shared logging
#include "LoggingPlugin.h"

// Scriptable API
#include "ScriptablePlugin.h"

namespace PinMAME {

LPI_USE();
#define LOGD PinMAME::LPI_LOGD
#define LOGI PinMAME::LPI_LOGI
#define LOGE PinMAME::LPI_LOGE

PSC_USE_ERROR();

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

string find_case_insensitive_directory_path(const string& szPath);

}