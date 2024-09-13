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
// Developed from code originally provided by:
//
//      Robert C. Tausworthe
//      email: robert.c.tausworthe@ieee.org
//
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
// wxx_ddx.h
//  Definition of the CDataExchange class

// This class provides support for Dialog Data eXchange(DDX) and Dialog Data
// Validation(DDV). This class has a set of DDX and DDV member functions which
// perform the data exchange and validation for different types of controls.
// Typically this is used for controls in a dialogs, but controls in any
// window support DDX and DDV.

// To use CDataExchange, perform the following steps:
// * Override DoDataExchange and specify the appropriate DDX and DDV functions.
// * Call UpdateData(dataExchange, FALSE) to initialize the controls and assign
//    their values. This is typically done in the dialog's OnInitDialog function.
// * Call UpdateData(dataExchange, TRUE) to validate and retrieve the control data.


#ifndef _WIN32XX_DDX_H_
#define _WIN32XX_DDX_H_


#include <float.h>
#include <iomanip>
#include "wxx_wincore.h"


namespace Win32xx
{

    ///////////////////////////////////////////////////////////////////////////
    //
    //  Definitions
    //
    //////////////////////////////////////////////////////////////////////////

    // DDX read/write direction (easier to remember)
    const BOOL SENDTOCONTROL   = FALSE; // set variable values into controls
    const BOOL READFROMCONTROL = TRUE;  // get values from controls into variables


    ///////////////////////////////////////////////////////////////////////////
    //
    //  Declaration of the CDataExchange class.
    //
    ///////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
    // CDataExchange is used to manage the data exchange between
    // the user and a dialog control. It also validates the data
    // passed between the user and the control.
    class CDataExchange
    {

    public:
        CDataExchange();
        virtual ~CDataExchange();

        // Dialog Data Validation (DDV) functions
        virtual void DDV_MaxChars(const CString& value, int count) const;
        virtual void DDV_MinMaxByte(BYTE value, BYTE min, BYTE max) const;
        virtual void DDV_MinMaxDateTime(const SYSTEMTIME&, const SYSTEMTIME&, const SYSTEMTIME&) const;
        virtual void DDV_MinMaxDouble(const double& value, double min, double max, int precision = DBL_DIG) const;
        virtual void DDV_MinMaxFloat(const float& value, float min, float max, int precision = FLT_DIG) const;
        virtual void DDV_MinMaxInt(int value, int min, int max) const;
        virtual void DDV_MinMaxLong(long value, long min, long max) const;
        virtual void DDV_MinMaxMonth(const SYSTEMTIME&, const SYSTEMTIME&, const SYSTEMTIME&) const;
        virtual void DDV_MinMaxShort(short value, short min, short max) const;
        virtual void DDV_MinMaxSlider(ULONG value, ULONG min, ULONG max) const;
        virtual void DDV_MinMaxUInt(UINT value, UINT min, UINT max) const;
        virtual void DDV_MinMaxULong(ULONG value, ULONG min, ULONG max) const;

        // DDX Initialization
        virtual void DDX_Control(UINT id, CWnd& control);

        // Dialog Data eXchange (DDX) functions
        virtual void DDX_CBIndex(UINT id, int& index);
        virtual void DDX_CBString(UINT id, CString& value);
        virtual void DDX_CBStringExact(UINT id, CString& value);
        virtual void DDX_Check(UINT id, int& value);
        virtual void DDX_DateTime(UINT id, SYSTEMTIME& value);
        virtual void DDX_LBIndex(UINT id, int& index);
        virtual void DDX_LBString(UINT id, CString& value);
        virtual void DDX_LBStringExact(UINT id, CString& value);
        virtual void DDX_MonthCal(UINT id, SYSTEMTIME& value);
        virtual void DDX_Progress(UINT id, int& value);
        virtual void DDX_Radio(UINT id, int& value);
        virtual void DDX_Scroll(UINT id, int& value);
        virtual void DDX_Slider(UINT id, int& value);

        // DDX text operations
        virtual void DDX_Text(UINT id, BYTE& value);
        virtual void DDX_Text(UINT id, short& value);
        virtual void DDX_Text(UINT id, int& value);
        virtual void DDX_Text(UINT id, UINT& value);
        virtual void DDX_Text(UINT id, long& value);
        virtual void DDX_Text(UINT id, ULONG& value);
        virtual void DDX_Text(UINT id, CString& value);
        virtual void DDX_Text(UINT id, LPTSTR value, int maxLen);
        virtual void DDX_Text(UINT id, float& value, int precision = FLT_DIG);
        virtual void DDX_Text(UINT id, double& value, int precision = DBL_DIG);

        // Helper operations
        void virtual Fail(LPCTSTR message) const;
        HWND GetLastControl()  const { return m_lastControl; }
        HWND GetLastEditControl() const { return m_lastEditControl; }
        void Init(CWnd& dlgWnd, BOOL retrieveAndValidate);
        HWND PrepareCtrl(UINT id);      // return HWND of control
        HWND PrepareEditCtrl(UINT id);  // record this is an edit

    private:
        // data members
        int   m_id;                     // ID of last-accessed control
        HWND  m_lastControl;            // handle of last-accessed control
        HWND  m_lastEditControl;        // handle of last edit control
        BOOL  m_isEditLastControl;      // most recent control is an edit box
        BOOL  m_retrieveAndValidate;    // TRUE means retrieve and validate data
        HWND  m_parent;                 // parent window
    };


    // Global function
    ULONGLONG SystemTimeToULL(const SYSTEMTIME &systime);


}   // namespace Win32xx


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{
    // Provide operator overload for BYTE.
    inline tStringStream& operator>>(tStringStream& ts, BYTE& value)
    {
        UINT u = 0;
        ts >> u;
        if (u < 256)
        {
            value = static_cast<BYTE>(u);
        }
        else
        {
            ts.setstate(std::ios_base::failbit);
        }
        return ts;
    }

    ////////////////////////////////////////////////////////////////
    //
    //  CDataExchange Class Methods
    //
    ////////////////////////////////////////////////////////////////


    // Constructs a DDX-DDV object.
    inline CDataExchange::CDataExchange()
    {
        m_id = 0;
        m_lastControl = nullptr;
        m_lastEditControl = nullptr;
        m_isEditLastControl = FALSE;
        m_retrieveAndValidate = FALSE;
        m_parent = nullptr;
    }


    // Destructor.
    inline CDataExchange::~CDataExchange()
    {
    }

    ////////////////////////////////////////////////////////////////
    //
    //  DDV: Dialog Data Validation Functions
    //
    ////////////////////////////////////////////////////////////////


    // Ensures that the length of value <= count when validating, otherwise
    // throws a CUserException. If writing, send the box the message to set
    // the limit to count, which must be at least one.
    inline void CDataExchange::DDV_MaxChars(const CString& value, int count) const
    {
        assert(count >= 1);  // must allow at least one char
        if (m_retrieveAndValidate && value.GetLength() > count)
        {
            CString message;
            message.Format(GetApp()->MsgDDV_StringSize(), value.c_str(), count);

            throw CUserException(message);
        }
        else if (m_lastControl != nullptr && m_isEditLastControl)
        {
            // limit the control max-chars automatically
            WPARAM wparam = static_cast<WPARAM>(count);
            ::SendMessage(m_lastControl, EM_LIMITTEXT, wparam, 0);
        }
    }

    // Ensures that minVal <= value <= maxVal when validating, otherwise throws
    // a CUserException.  BYTE is unsigned char.
    inline void CDataExchange::DDV_MinMaxByte(BYTE value, BYTE min, BYTE max) const
    {
          DDV_MinMaxULong(value, min, max);
    }

    // When validating, this method sets the range of the DateTime control
    // associated with the last visited window control to the pair
    // (minRange, maxRange). When assigning, this method verifies
    // that the refValue first falls between minRange and maxRange values
    // before setting the range; if refValue is outside these limits,
    // no setting of the range takes place and a trace message is written
    // in debug mode.
    inline void CDataExchange::DDV_MinMaxDateTime(const SYSTEMTIME& value,
        const  SYSTEMTIME& min, const  SYSTEMTIME& max) const
    {
        ULONGLONG zero = 0;
        ULONGLONG val = SystemTimeToULL(value);
        ULONGLONG nMin = SystemTimeToULL(min);
        ULONGLONG nMax = SystemTimeToULL(max);
        assert(nMin == zero || nMax == zero || nMin <= nMax);

        if (m_retrieveAndValidate)
        {
            if ((nMin != zero && nMin > val) ||
                (nMax != zero && nMax < val))
            {
                // Retrieve the control ID.
                int id = static_cast<int>(::GetWindowLongPtr(m_lastControl, GWLP_ID));
                CString str;
                str << _T("*** WARNING: Date-Time data is out of range ")
                        << _T("in control ID ") << id << _T(". ***\n");
                TRACE(str);

                return;
            }
        }

        // Set the given DateTime range.
        SYSTEMTIME sta[2]{};
        sta[0] = min;
        sta[1] = max;

        LPARAM lparam = reinterpret_cast<LPARAM>(sta);
        ::SendMessage(m_lastControl, DTM_SETRANGE, GDTR_MIN | GDTR_MAX, lparam);
    }


    // Ensures that minVal <= value <= maxVal when validating, otherwise
    // throws a CUserException.
    inline void CDataExchange::DDV_MinMaxDouble(const double& value, double min,
        double max, int precision /* = DBL_DIG */) const
    {
        assert(min <= max);
        if (min <= value && value <= max)
            return;

        if (!m_retrieveAndValidate)
        {
            // Just leave a debugging trace if writing to a control.
            TRACE(_T("*** WARNING: control data is out of range. ***\n"));
            return; // don't throw
        }

        // Throw includes an error message with the range tuple when
        // reading a number outside the range.
        CString message;
        message.Format(GetApp()->MsgDDV_RealRange(), precision, min, precision, max);

        throw CUserException(message);
    }

    // Ensures that minVal <= value <= maxVal when validating, otherwise
    // throws a CUserException.
    inline void CDataExchange::DDV_MinMaxFloat(const float& value, float min,
        float max, int precision /* = FLT_DIG */) const
    {
        DDV_MinMaxDouble(static_cast<double>(value), static_cast<double>(min),
            static_cast<double>(max), precision);
    }

    // Ensures that minVal <= value <= maxVal when validating, otherwise
    // throws a CUserException.
    inline void CDataExchange::DDV_MinMaxInt(int value, int min, int max) const
    {
        DDV_MinMaxLong(value, min, max);
    }

    // Ensures that minVal <= value <= maxVal when validating, otherwise
    // throws a CUserException.
    inline void CDataExchange::DDV_MinMaxLong(long value, long min, long max) const
    {
        assert(min <= max);
        if (min <= value && value <= max)
            return;

        // Just leave a debugging trace if writing to a control.
        if (!m_retrieveAndValidate)
        {
            TRACE(_T("*** WARNING: current control data is out of range. ***\n"));
            return;     // don't stop
        }

        // Throw includes an error message with the range tuple when
        // reading a number outside the range.
        CString message;
        message.Format(GetApp()->MsgDDV_IntRange(), min, max);

        throw CUserException(message);
    }

    // When validating, this method sets the range of the month
    // calendar control associated with the last visited window control to
    // (minRange, maxRange). When assigning, this method verifies
    // that the refValue first falls between minRange and maxRange values
    // before setting the range; if refValue is outside these limits,
    // no setting of the range takes place and a trace message is written
    // in debug mode.
    inline void CDataExchange::DDV_MinMaxMonth(const SYSTEMTIME& value, const SYSTEMTIME& min,
        const SYSTEMTIME& max) const
    {
        ULONGLONG zero = 0;
        ULONGLONG val = SystemTimeToULL(value);
        ULONGLONG nMin = SystemTimeToULL(min);
        ULONGLONG nMax = SystemTimeToULL(max);
        assert(nMin == zero || nMax == zero || nMin <= nMax);

        if (!m_retrieveAndValidate)
        {
            if ((nMin != zero && nMin > val) ||
                (nMax != zero && nMax < val))
            {
                int id = static_cast<int>(::GetWindowLongPtr(m_lastControl, GWLP_ID));
                CString str;
                str << _T("*** WARNING: Calendar data is out of range ")
                        << _T("in control ID ") << id << _T(". ***\n");
                TRACE(str);
                return;     // continue on
            }
        }

        SYSTEMTIME minMax[2]{};
        DWORD limit = GDTR_MIN | GDTR_MAX;
        minMax[0] = min;
        minMax[1] = max;
        WPARAM wparam = static_cast<WPARAM>(limit);
        LPARAM lparam = reinterpret_cast<LPARAM>(&minMax);

        ::SendMessage(m_lastControl, MCM_SETRANGE, wparam, lparam);
    }

    // Ensures that minVal <= value <= maxVal when validating, otherwise
    // throws a CUserException.
    inline void CDataExchange::DDV_MinMaxShort(short value, short min, short max) const
    {
        DDV_MinMaxLong(value, min, max);
    }

    // When validating, this method sets the range of the slider
    // control associated with the last visited window control to the pair
    // (minRange, maxRange). When assigning, this method verifies
    // that the refValue first falls between minRange and maxRange values
    // before setting the range; if refValue is outside these limits,
    // no setting of the range takes place and a trace message is written
    // in debug mode.
    inline void CDataExchange::DDV_MinMaxSlider(ULONG value, ULONG min, ULONG max) const
    {
        assert(min <= max);
        if (m_retrieveAndValidate)
        {
            if (min > value || max < value)
            {
                TRACE("*** WARNING: slider position is outside given limits. ***\n");
                return;
            }
        }

        // Set the range tuple.
        ::SendMessage(m_lastControl,TBM_SETRANGEMIN, static_cast<WPARAM>(FALSE),
                      static_cast<LPARAM>(min));
        ::SendMessage(m_lastControl, TBM_SETRANGEMAX, static_cast<WPARAM>(TRUE),
                      static_cast<LPARAM>(max));
    }

    // Ensures that minVal <= value <= maxVal when validating, otherwise
    // throws a CUserException.
    inline void CDataExchange::DDV_MinMaxUInt( UINT value, UINT min, UINT max) const
    {
        DDV_MinMaxULong(value, min, max);
    }

    // Ensures that minVal <= value <= maxVal when validating, otherwise
    // throws a CUserException.
    inline void CDataExchange::DDV_MinMaxULong(ULONG value, ULONG min, ULONG max) const
    {
        assert(min <= max);
        if (min <= value && value <= max)
            return;

        if (!m_retrieveAndValidate)
        {
            // just leave a debugging trace if writing to a control
            int id = static_cast<int>(::GetWindowLongPtr(m_lastControl, GWLP_ID));
            CString str;
            str << _T("*** WARNING: value is outside limits in control with ID ")
                    << id << _T(". ***\n");
            TRACE(str);
            return;     // don't stop
        }

        // Throw includes an error message with the range tuple when
        // reading a number outside the range.
        CString message;
        message.Format(GetApp()->MsgDDV_UINTRange(), min, max);

        throw CUserException(message);
    }


    ////////////////////////////////////////////////////////////////
    //
    //  DDX: Control Initialization
    //
    ////////////////////////////////////////////////////////////////


    // This function attaches the window with a control ID of id
    // to the specified CWnd. Controls are only attached once.
    inline void CDataExchange::DDX_Control(UINT id, CWnd& control)
    {
        if (!control.IsWindow())    // Not subclassed yet.
        {
            assert (!m_retrieveAndValidate);
            HWND handle = PrepareCtrl(id);
            assert(handle);
            control.Attach(handle);
        }
    }

    ////////////////////////////////////////////////////////////////
    //
    //  Data exchange for special controls
    //
    ////////////////////////////////////////////////////////////////


    // This function manages the transfer of data between a combo box control
    // in a dialog box or other window, and an integer
    // index of that object. It performs a data exchange for the state of
    // the combo box control appearing within the DDX/DDV object m_DX with
    // the control numbered id. When this function is called, index is set
    // to the index of the current combo box selection. If no item is
    // selected, index is set to 0. When m_DX.m_retrieveAndValidate is false,
    // the index item is selected.
    inline void CDataExchange::DDX_CBIndex(UINT id, int& index)
    {
        HWND control = PrepareCtrl(id);

        if (m_retrieveAndValidate)
            index = static_cast<int>(::SendMessage(control, CB_GETCURSEL, 0, 0));
        else
            ::SendMessage(control, CB_SETCURSEL, static_cast<WPARAM>(index), 0);
    }

    // This function manages the transfer of CString data between the edit
    // control of a ComboBox control in a dialog box or other window,
    // and a CString data member of that object. It performs a
    // data exchange for the state of the combo box control appearing within
    // the DDX/DDV object m_DX with the control numbered id. On reading the
    // list box, the combo box edit window value is returned as the value.
    // When m_DX.m_retrieveAndValidate is false, the list of the combo box
    // is searched for an item that begins with the characters in the value
    // string. If a matching item is found, it is selected and copied to the
    // edit control. If the search is unsuccessful, the current edit control
    // is not changed.
    inline void CDataExchange::DDX_CBString(UINT id, CString& value)
    {
        HWND control = PrepareCtrl(id);
        if (m_retrieveAndValidate)
        {
            // Get the current edit item text or drop list static where possible.
            int length = ::GetWindowTextLength(control);
            if (length > 0)
            {
                // Get the known length.
                ::GetWindowText(control, value.GetBuffer(length), length + 1);
            }
            else
            {
                // GetWindowTextLength does not work for drop lists
                // so here assume max of 255 characters.
                const int maxLen = 255;
                ::GetWindowText(control, value.GetBuffer(maxLen), maxLen + 1);
            }
            value.ReleaseBuffer();
        }
        else
        {
            // Set the current selection based on value string.
            if (::SendMessage(control, CB_SELECTSTRING, static_cast<WPARAM>(-1),
                reinterpret_cast<LPARAM>(value.c_str()) == CB_ERR))
            {
                // Value was not found, so just set the edit text.
                // (This will be ignored if the control is a DROPDOWNLIST.)
                ::SetWindowText(control, value);
            }
        }
    }

    // This function manages the transfer of CString data between the edit
    // control of a combo box control in a dialog box or other window,
    // and a CString data member of that object. It performs a
    // data exchange for the state of the combo box control appearing within
    // the DDX/DDV object m_DX with the control numbered id. On reading the
    // list box, the combo box edit window value is returned as the value.
    // When m_DX.m_retrieveAndValidate is false, a case-insensitive search is
    // made in the prevailing sort sense for the first list box string in
    // the combo box that matches the string specified in the value parameter.
    // If a matching item is found, it is selected and copied to the edit
    // window. If the search is unsuccessful, the current edit control
    // is not changed.
    inline void CDataExchange::DDX_CBStringExact(UINT id, CString& value)
    {
        HWND control = PrepareCtrl(id);
        if (m_retrieveAndValidate)
        {
            DDX_CBString(id, value);
        }
        else if (value != _T(""))   // write to control
        {
            // Set current selection based on data string.
            int i = static_cast<int>(::SendMessage(control, CB_FINDSTRINGEXACT,
                static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(value.c_str())));
            if (i < 0)
            {
                // Set the edit text (will be ignored if a DROPDOWNLIST).
                ::SetWindowText(control, value);
            }
            else
            {
                // Select it.
                ::SendMessage(control, CB_SETCURSEL, static_cast<WPARAM>(i), 0);
            }
        }
    }

    // Perform a check box control data exchange on the DDX/DDV object m_DX
    // with the control numbered id to and from the given int value.
    inline void CDataExchange::DDX_Check(UINT id, int& value)
    {
        HWND control = PrepareCtrl(id);
        if (m_retrieveAndValidate)
        {
            value = static_cast<int>(::SendMessage(control, BM_GETCHECK, 0, 0));
            assert(value == BST_CHECKED || value == BST_UNCHECKED ||
                value == BST_INDETERMINATE);
        }
        else
        {
            if (value < 0 || value > 2)
            {
                CString str;
                str << _T("*** WARNING: dialog data checkbox value ")
                        << value << _T(" out of range. ***\n");
                TRACE(str);
                value = 0;  // set default to off
            }

            ::SendMessage(control, BM_SETCHECK, static_cast<WPARAM>(value), 0);
        }
    }

    // This function manages the transfer of date and/or time data between a
    // date and time picker control (CDateTime) with control numbered id
    // in a dialog box or other window, and a SYSTEMTIME data member of
    // the object. When called in the read mode, the value is set to the
    // current state of the control. When called in the write mode, the
    // current state of the control is set to the given value.
    inline void CDataExchange::DDX_DateTime(UINT id, SYSTEMTIME &value)
    {
        HWND control = PrepareCtrl(id);
        LPARAM lparam = reinterpret_cast<LPARAM>(&value);
        if (m_retrieveAndValidate)
            ::SendMessage(control, DTM_GETSYSTEMTIME, 0, lparam);
        else
            ::SendMessage(control, DTM_SETSYSTEMTIME, 0, lparam);
    }

    // This function manages the transfer of data between a list box
    // control in a dialog box or other window, and an
    // integer index of that object. It performs a data exchange for the
    // state of the list box control appearing within the DDX/DDV object m_DX
    // with the control numbered id. When this function is called, index
    // is set to the index of the current list box selection. If no item
    // is selected, index is set to LB_ERR. When m_DX.m_retrieveAndValidate is
    // false, the index item is selected.
    inline void CDataExchange::DDX_LBIndex(UINT id, int& index)
    {
        HWND control = PrepareCtrl(id);
        if (m_retrieveAndValidate)
            index = static_cast<int>(::SendMessage(control, LB_GETCURSEL, 0, 0));
        else
            ::SendMessage(control, LB_SETCURSEL, static_cast<WPARAM>(index), 0);
    }

    // Perform a data exchange for the state of a list box control on the
    // DDX/DDV object m_DX with the control numbered id using the given
    // CString value. On reading the list box, the selected item is returned
    // in value. If no item is selected, value is set to a string of zero
    // length. When m_retrieveAndValidate is false, the item having the given
    // case-insensitive value as a prefix, if it exists, is selected.
    inline void CDataExchange::DDX_LBString(UINT id, CString& value)
    {
        HWND control = PrepareCtrl(id);
        if (m_retrieveAndValidate)
        {
             // Find the index of the item selected in the list box.
            int index = static_cast<int>(::SendMessage(control, LB_GETCURSEL, 0, 0));
            if (index != -1)
            {
                // Read selected text into the CString.
                int length = static_cast<int>(::SendMessage(control, LB_GETTEXTLEN,
                                              static_cast<WPARAM>(index), 0));

                ::SendMessage(control, LB_GETTEXT, static_cast<WPARAM>(index),
                              reinterpret_cast<LPARAM>(value.GetBuffer(length)));

                value.ReleaseBuffer();
            }
            else
            {
                // No text selected.
                value.Empty();
            }
        }
        else if (!value.IsEmpty())
        {
            // search the the entire list box for the given value
            // and select it if it is found
            int index  = static_cast<int>(::SendMessage(control, LB_SELECTSTRING,
                static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(value.c_str())));

            if (index == LB_ERR)
            {
                // The value string was not found.
                CString str = CString(_T("*** WARNING: listbox item was not found:  ")) + value + _T(". ***\n");
                TRACE(str);
            }
        }
    }

    // This function manages the transfer of CString data between the edit
    // control of a list box control in a dialog box or other window,
    // and a CString data member of that object. It performs a
    // data exchange for the state of the list box control appearing within
    // the DDX/DDV object m_DX with the control numbered id. On reading the
    // list box, the current list box selection is returned as the value.
    // If no item is selected, value is set to a string of zero length. When
    // m_DX.m_retrieveAndValidate is false, the item having the entire given
    // case-insensitive value as its prefix, if any exists, is selected.
    inline void CDataExchange::DDX_LBStringExact(UINT id, CString& value)
    {
        HWND control = PrepareCtrl(id);
        if (m_retrieveAndValidate)
        {
            // Read and return the CString value.
            DDX_LBString(id, value);
        }
        else if (!value.IsEmpty())
        {
            // Find the first entry that matches the entire value,
            // in a case insensitive search, perhaps in sorted order,
            // if the box has that style.
            int index = static_cast<int>(::SendMessage(control, LB_FINDSTRINGEXACT,
                static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(value.c_str())));

            if (index < 0)
            {
                // No match found.
                CString str = (_T("Warning: listbox item was not found:  ")) + value + _T( "\n");
                TRACE(str);
            }
            else
            {
                // Select it.
                ::SendMessage(control, LB_SETCURSEL, static_cast<WPARAM>(index), 0);
            }
        }
    }

    // This function manages the transfer of date data between a month
    // calendar control (CMonthCalendar) with control number id in a dialog
    // box or other window, and a SYSTEMTIME data member
    // of that object. In particular, the control manages a date value only.
    // The time fields in the time object are set to reflect the creation
    // time of the control window, or whatever time was set into the control
    // with a call to the CMonthCalendar::SetCurSel() member method. In read
    // mode, value is set to the current state of the month calendar control.
    // In write mode, the current state is set to the given value.
    inline void CDataExchange::DDX_MonthCal(UINT id, SYSTEMTIME& value)
    {
        HWND control = PrepareCtrl(id);

        if (m_retrieveAndValidate)
        {
            LPARAM lparam = reinterpret_cast<LPARAM>(&value);
            ::SendMessage(control, MCM_GETCURSEL, 0, lparam);
            value.wHour = 0;
            value.wMinute = 0;
            value.wSecond = 0;
            value.wMilliseconds = 0;
        }
        else
        {
            LPARAM lparam = reinterpret_cast<LPARAM>(&value);
            ::SendMessage(control, MCM_SETCURSEL, 0, lparam);
        }
    }

    // This function manages the transfer of data between a progress control
    // in a dialog box or other window, and an integer
    // progress value for that object. It performs a data exchange for the
    // slider control appearing within the DDX/DDV object m_DX with
    // the control numbered id. When this function is called, value is set
    // to the current slider entry.  When m_DX.m_retrieveAndValidate
    // is false, the slider entry is set to value.
    inline void CDataExchange::DDX_Progress(UINT id, int& value)
    {
        HWND control = PrepareCtrl(id);

        if (m_retrieveAndValidate)
            value = static_cast<int>(::SendMessage(control, PBM_GETPOS, 0, 0));
        else
            ::SendMessage(control, PBM_SETPOS, static_cast<WPARAM>(value), 0);
    }

    // Perform a data exchange for the state of a group of auto radio buttons
    // on the DDX/DDV object m_DX with the control numbered id to and from
    // the given int value, where the identifier id is that of the first
    // control in the group.
    inline void CDataExchange::DDX_Radio(UINT id, int& value)
    {
        HWND control = PrepareCtrl(id);

        // Assure that the control is a radio button and part of a group.
        bool firstInGroup = (::GetWindowLongPtr(control, GWL_STYLE) & WS_GROUP) != 0;
        assert(firstInGroup);

        // Assure the button is a radio button.
        bool isRadioButton = (::GetWindowLongPtr(control, GWL_STYLE) & (BS_RADIOBUTTON | BS_AUTORADIOBUTTON)) != 0;
        assert(isRadioButton);

        // Preset the returned value to empty in case no button is set.
        if (m_retrieveAndValidate)
            value = -1;

        // Traverse all buttons in the group.
        // There's a group, so set up for the radio buttons in the group.
        firstInGroup = FALSE;
        for (int button = 0; control != nullptr && !firstInGroup; )
        {
            if (isRadioButton)
            {
                if (m_retrieveAndValidate)
                {
                    if (::SendMessage(control, BM_GETCHECK, 0, 0) == BST_CHECKED) // Is this button set?
                    {
                        // Record the value the first time, but if it happens again, there
                        // is an error--only one button checked is allowed.
                        assert(value == -1);
                        value = button;
                    }
                }
                else
                {
                    WPARAM wparam = static_cast<WPARAM>(button == value);
                    ::SendMessage(control, BM_SETCHECK, wparam, 0);
                }
                button++;
            }
            else
            {
                TRACE(_T("*** Warning: there is a non-radio button in "));
                TRACE(_T("a radio button group. ***\n"));
            }

            // Check the next window in the group, if any.
            control = ::GetWindow(control, GW_HWNDNEXT);
            if (control)
            {
                isRadioButton = (::GetWindowLongPtr(control, GWL_STYLE) & (BS_RADIOBUTTON | BS_AUTORADIOBUTTON)) != 0;
                firstInGroup  = (::GetWindowLongPtr(control, GWL_STYLE) & WS_GROUP) != 0;
            }
        }
    }

    // This function manages the transfer of data between a scroll bar control
    // in a dialog box or other window, and an integer
    // value for that object. It performs a data exchange for the state of
    // the scroll bar control appearing within the DDX/DDV object m_DX with
    // the control numbered id. When this function is called, value is set
    // to the current scroll bar position.  When m_DX.m_retrieveAndValidate
    // is false, the scroll bar position is set to value.
    inline void CDataExchange::DDX_Scroll(UINT id, int& value)
    {
        HWND control = PrepareCtrl(id);

        if (m_retrieveAndValidate)
            value = ::GetScrollPos(control, SB_CTL);
        else
            ::SetScrollPos(control, SB_CTL, value, TRUE);
    }

    // This function manages the transfer of data between a slider control
    // in a dialog box or other window, and an integer
    // position value for that object. It performs a data exchange for the
    // slider control appearing within the DDX/DDV object m_DX with
    // the control numbered id. When this function is called, value is set
    // to the current slider entry.  When m_DX.m_retrieveAndValidate
    // is false, the slider entry is set to value.
    inline void CDataExchange::DDX_Slider(UINT id, int& value)
    {
        HWND control = PrepareCtrl(id);

        if (m_retrieveAndValidate)
            value = static_cast<int>(::SendMessage(control, TBM_GETPOS, 0, 0));
        else
            ::SendMessage(control, TBM_SETPOS, static_cast<WPARAM>(TRUE),
                static_cast<LPARAM>(value));
    }

    ////////////////////////////////////////////////////////////////
    //
    // DDX: Data Exchange Procedures
    //
    ////////////////////////////////////////////////////////////////

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type BYTE.
    inline void CDataExchange::DDX_Text(UINT id, BYTE& value)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            BYTE oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_Byte());
            }
        }
        else
        {
            tStringStream tss;
            tss << static_cast<int>(value);
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type short.
    inline void CDataExchange::DDX_Text(UINT id, short& value)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            short oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_Short());
            }
        }
        else
        {
            tStringStream tss;
            tss << value;
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type int.
    inline void CDataExchange::DDX_Text(UINT id, int& value)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            int oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_Int());
            }
        }
        else
        {
            tStringStream tss;
            tss << value;
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type UINT.
    inline void CDataExchange::DDX_Text(UINT id, UINT& value)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            UINT oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_UINT());
            }
        }
        else
        {
            tStringStream tss;
            tss << value;
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type long.
    inline void CDataExchange::DDX_Text(UINT id, long& value)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            long oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_Long());
            }
        }
        else
        {
            tStringStream tss;
            tss << value;
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type ULONG.
    inline void CDataExchange::DDX_Text(UINT id, ULONG& value)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            ULONG oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_ULONG());
            }
        }
        else
        {
            tStringStream tss;
            tss << value;
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type float with the given precision.
    inline void CDataExchange::DDX_Text(UINT id, float& value, int precision /* = FLT_DIG */)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            float oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_Real());
            }
        }
        else
        {
            tStringStream tss;
            tss << std::setprecision(precision) << value;
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type double with the given precision.
    inline void CDataExchange::DDX_Text(UINT id, double& value, int precision /* = DBL_DIG */)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            double oldvalue = value;
            CString str;
            str.GetWindowText(control);
            tStringStream ts(str.c_str());
            ts >> value;
            if (ts.fail())
            {
                value = oldvalue;
                throw CUserException(GetApp()->MsgDDX_Real());
            }
        }
        else
        {
            tStringStream tss;
            tss << std::setprecision(precision) << value;
            ::SetWindowText(control, tss.str().c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type CString.
    inline void CDataExchange::DDX_Text(UINT id, CString& value)
    {
        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            value.GetWindowText(control);
        }
        else
        {
            ::SetWindowText(control, value.c_str());
        }
    }

    // Perform a text box data exchange on the current DDX/DDV object m_DX with
    // data value of type LPTSTR with the given maximum length.
    inline void CDataExchange::DDX_Text(UINT id, LPTSTR value, int maxLen)
    {
        assert(value);

        HWND control = PrepareEditCtrl(id);
        if (m_retrieveAndValidate)
        {
            CString str;
            str.GetWindowText(control);
            StrCopy(value, str, static_cast<size_t>(maxLen));
        }
        else
        {
            ::SetWindowText(control, value);
        }
    }

    // This function is called when a CUserException is caught while
    // validating the value in a control. This is a virtual function that can
    // be overridden as required.
    inline void CDataExchange::Fail(LPCTSTR message) const
    {
        ::MessageBox(0, message, _T("Error"), MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);

        if (!m_retrieveAndValidate)
        {
            TRACE(_T("*** WARNING: CDataExchange::Fail() called while "));
            TRACE(_T("writing to a control. ***\n"));
        }
        else if (m_lastControl != nullptr)
        {
            if (m_isEditLastControl) // if the offender is an edit control
            {
                // select all characters in the edit item
                ::SetFocus(m_lastControl);
                ::SendMessage(m_lastControl, EM_SETSEL, 0, -1);
            }
        }
        else
        {
            TRACE(_T("*** WARNING: validation failed with no control to "));
            TRACE(_T("restore focus to. ***\n"));
        }
    }

    // Used by CWnd::UpdateData() to initialize the CDataExchange members.
    inline void CDataExchange::Init(CWnd& dlgWnd, BOOL retrieveAndValidate)
    {
        // the window has to be valid
        assert(dlgWnd.IsWindow());

        // record the default action and parent window
        m_retrieveAndValidate = retrieveAndValidate;
        m_parent       = dlgWnd;
        m_lastControl  = nullptr;
    }

    // Find the handle to the control whose numeric identifier is id and
    // record this as the last control handle encountered. Set the last-edit
    // member to a FALSE default value.
    inline HWND CDataExchange::PrepareCtrl(UINT id)
    {
        assert(id != 0);
        assert(static_cast<int>(id) != -1); // not allowed

        HWND    control = ::GetDlgItem(m_parent, static_cast<int>(id));
        assert(control);

        m_lastControl  = control;
        m_isEditLastControl = FALSE; // not an edit item by default
        return control;
    }

    // Get and record the handle corresponding to id and set the m_isEditLastControl
    // variable to TRUE. This method is only used for edit controls.
    inline HWND CDataExchange::PrepareEditCtrl(UINT id)
    {
        HWND control = PrepareCtrl(id);
        assert(control);

        m_isEditLastControl = TRUE;
        return control;
    }


    ////////////////////////////////////////////////////////////////
    //
    //  Global function
    //
    ////////////////////////////////////////////////////////////////

    // Convert the SYSTEMTIME struct to an ULONGLONG integer and
    // return this value.
    inline ULONGLONG SystemTimeToULL(const SYSTEMTIME &systime)
    {
        FILETIME ft;
        SystemTimeToFileTime(&systime, &ft);
        return static_cast<ULONGLONG>(ft.dwHighDateTime) << 32 | ft.dwLowDateTime;
    }

}   // namespace Win32xx

#endif // _WIN32XX_DDX_H_

