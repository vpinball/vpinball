#pragma once
#ifndef __WINTIMER_H__
#define __WINTIMER_H__

// call before 1st use of msec,usec or uSleep
void wintimer_init();

U32 msec();
unsigned long long usec();

// needs timeBeginPeriod(1) before calling 1st time to make the Sleep(1) in here behave more or less accurately (and timeEndPeriod(1) after not needing that precision anymore)
void uSleep(const unsigned long long u);
void uOverSleep(const unsigned long long u);

double TheoreticRadiation(const unsigned int day, const unsigned int month, const unsigned int year, const double rlat);
double MaxTheoreticRadiation(const unsigned int year, const double rlat);
double SunsetSunriseLocalTime(const unsigned int day, const unsigned int month, const unsigned int year, const double rlong, const double rlat, const bool sunrise);

#endif /* !__WINTIMER_H__ */
