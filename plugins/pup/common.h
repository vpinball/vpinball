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
#include "LoggingPlugin.h"

// Scriptable API
#include "ScriptablePlugin.h"

// Controller API
#include "ControllerPlugin.h"

// VPX main API
#include "VPXPlugin.h"

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
extern void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const uint8_t *image);
extern void DeleteTexture(VPXTexture texture);

extern CtlResId UpdateAudioStream(AudioUpdateMsg *msg);
extern void StopAudioStream(const CtlResId& id);

class AsyncCallback
{
public:
   AsyncCallback(vector<AsyncCallback*>& pendingList, std::mutex& pendingListMutex, std::function<void()> callback)
      : m_pendingList(pendingList)
      , m_pendingListMutex(pendingListMutex)
      , m_callback(callback)
   {
   }

   void DispatchOnMainThread(MsgPluginAPI* msgApi)
   {
      std::lock_guard<std::mutex> lock(m_pendingListMutex);
      m_pendingList.push_back(this);
      msgApi->RunOnMainThread(0, AsyncCallback::ProcessCallback, this);
   }

   void Invalidate() { m_valid = false; }

   static void DispatchOnMainThread(MsgPluginAPI* msgApi, vector<AsyncCallback*>& pendingList, std::mutex& pendingListMutex, std::function<void()> callback)
   {
      AsyncCallback* cb = new AsyncCallback(pendingList, pendingListMutex, callback);
      cb->DispatchOnMainThread(msgApi);
   }

   // Invalidate pending triggers as their execution context is not valid any more
   static void InvalidateAllPending(vector<AsyncCallback*>& pendingList, std::mutex& pendingListMutex)
   {
      std::lock_guard<std::mutex> lock(pendingListMutex);
      std::for_each(pendingList.begin(), pendingList.end(), [](AsyncCallback* cb) { cb->Invalidate(); });
   }

   static void ProcessCallback(void* userdata)
   {
      AsyncCallback* tcb = static_cast<AsyncCallback*>(userdata);
      if (tcb->m_valid)
      {
         std::unique_lock<std::mutex> lock(tcb->m_pendingListMutex);
         auto it = std::ranges::find(tcb->m_pendingList, tcb);
         if (it != tcb->m_pendingList.end())
            tcb->m_pendingList.erase(it);
         lock.unlock();
         tcb->m_callback();
      }
      delete tcb;
   }

private:
   bool m_valid = true;
   vector<AsyncCallback*>& m_pendingList;
   std::mutex& m_pendingListMutex;
   std::function<void()> m_callback;
};

string trim_string(const string &str);

// The following function are duplicates from the main VPX codebase
int string_to_int(const string &str, int default_value = 0);
float string_to_float(const string &str, float default_value = 0.0f);
vector<string> parse_csv_line(const string &line);
string string_replace_all(const string &szStr, const string &szFrom, const string &szTo, const size_t offs = 0);
string extension_from_path(const string &path);
string normalize_path_separators(const string &szPath);
string find_case_insensitive_file_path(const string &szPath);
string find_case_insensitive_directory_path(const string &szPath);
bool StrCompareNoCase(const string &strA, const string &strB);
string lowerCase(string input);
void SetThreadName(const string &name);

}
