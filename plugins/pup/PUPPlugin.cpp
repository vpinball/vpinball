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
static unsigned int onPinMAMEGameStartId, onGameEndId;

// The pup manager holds the overall state. It may be automatically created due to a PinMAME start event, or explicitely created
// through script interface. The script interface gives access to this context even when it has been created due to PinMAME.
static std::unique_ptr<PUPManager> pupManager;

LPI_IMPLEMENT // Implement shared log support

MSGPI_STRING_VAL_SETTING(pupPathProp, "PUPFolder", "PinUp Player Folder", "", true, "", 1024);


///////////////////////////////////////////////////////////////////////////////
// Script interface
//

PSC_CLASS_START(PUPPinDisplay)
   PSC_FUNCTION2(PUPPinDisplay, void, Init, int, string)
   PSC_FUNCTION4(PUPPinDisplay, void, playlistadd, int, string, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, playlistplay, int, string)
   PSC_FUNCTION5(PUPPinDisplay, void, playlistplayex, int, string, string, int, int)
   PSC_FUNCTION3(PUPPinDisplay, void, play, int, string, string)
   PSC_FUNCTION2(PUPPinDisplay, void, setWidth, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, setHeight, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, setPosX, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, setPosY, int, int)
   PSC_FUNCTION3(PUPPinDisplay, void, setAspect, int, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, setVolume, int, int)
   PSC_FUNCTION1(PUPPinDisplay, void, playpause, int)
   PSC_FUNCTION1(PUPPinDisplay, void, playresume, int)
   PSC_FUNCTION1(PUPPinDisplay, void, playstop, int)
   PSC_FUNCTION0(PUPPinDisplay, void, CloseApp)
   PSC_PROP_RW_ARRAY1(PUPPinDisplay, bool, isPlaying, int)
   PSC_FUNCTION2(PUPPinDisplay, void, SetLength, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, SetLoop, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, SetBackGround, int, int)
   PSC_FUNCTION2(PUPPinDisplay, void, BlockPlay, int, int)
   PSC_FUNCTION1(PUPPinDisplay, void, SetScreen, int)
   PSC_FUNCTION6(PUPPinDisplay, void, SetScreenEx, int, int, int, int, int, int)
   PSC_PROP_RW(PUPPinDisplay, int, SN)
   PSC_FUNCTION2(PUPPinDisplay, void, B2SData, string, int)
   PSC_PROP_RW(PUPPinDisplay, string, B2SFilter)
   PSC_FUNCTION1(PUPPinDisplay, void, Show, int)
   PSC_FUNCTION1(PUPPinDisplay, void, Hide, int)
   PSC_FUNCTION2(PUPPinDisplay, void, B2SInit, string, string)
   PSC_FUNCTION1(PUPPinDisplay, void, SendMSG, string)
   PSC_FUNCTION1(PUPPinDisplay, void, Show, int)
   PSC_FUNCTION12(PUPPinDisplay, void, LabelNew, int, string, string, int, int, int, int, int, int, int, int, bool)
   PSC_FUNCTION5(PUPPinDisplay, void, LabelSet, int, string, string, bool, string)
   PSC_FUNCTION0(PUPPinDisplay, void, LabelSetEx)
   PSC_FUNCTION4(PUPPinDisplay, void, LabelShowPage, int, int, int, string)
   PSC_FUNCTION1(PUPPinDisplay, void, LabelInit, int)
   PSC_PROP_RW(PUPPinDisplay, string, GetGame)
   PSC_PROP_RW(PUPPinDisplay, string, GetRoot)
   PSC_FUNCTION6(PUPPinDisplay, void, SoundAdd, string, string, int, double, double, string)
   PSC_FUNCTION1(PUPPinDisplay, void, SoundPlay, string)
   PSC_FUNCTION6(PUPPinDisplay, void, PuPSound, string, int, int, int, int, string)
   PSC_FUNCTION1(PUPPinDisplay, void, InitPuPMenu, int)
   PSC_PROP_R(PUPPinDisplay, string, B2SDisplays)
   PSC_FUNCTION2(PUPPinDisplay, void, setVolumeCurrent, int, int)
   //PSC_PROP_R_ARRAY4(PUPPinDisplay, int, GameUpdate, string, int, int, string)
   // STDMETHOD(GrabDC)(LONG pWidth, LONG pHeight, BSTR wintitle, VARIANT *pixels);
   PSC_FUNCTION0(PUPPinDisplay, string, GetVersion)
   // STDMETHOD(GrabDC2)(LONG pWidth, LONG pHeight, BSTR wintitle, SAFEARRAY **pixels);
   PSC_FUNCTION8(PUPPinDisplay, void, playevent, int, string, string, int, int, int, int, string)
   PSC_FUNCTION5(PUPPinDisplay, void, SetPosVideo, int, int, int, int, string)
   PSC_FUNCTION0(PUPPinDisplay, void, PuPClose)
PSC_CLASS_END(PUPPinDisplay)



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

void OnPinMAMEGameStart(const unsigned int eventId, void* userData, void* eventData)
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

   msgApi->SubscribeMsg(endpointId, onPinMAMEGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), OnPinMAMEGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), OnGameEnd, nullptr);

   onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);

   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);
   auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   RegisterPUPPinDisplaySCD(regLambda);
   PUPPinDisplay_SCD->CreateObject = []()
   {
      PUPPinDisplay* pinDisplay = new PUPPinDisplay(*pupManager.get());
      return static_cast<void*>(pinDisplay);
   };
   scriptApi->SubmitTypeLibrary();
   scriptApi->SetCOMObjectOverride("PinUpPlayer.PinDisplay", PUPPinDisplay_SCD);

   msgApi->RegisterSetting(endpointId, &pupPathProp);
   std::filesystem::path pupFolder = pupPathProp_Get();
   std::filesystem::path rootPath = find_case_insensitive_directory_path(pupFolder / "pupvideos");
   if (rootPath.empty())
   {
      LOGW("PUP folder was not found (settings is '%s')", pupFolder.string().c_str());
   }
   pupManager = std::make_unique<PUPManager>(msgApi, endpointId, rootPath);
}

MSGPI_EXPORT void MSGPIAPI PUPPluginUnload()
{
   pupManager = nullptr;
   
   msgApi->ReleaseMsgID(onAudioUpdateId);

   msgApi->UnsubscribeMsg(onPinMAMEGameStartId, OnPinMAMEGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, OnGameEnd);
   msgApi->ReleaseMsgID(onPinMAMEGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);

   // TODO we should unregister the script API contribution
   scriptApi->SetCOMObjectOverride("PinUpPlayer.PinDisplay", nullptr);
   
   scriptApi = nullptr;
   vpxApi = nullptr;
   msgApi = nullptr;
}
