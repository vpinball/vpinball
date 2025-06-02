#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
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
#include "LoggingPlugin.h"

// Scriptable API
#include "ScriptablePlugin.h"

namespace Flex
{

PSC_USE_ERROR();

LPI_USE();
#define LOGD Flex::LPI_LOGD
#define LOGI Flex::LPI_LOGI
#define LOGE Flex::LPI_LOGE

typedef uint32_t ColorRGBA32;
#ifndef RGB
#define RGB(r, g, b) (static_cast<ColorRGBA32>(r) | (static_cast<ColorRGBA32>(g) << 8) | (static_cast<ColorRGBA32>(b) << 16))
#endif
#ifndef GetRValue
#define GetRValue(rgba32) static_cast<uint8_t>(rgba32)
#define GetGValue(rgba32) static_cast<uint8_t>((rgba32) >> 8)
#define GetBValue(rgba32) static_cast<uint8_t>((rgba32) >> 16)
#endif

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
template <typename T> __forceinline T min(const T x, const T y) { return x < y ? x : y; }
template <typename T> __forceinline T max(const T x, const T y) { return x < y ? y : x; }

string string_to_lower(string str);
string trim_string(const string& str);
int string_to_int(const string& str, int defaultValue = 0);
bool try_parse_int(const string& str, int& value);
bool try_parse_color(const string& str, ColorRGBA32& value);
string normalize_path_separators(const string& szPath);
string extension_from_path(const string& path);
string find_case_insensitive_file_path(const string& szPath);
string GetPluginPath();

}