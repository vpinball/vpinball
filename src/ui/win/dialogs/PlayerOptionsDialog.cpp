// license:GPLv3+

#include "core/stdafx.h"
#include "PlayerOptionsDialog.h"

#include "core/VPApp.h"
#include "ui/win/resource.h"


PlayerOptionsDialog::PlayerOptionsDialog()
   : CDialog(IDD_PLAYER_OPTS)
{
}

void PlayerOptionsDialog::AddToolTip(const CWnd& wnd, const char* const tip) const { m_tooltip.AddTool(wnd, tip); }

BOOL PlayerOptionsDialog::OnInitDialog()
{
   m_tooltip.Create(GetHwnd());
   m_tooltip.SetMaxTipWidth(320);

   const Settings& settings = g_app->m_settings;

   // Misc section
   {
      bool on = g_app->m_settings.GetPlayer_EnableCameraModeFlyAround();
      SendDlgItemMessage(IDC_ENABLE_CAMERA_FLY_AROUND, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
      const int rumbleMode = g_app->m_settings.GetPlayer_RumbleMode();
      const HWND hwndRumble = GetDlgItem(IDC_COMBO_RUMBLE).GetHwnd();
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Off");
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Table only (N/A yet)"); //!! not supported yet
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Generic only (N/A yet)"); //!! not supported yet
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Table with generic fallback");
      ::SendMessage(hwndRumble, CB_SETCURSEL, rumbleMode, 0);

      const bool overwiteBallImage = settings.GetPlayer_OverwriteBallImage();
   }

   // VR section
   {
      AddToolTip(GetDlgItem(IDC_TURN_VR_ON), "Disable VR auto-detection, e.g. if Visual Pinball refuses to start up.");

#if defined(ENABLE_DX9) || defined(ENABLE_OPENGL)
      GetDlgItem(IDC_TURN_VR_ON).EnableWindow(FALSE);
#endif

      const int askToTurnOn = settings.GetPlayerVR_AskToTurnOn();
      HWND hwnd = GetDlgItem(IDC_TURN_VR_ON).GetHwnd();
      ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "VR enabled");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "VR autodetect");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "VR disabled");
      ::SendMessage(hwnd, CB_SETCURSEL, askToTurnOn, 0);
      ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
   }

   return TRUE;
}

void PlayerOptionsDialog::OnOK()
{
   Settings& settings = g_app->m_settings;

   // Misc section
   {
      size_t selected = IsDlgButtonChecked(IDC_ENABLE_CAMERA_FLY_AROUND);
      settings.SetPlayer_EnableCameraModeFlyAround(selected != 0, false);

      const int rumble = (int)SendDlgItemMessage(IDC_COMBO_RUMBLE, CB_GETCURSEL, 0, 0);
      settings.SetPlayer_RumbleMode(rumble, false);
   }

   // VR section
   {
      const size_t askToTurnOn = SendDlgItemMessage(IDC_TURN_VR_ON, CB_GETCURSEL, 0, 0);
      settings.SetPlayerVR_AskToTurnOn((int)askToTurnOn, false);
   }

   settings.Save();
   CDialog::OnOK();
}
