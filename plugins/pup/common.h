// license:GPLv3+

#pragma once

#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <cstdlib>

#include <thread>
#include <mutex>

#include <algorithm>
#include <ranges>

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

// Shared logging
#include "plugins/LoggingPlugin.h"

// Scriptable API
#include "plugins/ScriptablePlugin.h"

// Controller API
#include "plugins/ControllerPlugin.h"

// VPX main API
#include "plugins/VPXPlugin.h"

namespace PUP
{

LPI_USE();
#define LOGD PUP::LPI_LOGD
#define LOGI PUP::LPI_LOGI
#define LOGW PUP::LPI_LOGW
#define LOGE PUP::LPI_LOGE

#ifdef _DEBUG
   #define NOT_IMPLEMENTED(...) { assert(false); LOGE(__VA_ARGS__); }
#else
   #define NOT_IMPLEMENTED(...) LOGE(__VA_ARGS__)
#endif

PSC_USE_ERROR();

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

template <typename T> constexpr T clamp(const T x, const T mn, const T mx) { return x < mn ? mn : x > mx ? mx : x; }
template <typename T> constexpr T lerp(const T x1, const T x2, const float alpha) { return (1.f - alpha) * x1 + alpha * x2; }


// Rendering provided through plugin messages
extern VPXTexture CreateTexture(SDL_Surface* surf);
extern VPXTextureInfo* GetTextureInfo(VPXTexture texture);
extern void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void *image);
extern void DeleteTexture(VPXTexture texture);

extern CtlResId UpdateAudioStream(AudioUpdateMsg *msg);
extern void StopAudioStream(const CtlResId& id);

string trim_string(const string &str);

// The following function are duplicates from the main VPX codebase

// trims leading whitespace or similar
int string_to_int(const string &str, int defaultValue = 0);
// trims leading whitespace or similar
float string_to_float(const string &str, float defaultValue = 0.0f);
vector<string> parse_csv_line(const string &line);
string string_replace_all(const string &szStr, const string &szFrom, const string &szTo, const size_t offs = 0);
string string_replace_all(const string &szStr, const string &szFrom, const char szTo, const size_t offs = 0);
string extension_from_path(const string &path);
string normalize_path_separators(const string &szPath);
std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path &searchedFile);
std::filesystem::path find_case_insensitive_directory_path(const std::filesystem::path &searchedFile);
bool StrCompareNoCase(const string &strA, const string &strB);
string lowerCase(string input);
std::filesystem::path lowerCase(std::filesystem::path input);
void SetThreadName(const string &name);

}
