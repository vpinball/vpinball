// license:GPLv3+

#define _ITERATOR_DEBUG_LEVEL 0 // to avoid conflicting with libSerum setup

#include <cassert>
#include <chrono>
#include <stdlib.h>

#include "MsgPlugin.h"
#include "CorePlugin.h"
#include "PinMamePlugin.h"
#include "libserum/serum-decode.h"

///////////////////////////////////////////////////////////////////////////////
// Serum Colorization plugin
//
// This plugin only rely on the generic message plugin API and the following
// messages:
// - PinMame/onGameStart: eventData is PinMame game identifier (rom name)
// - PinMame/onGameEnd
// - Controller/GetDMD: eventData is a request/response struct

MsgPluginAPI* msgApi = nullptr;
unsigned int getDmdId, onGameStartId, onGameEndId;

GetDmdMsg getRawDmdMsg;
Serum_Frame_Struc* pSerum;
uint8_t* colorFrame;
bool colorized;
bool ownColorFrame;
unsigned int lastRawFrameId;
unsigned int colorizedFrameFormat;
unsigned int colorizedframeId;
std::chrono::high_resolution_clock::time_point animationTick, animationNextTick;

void updateFrame()
{
   const unsigned int size = getRawDmdMsg.width * getRawDmdMsg.height;
   if (pSerum->SerumVersion == SERUM_V1)
   {
      if (colorFrame == nullptr)
      {
         ownColorFrame = true;
         colorFrame = static_cast<uint8_t*>(malloc(size * 3));
      }
      for (unsigned int i = 0; i < size; i++)
         memcpy(&colorFrame[i * 3], &pSerum->palette[pSerum->frame[i] * 3], 3);
      colorizedFrameFormat = CTLPI_GETDMD_FORMAT_SRGB888;
      colorizedframeId++;
   }
   else if ((pSerum->SerumVersion == SERUM_V2) && (getRawDmdMsg.width == pSerum->width32) && (getRawDmdMsg.height == 32))
   {
      colorFrame = reinterpret_cast<uint8_t*>(pSerum->frame32);
      colorizedFrameFormat = CTLPI_GETDMD_FORMAT_SRGB565;
      colorizedframeId++;
   }
   else if ((pSerum->SerumVersion == SERUM_V2) && (getRawDmdMsg.width == pSerum->width64) && (getRawDmdMsg.height == 64))
   {
      colorFrame = reinterpret_cast<uint8_t*>(pSerum->frame64);
      colorizedFrameFormat = CTLPI_GETDMD_FORMAT_SRGB565;
      colorizedframeId++;
   }
}

void onGetDMD(const unsigned int eventId, void* userData, void* eventData)
{
   assert(pSerum);
   
   // Only process render request on main DMD
   GetDmdMsg *getRenderDmdMsg = static_cast<GetDmdMsg*>(eventData);
   if (((getRenderDmdMsg->requestFlags & CTLPI_GETDMD_RENDER_FRAME) == 0) || (getRenderDmdMsg->dmdId != -1))
      return;

   // Request raw frame and try to colorize it
   getRawDmdMsg.frame = nullptr;
   msgApi->BroadcastMsg(getDmdId, &getRawDmdMsg);
   if (getRawDmdMsg.frame == nullptr)
      return;
   uint32_t firstrot = IDENTIFY_NO_FRAME;
   if (getRawDmdMsg.frameId != lastRawFrameId)
   {
      firstrot = Serum_Colorize(getRawDmdMsg.frame);
      lastRawFrameId = getRawDmdMsg.frameId;
   }
   
   // Update frame, eventually applying animation
   auto now = std::chrono::high_resolution_clock::now();
   if (firstrot != IDENTIFY_NO_FRAME)
   { // New frame, eventually starting a new animation
      colorized = true;
      animationTick = now;
      animationNextTick = animationTick + std::chrono::milliseconds(firstrot);
      updateFrame();
   }
   else if (colorized)
   { // Perform current animation (catch up with current timing)
      while (animationNextTick < now)
      {
         animationTick = animationNextTick;
         uint32_t nextrot = Serum_Rotate();
         if (((pSerum->SerumVersion == SERUM_V1) && (nextrot & FLAG_RETURNED_V1_ROTATED))
          || ((pSerum->SerumVersion == SERUM_V2) && (nextrot & FLAG_RETURNED_V2_ROTATED32) && (pSerum->width32 > 0))
          || ((pSerum->SerumVersion == SERUM_V2) && (nextrot & FLAG_RETURNED_V2_ROTATED64) && (pSerum->width64 > 0)))
         {
            animationNextTick = animationTick + std::chrono::milliseconds(nextrot & 0x0000ffff);
            updateFrame();
         }
         else
         {
            animationNextTick = now;
         }
      }
   }
   
   // Answer to requester with last colorized frame
   if (colorized)
   {
      getRenderDmdMsg->frameId = colorizedframeId;
      getRenderDmdMsg->format = colorizedFrameFormat;
      getRenderDmdMsg->frame = colorFrame;
      getRenderDmdMsg->width = getRawDmdMsg.width;
      getRenderDmdMsg->height = getRawDmdMsg.height;
   }
 }

void onGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   const char* gameId = static_cast<const char*>(eventData);
   assert(gameId != nullptr);
   char crzFolder[512];
   msgApi->GetSetting("serum.dmd", "crz_folder", crzFolder, sizeof(crzFolder));
   pSerum = Serum_Load(crzFolder, gameId, FLAG_REQUEST_32P_FRAMES | FLAG_REQUEST_64P_FRAMES);
   if (pSerum)
   {
      colorized = false;
      ownColorFrame = false;
      colorizedframeId = 0;
      msgApi->SubscribeMsg(getDmdId, onGetDMD, nullptr);
   }
}

void onGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   if (pSerum)
   {
      if (ownColorFrame)
         free(colorFrame);
      msgApi->UnsubscribeMsg(getDmdId, onGetDMD);
      Serum_Dispose();
   }
}

MSGPI_EXPORT void PluginLoad(MsgPluginAPI* api)
{
   msgApi = api;
   getDmdId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_MSG_GET_DMD);
   getRawDmdMsg.dmdId = -1;
   getRawDmdMsg.requestFlags = CTLPI_GETDMD_IDENTIFY_FRAME;
   // Subscribe to PinMame events (which is the only supported controller for the time being)
   msgApi->SubscribeMsg(onGameStartId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(onGameEndId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_END), onGameEnd, nullptr);
}

MSGPI_EXPORT void PluginUnload()
{
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(getDmdId);
   msgApi = nullptr;
}
