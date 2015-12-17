#pragma once
#ifndef __WINTIMER_H__
#define __WINTIMER_H__

void wintimer_init();

U32 msec();
unsigned long long usec();

void uSleep(const unsigned long long u);

double TheoreticRadiation(const unsigned int day, const unsigned int month, const unsigned int year, const double rlat);
double MaxTheoreticRadiation(const unsigned int year, const double rlat);
double SunsetSunriseLocalTime(const unsigned int day, const unsigned int month, const unsigned int year, const double rlong, const double rlat, const bool sunrise);

#endif /* !__WINTIMER_H__ */
