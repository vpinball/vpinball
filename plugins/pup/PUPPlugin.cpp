// license:GPLv3+

#include "common.h"
#include <future>

#include "MsgPlugin.h"
#include "CorePlugin.h"
#include "PinMamePlugin.h"

#include "PUPManager.h"
#include "PUPScreen.h"
#include "PUPPinDisplay.h"

LPI_IMPLEMENT // Implement shared login support

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
// - it is open sourced and portable
// - it does not come with additional dependencies (no need for B2S, nor DMDExt)
// - it renders through the provided 2D image drawing hooks, allowing to
//   render PinUp videos directly in 3D, especially for VR play.
//
// The plugin renders the PinUp screens inside 'standard' plugin anciliary windows
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

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static uint32_t endpointId;
static unsigned int onPinMAMEGameStartId, onGameEndId;
static std::thread::id apiThread;

// The pup manager holds the overall state. It may be automatically created due to a PinMAME start event, or explicitely created
// through script interface. The script interface gives access to this context even when it has been created due to PinMAME.
static std::unique_ptr<PUPManager> pupManager;


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

void UpdateTexture(VPXTexture* texture, int width, int height, VPXTextureFormat format, uint8_t* image)
{
   if (vpxApi)
      vpxApi->UpdateTexture(texture, width, height, format, image);
}

VPXTexture CreateTexture(SDL_Surface* surf)
{
   VPXTexture texture = nullptr;
   SDL_LockSurface(surf);
   UpdateTexture(&texture, surf->w, surf->h, VPXTextureFormat::VPXTEXFMT_sRGBA, static_cast<uint8_t*>(surf->pixels));
   SDL_UnlockSurface(surf);
   return texture;
}

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


///////////////////////////////////////////////////////////////////////////////
// Game lifecycle
//

void OnPinMAMEGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   const PMPI_MSG_ON_GAME_START* msg = static_cast<const PMPI_MSG_ON_GAME_START*>(eventData);
   assert(msg != nullptr && msg->gameId != nullptr);
   if (pupManager->IsInit())
   {
      LOGI("PinMAME started while Pup has already been directy initialized. Discarding initialization from PinMAME rom '%s'", msg->gameId);
   }
   else
   {
      pupManager->LoadConfig(msg->gameId);
   }
   pupManager->Start();
}

void OnGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   pupManager->Stop();
   pupManager->Unload();
}


///////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle
//

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   apiThread = std::this_thread::get_id();

   TTF_Init();

   // Request and setup shared login API
   LPISetup(endpointId, msgApi);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   msgApi->SubscribeMsg(endpointId, onPinMAMEGameStartId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_START), OnPinMAMEGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), OnGameEnd, nullptr);

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

   char pupFolder[512];
   msgApi->GetSetting("PUP", "PUPFolder", pupFolder, sizeof(pupFolder));
   string rootPath = normalize_path_separators(pupFolder);
   if (!rootPath.ends_with(PATH_SEPARATOR_CHAR))
      rootPath += PATH_SEPARATOR_CHAR;
   rootPath = find_case_insensitive_directory_path(rootPath + "pupvideos"s);
   if (rootPath.empty())
   {
      LOGE("PUP folder was not found (settings is '%s')", pupFolder);
   }
   pupManager = std::make_unique<PUPManager>(msgApi, endpointId, rootPath);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   pupManager = nullptr;
   
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
