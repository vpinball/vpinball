// license:GPLv3+

#pragma once

#include <chrono>

// Reports table load progress when there is no window to show it in, by logging it at a limited rate.
// ProgressDialog is the Win32 editor counterpart, offering the same SetProgress/GetProgress pair, see Player
class LoadProgress
{
public:
   void SetProgress(const string &text, const float value = -1.f);
   float GetProgress() const { return m_progress; }

private:
   float m_progress = 0.f;
   std::chrono::steady_clock::time_point m_lastLogTick;
   string m_lastLogText;
};
