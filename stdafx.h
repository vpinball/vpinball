#pragma once

//#define DISABLE_FORCE_NVIDIA_OPTIMUS // do not enable NVIDIA Optimus cards (on Laptops, etc) by default
//#define NVAPI_DEPTH_READ // forces NVAPI to do copydepth (as officially unspecified in DX9)

//#define DISABLE_FORCE_AMD_HIGHPERF // do not enable AMD high performance device (on Laptops, etc) by default

//#define TWOSIDED_TRANSPARENCY // transparent hit targets are rendered backsided first, then frontsided

#define USE_D3D9EX // if enabled can use extended API, does not benefit on XP though // currently only allows to enable adaptive vsync again (on win vista/7 only, seems to be broken in win8)

#define COMPRESS_MESHES // uses miniz for compressing the meshes

#define CRASH_HANDLER

//#define _CRTDBG_MAP_ALLOC

//#define DEBUG_NUDGE // debug new nudge code

//#define ONLY_USE_BASS

//#define DEBUG_NO_SOUND
//#define DEBUG_REFCOUNT_TRIGGER

//#define ENABLE_TRACE // enables all TRACE_FUNCTION() calls to use D3DPERF_Begin/EndEvent

//#define DEBUG_XXX // helps to detect out-of-bounds access, needs to link dbghelp.lib then
//#define SLINTF    // enable debug console output

#define EDITOR_BG_WIDTH    1000
#define EDITOR_BG_HEIGHT   750

#define MAIN_WINDOW_WIDTH  1280
#define MAIN_WINDOW_HEIGHT (720-50)

#define BASEDEPTHBIAS 5e-5f

#define THREADS_PAUSE 1000 // msecs/time to wait for threads to finish up

#include "physconst.h"

//

#define NUM_BG_SETS        3  // different backdrop settings: DT,FS,FSS
#define BG_DESKTOP         0
#define BG_FULLSCREEN      1
#define BG_FSS             2

#define MAX_BALL_TRAIL_POS 10 // fake/artistic ball motion trail

#define MAX_REELS          32

#define LIGHTSEQGRIDSCALE  20
#define	LIGHTSEQGRIDWIDTH  EDITOR_BG_WIDTH/LIGHTSEQGRIDSCALE
#define	LIGHTSEQGRIDHEIGHT (2*EDITOR_BG_WIDTH)/LIGHTSEQGRIDSCALE

#define LIGHTSEQQUEUESIZE  100

#define MAX_LIGHT_SOURCES  2
#define MAX_BALL_LIGHT_SOURCES  8

//

#define ADAPT_VSYNC_FACTOR 0.95 // safety factor where vsync is turned off (f.e. drops below 60fps * 0.95 = 57fps)

#define ACCURATETIMERS          // if undefd, timers will only be triggered as often as frames are rendered (e.g. they can fall behind)
#define MAX_TIMER_MSEC_INTERVAL 1 // amount of msecs to wait (at least) until same timer can be triggered again (e.g. they can fall behind, if set to > 1, as update cycle is 1000Hz)
#define MAX_TIMERS_MSEC_OVERALL 5 // amount of msecs that all timers combined can take per frame (e.g. they can fall behind, if set to < somelargevalue)

#define STEPPING                // enable Physics stepping

#if defined(_DEBUG) && defined(STEPPING)
 #define MOUSEPAUSE
#endif

//#define PLAYBACK              // bitrotted, also how to record the playback to c:\badlog.txt ?? via LOG ??
//#define LOG                   // bitrotted, will record stuff into c:\log.txt

//#define EVENPHYSICSTIME       // bitrotted, most likely not functional anymore

//#define DEBUGPHYSICS          // enables detailed physics/collision handling output for the 'F11' stats/debug texts

#define DEBUG_BALL_SPIN         // enables dots glued to balls if in 'F11' mode

//

#define LAST_OPENED_TABLE_COUNT 8

#define MAX_CUSTOM_PARAM_INDEX  10

#define MAX_OPEN_TABLES         9

#define AUTOSAVE_DEFAULT_TIME   10

#define DEFAULT_SECURITY_LEVEL  0

//

#if !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_)
#define AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_

#define _WINSOCKAPI_ // workaround some issue where windows.h is included before winsock2.h in some of the various includes

#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#define STRICT

#ifndef _WIN32_WINNT
#if defined(_WIN64) && defined(CRASH_HANDLER)
 #define _WIN32_WINNT 0x0501
#else
 #define _WIN32_WINNT 0x0500
#endif
#endif

#define _ATL_APARTMENT_THREADED


//#include <vld.h>
#ifdef _CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
#endif

#if(_WIN32_WINNT < 0x0500) // for Win32++ and old compilers
 #define WM_MENURBUTTONUP                0x0122
 #define WM_UNINITMENUPOPUP              0x0125
#endif
#include "main.h"


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED)
