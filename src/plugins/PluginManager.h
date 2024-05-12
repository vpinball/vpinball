// license:GPLv3+

#pragma once

#include "VPXPlugin.h"

typedef bool (*vpxpi_load_plugin)(VPXPluginAPI* api);
typedef void (*vpxpi_unload_plugin)();

class VPXPlugin
{
public:
   VPXPlugin(const string& id, const string& name, const string& description, const string& author, const string& version, const string& link, const string& library)
      : m_id(id)
      , m_name(name)
      , m_description(description)
      , m_author(author)
      , m_version(version)
      , m_link(link)
      , m_library(library) { }
   ~VPXPlugin() { }

   void Load(VPXPluginAPI* vpxAPI);
   void Unload();
   bool IsLoaded() const { return m_is_loaded; }

   const string m_id; // Unique ID of the plugin, used to identify it
   const string m_name; // Human-readable name of the plugin
   const string m_description; // Human-readable description of the plugin intent
   const string m_author; // Human-readable author name, if you really want to share it :)
   const string m_version; // Human-readable version (VPX does not perform any version management)
   const string m_link; // Web link to online information
   const string m_library; // Library implementing this plugin for the current platform

private:
   bool m_is_loaded = false;
   vpxpi_load_plugin m_loadPlugin = nullptr;
   vpxpi_unload_plugin m_unloadPlugin = nullptr;
   #if defined(__STANDALONE__)
      void* m_module = nullptr;
   #elif defined(_WIN32) || defined(_WIN64)
      HMODULE m_module = NULL;
   #endif
};

class PluginManager
{
public:
   PluginManager();
   ~PluginManager();

   void ScanPluginFolder(const string& pluginDir);

   static PluginManager& GetInstance() { return s_instance; }

   // Event API implementation
   static unsigned int GetEventID(const char* name);
   static void SubscribeEvent(const unsigned int eventId, const vpxpi_event_callback callback);
   static void UnsubscribeEvent(const unsigned int eventId, const vpxpi_event_callback callback);
   static void BroadcastEvent(const unsigned int eventId, void* data);

   // General information API
   static void GetTableInfo(VPXPluginAPI::TableInfo* info);

   // View API implementation
   static void DisableStaticPrerendering(bool disable);
   static void GetActiveViewSetup(VPXPluginAPI::ViewSetupDef* view);
   static void SetActiveViewSetup(VPXPluginAPI::ViewSetupDef* view);

private:
   vector<VPXPlugin*> m_plugins;
   VPXPluginAPI m_vpxAPI;

   // Event API implementation
   static robin_hood::unordered_map<string, unsigned int> s_eventIds;
   static constexpr unsigned int s_maxEventCallbacks = 500;
   static vector<vpxpi_event_callback> s_eventCallbacks[s_maxEventCallbacks];
   static unsigned int s_nextEventId;
   static PluginManager s_instance;
};
