// license:GPLv3+

#include <cassert>
#include <cstdlib>
#include <chrono>
#include <cstdlib>
#include <cstring>

#include "MsgPlugin.h"
#include "CorePlugin.h"
#include "PinMamePlugin.h"
#include "DMDUtil/DMDUtil.h"

static MsgPluginAPI* msgApi = nullptr;
static uint32_t endpointId;

static unsigned int onGameStartId;
static unsigned int onGameEndId;
static unsigned int onDmdSrcChangedId;
static unsigned int getDmdSrcMsgId;
static unsigned int getDmdMsgId;

static DMDUtil::DMD* pDmd = nullptr;
static int lastFrameID = 0;
static DmdSrcId m_defaultDmdId = {0};

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
            uint8_t* luminanceData = getMsg.frame;
            uint8_t* rgb24Data = (uint8_t*)malloc(getMsg.dmdId.width * getMsg.dmdId.height * 3);

            uint8_t redTint = 255;
            uint8_t greenTint = 0;
            uint8_t blueTint = 0;

            for (int i = 0; i < getMsg.dmdId.width * getMsg.dmdId.height; ++i) {
                uint8_t lum = luminanceData[i];
                rgb24Data[i * 3] = (lum * redTint) / 255;
                rgb24Data[i * 3 + 1] = (lum * greenTint) / 255;
                rgb24Data[i * 3 + 2] = (lum * blueTint) / 255;
            }

            pDmd->UpdateRGB24Data(rgb24Data, getMsg.dmdId.width, getMsg.dmdId.height);
            free(rgb24Data);
         }
         break;

         case CTLPI_GETDMD_FORMAT_SRGB888:
            pDmd->UpdateRGB24Data(getMsg.frame, getMsg.dmdId.width, getMsg.dmdId.height);
         break;

         case CTLPI_GETDMD_FORMAT_SRGB565:
         {
             uint16_t* rgb565Data = (uint16_t*)getMsg.frame;
             uint8_t* rgb24Data = (uint8_t*)malloc(getMsg.dmdId.width * getMsg.dmdId.height * 3);

             for (int i = 0; i < getMsg.dmdId.width * getMsg.dmdId.height; ++i) {
                 uint16_t pixel = rgb565Data[i];

                 uint8_t red = ((pixel >> 11) & 0x1F) * 255 / 31;
                 uint8_t green = ((pixel >> 5) & 0x3F) * 255 / 63;
                 uint8_t blue = (pixel & 0x1F) * 255 / 31;

                 rgb24Data[i * 3] = red;
                 rgb24Data[i * 3 + 1] = green;
                 rgb24Data[i * 3 + 2] = blue;
             }

             pDmd->UpdateRGB24Data(rgb24Data, getMsg.dmdId.width, getMsg.dmdId.height);
             free(rgb24Data);
         }
         break;
      }
   }

   msgApi->RunOnMainThread(1. / 60., onUpdateDMD, nullptr);
}

void onGameStart(const unsigned int msgId, void* userData, void* msgData)
{
   pDmd = new DMDUtil::DMD();
   pDmd->FindDisplays();

   onUpdateDMD(nullptr);
}

void onGameEnd(const unsigned int msgId, void* userData, void* msgData)
{
   delete pDmd;
   pDmd = nullptr;
}

void onDmdSrcChanged(const unsigned int msgId, void* userData, void* msgData)
{
   GetDmdSrcMsg getSrcMsg = { 1024, 0, new DmdSrcId[1024] };
   msgApi->BroadcastMsg( endpointId, getDmdSrcMsgId, &getSrcMsg);

   m_defaultDmdId = getSrcMsg.entries[0];
}

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   
   msgApi->SubscribeMsg(endpointId, onGameStartId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(endpointId, onGameEndId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(endpointId, onDmdSrcChangedId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_ONDMD_SRC_CHG_MSG), onDmdSrcChanged, nullptr);

   getDmdSrcMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_SRC_MSG);
   getDmdMsgId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_GETDMD_RENDER_MSG);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->UnsubscribeMsg(onDmdSrcChangedId, onDmdSrcChanged);

   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onDmdSrcChangedId);
   msgApi->ReleaseMsgID(getDmdSrcMsgId);
   msgApi->ReleaseMsgID(getDmdMsgId);

   msgApi = nullptr;
}
