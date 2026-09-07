// Win32++   Version 10.3.0
// Release Date: 4th September 2026
//
//      David Nash
//      email: dnash@bigpond.net.au
//      url: https://sourceforge.net/projects/win32-framework
//           https://github.com/DavidNash2024/Win32xx
//
//
// Copyright (c) 2005-2026  David Nash
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


#ifndef WIN32XX_TOOLBAR_H_
#define WIN32XX_TOOLBAR_H_

#include "wxx_wincore.h"


namespace Win32xx
{

    ////////////////////////////////////////////////
    // The CToolBar class provides the functionality
    // a toolbar control.
    class CToolBar : public CWnd
    {
    public:
        CToolBar();
        virtual ~CToolBar() override = default;

        // Operations
        virtual BOOL AddButton(UINT buttonID, BOOL isEnabled = TRUE,
            int image = -1);
        virtual void Destroy() override;
        virtual BOOL SetButtonText(UINT buttonID, LPCTSTR text);

        // Wrappers for Win32 API functions
        BOOL  AddButtons(UINT buttonCount, LPTBBUTTON pButtonInfoArray);
        int   AddString(UINT stringID);
        int   AddStrings(LPCTSTR strings);
        void  Autosize();
        BOOL  CheckButton(UINT buttonID, BOOL isChecked);
        int   CommandToIndex(UINT buttonID) const;
        void  Customize();
        BOOL  DeleteButton(int index);
        BOOL  DisableButton(UINT buttonID);
        BOOL  EnableButton(UINT buttonID, BOOL isEnabled = TRUE);
        BOOL  GetButton(int index, TBBUTTON& buttonInfo) const;
        int   GetButtonCount() const;
        CSize GetButtonSize() const;
        UINT  GetButtonState(UINT buttonID) const;
        BYTE  GetButtonStyle(UINT buttonID) const;
        CString GetButtonText(UINT buttonID) const;
        UINT  GetCommandID(int index) const;
        CImageList GetDisabledImageList() const;
        DWORD GetExtendedStyle() const;
        CImageList GetHotImageList() const;
        int   GetHotItem() const;
        CImageList GetImageList() const;
        CRect GetItemRect(int index) const;
        CSize GetMaxSize() const;
        CSize GetPadding() const;
        CRect GetRect(UINT buttonID) const;
        int   GetRows() const;
        int   GetTextRows() const;
        HWND  GetToolTips() const;
        BOOL  HasText() const;
        BOOL  HideButton(UINT buttonID, BOOL hide);
        int   HitTest(POINT pt) const;
        int   HitTest() const;
        BOOL  Indeterminate(UINT buttonID, BOOL isIndeterminate);
        BOOL  InsertButton(int index, const TBBUTTON& buttonInfo);
        BOOL  IsButtonHidden(UINT buttonID) const;
        BOOL  IsButtonHighlighted(UINT buttonID) const;
        BOOL  IsButtonIndeterminate(UINT buttonID) const;
        BOOL  IsButtonPressed(UINT buttonID) const;
        int   MapAccelerator(TCHAR accelChar);
        BOOL  MarkButton(UINT buttonID, BOOL highlight = TRUE );
        BOOL  MoveButton(UINT oldPos, UINT newPos);
        BOOL  PressButton(UINT buttonID, BOOL press);
        void  SaveRestore(BOOL save, TBSAVEPARAMS* pSaveInfo);
        BOOL  SetBitmapSize(int cx, int cy);
        void  SetButtonInfo(UINT buttonID, UINT buttonNewID, int image,
            BYTE style = 0xFF, BYTE state = 0xFF);
        BOOL  SetButtonInfo(UINT buttonID, const TBBUTTONINFO& tbbi);
        BOOL  SetButtonSize(int cx, int cy);
        BOOL  SetButtonState(UINT buttonID, UINT state) ;
        BOOL  SetButtonStyle(UINT buttonID, BYTE style);
        BOOL  SetButtonWidth(UINT buttonID, int width);
        BOOL  SetCommandID(int index, UINT buttonID);
        CImageList SetDisableImageList(HIMAGELIST disabledImages);
        DWORD SetDrawTextFlags(DWORD mask, DWORD flags);
        DWORD SetExtendedStyle(DWORD exStyle);
        CImageList SetHotImageList(HIMAGELIST hotImages);
        int   SetHotItem(int index);
        CImageList SetImageList(HIMAGELIST normalImages);
        BOOL  SetIndent(int indent);
        BOOL  SetMaxTextRows(int maxRows);
        BOOL  SetPadding(int cx, int cy);
        void  SetToolTips(HWND toolTip);

    protected:
        // Overridables
        virtual void OnAttach() override;
        virtual LRESULT OnWindowPosChanging(UINT msg, WPARAM wparam, LPARAM lparam);
        virtual void PreCreate(CREATESTRUCT& cs) override;
        virtual void PreRegisterClass(WNDCLASS& wc) override;

        // Not intended to be overridden
        LRESULT WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam) override;

    private:
        using CWnd::GetMenu;                    // Make GetMenu private
        using CWnd::SetMenu;                    // Make SetMenu private

        CToolBar(const CToolBar&) = delete;
        CToolBar& operator=(const CToolBar&) = delete;

        std::map<CString, int> m_stringMap;     // A map of strings used in SetButtonText.

        CImageList m_normalImages;              // Image-list for normal buttons.
        CImageList m_hotImages;                 // Image-list for hot buttons.
        CImageList m_disabledImages;            // Image-list for disabled buttons.

    };  // class CToolBar

}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

namespace Win32xx
{

    //////////////////////////////////////
    // Definitions for the CToolBar class.
    //

    inline CToolBar::CToolBar()
    {
    }

    // Adds a single button to the Toolbar. It provides a convenient
    // alternative to AddButtons. A resource ID of 0 is a separator.
    // image is the index of the image in the ImageList.
    // The default is -1 in which case the image based on the button's position
    // is chosen.
    // Refer to TB_ADDBUTTONS in the Windows API documentation for more information.
    inline BOOL CToolBar::AddButton(UINT id, BOOL isEnabled /* = TRUE */,
        int image /* = -1 */)
    {
        assert(IsWindow());

        // Count toolbar buttons with Command IDs.
        int nImages = 0;

        if (image == -1)
        {
            // choose the image based on the number of buttons already used.
            for (int i = 0; i < GetButtonCount(); ++i)
            {
                if (GetCommandID(i) > 0)
                    nImages++;
            }
        }
        else
        {
            nImages = image;
        }

        // TBBUTTON structure for each button in the toolbar.
        TBBUTTON tbb = {};

        if (id == 0)
        {
            tbb.fsStyle = BTNS_SEP;
        }
        else
        {
            tbb.iBitmap = nImages;
            tbb.idCommand = static_cast<int>(id);
            tbb.fsState = isEnabled? TBSTATE_ENABLED : 0U;
            tbb.fsStyle = BTNS_BUTTON;
        }

        // Add the button to the toolbar.
        return AddButtons(1, &tbb);
    }

    // Adds one or more buttons to a ToolBar. pButtonInfoArray is a pointer to
    // an array of TBBUTTON.
    // Refer to TB_ADDBUTTONS in the Windows API documentation for more information.
    inline BOOL CToolBar::AddButtons(UINT buttonCount,
        LPTBBUTTON pButtonInfoArray)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonCount);
        LPARAM lparam = reinterpret_cast<LPARAM>(pButtonInfoArray);
        return SendMessage(TB_ADDBUTTONS, wparam, lparam) ? TRUE : FALSE;
    }

    // Adds a new string, passed as a resource ID, to the ToolBar's internal
    // list of strings.
    // Refer to TB_ADDSTRING in the Windows API documentation for more information.
    inline int CToolBar::AddString(UINT stringID)
    {
        assert(IsWindow());
        HANDLE resource = GetApp()->GetResourceHandle();
        WPARAM wparam = reinterpret_cast<WPARAM>(resource);
        LPARAM lparam = static_cast<LPARAM>(stringID);
        return static_cast<int>(SendMessage(TB_ADDSTRING, wparam, lparam));
    }

    // Adds a new string or strings to the list of strings available for a
    // ToolBar control. Strings in the buffer must be separated by a null
    // character. The last string must have two null terminators.
    // Refer to TB_ADDSTRING in the Windows API documentation for more information.
    inline int CToolBar::AddStrings(LPCTSTR strings)
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(strings);
        return static_cast<int>(SendMessage(TB_ADDSTRING, 0, lparam));
    }

    // Causes a ToolBar to be resized.
    // Refer to TB_AUTOSIZE in the Windows API documentation for more information.
    inline void CToolBar::Autosize()
    {
        assert(IsWindow());
        SendMessage(TB_AUTOSIZE, 0, 0);
    }

    // Checks or unchecks the specified button in a ToolBar.
    // When a button is checked, it is displayed in the pressed state.
    // Refer to TB_CHECKBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::CheckButton(UINT buttonID, BOOL isChecked)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_CHECKBUTTON, wparam, MAKELONG(isChecked, 0)) ? TRUE : FALSE;
    }

    // Retrieves the zero-based index for the button associated with the
    // specified command identifier.
    // Refer to TB_COMMANDTOINDEX in the Windows API documentation for more information.
    inline int CToolBar::CommandToIndex(UINT buttonID) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);

        // Returns -1 on fail.
        return static_cast<int>(SendMessage(TB_COMMANDTOINDEX, wparam, 0));
    }

    // Displays the Customize Toolbar dialog box. The parent must handle the
    // TBN_QUERYINSERT and TBN_QUERYDELETE notifications for the Customize
    // Toolbar dialog box to appear.
    // Requires the CCS_ADJUSTABLE style to be set when the toolbar is created.
    // Refer to TB_CUSTOMIZE in the Windows API documentation for more information.
    inline void CToolBar::Customize()
    {
        assert(IsWindow());
        SendMessage(TB_CUSTOMIZE, 0, 0);
    }

    // Deletes a button from the ToolBar.
    // index is the Zero-based index of the button to delete.
    // Refer to TB_DELETEBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::DeleteButton(int index)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return SendMessage(TB_DELETEBUTTON, wparam, 0) ? TRUE : FALSE;
    }

    // Destroys the toolbar.
    inline void CToolBar::Destroy()
    {
        CWnd::Destroy();
        m_stringMap.clear();
    }

    // Disables the specified button in a ToolBar.
    // An example of buttonID would be IDM_FILE_OPEN.
    // Refer to TB_ENABLEBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::DisableButton(UINT buttonID)
    {
        assert(IsWindow());
        return EnableButton(buttonID, FALSE);
    }

    // Enables the specified button in a ToolBar.
    // Refer to TB_ENABLEBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::EnableButton(UINT buttonID, BOOL isEnabled)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_ENABLEBUTTON, wparam, MAKELONG(isEnabled, 0)) ?
            TRUE : FALSE;
    }

    // Receives the TBBUTTON structure information from the specified button.
    // Refer to TB_GETBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::GetButton(int index, TBBUTTON& buttonInfo) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(&buttonInfo);
        return SendMessage(TB_GETBUTTON, wparam, lparam) ? TRUE : FALSE;
    }

    // Retrieves a count of the buttons currently in the ToolBar.
    // Refer to TB_BUTTONCOUNT in the Windows API documentation for more information.
    inline int CToolBar::GetButtonCount() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TB_BUTTONCOUNT, 0, 0));
    }

    // Retrieves the current width and height of ToolBar buttons, in pixels.
    // Returns a DWORD value that contains the width and height values in the
    // low word and high word, respectively.
    // Refer to TB_GETBUTTONSIZE in the Windows API documentation for more information.
    inline CSize CToolBar::GetButtonSize() const
    {
        assert(IsWindow());
        DWORD size = static_cast<DWORD>(SendMessage(TB_GETBUTTONSIZE, 0, 0));
        return CSize(size);
    }

    // Retrieves the state of an individual button
    //  TBSTATE_CHECKED     The button has the TBSTYLE_CHECK style and is being clicked.
    //  TBSTATE_ELLIPSES    The button's text is cut off and an ellipsis is displayed.
    //  TBSTATE_ENABLED     The button accepts user input. A button that doesn't have
    //                      this state is grayed.
    //  TBSTATE_HIDDEN      The button is not visible and cannot receive user input.
    //  TBSTATE_INDETERMINATE   The button is grayed.
    //  TBSTATE_MARKED      The button is marked. The interpretation of a marked
    //                      item is dependent upon the application.
    //  TBSTATE_PRESSED     The button is being clicked.
    //  TBSTATE_WRAP        The button is followed by a line break.
    // Refer to TB_GETSTATE in the Windows API documentation for more information.
    inline UINT CToolBar::GetButtonState(UINT buttonID) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return static_cast<UINT>(SendMessage(TB_GETSTATE, wparam, 0));
    }

    // Retrieves the the style of the ToolBar control. The following button
    // styles are supported:
    //  BTNS_BUTTON      Standard push button (default)
    //  BTNS_SEP         Separator
    //  BTNS_CHECK       Auto check-box button
    //  BTNS_GROUP       Marks the start of a group of buttons
    //  BTNS_CHECKGROUP  Marks the start of a group of check-box buttons
    //  BTNS_DROPDOWN    Creates a drop-down list button
    //  BTNS_AUTOSIZE    The button's width will be calculated based on the text
    //                   of the button, not on the size of the image
    //  BTNS_NOPREFIX    The button text will not have an accelerator prefix
    //                   associated with it
    // Refer to TB_GETBUTTON in the Windows API documentation for more information.
    inline BYTE CToolBar::GetButtonStyle(UINT buttonID) const
    {
        assert(IsWindow());

        int index = CommandToIndex(buttonID);
        TBBUTTON tbb = {};
        if (GetButton(index, tbb))
            return tbb.fsStyle;

        return 0;
    }

    // Retrieves the display text of a button on a ToolBar.
    // Refer to TB_GETBUTTONTEXT in the Windows API documentation for more information.
    inline CString CToolBar::GetButtonText(UINT buttonID) const
    {
        assert(IsWindow());

        WPARAM wparam = static_cast<WPARAM>(buttonID);
        int length = static_cast<int>(SendMessage(TB_GETBUTTONTEXT, wparam, 0));
        CString str;
        if (length > 0)
        {
            LPTSTR szStr = str.GetBuffer(length + 1);
            LPARAM lparam = reinterpret_cast<LPARAM>(szStr);
            SendMessage(TB_GETBUTTONTEXT, wparam, lparam);
            str.ReleaseBuffer();
        }

        return str;
    }

    // Retrieves information about the specified button in a ToolBar.
    // Refer to TB_GETBUTTON in the Windows API documentation for more information.
    inline UINT CToolBar::GetCommandID(int index) const
    {
        assert(IsWindow());
        TBBUTTON tbb = {};
        GetButton(index, tbb);

        // returns zero if failed
        return static_cast<UINT>(tbb.idCommand);
    }

    // Retrieves the image list that a ToolBar control uses to display inactive
    // buttons.
    // Refer to TB_GETDISABLEDIMAGELIST in the Windows API documentation for more information.
    inline CImageList CToolBar::GetDisabledImageList() const
    {
        assert(IsWindow());
        HIMAGELIST images = reinterpret_cast<HIMAGELIST>(SendMessage(
            TB_GETDISABLEDIMAGELIST, 0, 0));

        return CImageList(images);
    }

    // Retrieves the he extended styles currently in use for the toolbar control.
    // Possible extended styles: TBSTYLE_EX_DRAWDDARROWS,
    // TBSTYLE_EX_HIDECLIPPEDBUTTONS, TBSTYLE_EX_DOUBLEBUFFER and
    // TBSTYLE_EX_MIXEDBUTTONS.
    // Refer to TB_GETEXTENDEDSTYLE in the Windows API documentation for more information.
    inline DWORD CToolBar::GetExtendedStyle() const
    {
        assert(IsWindow());
        return static_cast<DWORD>(SendMessage(TB_GETEXTENDEDSTYLE, 0, 0));
    }

    // Retrieves the image list that a ToolBar control uses to display hot
    // buttons.
    // Refer to TB_GETHOTIMAGELIST in the Windows API documentation for more information.
    inline CImageList CToolBar::GetHotImageList() const
    {
        assert(IsWindow());
        HIMAGELIST images = reinterpret_cast<HIMAGELIST>(SendMessage(
            TB_GETHOTIMAGELIST, 0, 0));

        return CImageList(images);
    }

    // Retrieves the index of the hot item in a ToolBar, or -1 if no hot item
    // is set.
    // Refer to TB_GETHOTITEM in the Windows API documentation for more information.
    inline int CToolBar::GetHotItem() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TB_GETHOTITEM, 0, 0));
    }

    // Retrieves the image list that a ToolBar control uses to display buttons
    // in their default state.
    // Refer to TB_GETIMAGELIST in the Windows API documentation for more information.
    inline CImageList CToolBar::GetImageList() const
    {
        assert(IsWindow());
        HIMAGELIST images = reinterpret_cast<HIMAGELIST>(SendMessage(
            TB_GETIMAGELIST, 0, 0));

        return CImageList(images);
    }

    // Retrieves the bounding rectangle of a button in a ToolBar.
    // Refer to TB_GETITEMRECT in the Windows API documentation for more information.
    inline CRect CToolBar::GetItemRect(int index) const
    {
        assert(IsWindow());
        CRect rc;
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        SendMessage(TB_GETITEMRECT, wparam, lparam);

        return rc;
    }

    // Retrieves the total size of all of the visible buttons and separators in
    // the ToolBar.
    // Refer to TB_GETMAXSIZE in the Windows API documentation for more information.
    inline CSize CToolBar::GetMaxSize() const
    {
        assert(IsWindow());
        CSize sz;
        if (SendMessage(TB_GETMAXSIZE, 0, reinterpret_cast<LPARAM>(&sz)))
        {
            int maxButtonWidth = 0;
            int maxButtonHeight = 0;
            int totalWidth = 0;
            int totalHeight = 0;

            for (int i = 0; i < GetButtonCount(); ++i)
            {
                CRect itemRect = GetItemRect(i);
                totalWidth += itemRect.Width();
                totalHeight += itemRect.Height();

                if (itemRect.Width() > maxButtonWidth) maxButtonWidth = itemRect.Width();
                if (itemRect.Height() > maxButtonHeight) maxButtonHeight = itemRect.Height();
            }

            if (GetStyle() & CCS_VERT)
            {
                sz.cx = maxButtonWidth;
                sz.cy = totalHeight;
            }
            else
            {
                sz.cx = totalWidth;
                sz.cy = maxButtonHeight;
            }
        }

        return sz;
    }

    // Returns a DWORD value that contains the horizontal padding in the low
    // word and the vertical padding in the high word, in pixels.
    // Refer to TB_GETPADDING in the Windows API documentation for more information.
    inline CSize CToolBar::GetPadding() const
    {
        assert(IsWindow());
        DWORD padding = static_cast<DWORD>(SendMessage(TB_GETPADDING, 0, 0));
        return CSize(padding);
    }

    // Retrieves the bounding rectangle for a specified ToolBar button.
    // Refer to TB_GETRECT in the Windows API documentation for more information.
    inline CRect CToolBar::GetRect(UINT buttonID) const
    {
        assert(IsWindow());
        CRect rc;
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        LPARAM lparam = reinterpret_cast<LPARAM>(&rc);
        SendMessage(TB_GETRECT, wparam, lparam);
        return rc;
    }

    // Retrieves the number of rows of buttons in a ToolBar with the
    // TBSTYLE_WRAPABLE style.
    // Refer to TB_GETROWS in the Windows API documentation for more information.
    inline int CToolBar::GetRows() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TB_GETROWS, 0, 0));
    }

    // Retrieves the maximum number of text rows that can be displayed on a
    // ToolBar button.
    // Refer to TB_GETTEXTROWS in the Windows API documentation for more information.
    inline int CToolBar::GetTextRows() const
    {
        assert(IsWindow());
        return static_cast<int>(SendMessage(TB_GETTEXTROWS, 0, 0));
    }

    // Retrieves the handle to the ToolTip control, if any, associated with
    // the ToolBar.
    // Refer to TB_GETTOOLTIPS in the Windows API documentation for more information.
    inline HWND CToolBar::GetToolTips() const
    {
        assert(IsWindow());
        return reinterpret_cast<HWND>(SendMessage(TB_GETTOOLTIPS, 0, 0));
    }

    // Returns TRUE if any button has text.
    // Refer to TB_GETBUTTONTEXT in the Windows API documentation for more information.
    inline BOOL CToolBar::HasText() const
    {
        assert(IsWindow());

        for (int i = 0; i < GetButtonCount(); ++i)
        {
            UINT id = GetCommandID(i);
            if (id != 0) // Skip separators
            {
                WPARAM wparam = static_cast<WPARAM>(id);
                if (static_cast<int>(SendMessage(TB_GETBUTTONTEXT, wparam, 0)) > 0)
                    return TRUE;
            }
        }
        return FALSE;
    }

    // Hides or shows the specified button in a ToolBar.
    // Refer to TB_HIDEBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::HideButton(UINT buttonID, BOOL hide)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_HIDEBUTTON, wparam, MAKELONG (hide, 0)) ? TRUE : FALSE;
    }

    // Returns the button that's positioned under the cursor.
    // Refer to TB_HITTEST in the Windows API documentation for more information.
    inline int CToolBar::HitTest() const
    {
        assert(IsWindow());
        CPoint pos = GetCursorPos();
        VERIFY(ScreenToClient(pos));

        return HitTest(pos);
    }

    // Returns the button that's positioned at the specified point.
    // Refer to TB_HITTEST in the Windows API documentation for more information.
    inline int CToolBar::HitTest(POINT pt) const
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(&pt);
        return static_cast<int>(SendMessage(TB_HITTEST, 0, lparam));
    }

    // Sets or clears the indeterminate state of the specified button in a
    // toolbar.
    // Refer to TB_INDETERMINATE in the Windows API documentation for more information.
    inline BOOL CToolBar::Indeterminate(UINT buttonID, BOOL isIndeterminate)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_INDETERMINATE, wparam, MAKELONG(
            isIndeterminate, 0)) ? TRUE : FALSE;
    }

    // Inserts a button to the left of the specified button index.
    // Refer to TB_INSERTBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::InsertButton(int index, const TBBUTTON& buttonInfo)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = reinterpret_cast<LPARAM>(&buttonInfo);
        return SendMessage(TB_INSERTBUTTON, wparam, lparam) ? TRUE : FALSE;
    }

    // Determines whether the specified button in a ToolBar is hidden.
    // Refer to TB_ISBUTTONHIDDEN in the Windows API documentation for more information.
    inline BOOL CToolBar::IsButtonHidden(UINT buttonID) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_ISBUTTONHIDDEN, wparam, 0) ? TRUE : FALSE;
    }

    // Checks the highlight state of a ToolBar button.
    // Refer to TB_ISBUTTONHIGHLIGHTED in the Windows API documentation for more information.
    inline BOOL CToolBar::IsButtonHighlighted(UINT buttonID) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_ISBUTTONHIGHLIGHTED, wparam, 0) ? TRUE : FALSE;
    }

    // Determines whether the specified button in a ToolBar is indeterminate.
    // Refer to TB_ISBUTTONINDETERMINATE in the Windows API documentation for more information.
    inline BOOL CToolBar::IsButtonIndeterminate(UINT buttonID) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_ISBUTTONINDETERMINATE, wparam, 0) ? TRUE : FALSE;
    }

    // Determines whether the specified button in a ToolBar is pressed.
    // Refer to TB_ISBUTTONPRESSED in the Windows API documentation for more information.
    inline BOOL CToolBar::IsButtonPressed(UINT buttonID) const
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_ISBUTTONPRESSED, wparam, 0) ? TRUE : FALSE;
    }

    // Determines the ID of the button that corresponds to the specified
    // accelerator character.
    // Refer to TB_MAPACCELERATOR in the Windows API documentation for more information.
    inline int CToolBar::MapAccelerator(TCHAR accelChar)
    {
        assert(IsWindow());
        int idButton = -1;
        WPARAM wparam = static_cast<WPARAM>(accelChar);
        LPARAM lparam = reinterpret_cast<LPARAM>(&idButton);
        if (!SendMessage(TB_MAPACCELERATOR, wparam, lparam))
            idButton = -1;

        return idButton;
    }

    // Sets the highlight state of a given button in a ToolBar control.
    // Refer to TB_MARKBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::MarkButton(UINT buttonID, BOOL highlight /*= TRUE*/ )
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        LPARAM lparam = static_cast<LPARAM>(highlight);
        return SendMessage(TB_MARKBUTTON, wparam, lparam) ? TRUE : FALSE;
    }

    // Moves a button from one index to another.
    // Refer to TB_MOVEBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::MoveButton(UINT oldPos, UINT newPos)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(oldPos);
        LPARAM lparam = static_cast<LPARAM>(newPos);
        return SendMessage(TB_MOVEBUTTON, wparam, lparam) ? TRUE : FALSE;
    }

    // Called when the toolbar window is created and attached to this object.
    inline void CToolBar::OnAttach()
    {
        WPARAM wparam = static_cast<WPARAM>(sizeof(TBBUTTON));

        // We must send this message before sending the TB_ADDBITMAP or
        // TB_ADDBUTTONS message.
        SendMessage(TB_BUTTONSTRUCTSIZE, wparam, 0);

        // Allows buttons to have a separate drop-down arrow.
        // Note: TBN_DROPDOWN notification is sent by a ToolBar control,
        // when the user clicks a drop-down button.
        SendMessage(TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
    }

    // Called when the toolbar is resized.
    inline LRESULT CToolBar::OnWindowPosChanging(UINT msg, WPARAM wparam,
        LPARAM lparam)
    {
        //  Used by ReBar controls to adjust ToolBar window size
        if ( GetParent().SendMessage(UWM_TBWINPOSCHANGING, reinterpret_cast<
            WPARAM>(GetHwnd()), lparam) )
        {
            LPWINDOWPOS pWinPos = reinterpret_cast<LPWINDOWPOS>(lparam);
            pWinPos->cx = GetMaxSize().cx;
            pWinPos->cy = GetMaxSize().cy;
        }

        return FinalWindowProc(msg, wparam, lparam);
    }

    // Sets the CREATESTRUCT parameters prior to window creation.
    inline void CToolBar::PreCreate(CREATESTRUCT& cs)
    {
        cs.style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
            TBSTYLE_TOOLTIPS | TBSTYLE_FLAT;
    }

    // Sets the window class.
    inline void CToolBar::PreRegisterClass(WNDCLASS& wc)
    {
        wc.lpszClassName =  TOOLBARCLASSNAME;
    }

    // Presses or releases the specified button in a ToolBar.
    // Refer to TB_PRESSBUTTON in the Windows API documentation for more information.
    inline BOOL CToolBar::PressButton(UINT buttonID, BOOL press)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_PRESSBUTTON, wparam, MAKELONG(press, 0)) ? TRUE : FALSE;
    }

    // Saves or restores the toolbar state in the registry. Parameter values:
    //  save       If this parameter is TRUE, the information is saved,
    //             otherwise it is restored.
    //  pSaveInfo  Pointer to a TBSAVEPARAMS structure that specifies the
    //             registry key, subkey, and value name for the toolbar state
    //             information.
    // Refer to TB_SAVERESTORE in the Windows API documentation for more information.
    inline void CToolBar::SaveRestore(BOOL save, TBSAVEPARAMS* pSaveInfo)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(save);
        LPARAM lparam = reinterpret_cast<LPARAM>(pSaveInfo);
        SendMessage(TB_SAVERESTORE, wparam, lparam);
    }

    // Sets the size of the bitmapped images to be added to a ToolBar.
    // Needs to be used when the image size is not the default 16 x 15.
    // Call this function before using AddBitmap or ReplaceBitmap.
    // Refer to TB_SETBITMAPSIZE in the Windows API documentation for more information.
    inline BOOL CToolBar::SetBitmapSize(int cx, int cy)
    {
        assert(IsWindow());
        return SendMessage(TB_SETBITMAPSIZE, 0, MAKELONG(cx, cy)) ? TRUE : FALSE;
    }

    // Sets the size of the buttons to be added to a ToolBar.
    // This function should generally be called after adding buttons.
    // Refer to TB_SETBUTTONSIZE in the Windows API documentation for more information.
    inline BOOL CToolBar::SetButtonSize(int cx, int cy)
    {
        assert(IsWindow());
        return SendMessage(TB_SETBUTTONSIZE, 0, MAKELONG(cx, cy)) ? TRUE : FALSE;
    }

    // Change a button's command ID, image, style and state.
    // Refer to TB_SETBUTTONINFO in the Windows API documentation for more information.
    inline void CToolBar::SetButtonInfo(UINT buttonID, UINT buttonNewID,
        int image, BYTE style /* = 0xFF */, BYTE state /* = 0xFF */)
    {
        // Retrieve existing state and style
        TBBUTTON tb = {};
        int index = CommandToIndex(buttonID);
        BOOL result = GetButton(index, tb);
        assert(result);

        if (result)
        {
            TBBUTTONINFO tbbi = {};
            tbbi.cbSize = sizeof(tbbi);
            tbbi.dwMask = TBIF_COMMAND | TBIF_IMAGE | TBIF_STYLE | TBIF_STATE;
            tbbi.idCommand = static_cast<int>(buttonNewID);
            tbbi.iImage = image;
            tbbi.fsStyle = (style == 0xFF) ? tb.fsStyle : style;
            tbbi.fsState = (state == 0xFF) ? tb.fsState : state;

            SetButtonInfo(buttonID, tbbi);
        }
    }

    // Change a button's command ID, image, style and state, from the specified
    // TBBUTTONINFO structure.
    // Refer to TB_SETBUTTONINFO in the Windows API documentation for more information.
    inline BOOL CToolBar::SetButtonInfo(UINT buttonID, const TBBUTTONINFO& tbbi)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        LPARAM lparam = reinterpret_cast<LPARAM>(&tbbi);
        return SendMessage(TB_SETBUTTONINFO, wparam, lparam) ? TRUE : FALSE;
    }

    // Set the state of an individual button
    //  TBSTATE_CHECKED     The button has the TBSTYLE_CHECK style and is being clicked.
    //  TBSTATE_ELLIPSES    The button's text is cut off and an ellipsis is displayed.
    //  TBSTATE_ENABLED     The button accepts user input. A button that
    //                      doesn't have this state is grayed.
    //  TBSTATE_HIDDEN      The button is not visible and cannot receive user input.
    //  TBSTATE_INDETERMINATE   The button is grayed.
    //  TBSTATE_MARKED      The button is marked. The interpretation of a
    //                      marked item is dependent upon the application.
    //  TBSTATE_PRESSED     The button is being clicked.
    //  TBSTATE_WRAP        The button is followed by a line break.
    // Refer to TB_SETSTATE in the Windows API documentation for more information.
    inline BOOL CToolBar::SetButtonState(UINT buttonID, UINT state)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(buttonID);
        return SendMessage(TB_SETSTATE, wparam, MAKELONG (state, 0)) ? TRUE : FALSE;
    }

    //  The the style of the ToolBar control. The following button styles are supported:
    //  BTNS_BUTTON      Standard push button (default)
    //  BTNS_SEP         Separator
    //  BTNS_CHECK       Auto check-box button
    //  BTNS_GROUP       Marks the start of a group of buttons
    //  BTNS_CHECKGROUP  Marks the start of a group of check-box buttons
    //  BTNS_DROPDOWN    Creates a drop-down list button
    //  BTNS_AUTOSIZE    The button's width will be calculated based on the
    //                      text of the button, not on the size of the image
    //  BTNS_NOPREFIX    The button text will not have an accelerator prefix associated with it.
    // Refer to TB_SETBUTTONINFO in the Windows API documentation for more information.
    inline BOOL CToolBar::SetButtonStyle(UINT buttonID, BYTE style)
    {
        assert(IsWindow());

        TBBUTTONINFO tbbi = {};
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_STYLE;
        tbbi.fsStyle = style;

        return SetButtonInfo(buttonID, tbbi);
    }

    // Sets the button's text using AddString. The AddString approach always sets
    // the button size correctly, whereas the SetButtonInfo alternative doesn't.
    // Refer to TB_ADDSTRING in the Windows API documentation for more information.
    inline BOOL CToolBar::SetButtonText(UINT buttonID, LPCTSTR text)
    {
        assert(IsWindow());
        int index = CommandToIndex(buttonID);
        assert(index != -1);

        BOOL succeeded = FALSE;
        CString string = text;
        int stringIndex;

        // Check to see if the string is already added.
        auto m = m_stringMap.find(string);
        if (m_stringMap.end() == m)
        {
            CString str;
            if (m_stringMap.size() == 0)
            {
                // Place a blank string first in the string table, in case some
                // buttons don't have text.

                str = _T(" ");
                str += _T('\0');    // Double-null terminate.
                AddStrings(str);
            }

            // No index for this string exists, so create it now.
            str = text;
            str += _T('\0');        // Double-null terminate.

            stringIndex = AddStrings(str);
            if (stringIndex != -1)
            {
                // Save the string its index in our map.
                m_stringMap.emplace(string, stringIndex);

                succeeded = TRUE;
            }
        }
        else
        {
            // String found, use the index from our map.
            stringIndex = m->second;
            succeeded = TRUE;
        }

        if (succeeded)
        {
            TBBUTTON tbb = {};
            succeeded = GetButton(index, tbb);
            tbb.iString = stringIndex;

            // Turn off ToolBar drawing.
            SetRedraw(FALSE);

            // Replace the button to resize it to fit the text.
            succeeded = succeeded ? DeleteButton(index) : FALSE;
            succeeded = succeeded ? InsertButton(index, tbb) : FALSE;

            // Ensure the button now includes some text rows.
            if (SendMessage(TB_GETTEXTROWS, 0, 0) == 0)
                SendMessage(TB_SETMAXTEXTROWS, static_cast<WPARAM>(1), 0);

            // Turn on ToolBar drawing.
            SetRedraw(TRUE);
        }

        Invalidate();
        UpdateWindow();

        return succeeded;
    }

    // Sets the button width.
    // Adjust the width of a toolbar button after it is created.
    // This is useful when replacing a button with a ComboBox or other control.
    // Refer to TB_SETBUTTONINFO in the Windows API documentation for more information.
    inline BOOL CToolBar::SetButtonWidth(UINT buttonID, int width)
    {
        assert(IsWindow());

        TBBUTTONINFO tbbi = {};
        tbbi.cbSize = sizeof(tbbi);
        tbbi.dwMask = TBIF_SIZE;
        tbbi.cx = static_cast<WORD>(width);
        BOOL result = SetButtonInfo(buttonID, tbbi);

        // Send a changed message to the parent (used by the ReBar)
        SIZE maxSize = GetMaxSize();
        WPARAM wparam = reinterpret_cast<WPARAM>(GetHwnd());
        LPARAM lparam = reinterpret_cast<LPARAM>(&maxSize);
        GetParent().SendMessage(UWM_TBRESIZE, wparam, lparam);

        return result;
    }

    // Sets the command identifier of a ToolBar button.
    // Refer to TB_SETCMDID in the Windows API documentation for more information.
    inline BOOL CToolBar::SetCommandID(int index, UINT buttonID)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        LPARAM lparam = static_cast<LPARAM>(buttonID);
        return SendMessage(TB_SETCMDID, wparam, lparam) ? TRUE : FALSE;
    }

    // Sets the ImageList that the ToolBar control will use to display disabled
    // buttons.
    // Refer to TB_SETDISABLEDIMAGELIST in the Windows API documentation for more information.
    inline CImageList CToolBar::SetDisableImageList(HIMAGELIST disabledImages)
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(disabledImages);
        CImageList images = reinterpret_cast<HIMAGELIST>(SendMessage(
            TB_SETDISABLEDIMAGELIST, 0, lparam));

        m_disabledImages = disabledImages;
        return images;
    }

    // Sets the text drawing flags for the ToolBar.
    // Refer to TB_SETDRAWTEXTFLAGS in the Windows API documentation for more information.
    inline DWORD CToolBar::SetDrawTextFlags(DWORD mask, DWORD flags)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(mask);
        LPARAM lparam = static_cast<LPARAM>(flags);
        return static_cast<DWORD>(SendMessage(TB_SETDRAWTEXTFLAGS, wparam, lparam));
    }

    // Sets the text drawing flags for the ToolBar.
    // Extended styles include: TBSTYLE_EX_DRAWDDARROWS,
    // TBSTYLE_EX_HIDECLIPPEDBUTTONS, TBSTYLE_EX_DOUBLEBUFFER and
    // TBSTYLE_EX_MIXEDBUTTONS.
    // Refer to TB_SETEXTENDEDSTYLE in the Windows API documentation for more information.
    inline DWORD CToolBar::SetExtendedStyle(DWORD exStyle)
    {
        assert(IsWindow());
        LPARAM lparam = static_cast<LPARAM>(exStyle);
        return static_cast<DWORD>(SendMessage(TB_SETEXTENDEDSTYLE, 0, lparam));
    }

    // Sets the image list that the ToolBar control will use to display hot buttons.
    // Refer to TB_SETHOTIMAGELIST in the Windows API documentation for more information.
    inline CImageList CToolBar::SetHotImageList(HIMAGELIST hotImages)
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(hotImages);
        CImageList images = reinterpret_cast<HIMAGELIST>(SendMessage(TB_SETHOTIMAGELIST, 0, lparam));
        m_hotImages = hotImages;
        return images;
    }

    // Sets the hot item in a ToolBar.
    // Refer to TB_SETHOTITEM in the Windows API documentation for more information.
    inline int CToolBar::SetHotItem(int index)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(index);
        return static_cast<int>(SendMessage(TB_SETHOTITEM, wparam, 0));
    }

    // Sets the image list that the ToolBar will use to display buttons that
    // are in their default state.
    // Refer to TB_SETIMAGELIST in the Windows API documentation for more information.
    inline CImageList CToolBar::SetImageList(HIMAGELIST normalImages)
    {
        assert(IsWindow());
        LPARAM lparam = reinterpret_cast<LPARAM>(normalImages);
        CImageList images = reinterpret_cast<HIMAGELIST>(SendMessage(
            TB_SETIMAGELIST, 0, lparam));

        m_normalImages = normalImages;
        return images;
    }

    // Sets the indentation for the first button in a ToolBar control.
    // Refer to TB_SETINDENT in the Windows API documentation for more information.
    inline BOOL CToolBar::SetIndent(int indent)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(indent);
        return SendMessage(TB_SETINDENT, wparam, 0) ? TRUE : FALSE;
    }

    // Sets the maximum number of text rows displayed on a ToolBar button.
    // Refer to TB_SETMAXTEXTROWS in the Windows API documentation for more information.
    inline BOOL CToolBar::SetMaxTextRows(int maxRows)
    {
        assert(IsWindow());
        WPARAM wparam = static_cast<WPARAM>(maxRows);
        return SendMessage(TB_SETMAXTEXTROWS, wparam, 0) ? TRUE : FALSE;
    }

    // Sets the padding for a ToolBar control.
    // Refer to TB_SETPADDING in the Windows API documentation for more information.
    inline BOOL CToolBar::SetPadding(int cx, int cy)
    {
        assert(IsWindow());
        return SendMessage(TB_SETPADDING, 0, MAKELONG(cx, cy)) ? TRUE : FALSE;
    }

    // Associates a ToolTip control with a ToolBar. This should be done before
    // adding any buttons to the ToolBar.
    // Refer to TB_SETTOOLTIPS in the Windows API documentation for more information.
    inline void CToolBar::SetToolTips(HWND toolTip)
    {
        assert(IsWindow());
        WPARAM wparam = reinterpret_cast<WPARAM>(toolTip);
        SendMessage(TB_SETTOOLTIPS, wparam, 0);
    }

    // Provides default processing of the toolbar's messages.
    inline LRESULT CToolBar::WndProcDefault(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        switch (msg)
        {
        case WM_WINDOWPOSCHANGING:  return OnWindowPosChanging(msg, wparam, lparam);
        case UWM_GETCTOOLBAR:       return reinterpret_cast<LRESULT>(this);

        // Do default processing for other messages.
        default: return CWnd::WndProcDefault(msg, wparam, lparam);
        }
    }

} // namespace Win32xx

#endif // WIN32XX_TOOLBAR_H_
