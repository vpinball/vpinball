// Win32++   Version 9.1
// Release Date: 26th September 2022
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//
//
// Copyright (c) 2005-2022  David Nash
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
//  Declaration of the CTime class
//
////////////////////////////////////////////////////////

#ifndef _WIN32XX_CTIME_H_
#define _WIN32XX_CTIME_H_

#include "wxx_wincore.h"
#include "wxx_archive.h"
#include <errno.h>
#include <time.h>


//  The CTime class is based internally on the time_t data type, which is
//  measured in seconds past the January 1, 00:00:00 1970 UTC epoch. Thus,
//  any given time_t instant is strictly a UTC epoch, which may then be
//  rendered, using standard C library functions, as either a UTC or local
//  time.  Local instants may further appear as standard or daylight times.
//  Thus, the base epoch on a system in the PST zone will be displayed as
//  December 31, 1969 16:00:00. Any attempt to construct or evaluate a date
//  before this epoch will assert or be in error. Even though the time_t
//  type is implemented (in MinGW, anyway) as a signed integer type,
//  negative values are not allowed.

//  On systems where time_t is defined as a 32-bit integer, there is an
//  upper date limit of January 18, 19:14:07, 2038. On 64-bit systems,
//  there is no such upper date limit.

//  Windows also has other time types that also interface with the CTime
//  type. These are FILETIME, SYSTEMTIME, and MS-DOS date and time, each
//  of which are described in the MSDN API documentation. Basically:

//  FILETIME is a structure containing a 64-bit value representing the
//  number of 100-nanosecond intervals since January 1, 1601 UTC.

//  SYSTEMTIME is a structure representing a date and  time using individual
//  values for the month, day, year, weekday, hour, minute, second, and
//  millisecond. This is not useful in itself, but must be translated into
//  a FILETIME or time_t.

//  MS-DOS date and time are two 16-bit words packed with day, month, year
//  and second, minute, hour fields.  The format may be found in the MSDN
//  Library article on DosDateTimeToFileTime().



namespace Win32xx
{


    // define the time_tm type.
    typedef struct tm time_tm;

    // define the timespane_t type.
    // This can be int or __int64 depending on the compiler
    typedef time_t timespan_t;

    // forward declaration
    class CTimeSpan;

    // Declaration of a global time conversion function.
    time_t UTCtime(time_tm *atm);


    ////////////////////////////////////////////////////////
    // The CTime class represents an absolute time and date.
    class CTime
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
        CTime(UINT yr, UINT mo, UINT wkday, UINT nthwk, UINT hr, UINT min, UINT sec, int isDST = -1);
        CTime(UINT year, UINT month, UINT day, UINT hour, UINT min, UINT sec, int isDST = -1);
        CTime(UINT yr, UINT doy, UINT hr, UINT min, UINT sec, int isDST = -1);
        CTime(WORD dosDate, WORD dosTime, int isDST = -1);
        CTime(const SYSTEMTIME& st, int isDST = -1);
        CTime(const FILETIME& ft,  int isDST = -1);

        // Method members
        bool      GetAsFileTime(FILETIME& ft) const;
        bool      GetAsSystemTime(SYSTEMTIME& st) const;
        int       GetDay(bool local = true) const;
        int       GetDayOfWeek(bool local = true) const;
        int       GetDayOfYear(bool local = true) const;
        time_tm*  GetGmtTm(time_tm* ptm) const;
        int       GetHour(bool local = true) const;
        time_tm*  GetLocalTm(time_tm* ptm) const;
        int       GetMinute(bool local = true) const;
        int       GetMonth(bool local = true) const;
        int       GetSecond(bool local = true) const;
        int       GetYear(bool local = true) const;
        time_t    GetTime() const;

        // Assignment operators
        CTime&  operator=(const CTime& timeSrc);
        CTime&  operator=(const time_t& t);

        // Computational operators
        CTime&  operator+=(const CTimeSpan& ts);
        CTime&  operator-=(const CTimeSpan& ts);
        const CTimeSpan operator-(const CTime& t) const;
        const CTime operator-(const CTimeSpan& ts) const;
        const CTime operator+(const CTimeSpan& ts) const;
        bool        operator==(const CTime& t) const;
        bool        operator!=(const CTime& t) const;
        bool        operator<(const CTime& time) const;
        bool        operator>(const CTime& time) const;
        bool        operator<=(const CTime& time) const;
        bool        operator>=(const CTime& time) const;

        operator time_t() const { return m_time; }

        // CString conversion
        CString     Format(LPCTSTR format) const;
        CString     Format(UINT formatID) const;
        CString     FormatGmt(LPCTSTR format) const;
        CString     FormatGmt(UINT formatID) const;

        // Static methods
        static  CTime   GetCurrentTime();

    private:

        // private data members
        time_t      m_time;
    };


    ////////////////////////////////////////////////////////////
    // The CTimeSpan class defines the data type for differences
    // between two CTime values, measured in seconds of time.
    class CTimeSpan
    {
        friend class CTime;     // CTime can access private members

    public:
        // Constructors
        CTimeSpan();
        CTimeSpan(timespan_t t);
        CTimeSpan(long days, int hours, int mins, int secs);
        CTimeSpan(const CTimeSpan& ts);

        // Methods to extract items
        LONGLONG    GetDays() const;
        LONGLONG    GetTotalHours() const;
        int         GetHours() const;
        LONGLONG    GetTotalMinutes() const;
        int         GetMinutes() const;
        LONGLONG    GetTotalSeconds() const;
        int         GetSeconds() const;

        // assignment operators
        CTimeSpan& operator=(const CTimeSpan& ts);
        CTimeSpan& operator=(const timespan_t& t);

        // computational operators
        const CTimeSpan operator-() const;
        const CTimeSpan operator-(CTimeSpan& ts) const;
        const CTimeSpan operator+(CTimeSpan& ts) const;
        CTimeSpan& operator+=(CTimeSpan& ts);
        CTimeSpan& operator-=(CTimeSpan& ts);
        bool       operator==(const CTimeSpan& ts) const;
        bool       operator!=(const CTimeSpan& ts) const;
        bool       operator<(const CTimeSpan& ts) const;
        bool       operator>(const CTimeSpan& ts) const;
        bool       operator<=(const CTimeSpan& ts) const;
        bool       operator>=(const CTimeSpan& ts) const;

        operator timespan_t() const { return m_timespan; }

        // CString conversion
        CString     Format(LPCTSTR format) const;
        CString     Format(UINT formatID) const;

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

    // Returns the time_t t corresponding to the date given in atm as a UTC time.
    inline time_t UTCtime(time_tm* atm)
    {
        // compute the local time from atm
        assert(atm != NULL);           // atm must exist
        time_t t0 = ::mktime(atm);     // atm = *localtime(t0)
        assert(t0 != -1);
#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )  // not VS or VS < 2005
        time_tm* ptm0 = ::gmtime(&t0); // atm0 = UTC time of atm
#else
        time_tm tm0;
        time_tm* ptm0 = &tm0;
        ::gmtime_s(ptm0, &t0);         // atm0 = UTC time of atm
#endif
        time_t t1 = ::mktime(ptm0);    // atm0 = localtime(t1)
        assert(t1 != -1);
        timespan_t zt = static_cast<timespan_t>(t0 - t1);  // time zone bias
        t0 += zt;

#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )  // not VS or VS < 2005
        assert(::gmtime(&t0));
#else
        assert(::gmtime_s(ptm0, &t0) == 0);
#endif

        return t0;
    }

    // Constructs an CTime object initialized to the Jan 1, 1970 00:00:00 epoch.
    inline CTime::CTime()
    {
        m_time = 0;
    }

    // Constructs a CTime object from another (valid) CTime object t.
    inline CTime::CTime(const CTime& t)
    {
        m_time = t.m_time;
    }

    // Constructs a CTime object from the time_t value t, or assert if t is invalid.
    inline CTime::CTime(time_t t)
    {

#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )  // not VS or VS < 2005
        assert(::gmtime(&t));
#else
        time_tm tm;
        ::gmtime_s(&tm, &t);
#endif
        m_time = t;
    }

    // Constructs a CTime object from the time_tm atm, or assert if atm is invalid
    inline CTime::CTime(time_tm& atm)
    {
        // compute the object time_t
        m_time = ::mktime(&atm);

        // check for acceptable range
        assert(m_time != -1);
    }

    // Constructs a CTime of the nthwk occurrence of the given wkday (0..6)
    // in the mo month of yr year, at hr:min:sec of that day, local time.
    inline CTime::CTime(UINT yr, UINT mo, UINT wkday, UINT nthwk, UINT hr,
        UINT min, UINT sec, int isDST /* = -1 */)
    {
        // validate parameters w.r.t. ranges
        assert(yr >= 1969); // Last few hours of 1969 might be a valid local time
        assert(wkday <= 6);
        assert(1 <= mo && mo <= 12);

        // This computation is tricky because adding whole days to a time_t
        // may result in date within the DST zone, which, when rendered into
        // calendar date form, appears off by the daylight bias. Rather, we
        // need to work in UTC calendar days and  add integer calendar days to
        // the first-of-month epoch in the given year to yield the desired
        // date.  To start, compute the first of the month in the given year
        // at the given hour, minute, and  second.
        time_tm atm = { static_cast<int>(sec), static_cast<int>(min), static_cast<int>(hr),
                       1, static_cast<int>(mo - 1), static_cast<int>(yr - 1900), 0, 0, isDST};

        // get the (valid) local time of the UTC time corresponding to this
        time_t t1st = UTCtime(&atm);

        // recover the day of the week

#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )  // not VS or VS < 2005
        time_tm* ptm1 = ::gmtime(&t1st);
        assert(ptm1);
#else
        time_tm tm1;
        time_tm* ptm1 = &tm1;
        gmtime_s(ptm1, &t1st);
#endif

        // Compute number of days until the nthwk occurrence of wkday
        time_t nthwkday = (7 + time_t(wkday) - ptm1->tm_wday) % 7 + time_t(nthwk - 1) * 7;

        // add this to the first of the month
        time_t sec_per_day = 86400;
        time_t tnthwkdy = t1st + nthwkday * sec_per_day;
#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )
        ptm1 = ::gmtime(&tnthwkdy);
        assert(ptm1);
#else
        VERIFY( ::gmtime_s(ptm1, &tnthwkdy) == 0);
#endif

        // compute the object time_t
        ptm1->tm_isdst = isDST;
        m_time = ::mktime(ptm1);
        assert(m_time != -1);
    }

    // Constructs a CTime object from local time elements. Each element is
    // constrained to lie within the following UTC ranges:
    //   year       1970-2038 (on 32-bit systems)
    //   month      1-12
    //   day        1-31
    //   hour, min, sec no constraint
    inline CTime::CTime(UINT year, UINT month, UINT day, UINT hour, UINT min,
        UINT sec, int isDST /* = -1 */)
    {
        // validate parameters w.r.t. ranges
        assert(1 <= day && day   <= 31);
        assert(1 <= month && month <= 12);
        assert(year >= 1969);  // Last few hours of 1969 might be a valid local time

        // fill out a time_tm with the calendar date
        time_tm atm = {static_cast<int>(sec), static_cast<int>(min), static_cast<int>(hour),
            static_cast<int>(day), static_cast<int>(month - 1), static_cast<int>(year - 1900),
            0, 0, isDST};

        // compute the object time_t
        m_time = ::mktime(&atm);
        assert(m_time != -1);
    }

    // Constructs a CTime using the day-of-year doy, where doy = 1 is
    // January 1 in the specified year.
    inline CTime::CTime(UINT yr, UINT doy, UINT hr, UINT min, UINT sec, int isDST /* = -1 */)
    {
        // validate parameters w.r.t. ranges
        assert(yr >= 1969);  // Last few hours of 1969 might be a valid local time

        // fill out a time_tm with the calendar date for Jan 1, yr, hr:min:sec
        time_tm atm1st = {static_cast<int>(sec), static_cast<int>(min), static_cast<int>(hr),
            1, 0, static_cast<int>(yr - 1900), 0, 0, isDST};

        // get the local time of the UTC time corresponding to this
        time_t Jan1 = UTCtime(&atm1st);
        time_t sec_per_day = 86400;
        time_t tDoy = Jan1 + (doy - 1) * sec_per_day;
#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )  // not VS or VS < 2005
        time_tm* ptm = ::gmtime(&tDoy);
        assert(ptm);
#else
        time_tm tm;
        time_tm* ptm = &tm;
        ::gmtime_s(ptm, &tDoy);
#endif

        // compute the object time_t
        ptm->tm_isdst = isDST;
        m_time = ::mktime(ptm);
        assert(m_time != -1);
    }

    // Constructs a CTime object from the MS-DOS dosDate and dosTime values.
    inline CTime::CTime(WORD dosDate, WORD dosTime, int isDST /* = -1 */)
    {
        FILETIME ft;
        VERIFY( ::DosDateTimeToFileTime(dosDate, dosTime, &ft) );
        CTime t(ft, isDST);
        m_time = t.m_time;
    }

    // Constructs a CTime object from a SYSTEMTIME structure st.
    inline CTime::CTime(const SYSTEMTIME& st, int isDST /* = -1 */)
    {
        CTime t(static_cast<UINT>(st.wYear), static_cast<UINT>(st.wMonth),
            static_cast<UINT>(st.wDay), static_cast<UINT>(st.wHour),
            static_cast<UINT>(st.wMinute), static_cast<UINT>(st.wSecond), isDST); // asserts if invalid

        m_time = t.m_time;
    }

    // Constructs a CTime object from a (UTC) FILETIME structure ft.
    inline CTime::CTime(const FILETIME& ft, int isDST /* = -1 */)
    {
        // start by converting ft (a UTC time) to local time
        FILETIME localTime;
        VERIFY( ::FileTimeToLocalFileTime(&ft, &localTime) );

        //  convert localTime to a SYSTEMTIME structure
        SYSTEMTIME st;
        VERIFY( ::FileTimeToSystemTime(&localTime, &st) );

        // then convert the system time to a CTime
        CTime t(st, isDST);  // asserts if invalid
        m_time = t.m_time;
    }

    // Converts *this CTime object into a FILETIME structure and stores it
    // in ft. Returns true if successful.
    inline bool CTime::GetAsFileTime(FILETIME& ft) const
    {
        bool rval = false;
        time_tm tm;
        time_tm* ptm = GetGmtTm(&tm);
        assert(ptm != NULL);

        if (ptm)
        {
            SYSTEMTIME st = {static_cast<WORD>(1900 + ptm->tm_year), static_cast<WORD>(1 + ptm->tm_mon),
                static_cast<WORD>(ptm->tm_wday), static_cast<WORD>(ptm->tm_mday), static_cast<WORD>(ptm->tm_hour),
                static_cast<WORD>(ptm->tm_min), static_cast<WORD>(ptm->tm_sec), 0};
            SystemTimeToFileTime(&st, &ft);
            rval = true;
        }

        return rval;
    }

    // Converts *this CTime object into a SYSTEMTIME structure and stores it
    // in st. Returns true if successful.
    inline bool CTime::GetAsSystemTime(SYSTEMTIME& st) const
    {
        bool rval = false;
        time_tm tm;
        time_tm* ptm = GetLocalTm(&tm);
        assert(ptm != NULL);

        if (ptm)
        {
            st.wYear    = static_cast<WORD>(1900 + ptm->tm_year);
            st.wMonth   = static_cast<WORD>(1 + ptm->tm_mon);
            st.wDayOfWeek   = static_cast<WORD>(ptm->tm_wday);
            st.wDay     = static_cast<WORD>(ptm->tm_mday);
            st.wHour    = static_cast<WORD>(ptm->tm_hour);
            st.wMinute  = static_cast<WORD>(ptm->tm_min);
            st.wSecond  = static_cast<WORD>(ptm->tm_sec);
            st.wMilliseconds = 0;
            rval = true;
        }

        return rval;
    }

    // Returns a pointer to a time_tm that contains a decomposition of *this
    // CTime object expressed in UTC. If ptm is non NULL, this decomposition
    // is also copied into ptm.
    inline time_tm* CTime::GetGmtTm(time_tm* ptm) const
    {
        // Null argument not supported
        assert (ptm != NULL);
        if (ptm)
        {

#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )
        time_tm* ptmTemp = ::gmtime(&m_time);
        if (ptmTemp == NULL)
            return NULL;    // the m_time was not initialized!

        *ptm = *ptmTemp;
#else
        time_tm tmTemp;
        errno_t result = ::gmtime_s(&tmTemp, &m_time);
        if (result != 0)
            return NULL;

        *ptm = tmTemp;
#endif

        }
        return ptm;
    }

    // Return a pointer to a time_tm that contains a decomposition of *this
    // CTime object expressed in the local time base. If ptm is non NULL, this
    // decomposition is also copied into ptm.
    inline time_tm* CTime::GetLocalTm(time_tm* ptm) const
    {
        // Null argument not supported.
        assert(ptm != NULL);
        if (ptm)
        {

#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )  // not VS or VS < 2005
            time_tm* ptmTemp = ::localtime(&m_time);
            if (ptmTemp == NULL)
                return NULL;    // the m_time was not initialized!

            *ptm = *ptmTemp;
#else
            time_tm tmTemp;
            errno_t result = ::localtime_s(&tmTemp, &m_time);
            if (result != 0)
                return NULL;    // the m_time was not initialized!

            *ptm = tmTemp;
#endif
        }

        return ptm;
    }

    // Return *this time as a time_t value.
    inline time_t  CTime::GetTime() const
    {
        return m_time;
    }

    // Returns the year of *this time object, local (true) or UTC (false).
    inline int  CTime::GetYear(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return 1900 + ptmbuffer->tm_year;
    }

    // Returns the month of *this time object (1 through 12), local (true) or
    // UTC (false).
    inline int  CTime::GetMonth(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_mon + 1;
    }

    // Returns the day of *this object (1 through 31), local (true) or
    // UTC (false).
    inline int  CTime::GetDay(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_mday ;
    }

    // Returns the hour of *this object (0 through 23), local (true) or
    // UTC (false).
    inline int  CTime::GetHour(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_hour;
    }

    // Returns the minute of *this object (0 through 59), local (true) or
    // UTC (false).
    inline int  CTime::GetMinute(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_min;
    }

    // Returns the second of *this object (0 through 61), local (true) or
    // UTC (false).
    inline int  CTime::GetSecond(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_sec;
    }

    // Returns the day of the week of *this object (0-6, Sunday = 0), local
    // (true) or UTC (false).
    inline int  CTime::GetDayOfWeek(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_wday;
    }

    // Returns the day of the year of *this object (1-366), local (true)  or
    // UTC (false).
    inline int  CTime::GetDayOfYear(bool local /* = true */) const
    {
        time_tm tm;
        time_tm* ptmbuffer = (local ? GetLocalTm(&tm) : GetGmtTm(&tm));
        return ptmbuffer->tm_yday + 1;
    }

    // Assigns the CTime t value to *this
    inline CTime& CTime::operator=(const CTime& t)
    {
        // Self assignment is safe
        m_time = t.m_time;
        return *this;
    }

    // Assigns the time_t value to *this time.
    inline CTime& CTime::operator=(const time_t& t)
    {
        assert(t >= 0);
        // Self assignment is safe
        m_time = t;
        return *this;
    }

    // Returns the time span between *this time and  time t
    inline const CTimeSpan CTime::operator-(const CTime& t) const
    {
        timespan_t d = static_cast<timespan_t>(m_time - t.m_time);
        CTimeSpan t0(d);
        return  t0;
    }

    // Returns the CTime that is the time span ts before *this time.
    inline const CTime CTime::operator-(const CTimeSpan& ts) const
    {
        time_t d = m_time - ts.m_timespan;
        CTime t(d);  // asserts if d is invalid
        return t;
    }

    // Returns the CTime that is the time span ts after *this time.
    inline const CTime CTime::operator+(const CTimeSpan& ts) const
    {
        time_t s = m_time + ts.m_timespan;
        CTime t(s); // asserts if s is invalid
        return t;
    }

    // Increments *this time by the time span ts and  return this CTime.
    inline CTime& CTime::operator+=(const CTimeSpan& ts)
    {
        m_time += ts.m_timespan;
        assert(m_time >= 0); // Invalid addition to time object.
        return *this;
    }

    // Decrements *this time by the time span ts and  return this CTime.
    inline CTime& CTime::operator-=(const CTimeSpan& ts)
    {
        m_time -= ts.m_timespan;
        assert(m_time >= 0); // Invalid subtraction from time object.
        return *this;
    }

    // Returns true if *this and t are the same times.
    inline bool CTime::operator==(const CTime& t) const
    {
        return m_time == t.m_time;
    }

    // Returns true if *this and t are not the same times.
    inline bool CTime::operator!=(const CTime& t) const
    {
        return m_time != t.m_time;
    }

    // Returns true if *this time is less than time t.
    inline bool CTime::operator<(const CTime& t) const
    {
        return m_time < t.m_time;
    }

    // Returns true if *this time is greater than time t.
    inline bool CTime::operator>(const CTime& t) const
    {
        return m_time > t.m_time;
    }

    // Returns true if *this time is less than or equal to time t.
    inline bool CTime::operator<=(const CTime& time) const
    {
        return m_time <= time.m_time;
    }

    // Returns true if *this time is greater than or equal to time t.
    inline bool CTime::operator>=(const CTime& time) const
    {
        return m_time >= time.m_time;
    }

    // Returns a CString that contains formatted time. The format parameter
    // is a formatting string similar to the printf formatting string.
    // The valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTime::Format(LPCTSTR format) const
    {
        const size_t  maxTimeBufferSize = 128;
        TCHAR szBuffer[maxTimeBufferSize];

#if !defined (_MSC_VER) ||  ( _MSC_VER < 1400 )  // not VS or VS < 2005
        time_tm* ptm = ::localtime(&m_time);
#else
        time_tm tm;
        time_tm* ptm = &tm;
        ::localtime_s(ptm, &m_time);
#endif

        if (ptm == NULL || !::_tcsftime(szBuffer, maxTimeBufferSize, format, ptm))
            szBuffer[0] = '\0';
        return CString(szBuffer);
    }

    // Returns a CString that contains formatted time. The FormatID parameter
    // specifies a resource containing the formatting string which is similar
    // to the printf formatting string.  The valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTime::Format(UINT formatID) const
    {
        CString strFormat;
        strFormat.LoadString(formatID);
        return Format(strFormat);
    }

    // Returns a CString that contains formatted time as a UTC time. The format
    // parameter is a formatting string similar to the printf formatting string.
    inline CString CTime::FormatGmt(LPCTSTR format) const
    {
        const size_t  maxTimeBufferSize = 128;
        TCHAR szBuffer[maxTimeBufferSize];
        CString fmt0 = format;
        while (fmt0.Replace(_T("%Z"), _T("Coordinated Universal Time")))
            ;
        while (fmt0.Replace(_T("%z"), _T("UTC")))
            ;

        time_tm tmTemp;
        time_tm* ptmTemp = GetGmtTm(&tmTemp);
        if (ptmTemp == NULL || !::_tcsftime(szBuffer, maxTimeBufferSize, fmt0.c_str(), ptmTemp))
            szBuffer[0] = '\0';
        return CString(szBuffer);
    }


    // Returns a CString that contains formatted time as a UTC time. The FormatID
    // parameter specifies a resource containing the formatting string which is
    // similar to the printf formatting string.
    inline CString CTime::FormatGmt(UINT formatID) const
    {
        CString strFormat;
        strFormat.LoadString(formatID);
        return FormatGmt(strFormat);
    }

    //
    //  Static and  Friend Functions
    //


    // Returns a CTime holding the current system time.
    inline CTime CTime::GetCurrentTime()
    {
        return CTime(::time(NULL));
    }


    //
    // Global functions within the Win32xx namespace
    //

    // Reads a CTime from the archive and stores it in t.
    // Throws an exception on failure.
    inline CArchive& operator>>(CArchive& ar, CTime& t)
    {
        UINT size;
        ar.Read(&size, sizeof(size));
        if (size != sizeof(ULONGLONG))
        {
            CString str = ar.GetFile().GetFilePath();
            throw CFileException(str, GetApp()->MsgArReadFail());
        }

        // load CTime as x64
        ULONGLONG tx64 = 0;
        ar.Read(&tx64, size);
        time_t tt = static_cast<time_t>(tx64);
        t = CTime(tt);
        return ar;
    }

    // Writes the time t into the archive file.
    // Throw an exception on failure.
    inline CArchive& operator<<(CArchive& ar, CTime& t)
    {
        ULONGLONG tx64 = 0;
        UINT size = sizeof(tx64);

        // store CTime as x64
        time_t tt = t;
        tx64 = static_cast<ULONGLONG>(tt);
        ar.Write(&tx64, size);
        return ar;
    }


    ///////////////////////////////////////////////////////////////
    //
    //  CTimeSpan class implementation
    //
    ///////////////////////////////////////////////////////////////


    inline CTimeSpan::CTimeSpan()
    {
        m_timespan = 0;
    }

    inline CTimeSpan::CTimeSpan(timespan_t t)
    {
        m_timespan = t;
    }

    // Contructs a CTimeSpan. Valid parameter ranges:
    //   lDays  0-24,855 (approximately)
    //   nHours 0-23
    //   nMins  0-59
    //   nSecs  0-59
    inline CTimeSpan::CTimeSpan(long days, int hours, int mins, int secs)
    {
        time_t sec_per_day  = 86400;
        time_t sec_per_hour = 3600;
        time_t sec_per_min  = 60;
        m_timespan = days * sec_per_day + hours * sec_per_hour +
            mins * sec_per_min + secs;
    }

    inline CTimeSpan::CTimeSpan(const CTimeSpan& ts)
    {
        m_timespan = ts.m_timespan;
    }

    // Returns the number of complete days in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetDays() const
    {
        int sec_per_day = 86400;
        return m_timespan / sec_per_day;
    }

    // Returns the total number of complete hours in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetTotalHours() const
    {
        int sec_per_hour = 3600;
        return m_timespan / sec_per_hour;
    }

    // Returns the total number of complete minutes in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetTotalMinutes() const
    {
        int sec_per_min = 60;
        return m_timespan / sec_per_min;
    }

    // Returns the total number of complete seconds in this CTimeSpan.
    inline LONGLONG CTimeSpan::GetTotalSeconds() const
    {
        return m_timespan;
    }

    // Return the number of hours in the day component of this time
    // span (-23 through 23).
    inline int CTimeSpan::GetHours() const
    {
        int sec_per_hour  = 3600;
        int hours_per_day = 24;
        return static_cast<int>((m_timespan / sec_per_hour) % hours_per_day);
    }

    // Returns the number of minutes in the hour component of this time
    // span (-59 through 59).
    inline int CTimeSpan::GetMinutes() const
    {
        int sec_per_min = 60;
        return static_cast<int>((m_timespan / sec_per_min) % sec_per_min);
    }

    // Returns the number of seconds in the minute component of this time
    // span (-59 through 59).
    inline int CTimeSpan::GetSeconds() const
    {
        int sec_per_min = 60;
        return static_cast<int>(m_timespan % sec_per_min);
    }

    // Assigns the CTimeSpan ts to *this time span.
    inline CTimeSpan& CTimeSpan::operator=(const CTimeSpan& ts)
    {
        // Self assignment is safe
        m_timespan = ts.m_timespan;
        return *this;
    }

    // Assigns the timespan_t t value to *this time span.
    inline CTimeSpan& CTimeSpan::operator=(const timespan_t& t)
    {
        // Self assignment is safe
        m_timespan = t;
        return *this;
    }

    // Returns the negated value of *this time span.
    inline const CTimeSpan CTimeSpan::operator-() const
    {
        CTimeSpan t0(-m_timespan);
        return  t0;
    }

    // Returns the result of subtracting the time span ts from *this time span.
    inline const CTimeSpan CTimeSpan::operator-(CTimeSpan& ts) const
    {
        timespan_t d = m_timespan - ts.m_timespan;
        CTimeSpan tsp(d);
        return tsp;
    }

    // Returns the result of adding the time span ts to *this time span.
    inline const CTimeSpan CTimeSpan::operator+(CTimeSpan& ts) const
    {
        timespan_t s = m_timespan + ts.m_timespan;
        CTimeSpan tsp(s);
        return tsp;
    }

    // Increases *this time span by the ts time span and  return the result.
    inline CTimeSpan& CTimeSpan::operator+=(CTimeSpan& ts)
    {
        m_timespan += ts.m_timespan;
        return *this;
    }

    // Decreases *this time span by the ts time span and  return the result.
    inline CTimeSpan& CTimeSpan::operator-=(CTimeSpan& ts)
    {
        m_timespan -= ts.m_timespan;
        return *this;
    }

    // Returns true if the time span ts equals *this one.
    inline bool CTimeSpan::operator==(const CTimeSpan& ts) const
    {
        return m_timespan == ts.m_timespan;
    }

    // Returns true if the time span ts does not equal *this one.=
    inline bool CTimeSpan::operator!=(const CTimeSpan& ts) const
    {
        return m_timespan != ts.m_timespan;
    }

    // Returns true if *this time span is less than the ts time span.
    inline bool CTimeSpan::operator<(const CTimeSpan& ts) const
    {
        return m_timespan < ts.m_timespan;
    }

    // Returns true if *this time span is greater than the ts time span.
    inline bool CTimeSpan::operator>(const CTimeSpan& ts) const
    {
        return m_timespan > ts.m_timespan;
    }

    // Returns true if *this time span is less than or equal to the ts
    // time span.
    inline bool CTimeSpan::operator<=(const CTimeSpan& ts) const
    {
        return m_timespan <= ts.m_timespan;
    }

    // Returns true if *this time span is greater than or equal to the ts
    // time span.
    inline bool CTimeSpan::operator>=(const CTimeSpan& ts) const
    {
        return m_timespan >= ts.m_timespan;
    }

    // Returns a rendering of *this CTimeSpan object in CString form using the
    // format as the template. The valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTimeSpan::Format(LPCTSTR format) const
    {
        CString fmt0 = format;
        CString insert;

        while (fmt0.Find(_T("%D")) != -1)  // number of days
        {
            insert.Format(_T("%ld"), GetDays());
            fmt0.Replace(_T("%D"), insert);
        }
        while (fmt0.Find(_T("%H")) != -1)  // hours (00 - 23)
        {
            insert.Format(_T("%02d"), GetHours());
            fmt0.Replace(_T("%H"), insert);
        }
        while (fmt0.Find(_T("%M")) != -1)  // minutes (00 - 59)
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

    // Returns a rendering of *this CTimeSpan object in CString form using the
    // string resource having the formatID identifier as the template. The
    // valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTimeSpan::Format(UINT formatID) const
    {
        CString strFormat;
        VERIFY( strFormat.LoadString(formatID) );
        return Format(strFormat);
    }


    //
    // Global functions within the Win32xx namespace
    //


    // Reads a CTimeSpan object from the archive and  store it in t. Throw an
    // exception if unable to do so correctly.
    inline CArchive& operator>>(CArchive& ar, CTimeSpan& ts)
    {
        UINT size;
        ar.Read(&size, sizeof(size));
        if (size != sizeof(ULONGLONG))
        {
            CString str = ar.GetFile().GetFilePath();
            throw CFileException(str, GetApp()->MsgArReadFail());
        }

        // load CTimeSpan as x64
        ULONGLONG tsx64 = 0;
        ar.Read(&tsx64, size);
        timespan_t tst = static_cast<timespan_t>(tsx64);
        ts = CTimeSpan(tst);
        return ar;
    }

    // Writes the time span object s into the archive file. Throw an exception
    // if an error occurs.
    inline CArchive& operator<<(CArchive &ar, CTimeSpan& ts)
    {
        ULONGLONG tsx64 = 0;
        UINT size = sizeof(tsx64);
        ar.Write(&size, sizeof(size));

        // store CTimeSpan as x64
        tsx64 = static_cast<ULONGLONG>(ts);
        ar.Write(&tsx64, size);
        return ar;
    }

} // namespace Win32XX


#endif // _WIN32XX_CTIME_H_

