#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void VPinball_SetIOSStartupHandler(void (*handler)(void* window));

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace VPinballLib {

bool InitIOS(void* pWindow);

}
#endif
