#pragma once

// Shared logging
#include "plugins/LoggingPlugin.h"

// Scriptable API
#include "plugins/ScriptablePlugin.h"

// Controller plugin message support
#include "plugins/ControllerPlugin.h"

#include <string>
#include <cstdarg>
#include <filesystem>

using std::string;

namespace AltSound {

std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile);
bool StrCompareNoCase(const string& strA, const string& strB);

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGW LPI_LOGW
#define LOGE LPI_LOGE

PSC_USE_ERROR();

}