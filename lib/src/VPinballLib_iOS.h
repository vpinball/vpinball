// license:GPLv3+

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void VPinball_SetIOSStartupHandler(void (*handler)(void* window));
void VPinball_SetIOSOpenURLHandler(void (*handler)(const char* url));
void VPinball_CallIOSOpenURLHandler(const char* url);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace VPinballLib {

bool InitIOS(void* pWindow);

}
#endif
