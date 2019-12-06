#include "stdafx.h"
#include "AboutDialog.h"
#include "vpversion.h"
#include "svn_version.h"
#include <fstream>
#include <sstream>

AboutDialog::AboutDialog() : CDialog(IDD_ABOUT)
{
   m_urlString[0] = '\0';
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
         const HWND hwndDlg = GetHwnd();
         char versionString[256];
         sprintf_s(versionString, "Version %i.%i.%i (Revision %i, %ubit)", VP_VERSION_MAJOR,VP_VERSION_MINOR,VP_VERSION_REV, SVN_REVISION,
#ifdef _WIN64
            64u
#else
            32u
#endif
            );
         GetDlgItem(IDC_ABOUT_VERSION).SetWindowText(versionString);

         {
            std::ifstream file(string(g_pvp->m_szMyPath) + "Changelog.txt");
            if (!file.is_open())
               file = std::ifstream(string(g_pvp->m_szMyPath) + "Doc\\Changelog.txt");
            std::string line;
            std::string text;
            while (std::getline(file, line))
            {
               line += "\r\n";
               text += line;
            }
            SetDlgItemText(IDC_CHANGELOG, text.c_str());

            HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
               CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));

            ::SendMessage(::GetDlgItem(hwndDlg, IDC_CHANGELOG),
               WM_SETFONT,
               (WPARAM)hFont,
               MAKELPARAM(TRUE, 0) // Redraw text
               );
         }

#if !(defined(IMSPANISH) | defined(IMGERMAN) | defined(IMFRENCH))
         ::ShowWindow(::GetDlgItem(hwndDlg, IDC_TRANSNAME), SW_HIDE);
#endif

#if !(defined(IMSPANISH))
         ::ShowWindow(::GetDlgItem(hwndDlg, IDC_TRANSLATEWEBSITE), SW_HIDE);
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
            /*const HRESULT hr =*/ OpenURL("http://www.vpforums.org");
         else
         {
            const LPCTSTR szSite = GetDlgItem(IDC_TRANSWEBSITE).GetWindowText();
            strncpy_s(m_urlString, szSite, MAX_PATH-1);
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
