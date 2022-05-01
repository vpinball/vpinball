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


///////////////////////////////////////////////////////
// wxx_wincore0.h
// This file contains the declaration of the CWnd class.
//

#ifndef _WIN32XX_WINCORE0_H_
#define _WIN32XX_WINCORE0_H_


#include "wxx_rect.h"
#include "wxx_appcore0.h"


// Messages defined by Win32++
// WM_APP range: 0x8000 through 0xBFFF
// Note: The numbers defined for window messages don't always need to be unique. View windows defined by users for example,
//  could use other user defined messages with the same number as those below without issue.
#define UWM_DOCKACTIVATE      (WM_APP + 0x3F01) // Message - posted to dock ancestor when a docker is activated.
#define UWM_DOCKDESTROYED     (WM_APP + 0x3F02) // Message - posted when docker is destroyed.
#define UWM_DRAWRBBKGND       (WM_APP + 0x3F03) // Message - sent by rebar to parent to perform background drawing. Return TRUE if handled.
#define UWM_DRAWSBBKGND       (WM_APP + 0x3F04) // Message - sent by statusbar to parent to perform background drawing. Return TRUE if handled.
#define UWM_GETCDIALOG        (WM_APP + 0x3F05) // Message - returns a pointer to this CWnd if it inherits from CDialog.
#define UWM_GETCDOCKCONTAINER (WM_APP + 0x3F06) // Message - returns a pointer to this CWnd if it inherits from CDockContainer.
#define UWM_GETCDOCKER        (WM_APP + 0x3F07) // Message - returns a pointer to this CWnd if it inherits from CDocker.
#define UWM_GETCFRAMET        (WM_APP + 0x3F08) // Message - returns a pointer to this CWnd if it inherits from CFrameT.
#define UWM_GETCMENUBAR       (WM_APP + 0x3F09) // Message - returns a pointer to this CWnd if it inherits from CMenuBar.
#define UWM_GETCTABBEDMDI     (WM_APP + 0x3F0A) // Message - returns a pointer to this CWnd if it inherits from CTabbedMDI.
#define UWM_GETCTOOLBAR       (WM_APP + 0x3F0B) // Message - returns a pointer to this CWnd if it inherits from CToolBar.
#define UWM_GETCWND           (WM_APP + 0x3F0C) // Message - returns a pointer to this CWnd.
#define UWM_GETFRAMEVIEW      (WM_APP + 0x3F0D) // Message - returns the HWND of the frame's view window. Used by CMenuBar.
#define UWM_GETMBTHEME        (WM_APP + 0x3F0E) // Message - returns a pointer to MenuBarTheme.
#define UWM_GETRBTHEME        (WM_APP + 0x3F0F) // Message - returns a pointer to ReBarTheme.
#define UWM_GETSBTHEME        (WM_APP + 0x3F10) // Message - returns a pointer to StatusBarTheme.
#define UWM_GETTBTHEME        (WM_APP + 0x3F11) // Message - returns a pointer to ToolBarTheme.
#define UWM_MDIACTIVATED      (WM_APP + 0x3F12) // Message - sent by MDI child to MDIFrame when it is activated.
#define UWM_MDIDESTROYED      (WM_APP + 0x3F13) // Message - sent by MDI client when a MDI child is destroyed.
#define UWM_MDIGETACTIVE      (WM_APP + 0x3F14) // Message - sent by MDI client when the state of a MDI child is queried.
#define UWM_POPUPMENU         (WM_APP + 0x3F15) // Message - creates the menubar popup menu.
#define UWM_TBRESIZE          (WM_APP + 0x3F16) // Message - sent by toolbar to parent. Used by the rebar.
#define UWM_TBWINPOSCHANGING  (WM_APP + 0x3F17) // Message - sent to parent. Toolbar is resizing.
#define UWM_UPDATECOMMAND     (WM_APP + 0x3F18) // Message - sent before a menu is displayed. Used by OnMenuUpdate.

#define UWN_BARSTART          (WM_APP + 0x3F20) // Notification - sent by CDocker when the docker bar selected for move.
#define UWN_BARMOVE           (WM_APP + 0x3F21) // Notification - sent by CDocker when the docker bar is moved.
#define UWN_BAREND            (WM_APP + 0x3F22) // Notification - sent by CDocker when moving the docker bar has ended.
#define UWN_DOCKSTART         (WM_APP + 0x3F23) // Notification - sent by CDocker when it's about to start undocking.
#define UWN_DOCKMOVE          (WM_APP + 0x3F24) // Notification - sent by CDocker when the undocked docker is being moved.
#define UWN_DOCKEND           (WM_APP + 0x3F25) // Notification - sent by CDocker when the docker has been docked.
#define UMN_TABCHANGED        (WM_APP + 0x3F26) // Notification - sent by CTab when the tab size or position changed.
#define UWN_TABDRAGGED        (WM_APP + 0x3F27) // Notification - sent by CTab when the tab is being dragged.
#define UWN_TABCLOSE          (WM_APP + 0x3F28) // Notification - sent by CTab when a tab is about to be closed.
#define UWN_UNDOCKED          (WM_APP + 0x3F29) // Notification - sent by docker when undocked.

// print preview messages
#define UWM_PREVIEWCLOSE      (WM_APP + 0x3F2A) // Message - sent by CPrintPreview when the 'Close' button is pressed.
#define UWM_PREVIEWPRINT      (WM_APP + 0x3F2B) // Message - sent by CPrintPreview when the 'Print Now' button is pressed.
#define UWM_PREVIEWSETUP        (WM_APP + 0x3F2C) // Message - sent by CPrintPreview when the 'Print Setup' is button pressed.


namespace Win32xx
{

    ////////////////////////////////
    // Registered messages defined by Win32++
    const UINT UWM_WINDOWCREATED = ::RegisterWindowMessage(_T("UWM_WINDOWCREATED"));    // Posted when a window is created or attached.

    ////////////////////////////////////////////////////////////////
    // The CWnd class manages a generic window. Other classes
    // which provide the functionality of more specialized windows,
    // inherit from CWnd. These include dialogs and window controls.
    class CWnd : public CObject
    {
    friend class CColorDialog;
    friend class CCommonDialog;
    friend class CDialog;
    friend class CFileDialog;
    friend class CFolderDialog;
    friend class CFontDialog;
    friend class CMDIChild;
    friend class CMessagePump;
    friend class CPageSetupDialog;
    friend class CPrintDialog;
    friend class CPrintDialogEx;
    friend class CPropertyPage;
    friend class CPropertySheet;
    friend class CTaskDialog;
    friend class CWinApp;
    friend class CWinThread;

    public:
        CWnd();             // Constructor
        virtual ~CWnd();    // Destructor

        // These virtual functions can be overridden.
        virtual BOOL Attach(HWND wnd);
        virtual BOOL AttachDlgItem(UINT id, HWND parent);
        virtual void CenterWindow() const;
        virtual HWND Create(HWND parent = 0);
        virtual HWND CreateEx(DWORD exStyle, LPCTSTR className, LPCTSTR windowName,
                              DWORD style, int x, int y, int width, int height, HWND parent,
                              HMENU idOrMenu, LPVOID lparam = NULL);
        virtual HWND CreateEx(DWORD exStyle, LPCTSTR className, LPCTSTR windowName,
                              DWORD style, const RECT& rectc, HWND parent, UINT id, LPVOID lparam = NULL);
        virtual void Destroy();
        virtual HWND Detach();
        static  CWnd* GetCWndPtr(HWND wnd);
        virtual HICON SetIconLarge(int iconID);
        virtual HICON SetIconSmall(int iconID);

        // For Data Exchange
        virtual void DoDataExchange(CDataExchange& dx);
        virtual BOOL UpdateData(CDataExchange& dx, BOOL retrieveAndValidate);

        // Accessors
        HWND GetHwnd() const                { return m_wnd; }
        WNDPROC GetPrevWindowProc() const   { return m_prevWindowProc; }

        // Wrappers for Win32 API functions.
        // These functions aren't virtual, and shouldn't be overridden.
        HDC   BeginPaint(PAINTSTRUCT& ps) const;
        BOOL  BringWindowToTop() const;
        LRESULT CallWindowProc(WNDPROC pPrevWndFunc, UINT msg, WPARAM wparam, LPARAM lparam) const;
        BOOL  CheckDlgButton(int buttonID, UINT check) const;
        BOOL  CheckRadioButton(int firstButtonID, int lastButtonID, int checkButtonID) const;
        CWnd  ChildWindowFromPoint(POINT point) const;
        BOOL  ClientToScreen(POINT& point) const;
        BOOL  ClientToScreen(RECT& rect) const;
        void  Close() const;
        LRESULT DefWindowProc(UINT msg, WPARAM wparam, LPARAM lparam) const;
        HDWP  DeferWindowPos(HDWP winPosInfo, HWND insertAfter, int x, int y, int cx, int cy, UINT flags) const;
        HDWP  DeferWindowPos(HDWP winPosInfo, HWND insertAfter, const RECT& rect, UINT flags) const;
        BOOL  DrawMenuBar() const;
        BOOL  EnableWindow(BOOL Enable = TRUE) const;
        BOOL  EndPaint(PAINTSTRUCT& ps) const;
        CWnd  GetActiveWindow() const;
        CWnd  GetAncestor(UINT flag = 3 /*= GA_ROOTOWNER*/) const;
        CWnd  GetCapture() const;
        ULONG_PTR GetClassLongPtr(int index) const;
        CString GetClassName() const;
        CRect GetClientRect() const;
        CClientDC GetDC() const;
        CClientDCEx GetDCEx(HRGN clip, DWORD flags) const;
        CWnd  GetDesktopWindow() const;
        int   GetDlgCtrlID() const;
        CWnd  GetDlgItem(int dlgItemID) const;
        UINT  GetDlgItemInt(int dlgItemID, BOOL& isTranslated, BOOL isSigned) const;
        UINT  GetDlgItemInt(int dlgItemID, BOOL isSigned) const;
        CString GetDlgItemText(int dlgItemID) const;
        DWORD GetExStyle() const;
        CWnd  GetFocus() const;
        CFont GetFont() const;
        HICON GetIcon(BOOL isBigIcon) const;
        CWnd  GetNextDlgGroupItem(HWND control, BOOL isPrevious) const;
        CWnd  GetNextDlgTabItem(HWND control, BOOL isPrevious) const;
        CWnd  GetParent() const;
        BOOL  GetScrollInfo(int barType, SCROLLINFO& si) const;
        DWORD GetStyle() const;
        CRect GetUpdateRect(BOOL erase) const;
        int GetUpdateRgn(HRGN rgn, BOOL erase) const;
        CWnd  GetWindow(UINT cmd) const;
        CWindowDC GetWindowDC() const;
        LONG_PTR GetWindowLongPtr(int index) const;
        CRect GetWindowRect() const;
        CString GetWindowText() const;
        int   GetWindowTextLength() const;
        void  Invalidate(BOOL erase = TRUE) const;
        BOOL  InvalidateRect(const RECT& rect, BOOL erase = TRUE) const;
        BOOL  InvalidateRect(BOOL erase = TRUE) const;
        BOOL  InvalidateRgn(HRGN rgn, BOOL erase = TRUE) const;
        BOOL  IsChild(HWND child) const;
        BOOL  IsDialogMessage(MSG& msg) const;
        UINT  IsDlgButtonChecked(int buttonID) const;
        BOOL  IsWindow() const;
        BOOL  IsWindowEnabled() const;
        BOOL  IsWindowVisible() const;
        BOOL  KillTimer(UINT_PTR eventID) const;
        int   MessageBox(LPCTSTR text, LPCTSTR caption, UINT type) const;
        int   MapWindowPoints(HWND to, POINT& point) const;
        int   MapWindowPoints(HWND to, RECT& rect) const;
        int   MapWindowPoints(HWND to, LPPOINT pointsArray, UINT count) const;
        BOOL  MoveWindow(int x, int y, int width, int height, BOOL repaint = TRUE) const;
        BOOL  MoveWindow(const RECT& rect, BOOL repaint = TRUE) const;
        BOOL  PostMessage(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) const;
        BOOL  PostMessage(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) const;
        BOOL  RedrawWindow(const RECT& updateRect, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN) const;
        BOOL  RedrawWindow(HRGN rgn, UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN) const;
        BOOL  RedrawWindow(UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN) const;
        int   ReleaseDC(HDC dc) const;
        BOOL  ScreenToClient(POINT& point) const;
        BOOL  ScreenToClient(RECT& rect) const;
        LRESULT SendDlgItemMessage(int dlgItemID, UINT msg, WPARAM wparam, LPARAM lparam) const;
        LRESULT SendMessage(UINT msg, WPARAM wparam = 0, LPARAM lparam = 0) const;
        LRESULT SendMessage(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam) const;
        BOOL  SendNotifyMessage(UINT msg, WPARAM wparam, LPARAM lparam) const;
        HWND  SetActiveWindow() const;
        HWND  SetCapture() const;
        ULONG_PTR SetClassLongPtr(int index, LONG_PTR newLong) const;
        LONG_PTR SetDlgCtrlID(int id) const;
        BOOL  SetDlgItemInt(int dlgItemID, UINT value, BOOL isSigned) const;
        BOOL  SetDlgItemText(int dlgItemID, LPCTSTR string) const;
        void  SetExStyle(DWORD exStyle) const;
        HWND  SetFocus() const;
        void  SetFont(HFONT font, BOOL redraw = TRUE) const;
        BOOL  SetForegroundWindow() const;
        HICON SetIcon(HICON icon, BOOL isBigIcon) const;
        HWND  SetParent(HWND parent) const;
        BOOL  SetRedraw(BOOL redraw = TRUE) const;
        int   SetScrollInfo(int barType, const SCROLLINFO& si, BOOL redraw) const;
        void  SetStyle(DWORD style) const;
        UINT_PTR SetTimer(UINT_PTR eventID, UINT elapse, TIMERPROC pTimerFunc) const;
        LONG_PTR SetWindowLongPtr(int index, LONG_PTR newLong) const;
        BOOL  SetWindowPos(HWND insertAfter, int x, int y, int cx, int cy, UINT flags) const;
        BOOL  SetWindowPos(HWND insertAfter, const RECT& rect, UINT flags) const;
        int   SetWindowRgn(HRGN rgn, BOOL redraw = TRUE) const;
        BOOL  SetWindowText(LPCTSTR text) const;
        HRESULT SetWindowTheme(LPCWSTR subAppName, LPCWSTR subIdList) const;
        BOOL  ShowWindow(int showCmd = SW_SHOWNORMAL) const;
        BOOL  UpdateWindow() const;
        BOOL  ValidateRect(const RECT& rect) const;
        BOOL  ValidateRect() const;
        BOOL  ValidateRgn(HRGN rgn) const;
        CWnd  WindowFromPoint(POINT point) const;

        BOOL  CloseWindow() const;
        int   DlgDirList(LPTSTR pathSpec, int listBoxID, int staticPathID, UINT fileType) const;
        int   DlgDirListComboBox(LPTSTR pathSpec, int comboBoxID, int staticPathID, UINT filetype) const;
        BOOL  DlgDirSelectEx(LPTSTR string, int count, int listBoxID) const;
        BOOL  DlgDirSelectComboBoxEx(LPTSTR string, int count, int comboBoxID) const;
        BOOL  DrawAnimatedRects(int aniID, const RECT& from, const RECT& to) const;
        BOOL  DrawCaption(HDC dc, const RECT& rect, UINT flags) const;
        BOOL  EnableScrollBar(UINT flags, UINT arrows) const;
        CWnd  GetLastActivePopup() const;
        CMenu GetMenu() const;
        int   GetScrollPos(int barType) const;
        BOOL  GetScrollRange(int barType, int& minPos, int& maxPos) const;
        CMenu GetSystemMenu(BOOL revertToDefault = FALSE) const;
        CWnd  GetTopWindow() const;
        BOOL  GetWindowPlacement(WINDOWPLACEMENT& wp) const;
        BOOL  HiliteMenuItem(HMENU menu, UINT itemID, UINT hilite) const;
        BOOL  IsIconic() const;
        BOOL  IsZoomed() const;
        BOOL  LockWindowUpdate() const;
        BOOL  OpenIcon() const;
        void  Print(HDC dc, DWORD flags) const;
        BOOL  SetMenu(HMENU menu) const;
        BOOL  ScrollWindow(int xAmount, int yAmount, const RECT& scrollRect, LPCRECT pClipRect = 0) const;
        BOOL  ScrollWindow(int xAmount, int yAmount, LPCRECT pClipRect = 0) const;
        int   ScrollWindowEx(int dx, int dy, LPCRECT pScrollRect, LPCRECT pClipRect, HRGN updateRgn, LPRECT updateRect, UINT flags) const;
        int   SetScrollPos(int barType, int pos, BOOL redraw) const;
        BOOL  SetScrollRange(int barType, int minPos, int maxPos, BOOL redraw) const;
        BOOL  SetWindowPlacement(const WINDOWPLACEMENT& wndpl) const;
        BOOL  ShowOwnedPopups(BOOL show) const;
        BOOL  ShowScrollBar(int barType, BOOL show) const;
        BOOL  ShowWindowAsync(int showCmd) const;
        BOOL  UnLockWindowUpdate() const;
        CWnd  WindowFromDC(HDC dc) const;

    #ifndef WIN32_LEAN_AND_MEAN
        void  DragAcceptFiles(BOOL accept) const;
    #endif

        operator HWND() const { return GetHwnd(); }

    protected:
        // Override these functions as required
        virtual LRESULT FinalWindowProc(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void OnAttach();
        virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
        virtual void OnClose();
        virtual int  OnCreate(CREATESTRUCT& cs);
        virtual void OnDestroy();
        virtual void OnDraw(CDC& dc);
        virtual BOOL OnEraseBkgnd(CDC& dc);
        virtual void OnInitialUpdate();
        virtual void OnMenuUpdate(UINT id);
        virtual LRESULT OnMessageReflect(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);
        virtual LRESULT OnPaint(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void PreCreate(CREATESTRUCT& cs);
        virtual void PreRegisterClass(WNDCLASS& wc);
        virtual BOOL PreTranslateMessage(MSG& msg);
        virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

        // Not intended to be overridden
        virtual LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        CWnd(const CWnd&);              // Disable copy construction
        CWnd& operator = (const CWnd&); // Disable assignment operator
        CWnd(HWND wnd);                 // Private constructor used internally

        static LRESULT CALLBACK StaticWindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

        void AddToMap();
        void Cleanup();
        LRESULT MessageReflect(UINT msg, WPARAM wparam, LPARAM lparam);
        BOOL RegisterClass(WNDCLASS& wc);
        BOOL RemoveFromMap();
        void Subclass(HWND wnd);

        HWND m_wnd;                    // handle to this object's window
        WNDPROC m_prevWindowProc;
    }; // class CWnd

} // namespace Win32xx

#endif // _WIN32XX_WINCORE0_H_

