#include "StdAfx.h"
#include "resource.h"
#include "EditorOptionsDialog.h"

#define AUTOSAVE_DEFAULT_TIME 10

EditorOptionsDialog::EditorOptionsDialog() : CDialog(IDD_EDITOR_OPTIONS)
{
    m_toolTip = NULL;
}

void EditorOptionsDialog::OnClose()
{
    CDialog::OnClose();
}

void EditorOptionsDialog::AddToolTip(char *text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
    TOOLINFO toolInfo ={ 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = parentHwnd;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)controlHwnd;
    toolInfo.lpszText = text;
    SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

BOOL EditorOptionsDialog::OnInitDialog()
{
    m_toolTip = new CToolTip();

    HWND toolTipHwnd = ::CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), NULL, g_hinst, NULL);
    if(toolTipHwnd)
    {
        SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
        HWND controlHwnd = GetDlgItem(IDC_THROW_BALLS_ALWAYS_ON_CHECK);
        AddToolTip("If checked, the 'Throw Balls in Player' option is always active. You don't need to activate it in the debug menu again.", GetHwnd(), toolTipHwnd, controlHwnd);
        controlHwnd = GetDlgItem(IDC_THROW_BALLS_SIZE_EDIT);
        AddToolTip("Defines the default size of the ball when dropped onto the table.", GetHwnd(), toolTipHwnd, controlHwnd);
    }
    HWND hwndControl;

    // drag points
    int fdrawpoints = GetRegIntWithDefault("Editor", "ShowDragPoints", 0);
    hwndControl = GetDlgItem(IDC_DRAW_DRAGPOINTS).GetHwnd();
    SendMessage(hwndControl, BM_SETCHECK, fdrawpoints ? BST_CHECKED : BST_UNCHECKED, 0);

    HWND hwndColor = GetDlgItem(IDC_COLOR).GetHwnd();
    SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->dummyMaterial.m_cBase);

    hwndColor = GetDlgItem(IDC_COLOR2).GetHwnd();
    SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_elemSelectColor);

    hwndColor = GetDlgItem(IDC_COLOR3).GetHwnd();
    SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_elemSelectLockedColor);

    hwndColor = GetDlgItem(IDC_COLOR4).GetHwnd();
    SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_fillColor);

    hwndColor = GetDlgItem(IDC_COLOR5).GetHwnd();
    SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_backgroundColor);
    // light centers
    int fdrawcenters = GetRegIntWithDefault("Editor", "DrawLightCenters", 0);
    hwndControl = GetDlgItem(IDC_DRAW_LIGHTCENTERS).GetHwnd();
    SendMessage(hwndControl, BM_SETCHECK, fdrawcenters ? BST_CHECKED : BST_UNCHECKED, 0);

    int fautosave = GetRegIntWithDefault("Editor", "AutoSaveOn", 1);
    SendDlgItemMessage(IDC_AUTOSAVE, BM_SETCHECK, fautosave ? BST_CHECKED : BST_UNCHECKED, 0);

    int propFloating = GetRegIntWithDefault("Editor", "PropertiesFloating", 1);
    SendDlgItemMessage(IDC_PROP_FLOAT_CHECK, BM_SETCHECK, propFloating ? BST_CHECKED : BST_UNCHECKED, 0);

    int fautosavetime = GetRegIntWithDefault("Editor", "AutoSaveTime", AUTOSAVE_DEFAULT_TIME);
    SetDlgItemInt(IDC_AUTOSAVE_MINUTES, fautosavetime, FALSE);

    int gridsize = GetRegIntWithDefault("Editor", "GridSize", 50);
    SetDlgItemInt(IDC_GRID_SIZE, gridsize, FALSE);

    int throwBallsAlwaysOn = GetRegIntWithDefault("Editor", "ThrowBallsAlwaysOn", 0);
    SendDlgItemMessage(IDC_THROW_BALLS_ALWAYS_ON_CHECK, BM_SETCHECK, throwBallsAlwaysOn ? BST_CHECKED : BST_UNCHECKED, 0);

    int groupElementsCollection = GetRegIntWithDefault("Editor", "GroupElementsInCollection", 1);
    SendDlgItemMessage(IDC_DEFAULT_GROUP_COLLECTION_CHECK, BM_SETCHECK, groupElementsCollection ? BST_CHECKED : BST_UNCHECKED, 0);

    int throwBallSize = GetRegIntWithDefault("Editor", "ThrowBallSize", 50);
    SetDlgItemInt( IDC_THROW_BALLS_SIZE_EDIT, throwBallSize, FALSE);

    int startVPfileDialog = GetRegIntWithDefault("Editor", "SelectTableOnStart", 1);
    SendDlgItemMessage(IDC_START_VP_FILE_DIALOG, BM_SETCHECK, startVPfileDialog ? BST_CHECKED : BST_UNCHECKED, 0);
    return TRUE;
}

BOOL EditorOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch(HIWORD(wParam))
    {
        case COLOR_CHANGED:
        {
            const size_t color = ::GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
            HWND hwndEvent = (HWND)lParam;
            HWND hwndcolor1 = GetDlgItem(IDC_COLOR).GetHwnd();
            HWND hwndcolor2 = GetDlgItem(IDC_COLOR2).GetHwnd();
            HWND hwndcolor3 = GetDlgItem(IDC_COLOR3).GetHwnd();
            HWND hwndcolor4 = GetDlgItem(IDC_COLOR4).GetHwnd();
            HWND hwndcolor5 = GetDlgItem(IDC_COLOR5).GetHwnd();
            if (hwndEvent == hwndcolor1)
               g_pvp->dummyMaterial.m_cBase = (COLORREF)color;
            else if (hwndEvent == hwndcolor2)
               g_pvp->m_elemSelectColor = (COLORREF)color;
            else if (hwndEvent == hwndcolor3)
               g_pvp->m_elemSelectLockedColor = (COLORREF)color;
            else if (hwndEvent == hwndcolor4)
               g_pvp->m_fillColor = (COLORREF)color;
            else if (hwndEvent == hwndcolor5)
               g_pvp->m_backgroundColor = (COLORREF)color;

            return TRUE;
        }
    }
    switch (LOWORD(wParam))
    {
       case IDC_DEFAULT_COLORS_BUTTON:
       {
          g_pvp->dummyMaterial.m_cBase = 0xB469FF;
          HWND hwndColor = GetDlgItem(IDC_COLOR).GetHwnd();
          SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->dummyMaterial.m_cBase);

          g_pvp->m_elemSelectColor = 0x00FF0000;
          hwndColor = GetDlgItem(IDC_COLOR2).GetHwnd();
          SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_elemSelectColor);

          g_pvp->m_elemSelectLockedColor = 0x00A7726D;
          hwndColor = GetDlgItem(IDC_COLOR3).GetHwnd();
          SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_elemSelectLockedColor);

          g_pvp->m_fillColor = 0x00B1CFB3;
          hwndColor = GetDlgItem(IDC_COLOR4).GetHwnd();
          SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_fillColor);

          g_pvp->m_backgroundColor = 0x008D8D8D;
          hwndColor = GetDlgItem(IDC_COLOR5).GetHwnd();
          SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_backgroundColor);
          return TRUE;
       }
    }

    return FALSE;
}

INT_PTR EditorOptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case GET_COLOR_TABLE:
        {
            *((unsigned long **)lParam) = &g_pvp->dummyMaterial.m_cBase;
            return TRUE;
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}

void EditorOptionsDialog::OnOK()
{
    bool checked;
    BOOL nothing=0;

    // drag points
    checked = (SendDlgItemMessage(IDC_DRAW_DRAGPOINTS, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SetRegValueBool("Editor", "ShowDragPoints", checked);

    // light centers
    checked = (SendDlgItemMessage(IDC_DRAW_LIGHTCENTERS, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SetRegValueBool("Editor", "DrawLightCenters", checked);

    // auto save
    const bool autosave = (SendDlgItemMessage(IDC_AUTOSAVE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SetRegValueBool("Editor", "AutoSaveOn", autosave);

    checked = (SendDlgItemMessage(IDC_PROP_FLOAT_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SetRegValueBool("Editor", "PropertiesFloating", checked);

    const int autosavetime = GetDlgItemInt(IDC_AUTOSAVE_MINUTES, nothing, FALSE);
    SetRegValueInt("Editor", "AutoSaveTime", autosavetime);

    const int gridsize = GetDlgItemInt(IDC_GRID_SIZE, nothing, FALSE);
    SetRegValueInt("Editor", "GridSize", gridsize);

    checked = (SendDlgItemMessage(IDC_THROW_BALLS_ALWAYS_ON_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SetRegValueBool("Editor", "ThrowBallsAlwaysOn", checked);

    const int ballSize = GetDlgItemInt(IDC_THROW_BALLS_SIZE_EDIT, nothing, FALSE);
    SetRegValueInt("Editor", "ThrowBallSize", ballSize);

    checked = (SendDlgItemMessage(IDC_DEFAULT_GROUP_COLLECTION_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SetRegValueBool("Editor", "GroupElementsInCollection", checked);

    // Go through and reset the autosave time on all the tables
    if (autosave)
        g_pvp->SetAutoSaveMinutes(autosavetime);
    else
        g_pvp->m_autosaveTime = -1;

    for (int i = 0; i < g_pvp->m_vtable.Size(); i++)
        g_pvp->m_vtable.ElementAt(i)->BeginAutoSaveCounter();

    SetRegValue("Editor", "DefaultMaterialColor", REG_DWORD, &g_pvp->dummyMaterial.m_cBase, 4);
    SetRegValue("Editor", "ElementSelectColor", REG_DWORD, &g_pvp->m_elemSelectColor, 4);
    SetRegValue("Editor", "ElementSelectLockedColor", REG_DWORD, &g_pvp->m_elemSelectLockedColor, 4);
    SetRegValue("Editor", "BackgroundColor", REG_DWORD, &g_pvp->m_backgroundColor, 4);
    SetRegValue("Editor", "FillColor", REG_DWORD, &g_pvp->m_fillColor, 4);

    checked = (SendDlgItemMessage(IDC_START_VP_FILE_DIALOG, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SetRegValueBool("Editor", "SelectTableOnStart", checked);

    CDialog::OnOK();
}

