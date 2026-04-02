// license:GPLv3+

#pragma once

#include <filesystem>
#include <format>

#include <string>
using namespace std::string_literals;
using namespace std::string_view_literals;
using std::string;

#include "plugins/LoggingPlugin.h"

#include "plugins/VPXPlugin.h"

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

namespace Vni {

LPI_USE_CPP();
#define LOGD Vni::LPI_LOGD_CPP
#define LOGI Vni::LPI_LOGI_CPP
#define LOGW Vni::LPI_LOGW_CPP
#define LOGE Vni::LPI_LOGE_CPP

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
