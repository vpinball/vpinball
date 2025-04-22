#pragma once

#include <cassert>
#include <cstdarg>
#include <cstdio>

#include <string>
using namespace std::string_literals;
using std::string;

#include <vector>
using std::vector;

// Shared logging
#include "LoggingPlugin.h"
LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

// Scriptable API
#include "ScriptablePlugin.h"
PSC_USE_ERROR();

typedef uint32_t ColorRGBA32;
#define RGB(r, g, b) static_cast<ColorRGBA32>(static_cast<uint8_t>(r) | (static_cast<uint8_t>(g) << 8) | (static_cast<uint8_t>(b) << 16))
#define GetRValue(rgba32) static_cast<uint8_t>(rgba32)
#define GetGValue(rgba32) static_cast<uint8_t>((rgba32) >> 8)
#define GetBValue(rgba32) static_cast<uint8_t>((rgba32) >> 16)

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