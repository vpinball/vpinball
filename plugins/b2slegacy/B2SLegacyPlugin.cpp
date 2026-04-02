// license:GPLv3+

#include "common.h"
#include "plugins/MsgPlugin.h"
#include "plugins/ScriptablePlugin.h"
#include "plugins/LoggingPlugin.h"
#include "plugins/VPXPlugin.h"

#include "Server.h"
#include "utils/DMDOverlay.h"

namespace B2SLegacy {

LPI_IMPLEMENT_CPP // Implement shared log support

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId = 0;
static unsigned int getVpxApiId = 0;
static unsigned int getScriptApiId = 0;
static int nServer = 0;

///////////////////////////////////////////////////////////////////////////////
// Script interface
//

PSC_ARRAY1(B2SLegacy_ByteArray, uint8, 0)
PSC_ARRAY1(B2SLegacy_IntArray, int32, 0)
PSC_ARRAY2(B2SLegacy_StructArray, int32, 0, 0)

// Proxy class for PinMAME GameSettings
static std::unique_ptr<ScriptablePlugin::ScriptClassProxy> m_gameSettingsProxy;
class GameSettings { PSC_IMPLEMENT_REFCOUNT() }; // Dummy class as we directly use the proxied object
PSC_CLASS_START(B2SLegacy_GameSettings, GameSettings)
   members.clear();
   PSC_PROXY_PROP_R(m_gameSettingsProxy.get(), uint32, AddRef)
   PSC_PROXY_PROP_R(m_gameSettingsProxy.get(), uint32, Release)
   PSC_PROXY_PROP_RW_ARRAY1(m_gameSettingsProxy.get(), int, Value, string)
PSC_CLASS_END()

// Proxy class for PinMAME Game
static std::unique_ptr<ScriptablePlugin::ScriptClassProxy> m_gameProxy;
class Game { PSC_IMPLEMENT_REFCOUNT() }; // Dummy class as we directly use the proxied object
PSC_CLASS_START(B2SLegacy_Game, Game)
   members.clear();
   PSC_PROXY_PROP_R(m_gameProxy.get(), uint32, AddRef)
   PSC_PROXY_PROP_R(m_gameProxy.get(), uint32, Release)
   PSC_PROXY_PROP_R(m_gameProxy.get(), string, Version)
   PSC_PROXY_PROP_R(m_gameProxy.get(), string, Name)
   PSC_PROXY_PROP_R(m_gameProxy.get(), string, Description)
   PSC_PROXY_PROP_R(m_gameProxy.get(), string, Year)
   PSC_PROXY_PROP_R(m_gameProxy.get(), string, Manufacturer)
   PSC_PROXY_PROP_R(m_gameProxy.get(), string, CloneOf)
   PSC_PROXY_PROP_R(m_gameProxy.get(), B2SLegacy_GameSettings, Settings)
PSC_CLASS_END()

PSC_CLASS_START(B2SLegacy_Server, Server)
   PSC_FUNCTION0(void, Dispose)
   PSC_PROP_R(string, B2SServerVersion)
   PSC_PROP_R(double, B2SBuildVersion)
   PSC_PROP_R(string, B2SServerDirectory)
   PSC_PROP_RW(string, B2SName)
   PSC_PROP_RW(string, TableName)
   PSC_PROP_W(string, WorkingDir)
   PSC_FUNCTION1(void, SetPath, string)
   PSC_PROP_RW(bool, LaunchBackglass)
   PSC_PROP_RW(bool, PuPHide)
   PSC_FUNCTION2(void, B2SSetData, int, int)
   PSC_FUNCTION2(void, B2SSetData, string, int)
   PSC_FUNCTION1(void, B2SPulseData, int)
   PSC_FUNCTION1(void, B2SPulseData, string)
   PSC_FUNCTION3(void, B2SSetPos, int, int, int)
   PSC_FUNCTION3(void, B2SSetPos, string, int, int)
   PSC_FUNCTION2(void, B2SSetIllumination, string, int)
   PSC_FUNCTION2(void, B2SSetLED, int, int)
   PSC_FUNCTION2(void, B2SSetLED, int, string)
   PSC_FUNCTION2(void, B2SSetLEDDisplay, int, string)
   PSC_FUNCTION2(void, B2SSetReel, int, int)
   PSC_FUNCTION2(void, B2SSetScore, int, int)
   PSC_FUNCTION2(void, B2SSetScorePlayer, int, int)
   PSC_FUNCTION1(void, B2SSetScorePlayer1, int)
   PSC_FUNCTION1(void, B2SSetScorePlayer2, int)
   PSC_FUNCTION1(void, B2SSetScorePlayer3, int)
   PSC_FUNCTION1(void, B2SSetScorePlayer4, int)
   PSC_FUNCTION1(void, B2SSetScorePlayer5, int)
   PSC_FUNCTION1(void, B2SSetScorePlayer6, int)
   PSC_FUNCTION2(void, B2SSetScoreDigit, int, int)
   PSC_FUNCTION2(void, B2SSetScoreRollover, int, int)
   PSC_FUNCTION1(void, B2SSetScoreRolloverPlayer1, int)
   PSC_FUNCTION1(void, B2SSetScoreRolloverPlayer2, int)
   PSC_FUNCTION1(void, B2SSetScoreRolloverPlayer3, int)
   PSC_FUNCTION1(void, B2SSetScoreRolloverPlayer4, int)
   PSC_FUNCTION1(void, B2SSetCredits, int)
   PSC_FUNCTION2(void, B2SSetCredits, int, int)
   PSC_FUNCTION1(void, B2SSetPlayerUp, int)
   PSC_FUNCTION2(void, B2SSetPlayerUp, int, int)
   PSC_FUNCTION1(void, B2SSetCanPlay, int)
   PSC_FUNCTION2(void, B2SSetCanPlay, int, int)
   PSC_FUNCTION1(void, B2SSetBallInPlay, int)
   PSC_FUNCTION2(void, B2SSetBallInPlay, int, int)
   PSC_FUNCTION1(void, B2SSetTilt, int)
   PSC_FUNCTION2(void, B2SSetTilt, int, int)
   PSC_FUNCTION1(void, B2SSetMatch, int)
   PSC_FUNCTION2(void, B2SSetMatch, int, int)
   PSC_FUNCTION1(void, B2SSetGameOver, int)
   PSC_FUNCTION2(void, B2SSetGameOver, int, int)
   PSC_FUNCTION1(void, B2SSetShootAgain, int)
   PSC_FUNCTION2(void, B2SSetShootAgain, int, int)
   PSC_FUNCTION1(void, B2SStartAnimation, string)
   PSC_FUNCTION2(void, B2SStartAnimation, string, bool)
   PSC_FUNCTION1(void, B2SStartAnimationReverse, string)
   PSC_FUNCTION1(void, B2SStopAnimation, string)
   PSC_FUNCTION0(void, B2SStopAllAnimations)
   PSC_FUNCTION1(bool, B2SIsAnimationRunning, string)
   PSC_FUNCTION1(void, StartAnimation, string)
   PSC_FUNCTION2(void, StartAnimation, string, bool)
   PSC_FUNCTION1(void, StopAnimation, string)
   PSC_FUNCTION0(void, B2SStartRotation)
   PSC_FUNCTION0(void, B2SStopRotation)
   PSC_FUNCTION0(void, B2SShowScoreDisplays)
   PSC_FUNCTION0(void, B2SHideScoreDisplays)
   PSC_FUNCTION1(void, B2SStartSound, string)
   PSC_FUNCTION1(void, B2SPlaySound, string)
   PSC_FUNCTION1(void, B2SStopSound, string)
   PSC_FUNCTION2(void, B2SMapSound, int, string)
   PSC_PROP_R(string, VPMBuildVersion)
   PSC_PROP_RW(bool, LockDisplay)

   // B2SServer declares a second controller API by mirroring PinMAME controller
   // This is horribly hacky but needed for backward compatibility

   // Overall setup
   PSC_PROXY_PROP_R(me, string, Version)
   PSC_PROXY_PROP_RW(me, string, GameName)
   PSC_PROXY_PROP_R(me, string, ROMName)
   PSC_PROXY_PROP_RW(me, string, SplashInfoLine)
   PSC_PROXY_PROP_RW(me, bool, ShowTitle)
   PSC_PROXY_PROP_RW(me, bool, HandleKeyboard)
   PSC_PROXY_PROP_RW(me, bool, HandleMechanics)
   PSC_PROXY_PROP_RW_ARRAY1(me, int32, SolMask, int)
   // Run/Pause/Stop
   PSC_PROXY_FUNCTION0(me, void, Run)
   PSC_PROXY_FUNCTION1(me, void, Run, int32)
   PSC_PROXY_FUNCTION2(me, void, Run, int32, int)
   PSC_PROXY_PROP_W(me, double, TimeFence)
   PSC_PROXY_PROP_R(me, bool, Running)
   PSC_PROXY_PROP_RW(me, bool, Pause)
   PSC_PROXY_FUNCTION0(me, void, Stop)
   PSC_PROXY_PROP_RW(me, bool, Hidden)
   // Emulated machine state access
   PSC_PROXY_PROP_RW_ARRAY1(me, bool, Switch, int)
   PSC_PROXY_PROP_W_ARRAY1(me, int32, Mech, int)
   PSC_PROXY_PROP_R_ARRAY1(me, int32, GetMech, int)
   PSC_PROXY_PROP_R_ARRAY1(me, bool, Lamp, int)
   PSC_PROXY_PROP_R_ARRAY1(me, bool, Solenoid, int)
   PSC_PROXY_PROP_R_ARRAY1(me, int32, GIString, int)
   PSC_PROXY_PROP_RW_ARRAY1(me, int32, Dip, int)
   PSC_PROXY_PROP_R(me, B2SLegacy_ByteArray, NVRAM)
   PSC_PROXY_PROP_R(me, B2SLegacy_StructArray, ChangedNVRAM);
   PSC_PROXY_PROP_R(me, B2SLegacy_StructArray, ChangedLamps);
   PSC_PROXY_PROP_R(me, B2SLegacy_StructArray, ChangedGIStrings);
   PSC_PROXY_PROP_R(me, B2SLegacy_StructArray, ChangedSolenoids);
   PSC_PROXY_PROP_R(me, B2SLegacy_StructArray, NewSoundCommands);
   PSC_PROXY_PROP_R_ARRAY2(me, B2SLegacy_StructArray, ChangedLEDs, int, int)
   PSC_PROXY_PROP_R_ARRAY3(me, B2SLegacy_StructArray, ChangedLEDs, int, int, int)
   PSC_PROXY_PROP_R_ARRAY4(me, B2SLegacy_StructArray, ChangedLEDs, int, int, int, int)
   PSC_PROXY_PROP_R(me, int, RawDmdWidth)
   PSC_PROXY_PROP_R(me, int, RawDmdHeight)
   PSC_PROXY_PROP_R(me, B2SLegacy_ByteArray, RawDmdPixels)
   PSC_PROXY_PROP_R(me, B2SLegacy_IntArray, RawDmdColoredPixels)
   // Overall information
   PSC_PROXY_PROP_R_ARRAY1(me, B2SLegacy_Game, Games, string)
   // Deprecated properties
   PSC_PROXY_PROP_RW(me, bool, DoubleSize)
   PSC_PROXY_PROP_RW(me, bool, LockDisplay)
   PSC_PROXY_PROP_RW(me, bool, ShowFrame)
   PSC_PROXY_PROP_RW(me, bool, ShowDMDOnly)
   PSC_PROXY_PROP_RW(me, bool, ShowTitle)
   PSC_PROXY_PROP_RW(me, int, FastFrames)
   PSC_PROXY_PROP_RW(me, bool, IgnoreRomCrc)
   PSC_PROXY_PROP_RW(me, bool, CabinetMode)
   PSC_PROXY_PROP_RW(me, int, SoundMode)
   PSC_PROXY_FUNCTION0(me, void, ShowOptsDialog)
   PSC_PROXY_FUNCTION1(me, void, ShowOptsDialog, int32)
   // Custom property to allow host to identify the object as the plugin version
   PSC_PROXY_PROP_R(me, bool, IsPlugin)
PSC_CLASS_END()

}

using namespace B2SLegacy;

MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);

   getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);

   getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);

   DMDOverlay::RegisterSettings(msgApi, endpointId);

   nServer = 0;
   auto classLambda = [](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   auto arrayLambda = [](ScriptArrayDef* sad) { scriptApi->RegisterScriptArrayType(sad); };
   RegisterB2SLegacy_Server(classLambda);
   RegisterB2SLegacy_GameSettings(classLambda);
   RegisterB2SLegacy_Game(classLambda);
   RegisterB2SLegacy_ByteArray(arrayLambda);
   RegisterB2SLegacy_IntArray(arrayLambda);
   RegisterB2SLegacy_StructArray(arrayLambda);
   m_gameProxy = std::make_unique<ScriptablePlugin::ScriptClassProxy>(msgApi, endpointId, "PinMAME_", "PinMAME_Game", "B2SLegacy_", B2SLegacy_Game_SCD);
   m_gameSettingsProxy = std::make_unique<ScriptablePlugin::ScriptClassProxy>(msgApi, endpointId, "PinMAME_", "PinMAME_GameSettings", "B2SLegacy_", B2SLegacy_GameSettings_SCD);
   B2SLegacy_Server_SCD->CreateObject = []()
   {
      if (nServer > 0)
         LOGE("Invalid script: multiple B2S server are created."s);
      nServer++;
      Server* server = new Server(msgApi, endpointId, vpxApi, B2SLegacy_Server_SCD);
      server->SetOnDestroyHandler([](Server*) { nServer--; });
      return static_cast<void*>(server);
   };
   scriptApi->SubmitTypeLibrary(endpointId);
   scriptApi->SetCOMObjectOverride("B2S.Server", B2SLegacy_Server_SCD);
}

MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginUnload()
{
   assert(nServer == 0);
   scriptApi->SetCOMObjectOverride("B2S.Server", nullptr);
   auto classLambda = [](ScriptClassDef* scd) { scriptApi->UnregisterScriptClass(scd); };
   auto arrayLambda = [](ScriptArrayDef* sad) { scriptApi->UnregisterScriptArrayType(sad); };
   m_gameProxy = nullptr;
   m_gameSettingsProxy = nullptr;
   UnregisterB2SLegacy_ByteArray(arrayLambda);
   UnregisterB2SLegacy_IntArray(arrayLambda);
   UnregisterB2SLegacy_StructArray(arrayLambda);
   UnregisterB2SLegacy_Game(classLambda);
   UnregisterB2SLegacy_GameSettings(classLambda);
   UnregisterB2SLegacy_Server(classLambda);
   
   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(getScriptApiId);
   vpxApi = nullptr;
   scriptApi = nullptr;
   msgApi = nullptr;
}
