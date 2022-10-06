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

// Acknowledgement:
// ================
// A special thanks to Robert C. Tausworthe for his assistance
// in developing this class.


#ifndef _WIN32XX_PREVIEWDIALOG_H_
#define _WIN32XX_PREVIEWDIALOG_H_

#include "wxx_wincore.h"
#include "wxx_dialog.h"
#include "wxx_printdialogs.h"
#include "default_resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPrintPreview provides a preview of printed page before sending the print
// job to the printer. CPrintPreview creates a memory device context from the
// printer's device context. Instead of printing directly to the printer, we
// print to the memory DC instead.


// To use CPrintPreview, do the following:
// 1) Declare CPrintPreview as CMainFrame's member variable, specifying the
//    source's class as the template type. The source is where the PrintPage
//    function resides.
// 2) Specify values for the string resources used by CPrintPreview in
//    resource.rc.
// 3) Use SetSource to specify to where to call the PrintPage function.
// 4) Declare a PrintPage function in the source for printing and previewing:
//     void  PrintPage(CDC& dc, UINT page);
// 5) Call DoPrintPreview(HWND ownerWindow, UINT maxPage = 1) to initiate the
//    print preview.
// 6) Create the preview window, and swap it into the frame's view.
// 7) Handle UWM_PREVIEWCLOSE to swap back to the default view when the preview
//    closes.

// CPrintPreview calls the view's PrintPage function. This is the same
// function used to print a specified page. When printing, the PrintPage uses
// the specified printer DC. When previewing we supply the memory DC for the
// printer to the PrintPage function instead.

// After CPrintPreview calls PrintPage, it extracts the bitmap and displays
// it in CPrintPreview's preview pane.


namespace Win32xx
{
/*
    103 DIALOGEX 0, 0, 309, 178
    STYLE DS_SHELLFONT | WS_CHILD | WS_CLIPCHILDREN
    EXSTYLE WS_EX_CLIENTEDGE
    CAPTION ""
    FONT 8, "MS Shell Dlg", 400, 0
    {
       CONTROL "", 96, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 246, 2, 50, 14
       CONTROL "", 92, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 4, 2, 50, 14
       CONTROL "", 93, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 55, 2, 50, 14
       CONTROL "", 94, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 125, 2, 50, 14
       CONTROL "", 95, BUTTON, BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 176, 2, 50, 14
       CONTROL "", 97, "PreviewPane", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 0, 18, 309, 159
    }
*/

    // template of the PrintPreview dialog (used in place of a resource script).
    static unsigned char previewTemplate[] =
    {
        0x01,0x00,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x48,0x00,0x00,0x42,0x06,
        0x00,0x00,0x00,0x00,0x00,0x35,0x01,0xb2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,
        0x00,0x90,0x01,0x00,0x01,0x4d,0x00,0x53,0x00,0x20,0x00,0x53,0x00,0x68,0x00,0x65,
        0x00,0x6c,0x00,0x6c,0x00,0x20,0x00,0x44,0x00,0x6c,0x00,0x67,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0xf6,0x00,0x02,0x00,0x32,
        0x00,0x0e,0x00,0x60,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x04,0x00,0x02,0x00,0x32,
        0x00,0x0e,0x00,0x5c,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x37,0x00,0x02,0x00,0x32,
        0x00,0x0e,0x00,0x5d,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x7d,0x00,0x02,0x00,0x32,
        0x00,0x0e,0x00,0x5e,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0xb0,0x00,0x02,0x00,0x32,
        0x00,0x0e,0x00,0x5f,0x00,0x00,0x00,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x50,0x00,0x00,0x12,0x00,0x35,
        0x01,0x9f,0x00,0x61,0x00,0x00,0x00,0x50,0x00,0x72,0x00,0x65,0x00,0x76,0x00,0x69,
        0x00,0x65,0x00,0x77,0x00,0x50,0x00,0x61,0x00,0x6e,0x00,0x65,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00
    };


    //////////////////////////////////////////////
    // CPreviewPane provides the preview pane used
    // by CPrintPreview.
    class CPreviewPane : public CWnd
    {
    public:
        CPreviewPane();
        virtual ~CPreviewPane() {}

        void Render(CDC& dc);
        void SetBitmap(CBitmap bitmap) { m_bitmap = bitmap; }

    protected:
        virtual void OnDraw(CDC& dc);
        virtual BOOL OnEraseBkgnd(CDC&);
        virtual LRESULT OnPaint(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CPreviewPane(const CPreviewPane&);               // Disable copy construction
        CPreviewPane& operator = (const CPreviewPane&);  // Disable assignment operator
        CBitmap m_bitmap;
    };


    ////////////////////////////////////////////////////////////
    // CPrintPreview adds print preview support to applications.
    // It is used to view print output before it is sent to the
    // printer.
    template <typename T>
    class CPrintPreview : public CDialog
    {
    public:
        CPrintPreview();
        virtual ~CPrintPreview();

        virtual CPreviewPane& GetPreviewPane()  { return m_previewPane; }
        virtual void DoPrintPreview(HWND ownerWindow, UINT maxPage = 1);
        virtual BOOL OnCloseButton();
        virtual BOOL OnNextButton();
        virtual BOOL OnPrevButton();
        virtual BOOL OnPrintButton();
        virtual BOOL OnPrintSetup();
        virtual void PreviewPage(UINT page);
        virtual void SetSource(T& source) { m_pSource = &source; }
        virtual void UpdateButtons();

    protected:
        virtual void OnCancel() { OnCloseButton(); }
        virtual BOOL OnInitDialog();
        virtual void OnOK() { OnCloseButton(); }
        virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);

    private:
        CPrintPreview(const CPrintPreview&);               // Disable copy construction
        CPrintPreview& operator = (const CPrintPreview&);  // Disable assignment operator
        CPreviewPane m_previewPane;
        CResizer m_resizer;
        T*      m_pSource;
        CButton m_buttonPrint;
        CButton m_buttonSetup;
        CButton m_buttonNext;
        CButton m_buttonPrev;
        CButton m_buttonClose;
        UINT    m_currentPage;
        UINT    m_maxPage;
        HWND    m_ownerWindow;
    };

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


namespace Win32xx
{

    //////////////////////////////////////////////////////
    // Definitions for the CPreviewPane class
    // CPreviewPane provides a preview pane for CPrintPreview
    //

    // Constructor.
    inline CPreviewPane::CPreviewPane()
    {
        // The class name of this custom dialog control.
        // This matches the control's class name used in the dialog template.
        CString className = _T("PreviewPane");

        // Register the window class for use as a custom control in the dialog.
        WNDCLASS wc;
        ZeroMemory(&wc, sizeof(WNDCLASS));

        if (!::GetClassInfo(GetApp()->GetInstanceHandle(), className, &wc))
        {
            wc.lpszClassName = className;
            wc.lpfnWndProc = ::DefWindowProc;
            wc.hInstance = GetApp()->GetInstanceHandle();
            wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
            wc.hCursor = ::LoadCursor(0, IDC_ARROW);
            VERIFY(::RegisterClass(&wc));
        }

        assert(::GetClassInfo(GetApp()->GetInstanceHandle(), className, &wc));
    }

    // Handle the WM_ERASEBKGND message.
    inline BOOL CPreviewPane::OnEraseBkgnd(CDC&)
    {
        // Suppress background drawing to avoid flicker on Windows XP and earlier.
        return TRUE;
    }

    // Called to perform the drawing on this window.
    inline void CPreviewPane::OnDraw(CDC& dc)
    {
        Render(dc);
    }

    // Handle the WM_PAINT message and call OnDraw.
    // Normally OnDraw is suppressed for controls, but we need OnDraw for this window.
    inline LRESULT CPreviewPane::OnPaint(UINT, WPARAM, LPARAM)
    {
        if (::GetUpdateRect(*this, NULL, FALSE))
        {
            CPaintDC dc(*this);
            OnDraw(dc);
        }
        else
        // RedrawWindow can require repainting without an update rect.
        {
            CClientDC dc(*this);
            OnDraw(dc);
        }

        // No more drawing required
        return 0;
    }

    // Copies the bitmap (m_Bitmap) to the PreviewPane as a Device
    // Independent Bitmap (DIB), scaling the image to fit the window.
    inline void CPreviewPane::Render(CDC& dc)
    {
        if (m_bitmap.GetHandle())
        {
            BITMAP bm = m_bitmap.GetBitmapData();
            int border = 10;
            CRect rcClient = GetClientRect();

            double ratio = double(bm.bmHeight) / double(bm.bmWidth);
            int previewWidth;
            int previewHeight;

            // These borders center the preview with the PreviewPane.
            int xBorder = border;
            int yBorder = border;
            double cxClient = rcClient.Width();
            double cyClient = rcClient.Height();

            if ((cxClient - 2.0 * border)*ratio < (cyClient - 2.0 * border))
            {
                previewWidth = rcClient.Width() - (2 * border);
                previewHeight = static_cast<int>(previewWidth * ratio);
                yBorder = (rcClient.Height() - previewHeight) / 2;
            }
            else
            {
                previewHeight = rcClient.Height() - (2 * border);
                previewWidth = static_cast<int>(previewHeight / ratio);
                xBorder = (rcClient.Width() - previewWidth) / 2;
            }

            // Create the LPBITMAPINFO from the bitmap.
            CBitmapInfoPtr pbmi(m_bitmap);
            BITMAPINFOHEADER* pBIH = reinterpret_cast<BITMAPINFOHEADER*>(pbmi.get());

            // Extract the device independent image data.
            CMemDC memDC(dc);
            UINT scanLines = static_cast<UINT>(bm.bmHeight);
            memDC.GetDIBits(m_bitmap, 0, scanLines, NULL, pbmi, DIB_RGB_COLORS);
            std::vector<byte> byteArray(pBIH->biSizeImage, 0);
            byte* pByteArray = &byteArray.front();
            memDC.GetDIBits(m_bitmap, 0, scanLines, pByteArray, pbmi, DIB_RGB_COLORS);

            // Use half tone stretch mode for smoother rendering.
            dc.SetStretchBltMode(HALFTONE);
            dc.SetBrushOrgEx(0, 0);

            // Copy the DIB bitmap data to the PreviewPane's DC with stretching.
            dc.StretchDIBits(xBorder, yBorder, previewWidth, previewHeight, 0, 0,
                   bm.bmWidth, bm.bmHeight, pByteArray, pbmi, DIB_RGB_COLORS, SRCCOPY);

            // Draw a gray border around the preview.
            CRect rcFill(0, 0, xBorder, previewHeight + yBorder);
            dc.FillRect(rcFill, HBRUSH(::GetStockObject(GRAY_BRUSH)));

            rcFill.SetRect(0, 0, previewWidth + xBorder, yBorder);
            dc.FillRect(rcFill, HBRUSH(::GetStockObject(GRAY_BRUSH)));

            rcFill.SetRect(previewWidth + xBorder, 0, rcClient.Width(), rcClient.Height());
            dc.FillRect(rcFill, HBRUSH(::GetStockObject(GRAY_BRUSH)));

            rcFill.SetRect(0, previewHeight + yBorder, rcClient.Width(), rcClient.Height());
            dc.FillRect(rcFill, HBRUSH(::GetStockObject(GRAY_BRUSH)));
        }
    }

    ///////////////////////////////////////////
    // Definitions for the CPrintPreview class
    //

    // Constructor.
    template <typename T>
    inline CPrintPreview<T>::CPrintPreview() : CDialog((LPCDLGTEMPLATE)previewTemplate),
        m_pSource(0), m_currentPage(0), m_maxPage(1), m_ownerWindow(0)
    {
    }

    // Destructor.
    template <typename T>
    inline CPrintPreview<T>::~CPrintPreview()
    {
    }

    // The dialog's window procdure. It handles the dialog's window messages.
    template <typename T>
    inline INT_PTR CPrintPreview<T>::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Pass resizing messages on to the resizer
        m_resizer.HandleMessage(msg, wparam, lparam);

        //  switch (msg)
        //  {
        //  Additional messages to be handled go here
        //  }

        // Pass unhandled messages on to parent DialogProc
        return DialogProcDefault(msg, wparam, lparam);
    }

    // Called when the close button is pressed.
    template <typename T>
    inline BOOL CPrintPreview<T>::OnCloseButton()
    {
        ::SendMessage(m_ownerWindow, UWM_PREVIEWCLOSE, 0, 0);
        return TRUE;
    }

    // Processes the dialog's buttons.
    template <typename T>
    inline BOOL CPrintPreview<T>::OnCommand(WPARAM wparam, LPARAM)
    {
        UINT id = LOWORD(wparam);
        switch (id)
        {
        case IDW_PREVIEWPRINT:   return OnPrintButton();
        case IDW_PREVIEWSETUP:   return OnPrintSetup();
        case IDW_PREVIEWPREV:    return OnPrevButton();
        case IDW_PREVIEWNEXT:    return OnNextButton();
        case IDW_PREVIEWCLOSE:   return OnCloseButton();
        }

        return FALSE;
    }

    // Called when the dialog is initialized.
    template <typename T>
    inline BOOL CPrintPreview<T>::OnInitDialog()
    {
        // Attach the dialog controls to CWnd objects
        AttachItem(IDW_PREVIEWPRINT, m_buttonPrint);
        AttachItem(IDW_PREVIEWSETUP, m_buttonSetup);
        AttachItem(IDW_PREVIEWPREV, m_buttonPrev);
        AttachItem(IDW_PREVIEWNEXT, m_buttonNext);
        AttachItem(IDW_PREVIEWCLOSE, m_buttonClose);
        AttachItem(IDW_PREVIEWPANE, m_previewPane);

        // Assign button text from string resources
        m_buttonPrint.SetWindowText(LoadString(IDW_PREVIEWPRINT));
        m_buttonSetup.SetWindowText(LoadString(IDW_PREVIEWSETUP));
        m_buttonPrev.SetWindowText(LoadString(IDW_PREVIEWPREV));
        m_buttonNext.SetWindowText(LoadString(IDW_PREVIEWNEXT));
        m_buttonClose.SetWindowText(LoadString(IDW_PREVIEWCLOSE));

        // Support dialog resizing
        m_resizer.Initialize(*this, CRect(0, 0, 100, 120));
        m_resizer.AddChild(m_buttonPrint, CResizer::topleft, 0);
        m_resizer.AddChild(m_buttonSetup, CResizer::topleft, 0);
        m_resizer.AddChild(m_buttonPrev, CResizer::topleft, 0);
        m_resizer.AddChild(m_buttonNext, CResizer::topleft, 0);
        m_resizer.AddChild(m_buttonClose, CResizer::topleft, 0);
        m_resizer.AddChild(m_previewPane, CResizer::topleft, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);

        return TRUE;
    }

    // Called when the Next Page button is pressed.
    template <typename T>
    inline BOOL CPrintPreview<T>::OnNextButton()
    {
        PreviewPage(++m_currentPage);
        UpdateButtons();

        return TRUE;
    }

    // Called when the Prev Page button is pressed.
    template <typename T>
    inline BOOL CPrintPreview<T>::OnPrevButton()
    {
        PreviewPage(--m_currentPage);
        UpdateButtons();

        return TRUE;
    }

    // Called when the Print button is pressed.
    // Sends the UWM_PREVIEWPRINT message to the owner window.
    template <typename T>
    inline BOOL CPrintPreview<T>::OnPrintButton()
    {
        ::SendMessage(m_ownerWindow, UWM_PREVIEWPRINT, 0, 0);
        return TRUE;
    }

    // Called in response to the Print Setup button.
    // Sends a UWM_PREVIEWSETUP message to the owner.
    template <typename T>
    inline BOOL CPrintPreview<T>::OnPrintSetup()
    {
        ::SendMessage(m_ownerWindow, UWM_PREVIEWSETUP, 0, 0);
        return TRUE;
    }

    // Initiate the print preview.
    // ownerWindow: Print Preview's notifications are sent to this window.
    template <typename T>
    inline void CPrintPreview<T>::DoPrintPreview(HWND ownerWindow, UINT maxPage)
    {
        m_ownerWindow = ownerWindow;
        assert(maxPage >= 1);
        m_maxPage = maxPage;

        // Preview the first page;
        m_currentPage = 0;
        PreviewPage(0);
    }

    // Preview's the specified page.
    // This function calls the view's PrintPage function to render the same
    // information that would be printed on a page.
    // A CResourceException is thrown if there is no default printer.
    template <typename T>
    inline void CPrintPreview<T>::PreviewPage(UINT page)
    {
        // Get the device context of the default or currently chosen printer
        CPrintDialog printDlg;
        CDC printerDC = printDlg.GetPrinterDC();

        // Create a memory DC for the printer.
        // Note: we use the printer's DC here to render text accurately.
        CMemDC memDC(printerDC);

        // Create a compatible bitmap for the memory DC
        int width = printerDC.GetDeviceCaps(HORZRES);
        int height = printerDC.GetDeviceCaps(VERTRES);

        // A bitmap to hold all the pixels of the printed page would be too large.
        // Shrinking its dimensions by 4 reduces it to 1/16th its original size.
        int shrink = width > 8000 ? 8 : 4;
        memDC.CreateCompatibleBitmap(printerDC, width / shrink, height / shrink);

        memDC.SetMapMode(MM_ANISOTROPIC);
        memDC.SetWindowExtEx(width, height, NULL);
        memDC.SetViewportExtEx(width / shrink, height / shrink, NULL);

        // Fill the bitmap with a white background
        CRect rc(0, 0, width, height);
        memDC.FillRect(rc, (HBRUSH)::GetStockObject(WHITE_BRUSH));

        // Call PrintPage from the source.
        assert(m_pSource);
        m_pSource->PrintPage(memDC, page);

        // Detach the bitmap from the memory DC and save it
        CBitmap bitmap = memDC.DetachBitmap();
        GetPreviewPane().SetBitmap(bitmap);

        // Display the print preview
        UpdateButtons();
        CDC previewDC = GetPreviewPane().GetDC();
        GetPreviewPane().Render(previewDC);
    }

    // Enables or disables the page selection buttons.
    template <typename T>
    inline void CPrintPreview<T>::UpdateButtons()
    {
        m_buttonNext.EnableWindow(m_currentPage < m_maxPage - 1);
        m_buttonPrev.EnableWindow(m_currentPage > 0);
    }

}


#endif // _WIN32XX_PREVIEWDIALOG_H_
