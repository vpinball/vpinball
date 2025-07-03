// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <charconv>
#include <thread>
#include <mutex>

#include "VPXPlugin.h"
#include "ControllerPlugin.h"
#include "LoggingPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // xxx needs dll-interface
#include "DOF/DOF.h"
#pragma warning(pop)

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
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

static MsgPluginAPI* msgApi = nullptr;
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
static DevSrcId pinmameDevSrc = { 0 };
static InputSrcId pinmameInputSrc = { 0 };
static unsigned int nPmSolenoids = 0;
static int pmGiIndex = -1;
static unsigned int nPmGIs = 0;
static int pmLampIndex = -1;
static unsigned int nPmLamps = 0;

static std::thread pollThread;

static DOF::DOF* pDOF = nullptr;

static void OnPollStates(void* userData);

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

LPI_IMPLEMENT

void LIBDOFCALLBACK OnDOFLog(DOF_LogLevel logLevel, const char* format, va_list args)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      switch(logLevel) {
         case DOF_LogLevel_INFO:
            LOGI("%s", buffer);
            break;
         case DOF_LogLevel_DEBUG:
            LOGD("%s", buffer);
            break;
         case DOF_LogLevel_ERROR:
            LOGE("%s", buffer);
            break;
         default:
            break;
      }
      free(buffer);
   }
}

static string GetSettingString(MsgPluginAPI* pMsgApi, const char* section, const char* key, const string& def = string())
{
   char buf[256];
   pMsgApi->GetSetting(section, key, buf, sizeof(buf));
   return buf[0] ? string(buf) : def;
}

static int GetSettingInt(MsgPluginAPI* pMsgApi, const char* section, const char* key, int def = 0)
{
   const auto s = GetSettingString(pMsgApi, section, key, string());
   int result;
   return (s.empty() || (std::from_chars(s.c_str(), s.c_str() + s.length(), result).ec != std::errc{})) ? def : result;
}

static bool GetSettingBool(MsgPluginAPI* pMsgApi, const char* section, const char* key, bool def = false)
{
   const auto s = GetSettingString(pMsgApi, section, key, string());
   int result;
   return (s.empty() || (std::from_chars(s.c_str(), s.c_str() + s.length(), result).ec != std::errc{})) ? def : (result != 0);
}

#ifdef _WIN32
void SetThreadName(const std::string& name)
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
void SetThreadName(const std::string& name) { }
#endif

static void PollThread(const string& tablePath, const string& gameId)
{
   assert(pDOF != nullptr);
   SetThreadName("DOF.PollThread"s);
   pDOF->Init(tablePath.c_str(), gameId.c_str());
   bool isInitialState = true;
   vector<bool> wireStates, solStates, lampStates, giStates;
   while (isRunning)
   {
      {
         std::lock_guard<std::mutex> lock(sourceMutex);

         isInitialState |= wireStates.size() != pinmameInputSrc.nInputs;
         isInitialState |= solStates.size() != nPmSolenoids;
         isInitialState |= lampStates.size() != nPmLamps;
         isInitialState |= giStates.size() != nPmGIs;

         wireStates.resize(pinmameInputSrc.nInputs);
         for (unsigned int i = 0; i < pinmameInputSrc.nInputs; i++)
         {
            bool state = pinmameInputSrc.GetInputState(i);
            if (isInitialState || (wireStates[i] != state))
               pDOF->DataReceive('W', i + 1, state ? 1 : 0);
         }

         solStates.resize(nPmSolenoids);
         for (unsigned int i = 0; i < nPmSolenoids; i++)
         {
            float state = pinmameDevSrc.GetFloatState(i);
            if (isInitialState || (solStates[i] && state < 0.25f) || (!solStates[i] && state > 0.75f))
               pDOF->DataReceive('S', i + 1, state > 0.5f ? 1 : 0);
         }

         lampStates.resize(nPmLamps);
         for (unsigned int i = 0; i < nPmLamps; i++)
         {
            float state = pinmameDevSrc.GetFloatState(pmLampIndex + i);
            if (isInitialState || (lampStates[i] && state < 0.25f) || (!lampStates[i] && state > 0.75f))
               pDOF->DataReceive('L', i + 1, state > 0.5f ? 1 : 0);
         }

         giStates.resize(nPmGIs);
         for (unsigned int i = 0; i < nPmGIs; i++)
         {
            float state = pinmameDevSrc.GetFloatState(pmGiIndex + i);
            if (isInitialState || (giStates[i] && state < 0.25f) || (!giStates[i] && state > 0.75f))
               pDOF->DataReceive('G', i + 1, state > 0.5f ? 1 : 0);
         }

         isInitialState = false;
      }
      
      // Fixed update at 60 FPS
      std::this_thread::sleep_for(std::chrono::nanoseconds(16666));
   }
   pDOF->Finish();
}


static void OnControllerGameStart(const unsigned int eventId, void* userData, void* msgData)
{
   const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(msgData);
   assert(msg != nullptr && msg->gameId != nullptr);

   if (pollThread.joinable())
   {
      LOGE("DOFPlugin: Invalid state, game start happened while already running");
      isRunning = false;
      pollThread.join();
   }

   if (pDOF) {
      LOGI("DOFPlugin: OnControllerGameStart: gameId=%s", msg->gameId);
      isRunning = true;
      VPXTableInfo tableInfo;
      vpxApi->GetTableInfo(&tableInfo);
      pollThread = std::thread(PollThread, tableInfo.path, msg->gameId);
   }
}

static void OnControllerGameEnd(const unsigned int eventId, void* userData, void* msgData)
{
   if (pDOF) {
      LOGI("DOFPlugin: OnControllerGameEnd");
      isRunning = false;
      if (pollThread.joinable())
         pollThread.join();
   }
}

static void ClearDevices()
{
   delete[] pinmameDevSrc.deviceDefs;
   nPmSolenoids = 0;
   pmGiIndex = -1;
   nPmGIs = 0;
   pmLampIndex = -1;
   nPmLamps = 0;
   memset(&pinmameDevSrc, 0, sizeof(pinmameDevSrc));
}

static void OnDevSrcChanged(const unsigned int eventId, void* userData, void* msgData)
{
   std::lock_guard<std::mutex> lock(sourceMutex);
   ClearDevices();

   GetDevSrcMsg getSrcMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getDevSrcId, &getSrcMsg);
   if (getSrcMsg.count == 0)
   {
      LOGI("DOFPlugin: OnDevSrcChanged - No source");
      return;
   }

   getSrcMsg = { getSrcMsg.count, 0, new DevSrcId[getSrcMsg.count] };
   msgApi->BroadcastMsg(endpointId, getDevSrcId, &getSrcMsg);
   MsgEndpointInfo info;
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      memset(&info, 0, sizeof(info));
      msgApi->GetEndpointInfo(getSrcMsg.entries[i].id.endpointId, &info);
      if (info.id != nullptr && strcmp(info.id, "PinMAME") == 0)
      {
         pinmameDevSrc = getSrcMsg.entries[i];
         if (pinmameDevSrc.deviceDefs)
         {
            pinmameDevSrc.deviceDefs = new DeviceDef[pinmameDevSrc.nDevices];
            memcpy(pinmameDevSrc.deviceDefs, getSrcMsg.entries[i].deviceDefs, getSrcMsg.entries[i].nDevices * sizeof(DeviceDef));
         }
         break;
      }
   }
   delete[] getSrcMsg.entries;

   for (unsigned int i = 0; i < pinmameDevSrc.nDevices; i++)
   {
      if (pinmameDevSrc.deviceDefs[i].groupId == 0x0100)
      {
         if (pmGiIndex == -1)
            pmGiIndex = i;
         nPmGIs++;
      }
      else if (pinmameDevSrc.deviceDefs[i].groupId == 0x0200)
      {
         if (pmLampIndex == -1)
            pmLampIndex = i;
         nPmLamps++;
      }
      else if ((pmGiIndex == -1) && (pmLampIndex == -1))
         nPmSolenoids++;
   }

   LOGI("DOFPlugin: OnDevSrcChanged - Found %d PinMAME devices (Sol:%d, Lamps:%d, GI:%d)", 
        pinmameDevSrc.nDevices, nPmSolenoids, nPmLamps, nPmGIs);
}

static void OnInputSrcChanged(const unsigned int eventId, void* userData, void* msgData)
{
   std::lock_guard<std::mutex> lock(sourceMutex);
   delete[] pinmameInputSrc.inputDefs;
   memset(&pinmameInputSrc, 0, sizeof(pinmameInputSrc));

   GetInputSrcMsg getSrcMsg = { 1024, 0, new InputSrcId[1024] };
   msgApi->BroadcastMsg(endpointId, getInputSrcId, &getSrcMsg);

   MsgEndpointInfo info;
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      memset(&info, 0, sizeof(info));
      msgApi->GetEndpointInfo(getSrcMsg.entries[i].id.endpointId, &info);
      if (info.id != nullptr && strcmp(info.id, "PinMAME") == 0)
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

   LOGI("DOFPlugin: OnInputSrcChanged - Found %d PinMAME inputs", pinmameInputSrc.nInputs);
}

}

using namespace DOFPlugin;

MSGPI_EXPORT void MSGPIAPI DOFPluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
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
