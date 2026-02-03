// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstring>
#include <mutex>
#include <thread>

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "common.h"
#include "vni.h"

#include <filesystem>

#include "plugins/LoggingPlugin.h"

using namespace std::string_literals;

namespace Vni {

LPI_IMPLEMENT

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;

static uint32_t endpointId;
static unsigned int onControllerGameStartId, onControllerGameEndId;
static unsigned int onDmdSrcChangedId, getDmdSrcId;

static bool isRunning = false;
static std::mutex sourceMutex;
static std::mutex stateMutex;
static std::thread colorizeThread;
static DisplaySrcId dmdId = {};

static Vni_Context* pVni = nullptr;

MSGPI_STRING_VAL_SETTING(vniPathProp, "VniPath", "VNI Path", "Folder that contains VNI colorization files (PAL, VNI)", true, "", 1024);

class ColorizationState final
{
public:
   ColorizationState(unsigned int width, unsigned int height)
      : m_colorFrame(new uint8_t[width * height * 3])
      , m_width(width), m_height(height)
      , m_colorizedframeId(static_cast<unsigned int>(std::rand()))
   {
      assert(m_width > 0);
      assert(m_height > 0);
   }

   ~ColorizationState()
   {
      delete[] m_colorFrame;
   }

   void UpdateFrame(const Vni_Frame_Struc* frame)
   {
      if (!frame || !frame->has_frame || !frame->frame || !frame->palette)
         return;
      for (unsigned int i = 0; i < m_width * m_height; i++)
         memcpy(&(m_colorFrame[i * 3]), &frame->palette[frame->frame[i] * 3], 3);
      m_colorizedframeId++;
   }

   uint8_t* const m_colorFrame;
   const unsigned int m_width, m_height;
   unsigned int m_colorizedframeId = 0;
};

static ColorizationState* state = nullptr;

static void ColorizeThread()
{
   SetThreadName("Vni.ColorizeThread"s);
   unsigned int lastFrameId = 0;
   while (isRunning)
   {
      std::this_thread::sleep_for(std::chrono::microseconds(16666));

      std::lock_guard<std::mutex> lock1(sourceMutex);
      if (dmdId.id.id == 0)
         continue;

      const DisplayFrame frame = dmdId.GetIdentifyFrame(dmdId.id);
      if (frame.frame == nullptr)
         break;

      if (frame.frameId != lastFrameId)
      {
         lastFrameId = frame.frameId;
         const uint32_t result = Vni_Colorize(pVni, static_cast<const uint8_t*>(frame.frame), dmdId.width, dmdId.height, 2);
         if (result)
         {
            const Vni_Frame_Struc* vniFrame = Vni_GetFrame(pVni);
            if (vniFrame && vniFrame->has_frame)
            {
               std::lock_guard<std::mutex> lock2(stateMutex);
               bool newState = false;
               if (state == nullptr)
               {
                  state = new ColorizationState(dmdId.width, dmdId.height);
                  newState = true;
               }
               else if (state->m_width != dmdId.width || state->m_height != dmdId.height)
               {
                  delete state;
                  state = new ColorizationState(dmdId.width, dmdId.height);
                  newState = true;
               }

               state->UpdateFrame(vniFrame);

               if (newState)
                  msgApi->RunOnMainThread(endpointId, 0, [](void* userData) { msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr); }, nullptr);
            }
         }
      }
   }
   isRunning = false;
}

static DisplayFrame GetRenderFrame(const CtlResId id)
{
   std::lock_guard<std::mutex> lock(stateMutex);
   if (state == nullptr)
      return { 0, nullptr };
   return { state->m_colorizedframeId, state->m_colorFrame };
}

static void OnGetRenderDMDSrc(const unsigned int eventId, void* userData, void* msgData)
{
   if (pVni == nullptr || state == nullptr || dmdId.id.id == 0)
      return;
   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);
   if (state->m_colorFrame && state->m_width && state->m_height)
   {
      if (msg.count < msg.maxEntryCount)
      {
         msg.entries[msg.count] = {};
         msg.entries[msg.count].id = { { endpointId, 0 } };
         msg.entries[msg.count].overrideId = dmdId.id;
         msg.entries[msg.count].width = state->m_width;
         msg.entries[msg.count].height = state->m_height;
         msg.entries[msg.count].hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED;
         msg.entries[msg.count].frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888;
         msg.entries[msg.count].GetRenderFrame = &GetRenderFrame;
      }
      msg.count++;
   }
}

static void OnDmdSrcChanged(const unsigned int, void*, void*)
{
   if (pVni == nullptr)
      return;
   std::lock_guard<std::mutex> lock(sourceMutex);
   dmdId.id.id = 0;
   GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getDmdSrcId, &getSrcMsg);
   if (getSrcMsg.count == 0)
      return;
   getSrcMsg = { getSrcMsg.count, 0, new DisplaySrcId[getSrcMsg.count] };
   msgApi->BroadcastMsg(endpointId, getDmdSrcId, &getSrcMsg);
   for (unsigned int i = 0; i < getSrcMsg.count; i++)
   {
      if (getSrcMsg.entries[i].GetIdentifyFrame != nullptr && getSrcMsg.entries[i].width >= 128)
      {
         dmdId = getSrcMsg.entries[i];
         break;
      }
   }
   delete[] getSrcMsg.entries;
}

static void StopColorization()
{
   isRunning = false;
   if (colorizeThread.joinable())
      colorizeThread.join();
   if (pVni)
   {
      delete state;
      state = nullptr;
      Vni_Dispose(pVni);
      pVni = nullptr;
      msgApi->BroadcastMsg(endpointId, onDmdSrcChangedId, nullptr);
   }
   dmdId.id.id = 0;
}

static void OnControllerGameStart(const unsigned int eventId, void* userData, void* msgData)
{
   StopColorization();
   const CtlOnGameStartMsg* msg = static_cast<const CtlOnGameStartMsg*>(msgData);
   assert(msg != nullptr && msg->gameId != nullptr);

   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);
   std::filesystem::path tablePath = tableInfo.path;

   std::filesystem::path vniBasePath = vniPathProp_Get();
   const std::string gameId = msg->gameId;
   const std::filesystem::path palFile = gameId + ".pal"s;
   const std::filesystem::path vniFile = gameId + ".vni"s;
   const std::filesystem::path pin2dmdPal = "pin2dmd.pal"s;
   const std::filesystem::path pin2dmdVni = "pin2dmd.vni"s;

   std::filesystem::path palPath, vniPath;

   // Priority 1: vni/<rom>/<rom>.pal and vni/<rom>/<rom>.vni
   if (auto path1 = find_case_insensitive_file_path(tablePath.parent_path() / "vni" / gameId / palFile); !path1.empty())
   {
      palPath = path1;
      if (auto path2 = find_case_insensitive_file_path(tablePath.parent_path() / "vni" / gameId / vniFile); !path2.empty())
         vniPath = path2;
   }
   // Priority 2: vni/<rom>/pin2dmd.pal and vni/<rom>/pin2dmd.vni
   else if (auto path3 = find_case_insensitive_file_path(tablePath.parent_path() / "vni" / gameId / pin2dmdPal); !path3.empty())
   {
      palPath = path3;
      if (auto path4 = find_case_insensitive_file_path(tablePath.parent_path() / "vni" / gameId / pin2dmdVni); !path4.empty())
         vniPath = path4;
   }
   // Priority 3: pinmame/altcolor/<rom>/<rom>.pal and pinmame/altcolor/<rom>/<rom>.vni
   else if (auto path5 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame" / "altcolor" / gameId / palFile); !path5.empty())
   {
      palPath = path5;
      if (auto path6 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame" / "altcolor" / gameId / vniFile); !path6.empty())
         vniPath = path6;
   }
   // Priority 4: pinmame/altcolor/<rom>/pin2dmd.pal and pinmame/altcolor/<rom>/pin2dmd.vni
   else if (auto path7 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame" / "altcolor" / gameId / pin2dmdPal); !path7.empty())
   {
      palPath = path7;
      if (auto path8 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame" / "altcolor" / gameId / pin2dmdVni); !path8.empty())
         vniPath = path8;
   }
   // Priority 5: global setting path
   else if (!vniBasePath.empty())
   {
      if (auto path9 = find_case_insensitive_file_path(vniBasePath / gameId / palFile); !path9.empty())
      {
         palPath = path9;
         if (auto path10 = find_case_insensitive_file_path(vniBasePath / gameId / vniFile); !path10.empty())
            vniPath = path10;
      }
   }

   if (palPath.empty())
   {
      LOGI("VNI: No PAL file found for %s", gameId.c_str());
      return;
   }

   LOGI("VNI: Loading PAL from %s", palPath.string().c_str());
   if (!vniPath.empty())
      LOGI("VNI: Loading VNI from %s", vniPath.string().c_str());

   pVni = Vni_LoadFromPaths(palPath.string().c_str(), vniPath.empty() ? nullptr : vniPath.string().c_str(), nullptr, nullptr);
   OnDmdSrcChanged(onDmdSrcChangedId, nullptr, nullptr);
   if (pVni)
   {
      isRunning = true;
      colorizeThread = std::thread(ColorizeThread);
   }
   else
   {
      LOGE("VNI: Failed to load colorization data");
   }
}

static void OnControllerGameEnd(const unsigned int eventId, void* userData, void* msgData)
{
   StopColorization();
}

}

using namespace Vni;

MSGPI_EXPORT void MSGPIAPI VNIPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi);

   msgApi->RegisterSetting(endpointId, &vniPathProp);

   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   msgApi->SubscribeMsg(endpointId, onControllerGameStartId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_START), OnControllerGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onControllerGameEndId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_EVT_ON_GAME_END), OnControllerGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), OnDmdSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, getDmdSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG), OnGetRenderDMDSrc, nullptr);
}

MSGPI_EXPORT void MSGPIAPI VNIPluginUnload()
{
   StopColorization();
   msgApi->UnsubscribeMsg(getDmdSrcId, OnGetRenderDMDSrc);
   msgApi->UnsubscribeMsg(onDmdSrcChangedId, OnDmdSrcChanged);
   msgApi->UnsubscribeMsg(onControllerGameStartId, OnControllerGameStart);
   msgApi->UnsubscribeMsg(onControllerGameEndId, OnControllerGameEnd);
   msgApi->ReleaseMsgID(onControllerGameStartId);
   msgApi->ReleaseMsgID(onControllerGameEndId);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcId);
   msgApi->FlushPendingCallbacks(endpointId);
   msgApi = nullptr;
}
