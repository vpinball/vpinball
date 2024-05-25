// license:GPLv3+

#include "core/stdafx.h"
#include "PluginManager.h"
#include <iostream>
#include <filesystem>
#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include "core/vpversion.h"

PluginManager::PluginManager()
{
   #ifdef _DEBUG
   memset(&m_vpxAPI, 0, sizeof(m_vpxAPI));
   #endif
   // Event API
   m_vpxAPI.GetEventID = GetEventID;
   m_vpxAPI.SubscribeEvent = SubscribeEvent;
   m_vpxAPI.UnsubscribeEvent = UnsubscribeEvent;
   m_vpxAPI.BroadcastEvent = BroadcastEvent;
   // General information API
   m_vpxAPI.GetTableInfo = GetTableInfo;
   // User Interface API
   m_vpxAPI.GetOption = GetOption;
   m_vpxAPI.PushNotification = PushNotification;
   m_vpxAPI.UpdateNotification = UpdateNotification;
   // View API
   m_vpxAPI.DisableStaticPrerendering = DisableStaticPrerendering;
   m_vpxAPI.GetActiveViewSetup = GetActiveViewSetup;
   m_vpxAPI.SetActiveViewSetup = SetActiveViewSetup;
   // Validate API to ensure no field has been forgotten
   #ifdef _DEBUG
   for (size_t i = 0; i < sizeof(m_vpxAPI) / 4; i++)
      assert(((unsigned int*)&m_vpxAPI)[i] != 0);
   #endif
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

unsigned int PluginManager::s_nextEventId = 0;
robin_hood::unordered_map<string, unsigned int> PluginManager::s_eventIds;
vector<vpxpi_event_callback> PluginManager::s_eventCallbacks[s_maxEventCallbacks];

unsigned int PluginManager::GetEventID(const char* name)
{
   const auto result = s_eventIds.try_emplace(name, s_nextEventId);
   if (result.second)
   {
      // Event created, search a free id for next event creation
      // TODO allow event release to avoid overflowing event table
      s_nextEventId++;
      if (s_nextEventId >= s_maxEventCallbacks)
      {
         PLOGE << "Too many events created.";
         exit(-1);
      }
   }
   return result.first->second;
}

void PluginManager::SubscribeEvent(const unsigned int eventId, const vpxpi_event_callback callback)
{
   assert(callback != nullptr);
   assert(0 <= eventId && eventId < s_maxEventCallbacks);
   assert(FindIndexOf(s_eventCallbacks[eventId], callback) == -1);
   s_eventCallbacks[eventId].push_back(callback);
}

void PluginManager::UnsubscribeEvent(const unsigned int eventId, const vpxpi_event_callback callback)
{
   assert(callback != nullptr);
   assert(0 <= eventId && eventId < s_maxEventCallbacks);
   typename vector<vpxpi_event_callback>::const_iterator it = std::find(s_eventCallbacks[eventId].begin(), s_eventCallbacks[eventId].end(), callback);
   assert(it != s_eventCallbacks[eventId].end());
   s_eventCallbacks[eventId].erase(it);
}

void PluginManager::BroadcastEvent(const unsigned int eventId, void* data)
{
   assert(0 <= eventId && eventId < s_maxEventCallbacks); 
   for (const vpxpi_event_callback cb : s_eventCallbacks[eventId])
      cb(eventId, data);
}


///////////////////////////////////////////////////////////////////////////////
// General information API

void PluginManager::GetTableInfo(VPXPluginAPI::TableInfo* info)
{
   assert(g_pplayer); // Only allowed in game
   info->path = g_pplayer->m_ptable->m_szFileName.c_str();
   info->tableWidth = g_pplayer->m_ptable->m_right;
   info->tableHeight = g_pplayer->m_ptable->m_bottom;
}


///////////////////////////////////////////////////////////////////////////////
// User Input API

float PluginManager::GetOption(const char* pageId, const unsigned int showMask, const char* optionName, const float minValue, const float maxValue, const float step,
   const float defaultValue, const VPXPluginAPI::OptionUnit unit, const char** values)
{
   // TODO handle showMask flag
   // TODO handle core VPX setting pages
   if (strcmp(pageId, VPX_TWEAK_VIEW) == 0)
   {
      return 0.f;
   }
   if (strcmp(pageId, VPX_TWEAK_TABLE) == 0)
   {
      return 0.f;
   }
   else
   {
      Settings& settings = g_pplayer ? g_pplayer->m_ptable->m_settings : g_pvp->m_settings;
      const string sectionName = "Plugin."s + pageId;
      Settings::Section section = settings.GetSection(sectionName);
      vector<string> literals;
      if (values != nullptr)
      {
         const int nSteps = 1 + (int)(roundf((maxValue - minValue) / step));
         for (int i = 0; i < nSteps; i++)
            literals.push_back(values[i]);
      }
      settings.RegisterSetting(section, optionName, minValue, maxValue, step, defaultValue, (Settings::OptionUnit)unit, literals);
      const float value = settings.LoadValueWithDefault(section, optionName, defaultValue);
      return clamp(minValue + step * roundf((value - minValue) / step), minValue, maxValue);
   }
}

void* PluginManager::PushNotification(const char* msg, const unsigned int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_liveUI->PushNotification(msg, lengthMs);
   // FIXME implement
   return nullptr;
}

void PluginManager::UpdateNotification(const void* handle, const char* msg, const unsigned int lengthMs)
{
   assert(g_pplayer); // Only allowed in game
   // FIXME implement
}


///////////////////////////////////////////////////////////////////////////////
// View API

void PluginManager::DisableStaticPrerendering(const bool disable)
{
   assert(g_pplayer); // Only allowed in game
   g_pplayer->m_renderer->DisableStaticPrePass(disable);
}

void PluginManager::GetActiveViewSetup(VPXPluginAPI::ViewSetupDef* view)
{
   assert(g_pplayer); // Only allowed in game
   const ViewSetup& viewSetup = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set];
   view->viewMode = viewSetup.mMode;
   view->sceneScaleX = viewSetup.mSceneScaleX;
   view->sceneScaleY = viewSetup.mSceneScaleY;
   view->sceneScaleZ = viewSetup.mSceneScaleZ;
   view->viewX = viewSetup.mViewX;
   view->viewY = viewSetup.mViewY;
   view->viewZ = viewSetup.mViewZ;
   view->lookAt = viewSetup.mLookAt;
   view->viewportRotation = viewSetup.mViewportRotation;
   view->FOV = viewSetup.mFOV;
   view->layback = viewSetup.mLayback;
   view->viewHOfs = viewSetup.mViewHOfs;
   view->viewVOfs = viewSetup.mViewVOfs;
   view->windowTopZOfs = viewSetup.mWindowTopZOfs;
   view->windowBottomZOfs = viewSetup.mWindowBottomZOfs;
   view->screenWidth = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenWidth"s, 0.0f);
   view->screenHeight = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenHeight"s, 0.0f);
   view->screenInclination = g_pplayer->m_ptable->m_settings.LoadValueWithDefault(Settings::Player, "ScreenInclination"s, 0.0f);
   view->realToVirtualScale = viewSetup.GetRealToVirtualScale(g_pplayer->m_ptable);
}

void PluginManager::SetActiveViewSetup(VPXPluginAPI::ViewSetupDef* view)
{
   assert(g_pplayer); // Only allowed in game
   ViewSetup& viewSetup = g_pplayer->m_ptable->mViewSetups[g_pplayer->m_ptable->m_BG_current_set];
   viewSetup.mViewX = view->viewX;
   viewSetup.mViewY = view->viewY;
   viewSetup.mViewZ = view->viewZ;
   g_pplayer->m_renderer->InitLayout();
}


///////////////////////////////////////////////////////////////////////////////
// Plugin management

string unquote(const string& str)
{
   if (str.front() == '"' && str.back() == '"')
      return str.substr(1, str.size() - 2);
   return str;
}

void PluginManager::ScanPluginFolder(const string& pluginDir)
{
   assert(g_pplayer == nullptr); // Loading plugins while player is running is unsupported as it would lead to invalid state (onGameEnd event without priori onGameStart)
   if (!std::filesystem::exists(pluginDir))
   {
      PLOGE << "Missing plugin directory: " << pluginDir;
      return;
   }
   string libraryKey;
   #ifdef _MSC_VER
      #if (GET_PLATFORM_BITS_ENUM == 0)
         libraryKey = "windows.x86_32"s;
      #elif (GET_PLATFORM_BITS_ENUM == 1)
         libraryKey = "windows.x86_64"s;
      #endif
   #elif (defined(__linux) || defined(__linux__))
      #if (GET_PLATFORM_BITS_ENUM == 0)
         libraryKey = "linux.x86_32"s;
      #elif (GET_PLATFORM_BITS_ENUM == 1)
         libraryKey = "linux.x86_64"s;
      #endif
   #elif defined(__APPLE__)
      #if defined(TARGET_OS_IOS) && TARGET_OS_IOS
         // Not yet implemented
      #elif defined(TARGET_OS_TV) && TARGET_OS_TV
         // Not yet implemented
      #else
         #if (GET_PLATFORM_BITS_ENUM == 0)
            libraryKey = "macos.x86_32"s;
         #elif (GET_PLATFORM_BITS_ENUM == 1)
            libraryKey = "macos.x86_64"s;
         #endif
      #endif
   #elif defined(__ANDROID__)
      #if (GET_PLATFORM_CPU_ENUM == 0)
         #if (GET_PLATFORM_BITS_ENUM == 0)
            libraryKey = "android.x86_32"s;
         #elif (GET_PLATFORM_BITS_ENUM == 1)
            libraryKey = "android.x86_64"s;
         #endif
      #elif (GET_PLATFORM_CPU_ENUM == 1) && (GET_PLATFORM_BITS_ENUM == 0)
         libraryKey = "android.x86_32"s;
      #endif
   #endif
   if (libraryKey.empty())
   {
      // Unsupported platform
      return;
   }

   for (const auto& entry : std::filesystem::directory_iterator(pluginDir))
   {
      if (entry.is_directory())
      {
         mINI::INIStructure ini;
         mINI::INIFile file(entry.path().string() + PATH_SEPARATOR_CHAR + "plugin.cfg");
         if (file.read(ini) && ini.has("configuration"s) && ini["configuration"s].has("id"s) && ini["configuration"s].has("vpx_api"s) && ini.has("libraries"s) && ini["libraries"s].has(libraryKey))
         {
            string id = unquote(ini["configuration"s]["id"s]);
            for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it)
               if ((*it)->m_id == id)
                  it = m_plugins.erase(it);
            const string libraryFile = unquote(ini["libraries"s][libraryKey]);
            const string libraryPath = entry.path().string() + PATH_SEPARATOR_CHAR + libraryFile;
            if (!std::filesystem::exists(libraryPath))
            {
               PLOGE << "Plugin " << id << " has an invalid library reference to a missing file for " << libraryKey << ": " << libraryFile;
               return;
            }
            string api = unquote(ini["configuration"s]["vpx_api"s]);
            int nParsed, apiVersion[4];
            apiVersion[3] = 0;
            nParsed = sscanf(api.c_str(), "%d.%d.%d.%d", &apiVersion[0], &apiVersion[1], &apiVersion[2], &apiVersion[3]);
            if ((nParsed == 3) || (nParsed == 4))
            {
               #define VERSION(a,b,c,d) (a * 1000000 + b * 10000 + c * 100 + d)
               if (VERSION(apiVersion[0], apiVersion[1], apiVersion[2], apiVersion[3]) < VERSION(10, 8, 1, 0))
               {
                  PLOGE << "Plugin " << id << " is not compatible with this version of VPX (VPX version: " << VP_VERSION_STRING_COMMAS << " / Plugin API: " << api << ')';
                  return;
               }
               #undef VERSION
            }
            else
            {
               PLOGE << "Plugin " << id << " has an invalid target API: '" << api << '\'';
               return;
            }
            VPXPlugin* plugin = new VPXPlugin(id, 
               unquote(ini["configuration"s].get("name"s)),
               unquote(ini["configuration"s].get("description"s)),
               unquote(ini["configuration"s].get("author"s)),
               unquote(ini["configuration"s].get("version"s)),
               unquote(ini["configuration"s].get("link"s)),
               libraryPath);
            m_plugins.push_back(plugin);
            // TODO this directly loads the plugin which is a security concern, the initial load should be requested/validated by the user
            plugin->Load(&m_vpxAPI);
         }
      }
   }
}

#ifdef _MSC_VER
std::string GetLastErrorAsString()
{
   DWORD errorMessageID = ::GetLastError();
   if (errorMessageID == 0)
      return std::string();
   LPSTR messageBuffer = nullptr;
   size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      (LPSTR)&messageBuffer, 0, NULL);
   std::string message(messageBuffer, size);
   LocalFree(messageBuffer);
   return message;
}
#endif

void VPXPlugin::Load(VPXPluginAPI* vpxAPI)
{
   if (m_is_loaded)
   {
      PLOGE << "Requested to load plugin '" << m_name << "' which is already loaded";
      return;
   }
   if (m_module == nullptr)
   {
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
      // Windows XP does not support LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR and LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
      // This makes the plugin features buggy on Windows XP since a plugin coming with its own dependencies (dll in the plugin directory) will not load on WinXP
      DWORD flags = 0x00000100 /* LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR */ | 0x00001000 /* LOAD_LIBRARY_SEARCH_DEFAULT_DIRS */;
      m_module = LoadLibraryEx(m_library.c_str(), NULL, flags);
      if (m_module == NULL)
      {
         PLOGE << "Plugin " << m_id << " failed to load library " << m_library;
         PLOGE << "Last error was: " << GetLastErrorAsString();
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
   }
   m_is_loaded = m_loadPlugin(vpxAPI);
   PLOGI << "Plugin " << m_id << (m_is_loaded ? " loaded (library: " : " failed to loaded (library: ") << m_library << ')';
}

void VPXPlugin::Unload()
{
   if (!m_is_loaded)
   {
      PLOGE << "Requested to unload plugin '" << m_name << "' which is not loaded";
      return;
   }
   m_unloadPlugin();
   // Use module unload instead of explicit unloading (avoid crashes due to forced unloading modules with thread still running)
   // The only drawback is that the application keep the module (dll file) locked
   /*
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
   */
}

// the singleton instance must be declared in the same compilation unit, after other static fields to ensure correct destructor call order
PluginManager PluginManager::s_instance;
