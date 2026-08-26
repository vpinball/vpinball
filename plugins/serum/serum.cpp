// license:GPLv3+

#include "plugins/MsgPlugin.h"
#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"
#include "pinmame/PinMAMEPlugin.h"
#include "common.h"
#include "serum-decode.h"

#include <cassert>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// Serum Colorization plugin
//
// This plugin rely on the generic message plugin API and the generic controller
// plugin API, but also on VPX API to locate serum file

namespace Serum
{

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace PinballPlugin::Controller;

LPI_IMPLEMENT_CPP // Implement shared log support

static const MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;

static unsigned int onDmdTrigger;
static std::minstd_rand std_rand;

static std::unique_ptr<CtrlItemConsumer<ControllerDef>> controllers;
static std::unique_ptr<class SerumColorizer> colorizer;

MSGPI_STRING_VAL_SETTING(serumPathProp, "SerumPath", "Serum Path", "Folder that cotains Serum colorization files (cROMc, cRZ)", true, "", 1024);

class SerumColorizer
{
public:
   SerumColorizer(const std::filesystem::path& serumPath, const string& currentGameId, uint32_t controllerEndpointId)
      : m_pSerum(Serum_Load(serumPath.string().c_str(), currentGameId.c_str(), FLAG_REQUEST_32P_FRAMES | FLAG_REQUEST_64P_FRAMES))
      , m_controllerEndpointId(controllerEndpointId)
      , m_dmdSource(
           msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG, [this](std::vector<DisplaySrcId>& items) { FilterDmdSource(items); },
           [this]() { StopColorizeThread(); }, [this]() { StartColorizeThread(); })
      , m_colorizedDmd(msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG)
      , m_colorizedframeId(std_rand())
   {
      if (m_pSerum)
      {
         m_dmdSource.SelectItems(true);
      }
      else
      {
         LOGE("Failed to load colorization data");
      }
   }

   ~SerumColorizer()
   {
      StopColorizeThread();
      if (m_pSerum)
         Serum_Dispose();
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
               LOGI("Serum DMD colorizer stopped");
            }
            else
            {
               const DisplaySrcId& dmdSrc = items.front();
               LOGI(std::format("Serum colorizer source selected [endpointId={}.{}, {}x{} fmt={}]", dmdSrc.id.endpointId, dmdSrc.id.resId, dmdSrc.width, dmdSrc.height, dmdSrc.frameFormat));
               m_isRunning = true;
               m_colorizeThread = std::thread(&SerumColorizer::ColorizeThread, this, dmdSrc);
            }
         });
   }

   void StopColorizeThread()
   {
      m_isRunning = false;
      if (m_colorizeThread.joinable())
         m_colorizeThread.join();
      m_colorizedDmd.ClearItems();
      m_advertisedWidth32 = 0;
      m_advertisedWidth64 = 0;
      m_colorFrameV1.clear();
   }

   void ColorizeThread(DisplaySrcId dmdId)
   {
      SetThreadName("Serum.ColorizeThread"s);
      constexpr uint32_t SERUM_MAX_ROTATION_DELAY_MS = 2048;
      unsigned int lastFrameId = 0;
      bool hasAnimation = false;
      std::chrono::steady_clock::time_point animationTick;
      std::chrono::steady_clock::time_point animationNextTick;
      while (m_isRunning)
      {
         // Original PinMAME code would evaluate DMD frames at a fixed 60 FPS and color rotation are also based on a 60FPS rate. So update at this pace.
         std::this_thread::sleep_for(std::chrono::microseconds(16666));

         // Lock stateMutex as we directly returns internal Serum colorized frames (which may be modified by the calls here after)
         std::lock_guard targetLock(m_stateMutex);

         bool updated = false;

         // Process incoming frames from DMD source
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

               const uint32_t firstrot = Serum_Colorize(const_cast<uint8_t*>(static_cast<const uint8_t*>(frame.frame)));
               if (firstrot == IDENTIFY_NO_FRAME || firstrot == IDENTIFY_SAME_FRAME)
                  return;

               const uint32_t firstDelayMs = firstrot & 0x0000ffff;
               hasAnimation = (firstDelayMs != 0) && (firstDelayMs < SERUM_MAX_ROTATION_DELAY_MS);
               if (hasAnimation)
               {
                  animationTick = std::chrono::high_resolution_clock::now();
                  animationNextTick = animationTick + std::chrono::milliseconds(firstDelayMs);
               }

               if (m_pSerum->triggerID != 0xffffffff)
                  msgApi->RunOnMainThread(endpointId, 0, [](void* userData) { msgApi->BroadcastMsg(endpointId, onDmdTrigger, &colorizer->m_pSerum->triggerID); }, nullptr);

               updated = true;
            });

         // Perform current animation (catching up to the current time point)
         if (hasAnimation)
         {
            const auto now = std::chrono::high_resolution_clock::now();
            while (animationNextTick < now)
            {
               const uint32_t nextrot = Serum_Rotate();
               updated |= (m_pSerum->SerumVersion == SERUM_V1) && ((m_pSerum->flags & FLAG_RETURNED_V1_ROTATED) != 0);
               updated |= (m_pSerum->SerumVersion == SERUM_V2) && ((m_pSerum->flags & (FLAG_RETURNED_V2_ROTATED32 | FLAG_RETURNED_V2_ROTATED64)) != 0);
               const uint32_t delayMs = nextrot & 0x0000ffff;
               if (delayMs == 0 || delayMs >= SERUM_MAX_ROTATION_DELAY_MS)
               {
                  hasAnimation = false;
                  break;
               }
               animationTick = animationNextTick;
               animationNextTick = animationNextTick + std::chrono::milliseconds(delayMs);
            }
         }

         if (!updated)
            continue;

         if (m_pSerum->SerumVersion == SERUM_V1)
         {
            const unsigned int size = dmdId.width * dmdId.height;
            if (m_colorFrameV1.size() != size * 3)
            {
               msgApi->RunOnMainThread(
                  endpointId, -1,
                  [](void* userData)
                  {
                     SerumColorizer* colorizer = static_cast<SerumColorizer*>(userData);
                     DisplaySrcId dmdId = colorizer->m_dmdSource.With([&](const std::vector<DisplaySrcId>& items) { return items.front(); });
                     const unsigned int size = dmdId.width * dmdId.height;
                     colorizer->m_colorizedDmd.ClearItems();
                     // FIXME if a concurrent GetRenderFrame has been done returning the previous backing buffer, this will discard it and lead to an invalid mem access
                     colorizer->m_colorFrameV1.resize(size * 3);
                     colorizer->m_colorizedDmd.AddItem({
                        .id = { { endpointId, 0 } }, //
                        .groupId = { endpointId, 0 }, //
                        .overrideId = dmdId.id, //
                        .width = dmdId.width, //
                        .height = dmdId.height, //
                        .hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED, //
                        .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888, //
                        .GetRenderFrame = &GetRenderFrameSerumV1 //
                     });
                  },
                  this);
            }
            for (unsigned int i = 0; i < size; i++)
               memcpy(&(m_colorFrameV1[i * 3]), &m_pSerum->palette[m_pSerum->frame[i] * 3], 3);
         }
         else if (m_advertisedWidth32 != m_pSerum->width32 || m_advertisedWidth64 != m_pSerum->width64)
         {
            msgApi->RunOnMainThread(
               endpointId, -1,
               [](void* userData)
               {
                  SerumColorizer* colorizer = static_cast<SerumColorizer*>(userData);
                  DisplaySrcId dmdId = colorizer->m_dmdSource.With([&](const std::vector<DisplaySrcId>& items) { return items.front(); });
                  colorizer->m_colorizedDmd.ClearItems();
                  colorizer->m_advertisedWidth32 = colorizer->m_pSerum->width32;
                  colorizer->m_advertisedWidth64 = colorizer->m_pSerum->width64;
                  if (colorizer->m_advertisedWidth32 > 0)
                  {
                     colorizer->m_colorizedDmd.AddItem({
                        .id = { { endpointId, 1 } }, //
                        .groupId = { endpointId, 0 }, //
                        .overrideId = dmdId.id, //
                        .width = colorizer->m_advertisedWidth32, //
                        .height = 32, //
                        .hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED, //
                        .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB565, //
                        .GetRenderFrame = &GetRenderFrameSerumV2_32 //
                     });
                  }
                  if (colorizer->m_advertisedWidth64 > 0)
                  {
                     colorizer->m_colorizedDmd.AddItem({
                        .id = { { endpointId, 2 } }, //
                        .groupId = { endpointId, 0 }, //
                        .overrideId = dmdId.id, //
                        .width = colorizer->m_advertisedWidth64, //
                        .height = 64, //
                        .hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED, //
                        .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB565, //
                        .GetRenderFrame = &GetRenderFrameSerumV2_64 //
                     });
                  }
               },
               this);
         }
         m_colorizedframeId++;
      }
      m_isRunning = false;
   }

   // Note that to be fully clean we should do a copy of the render (since the direct data is updated asynchronously, so eventually while it is read by consumer)
   static DisplayFrame GetRenderFrameSerumV1(const CtlResId id)
   {
      std::lock_guard targetLock(colorizer->m_stateMutex);
      return { colorizer->m_colorizedframeId, colorizer->m_colorFrameV1.data() };
   }
   static DisplayFrame GetRenderFrameSerumV2_32(const CtlResId id)
   {
      std::lock_guard targetLock(colorizer->m_stateMutex);
      return { colorizer->m_colorizedframeId, reinterpret_cast<uint8_t*>(colorizer->m_pSerum->frame32) };
   }
   static DisplayFrame GetRenderFrameSerumV2_64(const CtlResId id)
   {
      std::lock_guard targetLock(colorizer->m_stateMutex);
      return { colorizer->m_colorizedframeId, reinterpret_cast<uint8_t*>(colorizer->m_pSerum->frame64) };
   }

   Serum_Frame_Struc* const m_pSerum;
   const uint32_t m_controllerEndpointId;

   CtrlItemConsumer<DisplaySrcId> m_dmdSource;
   CtrlItemProvider<DisplaySrcId> m_colorizedDmd;

   bool m_isRunning = false;
   std::thread m_colorizeThread;

   std::mutex m_stateMutex;
   std::vector<uint8_t> m_colorFrameV1;
   unsigned int m_advertisedWidth32 = 0;
   unsigned int m_advertisedWidth64 = 0;

   unsigned int m_colorizedframeId = 0;
};

static void OnControllerChanged()
{
   controllers->With(
      [](const std::vector<ControllerDef>& items)
      {
         if (items.empty())
         {
            LOGI("Serum colorizer stopped");
            return;
         }

         VPXTableInfo tableInfo;
         VPXPluginAPI* vpxApi = nullptr;
         unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
         msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
         msgApi->ReleaseMsgID(getVpxApiId);
         if (vpxApi == nullptr)
         {
            LOGE("Failed to get VPX API (needed to locate Serum files)");
            return;
         }
         vpxApi->GetTableInfo(&tableInfo);

         std::filesystem::path tablePath = tableInfo.path;

         // Simply select first controller exposing a PinMAME compatible game (should be only one anyway)
         const ControllerDef& selectedController = items.front();
         constexpr std::string_view pinmamePrefix(PMPI_GAMEID_PREFIX);
         const string currentGameId = string(selectedController.gameId).substr(pinmamePrefix.size());
         if (currentGameId.empty())
            return;

         std::filesystem::path serumPath = serumPathProp_Get();
         const std::filesystem::path cromc = currentGameId + ".cROMc"s;
         const std::filesystem::path crz = currentGameId + ".cRZ"s;

         // Priority 1: serum/rom/rom.cromc or .crz
         if (auto path1 = find_case_insensitive_file_path(tablePath.parent_path() / "serum"sv / currentGameId / cromc); !path1.empty())
            serumPath = path1.parent_path().parent_path();
         else if (auto path2 = find_case_insensitive_file_path(tablePath.parent_path() / "serum"sv / currentGameId / crz); !path2.empty())
            serumPath = path2.parent_path().parent_path();
         // Priority 2: pinmame/altcolor/rom/rom.cromc or .crz
         else if (auto path3 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / cromc); !path3.empty())
            serumPath = path3.parent_path().parent_path();
         else if (auto path4 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / currentGameId / crz); !path4.empty())
            serumPath = path4.parent_path().parent_path();
         // Priority 3: global setting path
         else if (!serumPath.empty())
         {
            if (find_case_insensitive_file_path(serumPath / currentGameId / cromc).empty() && find_case_insensitive_file_path(serumPath / currentGameId / crz).empty())
               serumPath.clear();
         }

         if (serumPath.empty())
         {
            LOGI("No colorization file found for "s + currentGameId);
            return;
         }

         LOGI("Loading from " + serumPath.string() + " for " + currentGameId);
         colorizer = std::make_unique<SerumColorizer>(serumPath, currentGameId, selectedController.endpointId);
      });
}

}

using namespace Serum;

MSGPI_EXPORT void MSGPIAPI SerumPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   LPISetup(endpointId, msgApi);
   msgApi->RegisterSetting(endpointId, &serumPathProp);
   onDmdTrigger = msgApi->GetMsgID("Serum", "OnDmdTrigger");
   controllers = std::make_unique<CtrlItemConsumer<ControllerDef>>(
      msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG,
      [](std::vector<ControllerDef>& items)
      {
         constexpr std::string_view pinmamePrefix(PMPI_GAMEID_PREFIX); // Keep only controllers exposing a PinMAME compatible game
         std::erase_if(items, [pinmamePrefix](const ControllerDef& controller) { return !string(controller.gameId).starts_with(pinmamePrefix); });
      },
      []() { colorizer = nullptr; }, []() { OnControllerChanged(); });
   controllers->SelectItems(true);
}

MSGPI_EXPORT void MSGPIAPI SerumPluginUnload()
{
   colorizer = nullptr;
   controllers = nullptr;
   msgApi->ReleaseMsgID(onDmdTrigger);
   msgApi = nullptr;
}
