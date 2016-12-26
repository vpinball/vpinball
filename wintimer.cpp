#include "StdAfx.h"
#include <time.h>

static unsigned int sTimerInit = 0;
static LARGE_INTEGER TimerFreq;
static LARGE_INTEGER sTimerStart;

// call before 1st use of msec,usec or uSleep
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

// tries(!) to be as exact as possible at the cost of potentially causing trouble with other threads/cores due to OS madness
// needs timeBeginPeriod(1) before calling 1st time to make the Sleep(1) in here behave more or less accurately (and timeEndPeriod(1) after not needing that precision anymore)
// but VP code does this already
void uSleep(const unsigned long long u)
{
   if (sTimerInit == 0) return;

   LARGE_INTEGER TimerNow;
   QueryPerformanceCounter(&TimerNow);
   LARGE_INTEGER TimerEnd;
   TimerEnd.QuadPart = TimerNow.QuadPart + ((u * TimerFreq.QuadPart) / 1000000ull);
   const LONGLONG TwoMSTimerTicks = (2000 * TimerFreq.QuadPart) / 1000000ull;

   while (TimerNow.QuadPart < TimerEnd.QuadPart)
   {
      if ((TimerEnd.QuadPart - TimerNow.QuadPart) > TwoMSTimerTicks)
         Sleep(1); // really pause thread for 1-2ms (depending on OS)
      else
         YieldProcessor(); // was: "SwitchToThread() let other threads on same core run" //!! could also try Sleep(0) or directly use _mm_pause() instead of YieldProcessor() here

      QueryPerformanceCounter(&TimerNow);
   }
}

// can sleep too long by 1000 to 2000 (=1 to 2ms)
// needs timeBeginPeriod(1) before calling 1st time to make the Sleep(1) in here behave more or less accurately (and timeEndPeriod(1) after not needing that precision anymore)
// but VP code does this already
void uOverSleep(const unsigned long long u)
{
   if (sTimerInit == 0) return;

   LARGE_INTEGER TimerNow;
   QueryPerformanceCounter(&TimerNow);
   LARGE_INTEGER TimerEnd;
   TimerEnd.QuadPart = TimerNow.QuadPart + ((u * TimerFreq.QuadPart) / 1000000ull);

   while (TimerNow.QuadPart < TimerEnd.QuadPart)
   {
      Sleep(1); // really pause thread for 1-2ms (depending on OS)
      QueryPerformanceCounter(&TimerNow);
   }
}

//

static const unsigned int daysPerMonths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }; // Number of days per month

// (Rough) angle of the day (radian)
double AngleOfDay(const unsigned int day, const unsigned int month, const unsigned int year)
{
	bool leapYear;
	unsigned int totalDaysInYear;
    if ((year % 400) == 0)
    {
        totalDaysInYear = 366;
		leapYear = true;
    }
    else if ((year % 100) == 0)
    {
        totalDaysInYear = 365;
		leapYear = false;
    }
    else if ((year % 4) == 0)
    {
        totalDaysInYear = 366;
		leapYear = true;
    }
    else
    {
        totalDaysInYear = 365;
		leapYear = false;
    }

	unsigned int numOfDays = 0;
    for (unsigned int i = 1; i < month; i++)
		numOfDays += daysPerMonths[i-1];
	if ((month > 2) && leapYear)
        numOfDays++;
    numOfDays += day;

    return ((2. * M_PI)*(numOfDays - 1)) / totalDaysInYear;
}

double SolarDeclination(const double dayAngle) // radian
{
	const double c = cos(dayAngle);
	const double s = sin(dayAngle);
	return 0.006918
         - 0.399912 * c
         + 0.070257 * s
         - 0.006758 * (2.*c*c - 1.)
         + 0.000907 * 2.*c*s
         - 0.002697 * (c*(4.*c*c - 3.))
         + 0.00148  * (s*(-4.*s*s + 3.));
}

double EquationOfTimeRadian(const double dayAngle) // radian
{
	const double c = cos(dayAngle);
	const double s = sin(dayAngle);
	return 0.000075
         + 0.001868 * c
         - 0.032077 * s
		 - 0.014615 * (2.*c*c - 1.)
		 - 0.04089  * 2.*c*s;
}

inline double DayDurationHalfRadian(const double declination, const double rlat) // radian, result radian*0.5
{
    return acos(-tan(rlat) * tan(declination));
}

inline double DayDurationHours(const double declination, const double rlat) // radian
{
	return DayDurationHalfRadian(declination, rlat) * (24. / M_PI);
}

// Decimal hour of sunset/sunrise: result in universal time
double SunsetSunriseUniversalTime(const unsigned int day, const unsigned int month, const unsigned int year, const double rlong, const double rlat, const bool sunrise) // longitude in radians (positive east)
{
    const double dayAngle = AngleOfDay(day, month, year);
    const double ddh = fabs(DayDurationHours(SolarDeclination(dayAngle), rlat));

	return 12. + (sunrise ? -0.5 : 0.5)*ddh - (rlong + EquationOfTimeRadian(dayAngle)) * (12. / M_PI);
}

double LocalTimeAdjust()
{
    time_t hour_machine;
    time(&hour_machine);
	tm gmt_hour;
	gmtime_s(&gmt_hour, &hour_machine);
	tm local_hour;
	localtime_s(&local_hour, &hour_machine);

	const int dif = local_hour.tm_hour - gmt_hour.tm_hour;
	return (dif < -12) ? dif + 24 : dif;
}

// Decimal hour of sunset/sunrise: result in local hour
double SunsetSunriseLocalTime(const unsigned int day, const unsigned int month, const unsigned int year, const double rlong, const double rlat, const bool sunrise) // longitude in radians (positive east)
{
	return SunsetSunriseUniversalTime(day, month, year, rlong, rlat, sunrise) + LocalTimeAdjust();
}

double OrbitalExcentricity(const double dayAngle)
{
	const double c = cos(dayAngle);
	const double s = sin(dayAngle);
	return 1.000110
		+ 0.034221 * c
		+ 0.001280 * s
		+ 0.000719 * (2.*c*c - 1.)
		+ 0.000077 * 2.*c*s;
}

// Theoretical energy flux for the day radiation
double TheoreticRadiation(const unsigned int day, const unsigned int month, const unsigned int year, const double rlat) // radian
{
	const double dayAngle = AngleOfDay(day, month, year);
	const double declination = SolarDeclination(dayAngle);
	const double e0 = OrbitalExcentricity(dayAngle);
	const double sunriseHourAngle = DayDurationHalfRadian(declination, rlat);

	const double c0 = cos(declination - rlat);
	const double c1 = cos(declination + rlat);
	// Theoretical radiation in W.m-2
	const double solarConst = 1367.; // solar constant W.m-2
	return 0.5 * solarConst * e0 * ((c0 + c1)*sin(sunriseHourAngle) / sunriseHourAngle + c0 - c1);
}

// Max/Year Theoretical energy flux for the day radiation
double MaxTheoreticRadiation(const unsigned int year, const double rlat) // radian
{
    double maxTR = 0.;
    for(unsigned int month = 0; month < 12; ++month)
        for(unsigned int day = 0; day < daysPerMonths[month]; ++day)
        {
            const double TR = TheoreticRadiation(day,month,year,rlat);
            if(TR > maxTR)
                maxTR = TR;
        }
    return maxTR;
}

#if 0
// Height of sun in radians
double SolarHeight(const unsigned int tu, // universal time (0,1,2,.....,23)
	               const unsigned int day, const unsigned int month, const unsigned int year, const double rlong, const double rlat) // longitude in radian (positive east and negative west)
{
    const double dayAngle = AngleOfDay(day, month, year);
    const double declination = SolarDeclination(dayAngle);
	const double tsvh = tu * (M_PI / 12.) + rlong * (180. / (15.*12.)) + EquationOfTimeRadian(dayAngle);

	const double c0 = cos(rlat - declination);
	const double c1 = cos(rlat + declination);
	const double c2 = cos(tsvh);
	return asin(0.5*(c0 - c1 - c2*(c0 + c1)));
}

// http://www.cs.utsa.edu/~cs1063/projects/Spring2011/Project1/jdn-explanation.html
int JulianConversion(const unsigned int year, const unsigned int month, const unsigned int day)
{
    const int a = (14 - month) / 12;
    const int y = year + 4800 - a;
    const int m = month + 12 * a - 3;
    return day + (153 * m + 2) / 5 + 365 * y + y / 4 + 
        ((year > 1582 || (year == 1582 && month > 10) || (year == 1582 && month == 10 && day >= 15)) ? (- y / 100 + y / 400 - 32045) : - 32083);
}

int _tmain(int argc, _TCHAR* argv[])
{
	const unsigned int day = 16;
	const unsigned int month = 12;
	const unsigned int year = 2015;
	const double lat = 52.52;
	const double lon = 13.37;

    //

	const double rlat = lat * (M_PI / 180.);
	const double rlong = lon * (M_PI / 180.);

	const double _AngleOfDay = AngleOfDay(day, month, year);
    std::cout << "Angle of day: " << _AngleOfDay << "\n";

	const double _Declination = SolarDeclination(_AngleOfDay);
    std::cout << "Declination (Delta): " << _Declination << "\n";

	const double _EquationOfTime = EquationOfTimeRadian(_AngleOfDay) * (12. / M_PI);
    std::cout << "Equation Of Time (Delta): " << _EquationOfTime << "\n";

	const double _DayDurationHours = DayDurationHours(_Declination, rlat);
    std::cout << "Day duration: " << _DayDurationHours << "\n";

	const double _OrbitalExcentricity = OrbitalExcentricity(_AngleOfDay);
	std::cout << "Excentricity: " << _OrbitalExcentricity << "\n";

	const double _TheoreticRadiation = TheoreticRadiation(day, month, year, rlat);
    std::cout << "Theoretical radiation: " << _TheoreticRadiation << "\n";

	const double _MaxTheoreticRadiation = MaxTheoreticRadiation(year, rlat);
    std::cout << "Max./Year Theoretical radiation: " << _MaxTheoreticRadiation << "\n";

	const double _SunriseLocalTime = SunsetSunriseLocalTime(day, month, year, rlong, rlat, true);
    std::cout << "Sunrise Local Time: " << _SunriseLocalTime << "\n";

	const double _SunsetLocalTime = SunsetSunriseLocalTime(day, month, year, rlong, rlat, false);
    std::cout << "Sunset Local Time: " << _SunsetLocalTime << "\n";

    return 0;
}
#endif
