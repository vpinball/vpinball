// license:GPLv3+

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
   #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
   #define NOMINMAX
#endif

#ifdef __STANDALONE__
   #define __WINE_WINCON_H
#endif

#include <windows.h>

#ifdef __STANDALONE__
   #ifdef GetClassInfo
   #undef GetClassInfo
   #endif
#endif

#ifndef __STANDALONE__
   #define RPC_NO_WINDOWS_H
   #define COM_NO_WINDOWS_H
   #include <objbase.h>
#endif

#if defined(ENABLE_DX9)
 #ifdef _DEBUG
  #define D3D_DEBUG_INFO
 #endif
 #include "minid3d9.h"
#endif

#include <atlbase.h>

#ifndef __STANDALONE__
   #include <atlctl.h>
#endif

#ifdef _MSC_VER
   #define PATH_SEPARATOR_CHAR '\\'
   #define PATH_SEPARATOR_WCHAR L'\\'
#else
   #define PATH_SEPARATOR_CHAR '/'
   #define PATH_SEPARATOR_WCHAR L'/'
#endif

#include <oleauto.h>

#ifndef __STANDALONE__
   #include <intrin.h>
#endif
#if defined(_M_IX86) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(__SSE2__) || defined(_M_X64) || defined(_M_AMD64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
   #define ENABLE_SSE_OPTIMIZATIONS
   #include <xmmintrin.h>
#elif (defined(_M_ARM) || defined(_M_ARM64) || defined(__arm__) || defined(__arm64__) || defined(__aarch64__)) && (!defined(__ARM_ARCH) || __ARM_ARCH >= 7) && (!defined(_MSC_VER) || defined(__clang__)) //!! disable sse2neon if MSVC&non-clang
   #define ENABLE_SSE_OPTIMIZATIONS
   #include "sse2neon.h"
#endif
#ifdef __SSSE3__
 #include <tmmintrin.h>
#endif

#include <vector>
using std::vector;

#include <format>
#include <algorithm>

#include <string>
using namespace std::string_literals;
using namespace std::string_view_literals;
using std::string;
using std::wstring;


#ifndef __STANDALONE__
// commdlg/dlgs/cderr: required by win32xx headers (wxx_appcore.h uses PRINTDLG, wxx_commondlg.h
// uses OPENFILENAME/pshHelp/CDERR_* ...) which never include them, and by the ui/win file
// dialogs. Kept here as the framework's expected pre-include rather than repeating it ahead of
// every wxx header.
#include <commdlg.h>
#include <dlgs.h>
#include <cderr.h>

#else
   #ifndef _MSC_VER // MSVC provides the secure CRT functions (and deprecates the plain ones)
      #define fopen_s(pFile, filename, mode) (((*(pFile)) = fopen((filename), (mode))) == nullptr)
      #define fprintf_s fprintf
      #define fscanf_s fscanf
      #define sscanf_s sscanf
   #endif

   #ifndef _WIN32
      #define localtime_s(x, y) localtime_r(y, x)
      #define gmtime_s(x, y) gmtime_r(y, x)
      // asctime_r needs at least 26 bytes and takes no size, so that argument is dropped
      #define asctime_s(x, size, y) asctime_r(y, x)
      #define strnlen_s strnlen
      #define _aligned_malloc(size, align) aligned_alloc(align, size)
      #define _aligned_free free
   #endif

   #define _ASSERTE(expr) ((void)0)

#ifndef VPX_HAS_WIN32_SDK // Otherwise from wincrypt.h
typedef ULONG_PTR HCRYPTKEY;
#endif

#include <atldef.h>
#include <atlcom.h>
#include <atlcomcli.h>
#include <atlsafe.h>
#endif


// TODO Custom defines to be removed from PCH to get back to a cleaner codebase with includes corresponding to the actual dependencies of each file (beside std/OS/...)

#ifndef __STANDALONE__
#include "vpinball.h"
#else
#include "core/generated/vpinball_standalone_i.h"
#endif

#include "utils/Logger.h"
#include "core/def.h"
#include "core/dispid.h"
#include "core/Settings.h"
#include "core/SettingsService.h"
#include "core/extern.h"
#include "core/player.h"
