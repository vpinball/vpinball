#pragma once

//#define _CRTDBG_MAP_ALLOC
#ifdef _CRTDBG_MAP_ALLOC
 #include <crtdbg.h>
#endif

//#define DEBUG_XXX // helps to detect out-of-bounds access, needs to link dbghelp.lib then
//#define SLINTF    // enable debug console output

//#define ULTRAPIN       // define to enable deprecated UltraPin functionality
//#define DONGLE_SUPPORT // define to enable deprecated UltraPin dongle (Remember to run hdd32.exe (HASP dongle driver setup) on host)

#define GRAVITYCONST 0.86543f

// Collisions (also some more defaults are in collide.cpp):
//
// test near zero conditions in linear, well behaved, conditions
#define C_PRECISION 0.01f
// tolerance for line segment endpoint and point radii collisions
#define C_TOL_ENDPNTS 0.0f
#define C_TOL_RADIUS 0.005f
// Physical Skin ... postive contact layer. Any contact (collision) in this layer reports zero time.
// layer is used to calculate contact effects ... beyond this and objects pass through each other
// Default 25.0
#define PHYS_SKIN 25.0
// Layer outside object which increases it's size for contact measurements. Used to determine clearances.
// Setting this value during testing to 0.1 will insure clearance. After testing set the value to 0.005
// Default 0.01
#define PHYS_TOUCH 0.05
// Low Normal speed collison is handled as contact process rather than impulse collision
#define C_LOWNORMVEL 0.0001f
#define C_CONTACTVEL 0.099f
// limit ball speed to C_SPEEDLIMIT
#define C_SPEEDLIMIT 60.0f
#define C_DAMPFRICTION 0.95f
// low velocity stabilization ... if embedding occurs add some velocity 
#define C_EMBEDDED 0.0f
#define C_EMBEDSHOT 0.05f
// Contact displacement corrections, hard ridgid contacts i.e. steel on hard plastic or hard wood
#define C_DISP_GAIN 0.9875f
#define C_DISP_LIMIT 5.0f
#define C_FRICTIONCONST 2.5e-3f
#define C_DYNAMIC 2
//trigger/kicker boundary crossing hysterisis
#define STATICTIME 0.005f

//Flippers:
#define C_FLIPPERACCEL   1.25f
#define C_FLIPPERIMPULSE 1.0f
#define C_INTERATIONS 20 // Precision level and cycles for interative calculations // acceptable contact time ... near zero time

//Ball:
#define ANGULARFORCE 1   // Number I pulled out of my butt - this number indicates the maximum angular change 
					     // per time unit, or the speed at which angluar momentum changes over time, 
					     // depending on how you interpret it.
//Ramp:
#define WALLTILT 0.5f

//Plumb:
#define	VELOCITY_EPSILON 0.05f	// The threshold for zero velocity.

#define PLUNGERHEIGHT 50.0f

//

#define MAX_REELS           32

#define LIGHTSEQGRIDSCALE	20
#define	LIGHTSEQGRIDWIDTH	1000/LIGHTSEQGRIDSCALE
#define	LIGHTSEQGRIDHEIGHT	2000/LIGHTSEQGRIDSCALE

#define LIGHTSEQQUEUESIZE	100

#define MAX_LIGHT_SOURCES   8

//

#define ONLY3DUPD               // faster 3D stereo code (same trick as VP2D does: only update regions each frame)

#define ADAPT_VSYNC_FACTOR 0.95 // safety factor where vsync is turned off (f.e. drops below 60fps * 0.95 = 57fps)

#define PHYSICS_STEPTIME 10000	// usecs to go between each physics update //!! cannot be changed, otherwise physics are slower/faster

#define ACCURATETIMERS 1

#define FPS 1                   // Enable FPS computation (default 'F10')
#define STEPPING 1              // Enable Physics stepping

#if defined(_DEBUG_) && defined(STEPPING)
 #define MOUSEPAUSE 1
#endif

//#define PLAYBACK

//#define LOG
//#define HITLOG
//#define GLOBALLOG

//#define _DEBUGPHYSICS
//#define DEBUG_FPS
//#define ANTI_TEAR	1			// define if you want to have some hacky code to prevent drawing more than one frame per 16ms = ~60fps
//#define EVENTIME 1

#define GPINFLOAT double

//

#if !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_)
#define AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED_

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

#include "main.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__35BEBBA5_0A4C_4321_A65C_AFFE89589F15__INCLUDED)
