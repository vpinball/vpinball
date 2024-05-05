// license:GPLv3+

#include "core/stdafx.h"
#include "PluginManager.h"
#include <iostream>
#include <filesystem>
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"

PluginManager::PluginManager()
{
   m_vpxAPI.GetEventID = GetEventID;
   m_vpxAPI.SubscribeEvent = SubscribeEvent;
   m_vpxAPI.UnsubscribeEvent = UnsubscribeEvent;
   m_vpxAPI.BroadcastEvent = BroadcastEvent;
}

PluginManager::~PluginManager()
{
   for (VPXPlugin* plugin : m_plugins)
   {
      if (plugin->IsLoaded())
         plugin->Unload();
      delete plugin;
   }
   m_plugins.clear();
}

///////////////////////////////////////////////////////////////////////////////
// Event API (very basic messaging API that allows to share data blocks)

unsigned int PluginManager::m_nextEventId = 0;
robin_hood::unordered_map<string, unsigned int> PluginManager::m_eventIds;
vector<vpxpi_event_callback> PluginManager::m_eventCallbacks[m_maxEventCallbacks];

unsigned int PluginManager::GetEventID(const char* name)
{
   auto result = m_eventIds.try_emplace(name, m_nextEventId);
   if (result.second)
   {
      // Event created, search a free id for next event creation
      // TODO allow event release to avoid overflowing event table
      m_nextEventId++;
      if (m_nextEventId >= m_maxEventCallbacks)
      {
         PLOGE << "Too many event created.";
         exit(-1);
      }
   }
   return result.first->second;
}

void PluginManager::SubscribeEvent(const unsigned int eventId, const vpxpi_event_callback callback)
{
   assert(callback != nullptr);
   assert(0 <= eventId && eventId < m_maxEventCallbacks);
   assert(FindIndexOf(m_eventCallbacks[eventId], callback) == -1);
   m_eventCallbacks[eventId].push_back(callback);
}

void PluginManager::UnsubscribeEvent(const unsigned int eventId, const vpxpi_event_callback callback)
{
   assert(callback != nullptr);
   assert(0 <= eventId && eventId < m_maxEventCallbacks);
   typename vector<vpxpi_event_callback>::const_iterator it = std::find(m_eventCallbacks[eventId].begin(), m_eventCallbacks[eventId].end(), callback);
   assert(it != m_eventCallbacks[eventId].end());
   m_eventCallbacks[eventId].erase(it);
}

void PluginManager::BroadcastEvent(const unsigned int eventId, void* data)
{
   assert(0 <= eventId && eventId < m_maxEventCallbacks); 
   for (vpxpi_event_callback cb : m_eventCallbacks[eventId])
      cb(eventId, data);
}


///////////////////////////////////////////////////////////////////////////////
// Plugin management

void PluginManager::ScanPluginFolder(const string& pluginDir)
{
   assert(g_pplayer == nullptr); // Loading plugins while player is running is unsupported as it would lead to invalid state (onGameEnd event without priori onGameStart)
   if (!std::filesystem::exists(pluginDir))
   {
      PLOGE << "Missing plugin directory: " << pluginDir;
      return;
   }
   #if defined(__STANDALONE__)
      // Not yet implemented
      const string libraryKey = "";
      return;
   #elif defined(_WIN64)
      const string libraryKey = "windows.x64";
   #elif defined(_WIN32)
      const string libraryKey = "windows.x86";
   #endif
   for (const auto& entry : std::filesystem::directory_iterator(pluginDir))
   {
      if (entry.is_directory())
      {
         mINI::INIStructure ini;
         mINI::INIFile file(entry.path().string() + "/plugin.cfg");
         if (file.read(ini) && ini.has("informations") && ini["informations"].has("id") && ini.has("libraries") && ini["libraries"].has(libraryKey))
         {
            string id = ini["informations"]["id"];
            for (auto it = m_plugins.begin(); it != m_plugins.end(); it++)
               if ((*it)->m_id == id)
                  it = m_plugins.erase(it);
            const string libraryFile = ini["libraries"][libraryKey];
            const string libraryPath = entry.path().string() + "/" + libraryFile;
            if (!std::filesystem::exists(libraryPath))
            {
               PLOGE << "Plugin " << id << " has an invalid library reference to a missing file for " << libraryKey << ": " << libraryFile;
               return;
            }
            VPXPlugin* plugin = new VPXPlugin(id, 
               ini["informations"].get("name"),
               ini["informations"].get("description"),
               ini["informations"].get("author"),
               ini["informations"].get("version"),
               ini["informations"].get("link"),
               libraryPath);
            m_plugins.push_back(plugin);
            // TODO this directly loads the plugin which is a security concern, the initial load should be requasted/validated by the user
            plugin->Load(&m_vpxAPI);
         }
      }
   }
}

void VPXPlugin::Load(VPXPluginAPI* vpxAPI)
{
   if (m_is_loaded)
   {
      PLOGE << "Requested to load plugin '" << m_name << "' which is already loaded";
      return;
   }
   #if defined(__STANDALONE__)
      m_module = SDL_LoadObject(m_library.c_str());
      if (m_module == nullptr)
      {
         PLOGE << "Plugin " << m_id << " failed to load library " << m_library;
         return;
      }
      m_loadPlugin = (vpxpi_load_plugin)SDL_LoadFunction(m_module, "PluginLoad");
      m_unloadPlugin = (vpxpi_unload_plugin)SDL_LoadFunction(m_module, "PluginUnload");
      if (m_loadPlugin == nullptr || m_unloadPlugin == nullptr)
      {
         SDL_UnloadObject(m_module);
         m_loadPlugin = nullptr;
         m_unloadPlugin = nullptr;
         m_module = nullptr;
         PLOGE << "Plugin " << m_id << " invalid library " << m_library << ": required load/unload functions are not correct.";
         return;
      }
   #elif defined(_WIN32) || defined(_WIN64)
      m_module = LoadLibrary(m_library.c_str());
      if (m_module == NULL)
      {
         PLOGE << "Plugin " << m_id << " failed to load library " << m_library;
         return;
      }
      m_loadPlugin = (vpxpi_load_plugin)GetProcAddress(m_module, "PluginLoad");
      m_unloadPlugin = (vpxpi_unload_plugin)GetProcAddress(m_module, "PluginUnload");
      if (m_loadPlugin == nullptr || m_unloadPlugin == nullptr)
      {
         FreeLibrary(m_module);
         m_loadPlugin = nullptr;
         m_unloadPlugin = nullptr;
         m_module = NULL;
         PLOGE << "Plugin " << m_id << " invalid library " << m_library << ": required load/unload functions are not correct.";
         return;
      }
   #else
      assert(false);
   #endif
   m_loadPlugin(vpxAPI);
   m_is_loaded = true;
   PLOGI << "Plugin " << m_id << " loaded (library: " << m_library << ")";
}

void VPXPlugin::Unload()
{
   if (!m_is_loaded)
   {
      PLOGE << "Requested to unload plugin '" << m_name << "' which is not loaded";
      return;
   }
   m_unloadPlugin();
   #if defined(__STANDALONE__)
      SDL_UnloadObject(m_module);
      m_module = nullptr;
   #elif defined(_WIN32) || defined(_WIN64)
      FreeLibrary(m_module);
      m_module = NULL;
   #endif
   m_loadPlugin = nullptr;
   m_unloadPlugin = nullptr;
   m_is_loaded = false;
}
