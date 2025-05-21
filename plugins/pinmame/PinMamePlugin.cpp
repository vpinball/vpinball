// license:GPLv3+

#include "common.h"
#include "MsgPlugin.h"
#include "LoggingPlugin.h"
#include "ScriptablePlugin.h"
#include "ControllerPlugin.h"
#include "PinMamePlugin.h"
#include "VPXPlugin.h" // Only used for optional feature (locating pinmame files along a VPX table)

#include <filesystem>
#include <cassert>

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Scriptable object definitions

#include "Rom.h"
#define PSC_VAR_SET_Rom(variant, value) PSC_VAR_SET_object(Rom, variant, value)
PSC_CLASS_START(Rom)
   PSC_PROP_R(Rom, string, Name)
   PSC_PROP_R(Rom, int32, State)
   PSC_PROP_R(Rom, string, StateDescription)
   PSC_PROP_R(Rom, int32, Length)
   PSC_PROP_R(Rom, int32, ExpLength)
   PSC_PROP_R(Rom, int32, Checksum)
   PSC_PROP_R(Rom, int32, ExpChecksum)
   PSC_PROP_R(Rom, int32, Flags)
   //PSC_FUNCTION0(Rom, void, Audit) // not yet supported (2 functions with the same name, matched by their arguments)
   PSC_FUNCTION1(Rom, void, Audit, bool)
PSC_CLASS_END(Rom)

#include "Roms.h"
PSC_CLASS_START(Roms)
PSC_CLASS_END(Roms)

#include "Settings.h"
#define PSC_VAR_SET_Settings(variant, value) PSC_VAR_SET_object(Settings, variant, value)
PSC_CLASS_START(Settings)
   PSC_PROP_RW_ARRAY1(Settings, int, Value, string)
PSC_CLASS_END(Settings)

#include "GameSettings.h"
#define PSC_VAR_SET_GameSettings(variant, value) PSC_VAR_SET_object(GameSettings, variant, value)
PSC_CLASS_START(GameSettings)
   PSC_PROP_RW_ARRAY1(GameSettings, int, Value, string)
PSC_CLASS_END(GameSettings)

#include "Game.h"
#define PSC_VAR_SET_Game(variant, value) PSC_VAR_SET_object(Game, variant, value)
PSC_CLASS_START(Game)
   PSC_PROP_R(Game, string, Name)
   PSC_PROP_R(Game, string, Description)
   PSC_PROP_R(Game, string, Year)
   PSC_PROP_R(Game, string, Manufacturer)
   PSC_PROP_R(Game, string, CloneOf)
   PSC_PROP_R(Game, GameSettings, Settings)
PSC_CLASS_END(Game)

#include "Games.h"
PSC_CLASS_START(Games)
PSC_CLASS_END(Games)

#include "ControllerSettings.h"
PSC_CLASS_START(ControllerSettings)
PSC_CLASS_END(ControllerSettings)

PSC_ARRAY1(ByteArray, uint8, 0)
#define PSC_VAR_SET_ByteArray(variant, value) PSC_VAR_SET_array1(ByteArray, variant, value)
#define PSC_VAR_ByteArray(variant) PSC_VAR_array1(uint8_t, variant)

PSC_ARRAY1(IntArray, int32, 0)
#define PSC_VAR_SET_IntArray(variant, value) PSC_VAR_SET_array1(IntArray, variant, value)

// Map a an array of struct to a 2 dimensions array of int32_t
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


#include "Controller.h"
PSC_CLASS_START(Controller)
   // Overall setup
   PSC_PROP_R(Controller, string, Version)
   PSC_PROP_RW(Controller, string, GameName)
   PSC_PROP_RW(Controller, string, SplashInfoLine)
   PSC_PROP_RW(Controller, bool, ShowTitle)
   PSC_PROP_RW(Controller, bool, HandleKeyboard)
   PSC_PROP_RW(Controller, bool, HandleMechanics)
   PSC_PROP_RW_ARRAY1(Controller, int32, SolMask, int)
   // Run/Pause/Stop
   PSC_FUNCTION0(Controller, void, Run)
   PSC_FUNCTION1(Controller, void, Run, int32)
   PSC_FUNCTION2(Controller, void, Run, int32, int)
   PSC_PROP_W(Controller, double, TimeFence)
   PSC_PROP_R(Controller, bool, Running)
   PSC_PROP_RW(Controller, bool, Pause)
   PSC_FUNCTION0(Controller, void, Stop)
   PSC_PROP_RW(Controller, bool, Hidden)
   // Emulated machine state access
   PSC_PROP_RW_ARRAY1(Controller, bool, Switch, int)
   PSC_PROP_W_ARRAY1(Controller, int32, Mech, int)
   PSC_PROP_R_ARRAY1(Controller, int32, GetMech, int)
   PSC_PROP_R_ARRAY1(Controller, bool, Lamp, int)
   PSC_PROP_R_ARRAY1(Controller, bool, Solenoid, int)
   PSC_PROP_R_ARRAY1(Controller, int32, GIString, int)
   PSC_PROP_RW_ARRAY1(Controller, int32, Dip, int)
   PSC_PROP_R_StructArray3(Controller, PinmameNVRAMState, nvramNo, oldStat, currStat, ChangedNVRAM);
   PSC_PROP_R_StructArray2(Controller, PinmameLampState, lampNo, state, ChangedLamps);
   PSC_PROP_R_StructArray2(Controller, PinmameGIState, giNo, state, ChangedGIStrings);
   PSC_PROP_R_StructArray2(Controller, PinmameSolenoidState, solNo, state, ChangedSolenoids);
   PSC_PROP_R_StructArray2(Controller, PinmameSoundCommand, sndNo, sndNo, NewSoundCommands); // 2nd field is unused
   PSC_PROP_R_StructArray3_2(Controller, PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int);
   PSC_PROP_R_StructArray3_3(Controller, PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int, int);
   PSC_PROP_R_StructArray3_4(Controller, PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int, int, int);
   PSC_PROP_R(Controller, int, RawDmdWidth)
   PSC_PROP_R(Controller, int, RawDmdHeight)
   PSC_PROP_R(Controller, ByteArray, RawDmdPixels)
   PSC_PROP_R(Controller, IntArray, RawDmdColoredPixels)
   // Overall information
   PSC_PROP_R_ARRAY1(Controller, Game, Games, string)
   // Deprecated properties
   PSC_PROP_RW(Controller, bool, DoubleSize)
   PSC_PROP_RW(Controller, bool, ShowFrame)
   PSC_PROP_RW(Controller, bool, ShowDMDOnly)
   PSC_PROP_RW(Controller, bool, ShowTitle)
   PSC_PROP_RW(Controller, int, FastFrames)
   PSC_PROP_RW(Controller, bool, IgnoreRomCrc)
   PSC_PROP_RW(Controller, bool, CabinetMode)
   PSC_PROP_RW(Controller, int, SoundMode)
   // Custom property to allow host to identify the object as the plugin version
   members.push_back({ { "IsPlugin" }, { "bool" }, 0, {}, [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { pRet->vBool = true; } });
PSC_CLASS_END(Controller)


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin interface

static MsgPluginAPI* msgApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;

static uint32_t endpointId;

static Controller* controller = nullptr;

PSC_ERROR_IMPLEMENT(scriptApi); // Implement script error

LPI_IMPLEMENT // Implement shared login support

void PINMAMECALLBACK OnLogMessage(PINMAME_LOG_LEVEL logLevel, const char* format, va_list args, void* const pUserData)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      if (logLevel == PINMAME_LOG_LEVEL_INFO)
      {
         LOGI("%s", buffer);
      }
      else if (logLevel == PINMAME_LOG_LEVEL_ERROR)
      {
         LOGE("%s", buffer);
      }
      free(buffer);
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Audio

static unsigned int onAudioUpdateId;
static AudioUpdateMsg* audioSrc = nullptr;

static void StopAudioStream()
{
   if (audioSrc != nullptr)
   {
      // Send an end of stream message
      AudioUpdateMsg* pendingAudioUpdate = new AudioUpdateMsg(); 
      memcpy(pendingAudioUpdate, audioSrc, sizeof(AudioUpdateMsg));
      msgApi->RunOnMainThread(0, [](void* userData) {
            AudioUpdateMsg* msg = static_cast<AudioUpdateMsg*>(userData);
            msgApi->BroadcastMsg(endpointId, onAudioUpdateId, msg);
            delete msg;
         }, pendingAudioUpdate);
      delete audioSrc;
      audioSrc = nullptr;
   }
}

int PINMAMECALLBACK OnAudioAvailable(PinmameAudioInfo* p_audioInfo, void* const pUserData)
{
   LOGI("format=%d, channels=%d, sampleRate=%.2f, framesPerSecond=%.2f, samplesPerFrame=%d, bufferSize=%d", p_audioInfo->format, p_audioInfo->channels, p_audioInfo->sampleRate,
      p_audioInfo->framesPerSecond, p_audioInfo->samplesPerFrame, p_audioInfo->bufferSize);
   if (((p_audioInfo->format == PINMAME_AUDIO_FORMAT_INT16) || (p_audioInfo->format == PINMAME_AUDIO_FORMAT_FLOAT))
      && ((p_audioInfo->channels == 1) || (p_audioInfo->channels == 2)))
   {
      audioSrc = new AudioUpdateMsg();
      audioSrc->id = { endpointId, 0 };
      audioSrc->type = (p_audioInfo->channels == 1) ? CTLPI_AUDIO_SRC_BACKGLASS_MONO : CTLPI_AUDIO_SRC_BACKGLASS_STEREO;
      audioSrc->format = (p_audioInfo->format == PINMAME_AUDIO_FORMAT_INT16) ? CTLPI_AUDIO_FORMAT_SAMPLE_INT16 : CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT;
      audioSrc->sampleRate = p_audioInfo->sampleRate;
   }
   else
   {
      StopAudioStream();
   }
   return p_audioInfo->samplesPerFrame;
}

int PINMAMECALLBACK OnAudioUpdated(void* p_buffer, int samples, void* const pUserData)
{
   if (audioSrc != nullptr)
   {
      // This callback is invoked on the emulation thread, with data only valid in the context of the call.
      // Therefore, we need to copy the data to feed them on the message thread.
      const int bytePerSample = (audioSrc->format == CTLPI_AUDIO_FORMAT_SAMPLE_INT16) ? 2 : 4;
      const int nChannels = (audioSrc->type == CTLPI_AUDIO_SRC_BACKGLASS_MONO) ? 1 : 2;
      AudioUpdateMsg* pendingAudioUpdate = new AudioUpdateMsg(); 
      memcpy(pendingAudioUpdate, audioSrc, sizeof(AudioUpdateMsg));
      pendingAudioUpdate->bufferSize = samples * bytePerSample * nChannels;
      pendingAudioUpdate->buffer = new uint8_t[pendingAudioUpdate->bufferSize];
      memcpy(pendingAudioUpdate->buffer, p_buffer, pendingAudioUpdate->bufferSize);
      msgApi->RunOnMainThread(0, [](void* userData) {
            AudioUpdateMsg* msg = static_cast<AudioUpdateMsg*>(userData);
            msgApi->BroadcastMsg(endpointId, onAudioUpdateId, msg);
            delete[] msg->buffer;
            delete msg;
         }, pendingAudioUpdate);
   }
   return samples;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Overall game messages

static void OnControllerGameStart(Controller*)
{
   assert(controller->GetRunning());
}

static void OnControllerGameEnd(Controller*)
{
   StopAudioStream();
}

static void OnControllerDestroyed(Controller*)
{
   StopAudioStream();
   controller = nullptr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   controller = nullptr;
   endpointId = sessionId;
   msgApi = api;

   // Request and setup shared login API
   LPISetup(endpointId, msgApi);

   // Setup our contribution to the controller messages
   onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);

   // Contribute our API to the script engine
   const unsigned int getScriptApiId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiId, &scriptApi);
   msgApi->ReleaseMsgID(getScriptApiId);
   auto regLambda = [&](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   auto aliasLambda = [&](const char* name, const char* aliasedType) { scriptApi->RegisterScriptTypeAlias(name, aliasedType); };
   auto arrayLambda = [&](ScriptArrayDef* sad) { scriptApi->RegisterScriptArrayType(sad); };
   RegisterRomSCD(regLambda);
   RegisterRomsSCD(regLambda);
   RegisterGameSCD(regLambda);
   RegisterGamesSCD(regLambda);
   RegisterSettingsSCD(regLambda);
   RegisterGameSettingsSCD(regLambda);
   RegisterControllerSCD(regLambda);
   RegisterControllerSettingsSCD(regLambda);
   RegisterByteArraySCD(arrayLambda);
   RegisterIntArraySCD(arrayLambda);
   RegisterStructArraySCD(arrayLambda);
   Controller_SCD->CreateObject = []()
   {
      assert(controller == nullptr); // We do not support having multiple instance running concurrently

      PinmameConfig config = {
         PINMAME_AUDIO_FORMAT_INT16,
         44100,
         "",
         NULL, // State update => prefer update on request
         NULL, // Display available => prefer state block
         NULL, // Display updated => prefer update on request
         &OnAudioAvailable,
         &OnAudioUpdated,
         NULL, // Mech available
         NULL, // Mech updated
         NULL, // Solenoid updated => prefer update on request
         NULL, // Console updated => TODO implement (for Stern SAM)
         NULL, // Is key pressed => TODO implement ?
         &OnLogMessage,
         NULL, // &OnSoundCommand, => see https://github.com/vpinball/libaltsound (implement inside this plugin or as another plugin ?)
      };

      // Define pinmame directory (for ROM, NVRAM, ... eventually using VPX API if available)
      string pinmamePath;
      VPXPluginAPI* vpxApi = nullptr;
      unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
      msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
      msgApi->ReleaseMsgID(getVpxApiId);
      if (vpxApi != nullptr)
      {
         VPXTableInfo tableInfo;
         vpxApi->GetTableInfo(&tableInfo);
         std::filesystem::path tablePath = tableInfo.path;
         pinmamePath = find_case_insensitive_directory_path(tablePath.parent_path().string() + PATH_SEPARATOR_CHAR + "pinmame"s);
      }
      if (pinmamePath.empty())
      {
         char pinMameFolder[512];
         msgApi->GetSetting("PinMAME", "PinMAMEPath", pinMameFolder, sizeof(pinMameFolder));
         pinmamePath = pinMameFolder;
         if (!pinmamePath.empty() && !pinmamePath.ends_with(PATH_SEPARATOR_CHAR))
            pinmamePath += PATH_SEPARATOR_CHAR;
      }
      if (pinmamePath.empty())
      {
         // FIXME implement a last resort or just ask the user to define its path setup in the settings ?
         #if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
            //pinmamePath = find_directory_case_insensitive(g_pvp->m_szMyPath, "pinmame"s);
         #else
            //pinmamePath = string(getenv("HOME")) + PATH_SEPARATOR_CHAR + ".pinmame" + PATH_SEPARATOR_CHAR;
         #endif
         LOGE("PinMAME path is not defined.");
      }
      #ifdef _MSC_VER
      strncpy_s(const_cast<char*>(config.vpmPath), PINMAME_MAX_PATH, pinmamePath.c_str(), PINMAME_MAX_PATH - 1);
      #else
      strncpy(const_cast<char*>(config.vpmPath), pinmamePath.c_str(), PINMAME_MAX_PATH - 1);
      #endif

      Controller* pController = new Controller(msgApi, endpointId, config);
      pController->SetOnDestroyHandler(OnControllerDestroyed);
      pController->SetOnGameStartHandler(OnControllerGameStart);
      pController->SetOnGameEndHandler(OnControllerGameEnd);
      controller = pController;
      return static_cast<void*>(pController);
   };
   scriptApi->SubmitTypeLibrary();
   scriptApi->SetCOMObjectOverride("VPinMAME.Controller", Controller_SCD);

   PinmameSetMsgAPI(msgApi, endpointId);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   PinmameSetMsgAPI(nullptr, 0);

   msgApi->ReleaseMsgID(onAudioUpdateId);
   scriptApi->SetCOMObjectOverride("VPinMAME.Controller", nullptr);
   // TODO we should unregister the script API contribution
   msgApi = nullptr;
}
