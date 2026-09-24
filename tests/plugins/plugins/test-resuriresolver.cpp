// license:GPLv3+

#include "core/stdafx.h"
#include "../../vpx-test.h"
#include "doctest.h"

#include "plugins/MsgPluginManager.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/ResURIResolver.h"

using namespace MsgPI;
using namespace PinballPlugin;
using namespace PinballPlugin::Controller;

namespace
{

// Fake controller plugin exposing one display chain (raw + override), one LCD display,
// one segment display and one state group through the CtrlItemProvider helpers
struct FakeController
{
   const MsgPluginAPI* api = nullptr;
   uint32_t endpoint = 0;
   std::unique_ptr<CtrlItemProvider<DisplaySrcId>> displays;
   std::unique_ptr<CtrlItemProvider<SegSrcId>> segs;
   std::unique_ptr<CtrlItemProvider<StateSrcId>> states;

   unsigned int frameId = 7;
   uint8_t dmdFrame[8] = {};
   float lumFrame[8] = {};
   uint8_t lcdFrame[8] = {};
   float segFrame[3 * 16] = {};
   float stateValueA = 0.25f;
   float stateValueB = 0.75f;
   uint8_t stateValueInt = 3;
   int getStateCalls = 0;
};

FakeController g_ctl;

CtlResId ResId(uint32_t endpoint, uint32_t res)
{
   CtlResId id;
   id.endpointId = endpoint;
   id.resId = res;
   return id;
}

DisplayFrame MSGPIAPI GetDmdFrame(void* ctx) { return { static_cast<FakeController*>(ctx)->frameId, static_cast<FakeController*>(ctx)->dmdFrame }; }
DisplayFrame MSGPIAPI GetLumFrame(void* ctx) { return { static_cast<FakeController*>(ctx)->frameId, static_cast<FakeController*>(ctx)->lumFrame }; }
DisplayFrame MSGPIAPI GetLcdFrame(void* ctx) { return { static_cast<FakeController*>(ctx)->frameId, static_cast<FakeController*>(ctx)->lcdFrame }; }
SegDisplayFrame MSGPIAPI GetSegFrame(void* ctx) { return { 3, static_cast<FakeController*>(ctx)->segFrame }; }
void MSGPIAPI GetStateA(void* ctx, void* result)
{
   ++static_cast<FakeController*>(ctx)->getStateCalls;
   *static_cast<float*>(result) = static_cast<FakeController*>(ctx)->stateValueA;
}
void MSGPIAPI GetStateB(void* ctx, void* result) { *static_cast<float*>(result) = static_cast<FakeController*>(ctx)->stateValueB; }
void MSGPIAPI GetStateInt(void* ctx, void* result) { *static_cast<uint8_t*>(result) = static_cast<FakeController*>(ctx)->stateValueInt; }

void ControllerLoad(const uint32_t endpointId, const MsgPluginAPI* api)
{
   g_ctl.api = api;
   g_ctl.endpoint = endpointId;
   g_ctl.displays = std::make_unique<CtrlItemProvider<DisplaySrcId>>(api, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   g_ctl.segs = std::make_unique<CtrlItemProvider<SegSrcId>>(api, endpointId, CTLPI_SEG_GET_SRC_MSG, CTLPI_SEG_ON_SRC_CHG_MSG);
   g_ctl.states = std::make_unique<CtrlItemProvider<StateSrcId>>(api, endpointId, CTLPI_STATE_GET_SRC_MSG, CTLPI_STATE_ON_SRC_CHG_MSG);
}

void ControllerUnload()
{
   g_ctl.displays.reset();
   g_ctl.segs.reset();
   g_ctl.states.reset();
}

void NoopLoad(const uint32_t, const MsgPluginAPI*) { }
void NoopUnload() { }

DisplaySrcId MakeDisplay(
   uint32_t endpoint, uint32_t resId, uint32_t overrideResId, unsigned int w, unsigned int h, uint32_t hardware, unsigned int format, DisplayFrame(MSGPIAPI* getFrame)(void*))
{
   DisplaySrcId src {};
   src.id = ResId(endpoint, resId);
   src.overrideId = overrideResId != 0 ? ResId(endpoint, overrideResId) : CtlResId {};
   src.width = w;
   src.height = h;
   src.hardware = hardware;
   src.callContext = &g_ctl;
   src.frameFormat = format;
   src.GetRenderFrame = getFrame;
   return src;
}

} // namespace

TEST_CASE("ResURIResolver ctrl URI resolution")
{
   MsgPluginManager pm;
   pm.SetSettingsHandler([](const std::string&, MsgPluginManager::SettingAction, MsgSettingDef*) { });

   auto ctl = pm.RegisterPlugin("testctl", "Test Controller", "", "", "", "", ControllerLoad, ControllerUnload);
   auto host = pm.RegisterPlugin("testhost", "Test Host", "", "", "", "", NoopLoad, NoopUnload);
   pm.LoadPlugin(*ctl);

   for (int i = 0; i < 3 * 16; i++)
      g_ctl.segFrame[i] = static_cast<float>(i);

   // A: raw DMD, B: override of A, C: LCD display (excluded by dmd_only)
   g_ctl.displays->AddItem(MakeDisplay(ctl->m_endpointId, 1, 0, 128, 32, CTLPI_DISPLAY_HARDWARE_RGB_LED, CTLPI_DISPLAY_FORMAT_SRGB888, GetDmdFrame));
   g_ctl.displays->AddItem(MakeDisplay(ctl->m_endpointId, 2, 1, 128, 32, CTLPI_DISPLAY_HARDWARE_RGB_LED, CTLPI_DISPLAY_FORMAT_LUM32F, GetLumFrame));
   g_ctl.displays->AddItem(MakeDisplay(ctl->m_endpointId, 3, 0, 256, 64, CTLPI_DISPLAY_HARDWARE_LCD_DISPLAY, CTLPI_DISPLAY_FORMAT_SRGB888, GetLcdFrame));

   SegSrcId segSrc {};
   segSrc.id = ResId(ctl->m_endpointId, 1);
   segSrc.groupId = ResId(ctl->m_endpointId, 0);
   segSrc.hardware = CTLPI_SEG_HARDWARE_VFD_GREEN;
   segSrc.nElements = 3;
   segSrc.elementType[0] = CTLPI_SEG_LAYOUT_16;
   segSrc.elementType[1] = CTLPI_SEG_LAYOUT_14;
   segSrc.elementType[2] = CTLPI_SEG_LAYOUT_7;
   segSrc.callContext = &g_ctl;
   segSrc.GetState = GetSegFrame;
   g_ctl.segs->AddItem(segSrc);

   static StateDef stateDefs[3];
   stateDefs[0] = { "sol11", "Solenoid 11", 11, CTLPI_STATE_FORMAT_FLOAT, CTLPI_STATE_TYPE_CUSTOM, &g_ctl, GetStateA, nullptr };
   stateDefs[1] = { "gi1", "GI string 1", 1, CTLPI_STATE_FORMAT_FLOAT, CTLPI_STATE_TYPE_RELATIVE_BRIGHTNESS, &g_ctl, GetStateB, nullptr };
   stateDefs[2] = { "sw7", "Switch 7", 7, CTLPI_STATE_FORMAT_UINT8, CTLPI_STATE_TYPE_SWITCH, &g_ctl, GetStateInt, nullptr };
   StateSrcId stateSrc {};
   stateSrc.id = ResId(ctl->m_endpointId, 1);
   stateSrc.nStates = 3;
   stateSrc.stateDefs = stateDefs;
   g_ctl.states->AddItem(stateSrc);

   ResURIResolver resolver(pm.GetMsgAPI(), host->m_endpointId, true, true, true);

   SUBCASE("display URIs")
   {
      // Resolving a source follows the override chain to its tail
      ResURIResolver::DisplayState dmd = resolver.GetDisplayState("ctrl://testctl/display?id=1");
      REQUIRE(dmd.source != nullptr);
      CHECK(dmd.source->id.resId == 2);
      CHECK(dmd.state.frame == g_ctl.lumFrame);
      CHECK(dmd.state.frameId == g_ctl.frameId);

      // No id selects the first source of the endpoint (again through overrides)
      CHECK(resolver.GetDisplayState("ctrl://testctl/display").state.frame == g_ctl.lumFrame);

      // LCD display has no override
      CHECK(resolver.GetDisplayState("ctrl://testctl/display?id=3").state.frame == g_ctl.lcdFrame);

      // Default picks the largest display
      ResURIResolver::DisplayState any = resolver.GetDisplayState("ctrl://default/display");
      REQUIRE(any.source != nullptr);
      CHECK(any.source->id.resId == 3);

      // dmd_only filters out CRT/LCD families
      ResURIResolver::DisplayState dmdOnly = resolver.GetDisplayState("ctrl://default/display?dmd_only");
      REQUIRE(dmdOnly.source != nullptr);
      CHECK(dmdOnly.source->id.resId == 2);

      // Failures resolve to an empty state
      CHECK(resolver.GetDisplayState("ctrl://unknown/display").source == nullptr);
      CHECK(resolver.GetDisplayState("ctrl://testctl/display?id=9").source == nullptr);
      CHECK(resolver.GetDisplayState("ctrl://testctl/state").source == nullptr);
      CHECK(resolver.GetDisplayState("not an uri").source == nullptr);
      CHECK(resolver.GetDisplayState("http://testctl/display").source == nullptr);
   }

   SUBCASE("segment display URIs")
   {
      ResURIResolver::SegDisplayState seg = resolver.GetSegDisplayState("ctrl://testctl/seg?id=1");
      REQUIRE(seg.source != nullptr);
      CHECK(seg.source->nElements == 3);
      CHECK(seg.state.frame == g_ctl.segFrame);
      CHECK(seg.state.frameId == 3);

      // 'sub' extracts a single element, exposed as a 1 element source pointing inside the frame
      ResURIResolver::SegDisplayState sub = resolver.GetSegDisplayState("ctrl://testctl/seg?id=1&sub=2");
      REQUIRE(sub.source != nullptr);
      CHECK(sub.source->nElements == 1);
      CHECK(sub.source->elementType[0] == CTLPI_SEG_LAYOUT_7);
      CHECK(sub.state.frame == g_ctl.segFrame + 2 * 16);

      CHECK(resolver.GetSegDisplayState("ctrl://testctl/seg?id=9").source == nullptr);
      CHECK(resolver.GetSegDisplayState("ctrl://testctl/seg?id=1&sub=9").source == nullptr);
      CHECK(resolver.GetSegDisplayState("ctrl://unknown/seg?id=1").source == nullptr);
   }

   SUBCASE("float state URIs")
   {
      CHECK(resolver.GetFloatState("ctrl://testctl/state?group=1&mapping=11") == doctest::Approx(0.25f));
      CHECK(resolver.GetFloatState("ctrl://testctl/state?group=1&name=gi1") == doctest::Approx(0.75f));

      // The cached lambda reads the live state, not a stale copy
      g_ctl.stateValueA = 1.f;
      CHECK(resolver.GetFloatState("ctrl://testctl/state?group=1&mapping=11") == doctest::Approx(1.f));

      // Non float states, unknown groups and unknown plugins resolve to 0
      CHECK(resolver.GetFloatState("ctrl://testctl/state?group=1&mapping=7") == 0.f);
      CHECK(resolver.GetFloatState("ctrl://testctl/state?group=2&mapping=11") == 0.f);
      CHECK(resolver.GetFloatState("ctrl://unknown/state?group=1&mapping=11") == 0.f);
      CHECK(resolver.GetFloatState("ctrl://default/state?group=1&mapping=11") == 0.f);
      CHECK(resolver.GetFloatState("garbage") == 0.f);
   }

   SUBCASE("source list updates are tracked")
   {
      const std::string dump = resolver.DumpDisplaySources();
      CHECK(dump.find("128x32") != std::string::npos);

      // A source added later is visible without recreating the resolver
      g_ctl.displays->AddItem(MakeDisplay(ctl->m_endpointId, 4, 0, 32, 16, CTLPI_DISPLAY_HARDWARE_NEON_PLASMA, CTLPI_DISPLAY_FORMAT_LUM32F, GetLumFrame));
      CHECK(resolver.GetDisplayState("ctrl://testctl/display?id=4").source != nullptr);
      CHECK(resolver.DumpDisplaySources().find("32x16") != std::string::npos);
   }

   pm.UnloadPlugin(*ctl);
}
