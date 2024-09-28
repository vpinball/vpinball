// license:GPLv3+

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <cassert>
#include <stdint.h>
#include <tchar.h>
#include <string>
#include <chrono>

#include "MsgPlugin.h"
#include "CorePlugin.h"
#include "PinMamePlugin.h"

///////////////////////////////////////////////////////////////////////////////
// PinUp Player DMD events plugin
//
// Forward DMD changes to dmddevicePUP64/dmddevicePUP.dll (which is closed source,
// windows only).
//
// This plugin requires dmddevicePUP64/dmddevicePUP.dll to be placed in its
// directory. Since these dll are closed source, they are not distributed
// with the plugin and must be obtained from PinUp distribution separately.
//
// For some strange reason, PinUp does not do its matching on the PinMame DMD but on a 
// stretched to fit 128x32 frame, shaded using 0xFF4500 color (C# Colors.OrangeRed), 
// with non linear shading for 2 bitplane frames.

MsgPluginAPI* msgApi = nullptr;
unsigned int endpointId, getDmdId, onGameStartId, onGameEndId, onSerumTriggerId;

HMODULE dmdDevicePupDll = nullptr;
unsigned int lastFrameId = 0;
std::chrono::high_resolution_clock::time_point lastFrameTick;

typedef int (*pup_open)();
typedef void (*pup_setGameName)(const char* cName, int len);
typedef void (*pup_trigger)(unsigned short triggerId);
typedef void (*pup_renderRGB24)(unsigned short width, unsigned short height, uint8_t* frame);
typedef int (*pup_close)();

pup_open pupOpen;
pup_setGameName pupSetGameName;
pup_trigger pupTrigger;
pup_renderRGB24 pupRender_RGB24;
pup_close pupClose;

uint8_t rgbFrame[128 * 32 * 3];
uint8_t palette4[4 * 3];
uint8_t palette16[16 * 3];

void processDMD(const GetDmdMsg& getDmdMsg, std::chrono::high_resolution_clock::time_point now)
{
   if (getDmdMsg.frame == nullptr)
      return;

   lastFrameTick = now;
   if (getDmdMsg.frameId == lastFrameId)
      return;

   uint8_t* palette;
   if (getDmdMsg.format == CTLPI_GETDMD_FORMAT_BITPLANE2)
      palette = palette4;
   else if (getDmdMsg.format == CTLPI_GETDMD_FORMAT_BITPLANE4)
      palette = palette16;
   else
      return;

   lastFrameId = getDmdMsg.frameId;
   if (getDmdMsg.width == 128 && getDmdMsg.height == 32)
   {
      for (unsigned int i = 0; i < 128 * 32; i++)
         memcpy(&rgbFrame[i * 3], &palette[getDmdMsg.frame[i] * 3], 3);
   }
   else if (getDmdMsg.width == 128 && getDmdMsg.height < 32)
   {
      const int ofsY = ((32 - getDmdMsg.height) / 2) * 128;
      for (unsigned int i = 0; i < 128 * 16; i++)
         memcpy(&rgbFrame[(ofsY + i) * 3], &palette[getDmdMsg.frame[i] * 3], 3);
   }
   else if (getDmdMsg.width <= 256 && getDmdMsg.height == 64)
   {
      // Resize with a triangle filter to mimic what original implementation in Freezy's DmdExt (https://github.com/freezy/dmd-extensions)
      // does, that is to say:
      // - convert from luminance to RGB (with hue = 0, sat = 1)
      // - resize using Windows 8.1 API which in turn uses IWICBitmapScaler with Fant interpolation mode (hence the triangle filter)
      // - convert back from RGB to HSL and send luminance to PinUp
      // 
      // Some references regarding Fant scaling:
      // - https://github.com/sarnold/urt/blob/master/tools/fant.c
      // - https://photosauce.net/blog/post/examining-iwicbitmapscaler-and-the-wicbitmapinterpolationmode-values
      // 
      // The Baywatch Pup pack was used to validate this (the filter is still a guess since Windows code is not available)
      const unsigned int ofsX = (128 - (getDmdMsg.width / 2)) / 2;
      for (unsigned int y = 0; y < 32; y++)
      {
         for (unsigned int x = 0; x < getDmdMsg.width / 2; x++)
         {
            float lum = 0., sum = 0.;
            constexpr int radius = 1;
            for (int dx = 1-radius; dx <= radius; dx++)
            {
               for (int dy = 1-radius; dy <= radius; dy++)
               {
                  const int px = x * 2 + dx;
                  const int py = y * 2 + dy;
                  float weight = radius * radius - fabs(dx - 0.5f) * fabs(dy - 0.5f);
                  if (px >= 0 && px < getDmdMsg.width && py >= 0 && py < getDmdMsg.height)
                     lum += getDmdMsg.frame[py * getDmdMsg.width + px] * weight;
                  sum += weight;
               }
            }
            const int l = (int)round(lum / sum);
            memcpy(&rgbFrame[(y * 128 + x + ofsX) * 3], &palette[l * 3], 3);
         }
      }
   }
   else
   {
      // Unsupported DMD format (would need to implement a dedicated stretch fit, matching what is used elsewhere)
      return;
   }

   pupRender_RGB24(128, 32, rgbFrame);
}

// Called every 1/60s to process at least one frame every 16ms
void onUpdateDMD(void* userData)
{
   if (dmdDevicePupDll == nullptr)
      return;
   auto now = std::chrono::high_resolution_clock::now();
   if (now - lastFrameTick >= std::chrono::milliseconds(16))
   {
      GetDmdMsg getDmdMsg;
      memset(&getDmdMsg, 0, sizeof(getDmdMsg));
      getDmdMsg.requestFlags = CTLPI_GETDMD_IDENTIFY_FRAME;
      getDmdMsg.dmdId = -1;
      msgApi->BroadcastMsg(endpointId, getDmdId, &getDmdMsg);
      processDMD(getDmdMsg, now);
   }
   msgApi->RunOnMainThread(1. / 60., onUpdateDMD, nullptr);
}

// Catch raw frame broadcasted for other uses
void onGetDMD(const unsigned int eventId, void* userData, void* eventData)
{
   GetDmdMsg* getDmdMsg = static_cast<GetDmdMsg*>(eventData);
   if ((getDmdMsg->requestFlags & CTLPI_GETDMD_IDENTIFY_FRAME) && (getDmdMsg->dmdId == -1))
      processDMD(*getDmdMsg, std::chrono::high_resolution_clock::now());
}

// Broadcasted by Serum plugin when frame triggers are identified
void onSerumTrigger(const unsigned int eventId, void* userData, void* eventData)
{
   unsigned int* trigger = static_cast<unsigned int*>(eventData);
   if (dmdDevicePupDll && pupTrigger)
      pupTrigger(*trigger);
}

void onGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   const char* gameId = static_cast<const char*>(eventData);
   assert(gameId != nullptr);
   TCHAR path[MAX_PATH];
   HMODULE hm = NULL;
   if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, _T("PluginLoad"), &hm) == 0)
      return;
   if (GetModuleFileName(hm, path, MAX_PATH) == 0)
      return;
   std::wstring fullpath(path);
   fullpath = fullpath.substr(0, fullpath.find_last_of(_T("\\/"))) + _T('\\');
   #if (INTPTR_MAX == INT32_MAX)
      fullpath += _T("dmddevicePUP.DLL");
   #else
      fullpath += _T("dmddevicePUP64.DLL");
   #endif
   dmdDevicePupDll = LoadLibraryEx(fullpath.c_str(), NULL, LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
   if (dmdDevicePupDll == nullptr)
      return;
   pupOpen = reinterpret_cast<pup_open>(GetProcAddress(dmdDevicePupDll, "Open"));
   pupSetGameName = reinterpret_cast<pup_setGameName>(GetProcAddress(dmdDevicePupDll, "SetGameName"));
   pupTrigger = reinterpret_cast<pup_trigger>(GetProcAddress(dmdDevicePupDll, "PuP_Trigger"));
   pupRender_RGB24 = reinterpret_cast<pup_renderRGB24>(GetProcAddress(dmdDevicePupDll, "Render_RGB24"));
   pupClose = reinterpret_cast<pup_close>(GetProcAddress(dmdDevicePupDll, "Close"));
   if ((pupOpen == nullptr) || (pupSetGameName == nullptr) || (pupRender_RGB24 == nullptr) || (pupClose == nullptr))
   {
      FreeLibrary(dmdDevicePupDll);
      dmdDevicePupDll = nullptr;
      return;
   }
   pupOpen();
   pupSetGameName(gameId, static_cast<int>(strlen(gameId)));
   lastFrameId = 123456;
   memset(rgbFrame, 0, sizeof(rgbFrame));
   msgApi->SubscribeMsg(endpointId, getDmdId, onGetDMD, nullptr);
   onUpdateDMD(nullptr);
}

void onGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   if (dmdDevicePupDll)
   {
      pupClose();
      FreeLibrary(dmdDevicePupDll);
      dmdDevicePupDll = nullptr;
      msgApi->UnsubscribeMsg(getDmdId, onGetDMD);
   }
}

MSGPI_EXPORT void PluginLoad(const unsigned int sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId; 
   getDmdId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_MSG_GET_DMD);
   msgApi->SubscribeMsg(sessionId, onGameStartId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(sessionId, onGameEndId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(sessionId, onSerumTriggerId = msgApi->GetMsgID("Serum", "OnDmdTrigger"), onSerumTrigger, nullptr);
   dmdDevicePupDll = nullptr;
   unsigned int mapping4[] = { 0, 1, 4, 15 };
   for (int i = 0; i < 4; i++)
   {
      palette4[i * 3 + 0] = (mapping4[i] * 0xFF) / 0xF; // R
      palette4[i * 3 + 1] = (mapping4[i] * 0x45) / 0xF; // G
      palette4[i * 3 + 2] = (mapping4[i] * 0x00) / 0xF; // B
   }
   for (int i = 0; i < 16; i++)
   {
      palette16[i * 3 + 0] = (i * 0xFF) / 0xF; // R
      palette16[i * 3 + 1] = (i * 0x45) / 0xF; // G
      palette16[i * 3 + 2] = (i * 0x00) / 0xF; // B
   }
}

MSGPI_EXPORT void PluginUnload()
{
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onSerumTriggerId);
   msgApi->ReleaseMsgID(getDmdId);
   msgApi = nullptr;
}
