#include "StdAfx.h"

static unsigned int sTimerInit = 0;
static LARGE_INTEGER TimerFreq;
static LARGE_INTEGER sTimerStart;

void wintimer_init()
{
	sTimerInit = 1;

    QueryPerformanceFrequency( &TimerFreq );
	QueryPerformanceCounter( &sTimerStart );
}

LONGLONG usec()
{
	if( sTimerInit == 0 ) return 0;

    LARGE_INTEGER TimerNow;
	QueryPerformanceCounter( &TimerNow );
	const LONGLONG cur_tick = TimerNow.QuadPart - sTimerStart.QuadPart;
	return (TimerFreq.QuadPart < 100000000ull) ? (LONGLONG)((unsigned long long)cur_tick*1000000ull/(unsigned long long)TimerFreq.QuadPart)
		                                       : (LONGLONG)((unsigned long long)cur_tick*1000ull/((unsigned long long)TimerFreq.QuadPart/1000ull));
}

U32 msec()
{
	if( sTimerInit == 0 ) return 0;

    LARGE_INTEGER TimerNow;
	QueryPerformanceCounter( &TimerNow );
	const LONGLONG cur_tick = TimerNow.QuadPart - sTimerStart.QuadPart;
	return (U32)((unsigned long long)cur_tick*1000ull/(unsigned long long)TimerFreq.QuadPart);
}
