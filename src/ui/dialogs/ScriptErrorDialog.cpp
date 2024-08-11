// license:GPLv3+

#include "core/stdafx.h"
#include "ScriptErrorDialog.h"

ScriptErrorDialog::ScriptErrorDialog(const wstring &message) :
	CDialog(IDD_SCRIPT_ERROR),
	initMessage(message)
{}

BOOL ScriptErrorDialog::OnInitDialog()
{
	const HWND textBoxHwnd = GetDlgItem(IDC_ERROR_EDIT);

	::SendMessage(textBoxHwnd, WM_SETFONT, (size_t)GetStockObject(ANSI_FIXED_FONT), 0);
	::SetWindowTextW(textBoxHwnd, initMessage.c_str());

	if (shouldHideInstallDebuggerText)
	{
		::ShowWindow(GetDlgItem(IDC_STATIC_INSTALL_DEBUGGER), SW_HIDE);
	}

	return TRUE;
}

BOOL ScriptErrorDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (LOWORD(wParam))
	{
		case IDC_SCRIPT_CONTINUE:
		{
			shouldSuppressErrors = IsDlgButtonChecked(IDC_SUPPRESS_ERRORS_CHECK) == BST_CHECKED;
			Close();
			return TRUE;
		}
		case IDC_SCRIPT_STOP:
		{
			shouldSuppressErrors = true;
			g_pvp->QuitPlayer(0);
			Close();
			return TRUE;
		}
	}

	return FALSE;
}

void ScriptErrorDialog::HideInstallDebuggerText()
{
	shouldHideInstallDebuggerText = true;
}
