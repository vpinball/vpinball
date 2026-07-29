// license:GPLv3+

#include "plugins/VPXPlugin.h"
#include "plugins/B2SPluginEventStream.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"
#include "pinmame/PinMAMEPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // xxx needs dll-interface
#include "DOF/DOF.h"
#pragma warning(pop)

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <charconv>
#include <format>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <locale>
#endif

using namespace std;

///////////////////////////////////////////////////////////////////////////////
//
// Direct Output Framework plugin
//

namespace DOFPlugin {

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;

static unsigned int onControllersChangedId;
static unsigned int getControllersId;
static string currentGameId;

static std::unique_ptr<B2SPluginEventStream> b2sPluginEventStream;

static std::unique_ptr<DOF::DOF> pDOF;

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

class DOFEventConsumer
{
public:
   DOFEventConsumer(const string& tablePath, const string& gameId)
      : m_tablePath(tablePath)
      , m_gameId(gameId)
   {
      m_thread = std::thread(&DOFEventConsumer::Run, this);
   }

   ~DOFEventConsumer()
   {
      {
         std::lock_guard lock(m_mutex);
         m_stopRequested = true;
      }
      m_cv.notify_one();

      if (m_thread.joinable())
         m_thread.join();
   }

   // Non-copyable, non-movable (owns a thread + sync primitives)
   DOFEventConsumer(const DOFEventConsumer&) = delete;
   DOFEventConsumer& operator=(const DOFEventConsumer&) = delete;

   struct B2SPluginEvent
   {
      uint8_t type;
      int32_t index;
      int32_t value;
   };

   void PostEvent(const B2SPluginEvent& ev)
   {
      {
         std::lock_guard<std::mutex> lock(m_mutex);
         m_queue.push(ev);
      }
      m_cv.notify_one();
   }

private:
   void Run()
   {
      SetThreadName("DOF.EventQueue"s);
      pDOF->Init(m_tablePath.c_str(), m_gameId.c_str());
      while (!m_stopRequested)
      {
         std::unique_lock lock(m_mutex);

         m_cv.wait(lock, [this] { return !m_queue.empty() || m_stopRequested; });

         if (m_stopRequested)
            break;

         while (!m_queue.empty())
         {
            B2SPluginEvent ev = m_queue.front();
            m_queue.pop();
            lock.unlock();
            pDOF->DataReceive(ev.type, ev.index, ev.value);
            lock.lock();
         }
      }
      pDOF->Finish();
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

   const string m_tablePath;
   const string m_gameId;

   std::queue<B2SPluginEvent> m_queue;
   std::mutex m_mutex;
   std::condition_variable m_cv;
   bool m_stopRequested = false;
   std::thread m_thread;
};

static std::unique_ptr<DOFEventConsumer> dofThread;

static void OnControllersChanged(const unsigned int eventId, void* userData, void* msgData)
{
   // Enumerate and select the first controller exposing a PinMAME compatible game
   string selectedGameId;
   GetControllersMsg getControllersMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getControllersId, &getControllersMsg);
   if (getControllersMsg.count > 0)
   {
      const string pinmamePrefix(PMPI_GAMEID_PREFIX);
      vector<ControllerDef> controllers(getControllersMsg.count);
      getControllersMsg = { getControllersMsg.count, 0, controllers.data() };
      msgApi->BroadcastMsg(endpointId, getControllersId, &getControllersMsg);
      for (const auto& controller : controllers)
      {
         string gameId = controller.gameId;
         if (gameId.starts_with(pinmamePrefix))
         {
            selectedGameId = gameId.substr(pinmamePrefix.length());
            if (!selectedGameId.empty())
               break;
         }
      }
   }
   if (currentGameId == selectedGameId)
      return;

   // Setup on the selected game if any
   currentGameId = selectedGameId;
   dofThread = nullptr;
   if (!currentGameId.empty() && pDOF) {
      LOGI("New PinMAME game started: gameId="s + currentGameId);
      VPXTableInfo tableInfo;
      vpxApi->GetTableInfo(&tableInfo);
      string path = tableInfo.path;
      dofThread = std::make_unique<DOFEventConsumer>(path, currentGameId);
   }
}

}

using namespace DOFPlugin;

MSGPI_EXPORT void MSGPIAPI DOFPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   b2sPluginEventStream = std::make_unique<B2SPluginEventStream>(msgApi, endpointId, [](char type, int index, int value) { dofThread->PostEvent({ static_cast<uint8_t>(type), index, value }); });

   VPXInfo vpxInfo;
   vpxApi->GetVpxInfo(&vpxInfo);

   DOF::Config* pConfig = DOF::Config::GetInstance();
   pConfig->SetLogCallback(OnDOFLog);
   pConfig->SetBasePath(vpxInfo.prefPath);

   pDOF = std::make_unique<DOF::DOF>();

   onControllersChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_ON_CHG_MSG);
   getControllersId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG);
   msgApi->SubscribeMsg(endpointId, onControllersChangedId, OnControllersChanged, nullptr);
   OnControllersChanged(onControllersChangedId, nullptr, nullptr);
}

MSGPI_EXPORT void MSGPIAPI DOFPluginUnload()
{
   dofThread = nullptr;
   pDOF = nullptr;
   b2sPluginEventStream = nullptr;
   currentGameId.clear();

   msgApi->UnsubscribeMsg(onControllersChangedId, OnControllersChanged, nullptr);
   msgApi->ReleaseMsgID(onControllersChangedId);
   msgApi->ReleaseMsgID(getControllersId);

   msgApi = nullptr;
   vpxApi = nullptr;
}
