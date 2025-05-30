#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>

#ifndef _WIN32
#include <dlfcn.h>
#include <limits.h>
#include <unistd.h>
#endif

#include <string>
using namespace std::string_literals;
using std::string;

#include <vector>
using std::vector;

// Shared logging
#include "LoggingPlugin.h"

// VPX main API
#include "VPXPlugin.h"

namespace ScoreView
{

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

template <typename T> constexpr __forceinline T clamp(const T x, const T mn, const T mx) { return std::max(std::min(x, mx), mn); }

string TrimLeading(const string& str, const string& whitespace);
string TrimTrailing(const string& str, const string& whitespace);
bool try_parse_float(const string& str, float& value);
bool try_parse_int(const string& str, int& value);
string find_case_insensitive_file_path(const string& szPath);
string PathFromFilename(const string& szfilename);
#ifndef _WIN32
string GetLibraryPath();
#endif
}