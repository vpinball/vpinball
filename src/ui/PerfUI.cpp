// license:GPLv3+

#include "core/stdafx.h"

#include "PerfUI.h"
#include "utils/wintimer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "implot/implot.h"


PerfUI::PlotData::PlotData()
   : m_maxSize(500)
{
   m_data.reserve(m_maxSize);
   SetRolling(!m_rolling);
}

void PerfUI::PlotData::SetRolling(bool rolling)
{
   if (rolling == m_rolling)
      return;
   if (!m_data.empty())
      m_data.shrink(0);
   m_offset = 0;
   m_rolling = rolling;
}

void PerfUI::PlotData::AddPoint(const float x, const float y)
{
   if (std::isinf(y))
      return;

   if (m_rolling)
   {
      const float xmod = fmodf(x, m_timeSpan);
      if (!m_data.empty() && m_data.back().x == xmod)
         return;

      if (y > m_movingMax)
         m_movingMax = y;
      else
         m_movingMax = lerp(m_movingMax, y, 0.01f);

      if (!m_data.empty() && xmod < m_data.back().x)
         m_data.shrink(0);

      m_data.push_back(ImVec2(xmod, y));
   }
   else
   {
      if (!m_data.empty() && m_data.back().x == x)
         return;

      if (y > m_movingMax)
         m_movingMax = y;
      else
         m_movingMax = lerp(m_movingMax, y, 0.01f);

      if (m_data.size() < m_maxSize)
         m_data.push_back(ImVec2(x, y));
      else
      {
         m_data[m_offset] = ImVec2(x, y);
         m_offset++;
         if (m_offset == m_maxSize)
            m_offset = 0;
      }
   }
}

bool PerfUI::PlotData::HasData() const
{
   return !m_data.empty();
}

ImVec2 PerfUI::PlotData::GetLast() const
{
   if (m_data.empty())
      return ImVec2 { 0.f, 0.f };
   else if (m_data.size() < m_maxSize || m_offset == 0)
      return m_data.back();
   else
      return m_data[m_offset - 1];
}

float PerfUI::PlotData::GetMovingMax() const
{
   return m_movingMax;
}



PerfUI::PerfUI(Player *const player)
   : m_player(player)
{
   ImPlot::CreateContext();
   m_showPerf = (PerfMode)g_pvp->m_settings.LoadValueInt(Settings::Player, "ShowFPS"s);
}

PerfUI::~PerfUI()
{
   g_pvp->m_settings.SaveValue(Settings::Player, "ShowFPS"s, m_showPerf);
   ImPlot::DestroyContext();
}

void PerfUI::NextPerfMode()
{
   m_showPerf = (PerfMode) ((m_showPerf + 1) % 3);
   if (m_showPerf == 1)
   {
      m_player->InitFPS();
      m_player->m_logicProfiler.EnableWorstFrameLogging(true);
   }
   if (m_showPerf == PerfMode::PM_DISABLED)
      m_player->m_renderer->m_renderDevice->LogNextFrame();
}

void PerfUI::Update()
{
   if (m_showPerf == PerfMode::PM_DISABLED)
      return;

   ImGuiIO &io = ImGui::GetIO();
   constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowBgAlpha(0.5f);
   if (m_player->m_vrDevice)
   {
      if (m_showPerf == PerfMode::PM_STATS)
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.3f, io.DisplaySize.y * 0.35f), 0, ImVec2(0.f, 0.f));
      else
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.35f), 0, ImVec2(0.5f, 0.f));
   }
   else
      ImGui::SetNextWindowPos(ImVec2(8.f * m_dpi, io.DisplaySize.y - 8.f * m_dpi), 0, ImVec2(0.f, 1.f));

   ImGui::Begin("FPS", nullptr, window_flags);

   // Frame sequence graph
   if (m_showPerf == PerfMode::PM_STATS)
   {
      ImGuiWindow *const window = ImGui::GetCurrentWindow();
      static const string infoLabels[] = {
         "Misc"s,
         "Script"s,
         "Physics"s,
         "Sleep"s,
         "Prepare Frame"s,
         "Custom 1"s,
         "Custom 2"s,
         "Custom 3"s,
         // Render thread
         "Render Wait"s,
         "Render Submit"s,
         "Render Flip"s,
         "Render Sleep"s,
      };
      static const string infoLabels2[] = {
         "Misc"s,
         "Script"s,
         "Physics"s,
         "Sleep"s,
         "(Prepare frame after next, including script calls)"s,
         "Custom 1"s,
         "Custom 2"s,
         "Custom 3"s,
         // Render thread
         "(Wait for logic thread to prepare a frame)"s,
         "(Submit to driver next frame)"s,
         "(Wait for GPU and display sync to finish current frame)"s,
         "(Sleep to synchronise on user selected FPS)"s,
      };
      static constexpr FrameProfiler::ProfileSection sections[] = {
         FrameProfiler::PROFILE_PREPARE_FRAME,
         FrameProfiler::PROFILE_RENDER_WAIT,
         FrameProfiler::PROFILE_RENDER_SUBMIT,
         FrameProfiler::PROFILE_RENDER_FLIP,
         FrameProfiler::PROFILE_RENDER_SLEEP,
         FrameProfiler::PROFILE_RENDER_SUBMIT, // For BGFX, since BGFX performs CPU->GPU submit after flip
      };
      static constexpr ImU32 cols[] = {
         IM_COL32(128, 255, 128, 255), // Prepare
         IM_COL32(255,   0,   0, 255), // Wait for Render Frame
         IM_COL32(  0, 128, 255, 255), // Submit (VPX->BGFX)
         IM_COL32(  0, 192, 192, 255), // Flip (wait for GPU and sync on display)
         IM_COL32(128, 128, 128, 255), // Sleep
         IM_COL32(  0, 128, 255, 255), // Submit (BGFX->Driver)
      };
      //const ImU32 col_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x77FFFFFF;
      //const ImU32 col_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x77FFFFFF;
      //const ImU32 col_outline_base = ImGui::GetColorU32(ImGuiCol_PlotHistogram) & 0x7FFFFFFF;
      //const ImU32 col_outline_hovered = ImGui::GetColorU32(ImGuiCol_PlotHistogramHovered) & 0x7FFFFFFF;
      ImGuiContext &g = *GImGui;
      const ImGuiStyle &style = g.Style;
      ImVec2 graph_size;
      const auto blockHeight = ImGui::GetTextLineHeight() + (style.FramePadding.y * 2);
      if (graph_size.x == 0.0f)
         graph_size.x = ImGui::CalcItemWidth();
      if (graph_size.y == 0.0f)
      #ifdef ENABLE_BGFX
         graph_size.y = (style.FramePadding.y * 1) + blockHeight * 2;
      #else
         graph_size.y = (style.FramePadding.y * 1) + blockHeight * 1;
      #endif
      const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + graph_size);
      const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
      const ImRect total_bb(frame_bb.Min, frame_bb.Max);
      ImGui::ItemSize(total_bb, style.FramePadding.y);
      if (ImGui::ItemAdd(total_bb, 0, &frame_bb))
      {
         ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 0.5f), true, style.FrameRounding);
         uint64_t minTS = UINT64_MAX;
         //uint64_t maxTS = 0;
         for (int i = 0; i < 5; i++)
         {
            FrameProfiler *profiler = i == 0 ? &m_player->m_logicProfiler : m_player->m_renderProfiler;
            if (profiler->GetPrevStart(sections[i]) == 0)
               continue;
            minTS = std::min(minTS, static_cast<uint64_t>(profiler->GetPrevStart(sections[i])));
            //maxTS = max(maxTS, static_cast<uint64_t>(profiler->GetPrevEnd(sections[i])));
         }
         const float elapse = static_cast<float>(m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME)) * 1.5f;
         const float width = inner_bb.Max.x - inner_bb.Min.x;
         for (int i = 0; i < 6; i++)
         {
            FrameProfiler *profiler = i == 0 ? &m_player->m_logicProfiler : m_player->m_renderProfiler;
            if (profiler->GetPrevStart(sections[i]) == 0)
               continue;
            float start = static_cast<float>(profiler->GetPrevStart(sections[i]) - minTS) / elapse;
            float end = static_cast<float>(profiler->GetPrevEnd(sections[i]) - minTS) / elapse;
            #ifdef ENABLE_BGFX
            const float height = blockHeight * (i == 0 ? 1.f : 0.f) - style.FramePadding.y;
            if (i == 5)
            {
               // For BGFX submit is done in 2 parts: VPX->BGFX, then BGFX->GPU after flip
               const uint64_t submit1 = profiler->GetPrevEnd(FrameProfiler::PROFILE_RENDER_SUBMIT) - profiler->GetPrevStart(FrameProfiler::PROFILE_RENDER_SUBMIT);
               const uint64_t submit2 = profiler->GetPrev(FrameProfiler::PROFILE_RENDER_SUBMIT) - submit1;
               start = static_cast<float>(profiler->GetPrevEnd(FrameProfiler::PROFILE_RENDER_FLIP) - minTS) / elapse;
               end = static_cast<float>(profiler->GetPrevEnd(FrameProfiler::PROFILE_RENDER_FLIP) - minTS + submit2) / elapse;
            }
            #else
            const float height = blockHeight * 0 - style.FramePadding.y;
            #endif
            const ImVec2 pos0 = inner_bb.Min + ImVec2(start * width, height);
            const ImVec2 pos1 = inner_bb.Min + ImVec2(end * width, height + blockHeight * 0.9f);
            if (ImGui::IsMouseHoveringRect(pos0, pos1))
               ImGui::SetTooltip("%s: %5.1fms\n%s", infoLabels[sections[i]].c_str(), (end - start) * elapse * 1e-3f, infoLabels2[sections[i]].c_str());
            window->DrawList->AddRectFilled(pos0, pos1, cols[i]);
         }
      }
   }

   // Main frame timing table
   if ((m_showPerf == PerfMode::PM_STATS) && ImGui::BeginTable("Timings", 3, ImGuiTableFlags_Borders))
   {
      const uint32_t period = m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME);
      ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(m_showAvgFPS ? "Avg Time" : "Time", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(m_showAvgFPS ? "Avg Ratio" : "Ratio", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableHeadersRow();

      #define PROF_ROW(name, section) \
      ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::Text("%s",name); \
      if (m_showAvgFPS) { \
      ImGui::TableNextColumn(); ImGui::Text("%4.1fms", profiler->GetSlidingAvg(section) * 1e-3); \
      ImGui::TableNextColumn(); ImGui::Text("%4.1f%%", profiler->GetSlidingRatio(section) * 100.0); \
      } else { \
      ImGui::TableNextColumn(); ImGui::Text("%4.1fms", profiler->GetPrev(section) * 1e-3); \
      ImGui::TableNextColumn(); ImGui::Text("%4.1f%%", profiler->GetPrev(section) * 100.0 / period); \
      }

      const int hoveredRow = ImGui::TableGetHoveredRow();
      int renderRow = 1, logicRow = 1, rowOffset = 0;

      FrameProfiler* profiler = g_pplayer->m_renderProfiler;
      #ifdef ENABLE_BGFX
         PROF_ROW("Render Thread", FrameProfiler::PROFILE_FRAME)
         PROF_ROW("> Wait", FrameProfiler::PROFILE_RENDER_WAIT)
         if (hoveredRow == 2)
            ImGui::SetTooltip("Time spent waiting for the CPU to prepare a frame");
         else if (hoveredRow == 3)
            ImGui::SetTooltip("Time spent submitting frame from VPX to BGFX then from BGFX to GPU");
         else if (hoveredRow == 4)
            ImGui::SetTooltip("Time spent sleeping to satisfy user requested framerate");
         else if (hoveredRow == 5)
            ImGui::SetTooltip("Time spent waiting for GPU to flip (including VSYNC)");
         logicRow = 6;
         rowOffset = 2;
      #else
         PROF_ROW("Frame", FrameProfiler::PROFILE_FRAME)
      #endif
      PROF_ROW("> Submit", FrameProfiler::PROFILE_RENDER_SUBMIT)
      PROF_ROW("> Sleep", FrameProfiler::PROFILE_RENDER_SLEEP)
      PROF_ROW("> Flip", FrameProfiler::PROFILE_RENDER_FLIP)

      profiler = &m_player->m_logicProfiler;
      #ifdef ENABLE_BGFX
         PROF_ROW("Logic Thread", FrameProfiler::PROFILE_FRAME)
      #endif
      PROF_ROW("> Prepare", FrameProfiler::PROFILE_PREPARE_FRAME)
      PROF_ROW("> Physics", FrameProfiler::PROFILE_PHYSICS)
      PROF_ROW("> Script", FrameProfiler::PROFILE_SCRIPT)
      PROF_ROW("> Sleep", FrameProfiler::PROFILE_SLEEP)
      PROF_ROW("> Misc", FrameProfiler::PROFILE_MISC)
      #ifdef DEBUG
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM1) > 0) { PROF_ROW("> Debug #1", FrameProfiler::PROFILE_CUSTOM1); ImGui::TableNextRow(); }
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM2) > 0) { PROF_ROW("> Debug #2", FrameProfiler::PROFILE_CUSTOM2); ImGui::TableNextRow(); }
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM3) > 0) { PROF_ROW("> Debug #3", FrameProfiler::PROFILE_CUSTOM3); ImGui::TableNextRow(); }
      #endif

      if ((hoveredRow == renderRow) || (hoveredRow == logicRow))
         ImGui::SetTooltip("FPS: %4.1f (%4.1f average)", 1e6 / m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME), 1e6 / m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME));
      else if (hoveredRow == 5 + rowOffset)
         ImGui::SetTooltip("Time spent preparing a frame for rendering");
      else if (hoveredRow == 6 + rowOffset)
         ImGui::SetTooltip("Time spent in physics simulation\nMax: %4.1fms, %4.1fms (over last second)",
            1e-3 * m_player->m_logicProfiler.GetMax(FrameProfiler::PROFILE_PHYSICS), 1e-3 * m_player->m_physics->GetPerfLengthMax());
      else if (hoveredRow == 7 + rowOffset)
      {
         string info = m_player->m_logicProfiler.GetWorstScriptInfo();
         ImGui::SetTooltip("Time spent in game logic and script\nMax: %4.1fms, %4.1fms (over last second)\nWorst frame:\n%s",
            1e-3 * m_player->m_logicProfiler.GetMax(FrameProfiler::PROFILE_SCRIPT), 1e-3 * m_player->m_script_max,
            info.c_str());
      }

      #undef PROF_ROW

      ImGui::EndTable();
      ImGui::NewLine();
   }

   // Display simple FPS window
   #if defined(ENABLE_BGFX)
   // TODO We are missing a way to evaluate properly if we are syncing on display or not
   bool pop = (m_player->m_videoSyncMode != VideoSyncMode::VSM_NONE)
      && ((m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME) - 100) * m_player->m_playfieldWnd->GetRefreshRate() < 1000000);
      // && (abs(static_cast<float>(m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME)) - (1000000.f / m_player->m_playfieldWnd->GetRefreshRate())) < 100.f);
   if (pop)
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.5f, 0.2f, 1.f)); // Rendering at target framerate => green background
   #else
   if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && m_player->m_lastFrameSyncOnFPS)
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.75f, 1.f)); // Running at app regulated speed (not hardware)
   else if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && !m_player->m_lastFrameSyncOnVBlank)
      ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.75f, 0.f, 0.f, 1.f)); // Running slower than expected
   #endif
   ImGui::SetNextWindowBgAlpha(0.5f);
   ImGui::BeginChild("FPSText", ImVec2(0.f, 0.f), ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar);
   const double frameLength = m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_FRAME);
   ImGui::Text("Render: %5.1ffps %4.1fms (%4.1fms)\nLatency: %4.1fms (%4.1fms max)",
      1e6 / frameLength, 1e-3 * frameLength, 1e-3 * m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME),
      1e-3 * m_player->m_logicProfiler.GetSlidingInputLag(false), 1e-3 * m_player->m_logicProfiler.GetSlidingInputLag(true));
   ImGui::EndChild();
   #if defined(ENABLE_BGFX)
   if (pop)
      ImGui::PopStyleColor();
   #else
   if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && m_player->m_lastFrameSyncOnFPS)
      ImGui::PopStyleColor();
   else if (m_player->m_videoSyncMode == VideoSyncMode::VSM_FRAME_PACING && !m_player->m_lastFrameSyncOnVBlank)
      ImGui::PopStyleColor();
   #endif

   ImGui::End();

   // Display plots
   if (m_showPerf == PerfMode::PM_STATS)
   {
      constexpr ImGuiWindowFlags window_flags_plots = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
      ImGui::SetNextWindowSize(ImVec2(530, 500));
      if (m_player->m_vrDevice)
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.70f, io.DisplaySize.y * 0.35f), 0, ImVec2(1.f, 0.f));
      else
         ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 8.f * m_dpi, io.DisplaySize.y - 8.f * m_dpi), 0, ImVec2(1.f, 1.f));
      ImGui::Begin("Plots", nullptr, window_flags_plots);

      float t = static_cast<float>(m_player->m_time_sec);
      constexpr int rt_axis = ImPlotAxisFlags_NoTickLabels;

      ImPlot::GetStyle().Colors[ImPlotCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.5f);
      ImPlot::GetStyle().Colors[ImPlotCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.15f);
      ImPlot::GetStyle().Colors[ImPlotCol_LegendBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.15f);

      m_plotFPS.SetRolling(m_showRollingFPS);
      m_plotFPSSmoothed.SetRolling(m_showRollingFPS);
      m_plotFPSSmoothed.AddPoint(t, 1e-3f * (float)m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_FRAME)); // Frame time in ms
      m_plotFPS.AddPoint(t, m_plotFPS.GetLast().y * 0.95f + m_plotFPSSmoothed.GetLast().y * 0.05f);
      if (m_plotFPS.HasData() && m_plotFPSSmoothed.HasData() && ImPlot::BeginPlot("##FPS", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2000.f / min(m_player->GetTargetRefreshRate(), 200.f), ImGuiCond_Always); // range is twice the target frame rate
         if (m_plotFPS.m_rolling)
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_plotFPS.m_timeSpan, ImGuiCond_Always);
         else
            ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t) - m_plotFPS.m_timeSpan, static_cast<double>(t), ImGuiCond_Always);
         ImPlot::PlotLine("ms Frame", &m_plotFPSSmoothed.m_data[0].x, &m_plotFPSSmoothed.m_data[0].y, m_plotFPSSmoothed.m_data.size(), 0, m_plotFPSSmoothed.m_offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Frame", &m_plotFPS.m_data[0].x, &m_plotFPS.m_data[0].y, m_plotFPS.m_data.size(), 0, m_plotFPS.m_offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }

      m_plotPhysx.SetRolling(m_showRollingFPS);
      m_plotPhysxSmoothed.SetRolling(m_showRollingFPS);
      m_plotPhysxSmoothed.AddPoint(t, 1e-3f * (float)m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_PHYSICS)); // Script in ms
      m_plotPhysx.AddPoint(t, m_plotPhysx.GetLast().y * 0.95f + m_plotPhysxSmoothed.GetLast().y * 0.05f);
      if (m_plotPhysx.HasData() && m_plotPhysxSmoothed.HasData() && ImPlot::BeginPlot("##Physics", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2.f * max(1.f, m_plotPhysx.GetMovingMax()), ImGuiCond_Always);
         if (m_plotPhysx.m_rolling)
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_plotPhysx.m_timeSpan, ImGuiCond_Always);
         else
            ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t) - m_plotPhysx.m_timeSpan, static_cast<double>(t), ImGuiCond_Always);
         ImPlot::PlotLine("ms Physics", &m_plotPhysxSmoothed.m_data[0].x, &m_plotPhysxSmoothed.m_data[0].y, m_plotPhysxSmoothed.m_data.size(), 0, m_plotPhysxSmoothed.m_offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Physics", &m_plotPhysx.m_data[0].x, &m_plotPhysx.m_data[0].y, m_plotPhysx.m_data.size(), 0, m_plotPhysx.m_offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }

      m_plotScript.SetRolling(m_showRollingFPS);
      m_plotScriptSmoothed.SetRolling(m_showRollingFPS);
      m_plotScriptSmoothed.AddPoint(t, 1e-3f * (float)m_player->m_logicProfiler.GetPrev(FrameProfiler::PROFILE_SCRIPT)); // Physics in ms
      m_plotScript.AddPoint(t, m_plotScript.GetLast().y * 0.95f + m_plotScriptSmoothed.GetLast().y * 0.05f);
      if (m_plotScript.HasData() && m_plotScriptSmoothed.HasData() && ImPlot::BeginPlot("##Script", ImVec2(-1, 150), ImPlotFlags_None))
      {
         ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
         ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
         ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2.f * max(1.f, m_plotScript.GetMovingMax()), ImGuiCond_Always);
         if (m_plotScript.m_rolling)
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_plotScript.m_timeSpan, ImGuiCond_Always);
         else
            ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t) - m_plotScript.m_timeSpan, static_cast<double>(t), ImGuiCond_Always);
         ImPlot::PlotLine("ms Script", &m_plotScriptSmoothed.m_data[0].x, &m_plotScriptSmoothed.m_data[0].y, m_plotScriptSmoothed.m_data.size(), 0, m_plotScriptSmoothed.m_offset, 2 * sizeof(float));
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Smoothed ms Script", &m_plotScript.m_data[0].x, &m_plotScript.m_data[0].y, m_plotScript.m_data.size(), 0, m_plotScript.m_offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
         ImPlot::EndPlot();
      }
      ImGui::End();
   }
}
