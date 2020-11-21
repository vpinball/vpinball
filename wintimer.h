#pragma once

// call if msec,usec or uSleep, etc. should be more precise
void set_lowest_possible_win_timer_resolution();
// and then call this after being finished with precise measurements to allow the OS to better save power
void restore_win_timer_resolution();

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
