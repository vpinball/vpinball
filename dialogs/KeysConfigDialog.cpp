#include "stdafx.h"
#include "resource.h"
#include "KeysConfigDialog.h"

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

    "Num Enter",	//#define DIK_NUMPADENTER     0x9C    /* Enter on numeric keypad */
    "R Ctrl",		//DIK_RCONTROL        0x9D

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

WNDPROC g_ButtonProc;

LRESULT CALLBACK MyKeyButtonProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_GETDLGCODE)
        // Eat all acceleratable messages
        return (DLGC_WANTARROWS | DLGC_WANTTAB | DLGC_WANTALLKEYS | DLGC_WANTCHARS);
    else
        return CallWindowProc(g_ButtonProc, hwnd, uMsg, wParam, lParam);
}


KeysConfigDialog::KeysConfigDialog() : CDialog(IDD_KEYS)
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
         rgszKeyName[scancode][0] = (char) result[0];
         rgszKeyName[scancode][1] = 0;
      }
   }
}

void KeysConfigDialog::AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { 0 };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = text;
   SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void KeysConfigDialog::AddStringDOF(const string &name, const int idc)
{
   const int selected = g_pvp->m_settings.LoadValueWithDefault(Settings::Controller, name, 2); // assume both as standard
   const HWND hwnd = GetDlgItem(idc).GetHwnd();
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Sound FX");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"DOF");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Both");
   ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
}

void KeysConfigDialog::AddStringAxis(const string &name, const int idc, const int def)
{
   const int selected = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, name, def);
   const HWND hwnd = GetDlgItem(idc).GetHwnd();
   ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"(disabled)");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"X Axis");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Y Axis");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Z Axis");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"rX Axis");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"rY Axis");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"rZ Axis");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Slider 1");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Slider 2");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"OpenPinDev");
   ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
   ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
}

void KeysConfigDialog::AddJoyCustomKey(const string &name, const int idc, const int default_key)
{
   int key;
   const bool hr = g_pvp->m_settings.LoadValue(Settings::Player, name, key);
   if (!hr || key > 0xdd)
      key = default_key;
   const HWND hwndControl = GetDlgItem(idc).GetHwnd();
   ::SetWindowText(hwndControl, rgszKeyName[key]);
   ::SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);
}

void KeysConfigDialog::AddWndProc(const int idc, const size_t MyKeyButtonProc, const size_t pksw)
{
   const HWND hwndButton = GetDlgItem(idc).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, MyKeyButtonProc);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, pksw);
}

BOOL KeysConfigDialog::OnInitDialog()
{
    bool on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWDefaultLayout"s, false);
    SendDlgItemMessage(IDC_DefaultLayout, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "DisableESC"s, false);
    SendDlgItemMessage(IDC_DisableESC_CB, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false);
    SendDlgItemMessage(IDC_CBGLOBALROTATION, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    int key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 0);
    SetDlgItemInt( IDC_GLOBALROTATION, key, FALSE);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "AccelVelocityInput"s, false);
    SendDlgItemMessage(IDC_CBGLOBALACCVEL, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensCB"s, false);
    SendDlgItemMessage(IDC_CBGLOBALTILT, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensValue"s, 400);
    SetDlgItemInt( IDC_GLOBALTILT, key, FALSE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "DeadZone"s, 0);
    SetDlgItemInt( IDC_DEADZONEAMT, key, FALSE);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWEnabled"s, false);
    SendDlgItemMessage(IDC_GLOBALACCEL, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWNormalMount"s, false);
    SendDlgItemMessage(IDC_GLOBALNMOUNT, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "ReversePlungerAxis"s, false);
    SendDlgItemMessage(IDC_ReversePlunger, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlungerRetract"s, false);
    SendDlgItemMessage(IDC_PLUNGERRETRACT, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "LRAxisFlip"s, false);
    SendDlgItemMessage(IDC_LRAXISFLIP, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "UDAxisFlip"s, false);
    SendDlgItemMessage(IDC_UDAXISFLIP, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainX"s, 150);
    SetDlgItemInt( IDC_LRAXISGAIN, key, FALSE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150);
    SetDlgItemInt( IDC_UDAXISGAIN, key, FALSE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100);
    SetDlgItemInt( IDC_XMAX_EDIT, key, FALSE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100);
    SetDlgItemInt( IDC_YMAX_EDIT, key, FALSE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedScale"s, 100);
    SetDlgItemInt(IDC_PLUNGERSPEEDSCALE, key, FALSE);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableMouseInPlayer"s, true);
    SendDlgItemMessage(IDC_ENABLE_MOUSE_PLAYER, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableCameraModeFlyAround"s, false);
    SendDlgItemMessage(IDC_ENABLE_CAMERA_FLY_AROUND, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, false);
    SendDlgItemMessage(IDC_ENABLE_NUDGE_FILTER, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyNudge"s, false);
    SendDlgItemMessage(IDC_ENABLE_LEGACY_NUDGE, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    const float legacyNudgeStrength = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "LegacyNudgeStrength"s, 1.f);
    SetDlgItemInt(IDC_LEGACY_NUDGE_STRENGTH, quantizeUnsignedPercent(legacyNudgeStrength), FALSE);

    for (unsigned int i = 0; i <= 34; ++i)
    {
        bool hr;
        int item,selected;
        switch (i)
        {
            case 0: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyLFlipKey"s, selected); item = IDC_JOYLFLIPCOMBO; break;
            case 1: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyRFlipKey"s, selected); item = IDC_JOYRFLIPCOMBO; break;
            case 31:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyStagedLFlipKey"s, selected); item = IDC_JOYSTAGEDLFLIPCOMBO; break;
            case 32:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyStagedRFlipKey"s, selected); item = IDC_JOYSTAGEDRFLIPCOMBO; break;
            case 2: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPlungerKey"s, selected); item = IDC_JOYPLUNGERCOMBO; break;
            case 3: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyAddCreditKey"s, selected); item = IDC_JOYADDCREDITCOMBO; break;
            case 4: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyAddCredit2Key"s, selected); item = IDC_JOYADDCREDIT2COMBO; break;
            case 5: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyLMagnaSave"s, selected); item = IDC_JOYLMAGNACOMBO; break;
            case 6: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyRMagnaSave"s, selected); item = IDC_JOYRMAGNACOMBO; break;
            case 7: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyStartGameKey"s, selected); item = IDC_JOYSTARTCOMBO; break;
            case 8: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyExitGameKey"s, selected); item = IDC_JOYEXITCOMBO; break;
            case 9: hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyFrameCount"s, selected); item = IDC_JOYFPSCOMBO; break;
            case 10:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyVolumeUp"s, selected); item = IDC_JOYVOLUPCOMBO; break;
            case 11:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyVolumeDown"s, selected); item = IDC_JOYVOLDNCOMBO; break;
            case 12:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyLTiltKey"s, selected); item = IDC_JOYLTILTCOMBO; break;
            case 13:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyCTiltKey"s, selected); item = IDC_JOYCTILTCOMBO; break;
            case 14:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyRTiltKey"s, selected); item = IDC_JOYRTILTCOMBO; break;
            case 15:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyMechTiltKey"s, selected); item = IDC_JOYMECHTILTCOMBO; break;
            case 16:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyDebugKey"s, selected); item = IDC_JOYDEBUGCOMBO; break;
            case 17:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyDebuggerKey"s, selected); item = IDC_JOYDEBUGGERCOMBO; break;
            case 18:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyCustom1"s, selected); item = IDC_JOYCUSTOM1COMBO; break;
            case 19:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyCustom2"s, selected); item = IDC_JOYCUSTOM2COMBO; break;
            case 20:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyCustom3"s, selected); item = IDC_JOYCUSTOM3COMBO; break;
            case 21:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyCustom4"s, selected); item = IDC_JOYCUSTOM4COMBO; break;
            case 22:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMBuyIn"s, selected); item = IDC_JOYPMBUYIN; break;
            case 23:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMCoin3"s, selected); item = IDC_JOYPMCOIN3; break;
            case 24:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMCoin4"s, selected); item = IDC_JOYPMCOIN4; break;
            case 25:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMCoinDoor"s, selected); item = IDC_JOYPMCOINDOOR; break;
            case 26:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMCancel"s, selected); item = IDC_JOYPMCANCEL; break;
            case 27:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMDown"s, selected); item = IDC_JOYPMDOWN; break;
            case 28:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMUp"s, selected); item = IDC_JOYPMUP; break;
            case 29:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPMEnter"s, selected); item = IDC_JOYPMENTER; break;
            case 30:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyLockbarKey"s, selected); item = IDC_JOYLOCKBARCOMBO; break;
            case 33:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyPauseKey"s, selected); item = IDC_JOYPAUSECOMBO; break;
            case 34:hr = g_pvp->m_settings.LoadValue(Settings::Player, "JoyTweakKey"s, selected); item = IDC_JOYTWEAKCOMBO; break;
        }

        if (!hr)
            selected = 0; // assume no assignment as standard

        const HWND hwnd = GetDlgItem(item).GetHwnd();
        ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0); // to speed up adding the entries :/
        ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"(none)");
        for (int j = 1; j <= 24; ++j)
           ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)("Button "+std::to_string(j)).c_str());
        if (item == IDC_JOYLFLIPCOMBO || item == IDC_JOYRFLIPCOMBO || item == IDC_JOYPLUNGERCOMBO || item == IDC_JOYLTILTCOMBO || item == IDC_JOYCTILTCOMBO || item == IDC_JOYRTILTCOMBO)
        {
            ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Left Mouse");
            ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Right Mouse");
            ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Middle Mouse");
        }
        ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
        ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
    }

    //

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Controller, "ForceDisableB2S"s, false);
    SendDlgItemMessage(IDC_DOF_FORCEDISABLE, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    AddStringDOF("DOFContactors"s, IDC_DOF_CONTACTORS);
    AddStringDOF("DOFKnocker"s, IDC_DOF_KNOCKER);
    AddStringDOF("DOFChimes"s, IDC_DOF_CHIMES);
    AddStringDOF("DOFBell"s, IDC_DOF_BELL);
    AddStringDOF("DOFGear"s, IDC_DOF_GEAR);
    AddStringDOF("DOFShaker"s, IDC_DOF_SHAKER);
    AddStringDOF("DOFFlippers"s, IDC_DOF_FLIPPERS);
    AddStringDOF("DOFTargets"s, IDC_DOF_TARGETS);
    AddStringDOF("DOFDropTargets"s, IDC_DOF_DROPTARGETS);

    //

    AddStringAxis("PlungerAxis"s, IDC_PLUNGERAXIS, 3); // assume Z Axis as standard
    AddStringAxis("LRAxis"s, IDC_LRAXISCOMBO, 1); // assume X Axis as standard
    AddStringAxis("UDAxis"s, IDC_UDAXISCOMBO, 2); // assume Y Axis as standard
    AddStringAxis("PlungerSpeedAxis"s, IDC_PLUNGERSPEEDAXIS, 0);  // not assigned by default

    for (unsigned int i = 0; i < eCKeys; ++i)
      if (regkey_idc[i] != -1 && GetDlgItem(regkey_idc[i]) && GetDlgItem(regkey_idc[i]).IsWindow())
      {
         const bool hr = g_pvp->m_settings.LoadValue(Settings::Player, regkey_string[i], key);
         if (!hr || key > 0xdd)
            key = regkey_defdik[i];
         const HWND hwndControl = GetDlgItem(regkey_idc[i]).GetHwnd();
         ::SetWindowText(hwndControl, rgszKeyName[key]);
         ::SetWindowLongPtr(hwndControl, GWLP_USERDATA, key);
      }

    AddJoyCustomKey("JoyCustom1Key"s, IDC_JOYCUSTOM1, DIK_UP);
    AddJoyCustomKey("JoyCustom2Key"s, IDC_JOYCUSTOM2, DIK_DOWN);
    AddJoyCustomKey("JoyCustom3Key"s, IDC_JOYCUSTOM3, DIK_LEFT);
    AddJoyCustomKey("JoyCustom4Key"s, IDC_JOYCUSTOM4, DIK_RIGHT);

    //

    KeyWindowStruct * const pksw = new KeyWindowStruct();
    pksw->pi.Init(GetHwnd());
    pksw->m_timerid = 0;
    ::SetWindowLongPtr(GetHwnd(), GWLP_USERDATA, (size_t)pksw);

    // Set buttons to ignore keyboard shortcuts when using DirectInput
    HWND hwndButton = GetDlgItem(IDC_LEFTFLIPPERBUTTON).GetHwnd();
    g_ButtonProc = (WNDPROC)::GetWindowLongPtr(hwndButton, GWLP_WNDPROC);

    AddWndProc(IDC_LEFTFLIPPERBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_RIGHTFLIPPERBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_STAGEDLEFTFLIPPERBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_STAGEDRIGHTFLIPPERBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_LEFTTILTBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_RIGHTTILTBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_CENTERTILTBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_PLUNGERBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_ADDCREDITBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_ADDCREDITBUTTON2, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_STARTGAMEBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_EXITGAMEBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_FRAMECOUNTBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_VOLUPBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_VOLDOWNBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_LOCKBARBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_DEBUGBALLSBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_DEBUGGERBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_RMAGSAVEBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_LMAGSAVEBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_MECHTILTBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_JOYCUSTOM1BUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_JOYCUSTOM2BUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_JOYCUSTOM3BUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_JOYCUSTOM4BUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_PAUSEBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);
    AddWndProc(IDC_TWEAKBUTTON, (size_t)MyKeyButtonProc, (size_t)pksw);

    int inputApi = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "InputApi"s, 0);
    int inputApiIndex = inputApi;
    const HWND hwndInputApi = GetDlgItem(IDC_COMBO_INPUT_API).GetHwnd();

    int inputApiCount = 1;
    ::SendMessage(hwndInputApi, CB_ADDSTRING, 0, (LPARAM)"Direct Input");//0
#ifdef ENABLE_XINPUT
    ::SendMessage(hwndInputApi, CB_ADDSTRING, 0, (LPARAM)"XInput");//1
    inputApiCount++;
#else
    if (inputApi == 1) inputApiIndex = 0;
    if (inputApi > 1) inputApiIndex--;
#endif
#ifdef ENABLE_SDL_INPUT
    ::SendMessage(hwndInputApi, CB_ADDSTRING, 0, (LPARAM)"SDL");//2
    inputApiCount++;
#else
    if (inputApi == 2) inputApiIndex = 0;
    if (inputApi > 2) inputApiIndex--;
#endif
#ifdef ENABLE_IGAMECONTROLLER
    ::SendMessage(hwndInputApi, CB_ADDSTRING, 0, (LPARAM)"IGameController");//3
    inputApiCount++;
#else
    if (inputApi == 3) inputApiIndex = 0;
    if (inputApi > 3) inputApiIndex--;
#endif
#ifdef ENABLE_VRCONTROLLER
    ::SendMessage(hwndInputApi, CB_ADDSTRING, 0, (LPARAM)"VR Controller");//4
    inputApiCount++;
#else
    if (inputApi == 4) inputApiIndex = 0;
    if (inputApi > 4) inputApiIndex--;
#endif
    ::SendMessage(hwndInputApi, CB_SETCURSEL, inputApiIndex, 0);

    GetDlgItem(IDC_COMBO_RUMBLE).EnableWindow(inputApiCount > 1);

    const int rumbleMode = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "RumbleMode"s, 3);
    const HWND hwndRumble = GetDlgItem(IDC_COMBO_RUMBLE).GetHwnd();
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Off");
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Table only (N/A yet)"); //!! not supported yet
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Generic only (N/A yet)"); //!! not supported yet
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Table with generic fallback");
    ::SendMessage(hwndRumble, CB_SETCURSEL, rumbleMode, 0);

    return TRUE;
}

INT_PTR KeysConfigDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_TIMER:
        {
            KeyWindowStruct * const pksw = (KeyWindowStruct *)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
            const int key = pksw->pi.GetNextKey();
            if (key != 0)
            {
                if (key < 0xDD)	// Key mapping
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

BOOL KeysConfigDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    if (HIWORD(wParam) == BN_CLICKED)
    {
        switch (LOWORD(wParam))
        {
            case IDC_LEFTFLIPPERBUTTON: StartTimer(IDC_LEFTFLIPPER); break;
            case IDC_RIGHTFLIPPERBUTTON: StartTimer(IDC_RIGHTFLIPPER); break;
            case IDC_STAGEDLEFTFLIPPERBUTTON: StartTimer(IDC_STAGEDLEFTFLIPPER); break;
            case IDC_STAGEDRIGHTFLIPPERBUTTON: StartTimer(IDC_STAGEDRIGHTFLIPPER); break;
            case IDC_LEFTTILTBUTTON: StartTimer(IDC_LEFTTILT); break;
            case IDC_RIGHTTILTBUTTON: StartTimer(IDC_RIGHTTILT); break;
            case IDC_CENTERTILTBUTTON: StartTimer(IDC_CENTERTILT); break;
            case IDC_PLUNGERBUTTON: StartTimer(IDC_PLUNGER_TEXT); break;
            case IDC_ADDCREDITBUTTON: StartTimer(IDC_ADDCREDIT); break;
            case IDC_ADDCREDITBUTTON2: StartTimer(IDC_ADDCREDIT2); break;
            case IDC_STARTGAMEBUTTON: StartTimer(IDC_STARTGAME); break;
            case IDC_EXITGAMEBUTTON: StartTimer(IDC_EXITGAME); break;
            case IDC_FRAMECOUNTBUTTON: StartTimer(IDC_FRAMECOUNT); break;
            case IDC_DEBUGBALLSBUTTON: StartTimer(IDC_DEBUGBALL); break;
            case IDC_DEBUGGERBUTTON: StartTimer(IDC_DEBUGGER); break;
            case IDC_VOLUPBUTTON: StartTimer(IDC_VOLUMEUP); break;
            case IDC_VOLDOWNBUTTON: StartTimer(IDC_VOLUMEDN); break;
            case IDC_LOCKBARBUTTON: StartTimer(IDC_LOCKBAR); break;
            case IDC_RMAGSAVEBUTTON: StartTimer(IDC_RMAGSAVE); break;
            case IDC_LMAGSAVEBUTTON: StartTimer(IDC_LMAGSAVE); break;
            case IDC_MECHTILTBUTTON: StartTimer(IDC_MECHTILT); break;
            case IDC_JOYCUSTOM1BUTTON: StartTimer(IDC_JOYCUSTOM1); break;
            case IDC_JOYCUSTOM2BUTTON: StartTimer(IDC_JOYCUSTOM2); break;
            case IDC_JOYCUSTOM3BUTTON: StartTimer(IDC_JOYCUSTOM3); break;
            case IDC_JOYCUSTOM4BUTTON: StartTimer(IDC_JOYCUSTOM4); break;
            case IDC_PAUSEBUTTON: StartTimer(IDC_PAUSE); break;
            case IDC_TWEAKBUTTON: StartTimer(IDC_TWEAK); break;
            default:
                return FALSE;
        }//switch
    }//if (HIWORD(wParam) == BN_CLICKED)

    if (LOWORD(wParam) == IDC_COMBO_INPUT_API) {
       const size_t inputApi = SendDlgItemMessage(IDC_COMBO_INPUT_API, CB_GETCURSEL, 0, 0);
       GetDlgItem(IDC_COMBO_RUMBLE).EnableWindow(inputApi > 0);
    }

    return TRUE;
}

void KeysConfigDialog::OnOK()
{
    SetValue(IDC_JOYCUSTOM1COMBO, Settings::Player, "JoyCustom1"s);
    SetValue(IDC_JOYCUSTOM2COMBO, Settings::Player, "JoyCustom2"s);
    SetValue(IDC_JOYCUSTOM3COMBO, Settings::Player, "JoyCustom3"s);
    SetValue(IDC_JOYCUSTOM4COMBO, Settings::Player, "JoyCustom4"s);
    SetValue(IDC_JOYPMBUYIN, Settings::Player, "JoyPMBuyIn"s);
    SetValue(IDC_JOYPMCOIN3, Settings::Player, "JoyPMCoin3"s);
    SetValue(IDC_JOYPMCOIN4, Settings::Player, "JoyPMCoin4"s);
    SetValue(IDC_JOYPMCOINDOOR, Settings::Player, "JoyPMCoinDoor"s);
    SetValue(IDC_JOYPMCANCEL, Settings::Player, "JoyPMCancel"s);
    SetValue(IDC_JOYPMDOWN, Settings::Player, "JoyPMDown"s);
    SetValue(IDC_JOYPMUP, Settings::Player, "JoyPMUp"s);
    SetValue(IDC_JOYPMENTER, Settings::Player, "JoyPMEnter"s);
    SetValue(IDC_JOYLFLIPCOMBO, Settings::Player, "JoyLFlipKey"s);
    SetValue(IDC_JOYRFLIPCOMBO, Settings::Player, "JoyRFlipKey"s);
    SetValue(IDC_JOYSTAGEDLFLIPCOMBO, Settings::Player, "JoyStagedLFlipKey"s);
    SetValue(IDC_JOYSTAGEDRFLIPCOMBO, Settings::Player, "JoyStagedRFlipKey"s);
    SetValue(IDC_JOYPLUNGERCOMBO, Settings::Player, "JoyPlungerKey"s);
    SetValue(IDC_JOYADDCREDITCOMBO, Settings::Player, "JoyAddCreditKey"s);
    SetValue(IDC_JOYADDCREDIT2COMBO, Settings::Player, "JoyAddCredit2Key"s);
    SetValue(IDC_JOYLMAGNACOMBO, Settings::Player, "JoyLMagnaSave"s);
    SetValue(IDC_JOYRMAGNACOMBO, Settings::Player, "JoyRMagnaSave"s);
    SetValue(IDC_JOYSTARTCOMBO, Settings::Player, "JoyStartGameKey"s);
    SetValue(IDC_JOYEXITCOMBO, Settings::Player, "JoyExitGameKey"s);
    SetValue(IDC_JOYFPSCOMBO, Settings::Player, "JoyFrameCount"s);
    SetValue(IDC_JOYVOLUPCOMBO, Settings::Player, "JoyVolumeUp"s);
    SetValue(IDC_JOYVOLDNCOMBO, Settings::Player, "JoyVolumeDown"s);
    SetValue(IDC_JOYLTILTCOMBO, Settings::Player, "JoyLTiltKey"s);
    SetValue(IDC_JOYCTILTCOMBO, Settings::Player, "JoyCTiltKey"s);
    SetValue(IDC_JOYRTILTCOMBO, Settings::Player, "JoyRTiltKey"s);
    SetValue(IDC_JOYMECHTILTCOMBO, Settings::Player, "JoyMechTiltKey"s);
    SetValue(IDC_JOYDEBUGCOMBO, Settings::Player, "JoyDebugKey"s);
    SetValue(IDC_JOYDEBUGGERCOMBO, Settings::Player, "JoyDebuggerKey"s);
    SetValue(IDC_JOYLOCKBARCOMBO, Settings::Player, "JoyLockbarKey"s);
    SetValue(IDC_PLUNGERAXIS, Settings::Player, "PlungerAxis"s);
    SetValue(IDC_PLUNGERSPEEDAXIS, Settings::Player, "PlungerSpeedAxis"s);
    SetValue(IDC_LRAXISCOMBO, Settings::Player, "LRAxis"s);
    SetValue(IDC_UDAXISCOMBO, Settings::Player, "UDAxis"s);
    SetValue(IDC_JOYPAUSECOMBO, Settings::Player, "JoyPauseKey"s);
    SetValue(IDC_JOYTWEAKCOMBO, Settings::Player, "JoyTweakKey"s);

    size_t selected;
    int newvalue;
    BOOL nothing;

    newvalue = max((int)GetDlgItemInt(IDC_LRAXISGAIN, nothing, TRUE), 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainX"s, newvalue);

    newvalue = max((int)GetDlgItemInt(IDC_UDAXISGAIN, nothing, TRUE), 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelGainY"s, newvalue);

    newvalue = max((int)GetDlgItemInt(IDC_PLUNGERSPEEDSCALE, nothing, TRUE), 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "PlungerSpeedScale"s, newvalue);

    newvalue = clamp((int)GetDlgItemInt(IDC_DEADZONEAMT, nothing, TRUE), 0, 100);
    g_pvp->m_settings.SaveValue(Settings::Player, "DeadZone"s, newvalue);

    newvalue = clamp((int)GetDlgItemInt(IDC_XMAX_EDIT, nothing, TRUE), 0, 100);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxX"s, newvalue);

    newvalue = clamp((int)GetDlgItemInt(IDC_YMAX_EDIT, nothing, TRUE), 0, 100);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWAccelMaxY"s, newvalue);

    selected = IsDlgButtonChecked(IDC_DefaultLayout);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWDefaultLayout"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_DisableESC_CB);
    g_pvp->m_settings.SaveValue(Settings::Player, "DisableESC"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_LRAXISFLIP);
    g_pvp->m_settings.SaveValue(Settings::Player, "LRAxisFlip"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_UDAXISFLIP);
    g_pvp->m_settings.SaveValue(Settings::Player, "UDAxisFlip"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_ReversePlunger);
    g_pvp->m_settings.SaveValue(Settings::Player, "ReversePlungerAxis"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_PLUNGERRETRACT);
    g_pvp->m_settings.SaveValue(Settings::Player, "PlungerRetract"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_GLOBALACCEL);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWEnabled"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_GLOBALNMOUNT);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWNormalMount"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_CBGLOBALROTATION);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationCB"s, selected != 0);

    newvalue = GetDlgItemInt(IDC_GLOBALROTATION, nothing, TRUE);
    g_pvp->m_settings.SaveValue(Settings::Player, "PBWRotationValue"s, newvalue);

    selected = IsDlgButtonChecked(IDC_CBGLOBALACCVEL);
    g_pvp->m_settings.SaveValue(Settings::Player, "AccelVelocityInput"s, selected != 0);

    const bool tscb = (IsDlgButtonChecked(IDC_CBGLOBALTILT) != 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensCB"s, tscb);

    newvalue = clamp((int)GetDlgItemInt(IDC_GLOBALTILT, nothing, TRUE), 0, 1000);
    g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensValue"s, newvalue);
    if (tscb)
        g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensitivity"s, newvalue);
    else
        g_pvp->m_settings.DeleteValue(Settings::Player, "TiltSensitivity"s);

    for (unsigned int i = 0; i < eCKeys; ++i) if (regkey_idc[i] != -1)
    {
        if (i == eTableRecenter || i == eTableUp || i == eTableDown)
           continue;
        const size_t key = ::GetWindowLongPtr(GetDlgItem(regkey_idc[i]).GetHwnd(), GWLP_USERDATA);
        g_pvp->m_settings.SaveValue(Settings::Player, regkey_string[i], (int)key);
    }

    size_t key = ::GetWindowLongPtr(GetDlgItem(IDC_JOYCUSTOM1).GetHwnd(), GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom1Key"s, (int)key);
    key = ::GetWindowLongPtr(GetDlgItem(IDC_JOYCUSTOM2).GetHwnd(), GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom2Key"s, (int)key);
    key = ::GetWindowLongPtr(GetDlgItem(IDC_JOYCUSTOM3).GetHwnd(), GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom3Key"s, (int)key);
    key = ::GetWindowLongPtr(GetDlgItem(IDC_JOYCUSTOM4).GetHwnd(), GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom4Key"s, (int)key);

    SetValue(IDC_DOF_CONTACTORS, Settings::Controller, "DOFContactors"s);
    SetValue(IDC_DOF_KNOCKER, Settings::Controller, "DOFKnocker"s);
    SetValue(IDC_DOF_CHIMES, Settings::Controller, "DOFChimes"s);
    SetValue(IDC_DOF_BELL, Settings::Controller, "DOFBell"s);
    SetValue(IDC_DOF_GEAR, Settings::Controller, "DOFGear"s);
    SetValue(IDC_DOF_SHAKER, Settings::Controller, "DOFShaker"s);
    SetValue(IDC_DOF_FLIPPERS, Settings::Controller, "DOFFlippers"s);
    SetValue(IDC_DOF_TARGETS, Settings::Controller, "DOFTargets"s);
    SetValue(IDC_DOF_DROPTARGETS, Settings::Controller, "DOFDropTargets"s);

    selected = IsDlgButtonChecked(IDC_ENABLE_NUDGE_FILTER);
    g_pvp->m_settings.SaveValue(Settings::Player, "EnableNudgeFilter"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_ENABLE_LEGACY_NUDGE);
    g_pvp->m_settings.SaveValue(Settings::Player, "EnableLegacyNudge"s, selected != 0);

    newvalue = GetDlgItemInt(IDC_LEGACY_NUDGE_STRENGTH, nothing, TRUE);
    g_pvp->m_settings.SaveValue(Settings::Player, "LegacyNudgeStrength"s, dequantizeUnsignedPercent((unsigned int)newvalue));

    selected = IsDlgButtonChecked(IDC_ENABLE_MOUSE_PLAYER);
    g_pvp->m_settings.SaveValue(Settings::Player, "EnableMouseInPlayer"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_ENABLE_CAMERA_FLY_AROUND);
    g_pvp->m_settings.SaveValue(Settings::Player, "EnableCameraModeFlyAround"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_DOF_FORCEDISABLE);
    g_pvp->m_settings.SaveValue(Settings::Controller, "ForceDisableB2S"s, selected != 0);

    int inputApi = (int)SendDlgItemMessage(IDC_COMBO_INPUT_API, CB_GETCURSEL, 0, 0);
#ifndef ENABLE_XINPUT
    if (inputApi >= 1) inputApi++;
#endif
#ifndef ENABLE_SDL_INPUT
    if (inputApi >= 2) inputApi++;
#endif
#ifndef ENABLE_IGAMECONTROLLER
    if (inputApi >= 3) inputApi++;
#endif
#ifndef ENABLE_VRCONTROLLER
    if (inputApi >= 4) inputApi++;
#endif
    g_pvp->m_settings.SaveValue(Settings::Player, "InputApi"s, inputApi);

    const int rumble = (int)SendDlgItemMessage(IDC_COMBO_RUMBLE, CB_GETCURSEL, 0, 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "RumbleMode"s, rumble);

    g_pvp->m_settings.Save();

    CDialog::OnOK();
}

void KeysConfigDialog::OnDestroy()
{
    KeyWindowStruct * const pksw = (KeyWindowStruct *)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
    if (pksw->m_timerid)
    {
        ::KillTimer(GetHwnd(), pksw->m_timerid);
        pksw->m_timerid = 0;
    }
    pksw->pi.UnInit();
    CDialog::OnDestroy();
}

HWND KeysConfigDialog::GetItemHwnd(int nID)
{
    return GetDlgItem(nID).GetHwnd();
}

void KeysConfigDialog::SetValue(int nID, const Settings::Section& section, const string& key)
{
    LRESULT selected = SendDlgItemMessage(nID, CB_GETCURSEL, 0, 0);
    if (selected == LB_ERR)
        selected = 2; // assume both as standard
    g_pvp->m_settings.SaveValue(section, key, (int)selected);
}

void KeysConfigDialog::StartTimer(int nID)
{
    KeyWindowStruct * const pksw = (KeyWindowStruct *)::GetWindowLongPtr(GetHwnd(), GWLP_USERDATA);
    const HWND hwndKeyWindow = GetItemHwnd(nID);
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
