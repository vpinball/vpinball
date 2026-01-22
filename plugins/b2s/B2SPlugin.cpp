// license:GPLv3+

#include "common.h"

#include "plugins/MsgPlugin.h"

#include "B2SServer.h"

namespace B2S {
   
LPI_IMPLEMENT // Implement shared log support

///////////////////////////////////////////////////////////////////////////////
// B2S plugin
// - implement rendering of directb2s backglass and score view
// - implement COM B2S Server which can be used as a game controller,
//   in turn, eventually using PinMAME plugin if requested

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId;
static unsigned int getVpxApiId, onPluginLoaded, onPluginUnloaded;
static bool serverRegistered = false;
static std::thread::id apiThread;

static B2SServer* server = nullptr;

///////////////////////////////////////////////////////////////////////////////
// Script interface
//

static ScriptClassDef* pinmameClassDef = nullptr;
static int pinmameMemberStartIndex = 0;

PSC_CLASS_START(B2SServer)
   PSC_FUNCTION0(B2SServer, void, Dispose)
   PSC_PROP_RW(B2SServer, string, TableName)
   PSC_PROP_W(B2SServer, string, WorkingDir)
   PSC_FUNCTION1(B2SServer, void, SetPath, string)
   PSC_PROP_R(B2SServer, string, VPMBuildVersion)
   PSC_PROP_RW(B2SServer, bool, LaunchBackglass)
   PSC_PROP_RW(B2SServer, bool, LockDisplay)
   PSC_PROP_RW(B2SServer, bool, PuPHide)
   PSC_PROP_RW(B2SServer, string, B2SName)
   PSC_PROP_R(B2SServer, string, B2SServerVersion)
   PSC_PROP_R(B2SServer, double, B2SBuildVersion)
   PSC_PROP_R(B2SServer, string, B2SServerDirectory)
   PSC_FUNCTION2(B2SServer, void, B2SSetData, int, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetData, string, int)
   PSC_FUNCTION1(B2SServer, void, B2SPulseData, int)
   PSC_FUNCTION1(B2SServer, void, B2SPulseData, string)
   PSC_FUNCTION3(B2SServer, void, B2SSetPos, int, int, int)
   PSC_FUNCTION3(B2SServer, void, B2SSetPos, string, int, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetIllumination, string, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetLED, int, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetLED, int, string)
   PSC_FUNCTION2(B2SServer, void, B2SSetLEDDisplay, int, string)
   PSC_FUNCTION2(B2SServer, void, B2SSetReel, int, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetScore, int, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetScorePlayer, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScorePlayer1, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScorePlayer2, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScorePlayer3, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScorePlayer4, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScorePlayer5, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScorePlayer6, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetScoreDigit, int, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetScoreRollover, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScoreRolloverPlayer1, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScoreRolloverPlayer2, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScoreRolloverPlayer3, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetScoreRolloverPlayer4, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetCredits, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetCredits, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetPlayerUp, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetPlayerUp, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetCanPlay, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetCanPlay, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetBallInPlay, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetBallInPlay, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetTilt, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetTilt, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetMatch, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetMatch, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetGameOver, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetGameOver, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SSetShootAgain, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetShootAgain, int, int)
   PSC_FUNCTION1(B2SServer, void, B2SStartAnimation, string)
   PSC_FUNCTION2(B2SServer, void, B2SStartAnimation, string, bool)
   PSC_FUNCTION1(B2SServer, void, B2SStartAnimationReverse, string)
   PSC_FUNCTION1(B2SServer, void, B2SStopAnimation, string)
   PSC_FUNCTION0(B2SServer, void, B2SStopAllAnimations)
   PSC_PROP_R_ARRAY1(B2SServer, bool, B2SIsAnimationRunning, string)
   PSC_FUNCTION1(B2SServer, void, StartAnimation, string)
   PSC_FUNCTION2(B2SServer, void, StartAnimation, string, bool)
   PSC_FUNCTION1(B2SServer, void, StopAnimation, string)
   PSC_FUNCTION0(B2SServer, void, B2SStartRotation)
   PSC_FUNCTION0(B2SServer, void, B2SStopRotation)
   PSC_FUNCTION0(B2SServer, void, B2SShowScoreDisplays)
   PSC_FUNCTION0(B2SServer, void, B2SHideScoreDisplays)
   PSC_FUNCTION1(B2SServer, void, B2SStartSound, string)
   PSC_FUNCTION1(B2SServer, void, B2SPlaySound, string)
   PSC_FUNCTION1(B2SServer, void, B2SStopSound, string)
   PSC_FUNCTION2(B2SServer, void, B2SMapSound, int, string)

   // PinMame API mirroring inside B2SServer
   if (pinmameClassDef)
   {
      pinmameMemberStartIndex = static_cast<int>(members.size());
      for (unsigned int i = 0; i < pinmameClassDef->nMembers; i++)
      {
         ScriptClassMemberDef member = pinmameClassDef->members[i];
         member.Call = [](void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
            {
               static_cast<B2SServer*>(me)->ForwardPinMAMECall(memberIndex - pinmameMemberStartIndex, pArgs, pRet);
            };
         members.push_back(member);
      }
   }
PSC_CLASS_END(B2SServer)


///////////////////////////////////////////////////////////////////////////////
// Renderer
//

VPXTexture CreateTexture(uint8_t* rawData, int size)
{
   if (vpxApi)
      return vpxApi->CreateTexture(rawData, size);
   return nullptr;
}

void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image)
{
   if (vpxApi)
      vpxApi->UpdateTexture(texture, width, height, format, image);
}

VPXTextureInfo* GetTextureInfo(VPXTexture texture)
{
   if (vpxApi)
      return vpxApi->GetTextureInfo(texture);
   else
      return nullptr;
}

void DeleteTexture(VPXTexture texture)
{
   if (vpxApi)
      vpxApi->DeleteTexture(texture);
}

// TODO we need to support a standalone mode when no PinMAME plugin is available

static void RegisterServerObject()
{
   if (serverRegistered)
      return;

   pinmameClassDef = scriptApi->GetClassDef("Controller");
   if (pinmameClassDef == nullptr)
      return;

   auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   RegisterB2SServerSCD(regLambda);
   B2SServer_SCD->CreateObject = []()
   {
      assert(server == nullptr);
      server = new B2SServer(msgApi, endpointId, vpxApi, pinmameClassDef);
      server->SetOnDestroyHandler(
         [](B2SServer* pServer)
         {
            assert(server == pServer);
            server = nullptr;
         });
      return static_cast<void*>(server);
   };
   scriptApi->SubmitTypeLibrary();
   scriptApi->SetCOMObjectOverride("B2S.Server", B2SServer_SCD);
   serverRegistered = true;
}

static void OnPluginLoaded(const unsigned int, void*, void* msgData)
{
   RegisterServerObject();
}

static void OnPluginUnloaded(const unsigned int, void*, void* msgData)
{
}

}

using namespace B2S;

MSGPI_EXPORT void MSGPIAPI B2SPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   apiThread = std::this_thread::get_id();
   LPISetup(endpointId, msgApi);

   getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);

   msgApi->SubscribeMsg(endpointId, onPluginLoaded = msgApi->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_LOADED), OnPluginLoaded, nullptr);
   msgApi->SubscribeMsg(endpointId, onPluginUnloaded = msgApi->GetMsgID(MSGPI_NAMESPACE, MSGPI_EVT_ON_PLUGIN_UNLOADED), OnPluginUnloaded, nullptr);

   // Contribute our API to the script engine
   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);

   RegisterServerObject();
}

MSGPI_EXPORT void MSGPIAPI B2SPluginUnload()
{
   assert(server == nullptr);
   serverRegistered = false;
   scriptApi->SetCOMObjectOverride("B2S.Server", nullptr);
   // TODO we should unregister the script API contribution
   msgApi->UnsubscribeMsg(onPluginLoaded, OnPluginLoaded);
   msgApi->UnsubscribeMsg(onPluginUnloaded, OnPluginUnloaded);
   msgApi->ReleaseMsgID(onPluginLoaded);
   msgApi->ReleaseMsgID(onPluginUnloaded);
   msgApi->ReleaseMsgID(getVpxApiId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
