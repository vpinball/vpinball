// Win32++   Version 10.0.0
// Release Date: 9th September 2024
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2024  David Nash
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
//  Declaration of the CTime class.
//
////////////////////////////////////////////////////////

#ifndef _WIN32XX_TIME_H_
#define _WIN32XX_TIME_H_

#include "wxx_wincore.h"
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
//  upper date limit of January 18, 19:14:07, 2038. This issue can affect
//  code compiled with Visual Studio 2003. Visual Studio 2005 and later
//  defines time_t as a 64 bit integer.

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
    // Forward declaration.
    class CTimeSpan;


    ////////////////////////////////////////////////////////
    // The CTime class represents an absolute time and date.
    class CTime
    {
    public:
        // Constructors.
        CTime();
        CTime(const CTime& t);
        CTime(time_t t);
        CTime(tm& atm);
        CTime(int year, int month, int day, int hour, int min, int sec, int isDST = -1);
        CTime(WORD dosDate, WORD dosTime, int isDST = -1);
        CTime(const SYSTEMTIME& st, int isDST = -1);
        CTime(const FILETIME& ft,  int isDST = -1);

        // CString conversion.
        CString Format(LPCTSTR format) const;
        CString Format(UINT formatID) const;
        CString FormatGmt(LPCTSTR format) const;
        CString FormatGmt(UINT formatID) const;

        // Method members.
        bool    GetAsFileTime(FILETIME& ft) const;
        bool    GetAsSystemTime(SYSTEMTIME& st) const;
        int     GetDay(bool local = true) const;
        int     GetDayOfWeek(bool local = true) const;
        int     GetDayOfYear(bool local = true) const;
        tm*     GetGmtTm(tm* ptm) const;
        int     GetHour(bool local = true) const;
        tm*     GetLocalTm(tm* ptm) const;
        int     GetMinute(bool local = true) const;
        int     GetMonth(bool local = true) const;
        int     GetSecond(bool local = true) const;
        int     GetYear(bool local = true) const;
        time_t  GetTime() const;

        // Assignment operators.
        CTime&  operator=(const CTime& time);
        CTime&  operator=(const time_t& t);

        // Computational operators.
        CTime&  operator+=(const CTimeSpan& ts);
        CTime&  operator-=(const CTimeSpan& ts);
        const CTimeSpan operator-(const CTime& time) const;
        const CTime operator-(const CTimeSpan& ts) const;
        const CTime operator+(const CTimeSpan& ts) const;
        bool        operator==(const CTime& time) const;
        bool        operator!=(const CTime& time) const;
        bool        operator<(const CTime& time) const;
        bool        operator>(const CTime& time) const;
        bool        operator<=(const CTime& time) const;
        bool        operator>=(const CTime& time) const;

        operator time_t() const { return m_time; }

        // Static methods
        static  CTime   GetCurrentTime();

    private:
        // Private data members.
        time_t      m_time;
    };


    ////////////////////////////////////////////////////////////
    // The CTimeSpan class defines the data type for differences
    // between two CTime values, measured in seconds of time.
    class CTimeSpan
    {
        // Global friends.
        friend class CTime;

    public:
        // Constructors.
        CTimeSpan();
        CTimeSpan(time_t ts);
        CTimeSpan(long days, int hours, int mins, int secs);
        CTimeSpan(const CTimeSpan& ts);

        // CString conversion.
        CString     Format(LPCTSTR format) const;
        CString     Format(UINT formatID) const;

        // Methods to extract items.
        LONGLONG    GetDays() const;
        LONGLONG    GetTotalHours() const;
        int         GetHours() const;
        LONGLONG    GetTotalMinutes() const;
        int         GetMinutes() const;
        LONGLONG    GetTotalSeconds() const;
        int         GetSeconds() const;

        // Assignment operators.
        CTimeSpan& operator=(const CTimeSpan& ts);
        CTimeSpan& operator=(const time_t& t);

        // Computational operators.
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

        operator time_t() const { return m_timespan; }

    private:
        // Private data members.
        time_t m_timespan;
    };

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    /////////////////////////////////////////////////
    // Global functions within the Win32xx namespace.
    //

    // Calls either ::gmtime or ::gmtime_s, depending on the compiler.
    // The value of atm is updated and its pointer is returned if successful.
    // Returns nullptr on failure.
    inline tm* GMTime(tm& atm, const time_t& t)
    {
        tm* ptm = &atm;

#if defined(__BORLANDC__)  // For Embacadero support
        if (::gmtime_s(&t, &atm) == 0)
            ptm = nullptr;
#else
        if (::gmtime_s(&atm, &t) != 0)
            ptm = nullptr;
#endif

        // Note: ptm points to atm (not a local variable) or nullptr.
        return ptm;
    }

    // Calls either ::localtime or ::localtime_s depending on the compiler.
    // The value of atm is updated and its pointer is returned if successful.
    // Returns nullptr on failure.
    inline tm* LocalTime(tm& atm, const time_t& t)
    {
        tm* ptm = &atm;

#if defined(__BORLANDC__)  // For Embacadero support
        if (::localtime_s(&t, &atm) == 0)
            ptm = nullptr;
#else
        if (::localtime_s(&atm, &t) != 0)
            ptm = nullptr;
#endif

        // Note: ptm points to atm (not a local variable) or nullptr.
        return ptm;
    }

    // Returns the time_t t corresponding to the date given in atm as a UTC time.
    inline time_t UTCtime(tm* atm)
    {
        // Compute the local time from atm.
        assert(atm != nullptr);
        time_t t0 = ::mktime(atm);
        assert(t0 != -1);
        tm tm0;
        tm* ptm0 = GMTime(tm0, t0);

        time_t t1 = ::mktime(ptm0);
        assert(t1 != -1);
        time_t zt = t0 - t1;
        t0 += zt;

        assert(GMTime(tm0, t0));
        return t0;
    }

    // Writes the time t into the archive file.
    // Throw an exception on failure.
    inline CArchive& operator<<(CArchive& ar, CTime& t)
    {
        ar.Write(&t, sizeof(t));
        return ar;
    }

    // Reads a CTime from the archive and stores it in t.
    // Throws an exception on failure.
    inline CArchive& operator>>(CArchive& ar, CTime& t)
    {
        ar.Read(&t, sizeof(t));
        return ar;
    }

    // Writes the time span object ts into the archive file.
    // Throws an exception if an error occurs.
    inline CArchive& operator<<(CArchive& ar, CTimeSpan& ts)
    {
        ar.Write(&ts, sizeof(ts));
        return ar;
    }

    // Reads a CTimeSpan object from the archive and store it in ts.
    // Throws an exception if an error occurs.
    inline CArchive& operator>>(CArchive& ar, CTimeSpan& ts)
    {
        ar.Read(&ts, sizeof(ts));
        return ar;
    }


    ///////////////////////////////////
    // Definitions for the CTime class.
    //

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
        tm atm;
        VERIFY(GMTime(atm, t));
        m_time = t;
    }

    // Constructs a CTime object from the time_tm atm, or assert if atm is invalid.
    inline CTime::CTime(tm& atm)
    {
        // Compute the object time_t.
        m_time = ::mktime(&atm);

        // Check for acceptable range.
        assert(m_time != -1);
    }

    // Constructs a CTime object from local time elements. Each element is
    // constrained to lie within the following UTC ranges:
    //   year       1970-2038 (on 32-bit systems)
    //   month      1-12
    //   day        1-31
    //   hour, min, sec no constraint
    inline CTime::CTime(int year, int month, int day, int hour, int min,
                        int sec, int isDST /* = -1*/)
    {
        // Validate parameters w.r.t. ranges.
        assert(1 <= day && day   <= 31);
        assert(1 <= month && month <= 12);
        assert(year >= 1969);  // Last few hours of 1969 might be a valid local time.

        // Fill out a time_tm with the calendar date.
        tm atm = {sec, min, hour, day, month - 1, year - 1900, 0, 0, isDST};

        // Compute the object time_t.
        m_time = ::mktime(&atm);
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
        CTime t(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
                st.wSecond, isDST);   // Asserts if invalid.

        m_time = t.m_time;
    }

    // Constructs a CTime object from a (UTC) FILETIME structure ft.
    inline CTime::CTime(const FILETIME& ft, int isDST /* = -1 */)
    {
        // Convert ft (a UTC time) to local time.
        FILETIME localTime;
        VERIFY( ::FileTimeToLocalFileTime(&ft, &localTime) );

        // Convert localTime to a SYSTEMTIME structure.
        SYSTEMTIME st;
        VERIFY( ::FileTimeToSystemTime(&localTime, &st) );

        // Convert the system time to a CTime.
        CTime t(st, isDST);  // Asserts if invalid.
        m_time = t.m_time;
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
        tm atm;
        tm* ptm = GetLocalTm(&atm);
        CString formatString;

        assert(ptm != nullptr);
        if (ptm != nullptr)
        {
            const size_t  bufferSize = 128;
            VERIFY((::_tcsftime(formatString.GetBuffer(bufferSize), bufferSize, format, &atm)) != 0);
            formatString.ReleaseBuffer();
        }

        return formatString;
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
        CString fmt0 = format;
        fmt0.Replace(_T("%Z"), _T("Coordinated Universal Time"));
        fmt0.Replace(_T("%z"), _T("UTC"));

        tm atm;
        tm* ptm = GetGmtTm(&atm);
        CString formatString;

        assert(ptm != nullptr);
        if (ptm != nullptr)
        {
            const size_t  bufferSize = 128;
            VERIFY(::_tcsftime(formatString.GetBuffer(bufferSize), bufferSize, fmt0.c_str(), ptm) != 0);
            formatString.ReleaseBuffer();
        }

        return formatString;
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

    // Converts this CTime object into a FILETIME structure and stores it in
    // the specified variable. Returns true if successful.
    inline bool CTime::GetAsFileTime(FILETIME& ft) const
    {
        bool rval = false;
        tm atm;
        tm* ptm = GetGmtTm(&atm);
        assert(ptm != nullptr);

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

    // Converts this CTime object into a SYSTEMTIME structure and stores it
    // in the specified variable. Returns true if successful.
    inline bool CTime::GetAsSystemTime(SYSTEMTIME& st) const
    {
        bool rval = false;
        tm atm;
        tm* ptm = GetLocalTm(&atm);
        assert(ptm != nullptr);

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

    // Returns a pointer to a tm struct that contains a decomposition of the
    // CTime object expressed in UTC. Returns nullptr on failure. The supplied
    // ptm cannot be nullptr.
    inline tm* CTime::GetGmtTm(tm* ptm) const
    {
        assert (ptm != nullptr);    // nullptr argument not supported.
        if (ptm)
            ptm = GMTime(*ptm, m_time);

        return ptm;
    }

    // Returns a pointer to a tm struct that contains a decomposition of the
    // CTime object expressed in the local time base. Returns nullptr on failure.
    // The supplied ptm cannot be nullptr.
    inline tm* CTime::GetLocalTm(tm* ptm) const
    {
        assert(ptm != nullptr);    // nullptr argument not supported.
        if (ptm)
            ptm = LocalTime(*ptm, m_time);

        return  ptm;
    }

    // Returns the time represented by this CTime object as a time_t value.
    inline time_t  CTime::GetTime() const
    {
        return m_time;
    }

    // Returns the year represented by this CTime object, local (true) or UTC (false).
    inline int  CTime::GetYear(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return 1900 + ptm->tm_year;
    }

    // Returns the month of the year represented by this CTime object in the
    // range 1 through 12, local (true) or UTC (false).
    inline int  CTime::GetMonth(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return ptm->tm_mon + 1;
    }

    // Returns the day of the month represented by this CTime object in the
    // range 1 through 31, local (true) or UTC (false).
    inline int  CTime::GetDay(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return ptm->tm_mday ;
    }

    // Returns the hour of the day represented by this CTime object in the
    // range 0 through 23, local (true) or UTC (false).
    inline int  CTime::GetHour(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return ptm->tm_hour;
    }

    // Returns the minute of the hour represented by this CTime object in the
    // range 0 through 59, local (true) or UTC (false).
    inline int  CTime::GetMinute(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return ptm->tm_min;
    }

    // Returns the second of the minute represented by this CTime object in the
    // range 0 through 59, local (true) or UTC (false).
    inline int  CTime::GetSecond(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return ptm->tm_sec;
    }

    // Returns the day of the week represented by this CTime object in the
    // range 1 through 7, local (true) or UTC (false).
    inline int  CTime::GetDayOfWeek(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return ptm->tm_wday + 1;
    }

    // Returns the day of the year represented by this CTime object in the
    // range 1 to 366, local (true)  or UTC (false).
    inline int  CTime::GetDayOfYear(bool local /* = true */) const
    {
        tm atm;
        tm* ptm = (local ? GetLocalTm(&atm) : GetGmtTm(&atm));
        return ptm->tm_yday + 1;
    }

    // Assigns the specified value to this CTime object.
    inline CTime& CTime::operator=(const CTime& t)
    {
        // Self assignment is safe.
        m_time = t.m_time;
        return *this;
    }

    // Assigns the time_t value to this CTime object.
    inline CTime& CTime::operator=(const time_t& t)
    {
        // Self assignment is safe.
        tm atm;
        m_time = t;
        VERIFY(GMTime(atm, m_time));  // Verify m_time is valid.
        // Self assignment is safe.

        return *this;
    }

    // Returns the time span between the specified time and this CTime object.
    inline const CTimeSpan CTime::operator-(const CTime& time) const
    {
        return CTimeSpan(m_time - time.m_time);
    }

    // Returns the subtraction the specified time span from this CTime object.
    inline const CTime CTime::operator-(const CTimeSpan& ts) const
    {
        return CTime(m_time - ts.m_timespan);
    }

    // Returns the addition of the specified time span to this CTime object.
    inline const CTime CTime::operator+(const CTimeSpan& ts) const
    {
        return CTime(m_time + ts.m_timespan);
    }

    // Increments this CTime object by the specifed time span.
    inline CTime& CTime::operator+=(const CTimeSpan& ts)
    {
        m_time += ts.m_timespan;
        tm atm;
        VERIFY(GMTime(atm, m_time));  // Verify m_time is valid.
        return *this;
    }

    // Decrements this CTime object by the specified time span.
    inline CTime& CTime::operator-=(const CTimeSpan& ts)
    {
        m_time -= ts.m_timespan;
        tm atm;
        VERIFY(GMTime(atm, m_time));  // Verify m_time is valid.
        return *this;
    }

    // Returns true if the times represented by this CTime object and the
    // specified CTime are equal.
    inline bool CTime::operator==(const CTime& t) const
    {
        return m_time == t.m_time;
    }

    // Returns true if the times represented by this CTime object and the
    // specified CTime are not equal.
    inline bool CTime::operator!=(const CTime& t) const
    {
        return m_time != t.m_time;
    }

    // Returns true if the time represented by this CTime object is less
    // than the one represented by the specified CTime.
    inline bool CTime::operator<(const CTime& t) const
    {
        return m_time < t.m_time;
    }

    // Returns true if the time represented by this CTime object is greater
    // than the one represented by the specified CTime.
    inline bool CTime::operator>(const CTime& t) const
    {
        return m_time > t.m_time;
    }

    // Returns true if the time represented by this CTime object is less
    // than or equal tothe one represented by the specified CTime.
    inline bool CTime::operator<=(const CTime& time) const
    {
        return m_time <= time.m_time;
    }

    // Returns true if the time represented by this CTime object is greater
    // than or eqaul to the one represented by the specified CTime.
    inline bool CTime::operator>=(const CTime& time) const
    {
        return m_time >= time.m_time;
    }

    // Static function. Returns a CTime object that represents the current time.
    inline CTime CTime::GetCurrentTime()
    {
        return CTime(::time(nullptr));
    }


    ///////////////////////////////////////
    // Definitions for the CTimeSpan class.
    //

    inline CTimeSpan::CTimeSpan()
    {
        m_timespan = 0;
    }

    inline CTimeSpan::CTimeSpan(time_t ts)
    {
        m_timespan = ts;
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

    // Returns a rendering of this CTimeSpan object in CString form using the
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

    // Returns a rendering of this CTimeSpan object in CString form using the
    // string resource having the formatID identifier as the template. The
    // valid format directives are
    //   %D - number of days
    //   %H - hour (0-23)
    //   %M - minute (0-59)
    //   %S - seconds (0-59)
    inline CString CTimeSpan::Format(UINT formatID) const
    {
        CString strFormat;
        VERIFY(strFormat.LoadString(formatID));
        return Format(strFormat);
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

    // Assigns the specified value to this CTimeSpan object.
    inline CTimeSpan& CTimeSpan::operator=(const CTimeSpan& ts)
    {
        // Self assignment is safe.
        m_timespan = ts.m_timespan;
        return *this;
    }

    // Assigns the specified value to the CTimeSpan object.
    inline CTimeSpan& CTimeSpan::operator=(const time_t& t)
    {
        // Self assignment is safe.
        tm atm;
        m_timespan = t;
        VERIFY(GMTime(atm, m_timespan));  // Verify m_timespan is valid.
        return *this;
    }

    // Returns the negated value of this CTimeSpan object.
    inline const CTimeSpan CTimeSpan::operator-() const
    {
        return CTimeSpan(-m_timespan);
    }

    // Returns the result of subtracting the specified value
    // from this CTimeSpan object.
    inline const CTimeSpan CTimeSpan::operator-(CTimeSpan& ts) const
    {
        return CTimeSpan(m_timespan - ts.m_timespan);
    }

    // Returns the result of adding the specified value to this
    // CTimeSpan object.
    inline const CTimeSpan CTimeSpan::operator+(CTimeSpan& ts) const
    {
        return CTimeSpan(m_timespan + ts.m_timespan);
    }

    // Increases this CTimeObject object by the specified value.
    inline CTimeSpan& CTimeSpan::operator+=(CTimeSpan& ts)
    {
        m_timespan += ts.m_timespan;
        tm atm;
        VERIFY(GMTime(atm, m_timespan));  // Verify m_timespan is valid.
        return *this;
    }

    // Decreases this CTimeSpan object by the specified value.
    inline CTimeSpan& CTimeSpan::operator-=(CTimeSpan& ts)
    {
        m_timespan -= ts.m_timespan;
        tm atm;
        VERIFY(GMTime(atm, m_timespan));  // Verify m_timespan is valid.
        return *this;
    }

    // Returns true specified value equals this CTimeSpan object.
    inline bool CTimeSpan::operator==(const CTimeSpan& ts) const
    {
        return m_timespan == ts.m_timespan;
    }

    // Returns true if the specified value is not equal to this CTimeSpan object.
    inline bool CTimeSpan::operator!=(const CTimeSpan& ts) const
    {
        return m_timespan != ts.m_timespan;
    }

    // Returns true if the specified value is less than this CTimeSpan object.
    inline bool CTimeSpan::operator<(const CTimeSpan& ts) const
    {
        return m_timespan < ts.m_timespan;
    }

    // Returns true if the specified value is greater than this CTimeSpan object.
    inline bool CTimeSpan::operator>(const CTimeSpan& ts) const
    {
        return m_timespan > ts.m_timespan;
    }

    // Returns true if the specified value is less than or eqaul to this
    // CTimeSpan object.
    inline bool CTimeSpan::operator<=(const CTimeSpan& ts) const
    {
        return m_timespan <= ts.m_timespan;
    }

    // Returns true if the specified value is greater than or equal to this
    // CTimeSpan object.
    inline bool CTimeSpan::operator>=(const CTimeSpan& ts) const
    {
        return m_timespan >= ts.m_timespan;
    }


} // namespace Win32XX


#endif // _WIN32XX_TIME_H_
