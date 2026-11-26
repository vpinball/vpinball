// license:GPLv3+

#pragma once

#include <atomic>
#include <chrono>

// Reports table load progress, by logging it at a limited rate. The Win32 editor instead shows it in a
// dialog through its ProgressDialog implementation. The reporter is provided by the caller to Player.
class LoadProgress
{
public:
   virtual ~LoadProgress() = default;

   virtual void SetProgress(const string &text, const float value = -1.f);
   float GetProgress() const { return m_progress.load(std::memory_order_relaxed); }

protected:
   std::atomic<float> m_progress { 0.f }; // Set by the texture loading worker threads, read by the loading UI

private:
   std::chrono::steady_clock::time_point m_lastLogTick;
   string m_lastLogText;
};
