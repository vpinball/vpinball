#include "core/stdafx.h"
#include "InputDeviceDialog.h"

InputDeviceDialog::InputDeviceDialog(CRect* sourcePos) : CDialog(IDD_INPUT_DEVICES)
{
   startPos = new CRect(*sourcePos);
}

InputDeviceDialog::~InputDeviceDialog()
{
}

void InputDeviceDialog::OnDestroy()
{
}

INT_PTR InputDeviceDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL InputDeviceDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   switch (LOWORD(wParam))
   {
      case IDC_WEBSITE:
      case IDC_TRANSSITE:
      {
         if (LOWORD(wParam) == IDC_WEBSITE)
            /*const HRESULT hr =*/ OpenURL("http://www.vpforums.org"s);
         else
         {
            m_urlString = GetDlgItem(IDC_TRANSWEBSITE).GetWindowText().c_str();
            /*const HRESULT hr =*/ OpenURL(m_urlString);
         }
         return TRUE;
      }
   }

   return FALSE;
}

BOOL InputDeviceDialog::OnInitDialog()
{
   CRect myPos = GetWindowRect();
   startPos->bottom = startPos->top + (myPos.bottom - myPos.top);
   startPos->right = startPos->left + (myPos.right - myPos.left);
   MoveWindow(*startPos);
   return TRUE;
}

void InputDeviceDialog::OnOK()
{
   CDialog::OnOK();
}
