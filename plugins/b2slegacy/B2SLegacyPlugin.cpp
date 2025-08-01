// license:GPLv3+

#include "common.h"
#include "ScriptablePlugin.h"
#include "LoggingPlugin.h"
#include "VPXPlugin.h"
#include "ControllerPlugin.h"

#include "Server.h"
#include "forms/FormDMD.h"
#include "classes/B2SScreen.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin interface

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId = 0;

static unsigned int onGetAuxRendererId = 0;
static unsigned int onAuxRendererChgId = 0;

static ScriptClassDef* pinmameClassDef = nullptr;
static void* pinmameInstance = nullptr;
static int pinmameMemberStartIndex = 0;
static B2SLegacy::Server* serverInstance = nullptr;

static unsigned int getDevSrcMsgId = 0;
static unsigned int onDevChangedMsgId = 0;

int OnRender(VPXRenderContext2D* const renderCtx, void* context);
void OnGetRenderer(const unsigned int msgId, void* context, void* msgData);
void OnDevSrcChanged(const unsigned int msgId, void* userData, void* msgData);

namespace B2SLegacy {

void MSGPIAPI ForwardPinMAMECall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   assert(pinmameClassDef);
   if (!pinmameClassDef)
      return;

   const int index = memberIndex - pinmameMemberStartIndex;
   const char* methodName = pinmameClassDef->members[index].name.name;

   if (!pinmameInstance) {
      LOGI("B2SLegacy: Creating PinMAME Controller instance via forwarding");
      pinmameInstance = pinmameClassDef->CreateObject();
   }
   if (pinmameInstance) {
      LOGD("B2SLegacy: Forwarding PinMAME call '%s' - memberIndex: %d, index: %d", methodName, memberIndex, index);
      pinmameClassDef->members[index].Call(pinmameInstance, index, pArgs, pRet);
   }

   B2SLegacy::Server* server = static_cast<B2SLegacy::Server*>(me);
   if (!server)
      return;

   if (strcmp(methodName, "GameName") == 0) {
      if (pArgs) {
         string gameName;
         if (pArgs[0].vString.string)
            gameName = string(pArgs[0].vString.string);
         LOGI("B2SLegacy: Setting GameName to '%s' in B2S settings", gameName.c_str());
         server->GetB2SSettings()->SetGameName(gameName);
         server->GetB2SSettings()->SetB2SName("");
      }
   }
   else if (strcmp(methodName, "Run") == 0)
      server->Run(0);
   else if (strcmp(methodName, "Stop") == 0)
      server->Stop();
   else if (strcmp(methodName, "ChangedLamps") == 0) {
      if (pRet && pRet->vArray)
         server->GetChangedLamps(pRet->vArray, true);
   }
   else if (strcmp(methodName, "ChangedSolenoids") == 0) {
      if (pRet && pRet->vArray)
         server->GetChangedSolenoids(pRet->vArray, true);
   }
   else if (strcmp(methodName, "ChangedGIStrings") == 0) {
      if (pRet && pRet->vArray)
         server->GetChangedGIStrings(pRet->vArray, true);
   }
   else if (strcmp(methodName, "ChangedLEDs") == 0) {
      if (pRet && pRet->vArray)
         server->GetChangedLEDs(pRet->vArray, true);
   }
   else if (strcmp(methodName, "GetMech") == 0) {
      if (pArgs && pRet)
         server->CheckGetMech(pArgs[0].vInt, pRet->vInt);
   }
}

PSC_ERROR_IMPLEMENT(scriptApi);

LPI_IMPLEMENT

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Renderer implementations

int OnRender(VPXRenderContext2D* const renderCtx, void* context)
{
   if (serverInstance)
      return serverInstance->OnRender(renderCtx, context);

   return 0;
}

void OnGetRenderer(const unsigned int msgId, void* context, void* msgData)
{
   static AnciliaryRendererDef backglassEntry = { "B2SLegacy", "B2S Legacy Backglass", "Renderer for B2S legacy backglass files", nullptr, OnRender };
   static AnciliaryRendererDef dmdEntry = { "B2SLegacyDMD", "B2S Legacy DMD", "Renderer for B2S legacy DMD files", nullptr, OnRender };

   GetAnciliaryRendererMsg* msg = static_cast<GetAnciliaryRendererMsg*>(msgData);

   if (msg->window == VPXAnciliaryWindow::VPXWINDOW_Backglass) {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = backglassEntry;
      msg->count++;
   }
   else if (msg->window == VPXAnciliaryWindow::VPXWINDOW_ScoreView) {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = dmdEntry;
      msg->count++;
   }
}

void OnDevSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   if (serverInstance)
      serverInstance->OnDevSrcChanged(msgId, userData, msgData);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Script interface

using namespace B2SLegacy;

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

      if (pinmameClassDef)
      {
         pinmameMemberStartIndex = static_cast<int>(members.size());
         for (unsigned int i = 0; i < pinmameClassDef->nMembers; i++)
         {
            ScriptClassMemberDef member = pinmameClassDef->members[i];
            member.Call = B2SLegacy::ForwardPinMAMECall;
            members.push_back(member);
         }
      }
PSC_CLASS_END(Server)

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle

MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   endpointId = sessionId;
   msgApi = api;

   LPISetup(endpointId, msgApi);
   const unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   msgApi->SubscribeMsg(endpointId, onGetAuxRendererId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), OnGetRenderer, nullptr);
   msgApi->BroadcastMsg(endpointId, onAuxRendererChgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);

   getDevSrcMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   onDevChangedMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);
   msgApi->SubscribeMsg(endpointId, onDevChangedMsgId, OnDevSrcChanged, nullptr);

   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);

   if (!scriptApi) {
      LOGE("Failed to get script API");
      return;
   }

   pinmameClassDef = scriptApi->GetClassDef("Controller");
   if (pinmameClassDef) {
      auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
      RegisterServerSCD(regLambda);
      Server_SCD->CreateObject = []()
      {
         B2SLegacy::Server* server = new B2SLegacy::Server(msgApi, endpointId, vpxApi);
         serverInstance = server;
         return static_cast<void*>(server);
      };
      scriptApi->SubmitTypeLibrary();
      scriptApi->SetCOMObjectOverride("B2S.Server", Server_SCD);
      LOGI("B2SLegacy Plugin registered with PinMAME Controller proxying");
   }
   else {
      LOGI("B2SLegacy Plugin loaded without PinMAME Controller (Controller class not available)");
   }

   LOGI("B2SLegacy Plugin loaded successfully");
}

MSGPI_EXPORT void MSGPIAPI B2SLegacyPluginUnload()
{
   if (scriptApi)
      scriptApi->SetCOMObjectOverride("B2S.Server", nullptr);

   if (msgApi) {
      msgApi->UnsubscribeMsg(onGetAuxRendererId, OnGetRenderer);
      msgApi->BroadcastMsg(endpointId, onAuxRendererChgId, nullptr);
      msgApi->ReleaseMsgID(onGetAuxRendererId);
      msgApi->ReleaseMsgID(onAuxRendererChgId);

      msgApi->UnsubscribeMsg(onDevChangedMsgId, OnDevSrcChanged);
      msgApi->ReleaseMsgID(onDevChangedMsgId);
      msgApi->ReleaseMsgID(getDevSrcMsgId);
   }

   serverInstance = nullptr;
   vpxApi = nullptr;
   scriptApi = nullptr;
   msgApi = nullptr;

   LOGI("B2SLegacy Plugin unloaded");
}