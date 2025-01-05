// license:GPLv3+

#include "core/stdafx.h"
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
    AttachItem(ID_EDIT_SCRIPT, m_scriptButton);
    AttachItem(ID_EDIT_BACKGLASSVIEW, m_backglassButton);
    AttachItem(ID_TABLE_PLAY, m_playButton);
    AttachItem(ID_TABLE_PLAY_CAMERA, m_playCameraButton);
    AttachItem(ID_INSERT_WALL, m_wallButton);
    AttachItem(ID_INSERT_GATE, m_gateButton);
    AttachItem(ID_INSERT_RAMP, m_rampButton);
    AttachItem(ID_INSERT_FLIPPER, m_flipperButton);
    AttachItem(ID_INSERT_PLUNGER, m_plungerButton);
    AttachItem(ID_INSERT_BALL, m_ballButton);
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
    m_tooltip.AddTool(m_selectButton, _T("Select Element"));
    m_tooltip.AddTool(m_scriptButton, _T("Toggle Script Editor"));
    m_tooltip.AddTool(m_backglassButton, _T("Toggle Backglass View"));
    m_tooltip.AddTool(m_playButton, _T("Play Table"));
    m_tooltip.AddTool(m_playCameraButton, _T("Adjust user settings (Camera/Options)"));

    m_tooltip.AddTool(m_wallButton, _T("Insert Wall"));
    m_tooltip.AddTool(m_gateButton, _T("Insert Gate"));
    m_tooltip.AddTool(m_rampButton, _T("Insert Ramp"));
    m_tooltip.AddTool(m_flipperButton, _T("Insert Flipper"));
    m_tooltip.AddTool(m_plungerButton, _T("Insert Plunger"));
    m_tooltip.AddTool(m_ballButton, _T("Insert Ball"));
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

    constexpr int iconSize = 24;
    HANDLE hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_MAGNIFY), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_magnifyButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_SELECT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_selectButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_SCRIPT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_scriptButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_BACKGLASS), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_backglassButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_playButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_CAMERA), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_playCameraButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_WALL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_wallButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_GATE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_gateButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_RAMP), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_rampButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_FLIPPER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_flipperButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_PLUNGER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_plungerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_BUMPER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_bumperButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_BALL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_ballButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_SPINNER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_spinnerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_TIMER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_timerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_TRIGGER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_triggerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_LIGHT), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_lightButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_KICKER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_kickerButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_TARGET), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_targetButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_DECAL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_decalButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_TEXTBOX), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_textboxButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_REEL), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_reelButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_LIGHTSEQ), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_lightseqButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_PRIMITIVE), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_primitiveButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_FLASHER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_flasherButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_RUBBER), IMAGE_ICON, iconSize, iconSize, LR_DEFAULTCOLOR);
    m_rubberButton.SetIcon((HICON)hIcon);


    m_resizer.Initialize(this->GetHwnd(), CRect(0, 0, 90, 600));
    m_resizer.AddChild(m_magnifyButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_selectButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_scriptButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_backglassButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_playButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_playCameraButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(GetDlgItem(IDC_STATIC_ELEMENTS).GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);

    m_resizer.AddChild(m_wallButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_gateButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_rampButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_flipperButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_plungerButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_ballButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_bumperButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_spinnerButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_timerButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_triggerButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_lightButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_kickerButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_targetButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_decalButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_textboxButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_reelButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_lightseqButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_primitiveButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_flasherButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(m_rubberButton.GetHwnd(), CResizer::center, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);

    m_resizer.RecalcLayout();

    EnableButtons();
    return TRUE;
}

INT_PTR ToolbarDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Pass resizing messages on to the resizer
    m_resizer.HandleMessage(msg, wparam, lparam);

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
    if (ptCur == nullptr && !g_pplayer)
    {
        m_magnifyButton.EnableWindow(FALSE);
        m_selectButton.EnableWindow(FALSE);
        m_scriptButton.EnableWindow(FALSE);
        m_backglassButton.EnableWindow(FALSE);
        m_playButton.EnableWindow(FALSE);
        m_playCameraButton.EnableWindow(FALSE);

        m_textboxButton.EnableWindow(FALSE);
        m_reelButton.EnableWindow(FALSE);
        m_wallButton.EnableWindow(FALSE);
        m_gateButton.EnableWindow(FALSE);
        m_rampButton.EnableWindow(FALSE);
        m_flipperButton.EnableWindow(FALSE);
        m_plungerButton.EnableWindow(FALSE);
        m_ballButton.EnableWindow(FALSE);
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
    else
    {
        BOOL lockable = ptCur->IsLocked() ? FALSE : TRUE;

        m_magnifyButton.EnableWindow(TRUE);
        m_selectButton.EnableWindow(lockable);
        m_scriptButton.EnableWindow(lockable);
        m_backglassButton.EnableWindow(TRUE);
        m_playButton.EnableWindow(TRUE);
        m_playCameraButton.EnableWindow(TRUE);

        m_decalButton.EnableWindow(lockable);
        m_lightButton.EnableWindow(lockable);
        m_timerButton.EnableWindow(lockable);
        m_lightseqButton.EnableWindow(lockable);
        m_flasherButton.EnableWindow(lockable);

        BOOL lockableNo3D = g_pvp->m_backglassView ? lockable : FALSE;
        m_textboxButton.EnableWindow(lockableNo3D);
        m_reelButton.EnableWindow(lockableNo3D);

        BOOL lockableNoBG = g_pvp->m_backglassView ? FALSE : lockable;
        m_wallButton.EnableWindow(lockableNoBG);
        m_gateButton.EnableWindow(lockableNoBG);
        m_rampButton.EnableWindow(lockableNoBG);
        m_flipperButton.EnableWindow(lockableNoBG);
        m_plungerButton.EnableWindow(lockableNoBG);
        m_ballButton.EnableWindow(lockableNoBG);
        m_bumperButton.EnableWindow(lockableNoBG);
        m_spinnerButton.EnableWindow(lockableNoBG);
        m_triggerButton.EnableWindow(lockableNoBG);
        m_targetButton.EnableWindow(lockableNoBG);
        m_kickerButton.EnableWindow(lockableNoBG);
        m_primitiveButton.EnableWindow(lockableNoBG);
        m_rubberButton.EnableWindow(lockableNoBG);
    }
}

bool ToolbarDialog::PreTranslateMessage(MSG* msg)
{
   if (!IsWindow())
      return false;

   // only pre-translate mouse and keyboard input events
   if (((msg->message >= WM_KEYFIRST && msg->message <= WM_KEYLAST) || (msg->message >= WM_MOUSEFIRST && msg->message <= WM_MOUSELAST)))
   {
      const int keyPressed = LOWORD(msg->wParam);
      // only pass F1-F12 to the main VPinball class to open subdialogs from everywhere
      if (keyPressed >= VK_F1 && keyPressed <= VK_F12 && TranslateAccelerator(g_pvp->GetHwnd(), g_haccel, msg))
         return true;
   }

   return !!IsDialogMessage(*msg);
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
        case ID_INSERT_BALL:
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
            const ItemTypeEnum type = EditableRegistry::TypeFromToolID((int)id);
            if (type != eItemInvalid)
            {
                g_pvp->m_ToolCur = (int)id;
                return TRUE;
            }
            break;
        }
        case IDC_SELECT:
        case ID_TABLE_MAGNIFY:
        {
            g_pvp->m_ToolCur = id;
            m_selectButton.SetCheck(BST_UNCHECKED);
            m_magnifyButton.SetCheck(BST_UNCHECKED);
            switch (HIWORD(wParam))
            {
                case BN_CLICKED:
                {
                    if (IsDlgButtonChecked(id) == BST_UNCHECKED)
                        SendDlgItemMessage(id, BM_SETCHECK, BST_CHECKED, 0);

                    return TRUE;
                }
                default:
                    break;
            }
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
            g_pvp->DoPlay(false); 
            break;
        }
        case ID_TABLE_PLAY_CAMERA:
        {
            g_pvp->DoPlay(true);  
            break;
        }
    }
    return FALSE;
}

CContainToolbar::CContainToolbar()
{
    SetView(m_toolbar); 
    SetTabText(_T("Toolbar"));
    SetTabIcon(IDI_TOOLBAR);
    SetDockCaption(_T("Toolbar"));
}

CDockToolbar::CDockToolbar()
{
    SetView(m_toolbarContainer);
    SetBarWidth(4);
}

void CDockToolbar::OnClose()
{
    // nothing to do only to prevent closing the window
}
