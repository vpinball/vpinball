#include "stdafx.h"
#include "resource.h"
#include <windowsx.h>
#include "Debugger.h"

#define RECOMPUTEBUTTONCHECK WM_USER+100
#define RESIZE_FROM_EXPAND   WM_USER+101

DebuggerDialog::DebuggerDialog() : CDialog(IDD_DEBUGGER)
{
}

BOOL DebuggerDialog::IsSubDialogMessage(MSG& msg) const
{
    return IsDialogMessage(msg);
}

BOOL DebuggerDialog::OnInitDialog()
{
    AttachItem(IDC_PLAY, m_playButton);
    AttachItem(IDC_PAUSE, m_pauseButton);
    AttachItem(IDC_STEP, m_stepButton);
    AttachItem(IDC_STEPAMOUNT, m_stepAmountEdit);
    m_hThrowBallsInPlayerCheck = ::GetDlgItem(GetHwnd(), IDC_BALL_THROWING);
    m_hBallControlCheck = ::GetDlgItem(GetHwnd(), IDC_BALL_CONTROL);
    AttachItem(IDC_THROW_BALL_SIZE_EDIT2, m_ballSizeEdit);
    AttachItem(IDC_THROW_BALL_MASS_EDIT2, m_ballMassEdit);
    AttachItem(IDC_EDITSIZE, m_notesEdit);

    HANDLE hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_PLAY), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_playButton.SetIcon((HICON)hIcon);

    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_PAUSE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_pauseButton.SetIcon((HICON)hIcon);
    hIcon = ::LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_STEP), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    m_stepButton.SetIcon((HICON)hIcon);

    SendMessage(RECOMPUTEBUTTONCHECK, 0, 0);

    RECT rcEditSize;
    ::GetWindowRect(GetDlgItem(IDC_EDITSIZE), &rcEditSize);
    ::ScreenToClient(GetHwnd(), (POINT*)&rcEditSize);
    ::ScreenToClient(GetHwnd(), &((POINT*)&rcEditSize)[1]);

    g_pplayer->m_hwndDebugOutput = CreateWindowEx(0, "Scintilla", "",
                                                WS_CHILD | ES_NOHIDESEL | WS_VISIBLE | ES_SUNKEN | WS_HSCROLL | WS_VSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_BORDER,
                                                rcEditSize.left, rcEditSize.top, rcEditSize.right - rcEditSize.left, rcEditSize.bottom - rcEditSize.top, GetHwnd(), nullptr, g_pvp->theInstance, 0);

    SendMessage(g_pplayer->m_hwndDebugOutput, SCI_STYLESETSIZE, 32, 10);
    SendMessage(g_pplayer->m_hwndDebugOutput, SCI_STYLESETFONT, 32, (LPARAM)"Courier");

    SendMessage(g_pplayer->m_hwndDebugOutput, SCI_SETMARGINWIDTHN, 1, 0);

    SendMessage(g_pplayer->m_hwndDebugOutput, SCI_SETTABWIDTH, 4, 0);

    SendMessage(m_hThrowBallsInPlayerCheck, BM_SETCHECK, g_pplayer->m_throwBalls ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(m_hBallControlCheck, BM_SETCHECK, g_pplayer->m_ballControl ? BST_CHECKED : BST_UNCHECKED, 0);

    m_ballSizeEdit.SetWindowText(std::to_string(g_pplayer->m_debugBallSize).c_str());

    m_ballMassEdit.SetWindowText(f2sz(g_pplayer->m_debugBallMass).c_str());

    m_resizer.Initialize(*this, GetWindowRect());
    m_resizer.AddChild(m_notesEdit.GetHwnd(), CResizer::bottomright, RD_STRETCH_HEIGHT | RD_STRETCH_WIDTH);
    m_resizer.AddChild(g_pplayer->m_hwndDebugOutput, CResizer::bottomright, RD_STRETCH_WIDTH | RD_STRETCH_HEIGHT);
    m_resizer.AddChild(GetDlgItem(IDC_GUIDE1).GetHwnd(), CResizer::topleft, 0);
    m_resizer.AddChild(GetDlgItem(IDC_GUIDE2).GetHwnd(), CResizer::bottomright, 0);

    LoadPosition();

    return TRUE;
}

BOOL DebuggerDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (LOWORD(wParam))
    {
        case IDC_PLAY:
        {
#ifdef STEPPING
            g_pplayer->m_pauseTimeTarget = 0;
#endif
            g_pplayer->m_userDebugPaused = false;
            g_pplayer->RecomputePseudoPauseState();
            SendMessage(RECOMPUTEBUTTONCHECK, 0, 0);
            return TRUE;
        }
        case IDC_PAUSE:
        {
#ifdef STEPPING
            g_pplayer->m_pauseTimeTarget = 0;
#endif
            g_pplayer->m_userDebugPaused = true;
            g_pplayer->RecomputePseudoPauseState();
            SendMessage(RECOMPUTEBUTTONCHECK, 0, 0);
            return TRUE;
        }
        case IDC_STEP:
        {
#ifdef STEPPING
            const int ms = GetDlgItemInt(IDC_STEPAMOUNT, FALSE);
            g_pplayer->m_pauseTimeTarget = g_pplayer->m_time_msec + ms;
#endif
            g_pplayer->m_userDebugPaused = false;
            g_pplayer->RecomputePseudoPauseState();
            SendMessage(RECOMPUTEBUTTONCHECK, 0, 0);
            return TRUE;
        }
        case IDC_EXPAND:
        {
            SendMessage(RESIZE_FROM_EXPAND, 0, 0);
            return TRUE;
        }
        case IDC_BALL_THROWING:
        {
            const size_t checked = SendMessage(m_hThrowBallsInPlayerCheck, BM_GETCHECK, 0, 0);
            g_pplayer->m_throwBalls = !!checked;
            return TRUE;
        }
        case IDC_BALL_CONTROL:
        {
            const size_t checked = SendMessage(m_hBallControlCheck, BM_GETCHECK, 0, 0);
            g_pplayer->m_ballControl = !!checked;
            return TRUE;
        }
    }
    return FALSE;
}

void DebuggerDialog::OnClose()
{
#ifdef STEPPING
    g_pplayer->m_pauseTimeTarget = 0;
#endif
    g_pplayer->m_userDebugPaused = false;
    g_pplayer->RecomputePseudoPauseState();
    g_pplayer->m_debugBallSize = GetDlgItemInt(IDC_THROW_BALL_SIZE_EDIT2, FALSE);

    const float fv = sz2f(GetDlgItemText(IDC_THROW_BALL_MASS_EDIT2).c_str());
    g_pplayer->m_debugBallMass = fv;

    g_pplayer->m_debugMode = false;
    g_pplayer->m_showDebugger = false;
    ShowWindow(SW_HIDE);
    if ((g_pplayer->m_fullScreen || (g_pplayer->m_wnd_width == g_pplayer->m_screenwidth && g_pplayer->m_wnd_height == g_pplayer->m_screenheight)) // detect windowed fullscreen
        && !(g_pplayer->m_throwBalls || g_pplayer->m_ballControl || g_pplayer->m_liveUI->IsOpened()))
    {
        while (ShowCursor(TRUE)<0) ;
        while (ShowCursor(FALSE)>=0) ;
    }

    SavePosition();
}

void DebuggerDialog::LoadPosition()
{
   const CRect rcMain = GetParent().GetWindowRect();
   const CRect rcDialog = GetWindowRect();
   const int x = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "DebuggerPosX"s, (int) ((rcMain.right + rcMain.left) / 2 - (rcDialog.right - rcDialog.left) / 2));
   const int y = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "DebuggerPosY"s, (int) ((rcMain.bottom + rcMain.top) / 2 - (rcDialog.bottom - rcDialog.top) / 2));
   const int w = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "DebuggerWidth"s, 1000);
   const int h = g_pvp->m_settings.LoadValueWithDefault(Settings::Editor, "DebuggerHeight"s, 800);
   POINT p { x, y };
   if (MonitorFromPoint(p, MONITOR_DEFAULTTONULL) != NULL) // Do not apply if point is offscreen
      SetWindowPos(nullptr, x, y, w, h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);
}

void DebuggerDialog::SavePosition()
{
   const CRect rect = GetWindowRect();
   g_pvp->m_settings.SaveValue(Settings::Editor, "DebuggerPosX"s, (int)rect.left);
   g_pvp->m_settings.SaveValue(Settings::Editor, "DebuggerPosY"s, (int)rect.top);
   const int w = rect.right - rect.left;
   g_pvp->m_settings.SaveValue(Settings::Editor, "DebuggerWidth"s, w);
   const int h = rect.bottom - rect.top;
   g_pvp->m_settings.SaveValue(Settings::Editor, "DebuggerHeight"s, h);
}

LRESULT DebuggerDialog::OnNotify(WPARAM wparam, LPARAM lparam)
{
    const NMHDR* const pnmh = (LPNMHDR)lparam;
    //HWND hwndRE = pnmh->hwndFrom;
    const int code = pnmh->code;

    switch (code)
    {
        case SCN_CHARADDED:
        {
            const SCNotification* const pscnmh = (SCNotification*)lparam;
            if (pscnmh->ch == '\n') // execute code
            {
                SendMessage(pnmh->hwndFrom, SCI_DELETEBACK, 0, 0);

                const size_t curpos = SendMessage(pnmh->hwndFrom, SCI_GETCURRENTPOS, 0, 0);
                const size_t line = SendMessage(pnmh->hwndFrom, SCI_LINEFROMPOSITION, curpos, 0);
                const size_t lineStart = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line, 0);
                const size_t lineEnd = SendMessage(pnmh->hwndFrom, SCI_GETLINEENDPOSITION, line, 0);

                char* const szText = new char[lineEnd - lineStart + 1];
                Sci_TextRange tr;
                tr.chrg.cpMin = (Sci_PositionCR)lineStart;
                tr.chrg.cpMax = (Sci_PositionCR)lineEnd;
                tr.lpstrText = szText;
                SendMessage(pnmh->hwndFrom, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

                const size_t maxlines = SendMessage(pnmh->hwndFrom, SCI_GETLINECOUNT, 0, 0);

                if (maxlines == line + 1)
                {
                    // need to add a new line to the end
                    SendMessage(pnmh->hwndFrom, SCI_DOCUMENTEND, 0, 0);
                    SendMessage(pnmh->hwndFrom, SCI_ADDTEXT, 1, (LPARAM)"\n");
                }
                else
                {
                    const size_t pos = SendMessage(pnmh->hwndFrom, SCI_POSITIONFROMLINE, line + 1, 0);
                    SendMessage(pnmh->hwndFrom, SCI_SETCURRENTPOS, pos, 0);
                }

                g_pplayer->m_ptable->m_pcv->EvaluateScriptStatement(szText);
                delete[] szText;
            }
            break;
        }
    }
    return CDialog::OnNotify(wparam, lparam);
}

INT_PTR DebuggerDialog::DialogProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    m_resizer.HandleMessage(uMsg, wParam, lParam);

    switch (uMsg)
    {
        case RECOMPUTEBUTTONCHECK:
        {
            int PlayDown = BST_UNCHECKED;
            int PauseDown = BST_UNCHECKED;
            int StepDown = BST_UNCHECKED;

            if (g_pplayer->m_userDebugPaused)
            {
                PauseDown = BST_CHECKED;
            }
#ifdef STEPPING
            else if (g_pplayer->m_pauseTimeTarget > 0)
            {
                StepDown = BST_CHECKED;
            }
#endif
            else
            {
                PlayDown = BST_CHECKED;
            }

            SendDlgItemMessage(IDC_PLAY, BM_SETCHECK, PlayDown, 0);
            SendDlgItemMessage(IDC_PAUSE, BM_SETCHECK, PauseDown, 0);
            SendDlgItemMessage(IDC_STEP, BM_SETCHECK, StepDown, 0);
            SendMessage(TB_CHECKBUTTON, IDC_PLAY, PlayDown);
            SendMessage(TB_CHECKBUTTON, IDC_PAUSE, PauseDown);
            SendMessage(TB_CHECKBUTTON, IDC_STEP, StepDown);
            return TRUE;
        }

        case WM_SIZE:
        {
            const CRect rc = m_notesEdit.GetClientRect();
            ::SetWindowPos(g_pplayer->m_hwndDebugOutput, nullptr,
                0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
        }
    }
    return DialogProcDefault(uMsg, wParam, lParam);
}
