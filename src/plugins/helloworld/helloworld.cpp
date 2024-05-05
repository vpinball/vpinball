#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "VPXPlugin.h"

VPXPluginAPI* vpx = nullptr;


extern "C" __declspec(dllexport) void onGameStart(const unsigned int eventId, void* data)
{
   // Game is starting (plugin can be loaded and kept alive through multiple game plays)
   // After this event, all functions of the API marked as 'in game only' can be called
}

extern "C" __declspec(dllexport) void onGameEnd(const unsigned int eventId, void* data)
{
   // Game is ending
   // After this event, all functions of the API marked as 'in game only' may not be called anymore
}

extern "C" __declspec(dllexport) void onPrepareFrame(const unsigned int eventId, void* data)
{
   // Called when the player is about to prepare a new frame
   // This can be used ot tweak any visual parameter before building the frame (for example head tracking,...)
}


extern "C" __declspec(dllexport) void PluginLoad(VPXPluginAPI * api)
{
   vpx = api;
   vpx->SubscribeEvent(vpx->GetEventID(VPX_EVT_ON_GAME_START), onGameStart);
   vpx->SubscribeEvent(vpx->GetEventID(VPX_EVT_ON_GAME_END), onGameEnd);
   vpx->SubscribeEvent(vpx->GetEventID(VPX_EVT_ON_PREPARE_FRAME), onPrepareFrame);
}

extern "C" __declspec(dllexport) void PluginUnload()
{
   // Cleanup is mandatory when plugin is unloaded. All registered callbacks must be unregistered.
   vpx->UnsubscribeEvent(vpx->GetEventID(VPX_EVT_ON_GAME_START), onGameStart);
   vpx->UnsubscribeEvent(vpx->GetEventID(VPX_EVT_ON_GAME_END), onGameEnd);
   vpx->UnsubscribeEvent(vpx->GetEventID(VPX_EVT_ON_PREPARE_FRAME), onPrepareFrame);
   vpx = nullptr;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
   switch (ul_reason_for_call)
   {
   case DLL_PROCESS_ATTACH:
   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   case DLL_PROCESS_DETACH: break;
   }
   return TRUE;
}