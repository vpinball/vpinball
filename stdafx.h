#pragma once

//#define DISABLE_FORCE_NVIDIA_OPTIMUS // do not enable NVIDIA Optimus cards (on Laptops, etc) by default

#define USE_D3D9EX // if enabled can use extended API, does not benefit on XP though // currently only allows to enable adaptive vsync again (on win vista/7 only, seems to be broken in win8)

#define COMPRESS_MESHES // uses miniz for compressing the meshes

#define CRASH_HANDLER

//#define _CRTDBG_MAP_ALLOC
#ifdef _CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
#endif

//#define DEBUG_NUDGE // debug new nudge code

//#define DEBUG_NO_SOUND
//#define DEBUG_REFCOUNT_TRIGGER

//#define DEBUG_XXX // helps to detect out-of-bounds access, needs to link dbghelp.lib then
//#define SLINTF    // enable debug console output

#define EDITOR_BG_WIDTH 1000
#define EDITOR_BG_HEIGHT 750

#define BASEDEPTHBIAS 5e-5f

#include "physconst.h"

//

#define NUM_BG_SETS        2

#define MAX_BALL_TRAIL_POS 10

#define MAX_REELS          32

#define LIGHTSEQGRIDSCALE  20
#define	LIGHTSEQGRIDWIDTH  EDITOR_BG_WIDTH/LIGHTSEQGRIDSCALE
#define	LIGHTSEQGRIDHEIGHT (2*EDITOR_BG_WIDTH)/LIGHTSEQGRIDSCALE

#define LIGHTSEQQUEUESIZE  100

#define MAX_LIGHT_SOURCES  2
#define MAX_BALL_LIGHT_SOURCES  4

//

#define ADAPT_VSYNC_FACTOR 0.95 // safety factor where vsync is turned off (f.e. drops below 60fps * 0.95 = 57fps)

#define ACCURATETIMERS 1

#define FPS 1                   // Enable FPS computation (default 'F11')
#define STEPPING 1              // Enable Physics stepping

#if defined(_DEBUG) && defined(STEPPING)
 #define MOUSEPAUSE 1
#endif

//#define PLAYBACK

//#define LOG

//#define _DEBUGPHYSICS
//#define DEBUG_FPS
//#define EVENTIME 1

#define DEBUG_BALL_SPIN

//

#if !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_)
#define AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_

#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#define STRICT

#ifndef _WIN32_WINNT
#if defined(_WIN64) && defined(CRASH_HANDLER)
 #define _WIN32_WINNT 0x0501
#elif _MSC_VER >= 1800
 #define _WIN32_WINNT 0x0500
#elif _MSC_VER < 1600
 #define _WIN32_WINNT 0x0400
#else
 #define _WIN32_WINNT 0x0403
#endif
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>

#include "main.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED)
