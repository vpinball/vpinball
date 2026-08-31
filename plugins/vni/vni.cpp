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

namespace Vni
{

using namespace PinballPlugin::Controller;

LPI_IMPLEMENT_CPP // Implement shared log support

static const MsgPluginAPI* msgApi = nullptr;
static VPXPluginAPI* vpxApi = nullptr;
static uint32_t endpointId;

static std::minstd_rand std_rand;

static std::unique_ptr<CtrlItemConsumer<ControllerDef>> controllers;
static std::unique_ptr<class VNIColorizer> colorizer;

MSGPI_STRING_VAL_SETTING(vniPathProp, "VniPath", "VNI Path", "Folder that contains VNI colorization files (PAL, VNI)", true, "", 1024);

class VNIColorizer
{
public:
   VNIColorizer(const std::filesystem::path& palPath, const std::filesystem::path& vniPath, uint32_t controllerEndpointId)
      : m_controllerEndpointId(controllerEndpointId)
      , m_palPath(palPath)
      , m_vniPath(vniPath)
      , m_colorizedDmd(msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG)
      , m_onConsoleDataId(msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_CONSOLE_DATA))
      , m_colorizedframeId(std_rand())
      , m_dmdSource(
           msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG, [this](std::vector<DisplaySrcId>& items) { FilterDmdSource(items); },
           [this]() { StopColorizeThread(); }, [this]() { StartColorizeThread(); })
   {
         msgApi->SubscribeMsg(endpointId, m_onConsoleDataId, OnConsoleDataStatic, this);
         m_dmdSource.Subscribe();
   }

   ~VNIColorizer()
   {
      StopColorizeThread();
      m_dmdSource.Unsubscribe();
      msgApi->UnsubscribeMsg(m_onConsoleDataId, OnConsoleDataStatic, this);
      msgApi->ReleaseMsgID(m_onConsoleDataId);
   }

private:
   void FilterDmdSource(std::vector<DisplaySrcId>& items)
   {
      // Only keep dmd corresponding to selected controller (or overrides to support alphanumeric rendered DMD for example)
      const std::function<bool(const DisplaySrcId&)> isFromController = [&](const DisplaySrcId& src)
      {
         if (src.id.endpointId == m_controllerEndpointId)
            return true;
         if (src.overrideId.id != 0)
            for (const DisplaySrcId& item : items)
               if (item.id == src.overrideId)
                  return isFromController(item);
         return false;
      };

      DisplaySrcId selected { };
      for (const DisplaySrcId& item : items)
         if (isFromController(item) && item.GetIdentifyFrame != nullptr && item.width >= 128)
            selected = item;

      items.clear();
      if (selected.id.id != 0)
         items.push_back(selected);
   }

   void StartColorizeThread()
   {
      m_dmdSource.With(
         [this](const std::vector<DisplaySrcId>& items)
         {
            if (items.empty())
            {
               LOGI("VNI DMD colorizer stopped");
            }
            else
            {
               const DisplaySrcId& dmdSrc = items.front();
               LOGI(std::format("VNI colorizer source selected [endpointId={}.{}, {}x{} fmt={}]", dmdSrc.id.endpointId, dmdSrc.id.resId, dmdSrc.width, dmdSrc.height, dmdSrc.frameFormat));
               m_isRunning = true;
               m_colorizeThread = std::thread(&VNIColorizer::ColorizeThread, this, dmdSrc);
            }
         });
   }

   void StopColorizeThread()
   {
      m_isRunning = false;
      if (m_colorizeThread.joinable())
         m_colorizeThread.join();
      {
         std::lock_guard lock(m_consoleDataMutex);
         m_consoleDataSize = 0;
      }
      m_colorizedDmd.ClearItems();
      m_advertisedWidth = 0;
      m_advertisedHeight = 0;
   }

   void ColorizeThread(DisplaySrcId dmdId)
   {
      m_pVNI = Vni_LoadFromPaths(m_palPath.string().c_str(), m_vniPath.empty() ? nullptr : m_vniPath.string().c_str(), nullptr, nullptr);
      if (m_pVNI == nullptr)
      {
         LOGE("Failed to load colorization data");
         m_isRunning = false;
         return;
      }

      SetThreadName("VNI.ColorizeThread"s);
      unsigned int lastFrameId = 0;
      while (m_isRunning)
      {
         // Original PinMAME code would evaluate DMD frames at a fixed 60 FPS and color rotation are also based on a 60FPS rate. So update at this pace.
         std::this_thread::sleep_for(std::chrono::microseconds(16666));

         {
            std::lock_guard stateLock(m_stateMutex);

            if (m_pendingAdvertisement)
               continue;

            // Process incoming frames from DMD source
            const Vni_Frame_Struc* vniFrame = nullptr;
            m_dmdSource.With(
               [&](const std::vector<DisplaySrcId>& items)
               {
                  const DisplayFrame frame = dmdId.GetIdentifyFrame(dmdId.id);
                  if (frame.frame == nullptr)
                  {
                     m_isRunning = false;
                     return;
                  }

                  if (frame.frameId == lastFrameId)
                     return;
                  lastFrameId = frame.frameId;

                  const uint8_t bitlen = dmdId.identifyFormat == CTLPI_DISPLAY_ID_FORMAT_BITPLANE4 ? 4 : 2;
                  const uint8_t* indexed = static_cast<const uint8_t*>(frame.frame);
                  const uint32_t result = Vni_Colorize(m_pVNI, indexed, dmdId.width, dmdId.height, bitlen);
                  if (!result)
                     return;

                  vniFrame = Vni_GetFrame(m_pVNI);
               });

            if (!vniFrame || !vniFrame->has_frame)
               continue;

            if (vniFrame->width != m_advertisedWidth || vniFrame->height != m_advertisedHeight)
            {
               m_pendingAdvertisement = true;
               msgApi->RunOnMainThread(
                  endpointId, 0,
                  [](void* userData)
                  {
                     std::lock_guard stateLock(colorizer->m_stateMutex);
                     const Vni_Frame_Struc* vniFrame = static_cast<const Vni_Frame_Struc*>(userData);
                     DisplaySrcId dmdId = colorizer->m_dmdSource.With([&](const std::vector<DisplaySrcId>& items) { return items.front(); });
                     colorizer->m_advertisedWidth = vniFrame->width;
                     colorizer->m_advertisedHeight = vniFrame->height;
                     colorizer->m_pendingAdvertisement = false;
                     colorizer->m_colorFrame.resize(vniFrame->width * vniFrame->height * 3);
                     colorizer->m_colorizedDmd.SetItem({
                        .id = { { endpointId, 0 } }, //
                        .groupId = { endpointId, 0 }, //
                        .overrideId = dmdId.id, //
                        .width = vniFrame->width, //
                        .height = vniFrame->height, //
                        .hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED, //
                        .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888, //
                        .GetRenderFrame = &GetRenderFrame //
                     });
                  },
                  const_cast<Vni_Frame_Struc*>(vniFrame));
               continue;
            }

            for (unsigned int i = 0; i < vniFrame->width * vniFrame->height; i++)
               memcpy(&(m_colorFrame[i * 3]), &vniFrame->palette[vniFrame->frame[i] * 3], 3);
            m_colorizedframeId++;
         }
      }
      Vni_Dispose(m_pVNI);
      m_pVNI = nullptr;
      m_isRunning = false;
   }

   // Note that to be fully clean we should do a copy of the render (since the direct data is updated asynchronously, so eventually while it is read by consumer)
   static DisplayFrame GetRenderFrame(const CtlResId id)
   {
      std::lock_guard targetLock(colorizer->m_stateMutex);
      return { colorizer->m_colorizedframeId, colorizer->m_colorFrame.data() };
   }

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

   static void OnConsoleDataStatic(const unsigned int msgId, void* context, void* msgData)
   {
      static_cast<VNIColorizer*>(context)->OnConsoleData(static_cast<PinMAMEConsoleDataMsg*>(msgData));
   }

   void OnConsoleData(PinMAMEConsoleDataMsg* msg)
   {
      if (msg == nullptr || msg->data == nullptr || msg->size == 0)
         return;

      std::lock_guard targetLock(m_stateMutex);
      std::lock_guard consoleLock(m_consoleDataMutex);
      for (uint32_t i = 0; i < msg->size; i++)
      {
         if (m_consoleDataSize < 4)
            m_consoleData[m_consoleDataSize++] = msg->data[i];
         else
         {
            m_consoleData[0] = m_consoleData[1];
            m_consoleData[1] = m_consoleData[2];
            m_consoleData[2] = m_consoleData[3];
            m_consoleData[3] = msg->data[i];
         }

         if (m_consoleDataSize == 4 && m_consoleData[0] == 'P')
         {
            const int hi = HexDigit(m_consoleData[1]);
            const int lo = HexDigit(m_consoleData[2]);
            if (hi >= 0 && lo >= 0)
            {
               if (m_pVNI != nullptr)
                  Vni_SetPalette(m_pVNI, static_cast<uint32_t>((hi << 4) | lo));
            }
         }
      }
   }

   const uint32_t m_controllerEndpointId;
   const std::filesystem::path m_palPath;
   const std::filesystem::path m_vniPath;

   CtrlItemProvider<DisplaySrcId> m_colorizedDmd;

   std::atomic_bool m_isRunning { false };
   std::thread m_colorizeThread;
   Vni_Context* m_pVNI = nullptr;

   const unsigned int m_onConsoleDataId;
   std::mutex m_consoleDataMutex;
   uint8_t m_consoleData[4] = { };
   uint32_t m_consoleDataSize = 0;

   std::mutex m_stateMutex;
   std::vector<uint8_t> m_colorFrame;
   bool m_pendingAdvertisement = false;
   unsigned int m_advertisedWidth = 0;
   unsigned int m_advertisedHeight = 0;
   unsigned int m_colorizedframeId = 0;

   CtrlItemConsumer<DisplaySrcId> m_dmdSource;
};

static void OnControllersChanged()
{
   controllers->With(
      [](const std::vector<ControllerDef>& items)
      {
         if (items.empty())
         {
            LOGI("VNI/PAL colorizer stopped");
            return;
         }

         // Simply select first controller exposing a PinMAME compatible game (should be only one anyway)
         const ControllerDef& selectedController = items.front();
         constexpr std::string_view pinmamePrefix(PMPI_GAMEID_PREFIX);
         const string currentGameId = string(selectedController.gameId).substr(pinmamePrefix.size());

         if (currentGameId.empty())
            return;

         VPXTableInfo tableInfo;
         vpxApi->GetTableInfo(&tableInfo);
         std::filesystem::path tablePath = tableInfo.path;

         std::filesystem::path vniBasePath = vniPathProp_Get();
         const std::filesystem::path palFile = currentGameId + ".pal";
         const std::filesystem::path vniFile = currentGameId + ".vni";
         const std::filesystem::path pin2dmdPal = "pin2dmd.pal"sv;
         const std::filesystem::path pin2dmdVni = "pin2dmd.vni"sv;

         std::filesystem::path palPath, vniPath;

         // Priority 1: vni/<rom>/<rom>.pal and vni/<rom>/<rom>.vni
         if (auto palTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / palFile); !palTestPath.empty())
         {
            palPath = palTestPath;
            if (auto vniTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / vniFile); !vniTestPath.empty())
               vniPath = vniTestPath;
         }
         // Priority 2: vni/<rom>/pin2dmd.pal and vni/<rom>/pin2dmd.vni
         else if (auto palTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / pin2dmdPal); !palTestPath.empty())
         {
            palPath = palTestPath;
            if (auto vniTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "vni"sv / currentGameId / pin2dmdVni); !vniTestPath.empty())
               vniPath = vniTestPath;
         }
         // Priority 3: pinmame/altcolor/<rom>/<rom>.pal and pinmame/altcolor/<rom>/<rom>.vni
         else if (auto palTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / palFile); !palTestPath.empty())
         {
            palPath = palTestPath;
            if (auto vniTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / vniFile); !vniTestPath.empty())
               vniPath = vniTestPath;
         }
         // Priority 4: pinmame/altcolor/<rom>/pin2dmd.pal and pinmame/altcolor/<rom>/pin2dmd.vni
         else if (auto palTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / pin2dmdPal); !palTestPath.empty())
         {
            palPath = palTestPath;
            if (auto vniTestPath = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / pin2dmdVni); !vniTestPath.empty())
               vniPath = vniTestPath;
         }
         else if (!vniBasePath.empty())
         {
            // Priority 5: global setting: path/<rom>/<rom>.vni
            if (auto palTestPath = find_case_insensitive_file_path(vniBasePath / currentGameId / palFile); !palTestPath.empty())
            {
               palPath = palTestPath;
               if (auto vniTestPath = find_case_insensitive_file_path(vniBasePath / currentGameId / vniFile); !vniTestPath.empty())
                  vniPath = vniTestPath;
            }
            // Priority 6: global setting: path/<rom>/pin2dmd.vni
            else if (auto palTestPath = find_case_insensitive_file_path(vniBasePath / currentGameId / pin2dmdPal); !palTestPath.empty())
            {
               palPath = palTestPath;
               if (auto vniTestPath = find_case_insensitive_file_path(vniBasePath / currentGameId / pin2dmdVni); !vniTestPath.empty())
                  vniPath = vniTestPath;
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

         colorizer = std::make_unique<VNIColorizer>(palPath, vniPath, selectedController.endpointId);
      });
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

   controllers = std::make_unique<CtrlItemConsumer<ControllerDef>>(
      msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
      [](std::vector<ControllerDef>& items)
      {
         constexpr std::string_view pinmamePrefix(PMPI_GAMEID_PREFIX); // Keep only controllers exposing a PinMAME compatible game
         std::erase_if(items, [pinmamePrefix](const ControllerDef& controller) { return !string(controller.gameId).starts_with(pinmamePrefix); });
      },
      []() { colorizer = nullptr; }, []() { OnControllersChanged(); });
   controllers->Subscribe();
}

MSGPI_EXPORT void MSGPIAPI VNIPluginUnload()
{
   controllers->Unsubscribe();
   controllers = nullptr;
   msgApi = nullptr;
}
