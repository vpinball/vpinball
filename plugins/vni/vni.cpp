// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"
#include "pinmame/PinMAMEPlugin.h"
#include "common.h"
#include "vni.h"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <thread>
#include <random>

namespace Vni {

LPI_IMPLEMENT_CPP // Implement shared log support

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;

static uint32_t endpointId;
static unsigned int onControllersChangedId;
static unsigned int getControllersId;
static string currentGameId;
static unsigned int onDmdSrcChangedId;
static unsigned int getDmdSrcId;
static unsigned int onConsoleDataId;

static bool isRunning = false;
static std::mutex sourceMutex;
static std::mutex stateMutex;
static std::mutex consoleDataMutex;
static std::thread colorizeThread;
static DisplaySrcId dmdId = {};

static Vni_Context* pVni = nullptr;

static std::minstd_rand std_rand;

MSGPI_STRING_VAL_SETTING(vniPathProp, "VniPath", "VNI Path", "Folder that contains VNI colorization files (PAL, VNI)", true, "", 1024);

class ColorizationState final
{
public:
   ColorizationState(unsigned int width, unsigned int height)
      : m_colorFrame(new uint8_t[width * height * 3])
      , m_width(width), m_height(height)
      , m_colorizedframeId(std_rand())
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

static uint8_t consoleData[4] = {};
static uint32_t consoleDataSize = 0;

static int HexDigit(const uint8_t value)
{
   if (value >= '0' && value <= '9')
      return value - '0';
   if (value >= 'A' && value <= 'F')
      return value - 'A' + 10;
   if (value >= 'a' && value <= 'f')
      return value - 'a' + 10;
   return -1;
}

static void OnConsoleData(const unsigned int, void*, void* msgData)
{
   const auto* msg = static_cast<const PinMAMEConsoleDataMsg*>(msgData);
   if (msg == nullptr || msg->data == nullptr || msg->size == 0)
      return;

   std::lock_guard sourceLock(sourceMutex);
   std::lock_guard consoleLock(consoleDataMutex);
   for (uint32_t i = 0; i < msg->size; i++)
   {
      if (consoleDataSize < 4)
         consoleData[consoleDataSize++] = msg->data[i];
      else
      {
         consoleData[0] = consoleData[1];
         consoleData[1] = consoleData[2];
         consoleData[2] = consoleData[3];
         consoleData[3] = msg->data[i];
      }

      if (consoleDataSize == 4 && consoleData[0] == 'P')
      {
         const int hi = HexDigit(consoleData[1]);
         const int lo = HexDigit(consoleData[2]);
         if (hi >= 0 && lo >= 0)
         {
            if (pVni != nullptr)
               Vni_SetPalette(pVni, static_cast<uint32_t>((hi << 4) | lo));
         }
      }
   }
}

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

      if (dmdId.GetIdentifyFrame == nullptr)
         continue;
      const DisplayFrame frame = dmdId.GetIdentifyFrame(dmdId.id);
      if (frame.frame == nullptr)
         break;

      if (frame.frameId != lastFrameId)
      {
         lastFrameId = frame.frameId;
         const uint8_t bitlen = dmdId.identifyFormat == CTLPI_DISPLAY_ID_FORMAT_BITPLANE4 ? 4 : 2;
         const uint8_t* indexed = static_cast<const uint8_t*>(frame.frame);
         const uint32_t result = Vni_Colorize(pVni, indexed, dmdId.width, dmdId.height, bitlen);
         if (result)
         {
            const Vni_Frame_Struc* vniFrame = Vni_GetFrame(pVni);
            if (vniFrame && vniFrame->has_frame)
            {
               std::lock_guard<std::mutex> lock2(stateMutex);
               bool newState = false;
               unsigned int outWidth = vniFrame->width;
               unsigned int outHeight = vniFrame->height;
               if (state == nullptr)
               {
                  state = new ColorizationState(outWidth, outHeight);
                  newState = true;
               }
               else if (state->m_width != outWidth || state->m_height != outHeight)
               {
                  delete state;
                  state = new ColorizationState(outWidth, outHeight);
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
      const DisplaySrcId& candidate = getSrcMsg.entries[i];
      if (candidate.id.endpointId != endpointId && candidate.GetIdentifyFrame != nullptr && candidate.width >= 128)
      {
         dmdId = candidate;
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
   {
      std::lock_guard lock(consoleDataMutex);
      consoleDataSize = 0;
   }
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

static void OnControllersChanged(const unsigned int eventId, void* userData, void* msgData)
{
   // Enumerate and select the first controller exposing a PinMAME compatible game
   string selectedGameId;
   GetControllersMsg getControllersMsg = { 0, 0, nullptr };
   msgApi->BroadcastMsg(endpointId, getControllersId, &getControllersMsg);
   if (getControllersMsg.count > 0)
   {
      const string pinmamePrefix(PMPI_GAMEID_PREFIX);
      std::vector<ControllerDef> controllers(getControllersMsg.count);
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
   StopColorization();
   currentGameId = selectedGameId;
   if (currentGameId.empty())
      return;
   
   VPXTableInfo tableInfo;
   vpxApi->GetTableInfo(&tableInfo);
   std::filesystem::path tablePath = tableInfo.path;

   std::filesystem::path vniBasePath = vniPathProp_Get();
   const std::filesystem::path palFile = currentGameId + ".pal";
   const std::filesystem::path vniFile = currentGameId + ".vni";
   const std::filesystem::path pin2dmdPal = "pin2dmd.pal"s;
   const std::filesystem::path pin2dmdVni = "pin2dmd.vni"s;

   std::filesystem::path palPath, vniPath;

   // Priority 1: vni/<rom>/<rom>.pal and vni/<rom>/<rom>.vni
   if (auto path1 = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / palFile); !path1.empty())
   {
      palPath = path1;
      if (auto path2 = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / vniFile); !path2.empty())
         vniPath = path2;
   }
   // Priority 2: vni/<rom>/pin2dmd.pal and vni/<rom>/pin2dmd.vni
   else if (auto path3 = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / pin2dmdPal); !path3.empty())
   {
      palPath = path3;
      if (auto path4 = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / pin2dmdVni); !path4.empty())
         vniPath = path4;
   }
   // Priority 3: pinmame/altcolor/<rom>/<rom>.pal and pinmame/altcolor/<rom>/<rom>.vni
   else if (auto path5 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / palFile); !path5.empty())
   {
      palPath = path5;
      if (auto path6 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / vniFile); !path6.empty())
         vniPath = path6;
   }
   // Priority 4: pinmame/altcolor/<rom>/pin2dmd.pal and pinmame/altcolor/<rom>/pin2dmd.vni
   else if (auto path7 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / pin2dmdPal); !path7.empty())
   {
      palPath = path7;
      if (auto path8 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / pin2dmdVni); !path8.empty())
         vniPath = path8;
   }
   // Priority 5: global setting path
   else if (!vniBasePath.empty())
   {
      if (auto path9 = find_case_insensitive_file_path(vniBasePath / currentGameId / palFile); !path9.empty())
      {
         palPath = path9;
         if (auto path10 = find_case_insensitive_file_path(vniBasePath / currentGameId / vniFile); !path10.empty())
            vniPath = path10;
      }
   }


   if (palPath.empty())
   {
      LOGI("No PAL file found for " + currentGameId);
      return;
   }

   LOGI("Loading PAL from " + palPath.string());

   if (!vniPath.empty())
      LOGI("Loading VNI from " + vniPath.string());

   pVni = Vni_LoadFromPaths(palPath.string().c_str(), vniPath.empty() ? nullptr : vniPath.string().c_str(), nullptr, nullptr);
   OnDmdSrcChanged(onDmdSrcChangedId, nullptr, nullptr);
   if (pVni)
   {
      isRunning = true;
      colorizeThread = std::thread(ColorizeThread);
   }
   else
   {
      LOGE("Failed to load colorization data");
   }
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

   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), OnDmdSrcChanged, nullptr);
   msgApi->SubscribeMsg(endpointId, getDmdSrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG), OnGetRenderDMDSrc, nullptr);

   onConsoleDataId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_CONSOLE_DATA);
   msgApi->SubscribeMsg(endpointId, onConsoleDataId, OnConsoleData, nullptr);

   onControllersChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_ON_CHG_MSG);
   getControllersId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_CONTROLLERS_GET_MSG);
   msgApi->SubscribeMsg(endpointId, onControllersChangedId, OnControllersChanged, nullptr);
   OnControllersChanged(onControllersChangedId, nullptr, nullptr);
}

MSGPI_EXPORT void MSGPIAPI VNIPluginUnload()
{
   StopColorization();
   msgApi->UnsubscribeMsg(getDmdSrcId, OnGetRenderDMDSrc, nullptr);
   msgApi->UnsubscribeMsg(onDmdSrcChangedId, OnDmdSrcChanged, nullptr);
   msgApi->UnsubscribeMsg(onConsoleDataId, OnConsoleData, nullptr);
   msgApi->UnsubscribeMsg(onControllersChangedId, OnControllersChanged, nullptr);
   msgApi->ReleaseMsgID(onControllersChangedId);
   msgApi->ReleaseMsgID(getControllersId);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcId);
   msgApi->ReleaseMsgID(onConsoleDataId);
   msgApi->FlushPendingCallbacks(endpointId);
   msgApi = nullptr;
}
