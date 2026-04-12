// license:GPLv3+

#include "core/stdafx.h"

#include "PerfUI.h"
#include "utils/wintimer.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "implot/implot.h"
#include "parts/flipper.h"


PerfUI::PerfUI(Player *const player)
   : m_player(player)
{
   ImPlot::CreateContext();
   m_showPerf = (PerfMode)g_app->m_settings.GetPlayer_ShowFPS();
}

PerfUI::~PerfUI()
{
   g_app->m_settings.SetPlayer_ShowFPS(m_showPerf, false);
   g_app->m_settings.Save();
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
   
   ImGui::PushFont(nullptr, min(13.0f * m_uiScale, ImGui::GetIO().DisplaySize.x / 60.f));
   
   RenderFPS();

   if (m_showPerf == PerfMode::PM_STATS)
      RenderPlots();

   ImGui::PopFont();
}

void PerfUI::RenderFPS()
{
   const ImGuiIO &io = ImGui::GetIO();
   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   if (m_player->m_vrDevice == nullptr)
      ImGui::SetNextWindowPos(ImVec2(8.f * m_uiScale, io.DisplaySize.y - 8.f * m_uiScale), 0, ImVec2(0.f, 1.f));
   else if (m_showPerf == PerfMode::PM_STATS) // VR with stats
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.25f, io.DisplaySize.y * 0.35f), 0, ImVec2(0.f, 0.f));
   else // VR without stats
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.35f), 0, ImVec2(0.5f, 0.f));
   ImGui::SetNextWindowBgAlpha(m_player->m_renderer->m_vrApplyColorKey ? 1.f : 0.666f);
   ImGui::Begin("FPS", nullptr, window_flags);

   // Frame sequence graph & stats
   if (m_showPerf == PerfMode::PM_STATS)
      RenderStats();

   // Base FPS & latency feedback
   if (m_showPerf == PerfMode::PM_FPS)
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

   // Dot for sync feedback
   if (m_player->GetVideoSyncMode() != VideoSyncMode::VSM_NONE)
   {
      ImU32 color = IM_COL32(0, 255, 0, 128); // Default to green when at target refresh rate
      #ifndef ENABLE_BGFX
      if (m_player->GetVideoSyncMode() == VideoSyncMode::VSM_FRAME_PACING && m_player->m_lastFrameSyncOnFPS)
         color = IM_COL32(0, 0, 255, 128); // Blue dot when running at app regulated speed (not hardware)
      else if (m_player->GetVideoSyncMode() == VideoSyncMode::VSM_FRAME_PACING && !m_player->m_lastFrameSyncOnVBlank)
         color = IM_COL32(255, 0, 0, 128); // Red dot when running slower than expected
      else
      #endif
      if (m_player->m_renderProfiler->GetPrev(FrameProfiler::PROFILE_FRAME) > m_player->m_renderer->m_renderDevice->GetTargetFrameLength() + 500)
         color = IM_COL32(255, 0, 0, 128); // Red dot when missing target refresh rate
      ImGui::GetWindowDrawList()->AddCircleFilled(
         ImGui::GetCursorScreenPos() + ImVec2(ImGui::GetWindowWidth() - 5.f * m_uiScale - 2.f * ImGui::GetStyle().WindowPadding.x, ImGui::GetTextLineHeight() * 0.5f),
         5.f * m_uiScale, color);
   }

   ImGui::SetNextWindowBgAlpha(m_player->m_renderer->m_vrApplyColorKey ? 1.f : 0.666f);
   ImGui::BeginChild("FPSText", ImVec2(0.f, 0.f), ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground);

   {
      const double frameLength = m_player->m_renderProfiler->GetSlidingAvg(FrameProfiler::PROFILE_FRAME);
      const ImVec2 renderTextPos = ImGui::GetCursorScreenPos();
      #ifdef ENABLE_BGFX
         if (const float latency = 1000.f * m_player->m_renderer->m_renderDevice->GetVisualLatency(); latency > 0.f)
         {
            ImGui::Text("Render: %5.1ffps (Latency %4.1fms)", 1e6 / frameLength, latency);
            ImGui::SameLine();
            if (ImGui::IsMouseHoveringRect(renderTextPos, ImGui::GetCursorScreenPos() + ImVec2(0, ImGui::GetTextLineHeight())))
               ImGui::SetTooltip(
                  "Latency is an (imprecise) evaluation of the average finger to photon latency\nIt includes median input latency, rendering latency and estimated display latency");
            ImGui::NewLine();
         }
         else
            ImGui::Text("Render: %5.1ffps", 1e6 / frameLength);
      #else
         ImGui::Text("Render: %5.1ffps %4.1fms (%4.1fms)", 1e6 / frameLength, 1e-3 * frameLength, 1e-3 * m_player->m_renderProfiler->GetPrev(FrameProfiler::PROFILE_FRAME));
      #endif
   }

   {
      const ImVec2 inputTextPos = ImGui::GetCursorScreenPos();
      bool hasFlipperLatency = false;
      const uint64_t lastLeftFlipChange = m_player->m_pininput.GetInputActions()[m_player->m_pininput.GetLeftFlipperActionId()]->GetLastStateChange();
      const uint64_t lastRightFlipChange = m_player->m_pininput.GetInputActions()[m_player->m_pininput.GetRightFlipperActionId()]->GetLastStateChange();
      if (const uint64_t now = usec(); now < lastLeftFlipChange + 1000000ULL && now > lastRightFlipChange + 1000000ULL)
      {
         for (const auto part : m_player->m_ptable->GetParts())
         {
            if (part->GetItemType() == eItemFlipper)
            {
               if (auto flipper = (Flipper *)part; lastLeftFlipChange < flipper->GetLastRotateTime())
               {
                  const double prevAcqToAction = 1e-3 * (static_cast<double>(flipper->GetLastRotateTime() - lastLeftFlipChange) + m_player->m_pininput.m_leftFlipperLastChangePollDelay);
                  const double acqToAction = 1e-3 * static_cast<double>(flipper->GetLastRotateTime() - lastLeftFlipChange);
                  ImGui::Text("Flipper latency: %3.1f - %3.1fms", acqToAction, prevAcqToAction);
                  hasFlipperLatency = true;
                  break;
               }
            }
         }
      }
      if (!hasFlipperLatency)
         ImGui::Text("Input Latency: %4.1fms (%4.1fms max)", 1e-3 * m_player->m_logicProfiler.GetSlidingInputLag(false), 1e-3 * m_player->m_logicProfiler.GetSlidingInputLag(true));
      ImGui::SameLine();
      if (ImGui::IsMouseHoveringRect(inputTextPos, ImGui::GetCursorScreenPos() + ImVec2(0, ImGui::GetTextLineHeight())))
         ImGui::SetTooltip("'Input Latency' is an evaluation of the average finger to physics latency inside VPX\nIt does not include the polling & processing delay of the input hardware.");
      ImGui::NewLine();
   }

   ImGui::EndChild();
   if (m_showPerf == PerfMode::PM_FPS)
      ImGui::PopStyleVar();

   ImGui::End();
}

void PerfUI::RenderStats() const
{
   // Frame sequence graph
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
      "Wait for frame"s, // PROFILE_RENDER_WAIT
      "Wait for swapchain"s, // PROFILE_RENDER_WAIT_SC
      #ifdef ENABLE_BGFX
      "Submit BGFX"s, // PROFILE_RENDER_SUBMIT
      "Submit GPU"s, // PROFILE_RENDER_FLIP
      "Render Sleep"s, // PROFILE_RENDER_SLEEP
      #else
      "Render Submit"s,
      "Render Flip"s,
      "Render Sleep"s, 
      #endif
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
      "(Wait for a free swapchain slot)"s,
      #ifdef ENABLE_BGFX
      "(Submit frame from VPX to BGFX)"s,
      "(Submit frame from BGFX to GPU)"s,
      "(Sleep to pace frames on chosen FPS)"s,
      #else
      "(Submit to driver next frame)"s,
      "(Wait for GPU and display sync to finish current frame)"s,
      "(Sleep to synchronise on user selected FPS)"s,
      #endif
   };
   static constexpr FrameProfiler::ProfileSection sections[] = {
      FrameProfiler::PROFILE_RENDER_SLEEP,
      FrameProfiler::PROFILE_RENDER_WAIT,
      FrameProfiler::PROFILE_RENDER_SUBMIT,
      FrameProfiler::PROFILE_RENDER_FLIP,
      FrameProfiler::PROFILE_RENDER_WAIT_SC,
      FrameProfiler::PROFILE_PREPARE_FRAME,
   };
   static constexpr ImU32 cols[] = {
      IM_COL32(128, 128, 128, 255), // Sleep
      IM_COL32(192,   0,   0, 255), // Wait for Render Frame (red as it is not uspposed ot ever happen)
      IM_COL32(  0, 128, 255, 255), // Submit (VPX->BGFX)
      IM_COL32(  0, 192, 192, 255), // Submit (BGFX->Driver)
      IM_COL32(192, 192, 192, 255), // Wait for Swapchain slot
      IM_COL32(128, 255, 128, 255), // Prepare
   };
   const ImGuiContext &g = *GImGui;
   const ImGuiStyle &style = g.Style;
   ImVec2 graph_size;
   graph_size.x = ImGui::CalcItemWidth();
   const float blockHeight = ImGui::GetTextLineHeight() * 0.75f;
   const float lineheight = blockHeight;
   graph_size.y = (style.FramePadding.y * 1.f) + blockHeight * 1.f; // Logic Thread
   #ifdef ENABLE_BGFX
      graph_size.y += blockHeight * 2.f; // Additional Render Thread and GPU info
   #endif
   const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + graph_size);
   const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
   const ImRect total_bb(frame_bb.Min, frame_bb.Max);
   ImGui::ItemSize(total_bb, style.FramePadding.y);
   if (ImGui::ItemAdd(total_bb, 0, &frame_bb))
   {
      ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 0.5f), true, style.FrameRounding);
         
      uint64_t minTS = UINT64_MAX;
      uint64_t maxTS = 0;
      for (const auto section : sections)
      {
         const FrameProfiler *profiler = (section == FrameProfiler::PROFILE_PREPARE_FRAME) ? &m_player->m_logicProfiler : m_player->m_renderProfiler;
         minTS = (profiler->GetPrevStart(section) == 0) ? minTS : std::min(minTS, profiler->GetPrevStart(section));
         maxTS = (profiler->GetPrevStart(section) == 0) ? maxTS : std::max(maxTS, profiler->GetPrevEnd(section));
      }

      // Full scale is expected frame length + 1 display frame (to show when we miss)
      const float elapse = m_player->IsVR()
         ? 1.5f * static_cast<float>(m_player->m_renderProfiler->GetAvg(FrameProfiler::PROFILE_FRAME))
         : static_cast<float>(m_player->m_renderer->m_renderDevice->GetTargetFrameLength()) + static_cast<float>(1000000. / (double)m_player->m_playfieldWnd->GetRefreshRate());
      const float width = inner_bb.Max.x - inner_bb.Min.x;
      for (int i = 0; i < std::size(sections); i++)
      {
         const FrameProfiler *const profiler = (sections[i] == FrameProfiler::PROFILE_PREPARE_FRAME) ? &m_player->m_logicProfiler : m_player->m_renderProfiler;
         if (profiler->GetPrevStart(sections[i]) == 0)
            continue;
         const float start = static_cast<float>(profiler->GetPrevStart(sections[i]) - minTS) / elapse;
         const float end = static_cast<float>(profiler->GetPrevEnd(sections[i]) - minTS) / elapse;
         #ifdef ENABLE_BGFX
            const float height = ((sections[i] == FrameProfiler::PROFILE_PREPARE_FRAME) ? 0.0f : blockHeight) - style.FramePadding.y;
         #else
            const float height = - style.FramePadding.y;
         #endif
         const ImVec2 pos0 = inner_bb.Min + ImVec2(start * width, height);
         const ImVec2 pos1 = inner_bb.Min + ImVec2(end * width, height + lineheight);
         if (ImGui::IsMouseHoveringRect(pos0, pos1))
            ImGui::SetTooltip(
               "%s: %5.1fms\n%s", infoLabels[sections[i]].c_str(), static_cast<float>(profiler->GetPrev(sections[i])) * 1e-3f, infoLabels2[sections[i]].c_str());
         window->DrawList->AddRectFilled(pos0, pos1, cols[i]);
      }
      #ifdef ENABLE_BGFX
      {
         const bgfx::Stats *stats = bgfx::getStats();
         const float height = blockHeight * 2.f - style.FramePadding.y;
         const uint64_t gpuStart = m_player->m_renderProfiler->GetPrevStart(FrameProfiler::PROFILE_RENDER_FLIP);
         const uint64_t gpuEnd = gpuStart + (stats->gpuTimeEnd - stats->gpuTimeBegin) * 1000000ULL / stats->gpuTimerFreq;
         {
            const float start = static_cast<float>(gpuStart - minTS) / elapse;
            const float end = static_cast<float>(min(gpuEnd, maxTS) - minTS) / elapse;
            const ImVec2 pos0 = inner_bb.Min + ImVec2(start * width, height);
            const ImVec2 pos1 = inner_bb.Min + ImVec2(end * width, height + lineheight);
            window->DrawList->AddRectFilled(pos0, pos1, IM_COL32(255, 128, 128, 255));
            if (ImGui::IsMouseHoveringRect(pos0, pos1))
               ImGui::SetTooltip("GPU render: %5.1fms\n(Time spent by the GPU to process frame)", static_cast<float>(gpuEnd - gpuStart) * 1e-3f);
         }
         if (gpuEnd > maxTS)
         {
            const float end = static_cast<float>(gpuEnd - maxTS) / elapse;
            const ImVec2 pos0 = inner_bb.Min + ImVec2(0, height);
            const ImVec2 pos1 = inner_bb.Min + ImVec2(end * width, height + lineheight);
            window->DrawList->AddRectFilled(pos0, pos1, IM_COL32(255, 128, 128, 255));
            if (ImGui::IsMouseHoveringRect(pos0, pos1))
               ImGui::SetTooltip("GPU render: %5.1fms\n(Time spent by the GPU to process frame)", static_cast<float>(gpuEnd - gpuStart) * 1e-3f);
         }
      }
      #endif
   }

   // Timing table
   if (ImGui::BeginTable("Timings", 3, ImGuiTableFlags_Borders))
   {
      const uint32_t period = m_player->m_renderProfiler->GetPrev(FrameProfiler::PROFILE_FRAME);
      ImGui::TableSetupColumn("##Cat", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(m_showAvgFPS ? "Avg Time" : "Time", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn(m_showAvgFPS ? "Avg Ratio" : "Ratio", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableHeadersRow();

      #define PROF_ROW(name, section) \
      ImGui::TableNextRow(); ImGui::TableNextColumn(); ImGui::TextUnformatted(name); \
      if (m_showAvgFPS) { \
      ImGui::TableNextColumn(); ImGui::Text("%4.1fms", profiler->GetSlidingAvg(section) * 1e-3); \
      ImGui::TableNextColumn(); ImGui::Text("%4.1f%%", profiler->GetSlidingRatio(section) * 100.0); \
      } else { \
      ImGui::TableNextColumn(); ImGui::Text("%4.1fms", profiler->GetPrev(section) * 1e-3); \
      ImGui::TableNextColumn(); ImGui::Text("%4.1f%%", profiler->GetPrev(section) * 100.0 / period); \
      }

      const int hoveredRow = ImGui::TableGetHoveredRow();
      int renderRow = 1;
      int logicRow = 1;
      int rowOffset = 0;

      const FrameProfiler* profiler = g_pplayer->m_renderProfiler;
      #ifdef ENABLE_BGFX
         PROF_ROW("Render Thread", FrameProfiler::PROFILE_FRAME)
         PROF_ROW("> Wait Frame", FrameProfiler::PROFILE_RENDER_WAIT)
         PROF_ROW("> VPX -> BGFX", FrameProfiler::PROFILE_RENDER_SUBMIT)
         PROF_ROW("> BGFX -> GPU", FrameProfiler::PROFILE_RENDER_FLIP)
         PROF_ROW("> Wait Swapchain", FrameProfiler::PROFILE_RENDER_WAIT_SC)
         PROF_ROW("> Sleep", FrameProfiler::PROFILE_RENDER_SLEEP)
         if (hoveredRow == 2)
            ImGui::SetTooltip("Time spent waiting for:\n- the CPU to prepare a frame\n- a swapchain slot to be free\n- anticipating part of the sync sleep to reduce latency");
         else if (hoveredRow == 3)
            ImGui::SetTooltip("Time spent sleeping to match desired framerate");
         else if (hoveredRow == 4)
            ImGui::SetTooltip("Time spent submiting frame from VPX to BGFX");
         else if (hoveredRow == 5)
            ImGui::SetTooltip("Time spent submiting frame from BGFX to GPU");
         logicRow = 6;
         rowOffset = 2;

         profiler = &m_player->m_logicProfiler;
         PROF_ROW("Logic Thread", FrameProfiler::PROFILE_FRAME)
         PROF_ROW("> Prepare", FrameProfiler::PROFILE_PREPARE_FRAME)
         PROF_ROW("> Physics", FrameProfiler::PROFILE_PHYSICS)
         PROF_ROW("> Script", FrameProfiler::PROFILE_SCRIPT)
         PROF_ROW("> Sleep", FrameProfiler::PROFILE_SLEEP)
         PROF_ROW("> Misc", FrameProfiler::PROFILE_MISC)
      #else
         profiler = &m_player->m_logicProfiler;
         PROF_ROW("Frame", FrameProfiler::PROFILE_FRAME)
         PROF_ROW("> Prepare", FrameProfiler::PROFILE_PREPARE_FRAME)
         PROF_ROW("> Submit", FrameProfiler::PROFILE_RENDER_SUBMIT)
         PROF_ROW("> Flip", FrameProfiler::PROFILE_RENDER_FLIP)
         PROF_ROW("> Physics", FrameProfiler::PROFILE_PHYSICS)
         PROF_ROW("> Script", FrameProfiler::PROFILE_SCRIPT)
         PROF_ROW("> Sleep", FrameProfiler::PROFILE_SLEEP)
         PROF_ROW("> Misc", FrameProfiler::PROFILE_MISC)
      #endif

      #ifdef DEBUG
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM1) > 0) { PROF_ROW("> Debug #1", FrameProfiler::PROFILE_CUSTOM1); ImGui::TableNextRow(); }
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM2) > 0) { PROF_ROW("> Debug #2", FrameProfiler::PROFILE_CUSTOM2); ImGui::TableNextRow(); }
      if (m_player->m_logicProfiler.GetSlidingAvg(FrameProfiler::PROFILE_CUSTOM3) > 0) { PROF_ROW("> Debug #3", FrameProfiler::PROFILE_CUSTOM3); ImGui::TableNextRow(); }
      #endif

      if ((hoveredRow == renderRow) || (hoveredRow == logicRow))
         ImGui::SetTooltip("FPS: %4.1f (%4.1f average)", 1e6 / m_player->m_renderProfiler->GetPrev(FrameProfiler::PROFILE_FRAME), 1e6 / m_player->m_renderProfiler->GetSlidingAvg(FrameProfiler::PROFILE_FRAME));
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
   }
}

void PerfUI::RenderPlots()
{
   const ImGuiIO &io = ImGui::GetIO();
   ImGuiWindowFlags window_flags_plots
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   if (!m_player->m_renderer->m_vrApplyColorKey)
   {
      window_flags_plots |= ImGuiWindowFlags_NoBackground;
   }

   ImGui::SetNextWindowSize(ImVec2(5.f * ImGui::CalcTextSize("1234567890").x, 500.f));
   if (m_player->m_vrDevice)
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.80f, io.DisplaySize.y * 0.35f), 0, ImVec2(1.f, 0.f));
   else
      ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 8.f * m_uiScale, io.DisplaySize.y - 8.f * m_uiScale), 0, ImVec2(1.f, 1.f));
   ImGui::Begin("Plots", nullptr, window_flags_plots);

   const float t = static_cast<float>(m_player->m_time_sec);
   constexpr int rt_axis = ImPlotAxisFlags_NoTickLabels;

   ImPlot::GetStyle().Colors[ImPlotCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.5f);
   ImPlot::GetStyle().Colors[ImPlotCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.15f);
   ImPlot::GetStyle().Colors[ImPlotCol_LegendBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.15f);

   m_plotFPS.SetRolling(m_showRollingFPS);
   m_plotFPSSmoothed.SetRolling(m_showRollingFPS);
   m_plotFPSSmoothed.AddPoint(t, 1e-3f * static_cast<float>(m_player->m_renderProfiler->GetPrev(FrameProfiler::PROFILE_FRAME))); // Frame time in ms
   m_plotFPS.AddPoint(t, m_plotFPS.GetLast().y * 0.95f + m_plotFPSSmoothed.GetLast().y * 0.05f);
   if (m_plotFPS.HasData() && m_plotFPSSmoothed.HasData() && ImPlot::BeginPlot("##FPS", ImVec2(-1, 150), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, rt_axis);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_LockMin);
      const float targetRefreshRate = clamp(m_player->m_vrDevice ? 60.f : m_player->GetTargetRefreshRate(), 24.f, 200.f);
      ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 2000.f / targetRefreshRate, ImGuiCond_Always); // range is twice the target frame rate
      if (m_plotFPS.m_rolling)
         ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_plotFPS.m_timeSpan, ImGuiCond_Always);
      else
         ImPlot::SetupAxisLimits(ImAxis_X1, static_cast<double>(t) - m_plotFPS.m_timeSpan, static_cast<double>(t), ImGuiCond_Always);
      ImPlot::PlotLine("ms Frame", &m_plotFPSSmoothed.m_data[0].x, &m_plotFPSSmoothed.m_data[0].y, m_plotFPSSmoothed.m_data.size(),
         { ImPlotProp_Offset, m_plotFPSSmoothed.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::PlotLine("Smoothed ms Frame", &m_plotFPS.m_data[0].x, &m_plotFPS.m_data[0].y, m_plotFPS.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_plotFPS.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
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
      ImPlot::PlotLine("ms Physics", &m_plotPhysxSmoothed.m_data[0].x, &m_plotPhysxSmoothed.m_data[0].y, m_plotPhysxSmoothed.m_data.size(),
         { ImPlotProp_Offset, m_plotPhysxSmoothed.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::PlotLine("Smoothed ms Physics", &m_plotPhysx.m_data[0].x, &m_plotPhysx.m_data[0].y, m_plotPhysx.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_plotPhysx.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
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
      ImPlot::PlotLine("ms Script", &m_plotScriptSmoothed.m_data[0].x, &m_plotScriptSmoothed.m_data[0].y, m_plotScriptSmoothed.m_data.size(),
         { ImPlotProp_Offset, m_plotScriptSmoothed.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::PlotLine("Smoothed ms Script", &m_plotScript.m_data[0].x, &m_plotScript.m_data[0].y, m_plotScript.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_plotScript.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::EndPlot();
   }

   ImGui::End();
}
