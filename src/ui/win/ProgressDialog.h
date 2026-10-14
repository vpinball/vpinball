// license:GPLv3+

#pragma once

#include <wxx_wincore.h> // Needed first: wxx_controls.h alone lacks the CString definition
#include <wxx_controls.h>
#include <wxx_dialog.h>
#include <wxx_stdcontrols.h> // Add CStatic

#include "ui/LoadProgress.h"

// Shows table load progress in a dialog: the LoadProgress implementation the Win32 editor hands to Player
class ProgressDialog final : public CDialog, public LoadProgress
{
public:
   ProgressDialog();

   void SetProgress(const string &text, const float value = -1.f) override;

protected:
   BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
   BOOL OnInitDialog() override;

private:
   CProgressBar m_progressBar;
   CStatic m_progressName;
};
