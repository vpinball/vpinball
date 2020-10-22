////////////////////////////////////////////////////////////////////////
// Header file used for WinCE (Windows Embedded Compact)
//
// WinCE is optimized for devices that have minimal memory. A Windows CE
// kernel may run with one megabyte of memory. Devices which use WinCE
// include the Pocket PC, industrial devices and embedded systems.
//

#pragma once

#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcd.lib")


//#include <ceconfig.h>
#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
  #define SHELL_AYGSHELL
#endif

#ifdef _CE_DCOM
  #define _ATL_APARTMENT_THREADED
#endif

#if defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP)
  #ifndef _DEVICE_RESOLUTION_AWARE
    #define _DEVICE_RESOLUTION_AWARE
  #endif
#endif


#if _WIN32_WCE == 420 || _WIN32_WCE == 0x420
  // For Pocket PC 2003
  #pragma comment(lib, "ccrtrtti.lib")
#endif

#if _MSC_VER >= 1300

  // NOTE - this value is not strongly correlated to the Windows CE OS version being targeted
  #undef  WINVER
  #define WINVER _WIN32_WCE

  #ifdef _DEVICE_RESOLUTION_AWARE
    #include "DeviceResolutionAware.h"
  #endif

  #if _WIN32_WCE < 0x500 && ( defined(WIN32_PLATFORM_PSPC) || defined(WIN32_PLATFORM_WFSP) )
    #ifdef _X86_
      #ifdef _DEBUG
        #pragma comment(lib, "libcmtx86d.lib")
      #else
        #pragma comment(lib, "libcmtx86.lib")
      #endif
    #endif
  #endif

  #if _WIN32_WCE < 0x500
    #include <altcecrt.h>
  #endif

#endif// _MSC_VER >= 1300

#ifdef SHELL_AYGSHELL
  #include <aygshell.h>
  #pragma comment(lib, "aygshell.lib")
#endif // SHELL_AYGSHELL


