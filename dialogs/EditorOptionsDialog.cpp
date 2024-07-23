#include "stdafx.h"
#include "resource.h"
#include "EditorOptionsDialog.h"

#include <filesystem>

#define AUTOSAVE_DEFAULT_TIME 10

EditorOptionsDialog::EditorOptionsDialog() : CDialog(IDD_EDITOR_OPTIONS)
{
    m_toolTip = nullptr;
}

void EditorOptionsDialog::OnClose()
{
    CDialog::OnClose();
}

void EditorOptionsDialog::AddToolTip(const char * const text, HWND parentHwnd, HWND toolTipHwnd, HWND controlHwnd)
{
    TOOLINFO toolInfo ={ 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = parentHwnd;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)controlHwnd;
    toolInfo.lpszText = (char*)text;
    SendMessage(toolTipHwnd, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
}

BOOL EditorOptionsDialog::OnInitDialog()
{
    m_toolTip = new CToolTip();

    const HWND toolTipHwnd = ::CreateWindowEx(0, TOOLTIPS_CLASS, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, GetHwnd(), nullptr, g_pvp->theInstance, nullptr);
    if (toolTipHwnd)
    {
        SendMessage(toolTipHwnd, TTM_SETMAXTIPWIDTH, 0, 180);
        AddToolTip("If checked, the 'Throw Balls in Player' option is always active. You don't need to activate it in the debug menu again.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_THROW_BALLS_ALWAYS_ON_CHECK));
        AddToolTip("Defines the default size of the ball when dropped onto the table.", GetHwnd(), toolTipHwnd, GetDlgItem(IDC_THROW_BALLS_SIZE_EDIT));
    }

    AttachItem(IDC_COLOR_BUTTON2, m_colorButton2);
    AttachItem(IDC_COLOR_BUTTON3, m_colorButton3);
    AttachItem(IDC_COLOR_BUTTON4, m_colorButton4);
    AttachItem(IDC_COLOR_BUTTON5, m_colorButton5);
    AttachItem(IDC_COLOR_BUTTON6, m_colorButton6);
    m_colorButton2.SetColor(g_pvp->m_dummyMaterial.m_cBase);
    m_colorButton3.SetColor(g_pvp->m_elemSelectColor);
    m_colorButton4.SetColor(g_pvp->m_elemSelectLockedColor);
    m_colorButton5.SetColor(g_pvp->m_fillColor);
    m_colorButton6.SetColor(g_pvp->m_backgroundColor);

    // drag points
    const bool fdrawpoints = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "ShowDragPoints"s, false);
    SendDlgItemMessage(IDC_DRAW_DRAGPOINTS, BM_SETCHECK, fdrawpoints ? BST_CHECKED : BST_UNCHECKED, 0);

    // light centers
    const bool fdrawcenters = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "DrawLightCenters"s, false);
    SendDlgItemMessage(IDC_DRAW_LIGHTCENTERS, BM_SETCHECK, fdrawcenters ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool fautosave = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "AutoSaveOn"s, true);
    SendDlgItemMessage(IDC_AUTOSAVE, BM_SETCHECK, fautosave ? BST_CHECKED : BST_UNCHECKED, 0);

    const int fautosavetime = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "AutoSaveTime"s, AUTOSAVE_DEFAULT_TIME);
    SetDlgItemInt(IDC_AUTOSAVE_MINUTES, fautosavetime, FALSE);

    const int gridsize = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "GridSize"s, 50);
    SetDlgItemInt(IDC_GRID_SIZE, gridsize, FALSE);

    const bool throwBallsAlwaysOn = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallsAlwaysOn"s, false);
    SendDlgItemMessage(IDC_THROW_BALLS_ALWAYS_ON_CHECK, BM_SETCHECK, throwBallsAlwaysOn ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool ballControlAlwaysOn = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "BallControlAlwaysOn"s, false);
    SendDlgItemMessage(IDC_BALL_CONTROL_ALWAYS_ON_CHECK, BM_SETCHECK, ballControlAlwaysOn ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool groupElementsCollection = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "GroupElementsInCollection"s, true);
    SendDlgItemMessage(IDC_DEFAULT_GROUP_COLLECTION_CHECK, BM_SETCHECK, groupElementsCollection ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool alwaysViewScript = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "AlwaysViewScript"s, false);
    SendDlgItemMessage(IDC_ALWAYSVIEWSCRIPT, BM_SETCHECK, alwaysViewScript ? BST_CHECKED : BST_UNCHECKED, 0);

    const int throwBallSize = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallSize"s, 50);
    SetDlgItemInt( IDC_THROW_BALLS_SIZE_EDIT, throwBallSize, FALSE);

    const bool startVPfileDialog = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SelectTableOnStart"s, true);
    SendDlgItemMessage(IDC_START_VP_FILE_DIALOG, BM_SETCHECK, startVPfileDialog ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool startVPfileDialogPlayerClose = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "SelectTableOnPlayerClose"s, true);
    SendDlgItemMessage(IDC_START_VP_FILE_DIALOG2, BM_SETCHECK, startVPfileDialogPlayerClose ? BST_CHECKED : BST_UNCHECKED, 0);

    const float throwBallMass = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "ThrowBallMass"s, 1.0f);
    SetDlgItemText(IDC_THROW_BALLS_MASS_EDIT, f2sz(throwBallMass).c_str());

    const bool enableLog = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "EnableLog"s, false);
    SendDlgItemMessage(IDC_ENABLE_LOGGING, BM_SETCHECK, enableLog ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool logScript = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "LogScriptOutput"s, false);
    SendDlgItemMessage(IDC_ENABLE_SCRIPT_LOGGING, BM_SETCHECK, logScript ? BST_CHECKED : BST_UNCHECKED, 0);

    const bool storeIniLocation = (g_pvp->m_szMyPrefPath == g_pvp->m_szMyPath);
    SendDlgItemMessage(IDC_STORE_INI_LOCATION, BM_SETCHECK, storeIniLocation ? BST_CHECKED : BST_UNCHECKED, 0);

    const int units = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "Units"s, 0);
    const HWND hwnd = GetDlgItem(IDC_UNIT_LIST_COMBO).GetHwnd();
    SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"VPUnits");
    SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Inches");
    SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)"Millimeters");
    SendMessage(hwnd, CB_SETCURSEL, units, 0);

    return TRUE;
}

BOOL EditorOptionsDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (LOWORD(wParam))
    {
       case IDC_COLOR_BUTTON2:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetColor(g_pvp->m_dummyMaterial.m_cBase);
           if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
           {
               g_pvp->m_dummyMaterial.m_cBase = m_colorDialog.GetColor();
               m_colorButton2.SetColor(g_pvp->m_dummyMaterial.m_cBase);
           }
           break;
       }
       case IDC_COLOR_BUTTON3:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetColor(g_pvp->m_elemSelectColor);
           if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
           {
               g_pvp->m_elemSelectColor = m_colorDialog.GetColor();
               m_colorButton3.SetColor(g_pvp->m_elemSelectColor);
           }
           break;
       }
       case IDC_COLOR_BUTTON4:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetColor(g_pvp->m_elemSelectLockedColor);
           if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
           {
               g_pvp->m_elemSelectLockedColor = m_colorDialog.GetColor();
               m_colorButton4.SetColor(g_pvp->m_elemSelectLockedColor);
           }
           break;
       }
       case IDC_COLOR_BUTTON5:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetColor(g_pvp->m_fillColor);
           if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
           {
               g_pvp->m_fillColor = m_colorDialog.GetColor();
               m_colorButton5.SetColor(g_pvp->m_fillColor);
           }
           break;
       }
       case IDC_COLOR_BUTTON6:
       {
           CHOOSECOLOR cc = m_colorDialog.GetParameters();
           cc.Flags = CC_FULLOPEN | CC_RGBINIT;
           m_colorDialog.SetParameters(cc);
           m_colorDialog.SetColor(g_pvp->m_backgroundColor);
           if (m_colorDialog.DoModal(GetHwnd()) == IDOK)
           {
               g_pvp->m_backgroundColor = m_colorDialog.GetColor();
               m_colorButton6.SetColor(g_pvp->m_backgroundColor);
           }
           break;
       }
       case IDC_DEFAULT_COLORS_BUTTON:
       {
          g_pvp->m_dummyMaterial.m_cBase = 0xB469FF;
          m_colorButton2.SetColor(g_pvp->m_dummyMaterial.m_cBase);

          g_pvp->m_elemSelectColor = 0x00FF0000;
          m_colorButton3.SetColor(g_pvp->m_elemSelectColor);

          g_pvp->m_elemSelectLockedColor = 0x00A7726D;
          m_colorButton4.SetColor(g_pvp->m_elemSelectLockedColor);

          g_pvp->m_fillColor = 0x00B1CFB3;
          m_colorButton5.SetColor(g_pvp->m_fillColor);

          g_pvp->m_backgroundColor = 0x008D8D8D;
          m_colorButton6.SetColor(g_pvp->m_backgroundColor);
          return TRUE;
       }
       case IDC_SET_DEFAULTS_BUTTON:
       {
          SendDlgItemMessage(IDC_DRAW_DRAGPOINTS, BM_SETCHECK, BST_UNCHECKED, 0);
          SendDlgItemMessage(IDC_DRAW_LIGHTCENTERS, BM_SETCHECK, BST_UNCHECKED, 0);
          SendDlgItemMessage(IDC_AUTOSAVE, BM_SETCHECK, BST_CHECKED, 0);
          SetDlgItemInt(IDC_AUTOSAVE_MINUTES, AUTOSAVE_DEFAULT_TIME, FALSE);
          SetDlgItemInt(IDC_GRID_SIZE, 50, FALSE);
          SendDlgItemMessage(IDC_THROW_BALLS_ALWAYS_ON_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
          SendDlgItemMessage(IDC_BALL_CONTROL_ALWAYS_ON_CHECK, BM_SETCHECK, BST_UNCHECKED, 0);
          SendDlgItemMessage(IDC_DEFAULT_GROUP_COLLECTION_CHECK, BM_SETCHECK, BST_CHECKED, 0);
          SendDlgItemMessage(IDC_ALWAYSVIEWSCRIPT, BM_SETCHECK, BST_CHECKED, 0);
          SetDlgItemInt(IDC_THROW_BALLS_SIZE_EDIT, 50, FALSE);
          SendDlgItemMessage(IDC_START_VP_FILE_DIALOG, BM_SETCHECK, BST_CHECKED, 0);
          SendDlgItemMessage(IDC_START_VP_FILE_DIALOG2, BM_SETCHECK, BST_CHECKED, 0);
          SendDlgItemMessage(IDC_UNIT_LIST_COMBO, CB_SETCURSEL, 0, 0);
          SetDlgItemText(IDC_THROW_BALLS_MASS_EDIT, "1.0");
          SendDlgItemMessage(IDC_ENABLE_LOGGING, BM_SETCHECK, BST_UNCHECKED, 0);
          SendDlgItemMessage(IDC_ENABLE_SCRIPT_LOGGING, BM_SETCHECK, BST_CHECKED, 0);
          SendDlgItemMessage(IDC_STORE_INI_LOCATION, BM_SETCHECK, BST_UNCHECKED, 0);
          constexpr int x = 0;
          constexpr int y = 0;
          g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosX"s, x);
          g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosY"s, y);
          constexpr int width = 640;
          constexpr int height = 490;
          g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosWidth"s, width);
          g_pvp->m_settings.SaveValue(Settings::Editor, "CodeViewPosHeight"s, height);

          return TRUE;
       }
       case IDC_RESET_WINDOW_POS:
       {
          g_pvp->ResetAllDockers();
          return TRUE;
       }
    }

    return FALSE;
}

INT_PTR EditorOptionsDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        case WM_DRAWITEM:
        {
            const LPDRAWITEMSTRUCT lpDrawItemStruct = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);
            const UINT nID = static_cast<UINT>(wParam);
            if (nID == IDC_COLOR_BUTTON2)
            {
                m_colorButton2.DrawItem(lpDrawItemStruct);
            }
            else if (nID == IDC_COLOR_BUTTON3)
            {
                m_colorButton3.DrawItem(lpDrawItemStruct);
            }
            else if (nID == IDC_COLOR_BUTTON4)
            {
                m_colorButton4.DrawItem(lpDrawItemStruct);
            }
            else if (nID == IDC_COLOR_BUTTON5)
            {
                m_colorButton5.DrawItem(lpDrawItemStruct);
            }
            else if (nID == IDC_COLOR_BUTTON6)
            {
                m_colorButton6.DrawItem(lpDrawItemStruct);
            }

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
    checked = (IsDlgButtonChecked(IDC_DRAW_DRAGPOINTS) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "ShowDragPoints"s, checked);

    // light centers
    checked = (IsDlgButtonChecked(IDC_DRAW_LIGHTCENTERS) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "DrawLightCenters"s, checked);

    // auto save
    const bool autosave = (IsDlgButtonChecked(IDC_AUTOSAVE) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "AutoSaveOn"s, autosave);

    const int autosavetime = GetDlgItemInt(IDC_AUTOSAVE_MINUTES, nothing, FALSE);
    g_pvp->m_settings.SaveValue(Settings::Editor, "AutoSaveTime"s, autosavetime);

    const int gridsize = GetDlgItemInt(IDC_GRID_SIZE, nothing, FALSE);
    g_pvp->m_settings.SaveValue(Settings::Editor, "GridSize"s, gridsize);

    checked = (IsDlgButtonChecked(IDC_THROW_BALLS_ALWAYS_ON_CHECK) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "ThrowBallsAlwaysOn"s, checked);

    checked = (IsDlgButtonChecked(IDC_BALL_CONTROL_ALWAYS_ON_CHECK) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "BallControlAlwaysOn"s, checked);

    const int ballSize = GetDlgItemInt(IDC_THROW_BALLS_SIZE_EDIT, nothing, FALSE);
    g_pvp->m_settings.SaveValue(Settings::Editor, "ThrowBallSize"s, ballSize);

    const float fv = sz2f(GetDlgItemText(IDC_THROW_BALLS_MASS_EDIT).c_str());
    g_pvp->m_settings.SaveValue(Settings::Editor, "ThrowBallMass"s, fv);

    checked = (IsDlgButtonChecked(IDC_DEFAULT_GROUP_COLLECTION_CHECK) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "GroupElementsInCollection"s, checked);

    checked = (IsDlgButtonChecked(IDC_ALWAYSVIEWSCRIPT) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "AlwaysViewScript"s, checked);

    checked = (IsDlgButtonChecked(IDC_ENABLE_LOGGING) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "EnableLog"s, checked);
    Logger::GetInstance()->SetupLogger(checked);

    checked = (IsDlgButtonChecked(IDC_ENABLE_SCRIPT_LOGGING) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "LogScriptOutput"s, checked);

    checked = (IsDlgButtonChecked(IDC_STORE_INI_LOCATION) == BST_CHECKED);
    const bool storeIniLocation_checked = (g_pvp->m_szMyPrefPath == g_pvp->m_szMyPath);
    if (checked != storeIniLocation_checked)
    {
       // if checkbox changed, copy ini from one default location to the other
       const string old = g_pvp->m_szMyPrefPath;
       if (storeIniLocation_checked)
           g_pvp->GetMyPrefPath();
       else
           g_pvp->m_szMyPrefPath = g_pvp->m_szMyPath;
       std::filesystem::rename(old + "VPinballX.ini", g_pvp->m_szMyPrefPath + "VPinballX.ini");
       std::filesystem::rename(old + "BAMViewSettings.xml", g_pvp->m_szMyPrefPath + "BAMViewSettings.xml");
       g_pvp->m_settings.SetIniPath(g_pvp->m_szMyPrefPath + "VPinballX.ini");
    }

    // Go through and reset the autosave time on all the tables
    if (autosave)
        g_pvp->SetAutoSaveMinutes(autosavetime);
    else
        g_pvp->m_autosaveTime = -1;

    for (size_t i = 0; i < g_pvp->m_vtable.size(); i++)
        g_pvp->m_vtable[i]->BeginAutoSaveCounter();

    g_pvp->m_settings.SaveValue(Settings::Editor, "DefaultMaterialColor"s, (int)g_pvp->m_dummyMaterial.m_cBase);
    g_pvp->m_settings.SaveValue(Settings::Editor, "ElementSelectColor"s, (int)g_pvp->m_elemSelectColor);
    g_pvp->m_settings.SaveValue(Settings::Editor, "ElementSelectLockedColor"s, (int)g_pvp->m_elemSelectLockedColor);
    g_pvp->m_settings.SaveValue(Settings::Editor, "BackgroundColor"s, (int)g_pvp->m_backgroundColor);
    g_pvp->m_settings.SaveValue(Settings::Editor, "FillColor"s, (int)g_pvp->m_fillColor);

    checked = (IsDlgButtonChecked(IDC_START_VP_FILE_DIALOG) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "SelectTableOnStart"s, checked);

    checked = (IsDlgButtonChecked(IDC_START_VP_FILE_DIALOG2) == BST_CHECKED);
    g_pvp->m_settings.SaveValue(Settings::Editor, "SelectTableOnPlayerClose"s, checked);

    LRESULT units = SendDlgItemMessage(IDC_UNIT_LIST_COMBO, CB_GETCURSEL, 0, 0);
    if (units == LB_ERR)
        units = 0;
    g_pvp->m_settings.SaveValue(Settings::Editor, "Units"s, (int)units);

    CDialog::OnOK();
}
