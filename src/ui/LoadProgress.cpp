// license:GPLv3+

#include "core/stdafx.h"
#include "LoadProgress.h"

void LoadProgress::SetProgress(const string &text, const float value)
{
   // A call without a value only updates the text, which there is nothing to update here
   if (value < 0.f)
      return;

   if (m_progress != value)
   {
      // Rate limited, as this is called once per texture of the table while loading them
      const auto now = std::chrono::steady_clock::now();
      if (now - m_lastLogTick >= std::chrono::seconds(1) || text != m_lastLogText)
      {
         PLOGI.printf("%s %d%%", text.c_str(), (int)value);
         m_lastLogTick = now;
         m_lastLogText = text;
      }
   }
   m_progress = value;
}
