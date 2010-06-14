//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// win_timer_usec.cpp
// Windows platform specific implementation of microsecond timer
// Copyright 2004  GFX Construction Inc.
// All rights reserved.
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#include "StdAfx.h"

#include <time.h>

static U64 sCpuSpeed;
static U64 sCpuSpeed_1024_M;
static U64 sStartTick;

void wintimer_init()
{
	if( sCpuSpeed ) return;

	LARGE_INTEGER freq;
	LARGE_INTEGER initial;
	QueryPerformanceFrequency( &freq );
	QueryPerformanceCounter( &initial );

	sStartTick = get_tick_count();

	LARGE_INTEGER cur;

	// Delay for effectively some small but mostly random amount of time ..
	// because we could exit this loop after switching back to this task..
	do
	{
		QueryPerformanceCounter( &cur );
	}
	while( cur.QuadPart - initial.QuadPart < freq.QuadPart / 10 );

	// Get an accurate sample assuming we busted the above loop on the first iteration after returning to this thread
	QueryPerformanceCounter( &cur );
	// BUG: if we switch between the two lines, the timer will be busted.  Let's hope it doesn't happen for now
	U64 end_tick = get_tick_count();

	sCpuSpeed = (U64) ( ( (F64) (end_tick - sStartTick) ) * ( (F64)freq.QuadPart / ( (F64)( cur.QuadPart - initial.QuadPart ) ) ) );
	sCpuSpeed_1024_M = (sCpuSpeed * 1024) / 1000000;

//	debug_message("    main CPU speed: %ld MHz\n", sCpuSpeed / 1000000);
}

U64 usec()
{
	U64		usec_time;
	U64		cur_tick;

	if( !sCpuSpeed ) return 0;

	cur_tick = get_tick_count() - sStartTick;

	usec_time = (cur_tick << 10);
	usec_time /= sCpuSpeed_1024_M;

	return (usec_time);
}

U32 msec()
{
	U64		usec_time;
	U64		cur_tick;

	if( !sCpuSpeed ) return 0;

	cur_tick = get_tick_count() - sStartTick;

	usec_time = (cur_tick << 10);
	usec_time /= ( sCpuSpeed_1024_M * 1000 );

	return (U32) usec_time;
}
