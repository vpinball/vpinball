// license:GPLv3+

#pragma once

#include <dlgs.h>
#include <cderr.h>
#include <wxx_appcore.h>		// Add CCriticalSection, CObject, CWinThread, CWinApp
#include <wxx_commondlg.h>		// Add CCommonDialog, CColorDialog, CFileDialog, CFindReplace, CFontDialog 
#include "ui/properties/PropertyDialog.h"
#include <WindowsX.h>

class MaterialDialog : public CDialog
{
public:
   MaterialDialog();

protected:
   virtual BOOL OnInitDialog();
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   virtual INT_PTR DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
   virtual void OnOK();
   virtual void OnClose();

private:
   void EnableAllMaterialDialogItems(const BOOL e);
   void SetEditedMaterial(const Material& mat);
   void SaveEditedMaterial(Material& mat);
   float getItemText(int id);
   void  setItemText(int id, float value);
   void LoadPosition();
   void SavePosition();
   void ShowWhereUsed();
   HWND m_hMaterialList;
   CResizer m_resizer;
   static int m_columnSortOrder;
   static bool m_deletingItem;
   CColorDialog m_colorDialog;
   ColorButton m_colorButton1;
   ColorButton m_colorButton2;
   ColorButton m_colorButton3;
   ColorButton m_colorButton4;
};
