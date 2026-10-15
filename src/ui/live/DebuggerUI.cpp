// license:GPLv3+

#include "core/stdafx.h"
#include "DebuggerUI.h"

#include "core/player.h"
#include "core/Settings.h"
#include "fonts/ForkAwesome.h"
#include "imgui/imgui_stdlib.h"
#include "ui/live/LiveUI.h"


void DebuggerUI::Open()
{
   if (m_isOpened)
      return;
   m_isOpened = true;
   m_applyGeometry = true;
   m_scrollToBottom = true;
   m_ballSize = g_settingsService.GetAppSettings().GetEditor_ThrowBallSize();
   m_ballMass = g_settingsService.GetAppSettings().GetEditor_ThrowBallMass();
}

void DebuggerUI::Close()
{
   if (!m_isOpened)
      return;
   m_isOpened = false;
   SaveWindowGeometry();
}

void DebuggerUI::SaveWindowGeometry() const
{
   if (m_lastSize.x <= 0.f) // Not rendered yet
      return;
   Settings &settings = g_settingsService.GetAppSettings();
   settings.SetEditor_DebuggerPosX(static_cast<int>(m_lastPos.x), false);
   settings.SetEditor_DebuggerPosY(static_cast<int>(m_lastPos.y), false);
   settings.SetEditor_DebuggerWidth(static_cast<int>(m_lastSize.x), false);
   settings.SetEditor_DebuggerHeight(static_cast<int>(m_lastSize.y), false);
}

void DebuggerUI::AddLog(const string &line)
{
   m_log.emplace_back(line);
   if (m_log.size() > MAX_LOG_LINES)
      m_log.pop_front();
   if (m_autoScroll)
      m_scrollToBottom = true;
}

void DebuggerUI::EvaluateCommand()
{
   const string cmd = std::move(m_input);
   m_input.clear();
   if (cmd.empty())
      return;
   m_history.push_back(cmd);
   m_historyPos = -1;
   AddLog("> "s + cmd);
   if (g_pplayer->m_scriptInterpreter)
      g_pplayer->m_scriptInterpreter->Evaluate(cmd, true);
}

int DebuggerUI::HistoryCallback(ImGuiInputTextCallbackData *data)
{
   if (data->EventFlag != ImGuiInputTextFlags_CallbackHistory)
      return 0;
   DebuggerUI *const self = static_cast<DebuggerUI *>(data->UserData);
   if (self->m_history.empty())
      return 0;
   if (data->EventKey == ImGuiKey_UpArrow)
   {
      if (self->m_historyPos == -1)
         self->m_historyPos = static_cast<int>(self->m_history.size()) - 1;
      else if (self->m_historyPos > 0)
         self->m_historyPos--;
   }
   else if (data->EventKey == ImGuiKey_DownArrow)
   {
      if (self->m_historyPos != -1 && ++self->m_historyPos >= static_cast<int>(self->m_history.size()))
         self->m_historyPos = -1;
   }
   else
      return 0;
   data->DeleteChars(0, data->BufTextLen);
   if (self->m_historyPos >= 0)
      data->InsertChars(0, self->m_history[self->m_historyPos].c_str());
   return 0;
}

void DebuggerUI::Update()
{
   if (!m_isOpened || g_pplayer == nullptr)
      return;
   Player *const player = g_pplayer;
   const float uiScale = m_liveUI.GetDPI();

   if (m_applyGeometry)
   {
      m_applyGeometry = false;
      Settings &settings = g_settingsService.GetAppSettings();
      ImGui::SetNextWindowPos(ImVec2(static_cast<float>(settings.GetEditor_DebuggerPosX()), static_cast<float>(settings.GetEditor_DebuggerPosY())));
      const float w = static_cast<float>(settings.GetEditor_DebuggerWidth());
      const float h = static_cast<float>(settings.GetEditor_DebuggerHeight());
      ImGui::SetNextWindowSize(ImVec2(w > 32.f ? w : 500.f * uiScale, h > 32.f ? h : 300.f * uiScale));
   }

   bool open = true;
   if (ImGui::Begin("Debugger", &open))
   {
      // Play controls (replicates the push-like play/pause/step buttons of the former dialog)
      const bool isPlaying = player->IsPlaying(false);
      const bool isStepping = player->m_pauseTimeTarget > 0;
      if (ImGui::RadioButton(ICON_FK_PLAY " Play", isPlaying && !isStepping))
         player->SetPlayState(true);
      ImGui::SameLine();
      if (ImGui::RadioButton(ICON_FK_PAUSE " Pause", !isPlaying))
         player->SetPlayState(false);
      ImGui::SameLine();
      if (ImGui::RadioButton(ICON_FK_STEP_FORWARD " Step", isStepping))
         player->SetPlayState(true, static_cast<uint32_t>(max(1, m_stepMs)));
      ImGui::SameLine();
      ImGui::SetNextItemWidth(60.f * uiScale);
      ImGui::InputInt("##StepAmount", &m_stepMs, 0, 0);
      m_stepMs = max(1, m_stepMs);
      ImGui::SameLine();
      ImGui::TextUnformatted("ms");

      // Ball control
      const BallControl::Mode mode = m_liveUI.m_ballControl.GetMode();
      bool throwBalls = (mode == BallControl::Mode::ThrowDraggedBall) || (mode == BallControl::Mode::ThrowNewBall);
      bool ballControl = (mode == BallControl::Mode::DragBall) || (mode == BallControl::Mode::ThrowDraggedBall);
      bool ballChanged = ImGui::Checkbox("Throw Balls in Player", &throwBalls);
      ImGui::SameLine();
      ballChanged |= ImGui::Checkbox("Ball Control", &ballControl);
      if (ballChanged)
         m_liveUI.m_ballControl.SetMode(ballControl, throwBalls);
      ImGui::SetNextItemWidth(80.f * uiScale);
      // Applied on change, as balls are thrown with the current settings
      if (ImGui::InputInt("Ball Size", &m_ballSize, 1, 10))
      {
         m_ballSize = clamp(m_ballSize, 5, 500);
         g_settingsService.GetAppSettings().SetEditor_ThrowBallSize(m_ballSize, false);
      }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(80.f * uiScale);
      if (ImGui::InputFloat("Mass", &m_ballMass, 0.1f, 1.f, "%.2f"))
      {
         m_ballMass = clamp(m_ballMass, 0.05f, 100.f);
         g_settingsService.GetAppSettings().SetEditor_ThrowBallMass(m_ballMass, false);
      }

      // Console output
      ImGui::Separator();
      if (ImGui::Button(ICON_FK_TRASH_O " Clear"))
         m_log.clear();
      ImGui::SameLine();
      ImGui::Checkbox("Auto-scroll", &m_autoScroll);
      ImGui::BeginChild("LogPane", ImVec2(0.f, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
      ImGuiListClipper clipper;
      clipper.Begin(static_cast<int>(m_log.size()));
      while (clipper.Step())
         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            ImGui::TextUnformatted(m_log[i].c_str());
      if (m_scrollToBottom)
      {
         ImGui::SetScrollHereY(1.0f);
         m_scrollToBottom = false;
      }
      ImGui::EndChild();

      // Immediate script evaluation
      ImGui::SetNextItemWidth(-1.f);
      if (ImGui::InputTextWithHint("##Eval", "VBScript statement (Enter to evaluate)", &m_input, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory, &DebuggerUI::HistoryCallback, this))
      {
         EvaluateCommand();
         ImGui::SetKeyboardFocusHere(-1);
      }
   }
   m_lastPos = ImGui::GetWindowPos();
   m_lastSize = ImGui::GetWindowSize();
   ImGui::End();

   // Keep the window reachable: it may never leave the display entirely, keeping at least part of the title bar inside the work area to grab it
   const ImGuiViewport *const viewport = ImGui::GetMainViewport();
   if (viewport->WorkSize.x > 0.f && viewport->WorkSize.y > 0.f)
   {
      const ImVec2 workMin = viewport->WorkPos;
      const ImVec2 workMax(viewport->WorkPos.x + viewport->WorkSize.x, viewport->WorkPos.y + viewport->WorkSize.y);
      const float titleBarHeight = ImGui::GetFrameHeight();
      const float minVisibleX = min(m_lastSize.x, titleBarHeight + 48.f * uiScale); // close button width + a grabbable area
      ImVec2 pos = m_lastPos;
      pos.x = clamp(pos.x, workMin.x - m_lastSize.x + minVisibleX, workMax.x - minVisibleX);
      pos.y = clamp(pos.y, workMin.y, workMax.y - titleBarHeight);
      if (pos.x != m_lastPos.x || pos.y != m_lastPos.y)
      {
         ImGui::SetWindowPos("Debugger", pos);
         m_lastPos = pos;
      }
   }
   if (!open)
      Close();
}
