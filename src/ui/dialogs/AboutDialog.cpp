// license:GPLv3+

#include "core/stdafx.h"
#include "AboutDialog.h"
#include "core/vpversion.h"
#include <fstream>

AboutDialog::AboutDialog() : CDialog(IDD_ABOUT)
{
}

AboutDialog::~AboutDialog()
{
}

void AboutDialog::OnDestroy()
{
}

INT_PTR AboutDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
      case WM_INITDIALOG:
      {
         GetDlgItem(IDC_ABOUT_VERSION).SetWindowText(VP_VERSION_STRING_FULL_LITERAL);

         {
            std::ifstream file(g_pvp->m_szMyPath + "Changelog.txt");
            if (!file.is_open())
               file = std::ifstream(g_pvp->m_szMyPath + "Doc"  + PATH_SEPARATOR_CHAR + "Changelog.txt");
            if (!file.is_open())
               file = std::ifstream(g_pvp->m_szMyPath + "docs" + PATH_SEPARATOR_CHAR + "Changelog.txt");
            string line, text;
            while (std::getline(file, line))
            {
               line += "\r\n";
               text += line;
            }
            SetDlgItemText(IDC_CHANGELOG, text.c_str());

            HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
               CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));

            ::SendMessage(GetDlgItem(IDC_CHANGELOG),
               WM_SETFONT,
               (WPARAM)hFont,
               MAKELPARAM(TRUE, 0) // Redraw text
               );
         }

#if !(defined(IMSPANISH) | defined(IMGERMAN) | defined(IMFRENCH))
         ::ShowWindow(GetDlgItem(IDC_TRANSNAME), SW_HIDE);
#endif

#if !(defined(IMSPANISH))
         ::ShowWindow(GetDlgItem(IDC_TRANSLATEWEBSITE), SW_HIDE);
#endif
      }
      return TRUE;
   }

   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL AboutDialog::OnCommand(WPARAM wParam, LPARAM lParam)
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

BOOL AboutDialog::OnInitDialog()
{
   return TRUE;
}

void AboutDialog::OnOK()
{
   CDialog::OnOK();
}
