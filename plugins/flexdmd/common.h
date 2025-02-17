#pragma once

#include <cstdint>
#include <assert.h>

#include <string>
using namespace std::string_literals;
using std::string;

#include <vector>
using std::vector;

typedef uint32_t ColorRGBA32;
#define RGB(r, g, b) static_cast<ColorRGBA32>(static_cast<uint8_t>(r) | (static_cast<uint8_t>(g) << 8) | (static_cast<uint8_t>(b) << 16))
#define GetRValue(rgba32) static_cast<uint8_t>(rgba32)
#define GetGValue(rgba32) static_cast<uint8_t>((rgba32) >> 8)
#define GetBValue(rgba32) static_cast<uint8_t>((rgba32) >> 16)

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#define PATH_SEPARATOR_WCHAR L'\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#define PATH_SEPARATOR_WCHAR L'/'
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
string extension_from_path(const string& path);
string normalize_path_separators(const string& szPath);
int string_to_int(const string& str, int defaultValue = 0);
bool try_parse_int(const string& str, int& value);
bool try_parse_color(const string& str, ColorRGBA32& value);
