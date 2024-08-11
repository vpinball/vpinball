// license:GPLv3+

#pragma once

#include <wxx_dialog.h>
#include <string>

// Dialog box shown when a runtime script error occurs
class ScriptErrorDialog : public CDialog
{
public:
	ScriptErrorDialog(const wstring &message);

	virtual BOOL OnInitDialog() override;
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

	// Call this after the dialog has been closed to determine whether the user wanted to suppress future runtime errors for this session
	inline bool WasSuppressErrorsRequested()
	{
		return shouldSuppressErrors;
	}

	// Hides the prompt to install a script debugger.
	// Call this if detailed debugging info is available.
	void HideInstallDebuggerText();

private:
	// The error message shown on the dialog when it is created
	wstring initMessage;

	// Set after either continue or stop are pressed
	bool shouldSuppressErrors = false;

	// If set to true before the window is shown, the prompt to install a script debugger for more detailed error information is hidden.
	bool shouldHideInstallDebuggerText = false;
};
