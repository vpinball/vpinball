// license:GPLv3+

#include "common.h"
#include <future>

#include "MsgPlugin.h"
#include "ControllerPlugin.h"

#include "B2SDataModel.h"
#include "B2SRenderer.h"
#include "B2SServer.h"

namespace B2S {
   
LPI_IMPLEMENT // Implement shared login support

///////////////////////////////////////////////////////////////////////////////
// B2S plugin
// - implement rendering of directb2s backglass and scoreview
// - implement COM B2S Server which can be used as a game controller,
//   in turn, eventually using PinMAME plugin if requested

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId, onGetAuxRendererId, onAuxRendererChgId;
static std::thread::id apiThread;


///////////////////////////////////////////////////////////////////////////////
// Script interface
//

static ScriptClassDef* pinmameClassDef = nullptr;
static void* pinmameInstance = nullptr;
static int pinmameMemberStartIndex = 0;

void MSGPIAPI ForwardPinMAMECall(void* me, int memberIndex, ScriptVariant* pArgs, ScriptVariant* pRet)
{
   assert(pinmameClassDef);
   if (pinmameClassDef == nullptr)
      return;
   if (pinmameInstance == nullptr)
      pinmameInstance = pinmameClassDef->CreateObject();
   const int index = memberIndex - pinmameMemberStartIndex;
   pinmameClassDef->members[index].Call(pinmameInstance, index, pArgs, pRet);
}

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
         member.Call = ForwardPinMAMECall;
         members.push_back(member);
      }
   }
PSC_CLASS_END(B2SServer)


///////////////////////////////////////////////////////////////////////////////
// Renderer
//

std::future<std::shared_ptr<B2STable>> loadedB2S;
B2SRenderer* renderer = nullptr;

VPXTexture CreateTexture(uint8_t* rawData, int size)
{
   if (vpxApi)
      return vpxApi->CreateTexture(rawData, size);
   return nullptr;
}

void GetTextureInfo(VPXTexture texture, int* width, int* height)
{
   if (vpxApi)
      vpxApi->GetTextureInfo(texture, width, height);
}

void DeleteTexture(VPXTexture texture)
{
   if (vpxApi)
      vpxApi->DeleteTexture(texture);
}

void OnGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);

   if (loadedB2S.valid())
      loadedB2S.get(); // Flush any loading in progress to trigger texture destruction and void memory leaks

   string b2sFilename = find_case_insensitive_file_path(TitleAndPathFromFilename(tableInfo.path) + ".directb2s");
   if (!b2sFilename.empty())
   {
      auto loadFile = [](const string& path)
      {
         std::shared_ptr<B2STable> b2s;
         try
         {
            tinyxml2::XMLDocument b2sTree;
            b2sTree.LoadFile(path.c_str());
            if (b2sTree.FirstChildElement("DirectB2SData"))
               b2s = std::make_shared<B2STable>(*b2sTree.FirstChildElement("DirectB2SData"));
         }
         catch (...)
         {
            LOGE("Failed to load B2S file: %s", path.c_str());
         }
         return b2s;
      };
      // B2S file format is heavily unoptimized so perform loading asynchronously (all assets are directly included in the XML file using Base64 encoding)
      loadedB2S = std::async(std::launch::async, loadFile, b2sFilename);
   }
}

void OnGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   if (loadedB2S.valid())
      loadedB2S.get();
   delete renderer;
   renderer = nullptr;
   if (pinmameInstance)
      PSC_RELEASE(pinmameClassDef, pinmameInstance);
   pinmameInstance = nullptr;
}

int OnRender(VPXRenderContext2D* ctx, void*)
{
   if ((ctx->window != VPXAnciliaryWindow::VPXWINDOW_Backglass) && (ctx->window != VPXAnciliaryWindow::VPXWINDOW_ScoreView))
      return false;
   if (renderer)
   {
      return renderer->Render(ctx);
   }
   else if (loadedB2S.valid())
   {
      if (loadedB2S.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      {
         renderer = new B2SRenderer(msgApi, endpointId, loadedB2S.get());
         renderer->Render(ctx);
      }
      return true; // Until loaded, we suppose that the file will succeeded load with the expected backglass/scoreview
   }
   else
   {
      return false;
   }
}

void OnGetRenderer(const unsigned int msgId, void* context, void* msgData)
{
   static AnciliaryRendererDef entry = { "B2S", "B2S Backglass & FullDMD", "Renderer for directb2s backglass files", nullptr, OnRender };
   GetAnciliaryRendererMsg* msg = static_cast<GetAnciliaryRendererMsg*>(msgData);
   if (msg->window == VPXAnciliaryWindow::VPXWINDOW_Backglass)
   {
      if (msg->count < msg->maxEntryCount)
         msg->entries[msg->count] = entry;
      msg->count++;
   }
}

// Only register if PinMAME is available
void RegisterServerObject(void* userData)
{
   pinmameClassDef = scriptApi->GetClassDef("Controller");
   if (pinmameClassDef != nullptr)
   {
      auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
      RegisterB2SServerSCD(regLambda);
      B2SServer_SCD->CreateObject = []()
      {
         B2SServer* server = new B2SServer();
         return static_cast<void*>(server);
      };
      scriptApi->SubmitTypeLibrary();
      scriptApi->SetCOMObjectOverride("B2S.Server", B2SServer_SCD);
   }
   else
   {
      msgApi->RunOnMainThread(0.1, RegisterServerObject, nullptr);
   }
}

}

using namespace B2S;

MSGPI_EXPORT void MSGPIAPI B2SPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   apiThread = std::this_thread::get_id();
   LPISetup(endpointId, msgApi);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), OnGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), OnGameEnd, nullptr);

   msgApi->SubscribeMsg(endpointId, onGetAuxRendererId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_AUX_RENDERER), OnGetRenderer, nullptr);
   msgApi->BroadcastMsg(endpointId, onAuxRendererChgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_AUX_RENDERER_CHG), nullptr);

   // Contribute our API to the script engine
   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);
   RegisterServerObject(nullptr);
}

MSGPI_EXPORT void MSGPIAPI B2SPluginUnload()
{
   scriptApi->SetCOMObjectOverride("B2S.Server", nullptr);
   // TODO we should unregister the script API contribution
   msgApi->UnsubscribeMsg(onGetAuxRendererId, OnGetRenderer);
   msgApi->UnsubscribeMsg(onGameStartId, OnGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, OnGameEnd);
   msgApi->BroadcastMsg(endpointId, onAuxRendererChgId, nullptr);
   msgApi->ReleaseMsgID(onGetAuxRendererId);
   msgApi->ReleaseMsgID(onAuxRendererChgId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
