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


#ifndef _WIN32XX_MESSAGEPUMP0_H_
#define _WIN32XX_MESSAGEPUMP0_H_

#include "wxx_textconv.h"

namespace Win32xx
{

    class CArchive;

    ////////////////////////////////////////////////////////////////////
    // The CObject class provides support for Serialization by CArchive.
    // Any class that uses CArchive to serialize data can inherit from
    // CObject.

    class CObject
    {
    public:
        CObject() {}
        virtual ~CObject() {}

        virtual void Serialize(CArchive& /*ar*/)
        {
            //  Override Serialize in the class inherited from CObject like this.

            //  if (ar.IsStoring())
            //  {
            //      // Store a member variable in the archive
            //      ar << m_someValue;
            //  }
            //  else
            //  {
            //      // Load a member variable from the archive
            //      ar >> m_someValue;
            //  }
        }
    };

    class CMessagePump : public CObject
    {
    public:
        CMessagePump() : m_accel(0), m_accelWnd(0) {}
        virtual ~CMessagePump() {}

        HACCEL GetAcceleratorTable() const { return m_accel; }
        HWND   GetAcceleratorsWindow() const { return m_accelWnd; }
        void   SetAccelerators(HACCEL accel, HWND accelWnd);

        // Override this function as required.
        virtual int  Run();

    protected:
        // Override these functions as required.
        virtual BOOL InitInstance();
        virtual int  MessageLoop();
        virtual BOOL OnIdle(LONG count);
        virtual BOOL PreTranslateMessage(MSG& msg);

    private:
        CMessagePump(const CMessagePump&) = delete;
        CMessagePump& operator=(const CMessagePump&) = delete;

        HACCEL m_accel;               // handle to the accelerator table
        HWND m_accelWnd;              // handle to the window for accelerator keys
    };

}

#endif // _WIN32XX_MESSAGEPUMP0_H_
