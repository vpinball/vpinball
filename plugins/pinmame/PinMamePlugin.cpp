// license:GPLv3+

#include "common.h"
#include "MsgPlugin.h"
#include "LoggingPlugin.h"
#include "ScriptablePlugin.h"
#include "CorePlugin.h"
#include "PinMamePlugin.h"
#include "VPXPlugin.h" // Only used for optional feature (locating pinmame files along a VPX table)

#include <filesystem>
#include <cassert>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Scriptable object definitions

#include "Rom.h"
#define PSC_VAR_SET_Rom(variant, value) PSC_VAR_SET_object(Rom, variant, value)
PSC_CLASS_START(Rom)
   PSC_PROP_R(Rom, string, Name)
   PSC_PROP_R(Rom, long, State)
   PSC_PROP_R(Rom, string, StateDescription)
   PSC_PROP_R(Rom, long, Length)
   PSC_PROP_R(Rom, long, ExpLength)
   PSC_PROP_R(Rom, long, Checksum)
   PSC_PROP_R(Rom, long, ExpChecksum)
   PSC_PROP_R(Rom, long, Flags)
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

#include "Game.h"
#define PSC_VAR_SET_Game(variant, value) PSC_VAR_SET_object(Game, variant, value)
PSC_CLASS_START(Game)
   PSC_PROP_R(Game, string, Name)
   PSC_PROP_R(Game, string, Description)
   PSC_PROP_R(Game, string, Year)
   PSC_PROP_R(Game, string, Manufacturer)
   PSC_PROP_R(Game, string, CloneOf)
   PSC_PROP_R(Game, Settings, Settings)
PSC_CLASS_END(Game)

#include "Games.h"
PSC_CLASS_START(Games)
PSC_CLASS_END(Games)

#include "GameSettings.h"
PSC_CLASS_START(GameSettings)
PSC_CLASS_END(GameSettings)

#include "ControllerSettings.h"
PSC_CLASS_START(ControllerSettings)
PSC_CLASS_END(ControllerSettings)

PSC_ARRAY1(ByteArray, uchar, 0)
#define PSC_VAR_SET_ByteArray(variant, value) PSC_VAR_SET_array1(ByteArray, variant, value)
#define PSC_VAR_ByteArray(variant) PSC_VAR_array1(uint8_t, variant)

PSC_ARRAY1(IntArray, uint, 0)
#define PSC_VAR_SET_IntArray(variant, value) PSC_VAR_SET_array1(IntArray, variant, value)

// Array of struct with fields of the same type that can be considered as a 2 dimensional array
#define PSC_VAR_SET_StructArray(structType, nFields, variant, value) { \
      const unsigned int nDimensions = 2; \
      const std::vector<structType>& vec = (value); \
      const size_t size0 = vec.size(); \
      const size_t subDataSize = sizeof(structType); \
      const size_t dataSize = size0 * subDataSize; \
      ScriptArray* array = static_cast<ScriptArray*>(malloc(sizeof(ScriptArray) + nDimensions * sizeof(int) + dataSize)); \
      array->Release = [](ScriptArray* me) { free(me); }; \
      array->lengths[0] = static_cast<unsigned int>(vec.size()); \
      array->lengths[1] = nFields; \
      char* pData = reinterpret_cast<char*>(&array->lengths[2]); \
      for (size_t i = 0; i < size0; i++, pData += subDataSize) \
         memcpy(pData, &vec[i], subDataSize); \
      (variant).vArray = array; \
   }
#define PSC_PROP_R_StructArray(className, type, nFields, changeType, name) \
   members.push_back( { { #name }, { #type }, 0, { }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { PSC_VAR_SET_##type(changeType, nFields, *pRet, static_cast<className*>(me)->Get##name()) } });
#define PSC_PROP_R_StructArray2(className, type, nFields, changeType, name, arg1, arg2) \
   members.push_back( { { #name }, { #type }, 2, { { #arg1 }, { #arg2 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { PSC_VAR_SET_##type(changeType, nFields, *pRet, static_cast<className*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]) )) } });
#define PSC_PROP_R_StructArray3(className, type, nFields, changeType, name, arg1, arg2, arg3) \
   members.push_back( { { #name }, { #type }, 3, { { #arg1 }, { #arg2 }, { #arg3 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { PSC_VAR_SET_##type(changeType, nFields, *pRet, static_cast<className*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]) )) } });
#define PSC_PROP_R_StructArray4(className, type, nFields, changeType, name, arg1, arg2, arg3, arg4) \
   members.push_back( { { #name }, { #type }, 4, { { #arg1 }, { #arg2 }, { #arg3 }, { #arg4 } }, \
      [](void* me, int, ScriptVariant* pArgs, ScriptVariant* pRet) { PSC_VAR_SET_##type(changeType, nFields, *pRet, static_cast<className*>(me)->Get##name( PSC_VAR_##arg1(pArgs[0]), PSC_VAR_##arg2(pArgs[1]), PSC_VAR_##arg3(pArgs[2]), PSC_VAR_##arg4(pArgs[3]) )) } });
PSC_ARRAY2(StructArray, int, 0, 0)


#include "Controller.h"
PSC_CLASS_START(Controller)
   // Overall setup
   PSC_PROP_R(Controller, string, Version)
   PSC_PROP_RW(Controller, string, GameName)
   PSC_PROP_RW(Controller, string, SplashInfoLine)
   PSC_PROP_RW(Controller, bool, ShowTitle)
   PSC_PROP_RW(Controller, bool, HandleKeyboard)
   PSC_PROP_RW(Controller, bool, HandleMechanics)
   PSC_PROP_RW_ARRAY1(Controller, long, SolMask, int)
   // Run/Pause/Stop
   PSC_FUNCTION0(Controller, void, Run)
   PSC_FUNCTION1(Controller, void, Run, long)
   PSC_FUNCTION2(Controller, void, Run, long, int)
   PSC_PROP_W(Controller, double, TimeFence)
   PSC_PROP_R(Controller, bool, Running)
   PSC_PROP_RW(Controller, bool, Pause)
   PSC_FUNCTION0(Controller, void, Stop)
   PSC_PROP_RW(Controller, bool, Hidden)
   // Emulated machine state access
   PSC_PROP_RW_ARRAY1(Controller, bool, Switch, int)
   PSC_PROP_W_ARRAY1(Controller, int, Mech, int)
   PSC_PROP_R_ARRAY1(Controller, int, GetMech, int)
   PSC_PROP_R_ARRAY1(Controller, bool, Lamp, int)
   PSC_PROP_R_ARRAY1(Controller, bool, Solenoid, int)
   PSC_PROP_R_ARRAY1(Controller, int, GIString, int)
   PSC_PROP_RW_ARRAY1(Controller, int, Dip, int)
   PSC_PROP_R_StructArray(Controller, StructArray, 3, PinmameNVRAMState, ChangedNVRAM);
   PSC_PROP_R_StructArray(Controller, StructArray, 2, PinmameLampState, ChangedLamps);
   PSC_PROP_R_StructArray(Controller, StructArray, 2, PinmameGIState, ChangedGIStrings);
   PSC_PROP_R_StructArray(Controller, StructArray, 2, PinmameSolenoidState, ChangedSolenoids);
   PSC_PROP_R_StructArray(Controller, StructArray, 2, PinmameSoundCommand, NewSoundCommands);
   PSC_PROP_R_StructArray2(Controller, StructArray, 3, PinmameLEDState, ChangedLEDs, int, int);
   PSC_PROP_R_StructArray3(Controller, StructArray, 3, PinmameLEDState, ChangedLEDs, int, int, int);
   PSC_PROP_R_StructArray4(Controller, StructArray, 3, PinmameLEDState, ChangedLEDs, int, int, int, int);
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
static unsigned int onGameStartId, onGameEndId;

static Controller* controller = nullptr;

PSC_ERROR_IMPLEMENT(scriptApi); // Implement script error

LPI_IMPLEMENT // Implement shared login support

void PINMAMECALLBACK OnLogMessage(PINMAME_LOG_LEVEL logLevel, const char* format, va_list args, void* const pUserData)
{
   int size = vsnprintf(nullptr, 0, format, args);
   if (size > 0)
   {
      char* buffer = static_cast<char*>(malloc(size + 1));
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
// Alphanumeric segment displays

static bool hasAlpha = false;
static unsigned int onSegSrcChangedId, getSegSrcId, getSegId;
static unsigned int segFrameId = 0;
static float segLuminances[16 * 128] = { 0 };
static float segPrevLuminances[16 * 128] = { 0 };

static void onGetSegSrc(const unsigned int eventId, void* userData, void* msgData)
{
   if (!hasAlpha || (controller == nullptr))
      return;
   GetSegSrcMsg& msg = *static_cast<GetSegSrcMsg*>(msgData);

   auto block = controller->GetStateBlock(PINMAME_STATE_REQMASK_ALPHA_DEVICE_STATE);
   if (block == nullptr)
      return;
   pinmame_tAlphaStates* states = block->alphaDisplayStates;
   if (states == nullptr)
      return;
   pinmame_tAlphaSegmentState* alphaStates = PINMAME_STATE_BLOCK_FIRST_ALPHA_FRAME(states);
   for (unsigned int i = 0, pos = 0; (msg.count < msg.maxEntryCount) && (i < states->nDisplays); i++)
   {
      assert(states->displayDefs[i].nElements < CTLPI_SEG_MAX_DISP_ELEMENTS);
      msg.entries[msg.count].id = { endpointId, 0 };
      msg.entries[msg.count].nDisplaysInGroup = states->nDisplays;
      msg.entries[msg.count].displayIndex = i;
      msg.entries[msg.count].hardware = CTLPI_GETSEG_HARDWARE_UNKNOWN;
      msg.entries[msg.count].nElements = states->displayDefs[i].nElements;
      for (unsigned int j = 0; j < states->displayDefs[i].nElements; j++, pos++)
         msg.entries[msg.count].elementType[j] = static_cast<SegElementType>(alphaStates[pos].type);
      msg.count++;
   }
}

static void onGetSeg(const unsigned int eventId, void* userData, void* msgData)
{
   if (!hasAlpha || (controller == nullptr))
      return;
   GetSegMsg& msg = *static_cast<GetSegMsg*>(msgData);
   if ((msg.frame != nullptr) || (msg.segId.endpointId != endpointId) || (msg.segId.resId != 0))
      return;

   auto block = controller->GetStateBlock(PINMAME_STATE_REQMASK_ALPHA_DEVICE_STATE);
   if (block == nullptr)
      return;
   pinmame_tAlphaStates* state = block->alphaDisplayStates;
   if (state == nullptr)
      return;
   pinmame_tAlphaSegmentState* alphaStates = PINMAME_STATE_BLOCK_FIRST_ALPHA_FRAME(state);
   static constexpr int nSegments[] = { 7, 8, 8, 10, 10, 15, 15, 16, 16 };
   unsigned int nElements = 0;
   for (unsigned int display = 0; display < state->nDisplays; display++)
      nElements += state->displayDefs[display].nElements;
   for (unsigned int i = 0; i < nElements; i++)
      memcpy(&segLuminances[i * 16], alphaStates[i].luminance, nSegments[alphaStates[i].type] * sizeof(float));
   if (memcmp(segPrevLuminances, segLuminances, nElements * 16 * sizeof(float)) != 0)
   {
      memcpy(segPrevLuminances, segLuminances, nElements * 16 * sizeof(float));
      segFrameId++;
   }
   msg.frame = segLuminances;
   msg.frameId = segFrameId;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// DMD & Displays

static bool hasDMD = false;
static unsigned int onDmdSrcChangedId, getDmdSrcId, getRenderDmdId, getIdentifyDmdId;

static void onGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData)
{
   if (!hasDMD || (controller == nullptr))
      return;
   GetDmdSrcMsg& msg = *static_cast<GetDmdSrcMsg*>(msgData);

   auto block = controller->GetStateBlock(PINMAME_STATE_REQMASK_DISPLAY_STATE);
   if (block == nullptr)
      return;
   pinmame_tDisplayStates* state = block->displayStates;
   if (state == nullptr)
      return;

   pinmame_tFrameState* frame = PINMAME_STATE_BLOCK_FIRST_DISPLAY_FRAME(state);
   for (unsigned int index = 0; index < state->nDisplays; index++)
   {
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count].id = { endpointId, static_cast <uint32_t>(frame->displayId) };
         msg.entries[msg.count].format = frame->dataFormat;
         msg.entries[msg.count].width = frame->width;
         msg.entries[msg.count].height = frame->height;
         msg.count++;
      }
      frame = PINMAME_STATE_BLOCK_NEXT_DISPLAY_FRAME(frame);
   }
}

static void onGetIdentifyDMD(const unsigned int eventId, void* userData, void* msgData)
{
   if (!hasDMD || (controller == nullptr))
      return;
   GetRawDmdMsg* const msg = static_cast<GetRawDmdMsg*>(msgData);
   if ((msg->frame != nullptr) || (msg->dmdId.endpointId != endpointId))
      return;

   auto block = controller->GetStateBlock(PINMAME_STATE_REQMASK_DISPLAY_STATE);
   if (block == nullptr)
      return;
   pinmame_tDisplayStates* state = block->rawDMDStates;
   if (state == nullptr)
      return;

   pinmame_tFrameState* frame = PINMAME_STATE_BLOCK_FIRST_DISPLAY_FRAME(state);
   for (unsigned int index = 0; index < state->nDisplays; index++)
   {
      if (msg->dmdId.resId == frame->displayId)
      {
         msg->format = frame->dataFormat;
         msg->width = frame->width;
         msg->height = frame->height;
         msg->frameId = frame->frameId;
         msg->frame = frame->frameData;
         return;
      }
      frame = PINMAME_STATE_BLOCK_NEXT_DISPLAY_FRAME(frame);
   }
}

static void onGetRenderDMD(const unsigned int eventId, void* userData, void* msgData)
{
   if (!hasDMD || (controller == nullptr))
      return;
   GetDmdMsg* const msg = static_cast<GetDmdMsg*>(msgData);
   if ((msg->frame != nullptr) || (msg->dmdId.id.endpointId != endpointId))
      return;

   auto block = controller->GetStateBlock(PINMAME_STATE_REQMASK_DISPLAY_STATE);
   if (block == nullptr)
      return;
   pinmame_tDisplayStates* state = block->displayStates;
   if (state == nullptr)
      return;

   pinmame_tFrameState* frame = PINMAME_STATE_BLOCK_FIRST_DISPLAY_FRAME(state);
   for (unsigned int index = 0; index < state->nDisplays; index++)
   {
      if (msg->dmdId.id.resId == frame->displayId)
      {
         if ((msg->dmdId.width == frame->width) && (msg->dmdId.height == frame->height) && (msg->dmdId.format == frame->dataFormat))
         {
            msg->frameId = frame->frameId;
            msg->frame = frame->frameData;
         }
         return;
      }
      frame = PINMAME_STATE_BLOCK_NEXT_DISPLAY_FRAME(frame);
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Overall game messages

static void OnControllerGameStart(Controller*)
{
   assert(controller->GetRunning());
   const string& gameName = controller->GetGameName();
   PMPI_MSG_ON_GAME_START msg = { controller->GetVpmPath().c_str(), gameName.c_str() };
   msgApi->BroadcastMsg(endpointId, onGameStartId, reinterpret_cast<void*>(&msg));
   auto block = controller->GetStateBlock(PINMAME_STATE_REQMASK_ALL);
   if (block != nullptr)
   {
      hasDMD = (block->displayStates != nullptr) && (block->displayStates->nDisplays > 0);
      if (hasDMD)
      {
         msgApi->SubscribeMsg(endpointId, getDmdSrcId, onGetRenderDMDSrc, controller);
         msgApi->SubscribeMsg(endpointId, getRenderDmdId, onGetRenderDMD, controller);
         msgApi->SubscribeMsg(endpointId, getIdentifyDmdId, onGetIdentifyDMD, controller);
         msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr);
      }
      hasAlpha = (block->alphaDisplayStates != nullptr) && (block->alphaDisplayStates->nDisplays > 0);
      if (hasAlpha)
      {
         msgApi->SubscribeMsg(endpointId, getSegSrcId, onGetSegSrc, controller);
         msgApi->SubscribeMsg(endpointId, getSegId, onGetSeg, controller);
         msgApi->BroadcastMsg(endpointId, onSegSrcChangedId, nullptr);
      }
   }
}

static void OnControllerGameEnd(Controller*)
{
   if (hasDMD)
   {
      hasDMD = false;
      msgApi->UnsubscribeMsg(getDmdSrcId, onGetRenderDMDSrc);
      msgApi->UnsubscribeMsg(getRenderDmdId, onGetRenderDMD);
      msgApi->UnsubscribeMsg(getIdentifyDmdId, onGetIdentifyDMD);
   }
   if (hasAlpha)
   {
      hasAlpha = false;
      msgApi->UnsubscribeMsg(getSegSrcId, onGetSegSrc);
      msgApi->UnsubscribeMsg(getSegId, onGetSeg);
   }
   // Broadcast message after unsubscribing to avoid receiving unwanted requests
   msgApi->BroadcastMsg(endpointId, onGameEndId, nullptr);
   msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr);
   msgApi->BroadcastMsg(endpointId, onSegSrcChangedId, nullptr);
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

   // Setup our own messages
   onGameStartId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_START);
   onGameEndId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_END);

   // Setup our contribution to the controller messages
   onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDMD_SRC_CHG_MSG);
   getDmdSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG);
   getRenderDmdId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
   getIdentifyDmdId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_IDENTIFY_MSG);
   onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDMD_SRC_CHG_MSG);
   onSegSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONSEG_SRC_CHG_MSG);
   getSegSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETSEG_SRC_MSG);
   getSegId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETSEG_MSG);
   onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONAUDIO_UPDATE_MSG);

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
      unsigned int getVpxApiId;
      VPXPluginAPI* vpxApi = nullptr;
      msgApi->BroadcastMsg(endpointId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);
      msgApi->ReleaseMsgID(getVpxApiId);
      if (vpxApi != nullptr)
      {
         VPXTableInfo tableInfo;
         vpxApi->GetTableInfo(&tableInfo);
         std::filesystem::path tablePath = tableInfo.path;
         pinmamePath = find_directory_case_insensitive(tablePath.parent_path().string(), "pinmame");
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
            //pinmamePath = find_directory_case_insensitive(g_pvp->m_szMyPath, "pinmame");
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

      Controller* pController = new Controller(config);
      pController->SetOnDestroyHandler(OnControllerDestroyed);
      pController->SetOnGameStartHandler(OnControllerGameStart);
      pController->SetOnGameEndHandler(OnControllerGameEnd);
      controller = pController;
      return static_cast<void*>(pController);
   };
   scriptApi->SubmitTypeLibrary();
   scriptApi->SetCOMObjectOverride("VPinMAME.Controller", Controller_SCD);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcId);
   msgApi->ReleaseMsgID(getRenderDmdId);
   msgApi->ReleaseMsgID(getIdentifyDmdId);
   msgApi->ReleaseMsgID(onSegSrcChangedId);
   msgApi->ReleaseMsgID(getSegSrcId);
   msgApi->ReleaseMsgID(getSegId);
   msgApi->ReleaseMsgID(onAudioUpdateId);
   scriptApi->SetCOMObjectOverride("VPinMAME.Controller", nullptr);
   // TODO we should unregister the script API contribution
   msgApi = nullptr;
}
