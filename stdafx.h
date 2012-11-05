#define ULTRACADE
#define ULTRA_FREE 1
#ifndef ULTRA_FREE
 #define DONGLE_SUPPORT			// UltraPin Dongle: Remember to run hdd32.exe (HASP dongle driver setup) on host
#endif
//#define ULTRAPIN              // define to enable deprecated UltraPin functionality

#define GRAVITYCONST 0.86543f

#define VP3D                    // enable 3D stereo code //!! remove define(s) after first succesful release
#define ONLY3DUPD               // faster 3D stereo code (same trick as VP2D does: only update regions each frame)

#define ADAPT_VSYNC_FACTOR 1.05 // safety factor where adaptive vsync kicks in (f.e. 60fps * 1.05 = 63fps)

#define ACCURATETIMERS 1

#define FPS 1                   // Enable FPS computation (default 'F10')
#define STEPPING 1              // Enable Physics stepping

//#define _DEBUGPHYSICSx
//#define DEBUG_FRATE
//#define ANTI_TEAR	1			// define if you want to have some hacky code to prevent drawing more than one frame per 16ms = ~60fps
//#define GDIDRAW 1
//#define EVENTIME 1

#define GPINFLOAT double 

#if !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_)
#define AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#define STRICT
#ifndef _WIN32_WINNT
#if _MSC_VER < 1600
#define _WIN32_WINNT 0x0400
#else
#define _WIN32_WINNT 0x0403
#endif
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
class CExeModule : public CComModule
{
public:
	LONG Unlock();
	DWORD dwThreadID;
	HANDLE hEventShutdown;
	void MonitorShutdown();
	bool StartMonitor();
	bool bActivity;
};
extern CExeModule _Module;

#include <Windows.h>

#include "Richedit.h"
#include "RichOle.h"
#include "Tom.h"

#include <atlcom.h>

#ifdef VBA
#define APC_IMPORT_MIDL
#endif

#include <atlbase.h>
#include <atlhost.h>
//extern CComModule _Module;
#ifdef VBA
#include <apcCpp.h>
using namespace MSAPC;
#endif

#include "math.h"

#include "commctrl.h"

#include <activdbg.h>

#include <atlctl.h>
#include <atlhost.h>

#define DIRECTINPUT_VERSION 0x0700
#define D3D_OVERLOADS 1
#include "dinput.h"
#include "ddraw.h"
#include "d3d.h"
#include "dsound.h"

#include <ocidl.h>	// Added by ClassView

#include "stdio.h"
#include "wchar.h"

#include "main.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED)
