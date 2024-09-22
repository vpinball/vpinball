// license:GPLv3+

#include "MsgPlugin.h"
#include "VPXPlugin.h"

MsgPluginAPI* msgApi = nullptr;
VPXPluginAPI* vpxApi = nullptr;

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

MSGPI_EXPORT void PluginLoad(MsgPluginAPI* api)
{
   msgApi = api;
   msgApi->BroadcastMsg(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_MSG_GET_API), &vpxApi);
   msgApi->SubscribeMsg(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), onGameStart, nullptr);
   msgApi->SubscribeMsg(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd, nullptr);
   msgApi->SubscribeMsg(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME), onPrepareFrame, nullptr);
}

MSGPI_EXPORT void PluginUnload()
{
   // Cleanup is mandatory when plugin is unloaded. All registered callbacks must be unregistered.
   msgApi->UnsubscribeMsg(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_START), onGameStart);
   msgApi->UnsubscribeMsg(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_GAME_END), onGameEnd);
   msgApi->UnsubscribeMsg(msgApi->GetMsgID(VPXPI_NAMESPACE, VPXPI_EVT_ON_PREPARE_FRAME), onPrepareFrame);
   vpxApi = nullptr;
   msgApi = nullptr;
}
