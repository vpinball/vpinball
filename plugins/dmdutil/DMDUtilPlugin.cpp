// license:GPLv3+

#include <cstdlib>
#include <chrono>
#include <cstring>
#include <charconv>

#include "plugins/VPXPlugin.h"
#include "plugins/ControllerPlugin.h"
#include "plugins/LoggingPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // xxx needs dll-interface
#include "DMDUtil/DMDUtil.h"
#pragma warning(pop)

#define DMDUTIL_TINT_R 255
#define DMDUTIL_TINT_G 140
#define DMDUTIL_TINT_B 0

using namespace std;

namespace DMDUtilPlugin {
   
static const MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;

static unsigned int onGameEndId;
static unsigned int onDmdSrcChangedId;
static unsigned int getDmdSrcMsgId;

static std::mutex sourceMutex;
static std::thread updateThread;
static DisplaySrcId selectedDmdId = {};
static bool isRunning = false;

static DMDUtil::DMD* pDmd = nullptr;

static uint8_t tintR;
static uint8_t tintG;
static uint8_t tintB;

MSGPI_BOOL_VAL_SETTING(zeDMDProp, "ZeDMD", "ZeDMD", "", true, true);
MSGPI_STRING_VAL_SETTING(zeDMDDeviceFolderProp, "ZeDMDDevice", "ZeDMDDevice", "", true, "", 1024);
MSGPI_BOOL_VAL_SETTING(zeDMDDebugFolderProp, "ZeDMDDebug", "ZeDMDDebug", "", true, false);
MSGPI_INT_VAL_SETTING(zeDMDBrightnessFolderProp, "ZeDMDBrightness", "ZeDMDBrightness", "", true, -1, 1000, -1);
MSGPI_BOOL_VAL_SETTING(zeDMDWifiProp, "ZeDMDWiFi", "ZeDMDWiFi", "", true, false);
MSGPI_STRING_VAL_SETTING(zeDMDWiFiAddrFolderProp, "ZeDMDWiFiAddr", "ZeDMDWiFiAddr", "", true, "zedmd-wifi.local", 1024);
MSGPI_BOOL_VAL_SETTING(pixelcadeProp, "Pixelcade", "Pixelcade", "", true, true);
MSGPI_STRING_VAL_SETTING(pixelcadeDeviceProp, "PixelcadeDevice", "PixelcadeDevice", "", true, "", 1024);
MSGPI_BOOL_VAL_SETTING(dmdServerFolderProp, "DMDServer", "DMDServer", "", true, false);
MSGPI_STRING_VAL_SETTING(dmdServerAddrFolderProp, "DMDServerAddr", "DMDServerAddr", "", true, "localhost", 1024);
MSGPI_INT_VAL_SETTING(dmdServerPortFolderProp, "DMDServerPort", "DMDServerPort", "", true, 0, 1000, 6789);

MSGPI_BOOL_VAL_SETTING(findDisplaysProp, "FindDisplays", "FindDisplays", "", true, true);
MSGPI_BOOL_VAL_SETTING(dumpDMDTxtProp, "DumpDMDTxt", "DumpDMDTxt", "", true, false);
MSGPI_BOOL_VAL_SETTING(dumpDMDRawProp, "DumpDMDRaw", "DumpDMDRaw", "", true, false);
MSGPI_INT_VAL_SETTING(lumTintRProp, "LumTintR", "LumTintR", "", true, 0, 255, DMDUTIL_TINT_R);
MSGPI_INT_VAL_SETTING(lumTintGProp, "LumTintG", "LumTintG", "", true, 0, 255, DMDUTIL_TINT_G);
MSGPI_INT_VAL_SETTING(lumTintBProp, "LumTintB", "LumTintB", "", true, 0, 255, DMDUTIL_TINT_B);


LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGW LPI_LOGW
#define LOGE LPI_LOGE

LPI_IMPLEMENT

void DMDUTILCALLBACK OnDMDUtilLog(DMDUtil_LogLevel logLevel, const char* format, va_list args)
{
   va_list args_copy;
   va_copy(args_copy, args);
   int size = vsnprintf(nullptr, 0, format, args_copy);
   va_end(args_copy);
   if (size > 0) {
      char* const buffer = static_cast<char*>(malloc(size + 1));
      vsnprintf(buffer, size + 1, format, args);
      switch(logLevel) {
         case DMDUtil_LogLevel_INFO:
            LOGI("%s", buffer);
            break;
         case DMDUtil_LogLevel_DEBUG:
            LOGD("%s", buffer);
            break;
         case DMDUtil_LogLevel_ERROR:
            LOGE("%s", buffer);
            break;
         default:
            break;
      }
      free(buffer);
   }
}

static void UpdateThread()
{
   int lastFrameID = 0;
   while (isRunning && pDmd && selectedDmdId.id.id != 0)
   {
      // Fixed update at 60 FPS
      std::this_thread::sleep_for(std::chrono::microseconds(16666));

      std::lock_guard<std::mutex> lock(sourceMutex);

      const DisplayFrame frame = selectedDmdId.GetRenderFrame(selectedDmdId.id);
      if (lastFrameID == frame.frameId)
         continue;
      lastFrameID = frame.frameId;

      switch(selectedDmdId.frameFormat) {
         case CTLPI_DISPLAY_FORMAT_LUM32F:
         {
            const float* const __restrict luminanceData = static_cast<const float*>(frame.frame);
            uint8_t* const __restrict rgb24Data = (uint8_t*)malloc(selectedDmdId.width * selectedDmdId.height * 3);

            for (unsigned int i = 0; i < selectedDmdId.width * selectedDmdId.height; ++i) {
                const float lum = luminanceData[i];
                rgb24Data[i * 3    ] = (uint8_t)(lum * tintR);
                rgb24Data[i * 3 + 1] = (uint8_t)(lum * tintG);
                rgb24Data[i * 3 + 2] = (uint8_t)(lum * tintB);
            }

            pDmd->UpdateRGB24Data(rgb24Data, selectedDmdId.width, selectedDmdId.height);
            free(rgb24Data);
         }
         break;

         case CTLPI_DISPLAY_FORMAT_SRGB888:
            pDmd->UpdateRGB24Data(static_cast<const uint8_t*>(frame.frame), selectedDmdId.width, selectedDmdId.height);
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
   DisplaySrcId newDmdId = {};

   GetDisplaySrcMsg getSrcMsg = { 1024, 0, new DisplaySrcId[1024] };
   msgApi->BroadcastMsg( endpointId, getDmdSrcMsgId, &getSrcMsg);

   bool foundDMD = false;

   // Select the largest color display
   for (unsigned int i = 0; i < getSrcMsg.count; i++) {
      if (getSrcMsg.entries[i].frameFormat != CTLPI_DISPLAY_FORMAT_LUM32F) {
          if (getSrcMsg.entries[i].width > newDmdId.width) {
              newDmdId = getSrcMsg.entries[i];
              foundDMD = true;
          }
      }
   }

   // Defaults to the largest monochrome display
   if (!foundDMD) {
      for (unsigned int i = 0; i < getSrcMsg.count; i++) {
         if (getSrcMsg.entries[i].frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F) {
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
      if (!pDmd) {
         pDmd = new DMDUtil::DMD();

         if (findDisplaysProp_Val)
             pDmd->FindDisplays();

         if (dumpDMDTxtProp_Val)
             pDmd->DumpDMDTxt();

         if (dumpDMDRawProp_Val)
             pDmd->DumpDMDRaw();

         tintR = static_cast<uint8_t>(lumTintRProp_Val);
         tintG = static_cast<uint8_t>(lumTintGProp_Val);
         tintB = static_cast<uint8_t>(lumTintBProp_Val);
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

}

using namespace DMDUtilPlugin;

MSGPI_EXPORT void MSGPIAPI DMDUtilPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi); // Request and setup shared login API

   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), onDmdSrcChanged, nullptr);

   msgApi->RegisterSetting(endpointId, &zeDMDProp);
   msgApi->RegisterSetting(endpointId, &zeDMDDeviceFolderProp);
   msgApi->RegisterSetting(endpointId, &zeDMDDebugFolderProp);
   msgApi->RegisterSetting(endpointId, &zeDMDBrightnessFolderProp);
   msgApi->RegisterSetting(endpointId, &zeDMDWifiProp);
   msgApi->RegisterSetting(endpointId, &zeDMDWiFiAddrFolderProp);
   msgApi->RegisterSetting(endpointId, &pixelcadeProp);
   msgApi->RegisterSetting(endpointId, &pixelcadeDeviceProp);
   msgApi->RegisterSetting(endpointId, &dmdServerFolderProp);
   msgApi->RegisterSetting(endpointId, &dmdServerAddrFolderProp);
   msgApi->RegisterSetting(endpointId, &dmdServerPortFolderProp);

   msgApi->RegisterSetting(endpointId, &findDisplaysProp);
   msgApi->RegisterSetting(endpointId, &dumpDMDTxtProp);
   msgApi->RegisterSetting(endpointId, &dumpDMDRawProp);
   msgApi->RegisterSetting(endpointId, &lumTintRProp);
   msgApi->RegisterSetting(endpointId, &lumTintGProp);
   msgApi->RegisterSetting(endpointId, &lumTintBProp);

   getDmdSrcMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);

   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetLogCallback(OnDMDUtilLog);
   pConfig->SetZeDMD(zeDMDProp_Val);
   pConfig->SetZeDMDDevice(zeDMDDeviceFolderProp_Get());
   pConfig->SetZeDMDDebug(zeDMDDebugFolderProp_Get());
   pConfig->SetZeDMDBrightness(zeDMDBrightnessFolderProp_Val);
   pConfig->SetZeDMDWiFiEnabled(zeDMDWifiProp_Val);
   pConfig->SetZeDMDWiFiAddr(zeDMDWiFiAddrFolderProp_Get());
   pConfig->SetPixelcade(pixelcadeProp_Val);
   pConfig->SetPixelcadeDevice(pixelcadeDeviceProp_Get());
   pConfig->SetDMDServer(dmdServerFolderProp_Val);
   pConfig->SetDMDServerAddr(dmdServerAddrFolderProp_Get());
   pConfig->SetDMDServerPort(dmdServerPortFolderProp_Val);
}

MSGPI_EXPORT void MSGPIAPI DMDUtilPluginUnload()
{
   onGameEnd(onGameEndId, nullptr, nullptr);

   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->UnsubscribeMsg(onDmdSrcChangedId, onDmdSrcChanged);

   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcMsgId);

   msgApi = nullptr;
}
