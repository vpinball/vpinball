// license:GPLv3+

#include "common.h"
#include "plugins/MsgPlugin.h"
#include "plugins/LoggingPlugin.h"
#include "plugins/ScriptablePlugin.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/VPXPlugin.h" // Only used for optional feature (locating PinMAME files along a VPX table)

#include <filesystem>
#include <cassert>
#include <charconv>

#include "Rom.h"
#include "Roms.h"
#include "Settings.h"
#include "GameSettings.h"
#include "Game.h"
#include "Games.h"
#include "ControllerSettings.h"
#include "Controller.h"

namespace PinMAME {

///////////////////////////////////////////////////////////////////////////////////////////////////
// Scriptable object definitions

#define PSC_VAR_SET_PinMAME_Rom(variant, value) PSC_VAR_SET_object(Rom, variant, value)
PSC_CLASS_START(PinMAME_Rom, Rom)
   PSC_PROP_R(string, Name)
   PSC_PROP_R(int32, State)
   PSC_PROP_R(string, StateDescription)
   PSC_PROP_R(int32, Length)
   PSC_PROP_R(int32, ExpLength)
   PSC_PROP_R(int32, Checksum)
   PSC_PROP_R(int32, ExpChecksum)
   PSC_PROP_R(int32, Flags)
   //PSC_FUNCTION0(void, Audit) // not yet supported (2 functions with the same name, matched by their arguments)
   PSC_FUNCTION1(void, Audit, bool)
PSC_CLASS_END()

PSC_CLASS_START(PinMAME_Roms, Roms)
PSC_CLASS_END()

#define PSC_VAR_SET_PinMAME_Settings(variant, value) PSC_VAR_SET_object(Settings, variant, value)
PSC_CLASS_START(PinMAME_Settings, Settings)
   PSC_PROP_RW_ARRAY1(int, Value, string)
PSC_CLASS_END()

#define PSC_VAR_SET_PinMAME_GameSettings(variant, value) PSC_VAR_SET_object(GameSettings, variant, value)
PSC_CLASS_START(PinMAME_GameSettings, GameSettings)
   PSC_PROP_RW_ARRAY1(int, Value, string)
PSC_CLASS_END()

#define PSC_VAR_SET_PinMAME_Game(variant, value) PSC_VAR_SET_object(Game, variant, value)
PSC_CLASS_START(PinMAME_Game, Game)
   PSC_PROP_R(string, Name)
   PSC_PROP_R(string, Description)
   PSC_PROP_R(string, Year)
   PSC_PROP_R(string, Manufacturer)
   PSC_PROP_R(string, CloneOf)
   PSC_PROP_R(PinMAME_GameSettings, Settings)
PSC_CLASS_END()

PSC_CLASS_START(PinMAME_Games, Games)
PSC_CLASS_END()

PSC_CLASS_START(PinMAME_ControllerSettings, ControllerSettings)
PSC_CLASS_END()

PSC_ARRAY1(PinMAME_ByteArray, uint8, 0)
#define PSC_VAR_SET_PinMAME_ByteArray(variant, value) PSC_VAR_SET_array1(PinMAME_ByteArray, variant, value)
#define PSC_VAR_PinMAME_ByteArray(variant) PSC_VAR_array1(uint8_t, variant)

PSC_ARRAY1(PinMAME_IntArray, int32, 0)
#define PSC_VAR_SET_PinMAME_IntArray(variant, value) PSC_VAR_SET_array1(PinMAME_IntArray, variant, value)

// Map a an array of struct to a 2 dimensions array of int32_t
PSC_ARRAY2(PinMAME_StructArray, int32, 0, 0)
#define PSC_VAR_SET_PinMAME_StructArray2(structType, fieldName1, fieldName2, variant, value) { \
      const unsigned int nDimensions = 2; \
      const std::vector<structType>& vec = (value); \
      const size_t size0 = vec.size(); \
      ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + nDimensions * sizeof(int) + size0 * (2 * sizeof(int32_t)))); \
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
#define PSC_VAR_SET_PinMAME_StructArray3(structType, fieldName1, fieldName2, fieldName3, variant, value) { \
      const unsigned int nDimensions = 3; \
      const std::vector<structType>& vec = (value); \
      const size_t size0 = vec.size(); \
      ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + nDimensions * sizeof(int) + size0 * (3 * sizeof(int32_t)))); \
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

#define PSC_PROP_R_StructArray2(type, fieldName1, fieldName2, name) \
   members.push_back( { { #name }, { "PinMAME_StructArray" }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_PinMAME_StructArray2(type, fieldName1, fieldName2, *pRet, static_cast<_BindedClass*>(me)->Get##name()); } });

#define PSC_PROP_R_StructArray3(type, fieldName1, fieldName2, fieldName3, name) \
   members.push_back( { { #name }, { "PinMAME_StructArray" }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_PinMAME_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<_BindedClass*>(me)->Get##name()); } });
#define PSC_PROP_R_StructArray3_2(type, fieldName1, fieldName2, fieldName3, name, arg1, arg2) \
   members.push_back( { { #name }, { "PinMAME_StructArray" }, 2, { { #arg1 }, { #arg2 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_PinMAME_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<_BindedClass*>(me)->Get##name(PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]))); } });
#define PSC_PROP_R_StructArray3_3(type, fieldName1, fieldName2, fieldName3, name, arg1, arg2, arg3) \
   members.push_back( { { #name }, { "PinMAME_StructArray"}, 3, { { #arg1 }, { #arg2 }, { #arg3 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_PinMAME_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<_BindedClass*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]) )); } } );
#define PSC_PROP_R_StructArray3_4(type, fieldName1, fieldName2, fieldName3, name, arg1, arg2, arg3, arg4) \
   members.push_back( { { #name }, { "PinMAME_StructArray" }, 4, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { \
         PSC_VAR_SET_PinMAME_StructArray3(type, fieldName1, fieldName2, fieldName3, *pRet, static_cast<_BindedClass*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]) )); } } );


PSC_CLASS_START(PinMAME_Controller, Controller)
   // Overall setup
   PSC_PROP_R(string, Version)
   PSC_PROP_RW(string, GameName)
   PSC_PROP_R(string, ROMName)
   PSC_PROP_RW(string, SplashInfoLine)
   PSC_PROP_RW(bool, ShowTitle)
   PSC_PROP_RW(bool, HandleKeyboard)
   PSC_PROP_RW(bool, HandleMechanics)
   PSC_PROP_RW_ARRAY1(int32, SolMask, int)
   // Run/Pause/Stop
   PSC_FUNCTION0(void, Run)
   PSC_FUNCTION1(void, Run, int32)
   PSC_FUNCTION2(void, Run, int32, int)
   PSC_PROP_W(double, TimeFence)
   PSC_PROP_R(bool, Running)
   PSC_PROP_RW(bool, Pause)
   PSC_FUNCTION0(void, Stop)
   PSC_PROP_RW(bool, Hidden)
   // Emulated machine state access
   PSC_PROP_RW_ARRAY1(bool, Switch, int)
   PSC_PROP_W_ARRAY1(int32, Mech, int)
   PSC_PROP_R_ARRAY1(int32, GetMech, int)
   PSC_PROP_R_ARRAY1(bool, Lamp, int)
   PSC_PROP_R_ARRAY1(bool, Solenoid, int)
   PSC_PROP_R_ARRAY1(int32, GIString, int)
   PSC_PROP_RW_ARRAY1(int32, Dip, int)
   PSC_PROP_R(PinMAME_ByteArray, NVRAM)
   PSC_PROP_R_StructArray3(PinmameNVRAMState, nvramNo, oldStat, currStat, ChangedNVRAM);
   PSC_PROP_R_StructArray2(PinmameLampState, lampNo, state, ChangedLamps);
   PSC_PROP_R_StructArray2(PinmameGIState, giNo, state, ChangedGIStrings);
   PSC_PROP_R_StructArray2(PinmameSolenoidState, solNo, state, ChangedSolenoids);
   PSC_PROP_R_StructArray2(PinmameSoundCommand, sndNo, sndNo, NewSoundCommands); // 2nd field is unused
   PSC_PROP_R_StructArray3_2(PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int);
   PSC_PROP_R_StructArray3_3(PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int, int);
   PSC_PROP_R_StructArray3_4(PinmameLEDState, ledNo, chgSeg, state, ChangedLEDs, int, int, int, int);
   PSC_PROP_R(int, RawDmdWidth)
   PSC_PROP_R(int, RawDmdHeight)
   PSC_PROP_R(PinMAME_ByteArray, RawDmdPixels)
   PSC_PROP_R(PinMAME_IntArray, RawDmdColoredPixels)
   // Overall information
   PSC_PROP_R_ARRAY1(PinMAME_Game, Games, string)
   // Deprecated properties
   PSC_PROP_RW(bool, DoubleSize)
   PSC_PROP_RW(bool, LockDisplay)
   PSC_PROP_RW(bool, ShowFrame)
   PSC_PROP_RW(bool, ShowDMDOnly)
   PSC_PROP_RW(bool, ShowTitle)
   PSC_PROP_RW(int, FastFrames)
   PSC_PROP_RW(bool, IgnoreRomCrc)
   PSC_PROP_RW(bool, CabinetMode)
   PSC_PROP_RW(int, SoundMode)
   PSC_FUNCTION0(void, ShowOptsDialog)
   PSC_FUNCTION1(void, ShowOptsDialog, int32)
   // Custom property to allow host to identify the object as the plugin version
   PSC_PROP_R(bool, IsPlugin)
PSC_CLASS_END()


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin interface

static const MsgPluginAPI* msgApi = nullptr;
static ScriptablePluginAPI* scriptApi = nullptr;
static unsigned int getScriptApiMsgId = 0;
static unsigned int getVpxApiMsgId = 0;

static uint32_t endpointId;

static Controller* controller = nullptr;

PSC_ERROR_IMPLEMENT(scriptApi); // Implement script error

LPI_IMPLEMENT_CPP // Implement shared log support

MSGPI_BOOL_VAL_SETTING(enableSoundProp, "Sound", "Enable Sound", "Enable sound emulation", true, true);
MSGPI_STRING_VAL_SETTING(pinMAMEPathProp, "PinMAMEPath", "PinMAME Path", "Folder that contains PinMAME subfolders (roms, nvram, ...)", true, "", 1024);
MSGPI_BOOL_VAL_SETTING(cheatProp, "Cheat", "Cheat Mode", "", true, false);

void PINMAMECALLBACK OnLogMessage(PINMAME_LOG_LEVEL logLevel, const char* format, va_list args, void* const pUserData)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      string buffer(size + 1, '\0');
      vsnprintf(buffer.data(), size + 1, format, args);
      buffer.pop_back(); // remove null terminator
      if (buffer.starts_with("Average FPS:"s))
      {
         // Skip as the FPS does not correspond to anything here
      }
      else if (logLevel == PINMAME_LOG_LEVEL_INFO)
      {
         LOGI("PinMAME: " + buffer);
      }
      else if (logLevel == PINMAME_LOG_LEVEL_ERROR)
      {
         LOGE("PinMAME: " + buffer);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Audio

static unsigned int onAudioUpdateId;
static unsigned int onAudioSrcChangedId;
static unsigned int getAudioSrcId;
static AudioUpdateMsg* audioSrc = nullptr;
static AudioSrcId audioSrcDef = {};

static void OnGetAudioSrc(const unsigned int msgId, void* userData, void* msgData)
{
   GetAudioSrcMsg* msg = static_cast<GetAudioSrcMsg*>(msgData);
   if (audioSrc != nullptr && msg->count < msg->maxEntryCount)
      memcpy(&msg->entries[msg->count], &audioSrcDef, sizeof(AudioSrcId));
   if (audioSrc != nullptr)
      msg->count++;
}

static void StopAudioStream()
{
   if (audioSrc != nullptr)
   {
      // Send an end of stream message
      AudioUpdateMsg* pendingAudioUpdate = new AudioUpdateMsg();
      memcpy(pendingAudioUpdate, audioSrc, sizeof(AudioUpdateMsg));
      msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
            AudioUpdateMsg* msg = static_cast<AudioUpdateMsg*>(userData);
            msgApi->BroadcastMsg(endpointId, onAudioUpdateId, msg);
            delete msg;
         }, pendingAudioUpdate);
      delete audioSrc;
      audioSrc = nullptr;
      memset(&audioSrcDef, 0, sizeof(audioSrcDef));
      msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
            msgApi->BroadcastMsg(endpointId, onAudioSrcChangedId, nullptr);
         }, nullptr);
   }
}

int PINMAMECALLBACK OnAudioAvailable(PinmameAudioInfo* p_audioInfo, void* const pUserData)
{
   LOGI(std::format("PinMAME: format={}, channels={}, sampleRate={:.2f}, framesPerSecond={:.2f}, samplesPerFrame={}, bufferSize={}", p_audioInfo->format == PINMAME_AUDIO_FORMAT_INT16 ? "INT16" : "FLOAT",
      p_audioInfo->channels, p_audioInfo->sampleRate,
      p_audioInfo->framesPerSecond, p_audioInfo->samplesPerFrame, p_audioInfo->bufferSize));
   if (((p_audioInfo->format == PINMAME_AUDIO_FORMAT_INT16) || (p_audioInfo->format == PINMAME_AUDIO_FORMAT_FLOAT))
      && ((p_audioInfo->channels == 1) || (p_audioInfo->channels == 2)))
   {
      audioSrc = new AudioUpdateMsg();
      audioSrc->volume = 1.0f;
      audioSrc->id = { endpointId, 0 };
      audioSrc->type = (p_audioInfo->channels == 1) ? CTLPI_AUDIO_SRC_BACKGLASS_MONO : CTLPI_AUDIO_SRC_BACKGLASS_STEREO;
      audioSrc->format = (p_audioInfo->format == PINMAME_AUDIO_FORMAT_INT16) ? CTLPI_AUDIO_FORMAT_SAMPLE_INT16 : CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT;
      audioSrc->sampleRate = p_audioInfo->sampleRate;

      audioSrcDef.id = audioSrc->id;
      audioSrcDef.overrideId = { 0, 0 };
      audioSrcDef.type = audioSrc->type;
      audioSrcDef.format = audioSrc->format;
      audioSrcDef.sampleRate = audioSrc->sampleRate;
      msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
            msgApi->BroadcastMsg(endpointId, onAudioSrcChangedId, nullptr);
         }, nullptr);
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
      msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
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

   // Little helper to log all devices exposed by the started game
   if (false)
   {
      msgApi->RunOnMainThread(
         endpointId, 1e-5,
         [](void*)
         {
            unsigned int getDevSrcMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
            GetDevSrcMsg getSrcMsg = { 0, 0, nullptr };
            msgApi->SendMsg(endpointId, getDevSrcMsgId, endpointId, &getSrcMsg);
            std::vector<DevSrcId> deviceSources(getSrcMsg.count);
            getSrcMsg = { getSrcMsg.count, 0, deviceSources.data() };
            msgApi->SendMsg(endpointId, getDevSrcMsgId, endpointId, &getSrcMsg);
            LOGD("PinMAME Controller started");
            for (const auto& devSrc : deviceSources)
            {
               LOGD(std::format("> Devices (id={:04x}.{:04x}):", devSrc.id.endpointId, devSrc.id.resId));
               for (unsigned int i = 0; i < devSrc.nDevices; i++)
               {
                  LOGD(std::format("  . {:04x}:{:04d} {}", devSrc.deviceDefs[i].id.groupId, devSrc.deviceDefs[i].id.deviceId, devSrc.deviceDefs[i].name));
               }
            }
            msgApi->ReleaseMsgID(getDevSrcMsgId);
         },
         nullptr);
   }
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

}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle

using namespace PinMAME;

MSGPI_EXPORT void MSGPIAPI PinMAMEPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   controller = nullptr;
   endpointId = sessionId;
   msgApi = api;

   // Optional VPX API
   getVpxApiMsgId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);

   // Request and setup shared login API
   LPISetup(endpointId, msgApi);

   msgApi->RegisterSetting(endpointId, &enableSoundProp);
   msgApi->RegisterSetting(endpointId, &pinMAMEPathProp);
   msgApi->RegisterSetting(endpointId, &cheatProp);

   // Setup our contribution to the controller messages
   onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);
   onAudioSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_SRC_CHG_MSG);
   getAudioSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_GET_SRC_MSG);
   msgApi->SubscribeMsg(endpointId, getAudioSrcId, OnGetAudioSrc, nullptr);

   // Contribute our API to the script engine
   getScriptApiMsgId = msgApi->GetMsgID(SCRIPTPI_NAMESPACE, SCRIPTPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getScriptApiMsgId, &scriptApi);
   auto regLambda = [](ScriptClassDef* scd) { scriptApi->RegisterScriptClass(scd); };
   auto arrayLambda = [](ScriptArrayDef* sad) { scriptApi->RegisterScriptArrayType(sad); };
   RegisterPinMAME_Rom(regLambda);
   RegisterPinMAME_Roms(regLambda);
   RegisterPinMAME_Game(regLambda);
   RegisterPinMAME_Games(regLambda);
   RegisterPinMAME_Settings(regLambda);
   RegisterPinMAME_GameSettings(regLambda);
   RegisterPinMAME_Controller(regLambda);
   RegisterPinMAME_ControllerSettings(regLambda);
   RegisterPinMAME_ByteArray(arrayLambda);
   RegisterPinMAME_IntArray(arrayLambda);
   RegisterPinMAME_StructArray(arrayLambda);
   PinMAME_Controller_SCD->CreateObject = []()
   {
      assert(controller == nullptr); // We do not support having multiple instance running concurrently

      PinmameConfig config = {
         PINMAME_AUDIO_FORMAT_INT16,
         44100,
         "",
         NULL, // State update => prefer update on request
         NULL, // Display available => prefer state block
         NULL, // Display updated => prefer update on request
         enableSoundProp_Val ? &OnAudioAvailable : NULL, //
         enableSoundProp_Val ? &OnAudioUpdated : NULL, //
         NULL, // Mech available
         NULL, // Mech updated
         NULL, // Solenoid updated => prefer update on request
         NULL, // Console updated => TODO implement (for Stern SAM)
         NULL, // Is key pressed => TODO implement ?
         &OnLogMessage,
         NULL, // Sound command callback - libpinmame broadcasts via message API directly
      };

      // Define pinmame directory (for ROM, NVRAM, ... eventually using VPX API if available)
      std::filesystem::path pinmamePath;
      VPXPluginAPI* vpxApi = nullptr;
      msgApi->BroadcastMsg(endpointId, getVpxApiMsgId, &vpxApi);
      
      // Priorize a pinmame folder along the table
      if (vpxApi != nullptr)
      {
         VPXTableInfo tableInfo;
         vpxApi->GetTableInfo(&tableInfo);
         std::filesystem::path tablePath = tableInfo.path;
         pinmamePath = find_case_insensitive_directory_path(tablePath.parent_path() / "pinmame" / "roms");
         if (!pinmamePath.empty())
            pinmamePath = pinmamePath.parent_path();
      }

      // Defaults to the global setting
      if (pinmamePath.empty())
         pinmamePath = pinMAMEPathProp_Get();

      // Custom platforms defaults
      #if (defined(__APPLE__) && ((defined(TARGET_OS_IOS) && TARGET_OS_IOS) || (defined(TARGET_OS_TV) && TARGET_OS_TV))) || defined(__ANDROID__)
      if (pinmamePath.empty() && vpxApi != nullptr)
      {
         VPXInfo vpxInfo;
         vpxApi->GetVpxInfo(&vpxInfo);
         pinmamePath = find_case_insensitive_directory_path(std::filesystem::path(vpxInfo.prefPath) / "pinmame"s);
      }
      #elif defined(__APPLE__) || defined(__linux__)
      if (pinmamePath.empty())
         pinmamePath = std::filesystem::path(getenv("HOME")) / ".pinmame";
      #endif

      // FIXME implement a last resort or just ask the user to define its path setup in the settings ?
      if (pinmamePath.empty())
         LOGE("PinMAME path is not defined."s);
      else
         strncpy_s(const_cast<char*>(config.vpmPath), PINMAME_MAX_PATH, (pinmamePath / "").string().c_str());

      Controller* pController = new Controller(msgApi, endpointId, config);
      pController->SetOnDestroyHandler(OnControllerDestroyed);
      pController->SetOnGameStartHandler(OnControllerGameStart);
      pController->SetOnGameEndHandler(OnControllerGameEnd);
      pController->SetCheat(cheatProp_Val);
      controller = pController;

      return static_cast<void*>(pController);
   };
   scriptApi->SubmitTypeLibrary(endpointId);
   scriptApi->SetCOMObjectOverride("VPinMAME.Controller", PinMAME_Controller_SCD);

   PinmameSetMsgAPI(const_cast<MsgPluginAPI*>(msgApi), endpointId);
}

MSGPI_EXPORT void MSGPIAPI PinMAMEPluginUnload()
{
   if (controller)
   {
      int nRemainingRef = 0;
      while (controller)
      {
         controller->Release();
         nRemainingRef++;
      }
      LOGE(std::format("PinMAME Controller was not destroyed before unloading the plugin ({} remaining references)", nRemainingRef));
   }
   StopAudioStream();

   scriptApi->SetCOMObjectOverride("VPinMAME.Controller", nullptr);
   auto regLambda = [](ScriptClassDef* scd) { scriptApi->UnregisterScriptClass(scd); };
   auto arrayLambda = [](ScriptArrayDef* sad) { scriptApi->UnregisterScriptArrayType(sad); };
   UnregisterPinMAME_Rom(regLambda);
   UnregisterPinMAME_Roms(regLambda);
   UnregisterPinMAME_Game(regLambda);
   UnregisterPinMAME_Games(regLambda);
   UnregisterPinMAME_Settings(regLambda);
   UnregisterPinMAME_GameSettings(regLambda);
   UnregisterPinMAME_Controller(regLambda);
   UnregisterPinMAME_ControllerSettings(regLambda);
   UnregisterPinMAME_ByteArray(arrayLambda);
   UnregisterPinMAME_IntArray(arrayLambda);
   UnregisterPinMAME_StructArray(arrayLambda);

   msgApi->ReleaseMsgID(getVpxApiMsgId);
   msgApi->ReleaseMsgID(getScriptApiMsgId);
   msgApi->ReleaseMsgID(onAudioUpdateId);
   msgApi->UnsubscribeMsg(getAudioSrcId, OnGetAudioSrc, nullptr);
   msgApi->ReleaseMsgID(getAudioSrcId);
   msgApi->ReleaseMsgID(onAudioSrcChangedId);
   msgApi->FlushPendingCallbacks(endpointId);
   PinmameSetMsgAPI(nullptr, 0);
   msgApi = nullptr;
}
