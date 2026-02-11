// license:GPLv3+

#include "core/stdafx.h"
#include "VPinballLib_iOS.h"
#include "VPinballLib.h"

#include <SDL3/SDL.h>

static void (*s_iosStartupHandler)(void*) = nullptr;
static void (*s_iosOpenURLHandler)(const char*) = nullptr;

extern "C" void VPinball_SetIOSStartupHandler(void (*handler)(void* window))
{
   s_iosStartupHandler = handler;
}

extern "C" void VPinball_SetIOSOpenURLHandler(void (*handler)(const char* url))
{
   s_iosOpenURLHandler = handler;
}

extern "C" void VPinball_CallIOSOpenURLHandler(const char* url)
{
   if (s_iosOpenURLHandler && url)
      s_iosOpenURLHandler(url);
}

namespace VPinballLib {

bool InitIOS(void* pWindow)
{
   if (!pWindow)
      return false;

   SDL_Window* pSDLWindow = static_cast<SDL_Window*>(pWindow);

   VPinballLib& lib = VPinballLib::Instance();

   void* pMetalLayer = SDL_GetRenderMetalLayer(SDL_CreateRenderer(pSDLWindow, "Metal"));
   if (pMetalLayer == nullptr)
      return false;

   lib.SetMetalLayer(pMetalLayer);

   if (s_iosStartupHandler) {
      SDL_PropertiesID props = SDL_GetWindowProperties(pSDLWindow);
      void* uiWindow = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
      s_iosStartupHandler(uiWindow);
   }

   return true;
}

}
