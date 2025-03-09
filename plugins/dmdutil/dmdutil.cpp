// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstdlib>
#include <cstring>

#include "VPXPlugin.h"
#include "CorePlugin.h"
#include "LoggingPlugin.h"
#include "DMDUtil/DMDUtil.h"

static MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;

static unsigned int onGameEndId;
static unsigned int onDmdSrcChangedId;
static unsigned int getDmdSrcMsgId;
static unsigned int getDmdMsgId;

static DMDUtil::DMD* pDmd = nullptr;
static int lastFrameID = 0;
static DmdSrcId m_defaultDmdId = {0};

static uint8_t tintR = 255;
static uint8_t tintG = 140;
static uint8_t tintB = 0;

LPI_USE();
#define LOGD LPI_LOGD
#define LOGI LPI_LOGI
#define LOGE LPI_LOGE

LPI_IMPLEMENT

void onUpdateDMD(void* userData)
{
   if (!pDmd)
      return;

   GetDmdMsg getMsg = { m_defaultDmdId, 0, nullptr };
   msgApi->BroadcastMsg( endpointId, getDmdMsgId, &getMsg);

   if (getMsg.frame && lastFrameID != getMsg.frameId) {
      lastFrameID = getMsg.frameId;

      switch(getMsg.dmdId.format) {
         case CTLPI_GETDMD_FORMAT_LUM8:
         {
            const uint8_t* const __restrict luminanceData = getMsg.frame;
            uint8_t* const __restrict rgb24Data = (uint8_t*)malloc(getMsg.dmdId.width * getMsg.dmdId.height * 3);

            for (int i = 0; i < getMsg.dmdId.width * getMsg.dmdId.height; ++i) {
                const uint32_t lum = luminanceData[i];
                rgb24Data[i * 3    ] = (uint8_t)((lum * tintR) / 255u);
                rgb24Data[i * 3 + 1] = (uint8_t)((lum * tintG) / 255u);
                rgb24Data[i * 3 + 2] = (uint8_t)((lum * tintB) / 255u);
            }

            pDmd->UpdateRGB24Data(rgb24Data, getMsg.dmdId.width, getMsg.dmdId.height);
            free(rgb24Data);
         }
         break;

         case CTLPI_GETDMD_FORMAT_SRGB888:
            pDmd->UpdateRGB24Data((const uint8_t*)getMsg.frame, getMsg.dmdId.width, getMsg.dmdId.height);
            break;

         case CTLPI_GETDMD_FORMAT_SRGB565:
            pDmd->UpdateRGB16Data((const uint16_t*)getMsg.frame, getMsg.dmdId.width, getMsg.dmdId.height);
            break;
      }
   }

   msgApi->RunOnMainThread(1. / 60., onUpdateDMD, nullptr);
}

void initDMD()
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

   onUpdateDMD(nullptr);
}

void onGameEnd(const unsigned int msgId, void* userData, void* msgData)
{
   delete pDmd;
   pDmd = nullptr;
}

void onDmdSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   DmdSrcId newDmdId = { 0 };

   GetDmdSrcMsg getSrcMsg = { 1024, 0, new DmdSrcId[1024] };
   msgApi->BroadcastMsg( endpointId, getDmdSrcMsgId, &getSrcMsg);

   bool foundDMD = false;

   for (unsigned int i = 0; i < getSrcMsg.count; i++) {
      if (getSrcMsg.entries[i].format != CTLPI_GETDMD_FORMAT_LUM8) {
          if (getSrcMsg.entries[i].width > newDmdId.width) {
              newDmdId = getSrcMsg.entries[i];
              foundDMD = true;
          }
      }
   }

   if (!foundDMD) {
      for (unsigned int i = 0; i < getSrcMsg.count; i++) {
         if (getSrcMsg.entries[i].format == CTLPI_GETDMD_FORMAT_LUM8) {
             if (getSrcMsg.entries[i].width > newDmdId.width) {
               newDmdId = getSrcMsg.entries[i];
               foundDMD = true;
            }
         }
      }
   }

   delete[] getSrcMsg.entries;

   m_defaultDmdId = newDmdId;

   if (foundDMD) {
      LOGI("DMD Source Changed: format=%d, width=%d, height=%d", newDmdId.format, newDmdId.width, newDmdId.height);

      if (!pDmd)
         initDMD();
   }
}

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;

   // Request and setup shared login API
   LPISetup(endpointId, msgApi);

   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDMD_SRC_CHG_MSG), onDmdSrcChanged, nullptr);

   getDmdSrcMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG);
   getDmdMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->UnsubscribeMsg(onDmdSrcChangedId, onDmdSrcChanged);

   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcMsgId);
   msgApi->ReleaseMsgID(getDmdMsgId);

   msgApi = nullptr;
}
