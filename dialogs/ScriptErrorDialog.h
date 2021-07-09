#ifndef H_SCRIPT_ERROR_DIALOG
#define H_SCRIPT_ERROR_DIALOG

#include <wxx_dialog.h>
#include <string>

/**
 * Dialog box shown when a runtime script error occurs
 */
class ScriptErrorDialog : public CDialog
{
public:
	ScriptErrorDialog(const std::wstring &message);

	virtual BOOL OnInitDialog() override;
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

	/**
	 * Call this after the dialog has been closed to determine whether the user wanted to suppress future runtime errors
	 * for this session
	 */
	inline bool WasSuppressErrorsRequested()
	{
		return shouldSuppressErrors;
	}

private:
	/**
	 * The error message shown on the dialog when it is created
	 */
	std::wstring initMessage;

	/**
	 * Set after either continue or stop are pressed
	 */
	bool shouldSuppressErrors = false;
};

#endif
