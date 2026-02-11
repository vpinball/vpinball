#import <Foundation/Foundation.h>

#include "../../../lib/src/VPinballLib_iOS.h"

extern void VPinball_IOSStartup(void* window);
extern void VPinball_IOSOpenURL(const char* url);

__attribute__((constructor))
static void vpinball_register_ios_handlers(void)
{
    VPinball_SetIOSStartupHandler(VPinball_IOSStartup);
    VPinball_SetIOSOpenURLHandler(VPinball_IOSOpenURL);
}
