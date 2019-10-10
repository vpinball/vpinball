#include "stdafx.h"
#include "ToolbarDialog.h"
#include <WindowsX.h>

ToolbarDialog::ToolbarDialog() : CDialog(IDD_TOOLBAR)
{
}

ToolbarDialog::~ToolbarDialog()
{
}

void ToolbarDialog::OnDestroy()
{
}

BOOL ToolbarDialog::OnInitDialog()
{
    m_hwnd = GetDlgItem(IDD_TOOLBAR).GetHwnd();

    SendDlgItemMessage(IDC_SELECT, BM_SETCHECK, BST_CHECKED, 0);
    return TRUE;
}

BOOL ToolbarDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int id = LOWORD(wParam);

    switch (id)
    {
        case IDC_SELECT:
        case ID_TABLE_MAGNIFY:
        {
            SendDlgItemMessage(IDC_SELECT, BM_SETCHECK, BST_UNCHECKED, 0);
            SendDlgItemMessage(ID_TABLE_MAGNIFY, BM_SETCHECK, BST_UNCHECKED, 0);
            switch (HIWORD(wParam))
            {
                case BN_CLICKED:
                {
                    if (SendDlgItemMessage(id, BM_GETCHECK, 0, 0))
                        g_pvp->m_ToolCur = id;
                    else
                        SendDlgItemMessage(id, BM_SETCHECK, BST_CHECKED, 0);

                    return TRUE;
                }
                default:
                    break;
            }
            break;
        }
    }
    return FALSE;
}

