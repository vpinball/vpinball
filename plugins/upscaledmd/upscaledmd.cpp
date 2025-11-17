// license:GPLv3+

#include "MsgPlugin.h"
#include "ControllerPlugin.h"
#include "LoggingPlugin.h"

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstring>
#include <cstdint>
#include <sstream>
#include <cstdarg>
#if defined(__APPLE__) || defined(__linux__) || defined(__ANDROID__)
#include <pthread.h>
#endif

#include "xbrz/xbrz.cpp"

#include "super-xbr/super-xbr.h"

#include "scalefx/scalefx.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <locale>
#endif

///////////////////////////////////////////////////////////////////////////////
//
// UpscaleDMD plugin: upscale DMD content to a high res DMD or a LCD/CRT display
//
// This plugin only relies on the generic messaging plugin API and the generic
// controller display and segment API. It listens for DMD source and,
// when found, provides a corresponding display source.
//
// All rendering is done by an ancillary thread, causing a one frame delay, but
// avoiding CPU load on the main thread.

namespace UpscaleDMD
{

using namespace std::string_literals;

static const MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;
static unsigned int onDisplaySrcChangedId, getDisplaySrcId;

static DisplaySrcId dmdSrc, displayId;

static std::mutex sourceMutex;
static std::thread renderThread;
static std::condition_variable updateCondVar;
static bool isRunning = false;

static std::vector<uint32_t> rgbaSrcFrame;
static std::vector<uint32_t> rgbaDstFrame;
static std::vector<uint8_t> renderFrame;
static unsigned int renderFrameId = 0;

enum UpscalerMode {
   UM_Disabled,
   UM_xBRZ_2x,
   UM_xBRZ_3x,
   UM_xBRZ_4x,
   UM_xBRZ_5x,
   UM_xBRZ_6x,
   UM_ScaleFX_AA,
   UM_ScaleFX_3x,
   UM_SuperXBR_2x
};
static const char* upscalerNames[] = { "Disabled", "xBRZ 2x", "xBRZ 3x", "xBRZ 4x", "xBRZ 5x", "xBRZ 6x", "ScaleFX AA 1x", "ScaleFX 3x", "Super-XBR 2x" };
static constexpr int scaleFactors[] = { 1, 2, 3, 4, 5, 6, 1, 3, 2 };
static UpscalerMode upscalerMode = UM_Disabled;
static UpscalerMode nextUpscalerMode = UM_Disabled;
static int GetUpscalerMode() { return (int)upscalerMode; }
static void OnDmdSrcChanged(const unsigned int, void*, void*);
static void SetUpscalerMode(int v)
{
   nextUpscalerMode = (UpscalerMode) v;
   OnDmdSrcChanged(endpointId, nullptr, nullptr);
}
MSGPI_ENUM_SETTING(upscaleModeProp, "UpscaleMode", "Mode", "Select upscaler", true, 0, std::size(scaleFactors), upscalerNames, 0, GetUpscalerMode, SetUpscalerMode);

LPI_USE();
LPI_IMPLEMENT // Implement shared login support


#ifdef _WIN32
static void SetThreadName(const std::string& name)
{
   const int size_needed = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, nullptr, 0);
   if (size_needed <= 1)
      return;
   std::wstring wstr(size_needed - 1, L'\0');
   if (MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wstr.data(), size_needed) == 0)
      return;
   SetThreadDescription(GetCurrentThread(), wstr.c_str());
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

static bool IsSourceSelected() { return dmdSrc.GetRenderFrame != nullptr; }

static void RenderThread()
{
   SetThreadName("UpscaleDMD.RenderThread"s);
   while (isRunning)
   {
      std::unique_lock lock(sourceMutex);
      updateCondVar.wait(lock); // Wait for a frame to be requested, locking the source for the time of its processing

      if (!isRunning)
         break;

      if (!IsSourceSelected())
         continue;

      // Render display
      DisplayFrame srcFrame = dmdSrc.GetRenderFrame(dmdSrc.id);

      if (srcFrame.frame && srcFrame.frameId != renderFrameId)
      {
         const unsigned int dmdSrcSize = dmdSrc.width * dmdSrc.height;
         if (dmdSrc.frameFormat == CTLPI_DISPLAY_FORMAT_LUM8)
            for (unsigned int ofs = 0; ofs < dmdSrcSize; ++ofs)
            {
               const uint32_t lum = srcFrame.frame[ofs];
               rgbaSrcFrame[ofs] = lum | (lum << 8) | (lum << 16) | 0xFF000000u;
            }
         else if (dmdSrc.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
            for (unsigned int ofs = 0; ofs < dmdSrcSize; ++ofs)
            {
               const uint32_t r = srcFrame.frame[ofs * 3 + 0];
               const uint32_t g = srcFrame.frame[ofs * 3 + 1];
               const uint32_t b = srcFrame.frame[ofs * 3 + 2];
               rgbaSrcFrame[ofs] = r | (g << 8) | (b << 16) | 0xFF000000u;
            }
         else if (dmdSrc.frameFormat == CTLPI_DISPLAY_FORMAT_SRGB565)
         {
            static constexpr uint8_t lum32[]
               = { 0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132, 140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255 };
            static constexpr uint8_t lum64[] = { 0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69, 73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134,
               138, 142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198, 202, 206, 210, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255 };
            const uint16_t* const __restrict frame = reinterpret_cast<const uint16_t*>(srcFrame.frame);
            for (unsigned int ofs = 0; ofs < dmdSrcSize; ++ofs)
            {
               const uint16_t rgb565 = frame[ofs];
               rgbaSrcFrame[ofs] = 0xFF000000u | ((uint32_t)lum32[rgb565 & 0x1F] << 16) | ((uint32_t)lum64[(rgb565 >> 5) & 0x3F] << 8) | (uint32_t)lum32[(rgb565 >> 11) & 0x1F];
            }
         }
         if (upscalerMode >= UpscalerMode::UM_xBRZ_2x && upscalerMode <= UpscalerMode::UM_xBRZ_6x)
            xbrz::scale(scaleFactors[upscalerMode], rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height, xbrz::ColorFormat::RGB);
         else if (upscalerMode >= UpscalerMode::UM_ScaleFX_AA && upscalerMode <= UpscalerMode::UM_ScaleFX_3x)
         {
            if (upscalerMode == UpscalerMode::UM_ScaleFX_3x)
               scalefx::upscale<true>(rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height, false);
            else
               scalefx::upscale<false>(rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height, false);
         }
         else if (upscalerMode == UpscalerMode::UM_SuperXBR_2x)
         {
            superxbr::scale<2,false>(rgbaSrcFrame.data(), rgbaDstFrame.data(), dmdSrc.width, dmdSrc.height);
         }

         const unsigned int displaySize = displayId.width * displayId.height;
         for (unsigned int ofs = 0; ofs < displaySize; ++ofs)
         {
            const uint32_t col = rgbaDstFrame[ofs];
            renderFrame[ofs * 3 + 0] = col & 0xFFu;
            renderFrame[ofs * 3 + 1] = (col >> 8) & 0xFFu;
            renderFrame[ofs * 3 + 2] = (col >> 16) & 0xFFu;
         }
         renderFrameId = srcFrame.frameId;
      }
   }
   isRunning = false;
}

static DisplayFrame GetRenderFrame(const CtlResId id)
{
   updateCondVar.notify_all();
   return { renderFrameId, renderFrame.data() };
}

static void OnGetDisplaySrc(const unsigned int, void*, void* msgData)
{
   if (!IsSourceSelected())
      return;
   GetDisplaySrcMsg& msg = *static_cast<GetDisplaySrcMsg*>(msgData);
   if (msg.count < msg.maxEntryCount)
      msg.entries[msg.count] = displayId;
   msg.count++;
}

static void OnDmdSrcChanged(const unsigned int, void*, void*)
{
   bool wasRendering = IsSourceSelected();
   bool modeChanged = upscalerMode != nextUpscalerMode;
   DisplaySrcId selectedSrc {};

   if (nextUpscalerMode != UpscalerMode::UM_Disabled)
   {
      GetDisplaySrcMsg getSrcMsg = { 0, 0, nullptr };
      msgApi->BroadcastMsg(endpointId, getDisplaySrcId, &getSrcMsg);
      if (getSrcMsg.count > 0)
      {
         std::vector<DisplaySrcId> sources(getSrcMsg.count);
         getSrcMsg = { (unsigned int)sources.size(), 0, sources.data() };
         msgApi->BroadcastMsg(endpointId, getDisplaySrcId, &getSrcMsg);
         for (unsigned int i = 0; i < getSrcMsg.count; i++)
         {
            if (getSrcMsg.entries[i].id.endpointId != endpointId)
            {
               const unsigned int sSize = getSrcMsg.entries[i].width * getSrcMsg.entries[i].height;
               const unsigned int dsSize = selectedSrc.width * selectedSrc.height;
               if (
                  // Priority 1: Find at least one display if any (size > 0)
                  selectedSrc.GetRenderFrame == nullptr
                  // Priority 2: Favor the highest resolution display
                  || (dsSize < sSize)
                  // Priority 3: Favor color over monochrome
                  || (dsSize == sSize && selectedSrc.frameFormat != getSrcMsg.entries[i].frameFormat && selectedSrc.frameFormat == CTLPI_DISPLAY_FORMAT_LUM8)
                  // Priority 4: Favor RGB8 over other formats
                  || (dsSize == sSize && selectedSrc.frameFormat != getSrcMsg.entries[i].frameFormat && getSrcMsg.entries[i].frameFormat == CTLPI_DISPLAY_FORMAT_SRGB888)
                  // Priority 5: Favor the first source provided by an endpoint
                  || (dsSize == sSize && selectedSrc.frameFormat == getSrcMsg.entries[i].frameFormat && selectedSrc.id.resId > getSrcMsg.entries[i].id.resId))
                  selectedSrc = getSrcMsg.entries[i];
            }
         }
      }
   }

   // Setup new source and rendering
   {
      std::unique_lock lock(sourceMutex);
      dmdSrc = selectedSrc;
      upscalerMode = nextUpscalerMode;
      if (IsSourceSelected())
      {
         displayId = {
            .id = { { endpointId, 0 } },
            .groupId = { { endpointId, 0 } },
            .overrideId = selectedSrc.overrideId.id == 0 ? selectedSrc.id : selectedSrc.overrideId,
            .width = dmdSrc.width * scaleFactors[upscalerMode],
            .height = dmdSrc.height * scaleFactors[upscalerMode],
            .hardware = CTLPI_DISPLAY_HARDWARE_UNKNOWN,
            .frameFormat = CTLPI_DISPLAY_FORMAT_SRGB888,
            .GetRenderFrame = &GetRenderFrame,
            .identifyFormat = CTLPI_DISPLAY_ID_FORMAT_BITPLANE2,
            .GetIdentifyFrame = nullptr //
         };
         renderFrameId = 0;
         rgbaSrcFrame.resize(dmdSrc.width * dmdSrc.height);
         rgbaDstFrame.resize(displayId.width * displayId.height);
         renderFrame.resize(displayId.width * displayId.height * 3);
         memset(renderFrame.data(), 0, renderFrame.size());
      }
      if (wasRendering != IsSourceSelected())
      {
         if (IsSourceSelected())
         {
            isRunning = true;
            renderThread = std::thread(RenderThread);
            msgApi->SubscribeMsg(endpointId, getDisplaySrcId, OnGetDisplaySrc, nullptr);
         }
         else
         {
            msgApi->UnsubscribeMsg(getDisplaySrcId, OnGetDisplaySrc);
            isRunning = false;
            lock.unlock();
            updateCondVar.notify_all();
            if (renderThread.joinable())
               renderThread.join();
         }
      }
   }

   // If we are starting or stopping rendering, or changed our display, report it
   if (modeChanged || wasRendering != IsSourceSelected())
      msgApi->BroadcastMsg(endpointId, onDisplaySrcChangedId, nullptr);
}

}

using namespace UpscaleDMD;

MSGPI_EXPORT void MSGPIAPI UpscaleDMDPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   onDisplaySrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG);
   getDisplaySrcId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
   msgApi->RegisterSetting(endpointId, &upscaleModeProp);
   msgApi->SubscribeMsg(endpointId, onDisplaySrcChangedId, OnDmdSrcChanged, nullptr);
   OnDmdSrcChanged(onDisplaySrcChangedId, nullptr, nullptr);
}

MSGPI_EXPORT void MSGPIAPI UpscaleDMDPluginUnload()
{
   if (IsSourceSelected())
      msgApi->UnsubscribeMsg(getDisplaySrcId, OnGetDisplaySrc);
   isRunning = false;
   updateCondVar.notify_all();
   if (renderThread.joinable())
      renderThread.join();
   msgApi->ReleaseMsgID(onDisplaySrcChangedId);
   msgApi->ReleaseMsgID(getDisplaySrcId);
   msgApi = nullptr;
}
