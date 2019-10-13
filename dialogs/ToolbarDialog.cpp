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

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_KICKER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_KICKER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TARGET), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_TARGET), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DECAL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_DECAL), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TEXTBOX), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_TEXTBOX), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_REEL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_DISPREEL), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LIGHTSEQ), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_LIGHTSEQ), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PRIMITIVE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_PRIMITIVE), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FLASHER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_FLASHER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RUBBER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    ::SendMessage(GetDlgItem(ID_INSERT_RUBBER), BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

    m_resizer.Initialize(*this, CRect(0, 0, 61, 422));

    m_resizer.AddChild(GetDlgItem(ID_TABLE_MAGNIFY), topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_SELECT), topright, 0);
    m_resizer.AddChild(GetDlgItem(ID_EDIT_PROPERTIES), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_EDIT_SCRIPT), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_EDIT_BACKGLASSVIEW), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_TABLE_PLAY), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC_LAYERS), topleft, RD_STRETCH_WIDTH|RD_STRETCH_HEIGHT);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER1), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER2), center, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER3), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER4), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER5), center, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER6), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER7), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER8), center, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER9), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER10), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_LAYER11), center, 0);
    m_resizer.AddChild(GetDlgItem(ID_LAYER_TOGGLEALL), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC_ELEMENTS), bottomright, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);

    m_resizer.AddChild(GetDlgItem(ID_INSERT_WALL), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_GATE), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_RAMP), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_FLIPPER), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_PLUNGER), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_BUMPER), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_SPINNER), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_TIMER), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_TRIGGER), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_LIGHT), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_KICKER), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_TARGET), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_DECAL), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_TEXTBOX), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_DISPREEL), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_LIGHTSEQ), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_PRIMITIVE), leftcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_FLASHER), rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(ID_INSERT_RUBBER), leftcenter, 0);
    m_resizer.RecalcLayout();
    return TRUE;
}

INT_PTR ToolbarDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Pass resizing messages on to the resizer
    m_resizer.HandleMessage(msg, wparam, lparam);

//  switch (msg)
//  {
        //Additional messages to be handled go here
//  }

    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);
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
