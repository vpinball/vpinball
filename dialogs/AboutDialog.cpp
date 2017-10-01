#include "stdafx.h"
#include "AboutDialog.h"
#include "vpversion.h"
#include "svn_version.h"
#include <fstream>
#include <sstream>

AboutDialog::AboutDialog() : CDialog(IDD_ABOUT)
{
   memset(urlString, 0, MAX_PATH);
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
         HWND hwndDlg = GetHwnd();
         char versionString[256];
         sprintf_s(versionString, "Version %i.%i.%i (Revision %i, %ubit)", VP_VERSION_MAJOR,VP_VERSION_MINOR,VP_VERSION_REV, SVN_REVISION,
#ifdef _WIN64
            64
#else
            32
#endif
            );
         GetDlgItem(IDC_ABOUT_VERSION).SetWindowText(versionString);

         {
            const string sPath = string(g_pvp->m_szMyPath) + "Changelog.txt";
            std::ifstream file(sPath);
            std::string line;
            std::string text;
            while (std::getline(file, line))
            {
               line += "\r\n";
               text += line;
            }
            SetDlgItemText(IDC_CHANGELOG, text.c_str());

            HWND hChangelog = ::GetDlgItem(hwndDlg, IDC_CHANGELOG);

            HFONT hFont = CreateFont(14, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
               CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, TEXT("Courier New"));

            ::SendMessage(hChangelog,
               WM_SETFONT,
               (WPARAM)hFont,
               MAKELPARAM(TRUE, 0) // Redraw text
               );
         }

#if !(defined(IMSPANISH) | defined(IMGERMAN) | defined(IMFRENCH))
         HWND hwndTransName = ::GetDlgItem(hwndDlg, IDC_TRANSNAME);
         ::ShowWindow(hwndTransName, SW_HIDE);
#endif

#if !(defined(IMSPANISH))
         HWND hwndTransSite = ::GetDlgItem(hwndDlg, IDC_TRANSLATEWEBSITE);
         ::ShowWindow(hwndTransSite, SW_HIDE);
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
         HRESULT hr;
         if (LOWORD(wParam) == IDC_WEBSITE)
            hr = OpenURL("http://www.vpforums.org");
         else
         {
            HWND hwndTransURL = GetDlgItem(IDC_TRANSWEBSITE);
            LPCTSTR szSite;
            szSite = GetDlgItem(IDC_TRANSWEBSITE).GetWindowText();
            strncpy_s(urlString, szSite, MAX_PATH);
            hr = OpenURL((char*)urlString);
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
