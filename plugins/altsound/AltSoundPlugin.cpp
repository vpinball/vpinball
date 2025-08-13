// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>
#include <filesystem>

#include "common.h"
#include <altsound.h>

using namespace std::string_literals;

#ifndef _MSC_VER
 #define strcpy_s(A, B, C) strncpy(A, C, B)
#endif

namespace AltSound {

LPI_IMPLEMENT

MSGPI_STRING_VAL_SETTING(altsoundFolderProp, "Folder", "AltSound Folder", "", true, "", 1024);

static MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;

static uint32_t endpointId;
static unsigned int onControllerGameStartId;
static unsigned int onControllerGameEndId;
static unsigned int onAudioUpdateId = 0;
static unsigned int onSoundCommandId = 0;

static bool isRunning = false;
static string currentGameId;

static CtlResId audioResId;
static uint32_t nextAudioResId = 1;

struct AudioCallbackData {
    MsgPluginAPI* msgApi;
    uint32_t endpointId;
    unsigned int onAudioUpdateId;
    AudioUpdateMsg* msg;
};

static void AudioCallback(const float* samples, size_t frameCount, uint32_t sampleRate, uint32_t channels, void* userData)
{
    if (!msgApi || onAudioUpdateId == 0 || !samples || frameCount == 0)
        return;

    const size_t bufferSizeBytes = frameCount * channels * sizeof(float);

    AudioUpdateMsg* pAudioUpdateMsg = new AudioUpdateMsg();
    pAudioUpdateMsg->id = audioResId;
    pAudioUpdateMsg->type = (channels == 1) ? CTLPI_AUDIO_SRC_BACKGLASS_MONO : CTLPI_AUDIO_SRC_BACKGLASS_STEREO;
    pAudioUpdateMsg->format = CTLPI_AUDIO_FORMAT_SAMPLE_FLOAT;
    pAudioUpdateMsg->sampleRate = sampleRate;
    pAudioUpdateMsg->volume = 1.0f;
    pAudioUpdateMsg->bufferSize = static_cast<unsigned int>(bufferSizeBytes);
    pAudioUpdateMsg->buffer = new uint8_t[bufferSizeBytes];

    memcpy(pAudioUpdateMsg->buffer, samples, bufferSizeBytes);

    AudioCallbackData* cbData = new AudioCallbackData{msgApi, endpointId, onAudioUpdateId, pAudioUpdateMsg};

    msgApi->RunOnMainThread(0, [](void* userData) {
        AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
        data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
        delete[] data->msg->buffer;
        delete data->msg;
        delete data;
    }, cbData);
}

static void StartAltSound(const string& gameId, const string& vpmPath, uint64_t hardwareGen)
{
    if (isRunning) {
        AltSoundShutdown();
        isRunning = false;
    }

    VPXInfo vpxInfo;
    vpxApi->GetVpxInfo(&vpxInfo);
    AltSoundSetLogger(vpxInfo.prefPath, ALTSOUND_LOG_LEVEL_DEBUG, true);

    LOGI("Initializing AltSound for game: %s, vpmPath: %s", gameId.c_str(), vpmPath.c_str());
        
    if (AltSoundInit(vpmPath, gameId, 44100, 2, 128)) {
        AltSoundSetAudioCallback(AudioCallback, nullptr);        
        AltSoundSetHardwareGen(static_cast<ALTSOUND_HARDWARE_GEN>(hardwareGen));
        
        currentGameId = gameId;
        isRunning = true;
        
        LOGI("AltSound initialized successfully for game: %s", gameId.c_str());
    } else {
        LOGE("Failed to initialize AltSound for game: %s", gameId.c_str());
    }
}

static void StopAltSound()
{
    if (!isRunning)
       return;

    AltSoundShutdown();
    isRunning = false;
    currentGameId.clear();
        
    AudioUpdateMsg* pAudioUpdateMsg = new AudioUpdateMsg();
    pAudioUpdateMsg->id = audioResId;
    pAudioUpdateMsg->buffer = nullptr;
    pAudioUpdateMsg->bufferSize = 0;

    AudioCallbackData* cbData = new AudioCallbackData{msgApi, endpointId, onAudioUpdateId, pAudioUpdateMsg};

    msgApi->RunOnMainThread(0, [](void* userData) {
        AudioCallbackData* data = static_cast<AudioCallbackData*>(userData);
        data->msgApi->BroadcastMsg(data->endpointId, data->onAudioUpdateId, data->msg);
        delete data->msg;
        delete data;
    }, cbData);
}

static void OnControllerGameStart(const unsigned int eventId, void* userData, void* msgData)
{
    const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(msgData);
    assert(msg != nullptr && msg->gameId != nullptr);

    string altsoundFolder = altsoundFolderProp_Get();

    if (altsoundFolder.empty()) {
        VPXTableInfo tableInfo;
        vpxApi->GetTableInfo(&tableInfo);
        std::filesystem::path tablePath = tableInfo.path;
        altsoundFolder = find_case_insensitive_file_path(tablePath.parent_path().string() + PATH_SEPARATOR_CHAR + "pinmame"s + PATH_SEPARATOR_CHAR + "altsound"s);
    }

    if (!altsoundFolder.empty()) {
        std::filesystem::path altsoundPath = altsoundFolder;
        string vpmPath = altsoundPath.parent_path().string();
        string altsoundGamePath = altsoundFolder + PATH_SEPARATOR_CHAR + msg->gameId;
        if (std::filesystem::exists(altsoundGamePath)) {
            LOGI("Found altsound directory for game: %s at %s", msg->gameId, altsoundGamePath.c_str());
            StartAltSound(msg->gameId, vpmPath, 0x4000);
        } else {
            LOGI("No altsound directory found for game: %s (checked: %s)", msg->gameId, altsoundGamePath.c_str());
        }
    } else {
        LOGI("No altsound base directory found for game: %s", msg->gameId);
    }
}

static void OnSoundCommand(const unsigned int eventId, void* userData, void* msgData)
{
    if (!isRunning)
        return;

    const CtlOnSoundCommandMsg* msg = static_cast<const CtlOnSoundCommandMsg*>(msgData);
    if (msg != nullptr)
        AltSoundProcessCommand(msg->cmd, 0);
}

static void OnControllerGameEnd(const unsigned int eventId, void* userData, void* msgData)
{
    StopAltSound();
}

}

using namespace AltSound;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Plugin lifecycle

MSGPI_EXPORT void MSGPIAPI AltSoundPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
    msgApi = const_cast<MsgPluginAPI*>(api);
    endpointId = sessionId;

    audioResId.endpointId = endpointId;
    audioResId.resId = nextAudioResId++;

    LPISetup(endpointId, msgApi);

    msgApi->RegisterSetting(endpointId, &altsoundFolderProp);

    unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
    msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
    msgApi->ReleaseMsgID(getVpxApiId);

    onAudioUpdateId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_AUDIO_ON_UPDATE_MSG);

    msgApi->SubscribeMsg(endpointId, onControllerGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), OnControllerGameStart, nullptr);
    msgApi->SubscribeMsg(endpointId, onControllerGameEndId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_END), OnControllerGameEnd, nullptr);
    msgApi->SubscribeMsg(endpointId, onSoundCommandId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_SOUND_COMMAND), OnSoundCommand, nullptr);

    LOGI("AltSound Plugin loaded successfully");
}

MSGPI_EXPORT void MSGPIAPI AltSoundPluginUnload()
{
    StopAltSound();

    if (msgApi) {
        if (onControllerGameStartId != 0) {
            msgApi->UnsubscribeMsg(endpointId, OnControllerGameStart);
            msgApi->ReleaseMsgID(onControllerGameStartId);
            onControllerGameStartId = 0;
        }
        if (onSoundCommandId != 0) {
            msgApi->UnsubscribeMsg(endpointId, OnSoundCommand);
            msgApi->ReleaseMsgID(onSoundCommandId);
            onSoundCommandId = 0;
        }
        if (onControllerGameEndId != 0) {
            msgApi->UnsubscribeMsg(endpointId, OnControllerGameEnd);
            msgApi->ReleaseMsgID(onControllerGameEndId);
            onControllerGameEndId = 0;
        }
        if (onAudioUpdateId != 0) {
            msgApi->ReleaseMsgID(onAudioUpdateId);
            onAudioUpdateId = 0;
        }
    }

    vpxApi = nullptr;
    msgApi = nullptr;

    LOGI("AltSound Plugin unloaded");
}