// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "VROptionsDialog.h"

#include "input/ScanCodes.h"

static bool oldScaleValue = false;
static float scaleRelative = 1.0f;
static float scaleAbsolute = 55.0f;


VROptionsDialog::VROptionsDialog() : CDialog(IDD_VR_OPTIONS)
{
}

void VROptionsDialog::AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = (char*)text;
   ::SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

BOOL VROptionsDialog::OnInitDialog()
{
   const HWND hwndDlg = GetHwnd();
   const HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_pvp->theInstance, NULL);
   if (toolTipHwnd)
   {
      ::SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
      AddToolTip("Disable VR auto-detection, e.g. if Visual Pinball refuses to start up.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_TURN_VR_ON).GetHwnd());
      AddToolTip("Pixel format for VR Rendering.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd());
      AddToolTip("Attempt to capture an external DMD window such as Freezy/DMDext, UltraDMD or P-ROC.\r\n\r\nFor Freezy/DMDext the DmdDevice.ini needs to set 'stayontop = true'.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_CAP_EXTDMD).GetHwnd());
      AddToolTip("Attempt to capture the PUP player window and display it as a Backglass in VR.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_CAP_PUP).GetHwnd());
   }

   #ifdef ENABLE_XR
      GetDlgItem(IDC_SCALE_TO_CM).ShowWindow(SW_HIDE); // OpenXR always use fixed scale to real world lockbar width
      GetDlgItem(IDC_VR_SCALE_LABEL).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_VR_SCALE).ShowWindow(SW_HIDE);

      GetDlgItem(IDC_STATIC1).ShowWindow(SW_HIDE); // No performance/hack option for the time being
      GetDlgItem(IDC_STATIC2).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_COMBO_TEXTURE).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_NEAR_LABEL).ShowWindow(SW_HIDE); // OpenXR use fixed near plane distance in real world unit
      GetDlgItem(IDC_NEAR_PLANE).ShowWindow(SW_HIDE);

      GetDlgItem(IDC_STATIC21).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_VR_SLOPE_LABEL).ShowWindow(SW_HIDE); // OpenXR only compensate the playfield slope (no additional user adjustment)
      GetDlgItem(IDC_VR_SLOPE).ShowWindow(SW_HIDE);
   #endif

   const bool scaleToFixedWidth = g_pvp->m_settings.GetPlayerVR_ScaleToFixedWidth();
   oldScaleValue = scaleToFixedWidth;
   SendDlgItemMessage(IDC_SCALE_TO_CM, BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

   scaleRelative = g_pvp->m_settings.GetPlayerVR_ScaleRelative();
   scaleAbsolute = g_pvp->m_settings.GetPlayerVR_ScaleAbsolute();

   SetDlgItemText(IDC_VR_SCALE, f2sz(scaleToFixedWidth ? scaleAbsolute : scaleRelative).c_str());

   SetDlgItemText(IDC_NEAR_PLANE, f2sz(g_pvp->m_settings.GetPlayerVR_NearPlane()).c_str());
   SetDlgItemText(IDC_VR_SLOPE, f2sz(g_pvp->m_settings.GetPlayerVR_Slope()).c_str());

   const int askToTurnOn = g_pvp->m_settings.GetPlayerVR_AskToTurnOn();
   HWND hwnd = GetDlgItem(IDC_TURN_VR_ON).GetHwnd();
   ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR enabled");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR autodetect");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR disabled");
   ::SendMessage(hwnd, CB_SETCURSEL, askToTurnOn, 0);
   ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   bool on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "CaptureExternalDMD"s, false);
   SendDlgItemMessage(IDC_CAP_EXTDMD, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "CapturePUP"s, false);
   SendDlgItemMessage(IDC_CAP_PUP, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   //AMD Debugging
   const int textureModeVR = g_pvp->m_settings.GetPlayerVR_EyeFBFormat();
   hwnd = GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd();
   ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGB 8");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGBA 8 (Recommended)");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGB 16F");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGBA 16F");
   ::SendMessage(hwnd, CB_SETCURSEL, textureModeVR, 0);
   ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   return TRUE;
}

BOOL VROptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_SCALE_TO_CM:
      {
         #ifdef ENABLE_XR
            // Disable the custom scale as we always scale against the real world lockbar width
            GetDlgItem(IDC_VR_SCALE).ShowWindow(SW_HIDE);
         #else
         const bool isScaleToLockbarWidth = IsDlgButtonChecked(IDC_SCALE_TO_CM) > 0;
         if (oldScaleValue != isScaleToLockbarWidth)
         {
            const float tmpf = sz2f(GetDlgItemText(IDC_VR_SCALE).GetString());
            if (oldScaleValue)
               scaleAbsolute = tmpf;
            else
               scaleRelative = tmpf;

            SetDlgItemText(IDC_VR_SCALE, f2sz(isScaleToLockbarWidth ? scaleAbsolute : scaleRelative).c_str());
            oldScaleValue = isScaleToLockbarWidth;
         }
         #endif
         break;
      }
      default:
         return FALSE;
   }
   return TRUE;
}

void VROptionsDialog::OnOK()
{
   const size_t textureModeVR = SendDlgItemMessage(IDC_COMBO_TEXTURE, CB_GETCURSEL, 0, 0);
   g_pvp->m_settings.SetPlayerVR_EyeFBFormat((int)textureModeVR, false);

   const bool scaleToFixedWidth = IsDlgButtonChecked(IDC_SCALE_TO_CM)!= 0;
   g_pvp->m_settings.SetPlayerVR_ScaleToFixedWidth(scaleToFixedWidth, false);
   
   if (scaleToFixedWidth)
      g_pvp->m_settings.SetPlayerVR_ScaleAbsolute(sz2f(GetDlgItemText(IDC_VR_SCALE).GetString()), false);
   else
      g_pvp->m_settings.SetPlayerVR_ScaleRelative(sz2f(GetDlgItemText(IDC_VR_SCALE).GetString()), false);

   g_pvp->m_settings.SetPlayerVR_NearPlane(sz2f(GetDlgItemText(IDC_NEAR_PLANE).GetString()), false);

   //For compatibility keep these in Player instead of PlayerVR
   g_pvp->m_settings.SetPlayerVR_Slope(sz2f(GetDlgItemText(IDC_VR_SLOPE).GetString()), false);

   const size_t askToTurnOn = SendDlgItemMessage(IDC_TURN_VR_ON, CB_GETCURSEL, 0, 0);
   g_pvp->m_settings.SetPlayerVR_AskToTurnOn((int)askToTurnOn, false);

   bool selected = IsDlgButtonChecked(IDC_CAP_EXTDMD)!= 0;
   g_pvp->m_settings.SaveValue(Settings::Player, "CaptureExternalDMD"s, selected);

   selected = IsDlgButtonChecked(IDC_CAP_PUP)!= 0;
   g_pvp->m_settings.SaveValue(Settings::Player, "CapturePUP"s, selected);

   CDialog::OnOK();
}
