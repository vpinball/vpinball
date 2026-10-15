// license:GPLv3+

#pragma once

#include "imgui/imgui.h"

#include <deque>

class LiveUI;

// In-game debugger tool window: play/pause/step controls, ball control toggles and a
// script console fed by the plog debug output (engine logs, Debug.Print) that evaluates
// user input through the script interpreter.
class DebuggerUI final
{
public:
   explicit DebuggerUI(LiveUI &liveUI)
      : m_liveUI(liveUI)
   {
   }
   ~DebuggerUI() { Close(); } // Saves the window geometry if the player closes while it is opened

   void Open();
   void Close();
   bool IsOpened() const { return m_isOpened; }

   // Appends a line to the console output, also while the window is closed.
   // Called by the plog appender, from the UI thread only.
   void AddLog(const string &line);

   void Update(); // Renders the window when opened, called every frame from LiveUI::RenderUI()

private:
   void SaveWindowGeometry() const;
   void EvaluateCommand();
   static int HistoryCallback(ImGuiInputTextCallbackData *data);

   LiveUI &m_liveUI;

   bool m_isOpened = false;
   bool m_applyGeometry = false; // Apply persisted window position/size on next render
   ImVec2 m_lastPos {};
   ImVec2 m_lastSize {};

   // Console output (bounded log of plog records)
   static constexpr size_t MAX_LOG_LINES = 4000;
   std::deque<string> m_log;
   bool m_scrollToBottom = false;
   bool m_autoScroll = true;

   // Immediate script evaluation input line with history
   string m_input;
   vector<string> m_history;
   int m_historyPos = -1;

   int m_stepMs = 10; // Timed simulation step duration for the Step button

   int m_ballSize = 50; // Edited value of the Editor.ThrowBallSize setting, written back on each change
   float m_ballMass = 1.f; // Edited value of the Editor.ThrowBallMass setting, written back on each change
};
