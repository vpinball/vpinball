#include "stdafx.h"
#include "ToolbarDialog.h"
#include <WindowsX.h>

ToolbarDialog::ToolbarDialog() : CDialog(IDD_TOOLBAR)
{
}

ToolbarDialog::~ToolbarDialog()
{
}

LRESULT ToolbarDialog::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
// Respond to a mouse click on the window
{
    // Set window focus. The docker will now report this as active.
    if (!IsChild(::GetFocus()))
        SetFocus();

    return FinalWindowProc(msg, wparam, lparam);
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

    m_tooltip.Create(*this);
    m_tooltip.AddTool(m_magnifyButton, _T("Zoom in/out"));
    m_tooltip.AddTool(m_selectButton, _T("Select element"));
    m_tooltip.AddTool(m_optionsButton, _T("Toggle properties"));
    m_tooltip.AddTool(m_scriptButton, _T("Toggle script editor"));
    m_tooltip.AddTool(m_backglassButton, _T("Toggle backglass view"));
    m_tooltip.AddTool(m_playButton, _T("Play table"));

    m_tooltip.AddTool(m_wallButton, _T("Insert Wall"));
    m_tooltip.AddTool(m_gateButton, _T("Insert Gate"));
    m_tooltip.AddTool(m_rampButton, _T("Insert Ramp"));
    m_tooltip.AddTool(m_flipperButton, _T("Insert Flipper"));
    m_tooltip.AddTool(m_plungerButton, _T("Insert Plunger"));
    m_tooltip.AddTool(m_bumperButton, _T("Insert Bumper"));
    m_tooltip.AddTool(m_spinnerButton, _T("Insert Spinner"));
    m_tooltip.AddTool(m_timerButton, _T("Insert Timer"));
    m_tooltip.AddTool(m_triggerButton, _T("Insert Trigger"));
    m_tooltip.AddTool(m_lightButton, _T("Insert Light"));
    m_tooltip.AddTool(m_kickerButton, _T("Insert Kicker"));
    m_tooltip.AddTool(m_targetButton, _T("Insert Target"));
    m_tooltip.AddTool(m_decalButton, _T("Insert Decal"));
    m_tooltip.AddTool(m_textboxButton, _T("Insert Textbox"));
    m_tooltip.AddTool(m_reelButton, _T("Insert Reel"));
    m_tooltip.AddTool(m_lightseqButton, _T("Insert Light Sequence"));
    m_tooltip.AddTool(m_primitiveButton, _T("Insert Primitive"));
    m_tooltip.AddTool(m_flasherButton, _T("Insert Flasher"));
    m_tooltip.AddTool(m_rubberButton, _T("Insert Rubber"));

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

    for (int i = 0; i < 11; i++)
    {
        m_layerButtons[i].SetCheck(BST_CHECKED);
    }
/*
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
*/
    EnableButtons();
    return TRUE;
}

INT_PTR ToolbarDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Pass resizing messages on to the resizer
//    m_resizer.HandleMessage(msg, wparam, lparam);

    switch (msg)
    {
        case WM_MOUSEACTIVATE:      return OnMouseActivate(msg, wparam, lparam);
    }

    // Pass unhandled messages on to parent DialogProc
    return DialogProcDefault(msg, wparam, lparam);
}

void ToolbarDialog::EnableButtons()
{
    if (!IsWindow())
        return;
    CComObject<PinTable> * const ptCur = g_pvp->GetActiveTable();
    if (ptCur == NULL && !g_pplayer)
    {
        m_magnifyButton.EnableWindow(FALSE);
        m_selectButton.EnableWindow(FALSE);
        m_optionsButton.EnableWindow(FALSE);
        m_scriptButton.EnableWindow(FALSE);
        m_backglassButton.EnableWindow(FALSE);
        m_playButton.EnableWindow(FALSE);

        m_textboxButton.EnableWindow(FALSE);
        m_reelButton.EnableWindow(FALSE);
        m_wallButton.EnableWindow(FALSE);
        m_gateButton.EnableWindow(FALSE);
        m_rampButton.EnableWindow(FALSE);
        m_flipperButton.EnableWindow(FALSE);
        m_plungerButton.EnableWindow(FALSE);
        m_bumperButton.EnableWindow(FALSE);
        m_spinnerButton.EnableWindow(FALSE);
        m_triggerButton.EnableWindow(FALSE);
        m_kickerButton.EnableWindow(FALSE);
        m_primitiveButton.EnableWindow(FALSE);
        m_flasherButton.EnableWindow(FALSE);
        m_rubberButton.EnableWindow(FALSE);

        m_targetButton.EnableWindow(FALSE);
        m_decalButton.EnableWindow(FALSE);
        m_lightButton.EnableWindow(FALSE);
        m_timerButton.EnableWindow(FALSE);
        m_lightseqButton.EnableWindow(FALSE);
    }
    else if (g_pvp->m_backglassView)
    {
        m_textboxButton.EnableWindow(TRUE);
        m_reelButton.EnableWindow(TRUE);
        m_decalButton.EnableWindow(TRUE);
        m_lightButton.EnableWindow(TRUE);
        m_timerButton.EnableWindow(TRUE);
        m_lightseqButton.EnableWindow(TRUE);

        m_wallButton.EnableWindow(FALSE);
        m_gateButton.EnableWindow(FALSE);
        m_rampButton.EnableWindow(FALSE);
        m_flipperButton.EnableWindow(FALSE);
        m_plungerButton.EnableWindow(FALSE);
        m_bumperButton.EnableWindow(FALSE);
        m_spinnerButton.EnableWindow(FALSE);
        m_triggerButton.EnableWindow(FALSE);
        m_targetButton.EnableWindow(FALSE);
        m_kickerButton.EnableWindow(FALSE);
        m_primitiveButton.EnableWindow(FALSE);
        m_flasherButton.EnableWindow(FALSE);
        m_rubberButton.EnableWindow(FALSE);
    }
    else
    {
        m_magnifyButton.EnableWindow(TRUE);
        m_selectButton.EnableWindow(TRUE);
        m_optionsButton.EnableWindow(TRUE);
        m_scriptButton.EnableWindow(TRUE);
        m_backglassButton.EnableWindow(TRUE);
        m_playButton.EnableWindow(TRUE);

        m_textboxButton.EnableWindow(FALSE);
        m_reelButton.EnableWindow(FALSE);
        m_wallButton.EnableWindow(TRUE);
        m_gateButton.EnableWindow(TRUE);
        m_rampButton.EnableWindow(TRUE);
        m_flipperButton.EnableWindow(TRUE);
        m_plungerButton.EnableWindow(TRUE);
        m_bumperButton.EnableWindow(TRUE);
        m_spinnerButton.EnableWindow(TRUE);
        m_triggerButton.EnableWindow(TRUE);
        m_kickerButton.EnableWindow(TRUE);
        m_primitiveButton.EnableWindow(TRUE);
        m_flasherButton.EnableWindow(TRUE);
        m_rubberButton.EnableWindow(TRUE);
        m_decalButton.EnableWindow(TRUE);
        m_lightButton.EnableWindow(TRUE);
        m_timerButton.EnableWindow(TRUE);
        m_lightseqButton.EnableWindow(TRUE);
        m_targetButton.EnableWindow(TRUE);
    }
}

void ToolbarDialog::SetOptionsButton(const bool checked)
{
    if(checked)
        m_optionsButton.SetCheck(BST_CHECKED);
    else
        m_optionsButton.SetCheck(BST_UNCHECKED);

}

BOOL ToolbarDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    const int id = LOWORD(wParam);

    switch (id)
    {
        case ID_INSERT_WALL:
        case ID_INSERT_GATE:
        case ID_INSERT_RAMP:
        case ID_INSERT_FLIPPER:
        case ID_INSERT_PLUNGER:
        case ID_INSERT_BUMPER:
        case ID_INSERT_SPINNER:
        case ID_INSERT_TIMER:
        case ID_INSERT_TRIGGER:
        case ID_INSERT_LIGHT:
        case ID_INSERT_KICKER:
        case ID_INSERT_TARGET:
        case ID_INSERT_DECAL:
        case ID_INSERT_TEXTBOX:
        case ID_INSERT_DISPREEL:
        case ID_INSERT_LIGHTSEQ:
        case ID_INSERT_PRIMITIVE:
        case ID_INSERT_FLASHER:
        case ID_INSERT_RUBBER:
        {
            ItemTypeEnum type = EditableRegistry::TypeFromToolID((int)id);
            if (type != eItemInvalid)
            {
                g_pvp->m_ToolCur = (int)id;

                POINT pt;
                GetCursorPos(&pt);
                SetCursorPos(pt.x, pt.y);
                return FALSE;
            }
            break;
        }
        case IDC_SELECT:
        case ID_TABLE_MAGNIFY:
        {
            m_selectButton.SetCheck(BST_UNCHECKED);
            m_magnifyButton.SetCheck(BST_UNCHECKED);
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
        case ID_EDIT_PROPERTIES:
        {
            g_pvp->ParseCommand(id, 3); //3=toggle
            break;
        }
        case ID_EDIT_SCRIPT:
        {
            g_pvp->ToggleScriptEditor();
            break;
        }
        case ID_EDIT_BACKGLASSVIEW:
        {
            g_pvp->ToggleBackglassView();
            break;
        }
        case ID_TABLE_PLAY:
        {
            g_pvp->DoPlay(false);  //only normaly play mode via this dialog
            break;
        }
        case ID_LAYER_LAYER1:
        case ID_LAYER_LAYER2:
        case ID_LAYER_LAYER3:
        case ID_LAYER_LAYER4:
        case ID_LAYER_LAYER5:
        case ID_LAYER_LAYER6:
        case ID_LAYER_LAYER7:
        case ID_LAYER_LAYER8:
        case ID_LAYER_LAYER9:
        case ID_LAYER_LAYER10:
        case ID_LAYER_LAYER11:
        {
            g_pvp->SetLayerStatus(id - ID_LAYER_LAYER1);
            break;
        }
        case ID_LAYER_TOGGLEALL:
        {
            g_pvp->ToggleAllLayers();
            break;
        }
    }
    return FALSE;
}

CContainToolbar::CContainToolbar()
{
    SetView(m_toolbar); 
    SetTabText(_T("Toolbar"));
    SetTabIcon(IDI_VPINBALL);
    SetDockCaption(_T("Toolbar"));
    
}

CDockToolbar::CDockToolbar()
{
    SetView(m_toolbarContainer);
    SetBarWidth(4);
}
