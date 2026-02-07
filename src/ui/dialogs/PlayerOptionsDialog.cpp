// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "PlayerOptionsDialog.h"


PlayerOptionsDialog::PlayerOptionsDialog()
   : CDialog(IDD_PLAYER_OPTS)
{
}

void PlayerOptionsDialog::AddToolTip(const CWnd& wnd, const char* const tip) const { m_tooltip.AddTool(wnd, tip); }

void PlayerOptionsDialog::AddStringDOF(const string& name, const int idc) const
{
   const int selected = g_app->m_settings.GetInt(Settings::GetRegistry().GetPropertyId("Controller"s, name).value());
   const HWND hwnd = GetDlgItem(idc).GetHwnd();
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Sound FX");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "DOF");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Both");
   ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
}

void PlayerOptionsDialog::SetDOFValue(int nID, const string& name) const
{
   LRESULT selected = SendDlgItemMessage(nID, CB_GETCURSEL, 0, 0);
   if (selected == LB_ERR)
      selected = 2; // assume both as standard
   g_app->m_settings.Set(Settings::GetRegistry().GetPropertyId("Controller"s, name).value(), (int)selected, false);
}

BOOL PlayerOptionsDialog::OnInitDialog()
{
   m_tooltip.Create(GetHwnd());
   m_tooltip.SetMaxTipWidth(320);

   const Settings& settings = g_app->m_settings;

   // Misc section
   {
      bool on = g_app->m_settings.GetPlayer_EnableCameraModeFlyAround();
      SendDlgItemMessage(IDC_ENABLE_CAMERA_FLY_AROUND, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
      on = g_app->m_settings.GetController_ForceDisableB2S();
      SendDlgItemMessage(IDC_DOF_FORCEDISABLE, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);
      const int rumbleMode = g_app->m_settings.GetPlayer_RumbleMode();
      const HWND hwndRumble = GetDlgItem(IDC_COMBO_RUMBLE).GetHwnd();
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Off");
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Table only (N/A yet)"); //!! not supported yet
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Generic only (N/A yet)"); //!! not supported yet
      ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM) "Table with generic fallback");
      ::SendMessage(hwndRumble, CB_SETCURSEL, rumbleMode, 0);

      AttachItem(IDC_HEADTRACKING, m_bamHeadtracking);
      AddToolTip(m_bamHeadtracking, "Enables BAM Headtracking. See https://www.ravarcade.pl for details.");
      AttachItem(IDC_OVERWRITE_BALL_IMAGE_CHECK, m_ballOverrideImages);
      AddToolTip(m_ballOverrideImages, "When checked, it overwrites the ball image/decal image(s) for every table.");
      AttachItem(IDC_BALL_IMAGE_EDIT, m_ballImage);
      AttachItem(IDC_BALL_DECAL_EDIT, m_ballDecal);

      m_bamHeadtracking.SetCheck(settings.GetPlayer_BAMHeadTracking() ? BST_CHECKED : BST_UNCHECKED);

      const bool overwiteBallImage = settings.GetPlayer_OverwriteBallImage();
      m_ballOverrideImages.SetCheck(overwiteBallImage ? BST_CHECKED : BST_UNCHECKED);
      m_ballImage.SetWindowText(settings.GetPlayer_BallImage().c_str());
      m_ballDecal.SetWindowText(settings.GetPlayer_DecalImage().c_str());
   }

   // VR section
   {
      AddToolTip(GetDlgItem(IDC_TURN_VR_ON), "Disable VR auto-detection, e.g. if Visual Pinball refuses to start up.");
      AddToolTip(GetDlgItem(IDC_COMBO_TEXTURE), "Pixel format for VR Rendering.");
      AddToolTip(GetDlgItem(IDC_CAP_EXTDMD),
         "Attempt to capture an external DMD window such as Freezy/DMDext, UltraDMD or P-ROC.\r\n\r\nFor Freezy/DMDext the DmdDevice.ini needs to set 'stayontop = true'.");
      AddToolTip(GetDlgItem(IDC_CAP_PUP), "Attempt to capture the PUP player window and display it as a Backglass in VR.");

#if defined(ENABLE_XR) || defined(ENABLE_DX9)
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

#if defined(ENABLE_DX9)
      GetDlgItem(IDC_TURN_VR_ON).EnableWindow(FALSE);
      GetDlgItem(IDC_CAP_EXTDMD).EnableWindow(FALSE);
      GetDlgItem(IDC_CAP_PUP).EnableWindow(FALSE);
#endif

      const bool scaleToFixedWidth = settings.GetPlayerVR_ScaleToFixedWidth();
      oldScaleValue = scaleToFixedWidth;
      SendDlgItemMessage(IDC_SCALE_TO_CM, BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

      scaleRelative = settings.GetPlayerVR_ScaleRelative();
      scaleAbsolute = settings.GetPlayerVR_ScaleAbsolute();

      SetDlgItemText(IDC_VR_SCALE, f2sz(scaleToFixedWidth ? scaleAbsolute : scaleRelative).c_str());

      SetDlgItemText(IDC_NEAR_PLANE, f2sz(settings.GetPlayerVR_NearPlane()).c_str());
      SetDlgItemText(IDC_VR_SLOPE, f2sz(settings.GetPlayerVR_Slope()).c_str());

      const int askToTurnOn = settings.GetPlayerVR_AskToTurnOn();
      HWND hwnd = GetDlgItem(IDC_TURN_VR_ON).GetHwnd();
      ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "VR enabled");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "VR autodetect");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "VR disabled");
      ::SendMessage(hwnd, CB_SETCURSEL, askToTurnOn, 0);
      ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

      bool on = settings.GetPlayer_CaptureExternalDMD();
      SendDlgItemMessage(IDC_CAP_EXTDMD, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

      on = settings.GetPlayer_CapturePUP();
      SendDlgItemMessage(IDC_CAP_PUP, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

      //AMD Debugging
      const int textureModeVR = settings.GetPlayerVR_EyeFBFormat();
      hwnd = GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd();
      ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "RGB 8");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "RGBA 8 (Recommended)");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "RGB 16F");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "RGBA 16F");
      ::SendMessage(hwnd, CB_SETCURSEL, textureModeVR, 0);
      ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
   }

   // Direct Output Framework
   {
      AddStringDOF("DOFContactors"s, IDC_DOF_CONTACTORS);
      AddStringDOF("DOFKnocker"s, IDC_DOF_KNOCKER);
      AddStringDOF("DOFChimes"s, IDC_DOF_CHIMES);
      AddStringDOF("DOFBell"s, IDC_DOF_BELL);
      AddStringDOF("DOFGear"s, IDC_DOF_GEAR);
      AddStringDOF("DOFShaker"s, IDC_DOF_SHAKER);
      AddStringDOF("DOFFlippers"s, IDC_DOF_FLIPPERS);
      AddStringDOF("DOFTargets"s, IDC_DOF_TARGETS);
      AddStringDOF("DOFDropTargets"s, IDC_DOF_DROPTARGETS);
   }

   OnCommand(IDC_OVERWRITE_BALL_IMAGE_CHECK, 0);
   return TRUE;
}

void PlayerOptionsDialog::OnOK()
{
   Settings& settings = g_app->m_settings;

   // Misc section
   {
      size_t selected = IsDlgButtonChecked(IDC_ENABLE_CAMERA_FLY_AROUND);
      settings.SetPlayer_EnableCameraModeFlyAround(selected != 0, false);

      selected = IsDlgButtonChecked(IDC_DOF_FORCEDISABLE);
      settings.SetController_ForceDisableB2S(selected != 0, false);

      const int rumble = (int)SendDlgItemMessage(IDC_COMBO_RUMBLE, CB_GETCURSEL, 0, 0);
      settings.SetPlayer_RumbleMode(rumble, false);

      settings.SetPlayer_BAMHeadTracking(m_bamHeadtracking.GetCheck() == BST_CHECKED, false);

      const bool overwriteEnabled = m_ballOverrideImages.GetCheck() == BST_CHECKED;
      settings.SetPlayer_OverwriteBallImage(overwriteEnabled, false);
      settings.SetPlayer_BallImage(overwriteEnabled ? m_ballImage.GetWindowText().GetString() : ""s, false);
      settings.SetPlayer_DecalImage(overwriteEnabled ? m_ballDecal.GetWindowText().GetString() : ""s, false);
   }

   // VR section
   {
      const size_t textureModeVR = SendDlgItemMessage(IDC_COMBO_TEXTURE, CB_GETCURSEL, 0, 0);
      settings.SetPlayerVR_EyeFBFormat((int)textureModeVR, false);

      const bool scaleToFixedWidth = IsDlgButtonChecked(IDC_SCALE_TO_CM) != 0;
      settings.SetPlayerVR_ScaleToFixedWidth(scaleToFixedWidth, false);

      if (scaleToFixedWidth)
         settings.SetPlayerVR_ScaleAbsolute(sz2f(GetDlgItemText(IDC_VR_SCALE).GetString()), false);
      else
         settings.SetPlayerVR_ScaleRelative(sz2f(GetDlgItemText(IDC_VR_SCALE).GetString()), false);

      settings.SetPlayerVR_NearPlane(sz2f(GetDlgItemText(IDC_NEAR_PLANE).GetString()), false);

      //For compatibility keep these in Player instead of PlayerVR
      settings.SetPlayerVR_Slope(sz2f(GetDlgItemText(IDC_VR_SLOPE).GetString()), false);

      const size_t askToTurnOn = SendDlgItemMessage(IDC_TURN_VR_ON, CB_GETCURSEL, 0, 0);
      settings.SetPlayerVR_AskToTurnOn((int)askToTurnOn, false);

      bool selected = IsDlgButtonChecked(IDC_CAP_EXTDMD) != 0;
      settings.SetPlayer_CaptureExternalDMD(selected, false);

      selected = IsDlgButtonChecked(IDC_CAP_PUP) != 0;
      settings.SetPlayer_CapturePUP(selected, false);
   }

   // Direct Output Framework
   {
      SetDOFValue(IDC_DOF_CONTACTORS, "DOFContactors"s);
      SetDOFValue(IDC_DOF_KNOCKER, "DOFKnocker"s);
      SetDOFValue(IDC_DOF_CHIMES, "DOFChimes"s);
      SetDOFValue(IDC_DOF_BELL, "DOFBell"s);
      SetDOFValue(IDC_DOF_GEAR, "DOFGear"s);
      SetDOFValue(IDC_DOF_SHAKER, "DOFShaker"s);
      SetDOFValue(IDC_DOF_FLIPPERS, "DOFFlippers"s);
      SetDOFValue(IDC_DOF_TARGETS, "DOFTargets"s);
      SetDOFValue(IDC_DOF_DROPTARGETS, "DOFDropTargets"s);
   }

   settings.Save();
   CDialog::OnOK();
}

BOOL PlayerOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
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
      return TRUE;
   }

   case IDC_OVERWRITE_BALL_IMAGE_CHECK:
   {
      const bool overwiteBallImage = m_ballOverrideImages.GetCheck() == BST_CHECKED;
      GetDlgItem(IDC_BROWSE_BALL_IMAGE).EnableWindow(overwiteBallImage ? TRUE : FALSE);
      GetDlgItem(IDC_BROWSE_BALL_DECAL).EnableWindow(overwiteBallImage ? TRUE : FALSE);
      m_ballImage.EnableWindow(overwiteBallImage ? TRUE : FALSE);
      m_ballDecal.EnableWindow(overwiteBallImage ? TRUE : FALSE);
      return TRUE;
   }

   case IDC_BROWSE_BALL_IMAGE:
   case IDC_BROWSE_BALL_DECAL:
   {
      char szFileName[MAXSTRING];
      szFileName[0] = '\0';
      OPENFILENAME ofn = {};
      ofn.lStructSize = sizeof(OPENFILENAME);
      ofn.hInstance = g_app->GetInstanceHandle();
      ofn.hwndOwner = g_pvp->GetHwnd();
      ofn.lpstrFilter = "Bitmap, JPEG, PNG, TGA, WEBP, EXR, HDR Files (.bmp/.jpg/.png/.tga/.webp/.exr/.hdr)\0*.bmp;*.jpg;*.jpeg;*.png;*.tga;*.webp;*.exr;*.hdr\0";
      ofn.lpstrFile = szFileName;
      ofn.nMaxFile = sizeof(szFileName);
      ofn.lpstrDefExt = "png";
      ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
      if (GetOpenFileName(&ofn))
      {
         if (LOWORD(wParam) == IDC_BALL_IMAGE_EDIT)
            m_ballImage.SetWindowText(szFileName);
         if (LOWORD(wParam) == IDC_BALL_DECAL_EDIT)
            m_ballDecal.SetWindowText(szFileName);
      }
      SetFocus();
      return TRUE;
   }

   default: break;
   }
   return FALSE;
}
