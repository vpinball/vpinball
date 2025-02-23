// license:GPLv3+

#pragma once

// This file provides a default C++ implementation of a plugin manager for the 'Generic Message Plugin API'.
// It includes SDL & Win32 implementations of plugin discovery, loading/unloading and messaging API.
// It has 2 strong dependencies beside C++ standard libraries: mINI and PLOG

#include "MsgPlugin.h"

#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <functional>

typedef void (*msgpi_load_plugin)(const uint32_t pluginId, const MsgPluginAPI* api);
typedef void (*msgpi_unload_plugin)();

class MsgPlugin
{
public:
   MsgPlugin(const std::string& id, const std::string& name, const std::string& description, const std::string& author, const std::string& version, const std::string& link,
      const std::string& directory, const std::string& library, const unsigned int endpointId)
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
      , m_isDynamicallyLinked(true) { }
   MsgPlugin(const std::string& id, const std::string& name, const std::string& description, const std::string& author, const std::string& version, const std::string& link,
      const msgpi_load_plugin& loadPlugin, const msgpi_unload_plugin& unloadPlugin, const unsigned int endpointId)
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
      , m_unloadPlugin(unloadPlugin)
      , m_isDynamicallyLinked(false) { }
   ~MsgPlugin();

   void Load(const MsgPluginAPI* msgAPI);
   void Unload();
   bool IsLoaded() const { return m_isLoaded; }


   const std::string m_id; // Unique ID of the plugin, used to identify it
   const std::string m_name; // Human-readable name of the plugin
   const std::string m_description; // Human-readable description of the plugin intent
   const std::string m_author; // Human-readable author name
   const std::string m_version; // Human-readable version
   const std::string m_link; // Web link to online information

   const bool m_isDynamicallyLinked; // Plugins can be either dynamically linked or statically linked and directly contributed
   const std::string m_library; // Library implementing this plugin for the current platform
   const std::string m_directory; // Directory containing this plugin

   const uint32_t m_endpointId; // Unique 'end point' ID of the plugin, used to identify it for the lifetime of this session

private:
   msgpi_load_plugin m_loadPlugin;
   msgpi_unload_plugin m_unloadPlugin;
   bool m_isLoaded = false;
   void* m_module = nullptr;
};

class MsgPluginManager
{
public:
   static MsgPluginManager& GetInstance();
   ~MsgPluginManager();

   std::shared_ptr<MsgPlugin> RegisterPlugin(const std::string& id, const std::string& name, const std::string& description, const std::string& author, const std::string& version, const std::string& link, const msgpi_load_plugin& loadPlugin, const msgpi_unload_plugin& unloadPlugin);
   void ScanPluginFolder(const std::string& pluginDir, const std::function<void(MsgPlugin&)>& callback);
   std::shared_ptr<MsgPlugin> GetPlugin(const std::string& pluginId) const;
   const MsgPluginAPI& GetMsgAPI() const { return m_api; }
   void ProcessAsyncCallbacks();
   void UnloadPlugins();
   void SetSettingsHandler(const std::function<void(const char*, const char*, char*, unsigned int)>& handler) { m_settingHandler = handler; }

private:
   MsgPluginManager();

   // API implementation
   static unsigned int GetMsgID(const char* name_space, const char* name);
   static void SubscribeMsg(const unsigned int endpointId, const unsigned int msgId, const msgpi_msg_callback callback, void* userData);
   static void UnsubscribeMsg(const unsigned int msgId, const msgpi_msg_callback callback);
   static void BroadcastMsg(const unsigned int endpointId, const unsigned int msgId, void* data);
   static void ReleaseMsgID(const unsigned int msgId);
   static void GetSetting(const char* name_space, const char* name, char* valueBuf, unsigned int valueBufSize);
   static void RunOnMainThread(const double delayInS, const msgpi_timer_callback callback, void* userData);

   std::vector<std::shared_ptr<MsgPlugin>> m_plugins;

   struct CallbackEntry
   {
      unsigned int endpointId;
      msgpi_msg_callback callback;
      void* userData;
   };
   struct MsgEntry
   {
      int refCount;
      std::string name_space;
      std::string name;
      unsigned int id;
      std::vector<CallbackEntry> callbacks;
   };
   std::vector<MsgEntry> m_msgs;

   struct TimerEntry
   {
      msgpi_timer_callback callback;
      void* userData;
      std::chrono::high_resolution_clock::time_point time;
   };
   std::vector<TimerEntry> m_timers;
   std::mutex m_timerListMutex;

   std::function<void(const char*, const char*, char*, unsigned int)> m_settingHandler = 
      [](const char* name_space, const char* name, char* valueBuf, unsigned int valueBufSize) { valueBuf[0] = 0; };
 
   MsgPluginAPI m_api;
   
   uint32_t m_nextEndpointId = 1;

   std::thread::id m_apiThread;
};
