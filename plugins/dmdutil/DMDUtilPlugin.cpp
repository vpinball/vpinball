// license:GPLv3+

#include <cstdlib>
#include <atomic>
#include <chrono>
#include <cstring>
#include <charconv>
#include <format>

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

using namespace PinballPlugin::Controller;

static const MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;

static std::unique_ptr<CtrlItemConsumer<DisplaySrcId>> dmdSource;
static std::unique_ptr<class DMDUtilDispatcher> dmdDispatcher;

MSGPI_BOOL_VAL_SETTING(zeDMDProp, "ZeDMD", "ZeDMD", "", true, false);
MSGPI_STRING_VAL_SETTING(zeDMDDeviceFolderProp, "ZeDMDDevice", "ZeDMDDevice", "", true, "", 1024);
MSGPI_BOOL_VAL_SETTING(zeDMDDebugFolderProp, "ZeDMDDebug", "ZeDMDDebug", "", true, false);
MSGPI_INT_VAL_SETTING(zeDMDBrightnessFolderProp, "ZeDMDBrightness", "ZeDMDBrightness", "", true, -1, 1000, -1);
MSGPI_BOOL_VAL_SETTING(zeDMDWiFiEnabledProp, "ZeDMDWiFiEnabled", "ZeDMDWiFiEnabled", "", true, false);
MSGPI_STRING_VAL_SETTING(zeDMDWiFiAddrFolderProp, "ZeDMDWiFiAddr", "ZeDMDWiFiAddr", "", true, "zedmd-wifi.local", 1024);
MSGPI_BOOL_VAL_SETTING(zeDMDSPIEnabledProp, "ZeDMDSPIEnabled", "ZeDMDSPIEnabled", "", true, false);
MSGPI_INT_VAL_SETTING(zeDMDSPISpeedProp, "ZeDMDSPISpeed", "ZeDMDSPISpeed", "", true, 0, 100000000, 72000000);
MSGPI_INT_VAL_SETTING(zeDMDSPIFramePauseProp, "ZeDMDSPIFramePause", "ZeDMDSPIFramePause", "", true, 0, 1000, 2);
MSGPI_INT_VAL_SETTING(zeDMDSPIWidthProp, "ZeDMDSPIWidth", "ZeDMDSPIWidth", "", true, 0, 1000, 128);
MSGPI_INT_VAL_SETTING(zeDMDSPIHeightProp, "ZeDMDSPIHeight", "ZeDMDSPIHeight", "", true, 0, 1000, 32);
MSGPI_BOOL_VAL_SETTING(pixelcadeProp, "Pixelcade", "Pixelcade", "", true, false);
MSGPI_STRING_VAL_SETTING(pixelcadeDeviceProp, "PixelcadeDevice", "PixelcadeDevice", "", true, "", 1024);
MSGPI_BOOL_VAL_SETTING(pin2dmdProp, "PIN2DMD", "PIN2DMD", "", true, false);
MSGPI_BOOL_VAL_SETTING(dmdServerFolderProp, "DMDServer", "DMDServer", "", true, false);
MSGPI_STRING_VAL_SETTING(dmdServerAddrFolderProp, "DMDServerAddr", "DMDServerAddr", "", true, "localhost", 1024);
MSGPI_INT_VAL_SETTING(dmdServerPortFolderProp, "DMDServerPort", "DMDServerPort", "", true, 0, 65535, 6789);

MSGPI_BOOL_VAL_SETTING(findDisplaysProp, "FindDisplays", "FindDisplays", "", true, true);
MSGPI_BOOL_VAL_SETTING(dumpDMDTxtProp, "DumpDMDTxt", "DumpDMDTxt", "", true, false);
MSGPI_BOOL_VAL_SETTING(dumpDMDRawProp, "DumpDMDRaw", "DumpDMDRaw", "", true, false);
MSGPI_INT_VAL_SETTING(lumTintRProp, "LumTintR", "LumTintR", "", true, 0, 255, DMDUTIL_TINT_R);
MSGPI_INT_VAL_SETTING(lumTintGProp, "LumTintG", "LumTintG", "", true, 0, 255, DMDUTIL_TINT_G);
MSGPI_INT_VAL_SETTING(lumTintBProp, "LumTintB", "LumTintB", "", true, 0, 255, DMDUTIL_TINT_B);


LPI_USE_CPP();
#define LOGD DMDUtilPlugin::LPI_LOGD_CPP
#define LOGI DMDUtilPlugin::LPI_LOGI_CPP
#define LOGW DMDUtilPlugin::LPI_LOGW_CPP
#define LOGE DMDUtilPlugin::LPI_LOGE_CPP

LPI_IMPLEMENT_CPP // Implement shared log support

class DMDUtilDispatcher
{
public:
   DMDUtilDispatcher()
   {
      DMDUtil::Config* pConfig = DMDUtil::Config::GetInstance();
      pConfig->SetLogCallback(OnDMDUtilLog);
      pConfig->SetZeDMD(zeDMDProp_Val);
      pConfig->SetZeDMDDevice(zeDMDDeviceFolderProp_Get());
      pConfig->SetZeDMDDebug(zeDMDDebugFolderProp_Get());
      pConfig->SetZeDMDBrightness(zeDMDBrightnessFolderProp_Val);
      pConfig->SetZeDMDWiFiEnabled(zeDMDWiFiEnabledProp_Val);
      pConfig->SetZeDMDWiFiAddr(zeDMDWiFiAddrFolderProp_Get());
      pConfig->SetZeDMDSpiEnabled(zeDMDSPIEnabledProp_Val);
      pConfig->SetZeDMDSpiSpeed(zeDMDSPISpeedProp_Val);
      pConfig->SetZeDMDSpiFramePause(zeDMDSPIFramePauseProp_Val);
      pConfig->SetZeDMDWidth(zeDMDSPIWidthProp_Val);
      pConfig->SetZeDMDHeight(zeDMDSPIHeightProp_Val);
      pConfig->SetPixelcade(pixelcadeProp_Val);
      pConfig->SetPixelcadeDevice(pixelcadeDeviceProp_Get());
      pConfig->SetPIN2DMD(pin2dmdProp_Val);
      pConfig->SetDMDServer(dmdServerFolderProp_Val);
      pConfig->SetDMDServerAddr(dmdServerAddrFolderProp_Get());
      pConfig->SetDMDServerPort(dmdServerPortFolderProp_Val);

      m_pDmd = std::make_unique<DMDUtil::DMD>();

      if (findDisplaysProp_Val)
         m_pDmd->FindDisplays();

      if (dumpDMDTxtProp_Val)
         m_pDmd->DumpDMDTxt();

      if (dumpDMDRawProp_Val)
         m_pDmd->DumpDMDRaw();

      m_updateThread = std::thread(&DMDUtilDispatcher::UpdateThread, this);
   }

   ~DMDUtilDispatcher()
   {
      m_isRunning = false;
      if (m_updateThread.joinable())
         m_updateThread.join();
   }

private:
   void UpdateThread()
   {
      int lastFrameID = 0;
      while (m_isRunning)
      {
         // Fixed update at 60 FPS
         // TODO the dispatch should be done at the refesh rate of the target display
         std::this_thread::sleep_for(std::chrono::microseconds(16666));

         std::lock_guard lock(dmdSource->GetListMutex());
         const std::vector<DisplaySrcId>& items = dmdSource->GetItems();
         if (items.empty())
            continue;

         const DisplaySrcId& dmdSource = items.front();
         const DisplayFrame frame = dmdSource.GetRenderFrame(dmdSource.id);
         if (lastFrameID == frame.frameId)
            continue;
         lastFrameID = frame.frameId;

         switch (dmdSource.frameFormat)
         {
         case CTLPI_DISPLAY_FORMAT_LUM32F:
         {
            const float* const __restrict luminanceData = static_cast<const float*>(frame.frame);
            uint8_t* const __restrict rgb24Data = new uint8_t[dmdSource.width * dmdSource.height * 3];

            const float tintR = static_cast<float>(lumTintRProp_Val);
            const float tintG = static_cast<float>(lumTintGProp_Val);
            const float tintB = static_cast<float>(lumTintBProp_Val);

            for (unsigned int i = 0; i < dmdSource.width * dmdSource.height; ++i)
            {
               const float lum = luminanceData[i];
               rgb24Data[i * 3] = (uint8_t)(lum * tintR);
               rgb24Data[i * 3 + 1] = (uint8_t)(lum * tintG);
               rgb24Data[i * 3 + 2] = (uint8_t)(lum * tintB);
            }

            m_pDmd->UpdateRGB24Data(rgb24Data, dmdSource.width, dmdSource.height);
            delete[] rgb24Data;
         }
         break;

         case CTLPI_DISPLAY_FORMAT_SRGB888: m_pDmd->UpdateRGB24Data(static_cast<const uint8_t*>(frame.frame), dmdSource.width, dmdSource.height); break;

         case CTLPI_DISPLAY_FORMAT_SRGB565: m_pDmd->UpdateRGB16Data((const uint16_t*)frame.frame, dmdSource.width, dmdSource.height); break;
         }
      }
   }

   static void DMDUTILCALLBACK OnDMDUtilLog(DMDUtil_LogLevel logLevel, const char* format, va_list args)
   {
      va_list args_copy;
      va_copy(args_copy, args);
      int size = vsnprintf(nullptr, 0, format, args_copy);
      va_end(args_copy);
      if (size > 0)
      {
         string buffer(size + 1, '\0');
         vsnprintf(buffer.data(), size + 1, format, args);
         buffer.pop_back(); // remove null terminator
         switch (logLevel)
         {
         case DMDUtil_LogLevel_INFO: LOGI(buffer); break;
         case DMDUtil_LogLevel_DEBUG: LOGD(buffer); break;
         case DMDUtil_LogLevel_ERROR: LOGE(buffer); break;
         default: break;
         }
      }
   }

   std::unique_ptr<DMDUtil::DMD> m_pDmd;
   std::thread m_updateThread;
   bool m_isRunning = true;
};


static void SelectSource(std::vector<DisplaySrcId>& items)
{
   bool foundDMD = false;
   DisplaySrcId newDmdId = { };

   // Skip video monitor sources of pinball/video hybrids like Baby Pac-Man or Granny
   // and the Gators, which are flagged as CRT displays and are not meant for DMD
   // devices. Also skip sources larger than libdmdutil's update buffers, which are
   // fixed at 256x64 pixels and overflowed by larger frames (vpinball/libdmdutil#65).
   constexpr unsigned int maxPixels = 256 * 64;
   auto isSupported = [](const DisplaySrcId& src)
   {
      if ((src.hardware & CTLPI_DISPLAY_HARDWARE_FAMILY_MASK) == CTLPI_DISPLAY_HARDWARE_CRT_DISPLAY)
      {
         LOGI(std::format("Display source of {}x{} pixels is a video display and cannot be shown on DMD devices, skipping it", src.width, src.height));
         return false;
      }
      if ((unsigned int)src.width * src.height > maxPixels)
      {
         LOGW(std::format("Display source of {}x{} pixels exceeds the size supported by libdmdutil and cannot be shown on DMD devices, skipping it", src.width, src.height));
         return false;
      }
      return true;
   };

   // Select the largest color display
   for (const DisplaySrcId& src : items)
   {
      if (src.frameFormat != CTLPI_DISPLAY_FORMAT_LUM32F && isSupported(src))
      {
         if (src.width > newDmdId.width)
         {
            newDmdId = src;
            foundDMD = true;
         }
      }
   }

   // Defaults to the largest monochrome display
   if (!foundDMD)
   {
      for (const DisplaySrcId& src : items)
      {
         if (src.frameFormat == CTLPI_DISPLAY_FORMAT_LUM32F && isSupported(src))
         {
            if (src.width > newDmdId.width)
            {
               newDmdId = src;
               foundDMD = true;
            }
         }
      }
   }

   items.clear();
   if (foundDMD)
      items.push_back(newDmdId);
}

static void OnSourceChanged()
{
   dmdDispatcher = nullptr;

   std::lock_guard lock(dmdSource->GetListMutex());
   const std::vector<DisplaySrcId>& items = dmdSource->GetItems();
   if (items.empty())
   {
      LOGI("No DMD source selected");
   }
   else
   {
      const DisplaySrcId& dmdSrc = items.front();
      LOGI(std::format("DMD source selected [endpointId={}.{}, {}x{} fmt={}]", dmdSrc.id.endpointId, dmdSrc.id.resId, dmdSrc.width, dmdSrc.height, dmdSrc.frameFormat));
      dmdDispatcher = std::make_unique<DMDUtilDispatcher>();
   }
}

}

using namespace DMDUtilPlugin;

MSGPI_EXPORT void MSGPIAPI DMDUtilPluginLoad(const uint32_t sessionId, const MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   LPISetup(endpointId, msgApi); // Request and setup shared login API

   msgApi->RegisterSetting(endpointId, &zeDMDProp);
   msgApi->RegisterSetting(endpointId, &zeDMDDeviceFolderProp);
   msgApi->RegisterSetting(endpointId, &zeDMDDebugFolderProp);
   msgApi->RegisterSetting(endpointId, &zeDMDBrightnessFolderProp);
   msgApi->RegisterSetting(endpointId, &zeDMDWiFiEnabledProp);
   msgApi->RegisterSetting(endpointId, &zeDMDWiFiAddrFolderProp);
   msgApi->RegisterSetting(endpointId, &zeDMDSPIEnabledProp);
   msgApi->RegisterSetting(endpointId, &zeDMDSPISpeedProp);
   msgApi->RegisterSetting(endpointId, &zeDMDSPIFramePauseProp);
   msgApi->RegisterSetting(endpointId, &zeDMDSPIWidthProp);
   msgApi->RegisterSetting(endpointId, &zeDMDSPIHeightProp);
   msgApi->RegisterSetting(endpointId, &pixelcadeProp);
   msgApi->RegisterSetting(endpointId, &pixelcadeDeviceProp);
   msgApi->RegisterSetting(endpointId, &pin2dmdProp);
   msgApi->RegisterSetting(endpointId, &dmdServerFolderProp);
   msgApi->RegisterSetting(endpointId, &dmdServerAddrFolderProp);
   msgApi->RegisterSetting(endpointId, &dmdServerPortFolderProp);

   msgApi->RegisterSetting(endpointId, &findDisplaysProp);
   msgApi->RegisterSetting(endpointId, &dumpDMDTxtProp);
   msgApi->RegisterSetting(endpointId, &dumpDMDRawProp);
   msgApi->RegisterSetting(endpointId, &lumTintRProp);
   msgApi->RegisterSetting(endpointId, &lumTintGProp);
   msgApi->RegisterSetting(endpointId, &lumTintBProp);

   dmdSource = std::make_unique<CtrlItemConsumer<DisplaySrcId>>(
      msgApi, endpointId, CTLPI_DISPLAY_GET_SRC_MSG, CTLPI_DISPLAY_ON_SRC_CHG_MSG, [](std::vector<DisplaySrcId>& items) { SelectSource(items); }, []() { OnSourceChanged(); });
   dmdSource->SelectItems(true);
}

MSGPI_EXPORT void MSGPIAPI DMDUtilPluginUnload()
{
   dmdDispatcher = nullptr;
   dmdSource = nullptr;
   msgApi = nullptr;
}
