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

    AttachItem(ID_TABLE_MAGNIFY, m_magnifyButton);
    AttachItem(IDC_SELECT, m_selectButton);
    AttachItem(ID_EDIT_PROPERTIES, m_optionsButton);
    AttachItem(ID_EDIT_SCRIPT, m_scriptButton);
    AttachItem(ID_EDIT_BACKGLASSVIEW, m_backglassButton);
    AttachItem(ID_TABLE_PLAY, m_playButton);
    AttachItem(ID_LAYER_LAYER1, m_layerButtons[0]);
    AttachItem(ID_LAYER_LAYER2, m_layerButtons[1]);
    AttachItem(ID_LAYER_LAYER3, m_layerButtons[2]);
    AttachItem(ID_LAYER_LAYER4, m_layerButtons[3]);
    AttachItem(ID_LAYER_LAYER5, m_layerButtons[4]);
    AttachItem(ID_LAYER_LAYER6, m_layerButtons[5]);
    AttachItem(ID_LAYER_LAYER7, m_layerButtons[6]);
    AttachItem(ID_LAYER_LAYER8, m_layerButtons[7]);
    AttachItem(ID_LAYER_LAYER9, m_layerButtons[8]);
    AttachItem(ID_LAYER_LAYER10, m_layerButtons[9]);
    AttachItem(ID_LAYER_LAYER11, m_layerButtons[10]);
    AttachItem(ID_LAYER_TOGGLEALL, m_layerButtons[11]);
    AttachItem(ID_INSERT_WALL, m_wallButton);
    AttachItem(ID_INSERT_GATE, m_gateButton);
    AttachItem(ID_INSERT_RAMP, m_rampButton);
    AttachItem(ID_INSERT_FLIPPER, m_flipperButton);
    AttachItem(ID_INSERT_PLUNGER, m_plungerButton);
    AttachItem(ID_INSERT_BUMPER, m_bumperButton);
    AttachItem(ID_INSERT_SPINNER, m_spinnerButton);
    AttachItem(ID_INSERT_TIMER, m_timerButton);
    AttachItem(ID_INSERT_TRIGGER, m_triggerButton);
    AttachItem(ID_INSERT_LIGHT, m_lightButton);
    AttachItem(ID_INSERT_KICKER, m_kickerButton);
    AttachItem(ID_INSERT_TARGET, m_targetButton);
    AttachItem(ID_INSERT_DECAL, m_decalButton);
    AttachItem(ID_INSERT_TEXTBOX, m_textboxButton);
    AttachItem(ID_INSERT_DISPREEL, m_reelButton);
    AttachItem(ID_INSERT_LIGHTSEQ, m_lightseqButton);
    AttachItem(ID_INSERT_PRIMITIVE, m_primitiveButton);
    AttachItem(ID_INSERT_FLASHER, m_flasherButton);
    AttachItem(ID_INSERT_RUBBER, m_rubberButton);

    const int iconSize = 24;
    HANDLE hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MAGNIFY), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_magnifyButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SELECT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_selectButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_OPTIONS), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_optionsButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SCRIPT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_scriptButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BACKGLASS), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_backglassButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_playButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WALL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_wallButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_GATE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_gateButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RAMP), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_rampButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FLIPPER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_flipperButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PLUNGER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_plungerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_BUMPER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_bumperButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_SPINNER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_spinnerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TIMER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_timerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRIGGER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_triggerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LIGHT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_lightButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_KICKER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_kickerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TARGET), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_targetButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DECAL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_decalButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TEXTBOX), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_textboxButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_REEL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_reelButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_LIGHTSEQ), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_lightseqButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PRIMITIVE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_primitiveButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FLASHER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_flasherButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RUBBER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_rubberButton.SetIcon((HICON)hIcon);

    m_resizer.Initialize(*this, CRect(0, 0, 61, 422));

    m_resizer.AddChild(m_magnifyButton, topleft, 0);
    m_resizer.AddChild(m_selectButton, topright, 0);
    m_resizer.AddChild(m_optionsButton, leftcenter, 0);
    m_resizer.AddChild(m_scriptButton, rightcenter, 0);
    m_resizer.AddChild(m_backglassButton, leftcenter, 0);
    m_resizer.AddChild(m_playButton, rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC_LAYERS), topleft, RD_STRETCH_WIDTH|RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_layerButtons[0], leftcenter, 0);
    m_resizer.AddChild(m_layerButtons[1], center, 0);
    m_resizer.AddChild(m_layerButtons[2], rightcenter, 0);
    m_resizer.AddChild(m_layerButtons[3], leftcenter, 0);
    m_resizer.AddChild(m_layerButtons[4], center, 0);
    m_resizer.AddChild(m_layerButtons[5], rightcenter, 0);
    m_resizer.AddChild(m_layerButtons[6], leftcenter, 0);
    m_resizer.AddChild(m_layerButtons[7], center, 0);
    m_resizer.AddChild(m_layerButtons[8], rightcenter, 0);
    m_resizer.AddChild(m_layerButtons[9], leftcenter, 0);
    m_resizer.AddChild(m_layerButtons[10], center, 0);
    m_resizer.AddChild(m_layerButtons[11], rightcenter, 0);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC_ELEMENTS), bottomright, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);

    m_resizer.AddChild(m_wallButton, leftcenter, 0);
    m_resizer.AddChild(m_gateButton, rightcenter, 0);
    m_resizer.AddChild(m_rampButton, leftcenter, 0);
    m_resizer.AddChild(m_flipperButton, rightcenter, 0);
    m_resizer.AddChild(m_plungerButton, leftcenter, 0);
    m_resizer.AddChild(m_bumperButton, rightcenter, 0);
    m_resizer.AddChild(m_spinnerButton, leftcenter, 0);
    m_resizer.AddChild(m_timerButton, rightcenter, 0);
    m_resizer.AddChild(m_triggerButton, leftcenter, 0);
    m_resizer.AddChild(m_lightButton, rightcenter, 0);
    m_resizer.AddChild(m_kickerButton, leftcenter, 0);
    m_resizer.AddChild(m_targetButton, rightcenter, 0);
    m_resizer.AddChild(m_decalButton, leftcenter, 0);
    m_resizer.AddChild(m_textboxButton, rightcenter, 0);
    m_resizer.AddChild(m_reelButton, leftcenter, 0);
    m_resizer.AddChild(m_lightseqButton, rightcenter, 0);
    m_resizer.AddChild(m_primitiveButton, leftcenter, 0);
    m_resizer.AddChild(m_flasherButton, rightcenter, 0);
    m_resizer.AddChild(m_rubberButton, leftcenter, 0);
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
