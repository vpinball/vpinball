// license:GPLv3+

#pragma once

#include <wxx_controls.h>
#include <wxx_dialog.h>

// Shows table load progress in a dialog. Offers the same SetProgress/GetProgress pair as LoadProgress, which is
// what Player falls back to when built without this editor, see Player::m_loadProgress
class ProgressDialog final : public CDialog
{
public:
   ProgressDialog();

   void SetProgress(const string &text, const float value = -1.f);
   float GetProgress() const { return m_progress; }

protected:
   BOOL OnCommand(WPARAM wparam, LPARAM lparam) override;
   BOOL OnInitDialog() override;

private:
   float m_progress = 0.f;
   CProgressBar m_progressBar;
   CStatic m_progressName;
};
