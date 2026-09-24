// license:GPLv3+

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

#include "plugins/MsgPluginManager.h"

#include <filesystem>
#include <fstream>
#include <thread>

using namespace MsgPI;

namespace
{

// Statically linked test plugin recording its lifecycle callbacks
int g_loadCalls = 0;
int g_unloadCalls = 0;
uint32_t g_loadedEndpoint = 0;
const MsgPluginAPI* g_loadedApi = nullptr;

void TestPluginLoad(const uint32_t endpointId, const MsgPluginAPI* api)
{
   ++g_loadCalls;
   g_loadedEndpoint = endpointId;
   g_loadedApi = api;
}

void TestPluginUnload() { ++g_unloadCalls; }

struct MsgProbe
{
   int calls = 0;
   unsigned int lastMsgId = 0;
   void* lastData = nullptr;
   static void MSGPIAPI Callback(const unsigned int msgId, void* context, void* msgData)
   {
      MsgProbe* probe = static_cast<MsgProbe*>(context);
      ++probe->calls;
      probe->lastMsgId = msgId;
      probe->lastData = msgData;
   }
};

struct SettingEvent
{
   std::string pluginId;
   MsgPluginManager::SettingAction action;
};

// Minimal module loader handing back a sentinel module and the static load/unload functions
class TestModuleLoader final : public MsgModuleLoader
{
public:
   void* Link(const std::string& directory, const std::string& file) override
   {
      ++linkCalls;
      linkedFile = file;
      return this;
   }
   void Unlink(void* dynamicModule) override { ++unlinkCalls; }
   void* GetFunction(void* dynamicModule, const std::string& functionName) override
   {
      requestedFunctions.push_back(functionName);
      if (functionName == "ScannedPluginPluginLoad")
         return reinterpret_cast<void*>(&TestPluginLoad);
      if (functionName == "ScannedPluginPluginUnload")
         return reinterpret_cast<void*>(&TestPluginUnload);
      return nullptr;
   }
   int linkCalls = 0;
   int unlinkCalls = 0;
   std::string linkedFile;
   std::vector<std::string> requestedFunctions;
};

MSGPI_BOOL_VAL_SETTING(testSetting, "enabled", "Enabled", "", true, true);

void WriteFile(const std::filesystem::path& path, const std::string& content)
{
   std::ofstream file(path);
   file << content;
}

} // namespace

TEST_CASE("MsgPluginManager message API")
{
   MsgPluginManager pm;
   const MsgPluginAPI& api = pm.GetMsgAPI();
   g_loadCalls = 0;
   g_unloadCalls = 0;
   g_loadedEndpoint = 0;
   g_loadedApi = nullptr;

   SUBCASE("message ids are refcounted, case insensitive and recycled")
   {
      const unsigned int msg1 = api.GetMsgID("Test", "Event:1");
      const unsigned int msg2 = api.GetMsgID("Test", "Other:1");
      CHECK(msg1 != msg2);

      // Same name resolves to the same id, whatever the case
      CHECK(api.GetMsgID("test", "event:1") == msg1);
      CHECK(api.GetMsgID("TEST", "EVENT:1") == msg1);
      api.ReleaseMsgID(msg1);
      api.ReleaseMsgID(msg1);
      api.ReleaseMsgID(msg1);

      // Fully released entries are recycled
      const unsigned int recycled = api.GetMsgID("Test", "Recycled:1");
      CHECK(recycled == msg1);
      api.ReleaseMsgID(recycled);
      api.ReleaseMsgID(msg2);
   }

   SUBCASE("broadcast reaches every subscriber with context and data")
   {
      auto plugin = pm.RegisterPlugin("test", "Test", "", "", "", "", TestPluginLoad, TestPluginUnload);
      const unsigned int msgId = api.GetMsgID("Test", "Event:1");

      MsgProbe probeA, probeB;
      api.SubscribeMsg(plugin->m_endpointId, msgId, MsgProbe::Callback, &probeA);
      api.SubscribeMsg(plugin->m_endpointId, msgId, MsgProbe::Callback, &probeB);

      int payload = 42;
      api.BroadcastMsg(plugin->m_endpointId, msgId, &payload);
      CHECK(probeA.calls == 1);
      CHECK(probeB.calls == 1);
      CHECK(probeA.lastMsgId == msgId);
      CHECK(probeA.lastData == &payload);

      api.UnsubscribeMsg(msgId, MsgProbe::Callback, &probeA);
      api.BroadcastMsg(plugin->m_endpointId, msgId, &payload);
      CHECK(probeA.calls == 1);
      CHECK(probeB.calls == 2);

      api.UnsubscribeMsg(msgId, MsgProbe::Callback, &probeB);
      api.ReleaseMsgID(msgId);
   }

   SUBCASE("send only reaches the targeted endpoint")
   {
      auto pluginA = pm.RegisterPlugin("a", "A", "", "", "", "", TestPluginLoad, TestPluginUnload);
      auto pluginB = pm.RegisterPlugin("b", "B", "", "", "", "", TestPluginLoad, TestPluginUnload);
      const unsigned int msgId = api.GetMsgID("Test", "Event:1");

      MsgProbe probeA, probeB;
      api.SubscribeMsg(pluginA->m_endpointId, msgId, MsgProbe::Callback, &probeA);
      api.SubscribeMsg(pluginB->m_endpointId, msgId, MsgProbe::Callback, &probeB);

      api.SendMsg(pluginA->m_endpointId, msgId, pluginB->m_endpointId, nullptr);
      CHECK(probeA.calls == 0);
      CHECK(probeB.calls == 1);

      api.UnsubscribeMsg(msgId, MsgProbe::Callback, &probeA);
      api.UnsubscribeMsg(msgId, MsgProbe::Callback, &probeB);
      api.ReleaseMsgID(msgId);
   }

   SUBCASE("endpoints are only resolvable once loaded")
   {
      auto plugin = pm.RegisterPlugin("test", "Test", "Desc", "Author", "1.0", "link", TestPluginLoad, TestPluginUnload);
      CHECK(pm.GetPlugin("TEST") == plugin);
      CHECK(api.GetPluginEndpoint("test") == 0); // Not loaded yet

      pm.LoadPlugin(*plugin);
      CHECK(plugin->IsLoaded());
      CHECK(g_loadCalls == 1);
      CHECK(g_loadedEndpoint == plugin->m_endpointId);
      CHECK(g_loadedApi == &api);
      CHECK(api.GetPluginEndpoint("Test") == plugin->m_endpointId); // Case insensitive

      MsgEndpointInfo info {};
      api.GetEndpointInfo(plugin->m_endpointId, &info);
      CHECK(std::string(info.id) == "test");
      CHECK(std::string(info.name) == "Test");
      CHECK(std::string(info.author) == "Author");

      std::vector<SettingEvent> events;
      pm.SetSettingsHandler([&events](const std::string& id, MsgPluginManager::SettingAction action, MsgSettingDef*) { events.push_back({ id, action }); });
      pm.UnloadPlugin(*plugin);
      CHECK_FALSE(plugin->IsLoaded());
      CHECK(g_unloadCalls == 1);
      REQUIRE(events.size() == 1);
      CHECK(events[0].pluginId == "test");
      CHECK(events[0].action == MsgPluginManager::SettingAction::UnregisterAll);
      CHECK(api.GetPluginEndpoint("test") == 0);
   }

   SUBCASE("loading broadcasts the plugin lifecycle events")
   {
      auto plugin = pm.RegisterPlugin("test", "Test", "", "", "", "", TestPluginLoad, TestPluginUnload);
      auto observer = pm.RegisterPlugin("observer", "Observer", "", "", "", "", TestPluginLoad, TestPluginUnload);
      const unsigned int loadedMsg = api.GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_LOADED);
      const unsigned int unloadedMsg = api.GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_UNLOADED);
      MsgProbe loadProbe, unloadProbe;
      // Subscriptions must use a different endpoint than the observed plugin: still subscribed
      // callbacks of the unloaded endpoint are reported as leaks by UnloadPlugin
      api.SubscribeMsg(observer->m_endpointId, loadedMsg, MsgProbe::Callback, &loadProbe);
      api.SubscribeMsg(observer->m_endpointId, unloadedMsg, MsgProbe::Callback, &unloadProbe);

      pm.SetSettingsHandler([](const std::string&, MsgPluginManager::SettingAction, MsgSettingDef*) { });
      pm.LoadPlugin(*plugin);
      CHECK(loadProbe.calls == 1);
      CHECK(std::string(static_cast<const char*>(loadProbe.lastData)) == "test");

      pm.UnloadPlugin(*plugin);
      CHECK(unloadProbe.calls == 1);
      CHECK(std::string(static_cast<const char*>(unloadProbe.lastData)) == "test");

      api.UnsubscribeMsg(loadedMsg, MsgProbe::Callback, &loadProbe);
      api.UnsubscribeMsg(unloadedMsg, MsgProbe::Callback, &unloadProbe);
      api.ReleaseMsgID(loadedMsg);
      api.ReleaseMsgID(unloadedMsg);
   }

   SUBCASE("settings are dispatched to the host handler")
   {
      auto plugin = pm.RegisterPlugin("test", "Test", "", "", "", "", TestPluginLoad, TestPluginUnload);
      std::vector<SettingEvent> events;
      pm.SetSettingsHandler([&events](const std::string& id, MsgPluginManager::SettingAction action, MsgSettingDef*) { events.push_back({ id, action }); });

      api.RegisterSetting(plugin->m_endpointId, &testSetting);
      CHECK(events.empty()); // Plugin not loaded: no endpoint resolution

      pm.LoadPlugin(*plugin);
      api.RegisterSetting(plugin->m_endpointId, &testSetting);
      api.SaveSetting(plugin->m_endpointId, &testSetting);
      REQUIRE(events.size() == 2);
      CHECK(events[0].action == MsgPluginManager::SettingAction::Load);
      CHECK(events[1].action == MsgPluginManager::SettingAction::Save);

      pm.UnloadPlugin(*plugin);
      CHECK(events.size() == 3);
      CHECK(events[2].action == MsgPluginManager::SettingAction::UnregisterAll);
   }

   SUBCASE("deferred callbacks run on the API thread")
   {
      auto plugin = pm.RegisterPlugin("test", "Test", "", "", "", "", TestPluginLoad, TestPluginUnload);
      int immediate = 0;
      int deferred = 0;
      api.RunOnMainThread(plugin->m_endpointId, 0., [](void* data) { ++*static_cast<int*>(data); }, &immediate);
      CHECK(immediate == 1); // Delay <= 0 on the API thread runs synchronously

      api.RunOnMainThread(plugin->m_endpointId, 60., [](void* data) { ++*static_cast<int*>(data); }, &deferred);
      pm.ProcessAsyncCallbacks();
      CHECK(deferred == 0); // Not due yet

      api.FlushPendingCallbacks(plugin->m_endpointId);
      CHECK(deferred == 1); // Flush runs all pending callbacks of the endpoint
   }

   SUBCASE("plugin folder scan reads plugin.cfg and resolves the library")
   {
      const std::filesystem::path pluginRoot = GetTestTmpDir() / "plugins";
      const std::filesystem::path pluginDir = pluginRoot / "scanned";
      std::filesystem::create_directories(pluginDir);
      WriteFile(pluginDir / "plugin.cfg",
         "[configuration]\n"
         "id = \"ScannedPlugin\"\n"
         "name = \"Scanned Plugin\"\n"
         "description = \"Scanned test plugin\"\n"
         "author = \"Tests\"\n"
         "version = \"2.0\"\n"
         "link = \"\"\n"
         "[libraries]\n"
         "windows.x86 = \"scanned.dll\"\n"
         "windows.x64 = \"scanned.dll\"\n"
         "linux.x64 = \"scanned.dll\"\n"
         "linux.aarch64 = \"scanned.dll\"\n"
         "macos.x64 = \"scanned.dll\"\n"
         "macos.arm64 = \"scanned.dll\"\n"
         "android.x86_32 = \"scanned.dll\"\n"
         "android.x86_64 = \"scanned.dll\"\n");
      WriteFile(pluginDir / "scanned.dll", ""); // Existence is all the scan checks
      WriteFile(pluginRoot / "notaplugin.txt", "no plugin.cfg here");

      auto loader = std::make_shared<TestModuleLoader>();
      std::vector<std::string> scannedIds;
      pm.ScanPluginFolder(loader, pluginRoot, [&scannedIds](MsgPlugin& plugin) { scannedIds.push_back(plugin.m_id); });
      REQUIRE(scannedIds.size() == 1);
      auto plugin = pm.GetPlugin("ScannedPlugin");
      REQUIRE(plugin != nullptr);
      CHECK(plugin->m_name == "Scanned Plugin");
      CHECK(plugin->m_author == "Tests");
      CHECK(plugin->IsDynamicallyLinked());

      // Loading resolves the <id>PluginLoad/<id>PluginUnload exports through the loader
      pm.SetSettingsHandler([](const std::string&, MsgPluginManager::SettingAction, MsgSettingDef*) { });
      pm.LoadPlugin(*plugin);
      CHECK(loader->linkCalls == 1);
      CHECK(loader->requestedFunctions == std::vector<std::string> { "ScannedPluginPluginLoad", "ScannedPluginPluginUnload" });
      CHECK(plugin->IsLoaded());
      CHECK(g_loadCalls == 1);
      CHECK(api.GetPluginEndpoint("scannedplugin") == plugin->m_endpointId);

      // A second scan matches the already registered plugin instead of duplicating it
      pm.ScanPluginFolder(loader, pluginRoot, [&scannedIds](MsgPlugin& plugin) { scannedIds.push_back(plugin.m_id); });
      CHECK(pm.GetPlugins().size() == 1);
      CHECK(scannedIds.size() == 2);

      pm.UnloadPlugin(*plugin);
      CHECK(loader->unlinkCalls == 1);
   }

   SUBCASE("missing plugin folder is ignored")
   {
      auto loader = std::make_shared<TestModuleLoader>();
      pm.ScanPluginFolder(loader, GetTestTmpDir() / "does-not-exist", [](MsgPlugin&) { FAIL("unexpected plugin"); });
      CHECK(pm.GetPlugins().empty());
   }
}
