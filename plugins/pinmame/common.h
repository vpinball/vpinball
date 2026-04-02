// license:GPLv3+

#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <format>

#include <string>
using namespace std::string_literals;
using namespace std::string_view_literals;
using std::string;

#include <vector>
using std::vector;

#include "libpinmame.h"

// Shared logging
#include "plugins/LoggingPlugin.h"

// Scriptable API
#include "plugins/ScriptablePlugin.h"

namespace PinMAME {

LPI_USE_CPP();
#define LOGD PinMAME::LPI_LOGD_CPP
#define LOGI PinMAME::LPI_LOGI_CPP
#define LOGW PinMAME::LPI_LOGW_CPP
#define LOGE PinMAME::LPI_LOGE_CPP

PSC_USE_ERROR();

std::filesystem::path find_case_insensitive_directory_path(const std::filesystem::path& searchedFile);

   // copies all characters of src incl. the null-terminator, BUT never more than dest_size-1, always null-terminates
inline void strncpy_s(char* const __restrict dest, const size_t dest_size, const char* const __restrict src)
{
   if (!dest || dest_size == 0)
      return;
   size_t i = 0;
   if (src)
      for (; i < dest_size-1 && src[i] != '\0'; ++i)
         dest[i] = src[i];
   dest[i] = '\0';
}

}
