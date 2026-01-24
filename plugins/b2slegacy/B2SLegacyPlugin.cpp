// license:GPLv3+

#include "common.h"
#include "plugins/MsgPlugin.h"
#include "plugins/ScriptablePlugin.h"
#include "plugins/LoggingPlugin.h"
#include "plugins/VPXPlugin.h"

#include "Server.h"
#include "utils/DMDOverlay.h"

namespace B2SLegacy {

LPI_IMPLEMENT

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId = 0;
static unsigned int getVpxApiId = 0;
static unsigned int onPluginLoaded = 0;
static unsigned int onPluginUnloaded = 0;
static Server* server = nullptr;
static bool serverRegistered = false;

///////////////////////////////////////////////////////////////////////////////
// Script interface
//

static ScriptClassDef* pinmameClassDef = nullptr;
static int pinmameMemberStartIndex = 0;

PSC_CLASS_START(Server)
   PSC_FUNCTION0(Server, void, Dispose)
   PSC_PROP_R(Server, string, B2SServerVersion)
   PSC_PROP_R(Server, double, B2SBuildVersion)
   PSC_PROP_R(Server, string, B2SServerDirectory)
   PSC_PROP_RW(Server, string, B2SName)
   PSC_PROP_RW(Server, string, TableName)
   PSC_PROP_W(Server, string, WorkingDir)
   PSC_FUNCTION1(Server, void, SetPath, string)
   PSC_PROP_RW(Server, bool, LaunchBackglass)
   PSC_PROP_RW(Server, bool, PuPHide)
   PSC_FUNCTION2(Server, void, B2SSetData, int, int)
   PSC_FUNCTION2(Server, void, B2SSetData, string, int)
   PSC_FUNCTION1(Server, void, B2SPulseData, int)
   PSC_FUNCTION1(Server, void, B2SPulseData, string)
   PSC_FUNCTION3(Server, void, B2SSetPos, int, int, int)
   PSC_FUNCTION3(Server, void, B2SSetPos, string, int, int)
   PSC_FUNCTION2(Server, void, B2SSetIllumination, string, int)
   PSC_FUNCTION2(Server, void, B2SSetLED, int, int)
   PSC_FUNCTION2(Server, void, B2SSetLED, int, string)
   PSC_FUNCTION2(Server, void, B2SSetLEDDisplay, int, string)
   PSC_FUNCTION2(Server, void, B2SSetReel, int, int)
   PSC_FUNCTION2(Server, void, B2SSetScore, int, int)
   PSC_FUNCTION2(Server, void, B2SSetScorePlayer, int, int)
   PSC_FUNCTION1(Server, void, B2SSetScorePlayer1, int)
   PSC_FUNCTION1(Server, void, B2SSetScorePlayer2, int)
   PSC_FUNCTION1(Server, void, B2SSetScorePlayer3, int)
   PSC_FUNCTION1(Server, void, B2SSetScorePlayer4, int)
   PSC_FUNCTION1(Server, void, B2SSetScorePlayer5, int)
   PSC_FUNCTION1(Server, void, B2SSetScorePlayer6, int)
   PSC_FUNCTION2(Server, void, B2SSetScoreDigit, int, int)
   PSC_FUNCTION2(Server, void, B2SSetScoreRollover, int, int)
   PSC_FUNCTION1(Server, void, B2SSetScoreRolloverPlayer1, int)
   PSC_FUNCTION1(Server, void, B2SSetScoreRolloverPlayer2, int)
   PSC_FUNCTION1(Server, void, B2SSetScoreRolloverPlayer3, int)
   PSC_FUNCTION1(Server, void, B2SSetScoreRolloverPlayer4, int)
   PSC_FUNCTION1(Server, void, B2SSetCredits, int)
   PSC_FUNCTION2(Server, void, B2SSetCredits, int, int)
   PSC_FUNCTION1(Server, void, B2SSetPlayerUp, int)
   PSC_FUNCTION2(Server, void, B2SSetPlayerUp, int, int)
   PSC_FUNCTION1(Server, void, B2SSetCanPlay, int)
   PSC_FUNCTION2(Server, void, B2SSetCanPlay, int, int)
   PSC_FUNCTION1(Server, void, B2SSetBallInPlay, int)
   PSC_FUNCTION2(Server, void, B2SSetBallInPlay, int, int)
   PSC_FUNCTION1(Server, void, B2SSetTilt, int)
   PSC_FUNCTION2(Server, void, B2SSetTilt, int, int)
   PSC_FUNCTION1(Server, void, B2SSetMatch, int)
   PSC_FUNCTION2(Server, void, B2SSetMatch, int, int)
   PSC_FUNCTION1(Server, void, B2SSetGameOver, int)
   PSC_FUNCTION2(Server, void, B2SSetGameOver, int, int)
   PSC_FUNCTION1(Server, void, B2SSetShootAgain, int)
   PSC_FUNCTION2(Server, void, B2SSetShootAgain, int, int)
   PSC_FUNCTION1(Server, void, B2SStartAnimation, string)
   PSC_FUNCTION2(Server, void, B2SStartAnimation, string, bool)
   PSC_FUNCTION1(Server, void, B2SStartAnimationReverse, string)
   PSC_FUNCTION1(Server, void, B2SStopAnimation, string)
   PSC_FUNCTION0(Server, void, B2SStopAllAnimations)
   PSC_FUNCTION1(Server, bool, B2SIsAnimationRunning, string)
   PSC_FUNCTION1(Server, void, StartAnimation, string)
   PSC_FUNCTION2(Server, void, StartAnimation, string, bool)
   PSC_FUNCTION1(Server, void, StopAnimation, string)
   PSC_FUNCTION0(Server, void, B2SStartRotation)
   PSC_FUNCTION0(Server, void, B2SStopRotation)
   PSC_FUNCTION0(Server, void, B2SShowScoreDisplays)
   PSC_FUNCTION0(Server, void, B2SHideScoreDisplays)
   PSC_FUNCTION1(Server, void, B2SStartSound, string)
   PSC_FUNCTION1(Server, void, B2SPlaySound, string)
   PSC_FUNCTION1(Server, void, B2SStopSound, string)
   PSC_FUNCTION2(Server, void, B2SMapSound, int, string)
   PSC_PROP_R(Server, string, VPMBuildVersion)
   PSC_PROP_RW(Server, bool, LockDisplay)

   if (pinmameClassDef)
   {
      pinmameMemberStartIndex = static_cast<int>(members.size());
      for (unsigned int i = 0; i < pinmameClassDef->nMembers; i++)
      {
         ScriptClassMemberDef member = pinmameClassDef->members[i];
         member.Call = [](void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
            {
               static_cast<Server*>(me)->ForwardPinMAMECall(memberIndex - pinmameMemberStartIndex, pArgs, pRet);
            };
         members.push_back(member);
      }
   }
PSC_CLASS_END(Server)

static void OnPluginLoaded(const unsigned int, void*, void* msgData)
{
   const char* pluginId = static_cast<const char*>(msgData);
   if (strcmp(pluginId, "PinMAME") == 0 && !serverRegistered)
   {
      pinmameClassDef = scriptApi->GetClassDef("Controller");
      if (pinmameClassDef)
      {
         auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
         RegisterServerSCD(regLambda);
         Server_SCD->CreateObject = []()
         {
            assert(server == nullptr);
            server = new Server(msgApi, endpointId, vpxApi, pinmameClassDef, pinmameMemberStartIndex);
            server->SetOnDestroyHandler(
               [](Server* pServer)
               {
                  assert(server == pServer);
                  server = nullptr;
               });
            return static_cast<void*>(server);
         };
         scriptApi->SubmitTypeLibrary();
         scriptApi->SetCOMObjectOverride("B2S.Server", Server_SCD);
         serverRegistered = true;
      }
   }
}

static void OnPluginUnloaded(const unsigned int, void*, void* msgData)
{
   const char* pluginId = static_cast<const char*>(msgData);
   if (strcmp(pluginId, "PinMAME") == 0)
   {
      pinmameClassDef = nullptr;
   }
}

}

using namespace B2SLegacy;

MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);

   getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);

   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);

   msgApi->SubscribeMsg(endpointId, onPluginLoaded = msgApi->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_LOADED), OnPluginLoaded, nullptr);
   msgApi->SubscribeMsg(endpointId, onPluginUnloaded = msgApi->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_UNLOADED), OnPluginUnloaded, nullptr);

   DMDOverlay::RegisterSettings(msgApi, endpointId);

   OnPluginLoaded(0, nullptr, const_cast<char*>("PinMAME"));
}

MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginUnload()
{
   assert(server == nullptr);
   serverRegistered = false;
   scriptApi->SetCOMObjectOverride("B2S.Server", nullptr);

   msgApi->UnsubscribeMsg(onPluginLoaded, OnPluginLoaded);
   msgApi->UnsubscribeMsg(onPluginUnloaded, OnPluginUnloaded);
   msgApi->ReleaseMsgID(onPluginLoaded);
   msgApi->ReleaseMsgID(onPluginUnloaded);
   msgApi->ReleaseMsgID(getVpxApiId);

   vpxApi = nullptr;
   scriptApi = nullptr;
   msgApi = nullptr;
}
