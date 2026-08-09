#pragma once

#ifndef __STANDALONE__
#include <wxx_controls.h>
#include <wxx_dialog.h>
#else
#include <chrono>
#endif

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
#ifndef __STANDALONE__
   CProgressBar m_progressBar;
   CStatic m_progressName;
#else
   std::chrono::steady_clock::time_point m_lastLogTick;
   std::string m_lastLogText;
#endif
};
