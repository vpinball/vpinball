// license:GPLv3+

#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <filesystem>

#include <string>
using namespace std::string_literals;
using std::string;

// Shared logging
#include "plugins/LoggingPlugin.h"

// VPX main API
#include "plugins/VPXPlugin.h"

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
std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path& searchedFile);

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
