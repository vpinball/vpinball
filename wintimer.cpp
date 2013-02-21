#include "StdAfx.h"

static unsigned int sTimerInit = 0;
static double sTimerFreqDiv1000;
static double sTimerFreqDiv1000000;
static LARGE_INTEGER sTimerStart;

void wintimer_init()
{
	sTimerInit = 1;

    LARGE_INTEGER TimerFreq;
	QueryPerformanceFrequency( &TimerFreq );
	sTimerFreqDiv1000 = 1000.0/TimerFreq.QuadPart;
	sTimerFreqDiv1000000 = 1000000.0/TimerFreq.QuadPart;
	QueryPerformanceCounter( &sTimerStart );
}

LONGLONG usec()
{
	if( sTimerInit == 0 ) return 0;

    LARGE_INTEGER TimerNow;
	QueryPerformanceCounter( &TimerNow );
	const LONGLONG cur_tick = TimerNow.QuadPart - sTimerStart.QuadPart;
	return (LONGLONG)((double)cur_tick*sTimerFreqDiv1000000);
}

U32 msec()
{
	if( sTimerInit == 0 ) return 0;

    LARGE_INTEGER TimerNow;
	QueryPerformanceCounter( &TimerNow );
	const LONGLONG cur_tick = TimerNow.QuadPart - sTimerStart.QuadPart;
	return (U32)((double)cur_tick*sTimerFreqDiv1000);
}
