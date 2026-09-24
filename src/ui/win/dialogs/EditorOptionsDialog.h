// license:GPLv3+

#pragma once

#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 

#include "ui/win/properties/PropertyDialog.h"

class WinEditor;

class EditorOptionsDialog final : public CDialog
{
public:
   explicit EditorOptionsDialog(WinEditor *vpxEditor);

protected:
   BOOL OnInitDialog() override;
   BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;
   INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
   void OnOK() override;

private:
   WinEditor *const m_vpxEditor;

   void AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd);

   int m_defaultMaterialColor;

   CToolTip *m_toolTip;
   CColorDialog m_colorDialog;
   ColorButton m_colorButton2;
   ColorButton m_colorButton3;
   ColorButton m_colorButton4;
   ColorButton m_colorButton5;
   ColorButton m_colorButton6;
};
