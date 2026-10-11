// license:GPLv3+

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

#include "plugins/MsgPluginManager.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/ResURIResolver.h"
#include "plugins/ResURIBuilder.h"

using namespace MsgPI;
using namespace PinballPlugin;
using namespace PinballPlugin::Controller;

namespace
{

// Fake controller plugin exposing one display, one segment display, one float state group
// and answering the controllers/run/stop messages
struct FakeController
{
   const MsgPluginAPI* api = nullptr;
   uint32_t endpoint = 0;
   std::unique_ptr<CtrlItemProvider<DisplaySrcId>> displays;
   std::unique_ptr<CtrlItemProvider<SegSrcId>> segs;
   std::unique_ptr<CtrlItemProvider<StateSrcId>> states;
   unsigned int getControllersMsgId = 0;
   unsigned int runCmdMsgId = 0;
   unsigned int runStopMsgId = 0;

   float stateValueA = 0.25f;
   float stateValueB = 0.75f;
   uint8_t stateValueInt = 3;
   uint8_t stateValueC = 0;

   bool running = false;
   std::string gameId;
   int stopCalls = 0;

   static void MSGPIAPI GetStateA(void* ctx, void* result) { *static_cast<float*>(result) = static_cast<FakeController*>(ctx)->stateValueA; }
   static void MSGPIAPI GetStateB(void* ctx, void* result) { *static_cast<float*>(result) = static_cast<FakeController*>(ctx)->stateValueB; }
   static void MSGPIAPI GetStateInt(void* ctx, void* result) { *static_cast<uint8_t*>(result) = static_cast<FakeController*>(ctx)->stateValueInt; }
   static void MSGPIAPI GetStateC(void* ctx, void* result) { *static_cast<uint8_t*>(result) = static_cast<FakeController*>(ctx)->stateValueC; }
   static void MSGPIAPI SetStateC(void* ctx, const void* value) { static_cast<FakeController*>(ctx)->stateValueC = *static_cast<const uint8_t*>(value); }
   static DisplayFrame MSGPIAPI GetDmdFrame(void*) { return { 1, nullptr }; }
   static SegDisplayFrame MSGPIAPI GetSegFrame(void*) { return { 1, nullptr }; }

   static void MSGPIAPI OnGetControllers(unsigned int, void* context, void* msgData)
   {
      FakeController* self = static_cast<FakeController*>(context);
      GetControllersMsg* msg = static_cast<GetControllersMsg*>(msgData);
      if (!self->running)
         return;
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = { self->endpoint, self->gameId.c_str() };
      ++msg->count;
   }

   static void MSGPIAPI OnRunCmd(unsigned int, void* context, void* msgData)
   {
      FakeController* self = static_cast<FakeController*>(context);
      RunCmdMsg* msg = static_cast<RunCmdMsg*>(msgData);
      if (self->running)
         msg->status = CTLPI_RUN_STATUS_RUNNING;
      else if (std::string(msg->gameId) == "unknown")
         msg->status = CTLPI_RUN_STATUS_UNKNOWN_GAME;
      else
      {
         self->running = true;
         self->gameId = msg->gameId;
         msg->status = CTLPI_RUN_STATUS_OK;
      }
   }

   static void MSGPIAPI OnRunStop(unsigned int, void* context, void*)
   {
      FakeController* self = static_cast<FakeController*>(context);
      self->running = false;
      ++self->stopCalls;
   }
};

FakeController g_ctl;

CtlResId ResId(uint32_t endpoint, uint32_t res)
{
   CtlResId id;
   id.endpointId = endpoint;
   id.resId = res;
   return id;
}

void ControllerLoad(const uint32_t endpointId, const MsgPluginAPI* api)
{
   g_ctl.api = api;
   g_ctl.endpoint = endpointId;
   g_ctl.displays = std::make_unique<CtrlItemProvider<DisplaySrcId>>(api, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   g_ctl.segs = std::make_unique<CtrlItemProvider<SegSrcId>>(api, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG);
   g_ctl.states = std::make_unique<CtrlItemProvider<StateSrcId>>(api, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG);
   g_ctl.getControllersMsgId = api->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG);
   g_ctl.runCmdMsgId = api->GetMsgID(CTLPI_NAMESPACE, CTLPI_RUN_CMD);
   g_ctl.runStopMsgId = api->GetMsgID(CTLPI_NAMESPACE, CTLPI_RUN_STOP);
   api->SubscribeMsg(endpointId, g_ctl.getControllersMsgId, FakeController::OnGetControllers, &g_ctl);
   api->SubscribeMsg(endpointId, g_ctl.runCmdMsgId, FakeController::OnRunCmd, &g_ctl);
   api->SubscribeMsg(endpointId, g_ctl.runStopMsgId, FakeController::OnRunStop, &g_ctl);

   DisplaySrcId display {};
   display.id = ResId(endpointId, 1);
   display.width = 128;
   display.height = 32;
   display.hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED;
   display.frameFormat = CTLPI_DISPLAY_FORMAT_LUM32F;
   display.GetRenderFrame = FakeController::GetDmdFrame;
   g_ctl.displays->AddItem(display);

   DisplaySrcId lcd {};
   lcd.id = ResId(endpointId, 2);
   lcd.width = 640;
   lcd.height = 480;
   lcd.hardware = CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY;
   lcd.frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888;
   lcd.GetRenderFrame = FakeController::GetDmdFrame;
   g_ctl.displays->AddItem(lcd);

   SegSrcId seg {};
   seg.id = ResId(endpointId, 1);
   seg.groupId = ResId(endpointId, 0);
   seg.nElements = 3;
   seg.GetState = FakeController::GetSegFrame;
   g_ctl.segs->AddItem(seg);

   static StateDef stateDefs[4];
   stateDefs[0] = { "sol11", "Solenoid 11", 11, CTLPI_STATE_FORMAT_FLOAT, CTLPI_STATE_TYPE_CUSTOM, &g_ctl, FakeController::GetStateA, nullptr };
   stateDefs[1] = { "gi1", "GI string 1", 1, CTLPI_STATE_FORMAT_FLOAT, CTLPI_STATE_TYPE_RELATIVE_BRIGHTNESS, &g_ctl, FakeController::GetStateB, nullptr };
   stateDefs[2] = { "sw7", "Switch 7", 7, CTLPI_STATE_FORMAT_UINT8, CTLPI_STATE_TYPE_SWITCH, &g_ctl, FakeController::GetStateInt, nullptr };
   stateDefs[3] = { "sw10", "Switch 10", 10, CTLPI_STATE_FORMAT_UINT8, CTLPI_STATE_TYPE_SWITCH, &g_ctl, FakeController::GetStateC, FakeController::SetStateC };
   StateSrcId stateSrc {};
   stateSrc.id = ResId(endpointId, 3);
   stateSrc.name = "Lamps";
   stateSrc.nStates = 4;
   stateSrc.stateDefs = stateDefs;
   g_ctl.states->AddItem(stateSrc);
}

void ControllerUnload()
{
   g_ctl.api->UnsubscribeMsg(g_ctl.getControllersMsgId, FakeController::OnGetControllers, &g_ctl);
   g_ctl.api->UnsubscribeMsg(g_ctl.runCmdMsgId, FakeController::OnRunCmd, &g_ctl);
   g_ctl.api->UnsubscribeMsg(g_ctl.runStopMsgId, FakeController::OnRunStop, &g_ctl);
   g_ctl.api->ReleaseMsgID(g_ctl.getControllersMsgId);
   g_ctl.api->ReleaseMsgID(g_ctl.runCmdMsgId);
   g_ctl.api->ReleaseMsgID(g_ctl.runStopMsgId);
   g_ctl.displays.reset();
   g_ctl.segs.reset();
   g_ctl.states.reset();
   g_ctl = {};
}

void NoopLoad(const uint32_t, const MsgPluginAPI*) { }
void NoopUnload() { }

} // namespace

TEST_CASE("ResURIBuilder source enumeration, URI generation and lifecycle")
{
   MsgPluginManager pm;
   pm.SetSettingsHandler([](const std::string&, MsgPluginManager::SettingAction, MsgSettingDef*) { });

   auto ctl = pm.RegisterPlugin("testctl", "Test Controller", "", "", "", "", ControllerLoad, ControllerUnload);
   auto host = pm.RegisterPlugin("testhost", "Test Host", "", "", "", "", NoopLoad, NoopUnload);
   auto aux = pm.RegisterPlugin("testaux", "Test Aux", "", "", "", "", NoopLoad, NoopUnload);
   pm.LoadPlugin(*ctl);

   ResURIBuilder builder(&pm, &pm.GetMsgAPI(), host->m_endpointId);

   SUBCASE("plugin list reflects load and controller states")
   {
      auto plugins = builder.GetPlugins();
      REQUIRE(plugins.size() == 3);
      const auto ctlInfo = std::ranges::find_if(plugins, [](const ResURIBuilder::PluginInfo& p) { return p.id == "testctl"; });
      const auto hostInfo = std::ranges::find_if(plugins, [](const ResURIBuilder::PluginInfo& p) { return p.id == "testhost"; });
      REQUIRE(ctlInfo != plugins.end());
      REQUIRE(hostInfo != plugins.end());
      CHECK(ctlInfo->loaded);
      CHECK(ctlInfo->endpointId == ctl->m_endpointId);
      CHECK(ctlInfo->runningGameId.empty());
      CHECK_FALSE(hostInfo->loaded);
      CHECK(hostInfo->endpointId == 0);
   }

   SUBCASE("run starts a controller, stop ends it")
   {
      CHECK(builder.RunController("testctl", "afm_113") == CTLPI_RUN_STATUS_OK);
      auto plugins = builder.GetPlugins();
      const auto ctlInfo = std::ranges::find_if(plugins, [](const ResURIBuilder::PluginInfo& p) { return p.id == "testctl"; });
      REQUIRE(ctlInfo != plugins.end());
      CHECK(ctlInfo->runningGameId == "afm_113");

      CHECK(builder.RunController("testctl", "mm_109") == CTLPI_RUN_STATUS_RUNNING);
      CHECK(builder.RunController("testctl", "unknown") == CTLPI_RUN_STATUS_RUNNING);
      CHECK(builder.RunController("nothere", "game") == CTLPI_RUN_STATUS_UNSUPPORTED);

      builder.StopController("testctl");
      CHECK(g_ctl.stopCalls == 1);
      plugins = builder.GetPlugins();
      CHECK(std::ranges::find_if(plugins, [](const ResURIBuilder::PluginInfo& p) { return p.id == "testctl"; })->runningGameId.empty());
   }

   SUBCASE("run on an unloaded plugin is unsupported") { CHECK(builder.RunController("testhost", "game") == CTLPI_RUN_STATUS_UNSUPPORTED); }

   SUBCASE("float state items: enumeration, filter, peek and URIs")
   {
      const auto items = builder.GetStateItems();
      REQUIRE(items.size() == 2); // non float states are filtered out
      CHECK(items[0].pluginId == "testctl");
      CHECK(items[0].groupId == 3);
      CHECK(items[0].groupName == "Lamps");

      const auto filtered = builder.GetStateItems("gi");
      REQUIRE(filtered.size() == 1);
      CHECK(filtered[0].name == "gi1");
      CHECK(filtered[0].mappingId == 1);

      CHECK(builder.GetStateItems("lamps").size() == 2); // matches the group name
      CHECK(builder.GetStateItems("11").size() == 1); // matches the mapping id
      CHECK(builder.GetStateItems("nope").empty());

      CHECK_FALSE(items[0].writable);

      // semanticType, writableOnly and dataFormat filters restrict the listing (e.g. switch links for triggers)
      const auto switches = builder.GetStateItems({}, CTLPI_STATE_TYPE_SWITCH, false, -1);
      REQUIRE(switches.size() == 2);
      CHECK(switches[0].name == "sw7");
      CHECK(switches[0].dataFormat == CTLPI_STATE_FORMAT_UINT8);
      CHECK_FALSE(switches[0].writable);
      CHECK(switches[1].name == "sw10");
      CHECK(switches[1].writable);
      CHECK(builder.GetStateItems({}, CTLPI_STATE_TYPE_SWITCH, true, -1).size() == 1);
      CHECK(builder.GetStateItems({}, CTLPI_STATE_TYPE_SWITCH).empty()); // switches are not float
      CHECK(builder.GetStateItems({}, CTLPI_STATE_TYPE_RELATIVE_BRIGHTNESS).size() == 1);

      float value = 0.f;
      REQUIRE(builder.PeekValue(items[0], value));
      CHECK(value == doctest::Approx(0.25f));
      g_ctl.stateValueA = 0.5f;
      REQUIRE(builder.PeekValue(items[0], value));
      CHECK(value == doctest::Approx(0.5f));

      // Generated URIs round-trip through the resolver
      ResURIResolver resolver(pm.GetMsgAPI(), host->m_endpointId, false, false, true);
      const std::string nameUri = builder.MakeStateURI(items[0], ResURIBuilder::StateLinkMode::Name);
      const std::string mappingUri = builder.MakeStateURI(items[0], ResURIBuilder::StateLinkMode::Mapping);
      CHECK(nameUri == "ctrl://testctl/state?group=3&name=sol11");
      CHECK(mappingUri == "ctrl://testctl/state?group=3&mapping=11");
      CHECK(resolver.GetFloatState(nameUri) == doctest::Approx(0.5f));
      CHECK(resolver.GetFloatState(mappingUri) == doctest::Approx(0.5f));
   }

   SUBCASE("display and seg items with URIs")
   {
      const auto displays = builder.GetDisplayItems();
      REQUIRE(displays.size() == 2);
      CHECK(displays[0].width == 128);
      CHECK(displays[0].resId == 1);
      CHECK(displays[1].resId == 2);
      CHECK(builder.MakeDisplayURI(displays[0]) == "ctrl://testctl/display?id=1");

      const auto segs = builder.GetSegItems();
      REQUIRE(segs.size() == 1);
      CHECK(segs[0].nElements == 3);
      CHECK(builder.MakeSegURI(segs[0], -1) == "ctrl://testctl/seg?id=1");
      CHECK(builder.MakeSegURI(segs[0], 2) == "ctrl://testctl/seg?id=1&sub=2");
   }

   SUBCASE("display family filtering and default pseudo entries")
   {
      // Dmd family keeps dot-matrix displays, NonDmd keeps CRT/LCD screens
      const auto dmds = builder.GetDisplayItems({}, DisplayFamily::Dmd);
      REQUIRE(dmds.size() == 1);
      CHECK(dmds[0].resId == 1);
      const auto screens = builder.GetDisplayItems({}, DisplayFamily::NonDmd);
      REQUIRE(screens.size() == 1);
      CHECK(screens[0].resId == 2);

      // withDefault prepends a pseudo item generating a 'ctrl://default' URI honoring the family
      const auto dmdWithDefault = builder.GetDisplayItems({}, DisplayFamily::Dmd, true);
      REQUIRE(dmdWithDefault.size() == 2);
      CHECK(dmdWithDefault[0].isDefault);
      CHECK(dmdWithDefault[0].defaultDmdOnly == 1);
      CHECK(builder.MakeDisplayURI(dmdWithDefault[0]) == "ctrl://default/display?dmd_only=1");

      const auto crtWithDefault = builder.GetDisplayItems({}, DisplayFamily::NonDmd, true);
      REQUIRE(crtWithDefault.size() == 2);
      CHECK(crtWithDefault[0].isDefault);
      CHECK(crtWithDefault[0].defaultDmdOnly == 0);
      CHECK(builder.MakeDisplayURI(crtWithDefault[0]) == "ctrl://default/display?dmd_only=0");

      const auto anyWithDefault = builder.GetDisplayItems({}, DisplayFamily::Any, true);
      REQUIRE(anyWithDefault.size() == 3);
      CHECK(anyWithDefault[0].isDefault);
      CHECK(anyWithDefault[0].defaultDmdOnly == -1);
      CHECK(builder.MakeDisplayURI(anyWithDefault[0]) == "ctrl://default/display");

      // The pseudo item follows the text filter and the pseudo plugin shows up in the plugin list
      CHECK(builder.GetDisplayItems("zzz", DisplayFamily::Dmd, true).empty());
      const auto plugins = builder.GetPlugins(true);
      REQUIRE(plugins.size() == 4);
      CHECK(plugins[0].id == "default");
      CHECK(plugins[0].isDefaultDisplay);
      CHECK(plugins[0].loaded);
      CHECK(builder.GetPlugins(false).size() == 3);
   }

   SUBCASE("plugin enable callback and load/unload")
   {
      int enableCalls = 0;
      builder.onPluginEnableChanged = [&enableCalls](const std::string& id, bool enabled)
      {
         ++enableCalls;
         CHECK(id == "testaux");
         CHECK(enabled == (enableCalls == 1));
      };
      CHECK(builder.LoadPlugin("testaux"));
      CHECK(aux->IsLoaded());
      CHECK(builder.UnloadPlugin("testaux"));
      CHECK_FALSE(aux->IsLoaded());
      CHECK(enableCalls == 2);
      CHECK_FALSE(builder.LoadPlugin("nothere"));
   }

   pm.UnloadPlugin(*ctl);
}
