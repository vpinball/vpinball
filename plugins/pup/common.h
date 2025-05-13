#pragma once

#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <cstdlib>

#include <thread>

#include <string>
using std::string;
using namespace std::string_literals;

#include <vector>
using std::vector;

#include <sstream>

#include <memory> // needed for std::shared_ptr on Linux
#include <filesystem>
#include <fstream>

#include <unordered_dense.h>

#include <SDL3/SDL_surface.h>

#define LONG long

#ifndef GetRValue
#define GetRValue(rgba32) static_cast<uint8_t>(rgba32)
#define GetGValue(rgba32) static_cast<uint8_t>((rgba32) >> 8)
#define GetBValue(rgba32) static_cast<uint8_t>((rgba32) >> 16)
#endif


// Shared logging
#include "LoggingPlugin.h"
LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

// Scriptable API
#include "ScriptablePlugin.h"
PSC_USE_ERROR();

// VPX main API
#include "VPXPlugin.h"


#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

#if defined(__GNUC__) && (__GNUC__ < 12)
#define CONSTEXPR
#else
#define CONSTEXPR constexpr
#endif

// Rendering provided through plugin messages
extern VPXTexture CreateTexture(SDL_Surface* surf);
extern VPXTexture CreateTexture(uint8_t *rawData, int size);
extern void GetTextureInfo(VPXTexture texture, int *width, int *height);
extern void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, uint8_t *image);
extern void DeleteTexture(VPXTexture texture);

// The following function are duplicates from the main VPX codebase
int string_to_int(const string &str, int default_value = 0);
float string_to_float(const string &str, float default_value = 0.0f);
vector<string> parse_csv_line(const string &line);
string string_replace_all(const string &szStr, const string &szFrom, const string &szTo, const size_t offs = 0);
string extension_from_path(const string &path);
string normalize_path_separators(const string &szPath);
string find_case_insensitive_directory_path(const string &szPath);
string find_case_insensitive_file_path(const string &szPath);
bool StrCompareNoCase(const string &strA, const string &strB);
string lowerCase(string input);
void SetThreadName(const string &name);
