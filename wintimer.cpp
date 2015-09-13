#include "StdAfx.h"

static unsigned int sTimerInit = 0;
static LARGE_INTEGER TimerFreq;
static LARGE_INTEGER sTimerStart;

void wintimer_init()
{
   sTimerInit = 1;

   QueryPerformanceFrequency(&TimerFreq);
   QueryPerformanceCounter(&sTimerStart);
}

unsigned long long usec()
{
   if (sTimerInit == 0) return 0;

   LARGE_INTEGER TimerNow;
   QueryPerformanceCounter(&TimerNow);
   const unsigned long long cur_tick = (unsigned long long)(TimerNow.QuadPart - sTimerStart.QuadPart);
   return ((unsigned long long)TimerFreq.QuadPart < 100000000ull) ? (cur_tick * 1000000ull / (unsigned long long)TimerFreq.QuadPart)
      : (cur_tick * 1000ull / ((unsigned long long)TimerFreq.QuadPart / 1000ull));
}

U32 msec()
{
   if (sTimerInit == 0) return 0;

   LARGE_INTEGER TimerNow;
   QueryPerformanceCounter(&TimerNow);
   const LONGLONG cur_tick = TimerNow.QuadPart - sTimerStart.QuadPart;
   return (U32)((unsigned long long)cur_tick * 1000ull / (unsigned long long)TimerFreq.QuadPart);
}

void uSleep(const unsigned long long u)
{
   if (sTimerInit == 0) return;

   LARGE_INTEGER TimerEnd;
   QueryPerformanceCounter(&TimerEnd);
   TimerEnd.QuadPart += (u * TimerFreq.QuadPart) / 1000000ull - sTimerStart.QuadPart;

   LARGE_INTEGER TimerNow;
   do
   {
      SwitchToThread();

      QueryPerformanceCounter(&TimerNow);
   } while (TimerNow.QuadPart - sTimerStart.QuadPart < TimerEnd.QuadPart);
}
