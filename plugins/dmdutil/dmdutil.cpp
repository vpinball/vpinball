// license:GPLv3+

#include <cstdlib>
#include <chrono>
#include <cstring>

#include "VPXPlugin.h"
#include "ControllerPlugin.h"
#include "LoggingPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // xxx needs dll-interface
#include "DMDUtil/DMDUtil.h"
#pragma warning(pop)

static MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;

static unsigned int onGameEndId;
static unsigned int onDmdSrcChangedId;
static unsigned int getDmdSrcMsgId;

static std::mutex sourceMutex;
static std::thread updateThread;
static DisplaySrcId selectedDmdId = {0};
static bool isRunning = false;

static DMDUtil::DMD* pDmd = nullptr;

static uint8_t tintR = 255;
static uint8_t tintG = 140;
static uint8_t tintB = 0;

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

LPI_IMPLEMENT

static void UpdateThread()
{
   int lastFrameID = 0;
   while (isRunning && pDmd && selectedDmdId.id.id != 0)
   {
      // Fixed update at 60 FPS
      std::this_thread::sleep_for(std::chrono::nanoseconds(16666));

      std::lock_guard<std::mutex> lock(sourceMutex);

      const DisplayFrame frame = selectedDmdId.GetRenderFrame(selectedDmdId.id);
      if (lastFrameID == frame.frameId)
         continue;
      lastFrameID = frame.frameId;

      switch(selectedDmdId.frameFormat) {
         case CTLPI_DISPLAY_FORMAT_LUM8:
         {
            const uint8_t* const __restrict luminanceData = frame.frame;
            uint8_t* const __restrict rgb24Data = (uint8_t*)malloc(selectedDmdId.width * selectedDmdId.height * 3);

            for (unsigned int i = 0; i < selectedDmdId.width * selectedDmdId.height; ++i) {
                const uint32_t lum = luminanceData[i];
                rgb24Data[i * 3    ] = (uint8_t)((lum * tintR) / 255u);
                rgb24Data[i * 3 + 1] = (uint8_t)((lum * tintG) / 255u);
                rgb24Data[i * 3 + 2] = (uint8_t)((lum * tintB) / 255u);
            }

            pDmd->UpdateRGB24Data(rgb24Data, selectedDmdId.width, selectedDmdId.height);
            free(rgb24Data);
         }
         break;

         case CTLPI_DISPLAY_FORMAT_SRGB888:
            pDmd->UpdateRGB24Data(frame.frame, selectedDmdId.width, selectedDmdId.height);
            break;

         case CTLPI_DISPLAY_FORMAT_SRGB565:
            pDmd->UpdateRGB16Data((const uint16_t*)frame.frame, selectedDmdId.width, selectedDmdId.height);
            break;
      }
   }
   isRunning = false;
}

static void onDmdSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   DisplaySrcId newDmdId = { 0 };

   GetDisplaySrcMsg getSrcMsg = { 1024, 0, new DisplaySrcId[1024] };
   msgApi->BroadcastMsg( endpointId, getDmdSrcMsgId, &getSrcMsg);

   bool foundDMD = false;

   // Select the largest color display
   for (unsigned int i = 0; i < getSrcMsg.count; i++) {
      if (getSrcMsg.entries[i].frameFormat != CTLPI_DISPLAY_FORMAT_LUM8) {
          if (getSrcMsg.entries[i].width > newDmdId.width) {
              newDmdId = getSrcMsg.entries[i];
              foundDMD = true;
          }
      }
   }

   // Defaults to the largest monochrome display
   if (!foundDMD) {
      for (unsigned int i = 0; i < getSrcMsg.count; i++) {
         if (getSrcMsg.entries[i].frameFormat == CTLPI_DISPLAY_FORMAT_LUM8) {
             if (getSrcMsg.entries[i].width > newDmdId.width) {
               newDmdId = getSrcMsg.entries[i];
               foundDMD = true;
            }
         }
      }
   }

   delete[] getSrcMsg.entries;

   std::lock_guard<std::mutex> lock(sourceMutex);
   selectedDmdId = newDmdId;

   if (foundDMD) {
      LOGI("DMD Source Changed: format=%d, width=%d, height=%d", newDmdId.frameFormat, newDmdId.width, newDmdId.height);
      if (!pDmd)
      {
         pDmd = new DMDUtil::DMD();
         pDmd->FindDisplays();

         char value[256];
         msgApi->GetSetting("DMDUtil", "LumTintR", value, sizeof(value));
         if (value[0] != '\0')
            tintR = (uint8_t)(strtol(value, NULL, 10) & 0xFF);

         msgApi->GetSetting("DMDUtil", "LumTintG", value, sizeof(value));
         if (value[0] != '\0')
            tintG = (uint8_t)(strtol(value, NULL, 10) & 0xFF);

         msgApi->GetSetting("DMDUtil", "LumTintB", value, sizeof(value));
         if (value[0] != '\0')
            tintB = (uint8_t)(strtol(value, NULL, 10) & 0xFF);
      }
      isRunning = true;
      if (!updateThread.joinable())
         updateThread = std::thread(UpdateThread);
   }
}

static void onGameEnd(const unsigned int msgId, void* userData, void* msgData)
{
   isRunning = false;
   if (updateThread.joinable())
      updateThread.join();
   delete pDmd;
   pDmd = nullptr;
}

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi); // Request and setup shared login API

   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), onDmdSrcChanged, nullptr);

   getDmdSrcMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   onGameEnd(onGameEndId, nullptr, nullptr);

   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->UnsubscribeMsg(onDmdSrcChangedId, onDmdSrcChanged);

   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcMsgId);

   msgApi = nullptr;
}
