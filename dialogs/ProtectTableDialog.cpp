#include "StdAfx.h"
#include "resource.h"
#include "ProtectTableDialog.h"

ProtectTableDialog::ProtectTableDialog() : CDialog(IDD_PROTECT_DIALOG)
{
}

BOOL ProtectTableDialog::OnInitDialog()
{
    // limit the password fields to 16 characters (or PROT_PASSWORD_LENGTH)
    HWND hwndPassword = GetDlgItem(IDC_PROTECT_PASSWORD).GetHwnd();
    ::SendMessage(hwndPassword, EM_LIMITTEXT, PROT_PASSWORD_LENGTH, 0L);
    hwndPassword = GetDlgItem(IDC_PROTECT_PASSWORD2).GetHwnd();
    ::SendMessage(hwndPassword, EM_LIMITTEXT, PROT_PASSWORD_LENGTH, 0L);
    return TRUE;
}

BOOL ProtectTableDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (LOWORD(wParam))
    {
        case IDC_PROTECT_TOTALLOCK:
        {
            // if the total lock check box is checked then disable any other options
            const size_t checked = SendDlgItemMessage(IDC_PROTECT_TOTALLOCK, BM_GETCHECK, 0, 0);

            HWND hwndScript = GetDlgItem(IDC_PROTECT_SCRIPT).GetHwnd();
            HWND hwndSaveAs = GetDlgItem(IDC_PROTECT_SAVEAS).GetHwnd();
            HWND hwndSaveAsProt = GetDlgItem(IDC_PROTECT_SAVEASPROT).GetHwnd();
            HWND hwndManagers = GetDlgItem(IDC_PROTECT_MANAGERS).GetHwnd();
            HWND hwndCopy = GetDlgItem(IDC_PROTECT_COPY).GetHwnd();
            HWND hwndView = GetDlgItem(IDC_PROTECT_VIEWTABLE).GetHwnd();
            HWND hwndDebugger = GetDlgItem(IDC_PROTECT_DEBUGGER).GetHwnd();

            const int checkstate = !(checked == BST_CHECKED);

            ::EnableWindow(hwndScript, checkstate);
            ::EnableWindow(hwndSaveAs, checkstate);
            ::EnableWindow(hwndSaveAsProt, checkstate);
            ::EnableWindow(hwndManagers, checkstate);
            ::EnableWindow(hwndCopy, checkstate);
            ::EnableWindow(hwndView, checkstate);
            ::EnableWindow(hwndDebugger, checkstate);
            break;
        }
        case IDD_PROTECT_SHOWPASSWORD:
        {
            HWND hwndPassword = GetDlgItem(IDC_PROTECT_PASSWORD).GetHwnd();
            HWND hwndPassword2 = GetDlgItem(IDC_PROTECT_PASSWORD2).GetHwnd();

            const size_t checked = SendDlgItemMessage(IDD_PROTECT_SHOWPASSWORD, BM_GETCHECK, 0, 0);
            if (checked == BST_CHECKED)
            {
                ::SendMessage(hwndPassword, EM_SETPASSWORDCHAR, 0, 0L);
                ::SendMessage(hwndPassword2, EM_SETPASSWORDCHAR, 0, 0L);
            }
            else
            {
                ::SendMessage(hwndPassword, EM_SETPASSWORDCHAR, '*', 0L);
                ::SendMessage(hwndPassword2, EM_SETPASSWORDCHAR, '*', 0L);
            }
            ::InvalidateRect(hwndPassword, NULL, FALSE);
            ::InvalidateRect(hwndPassword2, NULL, FALSE);
            break;
        }
        default:
            return FALSE;
    }
    return TRUE;
}

void ProtectTableDialog::OnOK()
{
    BOOL fail = fFalse;

    // get the check box status(s)
    unsigned long flags = 0;
    const size_t checked1 = SendDlgItemMessage(IDC_PROTECT_SAVEAS, BM_GETCHECK, 0, 0);
    if (checked1 == BST_CHECKED) flags |= DISABLE_TABLE_SAVE;
    const size_t checked2 = SendDlgItemMessage(IDC_PROTECT_SAVEASPROT, BM_GETCHECK, 0, 0);
    if (checked2 == BST_CHECKED) flags |= DISABLE_TABLE_SAVEPROT;
    const size_t checked3 = SendDlgItemMessage(IDC_PROTECT_SCRIPT, BM_GETCHECK, 0, 0);
    if (checked3 == BST_CHECKED) flags |= DISABLE_SCRIPT_EDITING;
    const size_t checked4 = SendDlgItemMessage(IDC_PROTECT_MANAGERS, BM_GETCHECK, 0, 0);
    if (checked4 == BST_CHECKED) flags |= DISABLE_OPEN_MANAGERS;
    const size_t checked5 = SendDlgItemMessage(IDC_PROTECT_COPY, BM_GETCHECK, 0, 0);
    if (checked5 == BST_CHECKED) flags |= DISABLE_CUTCOPYPASTE;
    const size_t checked6 = SendDlgItemMessage(IDC_PROTECT_VIEWTABLE, BM_GETCHECK, 0, 0);
    if (checked6 == BST_CHECKED) flags |= DISABLE_TABLEVIEW;
    const size_t checked7 = SendDlgItemMessage(IDC_PROTECT_DEBUGGER, BM_GETCHECK, 0, 0);
    if (checked7 == BST_CHECKED) flags |= DISABLE_DEBUGGER;
    const size_t checked0 = SendDlgItemMessage(IDC_PROTECT_TOTALLOCK, BM_GETCHECK, 0, 0);
    if (checked0 == BST_CHECKED) flags |= DISABLE_EVERYTHING;

    // get the passwords
    char pw[PROT_PASSWORD_LENGTH + 1];
    ZeroMemory(pw, sizeof(pw));
    HWND hwndPw = GetDlgItem(IDC_PROTECT_PASSWORD).GetHwnd();
    ::GetWindowText(hwndPw, pw, sizeof(pw));

    char pw2[PROT_PASSWORD_LENGTH + 2];
    ZeroMemory(pw2, sizeof(pw2));
    HWND hwndPw2 = GetDlgItem(IDC_PROTECT_PASSWORD2).GetHwnd();
    ::GetWindowText(hwndPw2, pw2, sizeof(pw2));

    // is there at least one box checked?? (flags must contain at least 1 protection bit)
    if (flags == 0)
    {
        LocalString ls(IDS_PROTECT_ONETICKED);
        MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
        fail = fTrue;
    }
    else
    {
        // if both strings are empty then bomb out
        if ((pw[0] == '\0') && (pw2[0] == '\0'))
        {
            LocalString ls(IDS_PROTECT_PW_ZEROLEN);
            MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
            fail = fTrue;
        }
        else
        {
            // do both strings match?
            if (strcmp(pw, pw2) != 0)
            {
                LocalString ls(IDS_PROTECT_PW_MISMATCH);
                MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
                fail = fTrue;
            }
        }
    }

    // has anything failed the sanity check?
    if (!fail)
    {
        // nope.. lets get started
        PinTable *pt = g_pvp->GetActiveTable();
        BOOL rc = pt->SetupProtectionBlock((unsigned char *)pw, flags);
        CDialog::OnOK();
    }

}

UnprotectDialog::UnprotectDialog() : CDialog(IDD_UNLOCK_DIALOG)
{
}

BOOL UnprotectDialog::OnInitDialog()
{
    // limit the password fields to 16 characters (or PROT_PASSWORD_LENGTH)
    HWND hwndPassword = GetDlgItem(IDC_UNLOCK_PASSWORD).GetHwnd();
    ::SendMessage(hwndPassword, EM_LIMITTEXT, PROT_PASSWORD_LENGTH, 0L);
    return TRUE;
}

void UnprotectDialog::OnOK()
{
    // get the password
    char pw[PROT_PASSWORD_LENGTH + 1];
    ZeroMemory(pw, sizeof(pw));
    HWND hwndPw = GetDlgItem(IDC_UNLOCK_PASSWORD).GetHwnd();
    ::GetWindowText(hwndPw, pw, sizeof(pw));

    // if both password is empty bring up a message box
    if (pw[0] == '\0')
    {
        LocalString ls(IDS_PROTECT_PW_ZEROLEN);
        MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
    }
    else
    {
        PinTable * const pt = g_pvp->GetActiveTable();
        if (pt)
        {
            const BOOL rc = pt->UnlockProtectionBlock((unsigned char *)pw);
            if (rc)
            {
                LocalString ls(IDS_UNLOCK_SUCCESS);
                MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONINFORMATION);
            }
            else
            {
                LocalString ls(IDS_UNLOCK_FAILED);
                MessageBox(ls.m_szbuffer, "Visual Pinball", MB_ICONWARNING);
            }
        }
    }
    CDialog::OnOK();
}
