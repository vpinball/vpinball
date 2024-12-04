// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "VROptionsDialog.h"

static bool oldScaleValue = false;
static float scaleRelative = 1.0f;
static float scaleAbsolute = 55.0f;

static char rgszKeyName[][10] = {
   "",
   "Escape", //DIK_ESCAPE          0x01
   "1", //DIK_1               0x02
   "2", //DIK_2               0x03
   "3", //DIK_3               0x04
   "4", //DIK_4               0x05
   "5", //DIK_5               0x06
   "6", //DIK_6               0x07
   "7", //DIK_7               0x08
   "8", //DIK_8               0x09
   "9", //DIK_9               0x0A
   "0", //DIK_0               0x0B
   "-", //DIK_MINUS           0x0C    /* - on main keyboard */
   "=", //DIK_EQUALS          0x0D
   "Backspace", //DIK_BACK            0x0E    /* backspace */
   "Tab", //DIK_TAB             0x0F
   "Q", //DIK_Q               0x10
   "W", //DIK_W               0x11
   "E", //DIK_E               0x12
   "R", //DIK_R               0x13
   "T", //DIK_T               0x14
   "Y", //DIK_Y               0x15
   "U", //DIK_U               0x16
   "I", //DIK_I               0x17
   "O", //DIK_O               0x18
   "P", //DIK_P               0x19
   "[", //DIK_LBRACKET        0x1A
   "]", //DIK_RBRACKET        0x1B
   "Enter", //DIK_RETURN          0x1C    /* Enter on main keyboard */
   "L Ctrl", //DIK_LCONTROL        0x1D
   "A", //DIK_A               0x1E
   "S", //DIK_S               0x1F
   "D", //DIK_D               0x20
   "F", //DIK_F               0x21
   "G", //DIK_G               0x22
   "H", //DIK_H               0x23
   "J", //DIK_J               0x24
   "K", //DIK_K               0x25
   "L", //DIK_L               0x26
   ";", //DIK_SEMICOLON       0x27
   "'", //DIK_APOSTROPHE      0x28
   "`", //DIK_GRAVE           0x29    /* accent grave */
   "L Shift", //DIK_LSHIFT          0x2A
   "\\", //DIK_BACKSLASH       0x2B
   "Z", //DIK_Z               0x2C
   "X", //DIK_X               0x2D
   "C", //DIK_C               0x2E
   "V", //DIK_V               0x2F
   "B", //DIK_B               0x30
   "N", //DIK_N               0x31
   "M", //DIK_M               0x32
   ",", //DIK_COMMA           0x33
   ".", //DIK_PERIOD          0x34    /* . on main keyboard */
   "/", //DIK_SLASH           0x35    /* / on main keyboard */
   "R Shift", //DIK_RSHIFT          0x36
   "*", //DIK_MULTIPLY        0x37    /* * on numeric keypad */
   "Menu", //DIK_LMENU           0x38    /* left Alt */
   "Space", //DIK_SPACE           0x39
   "Caps Lock", //DIK_CAPITAL         0x3A
   "F1", //DIK_F1              0x3B
   "F2", //DIK_F2              0x3C
   "F3", //DIK_F3              0x3D
   "F4", //DIK_F4              0x3E
   "F5", //DIK_F5              0x3F
   "F6", //DIK_F6              0x40
   "F7", //DIK_F7              0x41
   "F8", //DIK_F8              0x42
   "F9", //DIK_F9              0x43
   "F10", //DIK_F10             0x44
   "NumLock", //DIK_NUMLOCK         0x45
   "ScrollLck", //DIK_SCROLL          0x46    /* Scroll Lock */
   "Numpad 7", //DIK_NUMPAD7         0x47
   "Numpad 8", //DIK_NUMPAD8         0x48
   "Numpad 9", //DIK_NUMPAD9         0x49
   "-", //DIK_SUBTRACT        0x4A    /* - on numeric keypad */
   "Numpad 4", //DIK_NUMPAD4         0x4B
   "Numpad 5", //DIK_NUMPAD5         0x4C
   "Numpad 6", //DIK_NUMPAD6         0x4D
   "+", //DIK_ADD             0x4E    /* + on numeric keypad */
   "Numpad 1", //DIK_NUMPAD1         0x4F
   "Numpad 2", //DIK_NUMPAD2         0x50
   "Numpad 3", //DIK_NUMPAD3         0x51
   "Numpad 0", //DIK_NUMPAD0         0x52
   "Numpad .", //DIK_DECIMAL         0x53    /* . on numeric keypad */
   "", //0x54
   "", //0x55
   "<>|", //DIK_OEM_102         0x56    /* < > | on UK/Germany keyboards */
   "F11", //DIK_F11             0x57
   "F12", //DIK_F12             0x58
   "", //0x59
   "", //0x5A
   "", //0x5B
   "", //0x5C
   "", //0x5D
   "", //0x5E
   "", //0x5F
   "", //0x60
   "", //0x61
   "", //0x62
   "", //0x63
   "F13", //DIK_F13             0x64    /*                     (NEC PC98) */
   "F14", //DIK_F14             0x65    /*                     (NEC PC98) */
   "F15", //DIK_F15             0x66    /*                     (NEC PC98) */
   "", //0x67
   "", //0x68
   "", //0x69
   "", //0x6A
   "", //0x6B
   "", //0x6C
   "", //0x6D
   "", //0x6E
   "", //0x6F

   "", //0x70
   "", //0x71
   "", //0x72
   "", //0x73
   "", //0x74
   "", //0x75
   "", //0x76
   "", //0x77
   "", //0x78
   "", //0x79
   "", //0x7A
   "", //0x7B
   "", //0x7C
   "", //0x7D
   "", //0x7E
   "", //0x7F

   "", //0x80
   "", //0x81
   "", //0x82
   "", //0x83
   "", //0x84
   "", //0x85
   "", //0x86
   "", //0x87
   "", //0x88
   "", //0x89
   "", //0x8A
   "", //0x8B
   "", //0x8C
   "", //0x8D
   "", //0x8E
   "", //0x8F

   "", //0x90
   "", //0x91
   "", //0x92
   "", //0x93
   "", //0x94
   "", //0x95
   "", //0x96
   "", //0x97
   "", //0x98
   "", //0x99
   "", //0x9A
   "", //0x9B

   "Num Enter", //#define DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
   "R Ctrl", //DIK_RCONTROL        0x9D

   "", //0x9E
   "", //0x9F

   "", //0xA0
   "", //0xA1
   "", //0xA2
   "", //0xA3
   "", //0xA4
   "", //0xA5
   "", //0xA6
   "", //0xA7
   "", //0xA8
   "", //0xA9
   "", //0xAA
   "", //0xAB
   "", //0xAC
   "", //0xAD
   "", //0xAE
   "", //0xAF

   "", //0xB0
   "", //0xB1
   "", //0xB2
   "", //0xB3
   "", //0xB4

   "Numpad /", //#define DIK_DIVIDE          0xB5    /* / on numeric keypad */
   "", //0xB6
   "", //#define DIK_SYSRQ           0xB7
   "R Alt", //DIK_RMENU           0xB8    /* right Alt */

   "", //0xB9
   "", //0xBA
   "", //0xBB
   "", //0xBC
   "", //0xBD
   "", //0xBE
   "", //0xBF

   "", //0xC0
   "", //0xC1
   "", //0xC2
   "", //0xC3
   "", //0xC4
   "", //0xC5
   "", //0xC6

   "Home", //DIK_HOME            0xC7    /* Home on arrow keypad */
   "Up", //DIK_UP              0xC8    /* UpArrow on arrow keypad */
   "Page Up", //DIK_PRIOR           0xC9    /* PgUp on arrow keypad */
   "", //0xCA
   "Left", //DIK_LEFT            0xCB    /* LeftArrow on arrow keypad */
   "", //0xCC
   "Right", //DIK_RIGHT           0xCD    /* RightArrow on arrow keypad */
   "", //0xCE
   "End", //DIK_END             0xCF    /* End on arrow keypad */
   "Down", //DIK_DOWN            0xD0    /* DownArrow on arrow keypad */
   "Page Down", //DIK_NEXT            0xD1    /* PgDn on arrow keypad */
   "Insert", //DIK_INSERT          0xD2    /* Insert on arrow keypad */
   "Delete", //DIK_DELETE          0xD3    /* Delete on arrow keypad */

   "", //0xD4
   "", //0xD5
   "", //0xD6
   "", //0xD7
   "", //0xD8
   "", //0xD9
   "", //0xDA

   "L Windows", //DIK_LWIN            0xDB    /* Left Windows key */
   "R Windows", //DIK_RWIN            0xDC    /* Right Windows key */
   "Apps Menu", //DIK_APPS            0xDD    /* AppMenu key */
};

class KeyWindowStruct
{
public:
   PinInput pi;
   HWND hwndKeyControl; // window to get the key assignment
   UINT_PTR m_timerid; // timer id for our key assignment
};

WNDPROC g_ButtonProc2;

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
   // Adjust keyboard translation to user keyboard layout
   HKL layout = GetKeyboardLayout(0);
   BYTE State[256];
   // Translate for a default keyboard state
   //if (GetKeyboardState(State) != FALSE)
   memset(State, 0, sizeof(State));
   State[VK_CAPITAL] = 1; // Get CAPS LOCK variant of the key
   for (int scancode = 0x00; scancode <= 0xDD; scancode++)
   {
      BYTE result[2];
      UINT vk = MapVirtualKeyEx(scancode, 1, layout);
      if (rgszKeyName[scancode][1] == 0 && ToAsciiEx(vk, scancode, State, (LPWORD)result, 0, layout) == 1)
      {
         rgszKeyName[scancode][0] = (char)result[0];
         rgszKeyName[scancode][1] = '\0';
      }
   }
}

void VROptionsDialog::AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = (char*)text;
   SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void VROptionsDialog::ResetVideoPreferences()
{
   char tmp[256];

   SendDlgItemMessage(IDC_VR_PREVIEW, CB_SETCURSEL, VRPREVIEW_LEFT, 0);

   constexpr bool scaleToFixedWidth = false;
   oldScaleValue = scaleToFixedWidth;
   SendDlgItemMessage(IDC_SCALE_TO_CM, BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

   scaleRelative = 1.0f;
   scaleAbsolute = 55.0f;

   sprintf_s(tmp, sizeof(tmp), scaleToFixedWidth ? "%0.1f" : "%0.3f", scaleToFixedWidth ? scaleAbsolute : scaleRelative);
   SetDlgItemText(IDC_VR_SCALE, tmp);

   constexpr float vrNearPlane = 5.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrNearPlane);
   SetDlgItemText(IDC_NEAR_PLANE, tmp);

   constexpr float vrSlope = 6.5f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrSlope);
   SetDlgItemText(IDC_VR_SLOPE, tmp);

   constexpr float vrOrientation = 0.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrOrientation);
   SetDlgItemText(IDC_3D_VR_ORIENTATION, tmp);

   constexpr float vrX = 0.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrX);
   SetDlgItemText(IDC_VR_OFFSET_X, tmp);

   constexpr float vrY = 0.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrY);
   SetDlgItemText(IDC_VR_OFFSET_Y, tmp);

   constexpr float vrZ = 80.0f;
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrZ);
   SetDlgItemText(IDC_VR_OFFSET_Z, tmp);

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
      SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
      AddToolTip("Disable VR auto-detection, e.g. if Visual Pinball refuses to start up.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_TURN_VR_ON).GetHwnd());
      AddToolTip("What sources should be used for DMD?\nOnly internally supplied via Script/Text Label/Flasher\nScreenreader (see screenreader.ini)\nFrom Shared Memory API", hwndDlg, toolTipHwnd, GetDlgItem(IDC_DMD_SOURCE).GetHwnd());
      AddToolTip("What sources should be used for Backglass?\nOnly internal background\nTry to open a directb2s file\ndirectb2s file dialog\nScreenreader (see screenreader.ini)\nFrom Shared Memory API", hwndDlg, toolTipHwnd, GetDlgItem(IDC_BG_SOURCE).GetHwnd());
      AddToolTip("Select which VR-output/eye(s) to be displayed on the computer screen.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_VR_PREVIEW).GetHwnd());
      AddToolTip("Pixel format for VR Rendering.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd());
      AddToolTip("Attempt to capture an external DMD window such as Freezy/DMDext, UltraDMD or P-ROC.\r\n\r\nFor Freezy/DMDext the DmdDevice.ini needs to set 'stayontop = true'.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_CAP_EXTDMD).GetHwnd());
      AddToolTip("Attempt to capture the PUP player window and display it as a Backglass in VR.", hwndDlg, toolTipHwnd, GetDlgItem(IDC_CAP_PUP).GetHwnd());
   }

   char tmp[256];

   const VRPreviewMode vrPreview = (VRPreviewMode)g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "VRPreview"s, VRPREVIEW_LEFT);
   HWND hwnd = GetDlgItem(IDC_VR_PREVIEW).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Disabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Left Eye");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Right Eye");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM) "Both Eyes");
   SendMessage(hwnd, CB_SETCURSEL, (int)vrPreview, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const bool shrinkToFit = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ShrinkPreview"s, false);
   SendDlgItemMessage(IDC_SHRINK, BM_SETCHECK, shrinkToFit ? BST_CHECKED : BST_UNCHECKED, 0);

   const int vrPreviewWidth = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "PreviewWidth"s, 640);
   sprintf_s(tmp, sizeof(tmp), "%d", vrPreviewWidth);
   SetDlgItemText(IDC_VRPREVIEW_WIDTH, tmp);

   const int vrPreviewHeight = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "PreviewHeight"s, 640);
   sprintf_s(tmp, sizeof(tmp), "%d", vrPreviewHeight);
   SetDlgItemText(IDC_VRPREVIEW_HEIGHT, tmp);

   const bool scaleToFixedWidth = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleToFixedWidth"s, false);
   oldScaleValue = scaleToFixedWidth;
   SendDlgItemMessage(IDC_SCALE_TO_CM, BM_SETCHECK, scaleToFixedWidth ? BST_CHECKED : BST_UNCHECKED, 0);

   scaleRelative = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleRelative"s, 1.0f);
   scaleAbsolute = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "ScaleAbsolute"s, 55.0f);

   sprintf_s(tmp, sizeof(tmp), scaleToFixedWidth ? "%0.1f" : "%0.3f", scaleToFixedWidth ? scaleAbsolute : scaleRelative);
   SetDlgItemText(IDC_VR_SCALE, tmp);

   const float vrNearPlane = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "NearPlane"s, 5.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrNearPlane);
   SetDlgItemText(IDC_NEAR_PLANE, tmp);

   const float vrSlope = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Slope"s, 6.5f);
   sprintf_s(tmp, sizeof(tmp), "%0.2f", vrSlope);
   SetDlgItemText(IDC_VR_SLOPE, tmp);

   const float vrOrientation = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "Orientation"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrOrientation);
   SetDlgItemText(IDC_3D_VR_ORIENTATION, tmp);

   const float vrX = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableX"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrX);
   SetDlgItemText(IDC_VR_OFFSET_X, tmp);

   const float vrY = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableY"s, 0.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrY);
   SetDlgItemText(IDC_VR_OFFSET_Y, tmp);

   const float vrZ = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "TableZ"s, 80.0f);
   sprintf_s(tmp, sizeof(tmp), "%0.1f", vrZ);
   SetDlgItemText(IDC_VR_OFFSET_Z, tmp);

   const int askToTurnOn = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "AskToTurnOn"s, 1);
   hwnd = GetDlgItem(IDC_TURN_VR_ON).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR enabled");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR autodetect");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VR disabled");
   SendMessage(hwnd, CB_SETCURSEL, askToTurnOn, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const int DMDsource = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "DMDSource"s, 1); // Unimplemented for the time being
   hwnd = GetDlgItem(IDC_DMD_SOURCE).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Internal Text/Flasher (via vbscript)");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Screenreader");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"SharedMemory API");
   SendMessage(hwnd, CB_SETCURSEL, DMDsource, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   const int BGsource = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "BGSource"s, 1); // Unimplemented for the time being
   hwnd = GetDlgItem(IDC_BG_SOURCE).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Default table background");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"directb2s File (auto only)");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"directb2s File");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"SharedMemory API");
   SendMessage(hwnd, CB_SETCURSEL, BGsource, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   bool on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "CaptureExternalDMD"s, false);
   SendDlgItemMessage(IDC_CAP_EXTDMD, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "CapturePUP"s, false);
   SendDlgItemMessage(IDC_CAP_PUP, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

   //AMD Debugging
   const int textureModeVR = g_pvp->m_settings.LoadValueWithDefault(Settings::PlayerVR, "EyeFBFormat"s, 1);
   hwnd = GetDlgItem(IDC_COMBO_TEXTURE).GetHwnd();
   SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGB 8");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGBA 8 (Recommended)");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGB 16F");
   SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"RGBA 16F");
   SendMessage(hwnd, CB_SETCURSEL, textureModeVR, 0);
   SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

   int key;
   for (unsigned int i = eTableRecenter; i <= eTableDown; ++i)
      if (regkey_idc[i] != -1)
      {
         const bool hr = g_pvp->m_settings.LoadValue(Settings::Player, regkey_string[i], key);
         if (!hr || key > 0xdd)
            key = regkey_defdik[i];
         const HWND hwndControl = GetDlgItem(regkey_idc[i]);
         ::SetWindowText(hwndControl, rgszKeyName[key]);
         ::SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);
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
   }

   //

   KeyWindowStruct* const pksw = new KeyWindowStruct();
   pksw->pi.Init(GetHwnd());
   pksw->m_timerid = 0;
   ::SetWindowLongPtr(GetHwnd(), GWLP_USERDATA, (size_t)pksw);

   // Set buttons to ignore keyboard shortcuts when using DirectInput
   HWND hwndButton = GetDlgItem(IDC_BTTABLERECENTER).GetHwnd();
   g_ButtonProc2 = (WNDPROC)::GetWindowLongPtr(hwndButton, GWLP_WNDPROC);
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

   hwndButton = GetDlgItem(IDC_BTTABLEUP).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

   hwndButton = GetDlgItem(IDC_BTTABLEDOWN).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, (size_t)MyKeyButtonProc2);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, (size_t)pksw);

   return TRUE;
}

INT_PTR VROptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
   switch (uMsg)
   {
   case WM_TIMER:
   {
      KeyWindowStruct* const pksw = (KeyWindowStruct*)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
      const int key = pksw->pi.GetNextKey();
      if (key != 0)
      {
         if (key < 0xDD) // Key mapping
         {
            int key_esc;
            const bool hr = g_pvp->m_settings.LoadValue(Settings::Player, "EscapeKey"s, key_esc);
            if (!hr || key_esc > 0xdd)
               key_esc = DIK_ESCAPE;

            if (key == key_esc)
            {
               // reset key to old value
               const size_t oldkey = ::GetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA);
               ::SetWindowText(pksw->hwndKeyControl, rgszKeyName[oldkey]);
            }
            else
            {
               ::SetWindowText(pksw->hwndKeyControl, rgszKeyName[key]);
               ::SetWindowLongPtr(pksw->hwndKeyControl, GWLP_USERDATA, key);
            }
            ::KillTimer(GetHwnd(), pksw->m_timerid);
            pksw->m_timerid = 0;
         }
      }
      break;
   }
   }

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
         const bool newScaleValue = IsDlgButtonChecked(IDC_SCALE_TO_CM)> 0;
         if (oldScaleValue != newScaleValue) {
            CString tmpStr = GetDlgItemText(IDC_VR_SCALE);
            tmpStr.Replace(',', '.');
            if (oldScaleValue)
               scaleAbsolute = (float)atof(tmpStr.c_str());
            else
               scaleRelative = (float)atof(tmpStr.c_str());

            char tmp[256];
            sprintf_s(tmp, sizeof(tmp), newScaleValue ? "%0.1f" : "%0.3f", newScaleValue ? scaleAbsolute : scaleRelative);
            SetDlgItemText(IDC_VR_SCALE, tmp);
            oldScaleValue = newScaleValue;
         }
         break;
      }
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
      default:
         return FALSE;
   }
   return TRUE;
}

void VROptionsDialog::OnOK()
{
   const size_t textureModeVR = SendDlgItemMessage(IDC_COMBO_TEXTURE, CB_GETCURSEL, 0, 0);
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "EyeFBFormat"s, (int)textureModeVR);

   LRESULT vrPreview = SendDlgItemMessage(IDC_VR_PREVIEW, CB_GETCURSEL, 0, 0);
   if (vrPreview == LB_ERR)
      vrPreview = VRPREVIEW_LEFT;
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "VRPreview"s, (int)vrPreview);

   const bool shrinkToFit = IsDlgButtonChecked(IDC_SHRINK) != 0;
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "ShrinkPreview"s, shrinkToFit);

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "PreviewWidth"s, GetDlgItemText(IDC_VRPREVIEW_WIDTH).c_str());
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "PreviewHeight"s, GetDlgItemText(IDC_VRPREVIEW_HEIGHT).c_str());

   const bool scaleToFixedWidth = IsDlgButtonChecked(IDC_SCALE_TO_CM)!= 0;
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "ScaleToFixedWidth"s, scaleToFixedWidth);

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, scaleToFixedWidth ? "ScaleAbsolute"s : "ScaleRelative"s, GetDlgItemText(IDC_VR_SCALE).c_str());
   //g_pvp->m_settings.SaveValue(Settings::PlayerVR, scaleToFixedWidth ? "ScaleRelative"s : "ScaleAbsolute"s, scaleToFixedWidth ? scaleRelative : scaleAbsolute); //Also update hidden value?

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "NearPlane"s, GetDlgItemText(IDC_NEAR_PLANE).c_str());

   //For compatibility keep these in Player instead of PlayerVR
   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "Slope"s, GetDlgItemText(IDC_VR_SLOPE).c_str());

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "Orientation"s, GetDlgItemText(IDC_3D_VR_ORIENTATION).c_str());

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "TableX"s, GetDlgItemText(IDC_VR_OFFSET_X).c_str());

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "TableY"s, GetDlgItemText(IDC_VR_OFFSET_Y).c_str());

   g_pvp->m_settings.SaveValue(Settings::PlayerVR, "TableZ"s, GetDlgItemText(IDC_VR_OFFSET_Z).c_str());

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
        const size_t key = ::GetWindowLongPtr(GetDlgItem(regkey_idc[i]).GetHwnd(), GWLP_USERDATA);
        g_pvp->m_settings.SaveValue(Settings::Player, regkey_string[i], (int)key);
    }

   CDialog::OnOK();
}

void VROptionsDialog::OnDestroy()
{
   KeyWindowStruct* const pksw = (KeyWindowStruct*)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
   if (pksw->m_timerid)
   {
      ::KillTimer(GetHwnd(), pksw->m_timerid);
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
   KeyWindowStruct* const pksw = (KeyWindowStruct*)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
   const HWND hwndKeyWindow = GetDlgItem(nID).GetHwnd();
   if (pksw->m_timerid == NULL) //add
   { //add
      // corrects input error with space bar
      const int key = pksw->pi.GetNextKey();
      if (key == 0x39)
      {
         pksw->pi.GetNextKey(); // Clear the current buffer out
         return;
      }

      pksw->pi.GetNextKey(); // Clear the current buffer out

      pksw->m_timerid = ::SetTimer(GetHwnd(), 100, 50, nullptr);
      pksw->hwndKeyControl = hwndKeyWindow;
      ::SetWindowText(pksw->hwndKeyControl, "????");
      while (pksw->pi.GetNextKey() != NULL) //clear entire keyboard buffer contents
      {
         pksw->pi.GetNextKey();
      }
   }
}
