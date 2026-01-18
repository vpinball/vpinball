// license:GPLv3+

#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <tchar.h>
#include <locale>
#include <codecvt>
#endif

#ifndef _WIN32
#include <dlfcn.h>
#include <climits>
#include <unistd.h>
#endif

#include <string>
using namespace std::string_literals;
using std::string;

#include <vector>
using std::vector;

// Shared logging
#include "plugins/LoggingPlugin.h"

// VPX main API
#include "plugins/VPXPlugin.h"

namespace ScoreView
{

LPI_USE();
#ifndef LOGD
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGW LPI_LOGW
#define LOGE LPI_LOGE
#endif

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
string PathFromFilename(const string& filename);
string GetPluginPath();

}
