// license:GPLv3+

#pragma once

#include <cassert>

#include <string>
using std::string;
using namespace std::string_literals;

#include <vector>
using std::vector;

#include <sstream>

#include <cstring>
#include <cstdint>
#include <cstdarg>
#include <cstdlib>
#include <memory> // needed for std::shared_ptr on Linux
#include <functional>
#include <filesystem>

// Shared logging
#include "plugins/LoggingPlugin.h"

// Scriptable API
#include "plugins/ScriptablePlugin.h"

// VPX main API
#include "plugins/VPXPlugin.h"

namespace B2S
{

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGW LPI_LOGW
#define LOGE LPI_LOGE

PSC_USE_ERROR();

class vec4 final
{
public:
   constexpr vec4() { }
   constexpr vec4(float px, float py, float pz, float pw) : x(px), y(py), z(pz), w(pw) { }

   float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};

class ivec4 final
{
public:
   constexpr ivec4() { }
   constexpr ivec4(int px, int py, int pz, int pw) : x(px), y(py), z(pz), w(pw) { }

   int x = 0, y = 0, z = 0, w = 0;
};

// Rendering provided through plugin messages
extern VPXTexture CreateTexture(uint8_t *rawData, int size);
extern VPXTextureInfo* GetTextureInfo(VPXTexture texture);
extern void DeleteTexture(VPXTexture texture);
extern void UpdateTexture(VPXTexture *texture, int width, int height, VPXTextureFormat format, const void *image);

// The following function are duplicates from the main VPX codebase
string string_to_lower(string str);
std::filesystem::path find_case_insensitive_file_path(const std::filesystem::path &searchedFile);
vector<uint8_t> base64_decode(const char * const __restrict value, const size_t size_bytes);

}
