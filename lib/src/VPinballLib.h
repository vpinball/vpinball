// license:GPLv3+

#pragma once

#include <string>
#include <filesystem>
#include <functional>
#include <queue>
#include <mutex>
#include "../include/vpinball/VPinballLib_C.h"
#include "WebServer.h"
#include "core/vpversion.h"

namespace VPinballLib {

using std::string;
using std::vector;

struct ProgressData {
   int progress;
};

struct RumbleData {
   uint16_t lowFrequencyRumble;
   uint16_t highFrequencyRumble;
   uint32_t durationMs;
};

struct ScriptErrorData {
   VPINBALL_SCRIPT_ERROR_TYPE error;
   int line;
   int position;
   string description;
};

struct WebServerData {
   string url;
};

struct CommandData {
   string command;
   string data;
};

class VPinballLib
{
public:
   static VPinballLib& Instance()
   {
      static VPinballLib inst;
      return inst;
   }

   int AppInit(int argc, char** argv);
   void AppIterate();
   void AppEvent(SDL_Event* event);
   bool PollAppEvent(SDL_Event& event);
   SDL_Window* GetWindow() { return m_pWindow; }
#ifdef __APPLE__
   void* GetMetalLayer() { return m_pMetalLayer; }
   void SetMetalLayer(void* layer) { m_pMetalLayer = layer; }
#endif
   string GetVersionStringFull() { return VP_VERSION_STRING_FULL_LITERAL; };
   void Init(VPinballEventCallback callback);
   static void SendEvent(VPINBALL_EVENT event, void* data);
   void Log(VPINBALL_LOG_LEVEL level, const string& message);
   void ResetLog();
   int LoadValueInt(const string& sectionName, const string& key, int defaultValue);
   void SaveValueInt(const string& sectionName, const string& key, int value);
   float LoadValueFloat(const string& sectionName, const string& key, float defaultValue);
   void SaveValueFloat(const string& sectionName, const string& key, float value);
   string LoadValueString(const string& sectionName, const string& key, const string& defaultValue);
   void SaveValueString(const string& sectionName, const string& key, const string& value);
   bool LoadValueBool(const string& sectionName, const string& key, bool defaultValue);
   void SaveValueBool(const string& sectionName, const string& key, bool value);
   VPINBALL_STATUS ResetIni();
   void UpdateWebServer();
   std::filesystem::path GetPath(VPINBALL_PATH pathType);
   VPINBALL_STATUS LoadTable(const string& tablePath);
   VPINBALL_STATUS ExtractTableScript();
   VPINBALL_STATUS Play();
   VPINBALL_STATUS Stop();
   void SetGameLoop(std::function<void()> gameLoop) { m_gameLoop = gameLoop; }

private:
   VPinballLib();
   ~VPinballLib();
   VPinballLib(const VPinballLib&) = delete;
   VPinballLib& operator=(const VPinballLib&) = delete;
   void SetEventCallback(VPinballEventCallback callback);
   static void RegisterStaticPlugins();

   SDL_Window* m_pWindow = nullptr;
#ifdef __APPLE__
   void* m_pMetalLayer = nullptr;
#endif
   WebServer m_webServer;
   std::function<void*(VPINBALL_EVENT, void*)> m_eventCallback = nullptr;
   std::function<void()> m_gameLoop = nullptr;
   std::queue<SDL_Event> m_eventQueue;
   std::mutex m_eventMutex;
   bool m_captureInProgress = false;
};

}
