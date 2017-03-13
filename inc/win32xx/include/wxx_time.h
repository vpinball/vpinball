// Win32++   Version 8.4
// Release Date: 10th March 2017
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2017  David Nash
//
// Permission is hereby granted, free of charge, to
// any person obtaining a copy of this software and
// associated documentation files (the "Software"),
// to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify,
// merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom
// the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice
// shall be included in all copies or substantial portions
// of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// Acknowledgement:
//
// The original author of CTime and CTimeSpan is:
//
//      Robert C. Tausworthe
//      email: robert.c.tausworthe@ieee.org
//
////////////////////////////////////////////////////////

////////////////////////////////////////////////////////
//
//	Declaration of the CTime class
//
////////////////////////////////////////////////////////

#ifndef _WIN32XX_CTIME_H_
#define _WIN32XX_CTIME_H_

#include "wxx_wincore.h"
#include "wxx_archive.h"
#include <errno.h>
#include <time.h>


namespace Win32xx
{

	////////////////////////////////////////////////////////
	//
	//	Local types, constants, etc.

	// define an alias for struct tm
	typedef struct tm time_tm;

	// define the CTimeSpan data type
	// This can be int or __int64 depending on the compiler
	// VS2005 and above defaults to __int64
	typedef time_t timespan_t;

	// forward declaration
	class CTimeSpan;

	// declaration of a non-CTime-class (i.e., global) time conversion utility
	time_t UTCtime(time_tm *atm);


	//============================================================================
	class CTime
	//	The CTime class is based internally on the time_t data type, which is
	//	measured in seconds past the January 1, 00:00:00 1970 UTC epoch. Thus,
	//	any given time_t instant is strictly a UTC epoch, which may then be
	//	rendered, using standard C library functions, as either a UTC or local
	//	time.  Local instants may further appear as standard or daylight times.
	//	Thus, the base epoch on a system in the PST zone will be displayed as
	//	December 31, 1969 16:00:00. Any attempt to construct or evaluate a date
	//	before this epoch will assert or be in error. Even though the time_t
	//	type is implemented (in MinGW, anyway) as a signed integer type, 
	//	negative values are not allowed.

	//	On systems where time_t is defined as a 32-bit integer, there is an
	//	upper date limit of January 18, 19:14:07, 2038. On 64-bit systems,
	//	there is no such upper date limit.

	//	Windows also has other time types that also interface with the CTime
	//	type. These are FILETIME, SYSTEMTIME, and MS-DOS date and time, each
	//	of which are described in the MSDN API documentation. Basically:

	//	FILETIME is a structure containing a 64-bit value representing the
	//	number of 100-nanosecond intervals since January 1, 1601 UTC.

	//	SYSTEMTIME is a structure representing a date and  time using individual
	//	values for the month, day, year, weekday, hour, minute, second, and
	//	millisecond. This is not useful in itself, but must be translated into
	//	a FILETIME or time_t.

	//	MS-DOS date and time are two 16-bit words packed with day, month, year
	//  and second, minute, hour fields.  The format may be found in the MSDN
	//	Library article on DosDateTimeToFileTime().
	{
		// global friends.  These functions can access private members
		friend  CArchive& operator<<(CArchive&, CTime&);
		friend  CArchive& operator>>(CArchive&, CTime&);

	public:

		// Constructors
		CTime();
		CTime(const CTime& t);
		CTime(time_t t);
		CTime(time_tm& t);
		CTime(UINT yr, UINT mo, UINT wkday, UINT nthwk, UINT hr, UINT min, UINT sec, int nDST = -1);
		CTime(UINT year, UINT month, UINT day, UINT hour, UINT min, UINT sec, int nDST = -1);
		CTime(UINT yr, UINT doy, UINT hr, UINT min, UINT sec, int nDST = -1);
		CTime(WORD wDosDate, WORD wDosTime, int nDST = -1);
		CTime(const SYSTEMTIME& st, int nDST = -1);
		CTime(const FILETIME& ft,  int nDST = -1);

		// Method members
		bool 	  GetAsFileTime(FILETIME& ft) const;
		bool 	  GetAsSystemTime(SYSTEMTIME& st) const;
		int		  GetDay(bool local = true) const;
		int		  GetDayOfWeek(bool local = true) const;
		int		  GetDayOfYear(bool local = true) const;
		time_tm*  GetGmtTm(time_tm* ptm = NULL) const;
		int		  GetHour(bool local = true) const;
		time_tm*  GetLocalTm(time_tm* ptm = NULL) const;
		int		  GetMinute(bool local = true) const;
		int		  GetMonth(bool local = true) const;
		int		  GetSecond(bool local = true) const;
		int		  GetYear(bool local = true) const;
		time_t	  GetTime() const;

		// Assignment operators
		CTime& 	operator=(const CTime& timeSrc);
		CTime& 	operator=(const time_t& t);

		// Computational operators
		const CTimeSpan operator-(const CTime& t) const;
		const CTime	  operator-(const CTimeSpan& ts) const;
		const CTime   operator+(const CTimeSpan& ts) const;
		CTime&  operator+=(const CTimeSpan& ts);
		CTime&  operator-=(const CTimeSpan& ts);
		bool 		  operator==(const CTime& t) const;
		bool 		  operator!=(const CTime& t) const;
		bool 		  operator<(const CTime& time) const;
		bool 		  operator>(const CTime& time) const;
		bool 		  operator<=(const CTime& time) const;
		bool 		  operator>=(const CTime& time) const;

		operator time_t() const { return m_time; }

		// CString conversion
		CString 	Format(LPCTSTR pFormat) const;
		CString 	Format(UINT nFormatID) const;
		CString 	FormatGmt(LPCTSTR pFormat) const;
		CString 	FormatGmt(UINT nFormatID) const;

		// Static methods
		static	CTime 	GetCurrentTime();
		static	FILETIME FileTimePlus(const FILETIME& ft, double addend);

	private:

		// private data members
		time_t 		m_time;

	};

	//============================================================================
	class CTimeSpan
	//	The CTimeSpan class defines the data type for differences between two
	//	CTime values, measured in seconds of time. It also defines procedures
	//	for combining that data type with that of CTime to produce CTime
	//	elements that differ by a specified span of time and  methods to
	//	extract and  display CTimeSpan values.
	{
		friend class CTime;		// CTime can access private members
	
	public:
		// Constructors
		CTimeSpan();
		CTimeSpan(timespan_t t);
		CTimeSpan(long lDays, int nHours, int nMins, int nSecs);
		CTimeSpan(const CTimeSpan& ts);

		// Methods to extract items
		LONGLONG	GetDays() const;
		LONGLONG	GetTotalHours() const;
		int 		GetHours() const;
		LONGLONG 	GetTotalMinutes() const;
		int 		GetMinutes() const;
		LONGLONG 	GetTotalSeconds() const;
		int 		GetSeconds() const;

		// assignment operators
		CTimeSpan& operator=(const CTimeSpan& ts);
		CTimeSpan& operator=(const timespan_t& t);

		// computational operators
		const CTimeSpan		operator-() const;
		const CTimeSpan 	operator-(CTimeSpan& ts) const;
		const CTimeSpan 	operator+(CTimeSpan& ts) const;
		CTimeSpan& operator+=(CTimeSpan& ts);
		CTimeSpan& operator-=(CTimeSpan& ts);
		bool 	  		operator==(const CTimeSpan& ts) const;
		bool 	  		operator!=(const CTimeSpan& ts) const;
		bool 	  		operator<(const CTimeSpan& ts) const;
		bool 	  		operator>(const CTimeSpan& ts) const;
		bool 	  		operator<=(const CTimeSpan& ts) const;
		bool 	  		operator>=(const CTimeSpan& ts) const;

		operator timespan_t() const { return m_timespan; }

		// CString conversion
		CString   	Format(LPCTSTR pFormat) const;
		CString   	Format(UINT nFormatID) const;

		// Global friends
		friend  CArchive& operator<<(CArchive&, CTimeSpan&);
		friend  CArchive& operator>>(CArchive&, CTimeSpan&);

	private:

		// private data members
		timespan_t m_timespan;
	};

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

	////////////////////////////////////////////////////////////////
	//
	//	Implementation of the CTime class
	//
	////////////////////////////////////////////////////////////////

	//============================================================================
	inline time_t UTCtime(time_tm* atm)
	//	Return the time_t t corresponding to the date given in atm as a UTC
	//	time. That is, gmtime(t) == atm.  This is equal to the local time_t
	//	of the atm plus the current time zone bias. This method asserts if
	//  atm is NULL or corresponds to an invalid time.
	{
		// compute the local time from atm
		assert(atm != NULL);           // atm must exist
		time_t t0 = ::mktime(atm);     // atm = *localtime(t0)
		assert(t0 != -1);
		time_tm* ptm0 = ::gmtime(&t0); // atm0 = UTC time of atm
		time_t t1 = ::mktime(ptm0);    // atm0 = localtime(t1)
		assert(t1 != -1);
		timespan_t zt = (timespan_t)(t0 - t1);  // time zone bias
		t0 += zt;
		assert(::gmtime(&t0));
		return t0;
	}

	//============================================================================
	inline CTime::CTime()
	//	Construct an CTime object initialized to the Jan 1, 1970 00:00:00 epoch.
	//	This constructor allows defining CTime object arrays, which may then
	//	be initialized prior to use.
	{
		m_time = 0;
	}

	//============================================================================
	inline CTime::CTime(const CTime& t)
	//	Construct a CTime object from another (valid) CTime object t.
	{
		m_time = t.m_time;
	}

	//============================================================================
	inline CTime::CTime(time_t t)
	//	Construct a CTime object from the time_t value t, or assert if t is invalid.
	{
		assert(::gmtime(&t));
		m_time = t;
	}

	//============================================================================
	inline CTime::CTime(time_tm& atm)
	//	Construct a CTime object from the time_tm atm, or assert if atm is invalid
	{
		// compute the object time_t
		m_time = ::mktime(&atm);

		// check for acceptable range
		assert(m_time != -1);
	}

	//============================================================================
	inline CTime::CTime(UINT yr, UINT mo, UINT wkday, UINT nthwk, UINT hr,
	    UINT min, UINT sec, int nDST /* = -1 */)
	//	Construct a CTime of the nthwk occurrence of the given wkday (0..6)
	//	in the mo month of yr year, at hr:min:sec of that day, local time.
	//	Restrictions on yr, mo, hr, min, and  sec are the same as cited in the
	//	constructor CTime(yr, mo, da, hr, min, sec, nDST). Otherwise,
	//		wkday   0-6
	//		nthwk   no constraint
	{
		// validate parameters w.r.t. ranges
		assert(yr >= 1969);	// Last few hours of 1969 might be a valid local time
		assert(wkday <= 6);
		assert(1 <= mo && mo <= 12);

		// This computation is tricky because adding whole days to a time_t
		// may result in date within the DST zone, which, when rendered into
		// calendar date form, appears off by the daylight bias. Rather, we
		// need to work in UTC calendar days and  add integer calendar days to
		// the first-of-month epoch in the given year to yield the desired
		// date.  To start, compute the first of the month in the given year
		// at the given hour, minute, and  second.
		time_tm atm = {(int)sec, (int)min, (int)hr, (int)1, (int)(mo - 1),
			(int)(yr - 1900), (int)0, (int)0, nDST};

		// get the (valid) local time of the UTC time corresponding to this
		time_t t1st = UTCtime(&atm);

		// recover the day of the week
		time_tm* ptm1 = ::gmtime(&t1st);
		assert(ptm1);

		// Compute number of days until the nthwk occurrence of wkday
		int nthwkday = (7 + wkday - ptm1->tm_wday) % 7 + (nthwk - 1) * 7;

		// add this to the first of the month
		int sec_per_day = 86400;
		time_t tnthwkdy = t1st + nthwkday * sec_per_day;
		ptm1 = ::gmtime(&tnthwkdy);
		assert(ptm1);
		
		// compute the object time_t
		ptm1->tm_isdst = nDST;
		m_time = ::mktime(ptm1);
		assert(m_time != -1);
	}

	//============================================================================
	inline CTime::CTime(UINT year, UINT month, UINT day, UINT hour, UINT min,
		UINT sec, int nDST /* = -1 */)
	//	Construct a CTime object from local time elements. Each element is
	//	constrained to lie within the following UTC ranges:
	//		year 		1970–2038 (on 32-bit systems)
	//		month 		1–12
	//		day 		1–31
	//		hour, min, sec	no constraint
	// 
	//   Note:
	//      These valid ranges apply to UTC time. The local time zone might
	//      be shifted up to + or - 11 hours from UTC time. Hence the last
	//	    few hours of 1969 (local time) might be a valid time.

	//	The nDST value indicates whether daylight savings time is in effect.
	//	It can have one of three values, as follows:
	//              nDST == 0   Standard time is in effect.
	//              nDST >  0   Daylight savings time is in effect.
	//              nDST <  0   The default. Automatically computes whether
	//			    standard time or daylight time is in effect.
	{
		// validate parameters w.r.t. ranges
		assert(1 <= day && day   <= 31);
		assert(1 <= month && month <= 12);
		assert(year >= 1969);  // Last few hours of 1969 might be a valid local time

		// fill out a time_tm with the calendar date
		time_tm atm = {(int)sec, (int)min, (int)hour, (int)day,
			(int)(month - 1), (int)(year - 1900), (int)0, (int)0, nDST};

		// compute the object time_t
		m_time = ::mktime(&atm);
		assert(m_time != -1);
	}

	//============================================================================
	inline CTime::CTime(UINT yr, UINT doy, UINT hr, UINT min, UINT sec, int nDST /* = -1 */)
	//	Construct a CTime using the day-of-year doy, where doy = 1 is
	//	January 1 in the specified year.  Restrictions on yr, hr, min, and  sec
	//	are the same as in CTime(yr, mo, da, hr, min, sec, nDST). There is no
	//	constraint on doy.
	{
		// validate parameters w.r.t. ranges
		assert(yr >= 1969);  // Last few hours of 1969 might be a valid local time

		// fill out a time_tm with the calendar date for Jan 1, yr, hr:min:sec
		time_tm atm1st = {(int)sec, (int)min, (int)hr, (int)1,
			(int)0, (int)(yr - 1900), (int)0, (int)0, nDST};

		// get the local time of the UTC time corresponding to this
		time_t Jan1 = UTCtime(&atm1st);
		int sec_per_day = 86400;
		time_t tDoy = Jan1 + (doy - 1) * sec_per_day;	
		time_tm* ptm = ::gmtime(&tDoy);
		assert(ptm);
		
		// compute the object time_t
		ptm->tm_isdst = nDST;
		m_time = ::mktime(ptm);
		assert(m_time != -1);
	}

	//============================================================================
	inline CTime::CTime(WORD wDosDate, WORD wDosTime, int nDST /* = -1 */)
	//	Construct a CTime object from the MS-DOS wDosDate and wDosTime values.
	//	These are formats used by MS-DOS. The date is a packed 16-bit value
	//	in which bits in the value represent the day, month, and years past 1980. The
	//	time is a packed 16-bit value in which bits in the value represent
	//	the hour, minute, and  second.
	{
		FILETIME ft;
		VERIFY( ::DosDateTimeToFileTime(wDosDate, wDosTime, &ft) );
		CTime t(ft, nDST);
		m_time = t.m_time;
	}

	//============================================================================
	inline CTime::CTime(const SYSTEMTIME& st, int nDST /* = -1 */)
	//	 Construct a CTime object from a SYSTEMTIME structure st.
	{
		CTime t((UINT)st.wYear, (UINT)st.wMonth, (UINT)st.wDay, (UINT)st.wHour,
			(UINT)st.wMinute, (UINT)st.wSecond, nDST); // asserts if invalid

		m_time = t.m_time;
	}

	//============================================================================
	inline CTime::CTime(const FILETIME& ft, int nDST /* = -1 */)
	//	Construct a CTime object from a (UTC) FILETIME structure ft.
	{
		// start by converting ft (a UTC time) to local time
		FILETIME localTime;
		VERIFY( ::FileTimeToLocalFileTime(&ft, &localTime) );

		//  convert localTime to a SYSTEMTIME structure
		SYSTEMTIME st;
		VERIFY( ::FileTimeToSystemTime(&localTime, &st) );

		// then convert the system time to a CTime
		CTime t(st, nDST);  // asserts if invalid
		m_time = t.m_time;
	}

	//============================================================================
	inline bool CTime::GetAsFileTime(FILETIME& ft) const
	//	Convert *this CTime object into a FILETIME structure and  store it
	//	in st. The FILETIME data structure initialized by this function will
	//	reflect the UTC time of this object. Return true if successful.
	{
		bool rval = false;
		time_tm* ptm = GetGmtTm();
		assert(ptm != NULL);

		if (ptm)
		{
			SYSTEMTIME st = {(WORD)(1900 + ptm->tm_year), (WORD)(1 + ptm->tm_mon),
				(WORD)ptm->tm_wday, (WORD)ptm->tm_mday, (WORD)ptm->tm_hour,
				(WORD)ptm->tm_min, (WORD)ptm->tm_sec, (WORD)0};
			SystemTimeToFileTime(&st, &ft);
			rval = true;
		}
		
		return rval;
	}

	//============================================================================
	inline bool CTime::GetAsSystemTime(SYSTEMTIME& st) const
	//	Convert *this CTime object into a SYSTEMTIME structure and  store it
	//	in st. The SYSTEMTIME data structure initialized by this function will
	//	reflect the local time of this object and  will have its wMilliseconds
	//	member set to zero. Return true if successful.
	{
		bool rval = false;
		time_tm* ptm = GetLocalTm();
		assert(ptm != NULL);

		if (ptm)
		{
			st.wYear 	= (WORD) (1900 + ptm->tm_year);
			st.wMonth 	= (WORD) (1 + ptm->tm_mon);
			st.wDayOfWeek 	= (WORD) ptm->tm_wday;
			st.wDay 	= (WORD) ptm->tm_mday;
			st.wHour 	= (WORD) ptm->tm_hour;
			st.wMinute 	= (WORD) ptm->tm_min;
			st.wSecond 	= (WORD) ptm->tm_sec;
			st.wMilliseconds = 0;
			rval = true;
		}

		return rval;
	}

	//============================================================================
	inline time_tm* CTime::GetGmtTm(time_tm* ptm) const
	//	Return a pointer to a time_tm that contains a decomposition of *this
	//	CTime object expressed in UTC. If ptm is non NULL, this decomposition
	//	is also copied into ptm.
	{
		if (ptm != NULL)
		{
			time_tm* ptmTemp = ::gmtime(&m_time);
			if (ptmTemp == NULL)
				return NULL;    // the m_time was not initialized!

			*ptm = *ptmTemp;
			return ptm;
		}
		else
			return ::gmtime(&m_time);
	}

	//============================================================================
	inline time_tm* CTime::GetLocalTm(time_tm* ptm) const
	//	Return a pointer to a time_tm that contains a decomposition of *this
	//	CTime object expressed in the local time base. If ptm is non NULL, this
	//	decomposition is also copied into ptm.
	{
		if (ptm != NULL)
		{
			time_tm* ptmTemp = ::localtime(&m_time);
			if (ptmTemp == NULL)
				return NULL;    // the m_time was not initialized!

			*ptm = *ptmTemp;
			return ptm;
		}
		else
			return ::localtime(&m_time);
	}

	//============================================================================
	inline time_t  CTime::GetTime() const
	//	Return *this time as a time_t value.
	{
		return m_time;
	}

	//============================================================================
	inline int	CTime::GetYear(bool local /* = true */) const
	//	Return the year of *this time object, local (true) or UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return 1900 + ptmbuffer->tm_year;
	}

	//============================================================================
	inline int	CTime::GetMonth(bool local /* = true */) const
	//	Return the month of *this time object (1 through 12), local (true)  or
	//	UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return ptmbuffer->tm_mon + 1;
	}

	//============================================================================
	inline int	CTime::GetDay(bool local /* = true */) const
	//	Return the day of *this object (1 through 31), local (true)  or
	//	UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return ptmbuffer->tm_mday ;
	}

	//============================================================================
	inline int	CTime::GetHour(bool local /* = true */) const
	//	Return the hour of *this object (0 through 23), local (true)  or
	//	UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return ptmbuffer->tm_hour;
	}

	//============================================================================
	inline int	CTime::GetMinute(bool local /* = true */) const
	//	Return the minute of *this object (0 through 59), local (true)  or
	//	UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return ptmbuffer->tm_min;
	}

	//============================================================================
	inline int	CTime::GetSecond(bool local /* = true */) const
	//	Return the second of *this object (0 through 61), local (true)  or
	//	UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return ptmbuffer->tm_sec;
	}

	//============================================================================
	inline int	CTime::GetDayOfWeek(bool local /* = true */) const
	//	Return the day of the week of *this object (0–6, Sunday = 0), local
	//	(true) or UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return ptmbuffer->tm_wday;
	}

	//============================================================================
	inline int	CTime::GetDayOfYear(bool local /* = true */) const
	//	Return the day of the year of *this object (1-366), local (true)  or
	//	UTC (false).
	{
		time_tm* ptmbuffer = (local ? GetLocalTm(NULL) : GetGmtTm());
		return ptmbuffer->tm_yday + 1;
	}

	//============================================================================
	inline CTime& CTime::operator=(const CTime& t)
	//	Assign the CTime t value to *this
	{
		// Self assignment is safe
		m_time = t.m_time;
		return *this;
	}

	//============================================================================
	inline CTime& CTime::operator=(const time_t& t)
	//	Assign the time_t value to *this time.
	{
		assert(t >= 0);
		// Self assignment is safe
		m_time = t;
		return *this;
	}

	//============================================================================
	inline const CTimeSpan CTime::operator-(const CTime& t) const
	//	Return the time span between *this time and  time t
	{
		timespan_t d = (timespan_t)(m_time - t.m_time);
		CTimeSpan t0(d);
		return  t0;
	}

	//============================================================================
	inline const CTime CTime::operator-(const CTimeSpan& ts) const
	//	Return the CTime that is the time span ts before *this time.
	{
		time_t d = m_time - ts.m_timespan;
		CTime t(d);  // asserts if d is invalid
		return t;
	}

	//============================================================================
	inline const CTime CTime::operator+(const CTimeSpan& ts) const
	//	Return the CTime that is the time span ts after *this time.
	{
		time_t s = m_time + ts.m_timespan;
		CTime t(s); // asserts if s is invalid
		return t;
	}

	//============================================================================
	inline CTime& CTime::operator+=(const CTimeSpan& ts)
	//	Increment *this time by the time span ts and  return this CTime.
	{
		m_time += ts.m_timespan;
		assert(m_time >= 0); // Invalid addition to time object.
		return *this;
	}

	//============================================================================
	inline CTime& CTime::operator-=(const CTimeSpan& ts)
	//	Decrement *this time by the time span ts and  return this CTime.
	{
		m_time -= ts.m_timespan;
		assert(m_time >= 0); //Invalid subtraction from time object.
		return *this;
	}

	//============================================================================
	inline bool CTime::operator==(const CTime& t) const
	//	Return true if *this and t are the same times.
	{
		return m_time == t.m_time;
	}

	//============================================================================
	inline bool CTime::operator!=(const CTime& t) const
	//	Return true if *this and t are not the same times.
	{
		return m_time != t.m_time;
	}

	//============================================================================
	inline bool CTime::operator<(const CTime& t) const
	//	Return true if *this time is less than time t.
	{
		return m_time < t.m_time;
	}

	//============================================================================
	inline bool CTime::operator>(const CTime& t) const
	//	Return true if *this time is greater than time t.
	{
		return m_time > t.m_time;
	}

	//============================================================================
	inline bool CTime::operator<=(const CTime& time) const
	//	Return true if *this time is less than or equal to time t.
	{
		return m_time <= time.m_time;
	}

	//============================================================================
	inline bool CTime::operator>=(const CTime& time) const
	//	Return true if *this time is greater than or equal to time t.
	{
		return m_time >= time.m_time;
	}

	//============================================================================
	inline CString CTime::Format(LPCTSTR pFormat) const
	//	Create a formatted representation of this date/time value as a local
	//	time. If  this CTime object is null or invalid, the return value is
	//	an empty CString. The pFormat string is converted using the conventions
	//	of the C function strftime(). Consult the C++ reference for details.
	{
		const size_t  maxTimeBufferSize = 128;
		TCHAR szBuffer[maxTimeBufferSize];

		time_tm* ptm = ::localtime(&m_time);
		if (ptm == NULL || !::_tcsftime(szBuffer, maxTimeBufferSize, pFormat, ptm))
			szBuffer[0] = '\0';
		return CString(szBuffer);
	}

	//============================================================================
	inline CString CTime::Format(UINT nFormatID) const
	//	Create a formatted representation of this date/time value as a local
	//	time, in the same manner as an LPCTSTR argument, but nFormatID
	//	identifies a resource string.
	{
		CString strFormat;
		VERIFY( strFormat.LoadString(nFormatID) );
		return Format(strFormat);
	}

	//============================================================================
	inline CString CTime::FormatGmt(LPCTSTR pFormat) const
	//	Create a formatted representation of this date/time value as a UTC
	//	time. If  this CTime object is null or invalid, the return value is
	//	an empty CString. The pFormat string is converted using the conventions
	//	of the C function strftime(). Consult the C++ reference for details.
	{
		const size_t  maxTimeBufferSize = 128;
		TCHAR szBuffer[maxTimeBufferSize];
		CString fmt0 = pFormat;
		while (fmt0.Replace(_T("%Z"), _T("Coordinated Universal Time")))
			;
		while (fmt0.Replace(_T("%z"), _T("UTC")))
			;

		time_tm* ptmTemp = GetGmtTm();
		if (ptmTemp ==NULL || !::_tcsftime(szBuffer, maxTimeBufferSize, fmt0.c_str(), ptmTemp))
			szBuffer[0] = '\0';
		return CString(szBuffer);
	}

	//============================================================================
	inline CString CTime::FormatGmt(UINT nFormatID) const
	//	Create a formatted representation of this date/time value as a UTC
	//	time, in the same manner as an LPCTSTR argument, but nFormatID
	//	identifies a resource string.
	{
		CString strFormat;
		VERIFY( strFormat.LoadString(nFormatID) );
		return FormatGmt(strFormat);
	}

	//
	//	Static and  Friend Functions
	//

	//============================================================================
	inline CTime CTime::GetCurrentTime()
	//	This is a static method that return the current system time as a
	//	CTime object.
	{
		return CTime(::time(NULL));
	}

	//============================================================================
	inline FILETIME CTime::FileTimePlus(const FILETIME& ft, double addend)
	//	This is a static function to return a FILETIME structure containing the
	//	FILETIME ft increased by addend seconds.
	{
		// convert ft to unsigned long long
		ULONGLONG ftlong = (ULONGLONG)
			(ft.dwHighDateTime) << 32 | ft.dwLowDateTime;
		BOOL sign = FALSE;
		if (addend < 0.)
		{
			sign = TRUE;
			addend = -addend;
		}
		ULONGLONG ftaddend = (ULONGLONG)(addend * 10000000);
		if (sign)
		{
			assert(ftlong >= ftaddend); //FILETIME addition underflow.
			ftlong -= ftaddend;
		}
		else
			ftlong += ftaddend;
		FILETIME fts;
		fts.dwHighDateTime = (DWORD)(ftlong >> 32);
		fts.dwLowDateTime  = (DWORD)(ftlong & ~0);
		return fts;
	}

	//
	// Global functions within the Win32xx namespace
	//

	//============================================================================
	inline CArchive& operator>>(CArchive& ar, CTime& t)
	//	Read a CTime from the archive and  store it in t.  Throw an exception if
	//	unable to do so correctly.
	{
		UINT size;
		ar.Read(&size, sizeof(UINT));
		if (size != sizeof(ULONGLONG))
		{
			CString str = ar.GetFile().GetFilePath();
			throw CFileException(str, _T("Failed to read CTime from archive."));
		}

		// load CTime as x64
		ULONGLONG tx64 = 0;
		ar.Read(&tx64, size);
		time_t tt = (time_t)tx64;
		t = CTime(tt);
		return ar;
	}

	//============================================================================
	inline CArchive& operator<<(CArchive& ar, CTime& t)
	//	Write the time t into the archive file. Throw an exception if an
	//	error occurs.
	{
		ULONGLONG tx64 = 0;
		UINT size = sizeof(tx64);
		
		// store CTime as x64
		time_t tt = t;
		tx64 = tt;
		ar.Write(&tx64, size);
		return ar;
	}


	///////////////////////////////////////////////////////////////
	//
	//	CTimeSpan class implementation
	//
	///////////////////////////////////////////////////////////////

	//============================================================================
	inline CTimeSpan::CTimeSpan()
	//	Construct an CTimeSpan object initialized to 0.
	{
		m_timespan = 0;
	}

	//============================================================================
	inline CTimeSpan::CTimeSpan(timespan_t t)
	//	Construct a CTimeSpan object from time_t type t. This value is the
	//	difference between two absolute time_t values.
	{
		m_timespan = t;
	}

	//============================================================================
	inline CTimeSpan::CTimeSpan(long lDays, int nHours, int nMins, int nSecs)
	//	Construct a CTimeSpan object from given time elements, where each
	//	element is constrained to the following ranges:
	//		lDays 	0–24,855 (approximately)
	//		nHours 	0–23
	//		nMins 	0–59
	//		nSecs 	0–59
	{
		int sec_per_day  = 86400;
		int sec_per_hour = 3600;
		int sec_per_min  = 60;
		m_timespan = lDays * sec_per_day + nHours * sec_per_hour +
			nMins * sec_per_min + nSecs;
	}

	//============================================================================
	inline CTimeSpan::CTimeSpan(const CTimeSpan& ts)
	//	Construct a CTimeSpan object from another CTimeSpan value.
	{
		m_timespan = ts.m_timespan;
	}

	//============================================================================
	inline LONGLONG CTimeSpan::GetDays() const
	//	Return the number of complete days in this CTimeSpan.  This value may
	//	be negative if the time span is negative.
	{
		int sec_per_day = 86400;
		return m_timespan / sec_per_day;
	}

	//============================================================================
	inline LONGLONG CTimeSpan::GetTotalHours() const
	//	Return the total number of complete hours in this CTimeSpan.
	{
		int sec_per_hour = 3600;
		return m_timespan / sec_per_hour;
	}

	//============================================================================
	inline LONGLONG CTimeSpan::GetTotalMinutes() const
	//	Return the total number of complete minutes in this CTimeSpan.
	{
		int sec_per_min = 60;
		return m_timespan / sec_per_min;
	}

	//============================================================================
	inline LONGLONG CTimeSpan::GetTotalSeconds() const
	//	Return the total number of complete seconds in this CTimeSpan.
	{
		return m_timespan;
	}

	//============================================================================
	inline int CTimeSpan::GetHours() const
	//	Return the number of hours in the day component of this time
	//	span (–23 through 23).
	{
		int sec_per_hour  = 3600;
		int hours_per_day = 24;
		return static_cast<int>((m_timespan / sec_per_hour) % hours_per_day);
	}

	//============================================================================
	inline int CTimeSpan::GetMinutes() const
	//	Return the number of minutes in the hour component of this time
	//	span (–59 through 59).
	{
		int sec_per_min = 60;
		return static_cast<int>((m_timespan / sec_per_min) % sec_per_min);
	}

	//============================================================================
	inline int CTimeSpan::GetSeconds() const
	//	Return the number of seconds in the minute component of this time
	//	span (–59 through 59).
	{
		int sec_per_min = 60;
		return static_cast<int>(m_timespan % sec_per_min);
	}

	//============================================================================
	inline CTimeSpan& CTimeSpan::operator=(const CTimeSpan& ts)
	//	Assign the CTimeSpan ts to *this time span
	{
		// Self assignment is safe
		m_timespan = ts.m_timespan;
		return *this;
	}

	//============================================================================
	inline CTimeSpan& CTimeSpan::operator=(const timespan_t& t)
	//	Assign the timespan_t t value to *this time span.
	{
		// Self assignment is safe
		m_timespan = t;
		return *this;
	}

	//============================================================================
	inline const CTimeSpan CTimeSpan::operator-() const
	//	Return the negated value of *this time span.
	{
		CTimeSpan t0(-m_timespan);
		return  t0;
	}

	//============================================================================
	inline const CTimeSpan CTimeSpan::operator-(CTimeSpan& ts) const
	//	Return the result of subtracting the time span ts from *this time span.
	{
		timespan_t d = m_timespan - ts.m_timespan;
		CTimeSpan tsp(d);
		return tsp;
	}

	//============================================================================
	inline const CTimeSpan CTimeSpan::operator+(CTimeSpan& ts) const
	//	Return the result of adding the time span ts to *this time span.
	{
		timespan_t s = m_timespan + ts.m_timespan;
		CTimeSpan tsp(s);
		return tsp;
	}

	//============================================================================
	inline CTimeSpan& CTimeSpan::operator+=(CTimeSpan& ts)
	//	Increase *this time span by the ts time span and  return the result.
	{
		m_timespan += ts.m_timespan;
		return *this;
	}

	//============================================================================
	inline CTimeSpan& CTimeSpan::operator-=(CTimeSpan& ts)
	//	Decrease *this time span by the ts time span and  return the result.
	{
		m_timespan -= ts.m_timespan;
		return *this;
	}

	//============================================================================
	inline bool CTimeSpan::operator==(const CTimeSpan& ts) const
	//	Return true if the time span ts equals *this one.
	{
		return m_timespan == ts.m_timespan;
	}

	//============================================================================
	inline bool CTimeSpan::operator!=(const CTimeSpan& ts) const
	//	Return true if the time span ts does not equal *this one.
	{
		return m_timespan != ts.m_timespan;
	}

	//============================================================================
	inline bool CTimeSpan::operator<(const CTimeSpan& ts) const
	//	Return true if *this time span is less than the ts time span.
	{
		return m_timespan < ts.m_timespan;
	}

	//============================================================================
	inline bool CTimeSpan::operator>(const CTimeSpan& ts) const
	//	Return true if *this time span is greater than the ts time span.
	{
		return m_timespan > ts.m_timespan;
	}

	//============================================================================
	inline bool CTimeSpan::operator<=(const CTimeSpan& ts) const
	//	Return true if *this time span is less than or equal to the ts
	//	time span.
	{
		return m_timespan <= ts.m_timespan;
	}

	//============================================================================
	inline bool CTimeSpan::operator>=(const CTimeSpan& ts) const
	//	Return true if *this time span is greater than or equal to the ts
	//	time span.
	{
		return m_timespan >= ts.m_timespan;
	}

	//============================================================================*/
	inline CString CTimeSpan::Format(LPCTSTR pFormat) const
	//	Return a rendering of *this CTimeSpan object in CString form using the
	//	pFormat as the template. The valid format directives are
	//	      %D - number of days
	//	      %H - hour (0-23)
	//	      %M - minute (0-59)
	//	      %S - seconds (0-59)
	{
		CString fmt0 = pFormat;
		CString insert;

		while (fmt0.Find(_T("%D")) != -1)  // number of days
		{
			insert.Format(_T("%ld"), GetDays());
			fmt0.Replace(_T("%D"), insert);
		}
		while (fmt0.Find(_T("%H")) != -1)  // hours (00 – 23)
		{
			insert.Format(_T("%02d"), GetHours());
			fmt0.Replace(_T("%H"), insert);
		}
		while (fmt0.Find(_T("%M")) != -1)  // minutes (00 – 59)
		{
			insert.Format(_T("%02d"), GetMinutes());
			fmt0.Replace(_T("%M"), insert);
		}
		while (fmt0.Find(_T("%S")) != -1)  // seconds (00 - 59)
		{
			insert.Format(_T("%02d"), GetSeconds());
			fmt0.Replace(_T("%S"), insert);
		}
		return fmt0;
	}

	//============================================================================
	inline CString CTimeSpan::Format(UINT nFormatID) const
	//	Return a rendering of *this CTimeSpan object in CString form using the
	//	string resource having the nFormatID identifier as the template. The
	//	valid format directives are
	//	      %D - number of days
	//	      %H - hour (0-23)
	//	      %M - minute (0-59)
	//	      %S - seconds (0-59)
	{
		CString strFormat;
		VERIFY( strFormat.LoadString(nFormatID) );
		return Format(strFormat);
	}


	//
	// Global functions within the Win32xx namespace
	//

	//============================================================================
	inline CArchive& operator>>(CArchive& ar, CTimeSpan& ts)
	//	Read a CTimeSpan object from the archive and  store it in t.  Throw an
	//	exception if unable to do so correctly.
	{
		UINT size;
		ar.Read(&size, sizeof(size));
		if (size != sizeof(ULONGLONG))
		{
			CString str = ar.GetFile().GetFilePath();
			throw CFileException(str, _T("Failed to read CTimeSpan from archive"));
		}
		
		// load CTimeSpan as x64
		ULONGLONG tsx64 = 0;
		ar.Read(&tsx64, size);
		timespan_t tst = (timespan_t)tsx64;
		ts = CTimeSpan(tst);
		return ar;
	}

	//============================================================================
	inline CArchive& operator<<(CArchive &ar, CTimeSpan& ts)
	//	Write the time span object s into the archive file. Throw an exception
	//	if an error occurs.
	{
		ULONGLONG tsx64 = 0;
		UINT size = sizeof(tsx64);
		ar.Write(&size, sizeof(size));
				
		// store CTimeSpan as x64
		tsx64 = ts;
		ar.Write(&tsx64, size);
		return ar;
	}

} // namespace Win32XX


#endif // _WIN32XX_CTIME_H_

