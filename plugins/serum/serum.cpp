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
#include <algorithm>
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
// Serum skips frames it cannot identify. How long to keep showing the last
// known-good colorized frame before giving up on the unknown run, and how many
// unknown frames to skip within it, are colorization- and ROM-dependent, so
// they have to be tunable by the host. libserum defaults to 0/0, which means
// "no timeout, skip nothing".
MSGPI_INT_VAL_SETTING(serumIgnoreUnknownFramesTimeoutProp, "IgnoreUnknownFramesTimeout", "Ignore unknown frames timeout",
   "Milliseconds to keep the last colorized frame while frames cannot be identified (0 disables)", true, 0, 65535, 0);
MSGPI_INT_VAL_SETTING(serumMaxUnknownFramesToSkipProp, "MaximumUnknownFramesToSkip", "Maximum unknown frames to skip",
   "How many consecutive unidentified frames may be skipped (0 disables)", true, 0, 255, 0);
// A Serum v2 colorization can carry a 32 row and a 64 row output, and by
// default both are requested and both are published. Two things go wrong with
// that in a host driving one fixed panel:
//
// - Nothing downstream can say which one it wants. The two published sources
//   share an overrideId, and consumers disagree about how to break the tie --
//   DMDUtilPlugin takes the largest colour depth, ResURIResolver breaks on the
//   first match -- so a 128x32 panel can end up downscaling a colorization that
//   was upscaled to 256x64.
// - libserum computes both outputs every frame. On a Raspberry Pi driving an SD
//   panel that is half the colorization work thrown away.
//
// Setting this makes the choice explicit: only the requested size is computed
// and only it is published, so there is nothing left to disagree about. The
// default keeps today's behaviour, which is the right one for a host that does
// not know its output size in advance.
MSGPI_INT_VAL_SETTING(serumResolutionProp, "Resolution", "Colorization resolution",
   "Rows of the colorized output: 32, 64, or 0 to produce both", true, 0, 64, 0);

// A display Serum can colorize, and FilterDmdSource can later accept for the selected controller
static bool IsColorizableDmd(const DisplaySrcId& display) { return display.GetIdentifyFrame != nullptr && display.width >= 128; }

// Anything other than an exact 32 or 64 means "produce both", so a stray value
// degrades to the default rather than to no output at all.
static bool IsResolutionRequested(int rows)
{
   const int requested = serumResolutionProp_Get();
   return (requested != 32 && requested != 64) || requested == rows;
}

static unsigned int SerumRequestFlags()
{
   unsigned int flags = 0;
   if (IsResolutionRequested(32))
      flags |= FLAG_REQUEST_32P_FRAMES;
   if (IsResolutionRequested(64))
      flags |= FLAG_REQUEST_64P_FRAMES;
   return flags;
}

class SerumColorizer
{
public:
   SerumColorizer(const std::filesystem::path& serumPath, const std::string_view& currentGameId, uint32_t controllerEndpointId)
      : m_pSerum(Serum_Load(serumPath.string().c_str(), string(currentGameId).c_str(), SerumRequestFlags()))
      , m_controllerEndpointId(controllerEndpointId)
      , m_colorizedDmd(msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG)
      , m_colorizedframeId(std_rand())
      , m_dmdSource(
           msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG, [this](std::vector<DisplaySrcId>& items) { FilterDmdSource(items); },
           [this]() { StopColorizeThread(); }, [this]() { StartColorizeThread(); })
   {
      if (m_pSerum)
      {
         Serum_SetIgnoreUnknownFramesTimeout(static_cast<uint16_t>(serumIgnoreUnknownFramesTimeoutProp_Get()));
         Serum_SetMaximumUnknownFramesToSkip(static_cast<uint8_t>(serumMaxUnknownFramesToSkipProp_Get()));

         m_dmdSource.Subscribe();
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
      {
         m_dmdSource.Unsubscribe();
         Serum_Dispose();
      }
   }

private:
   void FilterDmdSource(std::vector<DisplaySrcId>& items)
   {
      // Only keep dmd corresponding to selected controller (or overrides to support alphanumeric rendered DMD for example)
      const std::function<bool(const DisplaySrcId&)> isFromController = [&](const DisplaySrcId& src)
      {
         if (CtlDisplayControllerId(&src) == m_controllerEndpointId)
            return true;
         if (src.overrideId.id != 0)
            for (const DisplaySrcId& item : items)
               if (item.id == src.overrideId)
                  return isFromController(item);
         return false;
      };

      DisplaySrcId selected { };
      for (const DisplaySrcId& item : items)
         if (isFromController(item) && IsColorizableDmd(item))
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
         std::unique_lock targetLock(m_stateMutex);

         bool updated = false;

         // Process incoming frames from DMD source
         m_dmdSource.With(
            [&](const std::vector<DisplaySrcId>& items)
            {
               const DisplayFrame frame = dmdId.GetIdentifyFrame(dmdId.callContext);
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
                  animationTick = std::chrono::steady_clock::now();
                  animationNextTick = animationTick + std::chrono::milliseconds(firstDelayMs);
               }

               if (m_pSerum->triggerID != 0xffffffff)
                  msgApi->RunOnMainThread(endpointId, 0, [](void* userData) { msgApi->BroadcastMsg(endpointId, onDmdTrigger, &colorizer->m_pSerum->triggerID); }, nullptr);

               updated = true;
            });

         // Perform current animation (catching up to the current time point)
         if (hasAnimation)
         {
            const auto now = std::chrono::steady_clock::now();
            while (animationNextTick < now)
            {
               const uint32_t nextrot = Serum_Rotate();
               updated |= (nextrot & (FLAG_RETURNED_V1_ROTATED | FLAG_RETURNED_V2_ROTATED32 | FLAG_RETURNED_V2_ROTATED64 | FLAG_RETURNED_V2_SCENE)) != 0;
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
               // Blocks on the main thread, which may be waiting for m_stateMutex in GetRenderFrame
               targetLock.unlock();
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
                        .overrideId = dmdId.id, //
                        .controllerId = CtlDisplayControllerId(&dmdId), //
                        .width = dmdId.width, //
                        .height = dmdId.height, //
                        .hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED, //
                        .callContext = colorizer, //
                        .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888, //
                        .GetRenderFrame = &Trampoline<&SerumColorizer::GetRenderFrameSerumV1>::Call //
                     });
                  },
                  this);
               targetLock.lock();
            }
            for (unsigned int i = 0; i < size; i++)
               memcpy(&(m_colorFrameV1[i * 3]), &m_pSerum->palette[m_pSerum->frame[i] * 3], 3);
         }
         // Widths are per frame (0 when a size is missing), so advertise each size once and keep it
         else if ((m_pSerum->width32 != 0 && m_advertisedWidth32 != m_pSerum->width32) || (m_pSerum->width64 != 0 && m_advertisedWidth64 != m_pSerum->width64))
         {
            // Blocks on the main thread, which may be waiting for m_stateMutex in GetRenderFrame
            targetLock.unlock();
            msgApi->RunOnMainThread(
               endpointId, -1,
               [](void* userData)
               {
                  SerumColorizer* colorizer = static_cast<SerumColorizer*>(userData);
                  DisplaySrcId dmdId = colorizer->m_dmdSource.With([&](const std::vector<DisplaySrcId>& items) { return items.front(); });
                  colorizer->m_colorizedDmd.ClearItems();
                  if (colorizer->m_pSerum->width32 != 0)
                     colorizer->m_advertisedWidth32 = colorizer->m_pSerum->width32;
                  if (colorizer->m_pSerum->width64 != 0)
                     colorizer->m_advertisedWidth64 = colorizer->m_pSerum->width64;
                  LOGI(std::format("Publishing colorized output: {}{}",
                     (colorizer->m_advertisedWidth32 > 0 && IsResolutionRequested(32)) ? std::format("{}x32 ", colorizer->m_advertisedWidth32) : ""s,
                     (colorizer->m_advertisedWidth64 > 0 && IsResolutionRequested(64)) ? std::format("{}x64", colorizer->m_advertisedWidth64) : ""s));
                  if (colorizer->m_advertisedWidth32 > 0 && IsResolutionRequested(32))
                  {
                     colorizer->m_colorizedDmd.AddItem({
                        .id = { { endpointId, 1 } }, //
                        .overrideId = dmdId.id, //
                        .controllerId = CtlDisplayControllerId(&dmdId), //
                        .width = colorizer->m_advertisedWidth32, //
                        .height = 32, //
                        .hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED, //
                        .callContext = colorizer, //
                        .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB565, //
                        .GetRenderFrame = &Trampoline<&SerumColorizer::GetRenderFrameSerumV2_32>::Call //
                     });
                  }
                  if (colorizer->m_advertisedWidth64 > 0 && IsResolutionRequested(64))
                  {
                     colorizer->m_colorizedDmd.AddItem({
                        .id = { { endpointId, 2 } }, //
                        .overrideId = dmdId.id, //
                        .controllerId = CtlDisplayControllerId(&dmdId), //
                        .width = colorizer->m_advertisedWidth64, //
                        .height = 64, //
                        .hardware = CTLPI_DISPLAY_HARDWARE_RGB_LED, //
                        .callContext = colorizer, //
                        .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB565, //
                        .GetRenderFrame = &Trampoline<&SerumColorizer::GetRenderFrameSerumV2_64>::Call //
                     });
                  }
               },
               this);
            targetLock.lock();
         }
         m_colorizedframeId++;
      }
      m_isRunning = false;
   }

   // Note that to be fully clean we should do a copy of the render (since the direct data is updated asynchronously, so eventually while it is read by consumer)
   DisplayFrame GetRenderFrameSerumV1()
   {
      std::lock_guard targetLock(m_stateMutex);
      return { m_colorizedframeId, m_colorFrameV1.data() };
   }
   DisplayFrame GetRenderFrameSerumV2_32()
   {
      std::lock_guard targetLock(m_stateMutex);
      return { m_colorizedframeId, reinterpret_cast<uint8_t*>(m_pSerum->frame32) };
   }
   DisplayFrame GetRenderFrameSerumV2_64()
   {
      std::lock_guard targetLock(m_stateMutex);
      return { m_colorizedframeId, reinterpret_cast<uint8_t*>(m_pSerum->frame64) };
   }

   Serum_Frame_Struc* const m_pSerum;
   const uint32_t m_controllerEndpointId;

   CtrlItemProvider<DisplaySrcId> m_colorizedDmd;

   bool m_isRunning = false;
   std::thread m_colorizeThread;

   std::mutex m_stateMutex;
   std::vector<uint8_t> m_colorFrameV1;
   unsigned int m_advertisedWidth32 = 0;
   unsigned int m_advertisedWidth64 = 0;

   unsigned int m_colorizedframeId = 0;

   CtrlItemConsumer<DisplaySrcId> m_dmdSource;
};

static std::filesystem::path GetColorization(const std::string_view& gameId)
{
   VPXTableInfo tableInfo;
   VPXPluginAPI* vpxApi = nullptr;
   unsigned int getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API);
   msgApi->BroadcastMsg(endpointId, getVpxApiId, &vpxApi);
   msgApi->ReleaseMsgID(getVpxApiId);

   const std::filesystem::path cromc = std::format("{}{}", gameId, ".cROMc");
   const std::filesystem::path crz = std::format("{}{}", gameId, ".cRZ");

   // Priorities 1 and 2 are relative to the table, so they only apply in a host
   // that has tables. Other hosts of this plugin -- PPUC drives real pinball
   // hardware, and there are headless colorization tools -- have no VPX API at
   // all, and must still reach the global setting below. Returning early here
   // made GetColorization report "no colorization" for every game in those
   // hosts, which SelectController reads as "this controller cannot be
   // colorized", so Serum never loaded at all.
   if (vpxApi != nullptr)
   {
      vpxApi->GetTableInfo(&tableInfo);
      const std::filesystem::path tablePath = tableInfo.path;

      // Priority 1: serum/rom/rom.cromc or .crz
      if (auto path1 = find_case_insensitive_file_path(tablePath.parent_path() / "serum"sv / gameId / cromc); !path1.empty())
         return path1.parent_path().parent_path();
      else if (auto path2 = find_case_insensitive_file_path(tablePath.parent_path() / "serum"sv / gameId / crz); !path2.empty())
         return path2.parent_path().parent_path();
      // Priority 2: pinmame/altcolor/rom/rom.cromc or .crz
      else if (auto path3 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / gameId / cromc); !path3.empty())
         return path3.parent_path().parent_path();
      else if (auto path4 = find_case_insensitive_file_path(tablePath.parent_path() / "pinmame"sv / "altcolor"sv / gameId / crz); !path4.empty())
         return path4.parent_path().parent_path();
   }

   // Priority 3: global setting path
   if (std::filesystem::path serumPath = serumPathProp_Get();
      !serumPath.empty() && (!find_case_insensitive_file_path(serumPath / gameId / cromc).empty() || !find_case_insensitive_file_path(serumPath / gameId / crz).empty()))
      return serumPath;

   return std::filesystem::path();
}

// Select the first controller exposing an identifiable DMD and a game for which we have the corresponding assets
static void SelectController(std::vector<ControllerDef>& items)
{
   const unsigned int getDisplaySrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   const std::vector<DisplaySrcId> displays = PinballPlugin::Controller::GetCtrlItems<DisplaySrcId>(msgApi, endpointId, getDisplaySrcId);
   msgApi->ReleaseMsgID(getDisplaySrcId);
   for (const ControllerDef& controller : items)
   {
      const bool hasIdentifiableDmd = std::any_of(displays.begin(), displays.end(),
         [&controller](const DisplaySrcId& display) { return CtlDisplayControllerId(&display) == controller.endpointId && IsColorizableDmd(display); });
      const std::string_view gameId = PinballPlugin::Controller::CtrlGetGameKey(controller.gameId);
      if (hasIdentifiableDmd && !gameId.empty() && !GetColorization(gameId).empty())
      {
         items.clear();
         items.push_back(controller);
         return;
      }
   }
   items.clear();
}

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
         const ControllerDef& selectedController = items.front();
         const std::string_view currentGameId = PinballPlugin::Controller::CtrlGetGameKey(selectedController.gameId);
         const std::filesystem::path serumPath = GetColorization(currentGameId);
         LOGI(std::format("Loading from '{}' for '{}'", serumPath.string(), currentGameId));
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
   msgApi->RegisterSetting(endpointId, &serumIgnoreUnknownFramesTimeoutProp);
   msgApi->RegisterSetting(endpointId, &serumMaxUnknownFramesToSkipProp);
   msgApi->RegisterSetting(endpointId, &serumResolutionProp);
   onDmdTrigger = msgApi->GetMsgID("Serum", "OnDmdTrigger:1");
   controllers = std::make_unique<CtrlItemConsumer<ControllerDef>>(
      msgApi, endpointId, CTLPI_CONTROLLERS_GET_MSG, CTLPI_CONTROLLERS_ON_CHG_MSG, [](std::vector<ControllerDef>& items) { SelectController(items); }, []() { colorizer = nullptr; },
      []() { OnControllerChanged(); });
   controllers->Subscribe();
}

MSGPI_EXPORT void MSGPIAPI SerumPluginUnload()
{
   controllers->Unsubscribe();
   controllers = nullptr;
   msgApi->ReleaseMsgID(onDmdTrigger);
   msgApi = nullptr;
}
