// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

/*#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif*/

//#include <atlimpl.cpp>

// get rid of crt and include this as a define later
//,_ATL_MIN_CRT

//#include <apcguids.h>
#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
//#include <statreg.cpp>
#endif
//#include <atlimpl.cpp>
//#include <apcimpl.cpp>
#include "main.h"
//#include "Surface.h"
#include "Timer.h"
#include "Plunger.h"
#include "Textbox.h"
#include "Bumper.h"
#include "Trigger.h"
#include "Light.h"
#include "Kicker.h"
#include "Gate.h"
#include "Spinner.h"
#include "Ramp.h"
#include "BallEx.h"
#include "DispReel.h"
#include "LightSeq.h"
#include "ComControl.h"
//#include "Flipper.h"

//CComModule _Module;
BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Wall, Surface)
OBJECT_ENTRY(CLSID_DragPoint, DragPoint)
OBJECT_ENTRY(CLSID_Flipper, Flipper)
OBJECT_ENTRY(CLSID_Timer, Timer)
OBJECT_ENTRY(CLSID_Plunger, Plunger)
OBJECT_ENTRY(CLSID_Textbox, Textbox)
OBJECT_ENTRY(CLSID_Bumper, Bumper)
OBJECT_ENTRY(CLSID_Trigger, Trigger)
OBJECT_ENTRY(CLSID_Light, Light)
OBJECT_ENTRY(CLSID_Kicker, Kicker)
OBJECT_ENTRY(CLSID_Gate, Gate)
OBJECT_ENTRY(CLSID_Spinner, Spinner)
OBJECT_ENTRY(CLSID_Ramp, Ramp)
OBJECT_ENTRY(CLSID_Ball, BallEx)
OBJECT_ENTRY(CLSID_DispReel, DispReel)
OBJECT_ENTRY(CLSID_LightSeq, LightSeq)
OBJECT_ENTRY(CLSID_ComControl, PinComControl)
END_OBJECT_MAP()
