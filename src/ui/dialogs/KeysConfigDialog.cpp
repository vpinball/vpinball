// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "KeysConfigDialog.h"

#include "input/ScanCodes.h"

KeysConfigDialog::KeysConfigDialog() : CDialog(IDD_KEYS)
{
}

void KeysConfigDialog::AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd) const
{
   TOOLINFO toolInfo = {};
   toolInfo.cbSize = sizeof(toolInfo);
   toolInfo.hwnd = parentHwnd;
   toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
   toolInfo.uId = (UINT_PTR)controlHwnd;
   toolInfo.lpszText = text;
   ::SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

void KeysConfigDialog::AddStringDOF(const string &name, const int idc) const
{
   const int selected = g_pvp->m_settings.LoadValueWithDefault(Settings::Controller, name, 2); // assume both as standard
   const HWND hwnd = GetDlgItem(idc).GetHwnd();
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Sound FX");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"DOF");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Both");
   ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
}

void KeysConfigDialog::AddStringAxis(const string &name, const int idc, const int def) const
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

BOOL KeysConfigDialog::OnInitDialog()
{
    bool on = g_pvp->m_settings.LoadValueBool(Settings::Player, "EnableCameraModeFlyAround"s);
    SendDlgItemMessage(IDC_ENABLE_CAMERA_FLY_AROUND, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

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
    const int rumbleMode = g_pvp->m_settings.LoadValueWithDefault(Settings::Player, "RumbleMode"s, 3);
    const HWND hwndRumble = GetDlgItem(IDC_COMBO_RUMBLE).GetHwnd();
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Off");
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Table only (N/A yet)"); //!! not supported yet
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Generic only (N/A yet)"); //!! not supported yet
    ::SendMessage(hwndRumble, CB_ADDSTRING, 0, (LPARAM)"Table with generic fallback");
    ::SendMessage(hwndRumble, CB_SETCURSEL, rumbleMode, 0);

    return TRUE;
}

void KeysConfigDialog::OnOK()
{
    size_t selected;

    SetValue(IDC_DOF_CONTACTORS, Settings::Controller, "DOFContactors"s);
    SetValue(IDC_DOF_KNOCKER, Settings::Controller, "DOFKnocker"s);
    SetValue(IDC_DOF_CHIMES, Settings::Controller, "DOFChimes"s);
    SetValue(IDC_DOF_BELL, Settings::Controller, "DOFBell"s);
    SetValue(IDC_DOF_GEAR, Settings::Controller, "DOFGear"s);
    SetValue(IDC_DOF_SHAKER, Settings::Controller, "DOFShaker"s);
    SetValue(IDC_DOF_FLIPPERS, Settings::Controller, "DOFFlippers"s);
    SetValue(IDC_DOF_TARGETS, Settings::Controller, "DOFTargets"s);
    SetValue(IDC_DOF_DROPTARGETS, Settings::Controller, "DOFDropTargets"s);

    selected = IsDlgButtonChecked(IDC_ENABLE_CAMERA_FLY_AROUND);
    g_pvp->m_settings.SaveValue(Settings::Player, "EnableCameraModeFlyAround"s, selected != 0);

    selected = IsDlgButtonChecked(IDC_DOF_FORCEDISABLE);
    g_pvp->m_settings.SaveValue(Settings::Controller, "ForceDisableB2S"s, selected != 0);

    const int rumble = (int)SendDlgItemMessage(IDC_COMBO_RUMBLE, CB_GETCURSEL, 0, 0);
    g_pvp->m_settings.SaveValue(Settings::Player, "RumbleMode"s, rumble);

    g_pvp->m_settings.Save();

    CDialog::OnOK();
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

