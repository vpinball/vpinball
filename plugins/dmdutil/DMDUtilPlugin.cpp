// license:GPLv3+

#include <cstdlib>
#include <chrono>
#include <cstring>
#include <charconv>

#include "VPXPlugin.h"
#include "ControllerPlugin.h"
#include "LoggingPlugin.h"

#pragma warning(push)
#pragma warning(disable : 4251) // xxx needs dll-interface
#include "DMDUtil/DMDUtil.h"
#pragma warning(pop)

#define DMDUTIL_TINT_R 255
#define DMDUTIL_TINT_G 140
#define DMDUTIL_TINT_B 0

using namespace std;

namespace DMDUtilPlugin {
   
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

static uint8_t tintR;
static uint8_t tintG;
static uint8_t tintB;

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
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
      if (!pDmd) {
         pDmd = new DMDUtil::DMD();

         if (GetSettingBool(msgApi, "DMDUtil", "FindDisplays", true))
             pDmd->FindDisplays();

         if (GetSettingBool(msgApi, "DMDUtil", "DumpDMDTxt", false))
             pDmd->DumpDMDTxt();

         if (GetSettingBool(msgApi, "DMDUtil", "DumpDMDRaw", false))
             pDmd->DumpDMDRaw();

         tintR = static_cast<uint8_t>(GetSettingInt(msgApi, "DMDUtil", "LumTintR", DMDUTIL_TINT_R));
         tintG = static_cast<uint8_t>(GetSettingInt(msgApi, "DMDUtil", "LumTintG", DMDUTIL_TINT_G));
         tintB = static_cast<uint8_t>(GetSettingInt(msgApi, "DMDUtil", "LumTintB", DMDUTIL_TINT_B));
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
   msgApi = const_cast<MsgPluginAPI*>(api);
   endpointId = sessionId;

   LPISetup(endpointId, msgApi); // Request and setup shared login API

   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_ON_SRC_CHG_MSG), onDmdSrcChanged, nullptr);

   getDmdSrcMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_DISPLAY_GET_SRC_MSG);

   DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
   pConfig->SetLogCallback(OnDMDUtilLog);
   pConfig->SetZeDMD(GetSettingBool(msgApi, "DMDUtil", "ZeDMD", true));
   pConfig->SetZeDMDDevice(GetSettingString(msgApi, "DMDUtil", "ZeDMDDevice", string()).c_str());
   pConfig->SetZeDMDDebug(GetSettingBool(msgApi, "DMDUtil", "ZeDMDDebug", false));
   pConfig->SetZeDMDBrightness(GetSettingInt(msgApi, "DMDUtil", "ZeDMDBrightness", -1));
   pConfig->SetZeDMDWiFiEnabled(GetSettingBool(msgApi, "DMDUtil", "ZeDMDWiFi", false));
   pConfig->SetZeDMDWiFiAddr(GetSettingString(msgApi, "DMDUtil", "ZeDMDWiFiAddr", "zedmd-wifi.local"s).c_str());
   pConfig->SetPixelcade(GetSettingBool(msgApi, "DMDUtil", "Pixelcade", true));
   pConfig->SetPixelcadeDevice(GetSettingString(msgApi, "DMDUtil", "PixelcadeDevice", string()).c_str());
   pConfig->SetDMDServer(GetSettingBool(msgApi, "DMDUtil", "DMDServer", false));
   pConfig->SetDMDServerAddr(GetSettingString(msgApi, "DMDUtil", "DMDServerAddr", "localhost"s).c_str());
   pConfig->SetDMDServerPort(GetSettingInt(msgApi, "DMDUtil", "DMDServerPort", 6789));
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
