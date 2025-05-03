// license:GPLv3+

#include "common.h"
#include <future>

#include "MsgPlugin.h"
#include "CorePlugin.h"
#include "PinMamePlugin.h"

#include "B2SDataModel.h"
#include "B2SRenderer.h"
#include "B2SServer.h"

#include "libpinmame.h"

LPI_IMPLEMENT // Implement shared login support

///////////////////////////////////////////////////////////////////////////////
// B2S plugin
// - implement rendering of directb2s backglass and scoreview
// - implement COM B2S Server which can be used as a game controller,
//   in turn, eventually using PinMAME plugin if requested


///////////////////////////////////////////////////////////////////////////////
// Script interface
//

PSC_ARRAY1(ByteArray, uint8, 0)
#define PSC_VAR_SET_ByteArray(variant, value) PSC_VAR_SET_array1(ByteArray, variant, value)
#define PSC_VAR_ByteArray(variant) PSC_VAR_array1(uint8_t, variant)

PSC_ARRAY1(IntArray, int32, 0)
#define PSC_VAR_SET_IntArray(variant, value) PSC_VAR_SET_array1(IntArray, variant, value)

// Map an array of struct to a 2 dimensions array of int32_t
PSC_ARRAY2(StructArray, int32, 0, 0)
#define PSC_VAR_SET_StructArray2(structType, fieldName1, fieldName2, variant, value) { \
      const unsigned int nDimensions = 2; \
      const std::vector<structType>& vec = (value); \
      const size_t size0 = vec.size(); \
      ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + nDimensions * sizeof(int) + size0 * 2 * sizeof(int32_t))); \
      array->Release = [](ScriptArray* me) { free(me); }; \
      array->lengths[0] = static_cast<unsigned int>(vec.size()); \
      array->lengths[1] = 2; \
      int32_t* pData = reinterpret_cast<int32_t*>(&array->lengths[2]); \
      for (size_t i = 0; i < size0; i++, pData += 2) { \
         pData[0] = vec[i].fieldName1; \
         pData[1] = vec[i].fieldName2; \
      } \
      (variant).vArray = array; \
   }
#define PSC_VAR_SET_StructArray3(structType, fieldName1, fieldName2, fieldName3, variant, value) { \
      const unsigned int nDimensions = 3; \
      const std::vector<structType>& vec = (value); \
      const size_t size0 = vec.size(); \
      ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + nDimensions * sizeof(int) + size0 * 3 * sizeof(int32_t))); \
      array->Release = [](ScriptArray* me) { free(me); }; \
      array->lengths[0] = static_cast<unsigned int>(vec.size()); \
      array->lengths[1] = 3; \
      int32_t* pData = reinterpret_cast<int32_t*>(&array->lengths[2]); \
      for (size_t i = 0; i < size0; i++, pData += 3) { \
         pData[0] = vec[i].fieldName1; \
         pData[1] = vec[i].fieldName2; \
         pData[2] = vec[i].fieldName3; \
      } \
      (variant).vArray = array; \
   }

#define PSC_PROP_R_StructArray2(className, type, fieldName1, fieldName2, name) \
   members.push_back( { { #name }, { "StructArray" }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_StructArray2(type, fieldName1, fieldName2, *pRet, static_cast<className*>(me)->Get##name()); } });

#define PSC_PROP_R_StructArray3(className, type, fieldName1, fieldName2, fieldName3, name) \
   members.push_back( { { #name }, { "StructArray" }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<className*>(me)->Get##name()); } });
#define PSC_PROP_R_StructArray3_2(className, type, fieldName1, fieldName2, fieldName3, name, arg1, arg2) \
   members.push_back( { { #name }, { "StructArray" }, 2, { { #arg1 }, { #arg2 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<className*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]) )); } } );
#define PSC_PROP_R_StructArray3_3(className, type, fieldName1, fieldName2, fieldName3, name, arg1, arg2, arg3) \
   members.push_back( { { #name }, { "StructArray"}, 3, { { #arg1 }, { #arg2 }, { #arg3 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<className*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]) )); } } );
#define PSC_PROP_R_StructArray3_4(className, type, fieldName1, fieldName2, fieldName3, name, arg1, arg2, arg3, arg4) \
   members.push_back( { { #name }, { "StructArray" }, 4, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<className*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]) )); } } );

PSC_CLASS_START(B2SServer)
   PSC_FUNCTION0(B2SServer, void, Dispose)
   PSC_PROP_R(B2SServer, string, B2SServerVersion)
   PSC_PROP_R(B2SServer, double, B2SBuildVersion)
   PSC_PROP_R(B2SServer, string, B2SServerDirectory)

   // PinMame encapsulation (more or less a mirror of VPinMame interface but not strictly, also impacting B2S controller state)
   PSC_PROP_RW(B2SServer, string, GameName)
   PSC_PROP_R(B2SServer, string, ROMName)
   PSC_PROP_RW(B2SServer, string, B2SName)
   PSC_PROP_RW(B2SServer, string, TableName)
   PSC_PROP_W(B2SServer, string, WorkingDir)
   PSC_FUNCTION1(B2SServer, void, SetPath, string)
   // [id(0x0000000b), propget] HRESULT Games([in] BSTR GameName, [out, retval] VARIANT* pRetVal);
   // [id(0x0000000c), propget] HRESULT Settings([out, retval] VARIANT* pRetVal);
   PSC_PROP_R(B2SServer, bool, Running)
   PSC_PROP_W(B2SServer, double, TimeFence)
   PSC_PROP_RW(B2SServer, bool, Pause)
   PSC_PROP_R(B2SServer, string, Version)
   PSC_PROP_R(B2SServer, string, VPMBuildVersion)
   PSC_FUNCTION0(B2SServer, void, Run)
   PSC_FUNCTION1(B2SServer, void, Run, int32)
   PSC_FUNCTION0(B2SServer, void, Stop)
   PSC_PROP_RW(B2SServer, bool, LaunchBackglass)
   PSC_PROP_RW(B2SServer, string, SplashInfoLine)
   PSC_PROP_RW(B2SServer, bool, ShowFrame)
   PSC_PROP_RW(B2SServer, bool, ShowTitle)
   PSC_PROP_RW(B2SServer, bool, ShowDMDOnly)
   PSC_PROP_RW(B2SServer, bool, ShowPinDMD)
   PSC_PROP_RW(B2SServer, bool, LockDisplay)
   PSC_PROP_RW(B2SServer, bool, DoubleSize)
   PSC_PROP_RW(B2SServer, bool, Hidden)
   // [id(0x0000001d)] HRESULT SetDisplayPosition([in] VARIANT x, [in] VARIANT y, [in, optional] VARIANT handle);
   // [id(0x0000001e)] HRESULT ShowOptsDialog([in] VARIANT handle);
   // [id(0x0000001f)] HRESULT ShowPathesDialog([in] VARIANT handle);
   // [id(0x00000020)] HRESULT ShowAboutDialog([in] VARIANT handle);
   // [id(0x00000021)] HRESULT CheckROMS([in] VARIANT showoptions, [in] VARIANT handle);
   PSC_PROP_RW(B2SServer, bool, PuPHide)
   PSC_PROP_RW(B2SServer, bool, HandleKeyboard)
   PSC_PROP_RW(B2SServer, bool, HandleMechanics)
   PSC_PROP_R_StructArray2(B2SServer, PinmameLampState, lampNo, state, ChangedLamps);
   PSC_PROP_R_StructArray2(B2SServer, PinmameSolenoidState, solNo, state, ChangedSolenoids);
   PSC_PROP_R_StructArray2(B2SServer, PinmameGIState, giNo, state, ChangedGIStrings);
   PSC_PROP_R_StructArray3_2(B2SServer, PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int);
   PSC_PROP_R_StructArray3_3(B2SServer, PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int, int);
   PSC_PROP_R_StructArray3_4(B2SServer, PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int, int, int);
   PSC_PROP_R_StructArray2(B2SServer, PinmameSoundCommand, sndNo, sndNo, NewSoundCommands); // 2nd field is unused
   PSC_PROP_R_ARRAY1(B2SServer, bool, Lamp, int)
   PSC_PROP_R_ARRAY1(B2SServer, bool, Solenoid, int)
   PSC_PROP_R_ARRAY1(B2SServer, int32, GIString, int)
   PSC_PROP_RW_ARRAY1(B2SServer, bool, Switch, int)
   PSC_PROP_W_ARRAY1(B2SServer, int32, Mech, int)
   PSC_PROP_R_ARRAY1(B2SServer, int32, GetMech, int)
   PSC_PROP_RW_ARRAY1(B2SServer, int32, Dip, int)
   PSC_PROP_RW_ARRAY1(B2SServer, int32, SolMask, int)
   PSC_PROP_R(B2SServer, int, RawDmdWidth)
   PSC_PROP_R(B2SServer, int, RawDmdHeight)
   PSC_PROP_R(B2SServer, ByteArray, RawDmdPixels)
   PSC_PROP_R(B2SServer, IntArray, RawDmdColoredPixels)
   PSC_PROP_R_StructArray3(B2SServer, PinmameNVRAMState, nvramNo, oldStat, currStat, ChangedNVRAM);
   // [id(0x00000037), propget] HRESULT NVRAM([out, retval] VARIANT* pRetVal);
   PSC_PROP_RW(B2SServer, int, SoundMode)

   PSC_FUNCTION2(B2SServer, void, B2SSetData, int, int)
   PSC_FUNCTION2(B2SServer, void, B2SSetData, string, int)
   PSC_FUNCTION1(B2SServer, void, B2SPulseData, int)
   PSC_FUNCTION1(B2SServer, void, B2SPulseData, string)
/*   PSC_FUNCTION3(B2SServer, void, B2SSetPos, int, int, int)
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
   PSC_FUNCTION1(B2SServer, void, B2SStopAllAnimations, string)
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
   PSC_FUNCTION1(B2SServer, void, B2SStopSound, string)*/
   // [id(0x00000064)] HRESULT B2SMapSound([in] VARIANT digit, [in] BSTR soundname);
PSC_CLASS_END(B2SServer)


///////////////////////////////////////////////////////////////////////////////
// Renderer
//

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;
static unsigned int onGameStartId, onGameEndId, onRenderBackglassId;

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
      loadedB2S.get();

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
   else
   {
      loadedB2S = std::future<std::shared_ptr<B2STable>>();
   }
}

void OnGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   if (loadedB2S.valid())
      loadedB2S.get();
   delete renderer;
   renderer = nullptr;
}

void OnRenderBackglass(const unsigned int eventId, void* userData, void* eventData)
{
   if (renderer)
   {
      renderer->Render(static_cast<VPXRenderBackglassContext*>(eventData));
   }
   else if (loadedB2S.valid())
   {
      renderer = new B2SRenderer(msgApi, endpointId, loadedB2S.get());
      loadedB2S = std::future<std::shared_ptr<B2STable>>();
   }
}


MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   // Request and setup shared login API
   LPISetup(endpointId, msgApi);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), OnGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), OnGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onRenderBackglassId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_RENDER_BACKGLASS), OnRenderBackglass, nullptr);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   msgApi->UnsubscribeMsg(onRenderBackglassId, OnRenderBackglass);
   msgApi->UnsubscribeMsg(onGameStartId, OnGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, OnGameEnd);
   msgApi->ReleaseMsgID(onRenderBackglassId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
