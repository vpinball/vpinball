// license:GPLv3+

#pragma once

// This file provides a default C++ implementation of a plugin manager for the 'Generic Message Plugin API'.
// 
// It has 2 strong dependencies beside C++ standard libraries:
// - mINI from https://github.com/metayeti/mINI
// - PLOG from https://github.com/SergiusTheBest/plog

#include "MsgPlugin.h"

#include <chrono>
#include <list>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>
#include <filesystem>

namespace MsgPI
{

typedef void (*msgpi_load_plugin)(const uint32_t pluginId, const MsgPluginAPI* api);
typedef void (*msgpi_unload_plugin)();

class MsgModuleLoader
{
public:
   virtual ~MsgModuleLoader() { }
   virtual void* Link(const std::string& directory, const std::string& file) = 0;
   virtual void Unlink(void* module) = 0;
   virtual void* GetFunction(void* module, const std::string& functionName) = 0;
};

class MsgPlugin final
{
public:
   MsgPlugin(const std::string& id, const std::string& name, const std::string& description, const std::string& author, const std::string& version, const std::string& link,
      std::shared_ptr<MsgModuleLoader> loader, const std::string& directory, const std::string& library, const unsigned int endpointId)
      : m_id(id)
      , m_name(name)
      , m_description(description)
      , m_author(author)
      , m_version(version)
      , m_link(link)
      , m_library(library)
      , m_directory(directory)
      , m_endpointId(endpointId)
      , m_loadPlugin(nullptr)
      , m_unloadPlugin(nullptr)
      , m_loader(loader) { }
   MsgPlugin(const std::string& id, const std::string& name, const std::string& description, const std::string& author, const std::string& version, const std::string& link,
      msgpi_load_plugin loadPlugin, msgpi_unload_plugin unloadPlugin, const unsigned int endpointId)
      : m_id(id)
      , m_name(name)
      , m_description(description)
      , m_author(author)
      , m_version(version)
      , m_link(link)
      , m_library()
      , m_directory()
      , m_endpointId(endpointId)
      , m_loadPlugin(loadPlugin)
      , m_unloadPlugin(unloadPlugin) { }
   ~MsgPlugin();

   void Load(const MsgPluginAPI* msgAPI);
   void Unload();
   bool IsLoaded() const { return m_msgAPI != nullptr; }

   const std::string m_id; // Unique ID of the plugin, used to identify it
   const std::string m_name; // Human-readable name of the plugin
   const std::string m_description; // Human-readable description of the plugin intent
   const std::string m_author; // Human-readable author name
   const std::string m_version; // Human-readable version
   const std::string m_link; // Web link to online information

   bool IsDynamicallyLinked() const { return m_loader != nullptr; }
   const std::string m_directory; // Directory containing this plugin
   const std::string m_library; // Library implementing this plugin for the current platform

   const uint32_t m_endpointId; // Unique 'end point' ID of the plugin, used to identify it for the lifetime of this session

private:
   msgpi_load_plugin m_loadPlugin;
   msgpi_unload_plugin m_unloadPlugin;
   void* m_module = nullptr;
   const std::shared_ptr<MsgModuleLoader> m_loader;
   const MsgPluginAPI* m_msgAPI = nullptr;
};

class MsgPluginManager final
{
public:
   static MsgPluginManager& GetInstance();
   ~MsgPluginManager();

   const MsgPluginAPI& GetMsgAPI() const { return m_api; }

   std::shared_ptr<MsgPlugin> RegisterPlugin(const std::string& id, const std::string& name, const std::string& description, const std::string& author, const std::string& version, const std::string& link, msgpi_load_plugin loadPlugin, msgpi_unload_plugin unloadPlugin);
   void ScanPluginFolder(std::shared_ptr<MsgModuleLoader> loader, const std::filesystem::path& pluginDir, const std::function<void(MsgPlugin&)>& callback);
   std::shared_ptr<MsgPlugin> GetPlugin(const std::string& pluginId) const;
   const std::vector<std::shared_ptr<MsgPlugin>> GetPlugins() const { return m_plugins; }
   void LoadPlugin(MsgPlugin& plugin);
   void UnloadPlugin(MsgPlugin& plugin);
   void UnloadPlugins();

   enum class SettingAction
   {
      Load,
      Save,
      UnregisterAll
   };
   void SetSettingsHandler(const std::function<void(const std::string& pluginId, SettingAction action, MsgSettingDef* settingDef)>& handler) { m_settingHandler = handler; }

   void ProcessAsyncCallbacks();

   void UpdateAPIThread() { m_apiThread = std::this_thread::get_id(); }

private:
   MsgPluginManager();

   // API implementation
   static unsigned int MSGPIAPI GetPluginEndpoint(const char* id);
   static void MSGPIAPI GetEndpointInfo(const uint32_t endpointId, MsgEndpointInfo* info);
   static unsigned int MSGPIAPI GetMsgID(const char* name_space, const char* name);
   static void MSGPIAPI SubscribeMsg(const uint32_t endpointId, const unsigned int msgId, const msgpi_msg_callback callback, void* userData);
   static void MSGPIAPI UnsubscribeMsg(const unsigned int msgId, const msgpi_msg_callback callback);
   static void MSGPIAPI BroadcastMsg(const uint32_t endpointId, const unsigned int msgId, void* data);
   static void MSGPIAPI SendMsg(const uint32_t endpointId, const unsigned int msgId, const uint32_t targetEndpointId, void* data);
   static void MSGPIAPI ReleaseMsgID(const unsigned int msgId);
   static void MSGPIAPI RegisterSetting(const uint32_t endpointId, MsgSettingDef* settingDef);
   static void MSGPIAPI SaveSetting(const uint32_t endpointId, MsgSettingDef* settingDef);
   static void MSGPIAPI RunOnMainThread(const uint32_t endpointId, const double delayInS, const msgpi_timer_callback callback, void* userData);
   static void MSGPIAPI FlushPendingCallbacks(const uint32_t endpointId);

   std::vector<std::shared_ptr<MsgPlugin>> m_plugins;

   struct CallbackEntry
   {
      uint32_t endpointId;
      msgpi_msg_callback callback;
      void* context;
   };
   struct MsgEntry
   {
      int refCount;
      std::string name_space;
      std::string name;
      unsigned int id;
      std::list<CallbackEntry> callbacks;
   };
   std::vector<MsgEntry> m_msgs;

   struct TimerEntry
   {
      uint32_t endpointId;
      msgpi_timer_callback callback;
      void* userData;
      std::chrono::steady_clock::time_point time;
   };
   std::list<TimerEntry> m_timers;
   std::mutex m_timerListMutex;

   std::function<void(const std::string& pluginId, SettingAction action, MsgSettingDef* settingDef)> m_settingHandler;

   MsgPluginAPI m_api;
   
   uint32_t m_nextEndpointId = 1;

   std::function<void*(const std::string&, const std::string&)> m_dllLink;
   std::function<void(void*)> m_dllUnlink;
   std::function<void*(void*, const std::string&)> m_dllGetMethod;

   std::thread::id m_apiThread;
};

}