// stdafx.cpp : source file that includes just the standard includes
//  stdafx.pch will be the pre-compiled header
//  stdafx.obj will contain the pre-compiled type information

#include "StdAfx.h"

#ifdef _ATL_STATIC_REGISTRY
 #include <statreg.h>
#endif
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
#include "Primitive.h"
#include "Flasher.h"
#include "BallEx.h"
#include "DispReel.h"
#include "LightSeq.h"
#include "ComControl.h"

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
OBJECT_ENTRY(CLSID_Primitive, Primitive)
OBJECT_ENTRY(CLSID_Flasher, Flasher)
OBJECT_ENTRY(CLSID_Ball, BallEx)
OBJECT_ENTRY(CLSID_DispReel, DispReel)
OBJECT_ENTRY(CLSID_LightSeq, LightSeq)
OBJECT_ENTRY(CLSID_ComControl, PinComControl)
END_OBJECT_MAP()
