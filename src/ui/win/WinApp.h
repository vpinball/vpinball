// license:GPLv3+

#pragma once

#include <wxx_appcore.h> // CWinApp


// The win32++ application object: owns the module instance handle and runs the editor message loop.
// Created by Win32EditCommand before any wxx window (CWinApp must exist before the first CWnd).
class WinApp final : public CWinApp
{
public:
   WinApp() = default;

protected:
   BOOL OnIdle(LONG) override;
   BOOL PreTranslateMessage(MSG& msg) override;
};
