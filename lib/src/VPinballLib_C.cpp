#include <core/stdafx.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include "../include/vpinball/VPinballLib_C.h"
#include "VPinballLib.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
   return VPinballLib::VPinballLib::Instance().AppInit(argc, argv) ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
   VPinballLib::VPinballLib::Instance().AppIterate();
   return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
   VPinballLib::VPinballLib::Instance().AppEvent(event);
   return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

VPINBALLAPI const char* VPinballGetVersionStringFull()
{
   thread_local string version;
   version = VPinballLib::VPinballLib::Instance().GetVersionStringFull();
   return version.c_str();
}

VPINBALLAPI void VPinballInit(VPinballEventCallback callback)
{
   VPinballLib::VPinballLib::Instance().Init(callback);
}

VPINBALLAPI void VPinballLog(VPINBALL_LOG_LEVEL level, const char* pMessage)
{
   if (pMessage != nullptr)
      VPinballLib::VPinballLib::Instance().Log(level, pMessage);
}

VPINBALLAPI void VPinballResetLog()
{
   VPinballLib::VPinballLib::Instance().ResetLog();
}

VPINBALLAPI int VPinballLoadValueInt(const char* pSectionName, const char* pKey, int defaultValue)
{
    if (pSectionName == nullptr || pKey == nullptr)
      return defaultValue;

   return VPinballLib::VPinballLib::Instance().LoadValueInt(pSectionName, pKey, defaultValue);
}

VPINBALLAPI float VPinballLoadValueFloat(const char* pSectionName, const char* pKey, float defaultValue)
{
   if (pSectionName == nullptr || pKey == nullptr)
      return defaultValue;

   return VPinballLib::VPinballLib::Instance().LoadValueFloat(pSectionName, pKey, defaultValue);
}

VPINBALLAPI const char* VPinballLoadValueString(const char* pSectionName, const char* pKey, const char* pDefaultValue)
{
   if (pSectionName == nullptr || pKey == nullptr)
      return pDefaultValue;

   thread_local string value;
   value = VPinballLib::VPinballLib::Instance().LoadValueString(pSectionName, pKey, pDefaultValue);
   return value.c_str();
}

VPINBALLAPI void VPinballSaveValueInt(const char* pSectionName, const char* pKey, int value)
{
   if (pSectionName == nullptr || pKey == nullptr)
      return;

   VPinballLib::VPinballLib::Instance().SaveValueInt(pSectionName, pKey, value);
}

VPINBALLAPI void VPinballSaveValueFloat(const char* pSectionName, const char* pKey, float value)
{
   if (pSectionName == nullptr || pKey == nullptr)
      return;

   VPinballLib::VPinballLib::Instance().SaveValueFloat(pSectionName, pKey, value);
}

VPINBALLAPI void VPinballSaveValueString(const char* pSectionName, const char* pKey, const char* pValue)
{
   if (pSectionName == nullptr || pKey == nullptr || pValue == nullptr)
      return;

   VPinballLib::VPinballLib::Instance().SaveValueString(pSectionName, pKey, pValue);
}

VPINBALLAPI VPINBALL_STATUS VPinballResetIni()
{
   return VPinballLib::VPinballLib::Instance().ResetIni();
}

VPINBALLAPI void VPinballUpdateWebServer()
{
   VPinballLib::VPinballLib::Instance().UpdateWebServer();
}

VPINBALLAPI VPINBALL_STATUS VPinballLoadTable(const char* pPath)
{
   if (pPath == nullptr)
      return VPINBALL_STATUS_FAILURE;

   return VPinballLib::VPinballLib::Instance().LoadTable(pPath);
}

VPINBALLAPI VPINBALL_STATUS VPinballExtractTableScript()
{
   return VPinballLib::VPinballLib::Instance().ExtractTableScript();
}

VPINBALLAPI VPINBALL_STATUS VPinballPlay()
{
   return VPinballLib::VPinballLib::Instance().Play();
}

VPINBALLAPI VPINBALL_STATUS VPinballStop()
{
   return VPinballLib::VPinballLib::Instance().Stop();
}
