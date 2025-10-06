#import <Foundation/Foundation.h>

#include "../../../lib/src/VPinballLib_iOS.h"

extern void VPinball_IOSStartup(void* window);

__attribute__((constructor))
static void vpinball_register_ios_startup(void)
{
    VPinball_SetIOSStartupHandler(VPinball_IOSStartup);
}
