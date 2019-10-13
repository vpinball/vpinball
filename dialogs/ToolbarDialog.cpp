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
    
    const int iconSize = 24;
    HANDLE hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAGNIFY), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_TABLE_MAGNIFY), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SELECT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(IDC_SELECT), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_OPTIONS), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_EDIT_PROPERTIES), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SCRIPT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_EDIT_SCRIPT), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BACKGLASS), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_EDIT_BACKGLASSVIEW), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_TABLE_PLAY), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WALL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_WALL), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_GATE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_GATE), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RAMP), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_RAMP), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FLIPPER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_FLIPPER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PLUNGER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_PLUNGER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BUMPER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_BUMPER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SPINNER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_SPINNER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TIMER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_TIMER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRIGGER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_TRIGGER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LIGHT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_LIGHT), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TARGET), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_TARGET), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PRIMITIVE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_PRIMITIVE), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FLASHER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_FLASHER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RUBBER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_RUBBER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

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
