// license:GPLv3+

#include "MsgPlugin.h"
#include "VPXPlugin.h"

MsgPluginAPI* msgApi = nullptr;
VPXPluginAPI* vpxApi = nullptr;

uint32_t endpointId;
unsigned int getVpxApiId, onGameStartId, onGameEndId, onPrepareFrameId;

void onGameStart(const unsigned int eventId, void* userData, void* eventData)
{
   // Game is starting (plugin can be loaded and kept alive through multiple game plays)
   // After this event, all functions of the API marked as 'in game only' can be called
   if (vpxApi)
      vpxApi->PushNotification("Hello World", 5000);
}

void onGameEnd(const unsigned int eventId, void* userData, void* eventData)
{
   // Game is ending
   // After this event, all functions of the API marked as 'in game only' may not be called anymore
}

void onPrepareFrame(const unsigned int eventId, void* userData, void* eventData)
{
   // Called when the player is about to prepare a new frame
   // This can be used to tweak any visual parameter before building the frame (for example head tracking,...)
}

MSGPI_EXPORT void MSGPIAPI PluginLoad(const uint32_t sessionId, MsgPluginAPI* api)
{
   msgApi = api;
   msgApi->BroadcastMsg(sessionId, getVpxApiId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);
   msgApi->SubscribeMsg(sessionId, onGameStartId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(sessionId, onGameEndId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(sessionId, onPrepareFrameId = msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME), onPrepareFrame, nullptr);
}

MSGPI_EXPORT void MSGPIAPI PluginUnload()
{
   // Cleanup is mandatory when plugin is unloaded. All registered callbacks must be unregistered.
   msgApi->UnsubscribeMsg(onGameStartId, onGameStart);
   msgApi->UnsubscribeMsg(onGameEndId, onGameEnd);
   msgApi->UnsubscribeMsg(onPrepareFrameId, onPrepareFrame);
   msgApi->ReleaseMsgID(getVpxApiId);
   msgApi->ReleaseMsgID(onGameStartId);
   msgApi->ReleaseMsgID(onGameEndId);
   msgApi->ReleaseMsgID(onPrepareFrameId);
   vpxApi = nullptr;
   msgApi = nullptr;
}
