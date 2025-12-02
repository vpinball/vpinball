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

#ifdef _MSC_VER
#define PATH_SEPARATOR_CHAR '\\'
#else
#define PATH_SEPARATOR_CHAR '/'
#endif

class vec2
{
public:
   vec2() { }
   vec2(float px, float py) : x(px), y(py) { }

   float x = 0.f, y = 0.f;
};

class vec4
{
public:
   vec4() { }
   vec4(float px, float py, float pz, float pw) : x(px), y(py), z(pz), w(pw) { }

   float x = 0.f, y = 0.f, z = 0.f, w = 0.f;
};

class ivec4
{
public:
   ivec4() { }
   ivec4(int px, int py, int pz, int pw) : x(px), y(py), z(pz), w(pw) { }

   int x = 0, y = 0, z = 0, w = 0;
};

// Rendering provided through plugin messages
extern VPXTexture CreateTexture(uint8_t *rawData, int size);
extern VPXTextureInfo* GetTextureInfo(VPXTexture texture);
extern void DeleteTexture(VPXTexture texture);
extern void UpdateTexture(VPXTexture *texture, int width, int height, VPXTextureFormat format, const void *image);

// The following function are duplicates from the main VPX codebase
string find_case_insensitive_file_path(const string &szPath);
string TitleAndPathFromFilename(const string &filename);
vector<unsigned char> base64_decode(string encoded_string);

}
