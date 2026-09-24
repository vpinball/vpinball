// license:GPLv3+

#pragma once

#include <chrono>

// Reports table load progress, by logging it at a limited rate. The Win32 editor instead shows it in a
// dialog through its ProgressDialog implementation. The reporter is provided by the caller to Player.
class LoadProgress
{
public:
   virtual ~LoadProgress() = default;

   virtual void SetProgress(const string &text, const float value = -1.f);
   float GetProgress() const { return m_progress; }

protected:
   float m_progress = 0.f;

private:
   std::chrono::steady_clock::time_point m_lastLogTick;
   string m_lastLogText;
};
