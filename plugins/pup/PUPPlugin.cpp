// license:GPLv3+

#include "common.h"
#include <future>

#include "plugins/MsgPlugin.h"
#include "plugins/ControllerPlugin.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPPinDisplay.h"

#include "LibAv.h"

///////////////////////////////////////////////////////////////////////////////
// PinUp Player plugin
//
// This plugin is a rewrite of NailBuster's excellent PinUp Player as a plugin,
// based on Jason Millard port for VPX standalone.
//
// References:
// - https://www.nailbuster.com/wikipinup/doku.php?id=start
// - https://gist.github.com/jsm174/e5aa4ebe70052b5cf2ef49ab40c35dfb
// - https://github.com/francisdb/pup-research/tree/main
//
// This port comes with the following changes & enhancements:
// - it is open source and portable
// - it does not come with additional dependencies (no need for B2S, nor DMDExt)
// - it renders through the provided 2D image drawing hooks, allowing to
//   render PinUp videos directly in 3D, especially for VR play.
//
// The plugin renders the PinUp screens inside 'standard' plugin ancillary windows
// with the following mapping:
//  0. Topper         => Topper
//  1. DMD (4:1 slim) => ScoreView (selected through user settings)
//  2. BackGlass      => Backglass
//  3. Playfield      => Not rendered
//  4. Music          => Not rendered (supposed to be used without any visuals, but only audio)
//  5. Apron/FullDMD  => ScoreView (selected through user settings)
//  6. Game Select    => Not rendered
//  7. Loading        => Not rendered
//  8. Other2         => Not rendered
//  9. GameInfo       => Not rendered
// 10. GameHelp       => Not rendered
//

namespace PUP {

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId;

// The pup manager holds the overall state. It may be automatically created due to a PinMAME start event, or explicitely created
// through script interface. The script interface gives access to this context even when it has been created due to PinMAME.
static std::unique_ptr<PUPManager> pupManager;

LPI_IMPLEMENT_CPP // Implement shared log support

MSGPI_STRING_VAL_SETTING(pupPathProp, "PUPFolder", "PinUp Player Folder", "", true, "", 1024);


///////////////////////////////////////////////////////////////////////////////
// Script interface
//

PSC_CLASS_START(PUP_PinDisplay, PUPPinDisplay)
   PSC_FUNCTION2(void, Init, int, string)
   PSC_FUNCTION4(void, playlistadd, int, string, int, int)
   PSC_FUNCTION2(void, playlistplay, int, string)
   PSC_FUNCTION5(void, playlistplayex, int, string, string, int, int)
   PSC_FUNCTION3(void, play, int, string, string)
   PSC_FUNCTION2(void, setWidth, int, int)
   PSC_FUNCTION2(void, setHeight, int, int)
   PSC_FUNCTION2(void, setPosX, int, int)
   PSC_FUNCTION2(void, setPosY, int, int)
   PSC_FUNCTION3(void, setAspect, int, int, int)
   PSC_FUNCTION2(void, setVolume, int, int)
   PSC_FUNCTION1(void, playpause, int)
   PSC_FUNCTION1(void, playresume, int)
   PSC_FUNCTION1(void, playstop, int)
   PSC_FUNCTION0(void, CloseApp)
   PSC_PROP_RW_ARRAY1(bool, isPlaying, int)
   PSC_FUNCTION2(void, SetLength, int, int)
   PSC_FUNCTION2(void, SetLoop, int, int)
   PSC_FUNCTION2(void, SetBackGround, int, int)
   PSC_FUNCTION2(void, BlockPlay, int, int)
   PSC_FUNCTION1(void, SetScreen, int)
   PSC_FUNCTION6(void, SetScreenEx, int, int, int, int, int, int)
   PSC_PROP_RW(int, SN)
   PSC_FUNCTION2(void, B2SData, string, int)
   PSC_PROP_RW(string, B2SFilter)
   PSC_FUNCTION1(void, Show, int)
   PSC_FUNCTION1(void, Hide, int)
   PSC_FUNCTION2(void, B2SInit, string, string)
   PSC_FUNCTION1(void, SendMSG, string)
   PSC_FUNCTION1(void, Show, int)
   PSC_FUNCTION12(void, LabelNew, int, string, string, int, int, int, int, int, int, int, int, bool)
   PSC_FUNCTION5(void, LabelSet, int, string, string, bool, string)
   PSC_FUNCTION0(void, LabelSetEx)
   PSC_FUNCTION4(void, LabelShowPage, int, int, int, string)
   PSC_FUNCTION1(void, LabelInit, int)
   PSC_PROP_RW(string, GetGame)
   PSC_PROP_RW(string, GetRoot)
   PSC_FUNCTION6(void, SoundAdd, string, string, int, double, double, string)
   PSC_FUNCTION1(void, SoundPlay, string)
   PSC_FUNCTION6(void, PuPSound, string, int, int, int, int, string)
   PSC_FUNCTION1(void, InitPuPMenu, int)
   PSC_PROP_R(string, B2SDisplays)
   PSC_FUNCTION2(void, setVolumeCurrent, int, int)
   //PSC_PROP_R_ARRAY4(int, GameUpdate, string, int, int, string)
   // STDMETHOD(GrabDC)(LONG pWidth, LONG pHeight, BSTR wintitle, VARIANT *pixels);
   PSC_FUNCTION0(string, GetVersion)
   // STDMETHOD(GrabDC2)(LONG pWidth, LONG pHeight, BSTR wintitle, SAFEARRAY **pixels);
   PSC_FUNCTION8(void, playevent, int, string, string, int, int, int, int, string)
   PSC_FUNCTION5(void, SetPosVideo, int, int, int, int, string)
   PSC_FUNCTION0(void, PuPClose)
PSC_CLASS_END()



///////////////////////////////////////////////////////////////////////////////
// Renderer
//

void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, const void* image)
{
   if (vpxApi)
      vpxApi->UpdateTexture(texture, width, height, format, image);
}

VPXTexture CreateTexture(SDL_Surface* surf)
{
   VPXTexture texture = nullptr;
   SDL_LockSurface(surf);
   UpdateTexture(&texture, surf->w, surf->h, VPXTextureFormat::VPXTEXFMT_sRGBA8, surf->pixels);
   SDL_UnlockSurface(surf);
   return texture;
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


///////////////////////////////////////////////////////////////////////////////
// Audio streaming
//

static unsigned int onAudioUpdateId;
static vector<uint32_t> freeAudioStreamId;
uint32_t nextAudioStreamId = 1;

CtlResId UpdateAudioStream(AudioUpdateMsg* msg)
{
   if (msg->volume == 0.0f)
   {
      StopAudioStream(msg->id);
      return {};
   }
   CtlResId id = msg->id;
   if (id.id == 0)
   {
      id.endpointId = endpointId;
      if (freeAudioStreamId.empty())
      {
         id.resId = nextAudioStreamId;
         nextAudioStreamId++;
      }
      else
      {
         id.resId = freeAudioStreamId.back();
         freeAudioStreamId.pop_back();
      }
      msg->id = id;
   }
   msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
      AudioUpdateMsg* msg = static_cast<AudioUpdateMsg*>(userData);
      msgApi->BroadcastMsg(endpointId, onAudioUpdateId, msg);
      if (LibAV::LibAV::GetInstance().isLoaded)
         LibAV::LibAV::GetInstance()._av_free(msg->buffer);
      delete msg;
   }, msg);
   return id;
}

void StopAudioStream(const CtlResId& id)
{
   if (id.id != 0)
   {
      // Recycle stream id
      freeAudioStreamId.push_back(id.resId);
      // Send an end of stream message
      AudioUpdateMsg* pendingAudioUpdate = new AudioUpdateMsg();
      memset(pendingAudioUpdate, 0, sizeof(AudioUpdateMsg));
      pendingAudioUpdate->id.id = id.id;
      msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
         AudioUpdateMsg* msg = static_cast<AudioUpdateMsg*>(userData);
         msgApi->BroadcastMsg(endpointId, onAudioUpdateId, msg);
         delete msg;
      }, pendingAudioUpdate);
   }
}


///////////////////////////////////////////////////////////////////////////////
// Game lifecycle
//

void onGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(eventData);
   assert(msg != nullptr && msg->gameId != nullptr);
   pupManager->LoadConfig(msg->gameId);
}

void OnGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   pupManager->Unload();
}

}

///////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle
//

using namespace PUP;

MSGPI_EXPORT void MSGPIAPI PUPPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   TTF_Init();

   // Request and setup shared login API
   LPISetup(endpointId, msgApi);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), OnGameEnd, nullptr);

   onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);

   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);
   RegisterPUP_PinDisplay([](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); });
   PUP_PinDisplay_SCD->CreateObject = []()
   {
      PUPPinDisplay* pinDisplay = new PUPPinDisplay(*pupManager.get());
      return static_cast<void*>(pinDisplay);
   };
   scriptApi->SubmitTypeLibrary(endpointId);
   scriptApi->SetCOMObjectOverride("PinUpPlayer.PinDisplay", PUP_PinDisplay_SCD);

   msgApi->RegisterSetting(endpointId, &pupPathProp);
   std::filesystem::path pupFolder = pupPathProp_Get();
   std::filesystem::path rootPath = find_case_insensitive_directory_path(pupFolder / "pupvideos");
   if (rootPath.empty())
   {
      LOGW("PUP folder was not found (settings is '" + pupFolder.string() + "')");
   }
   pupManager = std::make_unique<PUPManager>(msgApi, endpointId, rootPath);
}

MSGPI_EXPORT void MSGPIAPI PUPPluginUnload()
{
   pupManager = nullptr;
   
   scriptApi->SetCOMObjectOverride("PinUpPlayer.PinDisplay", nullptr);
   UnregisterPUP_PinDisplay([](ScriptClassDef* scd) { scriptApi->UnregisterScriptClass(scd); });

   msgApi->ReleaseMsgID(onAudioUpdateId);

   msgApi->UnsubscribeMsg(onGameStartId, onGameStart, nullptr);
   msgApi->UnsubscribeMsg(onGameEndId, OnGameEnd, nullptr);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);

   scriptApi = nullptr;
   vpxApi = nullptr;
   msgApi = nullptr;
}
