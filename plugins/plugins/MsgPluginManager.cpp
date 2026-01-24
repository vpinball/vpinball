// license:GPLv3+

#include "MsgPluginManager.h"

#include <cassert>
#include <iostream>
#include <filesystem>
#include <chrono>

#define MINI_CASE_SENSITIVE
#include "mINI/ini.h"
#include <plog/Log.h>

using namespace std::string_literals;

#if defined(_WIN32) || defined(_WIN64)
   #ifndef WIN32_LEAN_AND_MEAN
      #define WIN32_LEAN_AND_MEAN
   #endif
   #include <windows.h>
#endif

#if !defined(PATH_SEPARATOR_CHAR)
  #ifdef _MSC_VER
    #define PATH_SEPARATOR_CHAR '\\'
  #else
    #define PATH_SEPARATOR_CHAR '/'
  #endif
#endif

#if !defined(GET_PLATFORM_BITS_ENUM)
   #if (INTPTR_MAX == INT32_MAX)
      #define GET_PLATFORM_BITS_ENUM 0
   #else
      #define GET_PLATFORM_BITS_ENUM 1
   #endif
#endif

namespace MsgPI
{

static constexpr inline char cLower(char c)
{
   if (c >= 'A' && c <= 'Z')
      c ^= 32; //ASCII convention
   return c;
}

static bool StrCompareNoCase(const std::string& strA, const std::string& strB)
{
   return strA.length() == strB.length() && std::equal(strA.begin(), strA.end(), strB.begin(), [](char a, char b) { return cLower(a) == cLower(b); });
}

static bool StrCompareNoCase(const std::string& strA, const char* const strB)
{
   return strA.length() == strlen(strB) && std::equal(strA.begin(), strA.end(), strB, [](char a, char b) { return cLower(a) == cLower(b); });
}

MsgPluginManager& MsgPluginManager::GetInstance()
{
   static MsgPluginManager instance;
   return instance;
}

MsgPluginManager::MsgPluginManager()
   : m_apiThread(std::this_thread::get_id())
{
   m_api.GetPluginEndpoint = GetPluginEndpoint;
   m_api.GetEndpointInfo = GetEndpointInfo;
   m_api.GetMsgID = GetMsgID;
   m_api.SubscribeMsg = SubscribeMsg;
   m_api.UnsubscribeMsg = UnsubscribeMsg;
   m_api.BroadcastMsg = BroadcastMsg;
   m_api.SendMsg = SendMsg;
   m_api.ReleaseMsgID = ReleaseMsgID;
   m_api.RegisterSetting = RegisterSetting;
   m_api.SaveSetting = SaveSetting;
   m_api.RunOnMainThread = RunOnMainThread;
   m_api.FlushPendingCallbacks = FlushPendingCallbacks;
}

MsgPluginManager::~MsgPluginManager()
{
   assert(std::this_thread::get_id() == m_apiThread);
   m_plugins.clear();
   for (const auto& msg : m_msgs)
      if (msg.refCount > 0)
      {
         assert(false);
         // PLOGE << "Message " << msg.name_space << '.' << msg.name << " was not released (leaked reference count: " << msg.refCount << ')';
      }
}


///////////////////////////////////////////////////////////////////////////////
// Message API

unsigned int MsgPluginManager::GetPluginEndpoint(const char* id)
{
   MsgPluginManager& pm = GetInstance();
   std::string searched_id(id);
   const auto item = std::ranges::find_if(pm.m_plugins,
      [searched_id](const std::shared_ptr<MsgPlugin>& plg)
      {
         return plg->IsLoaded()
            && std::equal(plg->m_id.begin(), plg->m_id.end(), searched_id.begin(), searched_id.end(),
               [](char a, char b) { return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b)); });
      });
   if (item == pm.m_plugins.end())
      return 0;
   return item->get()->m_endpointId;
}

void MsgPluginManager::GetEndpointInfo(const uint32_t endpointId, MsgEndpointInfo* info)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   const auto item = std::ranges::find_if(pm.m_plugins, [endpointId](const std::shared_ptr<MsgPlugin>& plg) { return plg->IsLoaded() && plg->m_endpointId == endpointId; });
   if (item == pm.m_plugins.end())
      return;
   info->id = (*item)->m_id.c_str();
   info->name = (*item)->m_name.c_str();
   info->description = (*item)->m_description.c_str();
   info->author = (*item)->m_author.c_str();
   info->version = (*item)->m_version.c_str();
   info->link = (*item)->m_link.c_str();
}

unsigned int MsgPluginManager::GetMsgID(const char* name_space, const char* name)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);

   MsgEntry* freeMsg = nullptr;
   for (MsgEntry& msg : pm.m_msgs)
      if (freeMsg == nullptr && msg.refCount == 0)
         freeMsg = &msg;
      else if (StrCompareNoCase(msg.name_space, name_space) && StrCompareNoCase(msg.name, name))
      {
         msg.refCount++;
         return msg.id;
      }
   if (freeMsg == nullptr)
   {
      pm.m_msgs.push_back(MsgEntry());
      freeMsg = &pm.m_msgs.back();
      freeMsg->id = static_cast<unsigned int>(pm.m_msgs.size()) - 1;
   }
   freeMsg->refCount = 1;
   freeMsg->name_space = name_space;
   freeMsg->name = name;
   freeMsg->callbacks.clear();
   return freeMsg->id;
}

void MsgPluginManager::SubscribeMsg(const uint32_t endpointId, const unsigned int msgId, const msgpi_msg_callback callback, void* userData)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   assert(callback != nullptr);
   assert(msgId < pm.m_msgs.size());
   assert(pm.m_msgs[msgId].refCount > 0);
   assert(1 <= endpointId && endpointId < pm.m_nextEndpointId);
#ifdef _DEBUG
   for (const CallbackEntry& entry : pm.m_msgs[msgId].callbacks)
      assert(entry.callback != callback); // Callback are only allowed to be registered once per message
#endif

   pm.m_msgs[msgId].callbacks.emplace_back(endpointId, callback, userData);
}

void MsgPluginManager::UnsubscribeMsg(const unsigned int msgId, const msgpi_msg_callback callback)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   assert(callback != nullptr);
   assert(msgId < pm.m_msgs.size());
   assert(pm.m_msgs[msgId].refCount > 0);
   
   std::list<CallbackEntry>& callbacks = pm.m_msgs[msgId].callbacks;
   const auto it = std::ranges::find_if(callbacks, [&callback](const CallbackEntry& entry) { return callback == entry.callback; });
   assert(it != callbacks.end()); // Subscribe/Unsubscribe pairs must match
   callbacks.erase(it);
}

void MsgPluginManager::BroadcastMsg(const uint32_t endpointId, const unsigned int msgId, void* data)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   assert(msgId < pm.m_msgs.size());
   assert(pm.m_msgs[msgId].refCount > 0);
   assert(1 <= endpointId && endpointId < pm.m_nextEndpointId);

   for (const CallbackEntry& entry : pm.m_msgs[msgId].callbacks)
      entry.callback(msgId, entry.context, data);
}

void MsgPluginManager::SendMsg(const uint32_t endpointId, const unsigned int msgId, const uint32_t targetEndpointId, void* data)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   assert(msgId < pm.m_msgs.size());
   assert(pm.m_msgs[msgId].refCount > 0);
   assert(1 <= endpointId && endpointId < pm.m_nextEndpointId);

   for (const CallbackEntry& entry : pm.m_msgs[msgId].callbacks)
      if (entry.endpointId == targetEndpointId)
      {
         entry.callback(msgId, entry.context, data);
         break;
      }
}

void MsgPluginManager::ReleaseMsgID(const unsigned int msgId)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   assert(msgId < pm.m_msgs.size());
   assert(pm.m_msgs[msgId].refCount > 0);
   
   pm.m_msgs[msgId].refCount--;
   if (pm.m_msgs[msgId].refCount == 0)
   {
      assert(pm.m_msgs[msgId].callbacks.empty()); // Callbacks must be unsbscribed before destroying the message
      while (!pm.m_msgs.empty() && pm.m_msgs.back().refCount == 0)
         pm.m_msgs.pop_back();
   }
}

void MsgPluginManager::RegisterSetting(const uint32_t endpointId, MsgSettingDef* settingDef)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   if (pm.m_settingHandler == nullptr)
      return;
   const auto item = std::ranges::find_if(pm.m_plugins, [endpointId](const std::shared_ptr<MsgPlugin>& plg) { return plg->IsLoaded() && plg->m_endpointId == endpointId; });
   if (item == pm.m_plugins.end())
      return;
   pm.m_settingHandler((*item)->m_id, SettingAction::Load, settingDef);
}

void MsgPluginManager::SaveSetting(const uint32_t endpointId, MsgSettingDef* settingDef)
{
   MsgPluginManager& pm = GetInstance();
   if (pm.m_settingHandler == nullptr)
      return;
   const auto item = std::ranges::find_if(pm.m_plugins, [endpointId](const std::shared_ptr<MsgPlugin>& plg) { return plg->IsLoaded() && plg->m_endpointId == endpointId; });
   if (item == pm.m_plugins.end())
      return;
   pm.m_settingHandler((*item)->m_id, SettingAction::Save, settingDef);
}

void MsgPluginManager::RunOnMainThread(const uint32_t endpointId, const double delayInS, const msgpi_timer_callback callback, void* userData)
{
   MsgPluginManager& pm = GetInstance();
   assert(callback != nullptr);

   if (delayInS <= 0. && std::this_thread::get_id() == pm.m_apiThread)
   {
      callback(userData);
      return;
   }
   std::unique_lock lock(pm.m_timerListMutex);
   if (delayInS < 0.)
   {
      pm.m_timers.emplace(pm.m_timers.begin(), endpointId, callback, userData, std::chrono::steady_clock::now());
#ifdef _MSC_VER
      // Wake up message loop
      PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0);
#endif
      // FIXME block cleanly until processed
      lock.unlock();
      while (!pm.m_timers.empty())
         std::this_thread::sleep_for(std::chrono::nanoseconds(100));
   }
   else
   {
      auto timer = TimerEntry { endpointId, callback, userData, std::chrono::steady_clock::now() + std::chrono::microseconds(static_cast<int64_t>(delayInS * 1000000)) };
      pm.m_timers.insert(std::ranges::upper_bound(pm.m_timers.begin(), pm.m_timers.end(), timer, [](const TimerEntry& a, const TimerEntry& b) { return a.time < b.time; }), timer);
#ifdef _MSC_VER
      // Wake up message loop
      PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0);
#endif
   }
}

void MsgPluginManager::FlushPendingCallbacks(const uint32_t endpointId)
{
   MsgPluginManager& pm = GetInstance();
   assert(std::this_thread::get_id() == pm.m_apiThread);
   
   bool modified = true; // The callbacks may result in new callbacks being registered, so continue until we have no more pending ones
   while (modified)
   {
      std::list<TimerEntry> timers;
      {
         modified = false;
         const std::lock_guard lock(pm.m_timerListMutex);
         for (auto it = pm.m_timers.begin(); it != pm.m_timers.end();)
         {
            if (it->endpointId == endpointId)
            {
               modified = true;
               timers.push_back(*it);
               it = pm.m_timers.erase(it);
            }
            else
               ++it;
         }
      }
      // Release lock before calling callbacks to avoid deadlock
      for (const auto& it : timers)
         it.callback(it.userData);
   }
}

void MsgPluginManager::ProcessAsyncCallbacks()
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (m_timers.empty())
      return;
   std::list<TimerEntry> timers;
   {
      const std::lock_guard lock(m_timerListMutex);
      const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
      for (auto it = m_timers.begin(); it != m_timers.end();)
      {
         if (it->time > now)
            break;
         timers.push_back(*it);
         it = m_timers.erase(it);
      }
   }
   // Release lock before calling callbacks to avoid deadlock
   for (const auto& it : timers)
      it.callback(it.userData);
}


///////////////////////////////////////////////////////////////////////////////
// Plugin management

static std::string unquote(const std::string& str)
{
   if (str.front() == '"' && str.back() == '"')
      return str.substr(1, str.size() - 2);
   return str;
}

std::shared_ptr<MsgPlugin> MsgPluginManager::RegisterPlugin(const std::string& id, const std::string& name, const std::string& description, const std::string& author,
   const std::string& version, const std::string& link, msgpi_load_plugin loadPlugin, msgpi_unload_plugin unloadPlugin)
{
   assert(loadPlugin != nullptr);
   assert(unloadPlugin != nullptr);
   std::shared_ptr<MsgPlugin> plugin = std::make_shared<MsgPlugin>(id, name, description, author, version, link, loadPlugin, unloadPlugin, m_nextEndpointId++);
   m_plugins.push_back(plugin);
   return plugin;
}

void MsgPluginManager::ScanPluginFolder(std::shared_ptr<MsgModuleLoader> loader, const std::filesystem::path& pluginDir, const std::function<void(MsgPlugin&)>& callback)
{
   assert(std::this_thread::get_id() == m_apiThread);
   if (!std::filesystem::exists(pluginDir))
   {
      PLOGE << "Missing plugin directory: " << pluginDir;
      return;
   }
   std::string libraryKey;
#ifdef _MSC_VER
#if (INTPTR_MAX == INT32_MAX)
   libraryKey = "windows.x86"s;
#else
   libraryKey = "windows.x64"s;
#endif
#elif defined(__ANDROID__) // leave here, as it also defines linux
#if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__)           \
   || defined(__x86_64__)
#if (INTPTR_MAX == INT32_MAX)
   libraryKey = "android.x86_32"s;
#else
   libraryKey = "android.x86_64"s;
#endif
#elif (INTPTR_MAX == INT32_MAX)
   libraryKey = "android.x86_32"s;
#endif
#elif (defined(__linux) || defined(__linux__))
#if defined(__aarch64__)
   libraryKey = "linux.aarch64"s;
#else
   libraryKey = "linux.x64"s;
#endif
#elif defined(__APPLE__)
#if defined(TARGET_OS_IOS) && TARGET_OS_IOS
   // Not yet implemented
#elif defined(TARGET_OS_TV) && TARGET_OS_TV
   // Not yet implemented
#elif defined(__aarch64__)
   libraryKey = "macos.arm64"s;
#else
   libraryKey = "macos.x64"s;
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
         if (file.read(ini) && ini.has("configuration"s) && ini["configuration"s].has("id"s) && ini.has("libraries"s) && ini["libraries"s].has(libraryKey))
         {
            std::string id = unquote(ini["configuration"s]["id"s]);
            for (auto it = m_plugins.begin(); it != m_plugins.end(); ++it)
               if ((*it)->m_id == id)
                  it = m_plugins.erase(it);
            const std::string libraryFile = unquote(ini["libraries"s][libraryKey]);
            const std::string libraryPath = entry.path().string() + PATH_SEPARATOR_CHAR + libraryFile;
            if (!std::filesystem::exists(libraryPath))
            {
               PLOGE << "Plugin " << id << " has an invalid library reference to a missing file for " << libraryKey << ": " << libraryFile;
               continue;
            }
            std::shared_ptr<MsgPlugin> plugin = std::make_shared<MsgPlugin>(id, unquote(ini["configuration"s].get("name"s)), unquote(ini["configuration"s].get("description"s)),
               unquote(ini["configuration"s].get("author"s)), unquote(ini["configuration"s].get("version"s)), unquote(ini["configuration"s].get("link"s)), loader, entry.path().string(), libraryPath,
               m_nextEndpointId++);
            m_plugins.push_back(plugin);
            callback(*plugin);
         }
      }
   }
}

void MsgPluginManager::LoadPlugin(MsgPlugin& plugin)
{
   plugin.Load(&m_api);
}

void MsgPluginManager::UnloadPlugin(MsgPlugin& plugin)
{
   m_settingHandler(plugin.m_id, SettingAction::UnregisterAll, nullptr);
   plugin.Unload();
   bool invalidPlugin = false;
   for (const auto& timer : m_timers)
      if (timer.endpointId == plugin.m_endpointId)
      {
         invalidPlugin = true;
         PLOGE << "Plugin '" << plugin.m_name << "' did not flush its deferred runnable while Unloading";
      }
   for (const auto& msg : m_msgs)
      for (const auto& callback : msg.callbacks)
         if (callback.endpointId == plugin.m_endpointId)
         {
            invalidPlugin = true;
            PLOGE << "Plugin '" << plugin.m_name << "' leaked a callback for message " << msg.name_space << '.' << msg.name;
         }
   assert(!invalidPlugin);
}

void MsgPluginManager::UnloadPlugins()
{
   for (const auto& plugin : m_plugins)
      if (!plugin->m_library.empty() && plugin->IsLoaded())
         plugin->Unload();
}

std::shared_ptr<MsgPlugin> MsgPluginManager::GetPlugin(const std::string& pluginId) const
{
   for (const auto& plugin : m_plugins)
      if (StrCompareNoCase(plugin->m_id, pluginId))
         return plugin;
   return nullptr;
}

MsgPlugin::~MsgPlugin()
{
   if (!m_library.empty() && IsLoaded())
      Unload();
}

void MsgPlugin::Load(const MsgPluginAPI* msgAPI)
{
   if (IsLoaded())
   {
      PLOGE << "Requested to load plugin '" << m_name << "' which is already loaded";
      return;
   }
   if (m_loader && m_module == nullptr)
   {
      const std::string load = m_id + "PluginLoad";
      const std::string unload = m_id + "PluginUnload";
      m_module = m_loader->Link(m_directory, m_library);
      if (m_module == nullptr)
      {
         PLOGE << "Plugin " << m_id << " failed to load library " << m_library;
         return;
      }
      m_loadPlugin = (msgpi_load_plugin)m_loader->GetFunction(m_module, load);
      m_unloadPlugin = (msgpi_unload_plugin)m_loader->GetFunction(m_module, unload);
      if (m_loadPlugin == nullptr || m_unloadPlugin == nullptr)
      {
         m_loader->Unlink(m_module);
         m_loadPlugin = nullptr;
         m_unloadPlugin = nullptr;
         m_module = nullptr;
         PLOGE << "Plugin " << m_id << " invalid library " << m_library << ": required " << load << '/' << unload << " functions are not correct.";
         return;
      }
   }
   m_msgAPI = msgAPI;
   m_loadPlugin(m_endpointId, m_msgAPI);
   if (m_loader)
   {
      PLOGI << "Plugin " << m_id << " loaded (library: " << m_library << ')';
   }
   else
   {
      PLOGI << "Plugin " << m_id << " loaded (statically linked plugin)";
   }
   unsigned int msgId = m_msgAPI->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_LOADED);
   m_msgAPI->BroadcastMsg(m_endpointId, msgId, const_cast<char*>(m_id.c_str()));
   m_msgAPI->ReleaseMsgID(msgId);
}

void MsgPlugin::Unload()
{
   if (!IsLoaded())
   {
      PLOGE << "Requested to unload plugin '" << m_name << "' which is not loaded";
      return;
   }
   m_unloadPlugin();
   if (m_loader)
   {
      m_loader->Unlink(m_module);
      m_module = nullptr;
      m_loadPlugin = nullptr;
      m_unloadPlugin = nullptr;
   }
   unsigned int msgId = m_msgAPI->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_UNLOADED);
   m_msgAPI->BroadcastMsg(m_endpointId, msgId, const_cast<char*>(m_id.c_str()));
   m_msgAPI->ReleaseMsgID(msgId);
   m_msgAPI = nullptr;
}

}
