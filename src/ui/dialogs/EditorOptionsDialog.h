// license:GPLv3+

#pragma once

#include <dlgs.h>
#include <cderr.h>
#include <wxx_appcore.h>		// Add CCriticalSection, CObject, CWinThread, CWinApp
#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 
#include "ui/properties/PropertyDialog.h"
#include <WindowsX.h>

class EditorOptionsDialog : public CDialog
{
public:
    EditorOptionsDialog();

protected:
    virtual void OnClose();
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    virtual void OnOK();

private:
    void AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);

    CToolTip *m_toolTip;
    CColorDialog m_colorDialog;
    ColorButton m_colorButton2;
    ColorButton m_colorButton3;
    ColorButton m_colorButton4;
    ColorButton m_colorButton5;
    ColorButton m_colorButton6;
};
