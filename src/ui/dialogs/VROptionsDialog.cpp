// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "VROptionsDialog.h"

#include "input/ScanCodes.h"

static bool oldScaleValue = false;
static float scaleRelative = 1.0f;
static float scaleAbsolute = 55.0f;


static SDL_Scancode GetNextKey()
{
   SDL_Scancode sdlk = SDL_SCANCODE_UNKNOWN;
   for (unsigned int i = VK_BACK; i <= VK_OEM_CLEAR; ++i)
   {
      const SHORT keyState = GetAsyncKeyState(i);
      if (keyState & 1)
      {
         const SDL_Scancode sc = GetSDLScancodeFromWin32VirtualKey(i);
         if (sc != SDL_SCANCODE_UNKNOWN)
            sdlk = sc; // search for a higher key value as some keys have first undifferentiated values (shift) then differentiated values (left shift, right shift)
      }
   }
   return sdlk;
}


class KeyWindowStruct
{
public:
   PinInput pi;
   HWND hwndKeyControl; // window to get the key assignment
   UINT_PTR m_timerid; // timer id for our key assignment
};

static WNDPROC g_ButtonProc2;

LRESULT CALLBACK MyKeyButtonProc2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   if (uMsg == WM_GETDLGCODE)
      // Eat all acceleratable messges
      return (DLGC_WANTARROWS | DLGC_WANTTAB | DLGC_WANTALLKEYS | DLGC_WANTCHARS);
   else
      return CallWindowProc(g_ButtonProc2, hwnd, uMsg, wParam, lParam);
}

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

void VROptionsDialog::ResetVideoPreferences()
{
   constexpr bool scaleToFixedWidth = false;
   oldScaleValue = scaleToFixedWidth;
   SendDlgItemMessage(IDC_SCALE_TO_CM, BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

   scaleRelative = 1.0f;
   scaleAbsolute = 55.0f;

   SetDlgItemText(IDC_VR_SCALE, f2sz(scaleToFixedWidth ? scaleAbsolute : scaleRelative).c_str());

   SetDlgItemText(IDC_NEAR_PLANE, f2sz(5.0f).c_str());
   SetDlgItemText(IDC_VR_SLOPE, f2sz(6.5f).c_str());

   SetDlgItemText(IDC_3D_VR_ORIENTATION, f2sz(0.0f).c_str());
   SetDlgItemText(IDC_VR_OFFSET_X, f2sz(0.0f).c_str());
   SetDlgItemText(IDC_VR_OFFSET_Y, f2sz(0.0f).c_str());
   SetDlgItemText(IDC_VR_OFFSET_Z, f2sz(80.0f).c_str());

   SendDlgItemMessage(IDC_TURN_VR_ON, CB_SETCURSEL, 1, 0);

   SendDlgItemMessage(IDC_DMD_SOURCE, CB_SETCURSEL, 1, 0);
   SendDlgItemMessage(IDC_BG_SOURCE, CB_SETCURSEL, 1, 0);

   SendDlgItemMessage(IDC_CAP_EXTDMD, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);
   SendDlgItemMessage(IDC_CAP_PUP, BM_SETCHECK, false ? BST_CHECKED : BST_UNCHECKED, 0);

   SendDlgItemMessage(IDC_COMBO_TEXTURE, CB_SETCURSEL, 1, 0);
}

BOOL VROptionsDialog::OnInitDialog()
{
   const HWND hwndDlg = GetHwnd();
   const HWND toolTipHwnd = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hwndDlg, NULL, g_pvp->theInstance, NULL);
   if (toolTipHwnd)
   {
      ::SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
      AddToolTip("Disable VR auto-detection, e.g. if Visual Pinball refuses to start up.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_TURN_VR_ON).GetHwnd());
      AddToolTip("What sources should be used for DMD?\nOnly internally supplied via Script/Text Label/Flasher\nScreenreader (see screenreader.ini)\nFrom Shared Memory API", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DMD_SOURCE).GetHwnd());
      AddToolTip("What sources should be used for Backglass?\nOnly internal background\nTry to open a directb2s file\ndirectb2s file dialog\nScreenreader (see screenreader.ini)\nFrom Shared Memory API", hwndDlg, toolTipHwnd, GetDlgItem(IDC_BG_SOURCE).GetHwnd());
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
      GetDlgItem(IDC_STATIC3).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_NEAR_LABEL).ShowWindow(SW_HIDE); // OpenXR use fixed near plane distance in real world unit
      GetDlgItem(IDC_NEAR_PLANE).ShowWindow(SW_HIDE);

      GetDlgItem(IDC_VR_SLOPE_LABEL).ShowWindow(SW_HIDE); // OpenXR only compensate the playfield slope (no additional user adjustment)
      GetDlgItem(IDC_VR_SLOPE).ShowWindow(SW_HIDE);

      GetDlgItem(IDC_BTTABLERECENTER).ShowWindow(SW_HIDE); // Position is managed through TweakUI, not custom key shortcuts
      GetDlgItem(IDC_TABLEREC_TEXT).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_JOYTABLERECENTER).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_STATIC4).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_STATIC5).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_STATIC6).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_BTTABLEUP).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_TABLEUP_TEXT).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_JOYTABLEUP).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_BTTABLEDOWN).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_TABLEDOWN_TEXT).ShowWindow(SW_HIDE);
      GetDlgItem(IDC_JOYTABLEDOWN).ShowWindow(SW_HIDE);
   #endif

   const bool scaleToFixedWidth = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleToFixedWidth"s, false);
   oldScaleValue = scaleToFixedWidth;
   SendDlgItemMessage(IDC_SCALE_TO_CM, BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

   scaleRelative = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleRelative"s, 1.0f);
   scaleAbsolute = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleAbsolute"s, 55.0f);

   SetDlgItemText(IDC_VR_SCALE, f2sz(scaleToFixedWidth ? scaleAbsolute : scaleRelative).c_str());

   SetDlgItemText(IDC_NEAR_PLANE, f2sz(g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "NearPlane"s, 5.0f)).c_str());
   SetDlgItemText(IDC_VR_SLOPE, f2sz(g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Slope"s, 6.5f)).c_str());
   SetDlgItemText(IDC_3D_VR_ORIENTATION, f2sz(g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Orientation"s, 0.0f)).c_str());
   SetDlgItemText(IDC_VR_OFFSET_X, f2sz(g_pvp->m_settings.LoadValueFloat(Settings::PlayerVR, "TableX"s)).c_str());
   SetDlgItemText(IDC_VR_OFFSET_Y, f2sz(g_pvp->m_settings.LoadValueFloat(Settings::PlayerVR, "TableY"s)).c_str());
   SetDlgItemText(IDC_VR_OFFSET_Z, f2sz(g_pvp->m_settings.LoadValueFloat(Settings::PlayerVR, "TableZ"s)).c_str());

   const int askToTurnOn = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "AskToTurnOn"s, 1);
   HWND hwnd = GetDlgItem(IDC_TURN_VR_ON).GetHwnd();
   ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR enabled");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR autodetect");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR disabled");
   ::SendMessage(hwnd, CB_SETCURSEL, askToTurnOn, 0);
   ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const int DMDsource = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "DMDSource"s, 1); // Unimplemented for the time being
   hwnd = GetDlgItem(IDC_DMD_SOURCE).GetHwnd();
   ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Internal Text/Flasher (via vbscript)");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Screenreader");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"SharedMemory API");
   ::SendMessage(hwnd, CB_SETCURSEL, DMDsource, 0);
   ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const int BGsource = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "BGSource"s, 1); // Unimplemented for the time being
   hwnd = GetDlgItem(IDC_BG_SOURCE).GetHwnd();
   ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Default table background");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"directb2s File (auto only)");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"directb2s File");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"SharedMemory API");
   ::SendMessage(hwnd, CB_SETCURSEL, BGsource, 0);
   ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   bool on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "CaptureExternalDMD"s, false);
   SendDlgItemMessage(IDC_CAP_EXTDMD, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "CapturePUP"s, false);
   SendDlgItemMessage(IDC_CAP_PUP, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   //AMD Debugging
   const int textureModeVR = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "EyeFBFormat"s, 1);
   hwnd = GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd();
   ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGB 8");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGBA 8 (Recommended)");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGB 16F");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGBA 16F");
   ::SendMessage(hwnd, CB_SETCURSEL, textureModeVR, 0);
   ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   for (unsigned int i = eTableRecenter; i <= eTableDown; ++i)
      if (regkey_idc[i] != -1)
      {
         const int dik = g_pvp->m_settings.LoadValueInt(Settings::Player, regkey_string[i]);
         const HWND hwndControl = GetDlgItem(regkey_idc[i]);
         ::SetWindowLongPtr(hwndControl, GWLP_USERDATA, dik);
         SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode(GetSDLScancodeFromDirectInputKey(dik), SDL_KMOD_NONE, false);
         ::SetWindowText(hwndControl, SDL_GetKeyName(sdlKeycode));
      }

   for (unsigned int i = 0; i < 3; ++i)
   {
      bool hr;
      int item, selected;
      switch (i)
      {
      case 0: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyTableRecenterKey"s, selected); item = IDC_JOYTABLERECENTER; break;
      case 1: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyTableUpKey"s, selected); item = IDC_JOYTABLEUP; break;
      case 2: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyTableDownKey"s, selected); item = IDC_JOYTABLEDOWN; break;
      }
      
      if (!hr)
         selected = 0; // assume no assignment as standard

      hwnd = GetDlgItem(item).GetHwnd();
      ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "(none)");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 1");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 2");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 3");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 4");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 5");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 6");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 7");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 8");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 9");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 10");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 11");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 12");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 13");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 14");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 15");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 16");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 17");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 18");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 19");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 20");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 21");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 22");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 23");
      ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Button 24");
      if (item == IDC_JOYLFLIPCOMBO || item == IDC_JOYRFLIPCOMBO || item == IDC_JOYPLUNGERCOMBO)
      {
         ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Left Mouse");
         ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Right Mouse");
         ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Middle Mouse");
      }
      ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
      ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
      #ifdef ENABLE_XR
         ::EnableWindow(hwnd, FALSE);
      #endif
   }

   //

   KeyWindowStruct* const pksw = new KeyWindowStruct();
   pksw->pi.SetFocusWindow(GetHwnd());
   pksw->pi.Init();
   pksw->m_timerid = 0;
   SetWindowLongPtr(GWLP_USERDATA, (size_t)pksw);

   // Set buttons to ignore keyboard shortcuts when using DirectInput
   HWND hwndButton = GetDlgItem(IDC_BTTABLERECENTER).GetHwnd();
   g_ButtonProc2 = (WNDPROC)::GetWindowLongPtr(hwndButton, GWLP_WNDPROC);
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);
   #ifdef ENABLE_XR
      ::EnableWindow(hwndButton, FALSE);
   #endif

   hwndButton = GetDlgItem(IDC_BTTABLEUP).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);
   #ifdef ENABLE_XR
      ::EnableWindow(hwndButton, FALSE);
   #endif

   hwndButton = GetDlgItem(IDC_BTTABLEDOWN).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);
   #ifdef ENABLE_XR
      ::EnableWindow(hwndButton, FALSE);
   #endif

   return TRUE;
}

INT_PTR VROptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifndef ENABLE_XR
   switch (uMsg)
   {
   case WM_TIMER:
   {
      KeyWindowStruct* const pksw = (KeyWindowStruct*)GetWindowLongPtr(GWLP_USERDATA);
      SDL_Scancode sdlScancode = GetNextKey();
      if (sdlScancode == SDL_SCANCODE_UNKNOWN)
         break; // no key pressed

      int dik = GetDirectInputKeyFromSDLScancode(sdlScancode);
      if (dik == 0)
         break; // unmapped key

      if (dik == DIK_ESCAPE)
         sdlScancode = GetSDLScancodeFromDirectInputKey((unsigned char)::GetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA)); // reset key to old value
      else
         ::SetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA, dik);

      // Display key name using user keyboard layout (but store it as scancode to support user layout change)
      SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode(sdlScancode, SDL_KMOD_NONE, false);
      ::SetWindowText(pksw->hwndKeyControl, SDL_GetKeyName(sdlKeycode));

      KillTimer(pksw->m_timerid);
      pksw->m_timerid = 0;
      break;
   }
   }
#endif

   return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL VROptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);

   switch (LOWORD(wParam))
   {
      case IDC_DEFAULTS:
      {
         ResetVideoPreferences();
         break;
      }
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
      #ifndef ENABLE_XR
      case IDC_BTTABLERECENTER:
      {
         if (HIWORD(wParam) == BN_CLICKED)
            StartTimer(IDC_TABLEREC_TEXT);
         break;
      }
      case IDC_BTTABLEUP:
      {
         if (HIWORD(wParam) == BN_CLICKED)
            StartTimer(IDC_TABLEUP_TEXT);
         break;
      }
      case IDC_BTTABLEDOWN:
      {
         if (HIWORD(wParam) == BN_CLICKED)
            StartTimer(IDC_TABLEDOWN_TEXT);
         break;
      }
      #endif
      default:
         return FALSE;
   }
   return TRUE;
}

void VROptionsDialog::OnOK()
{
   const size_t textureModeVR = SendDlgItemMessage(IDC_COMBO_TEXTURE, CB_GETCURSEL, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "EyeFBFormat"s, (int)textureModeVR);

   const bool scaleToFixedWidth = IsDlgButtonChecked(IDC_SCALE_TO_CM)!= 0;
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "ScaleToFixedWidth"s, scaleToFixedWidth);

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, scaleToFixedWidth ? "ScaleAbsolute"s : "ScaleRelative"s, sz2f(GetDlgItemText(IDC_VR_SCALE).GetString()));
   //g_pvp->m_settings.SaveValue(Settings::PlayerVR, scaleToFixedWidth ? "ScaleRelative"s : "ScaleAbsolute"s, scaleToFixedWidth ? scaleRelative : scaleAbsolute); //Also update hidden value?

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "NearPlane"s, sz2f(GetDlgItemText(IDC_NEAR_PLANE).GetString()));

   //For compatibility keep these in Player instead of PlayerVR
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "Slope"s, sz2f(GetDlgItemText(IDC_VR_SLOPE).GetString()));

   const size_t askToTurnOn = SendDlgItemMessage(IDC_TURN_VR_ON, CB_GETCURSEL, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "AskToTurnOn"s, (int)askToTurnOn);

   const size_t dmdSource = SendDlgItemMessage(IDC_DMD_SOURCE, CB_GETCURSEL, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "DMDSource"s, (int)dmdSource);

   const size_t bgSource = SendDlgItemMessage(IDC_BG_SOURCE, CB_GETCURSEL, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "BGSource"s, (int)bgSource);

   bool selected = IsDlgButtonChecked(IDC_CAP_EXTDMD)!= 0;
   g_pvp->m_settings.SaveValue(Settings::Player, "CaptureExternalDMD"s, selected);

   selected = IsDlgButtonChecked(IDC_CAP_PUP)!= 0;
   g_pvp->m_settings.SaveValue(Settings::Player, "CapturePUP"s, selected);

   SetValue(IDC_JOYTABLERECENTER, Settings::Player, "JoyTableRecenterKey"s);
   SetValue(IDC_JOYTABLEUP, Settings::Player, "JoyTableUpKey"s);
   SetValue(IDC_JOYTABLEDOWN, Settings::Player, "JoyTableDownKey"s);

    for (unsigned int i = eTableRecenter; i <= eTableDown; ++i) if (regkey_idc[i] != -1)
    {
       const size_t dik = GetDlgItem(regkey_idc[i]).GetWindowLongPtr(GWLP_USERDATA);
       g_pvp->m_settings.SaveValue(Settings::Player, regkey_string[i], (int)dik);
    }

   CDialog::OnOK();
}

void VROptionsDialog::OnDestroy()
{
   KeyWindowStruct* const pksw = (KeyWindowStruct*)GetWindowLongPtr(GWLP_USERDATA);
   if (pksw->m_timerid)
   {
      KillTimer(pksw->m_timerid);
      pksw->m_timerid = 0;
   }
   pksw->pi.UnInit();
   CDialog::OnDestroy();
}

void VROptionsDialog::SetValue(int nID, const Settings::Section& section, const string& key)
{
   LRESULT selected = SendDlgItemMessage(nID, CB_GETCURSEL, 0, 0);
   if (selected == LB_ERR)
      selected = 2; // assume both as standard
   g_pvp->m_settings.SaveValue(section, key, (int)selected);
}

void VROptionsDialog::StartTimer(int nID)
{
   KeyWindowStruct* const pksw = (KeyWindowStruct*)GetWindowLongPtr(GWLP_USERDATA);
   const HWND hwndKeyWindow = GetDlgItem(nID).GetHwnd();
   if (pksw->m_timerid == NULL) //add
   { //add
      // corrects input error with space bar
      const SDL_Scancode key = GetNextKey();
      if (key == SDL_SCANCODE_SPACE)
      {
         GetNextKey(); // Clear the current buffer out
         return;
      }

      GetNextKey(); // Clear the current buffer out

      pksw->m_timerid = SetTimer(100, 50, nullptr);
      pksw->hwndKeyControl = hwndKeyWindow;
      ::SetWindowText(pksw->hwndKeyControl, "????");
      while (GetNextKey() != NULL) //clear entire keyboard buffer contents
      {
         GetNextKey();
      }
   }
}
