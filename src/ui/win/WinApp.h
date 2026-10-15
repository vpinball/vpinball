// license:GPLv3+

#pragma once

#include <wxx_appcore.h> // CWinApp

#include "core/VPApp.h"

// The win32++ application object: owns the module instance handle and runs the message loop.
// Defined out of class here so that the win32xx dependency stays inside the Win32 UI module.
class VPApp::WinApp final : public CWinApp
{
public:
   WinApp() = default;
#ifdef VPX_ENABLE_WIN32_EDITOR
protected:
   BOOL OnIdle(LONG) override;
   BOOL PreTranslateMessage(MSG& msg) override;
#endif
};
