// license:GPLv3+

#include "core/stdafx.h"
#include "ui/resource.h"
#include "KeysConfigDialog.h"

#include "input/ScanCodes.h"

KeysConfigDialog::KeysConfigDialog() : CDialog(IDD_KEYS)
{
}

void KeysConfigDialog::AddStringDOF(const string &name, const int idc) const
{
   const int selected = g_pvp->m_settings.GetInt(Settings::GetRegistry().GetPropertyId("Controller"s, name).value());
   const HWND hwnd = GetDlgItem(idc).GetHwnd();
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Sound FX");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"DOF");
   ::SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Both");
   ::SendMessage(hwnd, CB_SETCURSEL, selected, 0);
}

void KeysConfigDialog::SetValue(int nID, const string& name)
{
    LRESULT selected = SendDlgItemMessage(nID, CB_GETCURSEL, 0, 0);
    if (selected == LB_ERR)
        selected = 2; // assume both as standard
    g_pvp->m_settings.Set(Settings::GetRegistry().GetPropertyId("Controller"s, name).value(), (int)selected, false);
}

BOOL KeysConfigDialog::OnInitDialog()
{
    bool on = g_pvp->m_settings.GetPlayer_EnableCameraModeFlyAround();
    SendDlgItemMessage(IDC_ENABLE_CAMERA_FLY_AROUND, BM_SETCHECK, on ? BST_CHECKED : BST_UNCHECKED, 0);

    on = g_pvp->m_settings.GetController_ForceDisableB2S();
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

    const int rumbleMode = g_pvp->m_settings.GetPlayer_RumbleMode();
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
    SetValue(IDC_DOF_CONTACTORS, "DOFContactors"s);
    SetValue(IDC_DOF_KNOCKER, "DOFKnocker"s);
    SetValue(IDC_DOF_CHIMES, "DOFChimes"s);
    SetValue(IDC_DOF_BELL, "DOFBell"s);
    SetValue(IDC_DOF_GEAR, "DOFGear"s);
    SetValue(IDC_DOF_SHAKER, "DOFShaker"s);
    SetValue(IDC_DOF_FLIPPERS, "DOFFlippers"s);
    SetValue(IDC_DOF_TARGETS, "DOFTargets"s);
    SetValue(IDC_DOF_DROPTARGETS, "DOFDropTargets"s);

    size_t selected = IsDlgButtonChecked(IDC_ENABLE_CAMERA_FLY_AROUND);
    g_pvp->m_settings.SetPlayer_EnableCameraModeFlyAround(selected != 0, false);

    selected = IsDlgButtonChecked(IDC_DOF_FORCEDISABLE);
    g_pvp->m_settings.SetController_ForceDisableB2S(selected != 0, false);

    const int rumble = (int)SendDlgItemMessage(IDC_COMBO_RUMBLE, CB_GETCURSEL, 0, 0);
    g_pvp->m_settings.SetPlayer_RumbleMode(rumble, false);

    g_pvp->m_settings.Save();

    CDialog::OnOK();
}
