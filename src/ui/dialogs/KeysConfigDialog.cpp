// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "KeysConfigDialog.h"
#include "InputDeviceDialog.h"

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

static WNDPROC g_ButtonProc;

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
}

void KeysConfigDialog::AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
   TOOLINFO toolInfo = { 0 };
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = text;
   ::SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
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

void KeysConfigDialog::AddJoyCustomKey(const string &name, const int idc, const int default_dik)
{
   int dik = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, name, default_dik);
   const HWND hwndControl = GetDlgItem(idc).GetHwnd();
   ::SetWindowLongPtr(hwndControl, GWLP_USERDATA, dik);
   SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode(GetSDLScancodeFromDirectInputKey(dik), SDL_KMOD_NONE, false);
   ::SetWindowText(hwndControl, SDL_GetKeyName(sdlKeycode));
}

void KeysConfigDialog::AddWndProc(const int idc, const size_t MyKeyButtonProc, const size_t pksw)
{
   const HWND hwndButton = GetDlgItem(idc).GetHwnd();
   ::SetWindowLongPtr(hwndButton, GWLP_WNDPROC, MyKeyButtonProc);
   ::SetWindowLongPtr(hwndButton, GWLP_USERDATA, pksw);
}

BOOL KeysConfigDialog::OnInitDialog()
{
    bool on = g_pvp->m_settings.LoadValueBool(Settings::Player, "PBWDefaultLayout"s);
    SendDlgItemMessage(IDC_DefaultLayout, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueBool(Settings::Player, "DisableESC"s);
    SendDlgItemMessage(IDC_DisableESC_CB, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationCB"s, false);
    SendDlgItemMessage(IDC_CBGLOBALROTATION, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    int key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWRotationValue"s, 0);
    SetDlgItemInt(IDC_GLOBALROTATION, key, TRUE);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "AccelVelocityInput"s, false);
    SendDlgItemMessage(IDC_CBGLOBALACCVEL, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensCB"s, false);
    SendDlgItemMessage(IDC_CBGLOBALTILT, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TiltSensValue"s, 400);
    SetDlgItemInt(IDC_GLOBALTILT, key, TRUE);

    const float fv = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "TiltInertia"s, 100.f);
    SetDlgItemText(IDC_TILT_INERTIA, f2sz(fv).c_str());

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "DeadZone"s, 0);
    SetDlgItemInt(IDC_DEADZONEAMT, key, TRUE);

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
    SetDlgItemInt(IDC_LRAXISGAIN, key, TRUE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelGainY"s, 150);
    SetDlgItemInt(IDC_UDAXISGAIN, key, TRUE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxX"s, 100);
    SetDlgItemInt(IDC_XMAX_EDIT, key, TRUE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PBWAccelMaxY"s, 100);
    SetDlgItemInt(IDC_YMAX_EDIT, key, TRUE);

    key = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "PlungerSpeedScale"s, 100);
    SetDlgItemInt(IDC_PLUNGERSPEEDSCALE, key, TRUE);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableMouseInPlayer"s, true);
    SendDlgItemMessage(IDC_ENABLE_MOUSE_PLAYER, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s);
    SendDlgItemMessage(IDC_ENABLE_CAMERA_FLY_AROUND, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableNudgeFilter"s, false);
    SendDlgItemMessage(IDC_ENABLE_NUDGE_FILTER, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "EnableLegacyNudge"s, false);
    SendDlgItemMessage(IDC_ENABLE_LEGACY_NUDGE, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    const float legacyNudgeStrength = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "LegacyNudgeStrength"s, 1.f);
    SetDlgItemInt(IDC_LEGACY_NUDGE_STRENGTH, quantizeUnsignedPercent(legacyNudgeStrength), FALSE);

    SendDlgItemMessage(IDC_DEVICES_BUTTON, BM_SETSTYLE, BS_PUSHBUTTON, 0);

    for (unsigned int i = 0; i <= 34; ++i)
    {
        bool hr = true;
        int item,selected;
        switch (i)
        {
            case 0: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyLFlipKey"s); item = IDC_JOYLFLIPCOMBO; break;
            case 1: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyRFlipKey"s); item = IDC_JOYRFLIPCOMBO; break;
            case 31:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyStagedLFlipKey"s); item = IDC_JOYSTAGEDLFLIPCOMBO; break;
            case 32:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyStagedRFlipKey"s); item = IDC_JOYSTAGEDRFLIPCOMBO; break;
            case 2: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyPlungerKey"s); item = IDC_JOYPLUNGERCOMBO; break;
            case 3: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyAddCreditKey"s); item = IDC_JOYADDCREDITCOMBO; break;
            case 4: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyAddCredit2Key"s); item = IDC_JOYADDCREDIT2COMBO; break;
            case 5: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyLMagnaSave"s); item = IDC_JOYLMAGNACOMBO; break;
            case 6: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyRMagnaSave"s); item = IDC_JOYRMAGNACOMBO; break;
            case 7: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyStartGameKey"s); item = IDC_JOYSTARTCOMBO; break;
            case 8: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyExitGameKey"s); item = IDC_JOYEXITCOMBO; break;
            case 9: selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyFrameCount"s); item = IDC_JOYFPSCOMBO; break;
            case 10:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyVolumeUp"s); item = IDC_JOYVOLUPCOMBO; break;
            case 11:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyVolumeDown"s); item = IDC_JOYVOLDNCOMBO; break;
            case 12:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyLTiltKey"s); item = IDC_JOYLTILTCOMBO; break;
            case 13:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyCTiltKey"s); item = IDC_JOYCTILTCOMBO; break;
            case 14:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyRTiltKey"s); item = IDC_JOYRTILTCOMBO; break;
            case 15:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyMechTiltKey"s); item = IDC_JOYMECHTILTCOMBO; break;
            case 16:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyDebugKey"s); item = IDC_JOYDEBUGCOMBO; break;
            case 17:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyDebuggerKey"s); item = IDC_JOYDEBUGGERCOMBO; break;
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
            case 30:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyLockbarKey"s); item = IDC_JOYLOCKBARCOMBO; break;
            case 33:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyPauseKey"s); item = IDC_JOYPAUSECOMBO; break;
            case 34:selected = g_pvp->m_settings.LoadValueInt(Settings::Player, "JoyTweakKey"s); item = IDC_JOYTWEAKCOMBO; break;
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
    AddStringAxis("PlungerSpeedAxis"s, IDC_PLUNGERSPEEDAXIS, 0); // not assigned by default

    for (unsigned int i = 0; i < eCKeys; ++i)
      if (regkey_idc[i] != -1 && GetDlgItem(regkey_idc[i]) && GetDlgItem(regkey_idc[i]).IsWindow())
      {
         const int dik = g_pvp->m_settings.LoadValueInt(Settings::Player, regkey_string[i]);
         const HWND hwndControl = GetDlgItem(regkey_idc[i]).GetHwnd();
         ::SetWindowLongPtr(hwndControl, GWLP_USERDATA, dik);
         SDL_Keycode sdlKeycode = SDL_GetKeyFromScancode(GetSDLScancodeFromDirectInputKey(dik), SDL_KMOD_NONE, false);
         ::SetWindowText(hwndControl, SDL_GetKeyName(sdlKeycode));
      }

    AddJoyCustomKey("JoyCustom1Key"s, IDC_JOYCUSTOM1, DIK_UP);
    AddJoyCustomKey("JoyCustom2Key"s, IDC_JOYCUSTOM2, DIK_DOWN);
    AddJoyCustomKey("JoyCustom3Key"s, IDC_JOYCUSTOM3, DIK_LEFT);
    AddJoyCustomKey("JoyCustom4Key"s, IDC_JOYCUSTOM4, DIK_RIGHT);

    //

    KeyWindowStruct * const pksw = new KeyWindowStruct();
    pksw->pi.SetFocusWindow(GetHwnd());
    pksw->pi.Init();
    pksw->m_timerid = 0;
    SetWindowLongPtr(GWLP_USERDATA, (size_t)pksw);

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
    ::SendMessage(hwndInputApi, CB_ADDSTRING, 0, (LPARAM)"SDL");//2
    inputApiCount++;
    ::SendMessage(hwndInputApi, CB_SETCURSEL, inputApiIndex, 0);

    GetDlgItem(IDC_COMBO_RUMBLE).EnableWindow(inputApiCount > 1);

    const int rumbleMode = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "RumbleMode"s, 3);
    const HWND hwndRumble = GetDlgItem(IDC_COMBO_RUMBLE).GetHwnd();
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Off");
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Table only (N/A yet)"); //!! not supported yet
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Generic only (N/A yet)"); //!! not supported yet
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Table with generic fallback");
    ::SendMessage(hwndRumble, CB_SETCURSEL, rumbleMode, 0);

    OnCommand(IDC_GLOBALACCEL, 0L);
    OnCommand(IDC_ENABLE_LEGACY_NUDGE, 0L);
    
    return TRUE;
}

INT_PTR KeysConfigDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_TIMER:
        {
            KeyWindowStruct * const pksw = (KeyWindowStruct *)GetWindowLongPtr(GWLP_USERDATA);
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
    return DialogProcDefault(uMsg, wParam, lParam);
}

BOOL KeysConfigDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
   UNREFERENCED_PARAMETER(lParam);
   switch (LOWORD(wParam))
   {
   case IDC_GLOBALACCEL:
   {
      const size_t checked = IsDlgButtonChecked(IDC_GLOBALACCEL);
      GetDlgItem(IDC_LRAXISCOMBO).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_LRAXISFLIP).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_XMAX_EDIT).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_LRAXISGAIN).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_UDAXISCOMBO).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_UDAXISFLIP).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_YMAX_EDIT).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_UDAXISGAIN).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_GLOBALNMOUNT).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_CBGLOBALROTATION).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_GLOBALROTATION).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_CBGLOBALACCVEL).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_CBGLOBALTILT).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_GLOBALTILT).EnableWindow(checked ? TRUE : FALSE);
      GetDlgItem(IDC_TILT_INERTIA).EnableWindow(checked ? TRUE : FALSE);
      break;
   }
   case IDC_ENABLE_LEGACY_NUDGE:
   {
      const size_t checked = IsDlgButtonChecked(IDC_ENABLE_LEGACY_NUDGE);
      GetDlgItem(IDC_LEGACY_NUDGE_STRENGTH).EnableWindow(checked ? TRUE : FALSE);
      break;
   }
   case IDC_DEVICES_BUTTON:
   {
      KeyWindowStruct *const pksw = (KeyWindowStruct *)GetWindowLongPtr(GWLP_USERDATA);
      if (pksw->pi.GetDirectInputJoystickHandler() != nullptr)
      {
         CRect pos = GetWindowRect();
         InputDeviceDialog *const deviceConfigDlg = new InputDeviceDialog(&pos, &pksw->pi);
         deviceConfigDlg->DoModal();
         delete deviceConfigDlg;
      }
      break;
   }
   }

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
       GetDlgItem(IDC_COMBO_RUMBLE).EnableWindow(inputApi > 0); // No rumble for DirectInput
       GetDlgItem(IDC_DEVICES_BUTTON).EnableWindow(inputApi == 0); // Manage device is only available for DirectInput
       KeyWindowStruct *const pksw = (KeyWindowStruct *)GetWindowLongPtr(GWLP_USERDATA);
       pksw->pi.ReInit(); // Reinit on API change to have access to the underlying controllers
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

    newvalue = IsDlgButtonChecked(IDC_CBGLOBALACCVEL);
    g_pvp->m_settings.SaveValue(Settings::Player, "AccelVelocityInput"s, newvalue);

    const bool tscb = (IsDlgButtonChecked(IDC_CBGLOBALTILT) != 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensCB"s, tscb);

    newvalue = clamp((int)GetDlgItemInt(IDC_GLOBALTILT, nothing, TRUE), 0, 1000);
    g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensValue"s, newvalue);
    if (tscb)
        g_pvp->m_settings.SaveValue(Settings::Player, "TiltSensitivity"s, newvalue);
    else
        g_pvp->m_settings.DeleteValue(Settings::Player, "TiltSensitivity"s);

    float tiltInertia = clamp(sz2f(GetDlgItemText(IDC_TILT_INERTIA).GetString()), 0.f, 1000.f);
    g_pvp->m_settings.SaveValue(Settings::Player, "TiltInertia"s, tiltInertia);

   for (unsigned int i = 0; i < eCKeys; ++i) if (regkey_idc[i] != -1)
      if (regkey_idc[i] != -1 && GetDlgItem(regkey_idc[i]) && GetDlgItem(regkey_idc[i]).IsWindow())
      {
         const size_t dik = GetDlgItem(regkey_idc[i]).GetWindowLongPtr(GWLP_USERDATA);
         g_pvp->m_settings.SaveValue(Settings::Player, regkey_string[i], (int)dik);
      }

    size_t dik = GetDlgItem(IDC_JOYCUSTOM1).GetWindowLongPtr(GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom1Key"s, (int)dik);
    dik = GetDlgItem(IDC_JOYCUSTOM2).GetWindowLongPtr(GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom2Key"s, (int)dik);
    dik = GetDlgItem(IDC_JOYCUSTOM3).GetWindowLongPtr(GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom3Key"s, (int)dik);
    dik = GetDlgItem(IDC_JOYCUSTOM4).GetWindowLongPtr(GWLP_USERDATA);
    g_pvp->m_settings.SaveValue(Settings::Player, "JoyCustom4Key"s, (int)dik);

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

    newvalue = GetDlgItemInt(IDC_LEGACY_NUDGE_STRENGTH, nothing, FALSE);
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
    g_pvp->m_settings.SaveValue(Settings::Player, "InputApi"s, inputApi);

    const int rumble = (int)SendDlgItemMessage(IDC_COMBO_RUMBLE, CB_GETCURSEL, 0, 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "RumbleMode"s, rumble);

    g_pvp->m_settings.Save();

    CDialog::OnOK();
}

void KeysConfigDialog::OnDestroy()
{
    KeyWindowStruct *const pksw = (KeyWindowStruct *)GetWindowLongPtr(GWLP_USERDATA);
    if (pksw->m_timerid)
    {
        KillTimer(pksw->m_timerid);
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
    KeyWindowStruct *const pksw = (KeyWindowStruct *)GetWindowLongPtr(GWLP_USERDATA);
    const HWND hwndKeyWindow = GetItemHwnd(nID);
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
