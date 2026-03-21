// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <charconv>
#include <thread>
#include <mutex>
#include <format>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif

#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // xxx needs dll-interface
#include "DOF/DOF.h"
#pragma warning(pop)

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <locale>
#endif

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// Direct Output Framework plugin
//
// TODO the polling system needs to be extended to also listen for B2S controller

namespace DOFPlugin {

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;

static unsigned int onControllerGameStartId;
static unsigned int onControllerGameEndId;

static unsigned int getDevSrcId;
static unsigned int onDevSrcChangedId;
static unsigned int getInputSrcId;
static unsigned int onInputSrcChangedId;

static std::mutex sourceMutex;
static bool isRunning = false;
static DevSrcId pinmameDevSrc = {};
static InputSrcId pinmameInputSrc = {};
static DevSrcId b2sDevSrc = {};

static std::thread pollThread;

static DOF::DOF* pDOF = nullptr;

static void OnPollStates(void* userData);

LPI_USE_CPP();
#define LOGD DOFPlugin::LPI_LOGD_CPP
#define LOGI DOFPlugin::LPI_LOGI_CPP
#define LOGW DOFPlugin::LPI_LOGW_CPP
#define LOGE DOFPlugin::LPI_LOGE_CPP

LPI_IMPLEMENT_CPP // Implement shared log support

void LIBDOFCALLBACK OnDOFLog(DOF_LogLevel logLevel, const char* format, va_list args)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      string buffer(size + 1, '\0');
      vsnprintf(buffer.data(), size + 1, format, args);
      buffer.pop_back(); // remove null terminator
      switch(logLevel) {
         case DOF_LogLevel_INFO:
            LOGI(buffer);
            break;
         case DOF_LogLevel_DEBUG:
            LOGD(buffer);
            break;
         case DOF_LogLevel_ERROR:
            LOGE(buffer);
            break;
         default:
            break;
      }
   }
}

#ifdef _WIN32
static void SetThreadName(const std::string& name)
{
   const int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
   if (size_needed <= 1)
      return;
   std::wstring wstr(size_needed - 1, L'\0');
   if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
      return;
   HRESULT hr = SetThreadDescription(GetCurrentThread(), wstr.c_str());
}
#else
static void SetThreadName(const std::string& name)
{
#ifdef __APPLE__
   pthread_setname_np(name.c_str());
#elif defined(__linux__) || defined(__ANDROID__)
   pthread_setname_np(pthread_self(), name.c_str());
#endif
}
#endif

static void PollThread(const string& tablePath, const string& gameId)
{
   assert(pDOF != nullptr);
   SetThreadName("DOF.PollThread"s);
   pDOF->Init(tablePath.c_str(), gameId.c_str());
   bool isInitialState = true;
   vector<bool> wireStates;
   vector<bool> pinmameDeviceStates;
   while (isRunning)
   {
      {
         std::lock_guard lock(sourceMutex);

         isInitialState |= wireStates.size() != pinmameInputSrc.nInputs;
         isInitialState |= pinmameDeviceStates.size() != pinmameDevSrc.nDevices;

         wireStates.resize(pinmameInputSrc.nInputs);
         for (unsigned int i = 0; i < pinmameInputSrc.nInputs; i++)
         {
            if (pinmameInputSrc.inputDefs[i].id.groupId == 0x0001)
            {
               bool state = pinmameInputSrc.GetInputState(i);
               if (isInitialState || (wireStates[i] != state))
               {
                  pDOF->DataReceive('W', pinmameInputSrc.inputDefs[i].id.deviceId, state ? 1 : 0); // Switches
                  wireStates[i] = state;
               }
            }
         }

         pinmameDeviceStates.resize(pinmameDevSrc.nDevices);
         for (unsigned int i = 0; i < pinmameDevSrc.nDevices; i++)
         {
            char type;
            switch (pinmameDevSrc.deviceDefs[i].id.groupId & 0xFF00)
            {
            case 0x0000: type = 'S'; break; // Solenoids
            case 0x0100: type = 'G'; break; // GI
            case 0x0200: type = 'L'; break; // Lamps
            default: type = '\0'; break; // Unsupported
            }
            if (type != '\0')
            {
               float state = pinmameDevSrc.GetFloatState(i);
               if (isInitialState || (pinmameDeviceStates[i] && state < 0.25f) || (!pinmameDeviceStates[i] && state > 0.75f))
               {
                  pDOF->DataReceive(type, pinmameDevSrc.deviceDefs[i].id.deviceId, state > 0.5f ? 1 : 0);
                  pinmameDeviceStates[i] = state > 0.5f;
               }
            }
         }

         isInitialState = false;
      }
      
      // Fixed update at 60 FPS
      std::this_thread::sleep_for(std::chrono::microseconds(16666));
   }
   pDOF->Finish();
}

static void MSGPIAPI OnB2SStateChg(unsigned int index, void* context)
{
   std::lock_guard lock(sourceMutex);
   if (index < b2sDevSrc.nDevices && pDOF != nullptr)
   {
      float state = b2sDevSrc.GetFloatState(index);
      // LOGD(std::format("DOFPlugin: B2S state change E{:d} = {:f}", b2sDevSrc.deviceDefs[index].id.deviceId, state));
      pDOF->DataReceive('E', b2sDevSrc.deviceDefs[index].id.deviceId, state > 0.5f ? 1 : 0);
   }
}

static void OnControllerGameStart(const unsigned int eventId, void* userData, void* msgData)
{
   const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(msgData);
   assert(msg != nullptr && msg->gameId != nullptr);

   if (pollThread.joinable())
   {
      LOGE("DOFPlugin: Invalid state, game start happened while already running"s);
      isRunning = false;
      pollThread.join();
   }

   if (pDOF) {
      LOGI("DOFPlugin: OnControllerGameStart: gameId="s + msg->gameId);
      isRunning = true;
      VPXTableInfo tableInfo;
      vpxApi->GetTableInfo(&tableInfo);
      string path = tableInfo.path;
      string gameId = msg->gameId;
      pollThread = std::thread(PollThread, path, gameId);
   }
}

static void OnControllerGameEnd(const unsigned int eventId, void* userData, void* msgData)
{
   if (pDOF) {
      LOGI("DOFPlugin: OnControllerGameEnd"s);
      isRunning = false;
      if (pollThread.joinable())
         pollThread.join();
   }
}

static void ClearDevices()
{
   delete[] pinmameDevSrc.deviceDefs;
   memset(&pinmameDevSrc, 0, sizeof(pinmameDevSrc));
   for (unsigned int i = 0; i < b2sDevSrc.nDevices; i++)
      if (b2sDevSrc.SetChangeCallback)
         b2sDevSrc.SetChangeCallback(i, 0, OnB2SStateChg, nullptr);
   memset(&b2sDevSrc, 0, sizeof(b2sDevSrc));
}

static void OnDevSrcChanged(const unsigned int eventId, void* userData, void* msgData)
{
   std::lock_guard lock(sourceMutex);
   ClearDevices();

   GetDevSrcMsg getSrcMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getDevSrcId, &getSrcMsg);
   if (getSrcMsg.count == 0)
   {
      LOGI("DOFPlugin: OnDevSrcChanged - No source"s);
      return;
   }

   getSrcMsg = { getSrcMsg.count, 0, new DevSrcId[getSrcMsg.count] };
   msgApi->BroadcastMsg(endpointId, getDevSrcId, &getSrcMsg);
   MsgEndpointInfo info;
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      memset(&info, 0, sizeof(info));
      msgApi->GetEndpointInfo(getSrcMsg.entries[i].id.endpointId, &info);
      if (info.id != nullptr && info.id == "PinMAME"s)
      {
         pinmameDevSrc = getSrcMsg.entries[i];
         if (pinmameDevSrc.deviceDefs)
         {
            pinmameDevSrc.deviceDefs = new DeviceDef[pinmameDevSrc.nDevices];
            memcpy(pinmameDevSrc.deviceDefs, getSrcMsg.entries[i].deviceDefs, getSrcMsg.entries[i].nDevices * sizeof(DeviceDef));
         }
      }
      else if (info.id != nullptr && (info.id == "B2S"s || info.id == "B2SLegacy"s))
      {
         b2sDevSrc = getSrcMsg.entries[i];
         for (unsigned int i = 0; i < b2sDevSrc.nDevices; i++)
            if (b2sDevSrc.SetChangeCallback)
               b2sDevSrc.SetChangeCallback(i, 1, OnB2SStateChg, nullptr);
      }
   }
   delete[] getSrcMsg.entries;

   LOGI(std::format("DOFPlugin: OnDevSrcChanged - Found {} PinMAME devices and {} B2S devices", pinmameDevSrc.nDevices, b2sDevSrc.nDevices));
}

static void OnInputSrcChanged(const unsigned int eventId, void* userData, void* msgData)
{
   std::lock_guard lock(sourceMutex);
   delete[] pinmameInputSrc.inputDefs;
   memset(&pinmameInputSrc, 0, sizeof(pinmameInputSrc));

   GetInputSrcMsg getSrcMsg = { 1024, 0, new InputSrcId[1024] };
   msgApi->BroadcastMsg(endpointId, getInputSrcId, &getSrcMsg);

   MsgEndpointInfo info;
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      memset(&info, 0, sizeof(info));
      msgApi->GetEndpointInfo(getSrcMsg.entries[i].id.endpointId, &info);
      if (info.id != nullptr && info.id == "PinMAME"s)
      {
         pinmameInputSrc = getSrcMsg.entries[i];
         if (pinmameInputSrc.inputDefs)
         {
            pinmameInputSrc.inputDefs = new DeviceDef[pinmameInputSrc.nInputs];
            memcpy(pinmameInputSrc.inputDefs, getSrcMsg.entries[i].inputDefs, getSrcMsg.entries[i].nInputs * sizeof(DeviceDef));
         }
         break;
      }
   }
   delete[] getSrcMsg.entries;

   LOGI(std::format("DOFPlugin: OnInputSrcChanged - Found {} PinMAME inputs", pinmameInputSrc.nInputs));
}

}

using namespace DOFPlugin;

MSGPI_EXPORT void MSGPIAPI DOFPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi);

   memset(&pinmameDevSrc, 0, sizeof(pinmameDevSrc));
   ClearDevices();

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   msgApi->SubscribeMsg(endpointId, onControllerGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), OnControllerGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onControllerGameEndId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_END), OnControllerGameEnd, nullptr);

   getDevSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_GET_SRC_MSG);
   onDevSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DEVICE_ON_SRC_CHG_MSG);
   getInputSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_GET_SRC_MSG);
   onInputSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_INPUT_ON_SRC_CHG_MSG);

   msgApi->SubscribeMsg(endpointId, onDevSrcChangedId, OnDevSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, onInputSrcChangedId, OnInputSrcChanged, nullptr);

   OnDevSrcChanged(onDevSrcChangedId, nullptr, nullptr);
   OnInputSrcChanged(onInputSrcChangedId, nullptr, nullptr);

   VPXInfo vpxInfo;
   vpxApi->GetVpxInfo(&vpxInfo);

   DOF::Config* pConfig = DOF::Config::GetInstance();
   pConfig->SetLogCallback(OnDOFLog);
   pConfig->SetBasePath(vpxInfo.prefPath);

   pDOF = new DOF::DOF();
}

MSGPI_EXPORT void MSGPIAPI DOFPluginUnload()
{
   isRunning = false;
   if (pollThread.joinable())
      pollThread.join();

   ClearDevices();
   delete[] pinmameInputSrc.inputDefs;
   memset(&pinmameInputSrc, 0, sizeof(pinmameInputSrc));

   delete pDOF;
   pDOF = nullptr;

   msgApi->UnsubscribeMsg(onControllerGameStartId, OnControllerGameStart);
   msgApi->UnsubscribeMsg(onControllerGameEndId, OnControllerGameEnd);
   msgApi->UnsubscribeMsg(onDevSrcChangedId, OnDevSrcChanged);
   msgApi->UnsubscribeMsg(onInputSrcChangedId, OnInputSrcChanged);

   msgApi->ReleaseMsgID(onControllerGameStartId);
   msgApi->ReleaseMsgID(onControllerGameEndId);
   msgApi->ReleaseMsgID(getDevSrcId);
   msgApi->ReleaseMsgID(onDevSrcChangedId);
   msgApi->ReleaseMsgID(getInputSrcId);
   msgApi->ReleaseMsgID(onInputSrcChangedId);

   msgApi = nullptr;
}
