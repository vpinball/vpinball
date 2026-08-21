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

namespace AltSound {

LPI_IMPLEMENT_CPP // Implement shared log support

MSGPI_STRING_VAL_SETTING(altsoundFolderProp, "Folder", "AltSound Folder", "", true, "", 1024);

static constexpr uint32_t BUFFER_SIZE_FRAMES = 128;

static const MsgPluginAPI* msgApi = nullptr;
static const VPXPluginAPI* vpxApi = nullptr;

static uint32_t endpointId;
static unsigned int getVpxApiId;
static unsigned int onControllersChangedId;
static unsigned int getControllersId;
static unsigned int getMachineStateId = 0;
static unsigned int onAudioCmdId = 0;
static unsigned int onAudioUpdateId = 0;
static unsigned int onAudioSrcChangedId = 0;
static unsigned int getAudioSrcId = 0;

static bool isRunning = false;
static string currentGameId;

static AudioSrcId audioSrcDef = { .id = { 0, 0 }, .overrideId = { 0, 0 }, .name = "AltSound", .desc = "AltSound audio stream", .target = CTLPI_AUDIO_TARGET_BACKGLASS };

struct AudioCallbackData {
    const MsgPluginAPI* msgApi;
    uint32_t endpointId;
    unsigned int onAudioUpdateId;
    AudioUpdateMsg* msg;
};

static void OnGetAudioSrc(const unsigned int msgId, void* userData, void* msgData)
{
    GetAudioSrcMsg* msg = static_cast<GetAudioSrcMsg*>(msgData);
    if (isRunning)
    {
       if (msg->count < msg->maxEntryCount)
          memcpy(&msg->entries[msg->count], &audioSrcDef, sizeof(AudioSrcId));
       msg->count++;
    }
}

static void AudioCallback(const float* samples, size_t frameCount, uint32_t sampleRate, uint32_t channels, void* userData)
{
    if (!isRunning || !msgApi || onAudioUpdateId == 0 || !samples || frameCount == 0)
        return;

    const size_t bufferSizeBytes = frameCount * channels * sizeof(float);

    AudioUpdateMsg* pAudioUpdateMsg = new AudioUpdateMsg();
    pAudioUpdateMsg->sourceId = audioSrcDef.id;
    pAudioUpdateMsg->streamId = audioSrcDef.id;
    pAudioUpdateMsg->channelFormat = (channels == 1) ? CTLPI_AUDIO_FORMAT_CHANNEL_MONO : CTLPI_AUDIO_FORMAT_CHANNEL_STEREO;
    pAudioUpdateMsg->sampleFormat = CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT;
    pAudioUpdateMsg->sampleRate = sampleRate;
    pAudioUpdateMsg->volume = 1.0f;
    pAudioUpdateMsg->bufferSize = static_cast<unsigned int>(bufferSizeBytes);
    pAudioUpdateMsg->buffer = new uint8_t[bufferSizeBytes];

    memcpy(pAudioUpdateMsg->buffer, samples, bufferSizeBytes);

    AudioCallbackData* cbData = new AudioCallbackData{msgApi, endpointId, onAudioUpdateId, pAudioUpdateMsg};

    msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
        AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
        data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
        delete[] data->msg->buffer;
        delete data->msg;
        delete data;
    }, cbData);
}

static void StartAltSound(const string& gameId, const string& basePath, uint64_t hardwareGen)
{
    if (isRunning) {
        isRunning = false;
        AltSoundShutdown();
    }

    VPXInfo vpxInfo;
    vpxApi->GetVpxInfo(&vpxInfo);
    AltSoundSetLogger(vpxInfo.prefPath, ALTSOUND_LOG_LEVEL_INFO, false);

    LOGI(std::format("Initializing AltSound for game: {}, basePath: {}", gameId, basePath));

    if (AltSoundInit(basePath, gameId, 44100, 2, BUFFER_SIZE_FRAMES)) {
        AltSoundSetAudioCallback(AudioCallback, nullptr);
        AltSoundSetHardwareGen(static_cast<ALTSOUND_HARDWARE_GEN>(hardwareGen));

        isRunning = true;
        msgApi->BroadcastMsg(endpointId, onAudioSrcChangedId, nullptr);

        LOGI("AltSound initialized successfully for game: " + gameId);
    } else {
        LOGE("Failed to initialize AltSound for game: " + gameId);
    }
}

static void StopAltSound()
{
    if (!isRunning)
       return;

    isRunning = false;
    AltSoundShutdown();

    AudioUpdateMsg* pAudioUpdateMsg = new AudioUpdateMsg();
    pAudioUpdateMsg->sourceId = audioSrcDef.id;
    pAudioUpdateMsg->streamId = audioSrcDef.id;
    pAudioUpdateMsg->buffer = nullptr;

    AudioCallbackData* cbData = new AudioCallbackData{msgApi, endpointId, onAudioUpdateId, pAudioUpdateMsg};

    msgApi->RunOnMainThread(endpointId, 0, [](void* userData) {
        AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
        data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
        delete data->msg;
        delete data;
    }, cbData);

    msgApi->BroadcastMsg(endpointId, onAudioSrcChangedId, nullptr);
}

static void OnGameEvent(const unsigned int eventId, void* userData, void* msgData)
{
   if (isRunning)
      AltSoundProcessCommand(static_cast<const PinMAMEChildBoardEventMsg*>(msgData)->cmd, 0);
}

static void OnControllersChanged(const unsigned int eventId, void* userData, void* msgData)
{
   // Enumerate and select the first controller exposing a PinMAME compatible game
   uint32_t sourceEndpointId = 0;
   string selectedGameId;
   const string pinmamePrefix(PMPI_GAMEID_PREFIX);
   for (const auto& controller : PinballPlugin::Controller::GetCtrlItems<ControllerDef>(msgApi, endpointId, getControllersId))
   {
      string gameId = controller.gameId;
      if (gameId.starts_with(pinmamePrefix))
      {
         sourceEndpointId = controller.endpointId;
         selectedGameId = gameId.substr(pinmamePrefix.length());
         if (!selectedGameId.empty())
            break;
      }
   }
   if (currentGameId == selectedGameId)
      return;

   // Setup on the selected game if any
   currentGameId = selectedGameId;
   
   if (isRunning)
      StopAltSound();
   
   if (currentGameId.empty())
      return;

   audioSrcDef.overrideId.endpointId = sourceEndpointId;

   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);
   std::filesystem::path tablePath = tableInfo.path;

   std::filesystem::path basePath;

   // Priority 1: altsound/<rom> (library adds /altsound/<rom> to basePath)
   if (auto path1 = find_case_insensitive_file_path(tablePath.parent_path() / "altsound"sv / currentGameId); !path1.empty())
      basePath = tablePath.parent_path();
   // Priority 2: pinmame/altsound/<rom>
   else if (auto path2 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altsound"sv / currentGameId); !path2.empty())
      basePath = tablePath.parent_path() / "pinmame"sv;
   // Priority 3: global setting
   else
   {
      std::filesystem::path altsoundFolder = altsoundFolderProp_Get();
      if (!altsoundFolder.empty())
         basePath = altsoundFolder.parent_path();
   }

   if (!basePath.empty())
   {
      std::filesystem::path altsoundGamePath = basePath / "altsound"sv / currentGameId;
      if (std::filesystem::exists(altsoundGamePath))
      {
         LOGI(std::format("Found altsound directory for game: {} at {}", currentGameId, altsoundGamePath.string()));
         PinMAMEMachineStateMsg state { };
         state.version = 1;
         state.hardwareGen = 0;
         msgApi->BroadcastMsg(endpointId, getMachineStateId, &state);
         StartAltSound(currentGameId, basePath.string(), state.hardwareGen);
      }
   }
}

}

using namespace AltSound;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle

MSGPI_EXPORT void MSGPIAPI AltSoundPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
    msgApi = api;
    endpointId = sessionId;
    audioSrcDef.id.endpointId = endpointId;

    LPISetup(endpointId, msgApi);

    msgApi->RegisterSetting(endpointId, &altsoundFolderProp);

    getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
    msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);

    onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);
    onAudioSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_SRC_CHG_MSG);
    getAudioSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_GET_SRC_MSG);
    msgApi->SubscribeMsg(endpointId, getAudioSrcId, OnGetAudioSrc, nullptr);

    onControllersChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_ON_CHG_MSG);
    getControllersId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG);
    msgApi->SubscribeMsg(endpointId, onControllersChangedId, OnControllersChanged, nullptr);
    OnControllersChanged(onControllersChangedId, nullptr, nullptr);
    
    msgApi->SubscribeMsg(endpointId, onAudioCmdId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_AUDIO_CMD), OnGameEvent, nullptr);
    getMachineStateId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_GET_MACHINE_STATE);
}

MSGPI_EXPORT void MSGPIAPI AltSoundPluginUnload()
{
    StopAltSound();

    msgApi->FlushPendingCallbacks(endpointId);
     
    msgApi->UnsubscribeMsg(onControllersChangedId, OnControllersChanged, nullptr);
    msgApi->ReleaseMsgID(onControllersChangedId);
    msgApi->ReleaseMsgID(getControllersId);
    
    msgApi->ReleaseMsgID(getMachineStateId);

    msgApi->UnsubscribeMsg(onAudioCmdId, OnGameEvent, nullptr);
    msgApi->ReleaseMsgID(onAudioUpdateId);
    msgApi->ReleaseMsgID(onAudioCmdId);
    
    msgApi->UnsubscribeMsg(getAudioSrcId, OnGetAudioSrc, nullptr);
    msgApi->ReleaseMsgID(onAudioSrcChangedId);
    msgApi->ReleaseMsgID(getAudioSrcId);

    msgApi->ReleaseMsgID(getVpxApiId);

    vpxApi = nullptr;
    msgApi = nullptr;
}
