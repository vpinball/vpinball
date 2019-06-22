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

    const HWND toolTipHwnd = ::CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), NULL, g_hinst, NULL);
    if (toolTipHwnd)
    {
        SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
        HWND controlHwnd = GetDlgItem(IDC_THROW_BALLS_ALWAYS_ON_CHECK);
        AddToolTip("If checked, the 'Throw Balls in Player' option is always active. You don't need to activate it in the debug menu again.", GetHwnd(), toolTipHwnd, controlHwnd);
        controlHwnd = GetDlgItem(IDC_THROW_BALLS_SIZE_EDIT);
        AddToolTip("Defines the default size of the ball when dropped onto the table.", GetHwnd(), toolTipHwnd, controlHwnd);
    }

    // drag points
    const bool fdrawpoints = LoadValueBoolWithDefault("Editor", "ShowDragPoints", false);
    SendMessage(GetDlgItem(IDC_DRAW_DRAGPOINTS).GetHwnd(), BM_SETCHECK, fdrawpoints ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessage(GetDlgItem(IDC_COLOR).GetHwnd(), CHANGE_COLOR, 0, g_pvp->m_dummyMaterial.m_cBase);
    SendMessage(GetDlgItem(IDC_COLOR2).GetHwnd(), CHANGE_COLOR, 0, g_pvp->m_elemSelectColor);
    SendMessage(GetDlgItem(IDC_COLOR3).GetHwnd(), CHANGE_COLOR, 0, g_pvp->m_elemSelectLockedColor);
    SendMessage(GetDlgItem(IDC_COLOR4).GetHwnd(), CHANGE_COLOR, 0, g_pvp->m_fillColor);
    SendMessage(GetDlgItem(IDC_COLOR5).GetHwnd(), CHANGE_COLOR, 0, g_pvp->m_backgroundColor);
    // light centers
    const bool fdrawcenters = LoadValueBoolWithDefault("Editor", "DrawLightCenters", false);
    SendMessage(GetDlgItem(IDC_DRAW_LIGHTCENTERS).GetHwnd(), BM_SETCHECK, fdrawcenters ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool fautosave = LoadValueBoolWithDefault("Editor", "AutoSaveOn", true);
    SendDlgItemMessage(IDC_AUTOSAVE, BM_SETCHECK, fautosave ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool propFloating = LoadValueBoolWithDefault("Editor", "PropertiesFloating", true);
    SendDlgItemMessage(IDC_PROP_FLOAT_CHECK, BM_SETCHECK, propFloating ? BST_CHECKED : BST_UNCHECKED, 0);

    const int fautosavetime = LoadValueIntWithDefault("Editor", "AutoSaveTime", AUTOSAVE_DEFAULT_TIME);
    SetDlgItemInt(IDC_AUTOSAVE_MINUTES, fautosavetime, FALSE);

    const int gridsize = LoadValueIntWithDefault("Editor", "GridSize", 50);
    SetDlgItemInt(IDC_GRID_SIZE, gridsize, FALSE);

    const bool throwBallsAlwaysOn = LoadValueBoolWithDefault("Editor", "ThrowBallsAlwaysOn", false);
    SendDlgItemMessage(IDC_THROW_BALLS_ALWAYS_ON_CHECK, BM_SETCHECK, throwBallsAlwaysOn ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool ballControlAlwaysOn = LoadValueBoolWithDefault("Editor", "BallControlAlwaysOn", false);
    SendDlgItemMessage(IDC_BALL_CONTROL_ALWAYS_ON_CHECK, BM_SETCHECK, ballControlAlwaysOn ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool groupElementsCollection = LoadValueBoolWithDefault("Editor", "GroupElementsInCollection", true);
    SendDlgItemMessage(IDC_DEFAULT_GROUP_COLLECTION_CHECK, BM_SETCHECK, groupElementsCollection ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool alwaysViewScript = LoadValueBoolWithDefault("Editor", "AlwaysViewScript", false);
    SendDlgItemMessage(IDC_ALWAYSVIEWSCRIPT, BM_SETCHECK, alwaysViewScript ? BST_CHECKED : BST_UNCHECKED, 0);

    const int throwBallSize = LoadValueIntWithDefault("Editor", "ThrowBallSize", 50);
    SetDlgItemInt( IDC_THROW_BALLS_SIZE_EDIT, throwBallSize, FALSE);

    const bool startVPfileDialog = LoadValueBoolWithDefault("Editor", "SelectTableOnStart", true);
    SendDlgItemMessage(IDC_START_VP_FILE_DIALOG, BM_SETCHECK, startVPfileDialog ? BST_CHECKED : BST_UNCHECKED, 0);

    const float throwBallMass = LoadValueFloatWithDefault("Editor", "ThrowBallMass", 1.0f);
    char textBuf[256] = { 0 };
    f2sz(throwBallMass, textBuf);
    const CString textStr(textBuf);
    SetDlgItemText(IDC_THROW_BALLS_MASS_EDIT, textStr);

    const int units = LoadValueIntWithDefault("Editor", "Units", 0);
    SendMessage(GetDlgItem(IDC_UNIT_LIST_COMBO).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"VPUnits");
    SendMessage(GetDlgItem(IDC_UNIT_LIST_COMBO).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Inches");
    SendMessage(GetDlgItem(IDC_UNIT_LIST_COMBO).GetHwnd(), CB_ADDSTRING, 0, (LPARAM)"Millimeters");
    SendMessage(GetDlgItem(IDC_UNIT_LIST_COMBO).GetHwnd(), CB_SETCURSEL, units, 0);

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
            const HWND hwndEvent = (HWND)lParam;
            if (hwndEvent == GetDlgItem(IDC_COLOR).GetHwnd())
               g_pvp->m_dummyMaterial.m_cBase = (COLORREF)color;
            else if (hwndEvent == GetDlgItem(IDC_COLOR2).GetHwnd())
               g_pvp->m_elemSelectColor = (COLORREF)color;
            else if (hwndEvent == GetDlgItem(IDC_COLOR3).GetHwnd())
               g_pvp->m_elemSelectLockedColor = (COLORREF)color;
            else if (hwndEvent == GetDlgItem(IDC_COLOR4).GetHwnd())
               g_pvp->m_fillColor = (COLORREF)color;
            else if (hwndEvent == GetDlgItem(IDC_COLOR5).GetHwnd())
               g_pvp->m_backgroundColor = (COLORREF)color;

            return TRUE;
        }
    }
    switch (LOWORD(wParam))
    {
       case IDC_DEFAULT_COLORS_BUTTON:
       {
          g_pvp->m_dummyMaterial.m_cBase = 0xB469FF;
          HWND hwndColor = GetDlgItem(IDC_COLOR).GetHwnd();
          SendMessage(hwndColor, CHANGE_COLOR, 0, g_pvp->m_dummyMaterial.m_cBase);

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
       case IDC_SET_DEFAULTS_BUTTON:
       {
          HWND hwndControl;
          hwndControl = GetDlgItem(IDC_DRAW_DRAGPOINTS).GetHwnd();
          SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
          hwndControl = GetDlgItem(IDC_DRAW_LIGHTCENTERS).GetHwnd();
          SendMessage(hwndControl, BM_SETCHECK, BST_UNCHECKED, 0);
          SendDlgItemMessage(IDC_AUTOSAVE, BM_SETCHECK, BST_CHECKED, 0);
          SendDlgItemMessage(IDC_PROP_FLOAT_CHECK, BM_SETCHECK, BST_CHECKED, 0);
          SetDlgItemInt(IDC_AUTOSAVE_MINUTES, AUTOSAVE_DEFAULT_TIME, FALSE);
          SetDlgItemInt(IDC_GRID_SIZE, 50, FALSE);
          SendDlgItemMessage(IDC_THROW_BALLS_ALWAYS_ON_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
		  SendDlgItemMessage(IDC_BALL_CONTROL_ALWAYS_ON_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
		  SendDlgItemMessage(IDC_DEFAULT_GROUP_COLLECTION_CHECK, BM_SETCHECK, BST_CHECKED, 0);
		  SendDlgItemMessage(IDC_ALWAYSVIEWSCRIPT, BM_SETCHECK, BST_CHECKED, 0);
          SetDlgItemInt(IDC_THROW_BALLS_SIZE_EDIT, 50, FALSE);
          SendDlgItemMessage(IDC_START_VP_FILE_DIALOG, BM_SETCHECK, BST_CHECKED, 0);
          SendMessage(GetDlgItem(IDC_UNIT_LIST_COMBO).GetHwnd(), CB_SETCURSEL, 0, 0);
          SetDlgItemText(IDC_THROW_BALLS_MASS_EDIT, "1.0");
          const int x = 0;
          const int y = 0;
          SaveValueInt("Editor", "CodeViewPosX", x);
          SaveValueInt("Editor", "CodeViewPosY", y);
          const int width = 640;
          const int height = 490;
          SaveValueInt("Editor", "CodeViewPosWidth", width);
          SaveValueInt("Editor", "CodeViewPosHeight", height);

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
            *((unsigned long **)lParam) = &g_pvp->m_dummyMaterial.m_cBase;
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
    SaveValueBool("Editor", "ShowDragPoints", checked);

    // light centers
    checked = (SendDlgItemMessage(IDC_DRAW_LIGHTCENTERS, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "DrawLightCenters", checked);

    // auto save
    const bool autosave = (SendDlgItemMessage(IDC_AUTOSAVE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "AutoSaveOn", autosave);

    checked = (SendDlgItemMessage(IDC_PROP_FLOAT_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "PropertiesFloating", checked);

    const int autosavetime = GetDlgItemInt(IDC_AUTOSAVE_MINUTES, nothing, FALSE);
    SaveValueInt("Editor", "AutoSaveTime", autosavetime);

    const int gridsize = GetDlgItemInt(IDC_GRID_SIZE, nothing, FALSE);
    SaveValueInt("Editor", "GridSize", gridsize);

    checked = (SendDlgItemMessage(IDC_THROW_BALLS_ALWAYS_ON_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "ThrowBallsAlwaysOn", checked);

    checked = (SendDlgItemMessage(IDC_BALL_CONTROL_ALWAYS_ON_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "BallControlAlwaysOn", checked);

    const int ballSize = GetDlgItemInt(IDC_THROW_BALLS_SIZE_EDIT, nothing, FALSE);
    SaveValueInt("Editor", "ThrowBallSize", ballSize);

    CString textStr;
    float fv;
    textStr = GetDlgItemText(IDC_THROW_BALLS_MASS_EDIT);
    fv = sz2f(textStr.c_str());
    SaveValueFloat("Editor", "ThrowBallMass", fv);

    checked = (SendDlgItemMessage(IDC_DEFAULT_GROUP_COLLECTION_CHECK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "GroupElementsInCollection", checked);

    checked = (SendDlgItemMessage(IDC_ALWAYSVIEWSCRIPT, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "AlwaysViewScript", checked);


    // Go through and reset the autosave time on all the tables
    if (autosave)
        g_pvp->SetAutoSaveMinutes(autosavetime);
    else
        g_pvp->m_autosaveTime = -1;

    for (size_t i = 0; i < g_pvp->m_vtable.size(); i++)
        g_pvp->m_vtable[i]->BeginAutoSaveCounter();

    SaveValueInt("Editor", "DefaultMaterialColor", g_pvp->m_dummyMaterial.m_cBase);
    SaveValueInt("Editor", "ElementSelectColor", g_pvp->m_elemSelectColor);
    SaveValueInt("Editor", "ElementSelectLockedColor", g_pvp->m_elemSelectLockedColor);
    SaveValueInt("Editor", "BackgroundColor", g_pvp->m_backgroundColor);
    SaveValueInt("Editor", "FillColor", g_pvp->m_fillColor);

    checked = (SendDlgItemMessage(IDC_START_VP_FILE_DIALOG, BM_GETCHECK, 0, 0) == BST_CHECKED);
    SaveValueBool("Editor", "SelectTableOnStart", checked);

    HWND hwndUnits = GetDlgItem(IDC_UNIT_LIST_COMBO).GetHwnd();
    size_t units = SendMessage(hwndUnits, CB_GETCURSEL, 0, 0);
    if (units == LB_ERR)
        units = 0;
    SaveValueInt("Editor", "Units", units);

    CDialog::OnOK();
}
