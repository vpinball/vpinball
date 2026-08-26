// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <format>

#include "common.h"
#include "plugins/VPXPlugin.h"
#include "pinmame/PinMAMEPlugin.h"
#include <altsound.h>

using namespace std::string_literals;
using namespace std::string_view_literals;

#ifndef _MSC_VER
#define strcpy_s(A, B, C) strncpy(A, C, B)
#endif

namespace AltSound
{

LPI_IMPLEMENT_CPP // Implement shared log support

   MSGPI_STRING_VAL_SETTING(altsoundFolderProp, "Folder", "AltSound Folder", "", true, "", 1024);

static constexpr uint32_t BUFFER_SIZE_FRAMES = 128;

static const MsgPluginAPI* msgApi = nullptr;
static const VPXPluginAPI* vpxApi = nullptr;

static uint32_t endpointId;
static unsigned int getVpxApiId;
static unsigned int getMachineStateId = 0;
static unsigned int onAudioCmdId = 0;
static unsigned int onAudioUpdateId = 0;
static std::unique_ptr<PinballPlugin::Controller::CtrlItemConsumer<ControllerDef>> controllers;
static std::unique_ptr<PinballPlugin::Controller::CtrlItemProvider<AudioSrcId>> altsoundAudioSrc;

static bool isRunning = false;

struct AudioCallbackData
{
   const MsgPluginAPI* msgApi;
   uint32_t endpointId;
   unsigned int onAudioUpdateId;
   AudioUpdateMsg* msg;
};

static void AudioCallback(const float* samples, size_t frameCount, uint32_t sampleRate, uint32_t channels, void* userData)
{
   if (!isRunning || !msgApi || onAudioUpdateId == 0 || !samples || frameCount == 0)
      return;

   const size_t bufferSizeBytes = frameCount * channels * sizeof(float);

   AudioUpdateMsg* pAudioUpdateMsg = new AudioUpdateMsg();
   pAudioUpdateMsg->sourceId = { endpointId, 0 };
   pAudioUpdateMsg->streamId = { endpointId, 0 };
   pAudioUpdateMsg->channelFormat = (channels == 1) ? CTLPI_AUDIO_FORMAT_CHANNEL_MONO : CTLPI_AUDIO_FORMAT_CHANNEL_STEREO;
   pAudioUpdateMsg->sampleFormat = CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT;
   pAudioUpdateMsg->sampleRate = sampleRate;
   pAudioUpdateMsg->volume = 1.0f;
   pAudioUpdateMsg->bufferSize = static_cast<unsigned int>(bufferSizeBytes);
   pAudioUpdateMsg->buffer = new uint8_t[bufferSizeBytes];

   memcpy(pAudioUpdateMsg->buffer, samples, bufferSizeBytes);

   AudioCallbackData* cbData = new AudioCallbackData { msgApi, endpointId, onAudioUpdateId, pAudioUpdateMsg };

   msgApi->RunOnMainThread(
      endpointId, 0,
      [](void* userData)
      {
         AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
         data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
         delete[] data->msg->buffer;
         delete data->msg;
         delete data;
      },
      cbData);
}

static void OnGameEvent(const unsigned int eventId, void* userData, void* msgData)
{
   if (isRunning)
      AltSoundProcessCommand(static_cast<const PinMAMEChildBoardEventMsg*>(msgData)->cmd, 0);
}

static void SetupAltSound()
{
   assert(!isRunning);

   const ControllerDef controller = controllers->With([](const std::vector<ControllerDef>& items) { return items.empty() ? ControllerDef { } : items.front(); });
   if (controller.gameId == nullptr || controller.endpointId == 0)
      return;
   const string pinmamePrefix(PMPI_GAMEID_PREFIX);
   const string gameId = string(controller.gameId).substr(pinmamePrefix.length());

   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);
   std::filesystem::path tablePath = tableInfo.path;

   std::filesystem::path basePath;

   // Priority 1: altsound/<rom> (library adds /altsound/<rom> to basePath)
   if (auto path1 = find_case_insensitive_file_path(tablePath.parent_path() / "altsound"sv / gameId); !path1.empty())
      basePath = tablePath.parent_path();
   // Priority 2: pinmame/altsound/<rom>
   else if (auto path2 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altsound"sv / gameId); !path2.empty())
      basePath = tablePath.parent_path() / "pinmame"sv;
   // Priority 3: global setting
   else
   {
      std::filesystem::path altsoundFolder = altsoundFolderProp_Get();
      if (!altsoundFolder.empty())
         basePath = altsoundFolder.parent_path();
   }

   if (basePath.empty())
      return;

   std::filesystem::path altsoundGamePath = basePath / "altsound"sv / gameId;
   if (!std::filesystem::exists(altsoundGamePath))
      return;

   LOGI(std::format("Found altsound directory for game: {} at {}", gameId, altsoundGamePath.string()));
   PinMAMEMachineStateMsg state { };
   state.version = 1;
   state.hardwareGen = 0;
   msgApi->BroadcastMsg(endpointId, getMachineStateId, &state);

   VPXInfo vpxInfo;
   vpxApi->GetVpxInfo(&vpxInfo);
   AltSoundSetLogger(vpxInfo.prefPath, ALTSOUND_LOG_LEVEL_INFO, false);

   LOGI(std::format("Initializing AltSound for game: {}, basePath: {}", gameId, basePath.string()));

   if (!AltSoundInit(basePath.string(), gameId, 44100, 2, BUFFER_SIZE_FRAMES))
   {
      LOGE("Failed to initialize AltSound for game: " + gameId);
      return;
   }

   AltSoundSetAudioCallback(AudioCallback, nullptr);
   AltSoundSetHardwareGen(static_cast<ALTSOUND_HARDWARE_GEN>(state.hardwareGen));

   isRunning = true;
   altsoundAudioSrc->SetItem(
      { .id = { endpointId, 0 }, .overrideId = { controller.endpointId, 0 }, .name = "AltSound", .desc = "AltSound audio stream", .target = CTLPI_AUDIO_TARGET_BACKGLASS });

   LOGI("AltSound initialized successfully for game: " + gameId);
}

static void StopAltSound()
{
   if (isRunning)
      return;

   isRunning = false;
   AltSoundShutdown();

   AudioUpdateMsg stopStreamMsg;
   stopStreamMsg.sourceId = { endpointId, 0 };
   stopStreamMsg.streamId = { endpointId, 0 };
   stopStreamMsg.buffer = nullptr;
   msgApi->BroadcastMsg(endpointId, onAudioUpdateId, &stopStreamMsg);

   altsoundAudioSrc->ClearItems();
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle

using namespace AltSound;

MSGPI_EXPORT void MSGPIAPI AltSoundPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi);

   msgApi->RegisterSetting(endpointId, &altsoundFolderProp);

   getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);

   onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);

   altsoundAudioSrc = std::make_unique<PinballPlugin::Controller::CtrlItemProvider<AudioSrcId>>(msgApi, endpointId, CTLPI_AUDIO_GET_SRC_MSG, CTLPI_AUDIO_ON_SRC_CHG_MSG);

   controllers = std::make_unique<PinballPlugin::Controller::CtrlItemConsumer<ControllerDef>>(
      msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
      [](std::vector<ControllerDef>& items)
      {
         const string pinmamePrefix(PMPI_GAMEID_PREFIX);
         std::erase_if(items, [&pinmamePrefix](const ControllerDef& src) { return !string(src.gameId).starts_with(pinmamePrefix); });
      },
      []() { StopAltSound(); }, []() { SetupAltSound(); });
   controllers->SelectItems(true);

   msgApi->SubscribeMsg(endpointId, onAudioCmdId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_AUDIO_CMD), OnGameEvent, nullptr);
   getMachineStateId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_GET_MACHINE_STATE);
}

MSGPI_EXPORT void MSGPIAPI AltSoundPluginUnload()
{
   if (isRunning)
      StopAltSound();

   msgApi->FlushPendingCallbacks(endpointId);

   controllers = nullptr;

   altsoundAudioSrc = nullptr;

   msgApi->UnsubscribeMsg(onAudioCmdId, OnGameEvent, nullptr);
   msgApi->ReleaseMsgID(onAudioUpdateId);
   msgApi->ReleaseMsgID(onAudioCmdId);

   msgApi->ReleaseMsgID(getMachineStateId);

   msgApi->ReleaseMsgID(getVpxApiId);

   vpxApi = nullptr;
   msgApi = nullptr;
}
