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
unsigned int endpointId, getDmdId, onGameStartId, onGameEndId, onDmdTrigger;

Serum_Frame_Struc* pSerum;
unsigned int lastRawFrameId;

class ColorizationState
{
public:
   ColorizationState(unsigned int width, unsigned int height)
      : m_width(width)
      , m_height(height)
      , m_size(width * height)
      , m_buffer(new uint8_t[width * height])
   {
      assert(m_width > 0);
      assert(m_height > 0);
      assert(m_buffer);
   }

   ~ColorizationState()
   {
      delete[] m_buffer;
   }

   void UpdateFrame()
   {
      assert(pSerum);
      if (pSerum->SerumVersion == SERUM_V1)
      {
         for (unsigned int i = 0; i < m_size; i++)
            memcpy(&(m_buffer[i * 3]), &pSerum->palette[pSerum->frame[i] * 3], 3);
         m_colorFrame = m_buffer;
         m_colorizedFrameFormat = CTLPI_GETDMD_FORMAT_SRGB888;
         m_colorizedframeId++;
      }
      else if ((pSerum->SerumVersion == SERUM_V2) && (m_width == pSerum->width32) && (m_height == 32))
      {
         m_colorFrame = reinterpret_cast<uint8_t*>(pSerum->frame32);
         m_colorizedFrameFormat = CTLPI_GETDMD_FORMAT_SRGB565;
         m_colorizedframeId++;
      }
      else if ((pSerum->SerumVersion == SERUM_V2) && (m_width == pSerum->width64) && (m_height == 64))
      {
         m_colorFrame = reinterpret_cast<uint8_t*>(pSerum->frame64);
         m_colorizedFrameFormat = CTLPI_GETDMD_FORMAT_SRGB565;
         m_colorizedframeId++;
      }
   }

   uint8_t* m_colorFrame = nullptr;
   unsigned int m_colorizedFrameFormat = 0;
   unsigned int m_colorizedframeId = 0;
   bool m_hasAnimation = false;
   
   std::chrono::high_resolution_clock::time_point m_animationTick;
   std::chrono::high_resolution_clock::time_point m_animationNextTick;

   uint8_t* const m_buffer;
   const unsigned int m_width, m_height, m_size;
};

ColorizationState* state = nullptr;



void onGetDMD(const unsigned int eventId, void* userData, void* eventData)
{
   assert(pSerum);
   GetDmdMsg* const getDmdMsg = static_cast<GetDmdMsg*>(eventData);

   // Only process main DMD
   if (getDmdMsg->dmdId != -1)
      return;
   
   // We received a raw frame to identify/colorize (eventually requested by us)
   if ((getDmdMsg->requestFlags & CTLPI_GETDMD_IDENTIFY_FRAME) && (getDmdMsg->frame != nullptr) && (getDmdMsg->frameId != lastRawFrameId))
   {
      const uint32_t firstrot = Serum_Colorize(getDmdMsg->frame);
      lastRawFrameId = getDmdMsg->frameId;
      if (pSerum->triggerID != 0xffffffff)
         msgApi->BroadcastMsg(endpointId, onDmdTrigger, &pSerum->triggerID);
      if (firstrot != IDENTIFY_NO_FRAME)
      { // New frame, eventually starting a new animation
         if (state == nullptr)
            state = new ColorizationState(getDmdMsg->width, getDmdMsg->height);
         else if (state->m_width != getDmdMsg->width || state->m_height != getDmdMsg->height)
         {
            delete state;
            state = new ColorizationState(getDmdMsg->width, getDmdMsg->height);
         }
         state->m_hasAnimation = firstrot != 0;
         if (state->m_hasAnimation)
         {
            state->m_animationTick = std::chrono::high_resolution_clock::now();
            state->m_animationNextTick = state->m_animationTick + std::chrono::milliseconds(firstrot);
         }
         state->UpdateFrame();
      }
      return;
   }

   // Someone requested a DMD frame to render, and no one has colorized it yet
   if ((getDmdMsg->requestFlags & CTLPI_GETDMD_RENDER_FRAME) && ((getDmdMsg->frame == nullptr) || (getDmdMsg->format == CTLPI_GETDMD_FORMAT_LUM8)))
   {
      // Request the last raw frame
      GetDmdMsg getRawDmdMsg;
      memset(&getRawDmdMsg, 0, sizeof(getRawDmdMsg));
      getRawDmdMsg.dmdId = -1;
      getRawDmdMsg.requestFlags = CTLPI_GETDMD_IDENTIFY_FRAME;
      msgApi->BroadcastMsg(endpointId, getDmdId, &getRawDmdMsg);
      onGetDMD(getDmdId, nullptr, static_cast<void*>(&getRawDmdMsg));

      if (state)
      {
         // Perform current animation (catch up with current timing)
         if (state->m_hasAnimation)
         {
            const auto now = std::chrono::high_resolution_clock::now();
            while (state->m_animationNextTick < now)
            {
               const uint32_t nextrot = Serum_Rotate();
               const uint32_t delayMs = nextrot & 0x0000ffff;
               if (delayMs == 0)
               {
                  state->m_hasAnimation = false;
                  break;
               }
               state->m_animationTick = state->m_animationNextTick;
               state->m_animationNextTick = state->m_animationNextTick + std::chrono::milliseconds(delayMs);
               if (((pSerum->SerumVersion == SERUM_V1) && (nextrot & FLAG_RETURNED_V1_ROTATED))
                  || ((pSerum->SerumVersion == SERUM_V2) && (nextrot & FLAG_RETURNED_V2_ROTATED32) && (pSerum->width32 > 0))
                  || ((pSerum->SerumVersion == SERUM_V2) && (nextrot & FLAG_RETURNED_V2_ROTATED64) && (pSerum->width64 > 0)))
               {
                  state->UpdateFrame();
               }
            }
         }

         // Answer to requester with last colorized frame
         getDmdMsg->frameId = state->m_colorizedframeId;
         getDmdMsg->format = state->m_colorizedFrameFormat;
         getDmdMsg->frame = state->m_colorFrame;
         getDmdMsg->width = state->m_width;
         getDmdMsg->height = state->m_height;
      }
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
      msgApi->SubscribeMsg(endpointId, getDmdId, onGetDMD, nullptr);
}

void onGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   if (pSerum)
   {
      delete state;
      state = nullptr;
      msgApi->UnsubscribeMsg(getDmdId, onGetDMD);
      Serum_Dispose();
   }
}

MSGPI_EXPORT void PluginLoad(const unsigned int sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   endpointId = sessionId;
   getDmdId = msgApi->GetMsgID(CTLPI_NAMESPACE, CTLPI_MSG_GET_DMD);
   onDmdTrigger = msgApi->GetMsgID("Serum", "OnDmdTrigger");
   msgApi->SubscribeMsg(sessionId, onGameStartId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(sessionId, onGameEndId = msgApi->GetMsgID(PMPI_NAMESPACE, PMPI_EVT_ON_GAME_END), onGameEnd, nullptr);
}

MSGPI_EXPORT void PluginUnload()
{
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onDmdTrigger);
   msgApi->ReleaseMsgID(getDmdId);
   msgApi = nullptr;
}
