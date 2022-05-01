// Win32++   Version 9.0
// Release Date: 30th April 2022
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



#ifndef _WIN32XX_MESSAGEPUMP_H_
#define _WIN32XX_MESSAGEPUMP_H_

#include "wxx_messagepump0.h"

namespace Win32xx
{
    // InitInstance is called when the thread or application starts.
    // Override this function to perform tasks such as creating a window.
    // Return TRUE to indicate success and run the message loop.
    inline BOOL CMessagePump::InitInstance()
    {
        return TRUE;
    }

    // This function translates the thread's window message and dispatches
    // them to a window procedure.
    inline int CMessagePump::MessageLoop()
    {
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        int status = 1;
        LONG lCount = 0;

        while (status != 0)
        {
            // While idle, perform idle processing until OnIdle returns FALSE
            while (!::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE) && OnIdle(lCount) != FALSE)
                ++lCount;

            lCount = 0;

            // Now wait until we get a message
            if ((status = ::GetMessage(&msg, NULL, 0, 0)) == -1)
                return -1;

            if (!PreTranslateMessage(msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }

        }

        return LOWORD(msg.wParam);
    }

    // This function is called by the MessageLoop. It is called when the message queue
    // is empty. Return TRUE to continue idle processing or FALSE to end idle processing
    // until another message is queued. The count is incremented each time OnIdle is
    // called, and reset to 0 each time a new messages is processed.
    inline BOOL CMessagePump::OnIdle(LONG)
    {
        return FALSE;
    }

    // Override this function if your class requires input messages to be
    // translated before normal processing.
    // Return TRUE if the message is translated.
    inline BOOL CMessagePump::PreTranslateMessage(MSG& msg)
    {
        BOOL isProcessed = FALSE;

        // only pre-translate mouse and keyboard input events
        if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
            (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
        {
            // Process keyboard accelerators
            if (::TranslateAccelerator(GetAcceleratorsWindow(), GetAcceleratorTable(), &msg))
                isProcessed = TRUE;
            else
            {
                // Search the chain of parents for pretranslated messages.
                for (HWND wnd = msg.hwnd; wnd != 0; wnd = ::GetParent(wnd))
                {
                    CWnd* pWnd = GetApp()->GetCWndFromMap(wnd);
                    if (pWnd)
                    {
                        isProcessed = pWnd->PreTranslateMessage(msg);
                        if (isProcessed)
                            break;
                    }
                }
            }
        }

        return isProcessed;
    }

    // Calls InitInstance and runs the message loop.
    inline int CMessagePump::Run()
    {
        // InitInstance runs the App's initialization code
        if (InitInstance())
        {
            // Dispatch the window messages
            return MessageLoop();
        }
        else
        {
            TRACE("InitInstance failed!  Terminating the thread\n");
            ::PostQuitMessage(-1);
            return -1;
        }
    }

    // accel is the handle of the accelerator table
    // accelWnd is the window handle for translated messages.
    inline void CMessagePump::SetAccelerators(HACCEL accel, HWND accelWnd)
    {
        m_accelWnd = accelWnd;
        m_accel = accel;
    }

}

#endif // _WIN32XX_MESSAGEPUMP_H_