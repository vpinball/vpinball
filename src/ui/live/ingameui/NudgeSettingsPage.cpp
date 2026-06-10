// license:GPLv3+

#include "core/stdafx.h"
#include "NudgeSettingsPage.h"

#include "parts/pintable.h"
#include "physics/cabinet/GamepadNudge.h"
#include "physics/cabinet/NudgeHandler.h"
#include "implot/implot.h"
#include "ui/live/LiveUI.h"
#include "ui/live/ingameui/NudgeSensorSettingsPage.h"


namespace VPX::InGameUI
{

NudgeSettingsPage::NudgeSettingsPage()
   : InGameUIPage("Nudge Settings"s, ""s, SaveMode::Global)
{
   m_nudgeXPlot.m_rolling = true;
   m_nudgeXPlot.m_timeSpan = 5.f;
   m_nudgeYPlot.m_rolling = true;
   m_nudgeYPlot.m_timeSpan = 5.f;
   m_cabXPlot.m_rolling = true;
   m_cabXPlot.m_timeSpan = 5.f;
   m_cabYPlot.m_rolling = true;
   m_cabYPlot.m_timeSpan = 5.f;
}

void NudgeSettingsPage::BuildPage()
{
   InputManager& input = GetInput();

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Hardware sensor based nudge"s));

   const int nudgeSensorCount = m_player->m_pininput.m_nudgeHandler->GetSensorCount();
   for (int i = 0; i < nudgeSensorCount; i++)
   {
      const string url = std::format("settings/nudge_sensor_{:d}", i);
      m_player->m_liveUI->m_inGameUI.AddPage(url, [i]() { return std::make_unique<NudgeSensorSettingsPage>(i); });
      const auto& sensor = m_player->m_pininput.m_nudgeHandler->GetSensor(i);
      const string label = std::format("Sensor #{}: {}", i, sensor->GetDisplayName());
      AddItem(std::make_unique<InGameUIItem>(label, "", url));
   }
   AddItem(std::make_unique<InGameUIItem>("Add a new nudge sensor"s, "Add and setup a new nudge sensor."s,
      [this]()
      {
         std::unique_ptr<VPX::Physics::NudgeSensor> sensor = std::make_unique<VPX::Physics::GamepadNudge>(&m_player->m_pininput);
         m_player->m_pininput.m_nudgeHandler->AddSensor(sensor);
         const int i = m_player->m_pininput.m_nudgeHandler->GetSensorCount() - 1;
         const string url = std::format("settings/nudge_sensor_{:d}", i);
         m_player->m_liveUI->m_inGameUI.AddPage(url, [i]() { return std::make_unique<NudgeSensorSettingsPage>(i); });
         m_player->m_liveUI->m_inGameUI.Navigate(url);
         RequestRebuild();
      }));

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Keyboard nudge"s));

   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_KeyboardNudgeMode, //
      [this]() { return (int)m_player->m_pininput.m_nudgeHandler->GetKeyboardNudgeMode(); }, //
      [this](int, int v)
      {
         m_player->m_pininput.m_nudgeHandler->SetKeyboardNudgeMode((VPX::Physics::NudgeHandler::KeyboardNudgeMode)v);
         RequestRebuild();
      }));

   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_KeyboardNudgeStrength, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_pininput.m_nudgeHandler->GetKeyboardNudgeStrength(); }, //
      [this](float, float v) { m_player->m_pininput.m_nudgeHandler->SetKeyboardNudgeStrength(v); }));

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Visual nudge feedback"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_NudgeStrength, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_NudgeStrength(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_NudgeStrength(v, false);
         m_notificationId = m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000, m_notificationId);
      }));

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Emulated tilt plumb"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_SimulatedPlumb, //
      [this]() { return m_player->m_physics->m_plumbHandler.IsPlumbSimulated(); }, //
      [this](bool v)
      {
         m_player->m_physics->m_plumbHandler.EnablePlumbSimulation(v);
         RequestRebuild();
      }));

   if (m_player->m_physics->m_plumbHandler.IsPlumbSimulated())
   {
      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propPlayer_PlumbDamping, 100.f, "%4.1f %%"s, //
         [this]() { return m_player->m_physics->m_plumbHandler.GetPlumbDamping(); }, //
         [this](float, float v) { m_player->m_physics->m_plumbHandler.SetPlumbDamping(v); }));

      AddItem(std::make_unique<InGameUIItem>(
         Settings::m_propPlayer_PlumbThresholdAngle, 1.f, "%4.2f deg"s, //
         [this]() { return RADTOANG(m_player->m_physics->m_plumbHandler.GetPlumbTiltThreshold()); }, //
         [this](float, float v) { m_player->m_physics->m_plumbHandler.SetPlumbTiltThreshold(ANGTORAD(v)); }));
   }
}


void NudgeSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_player->m_pininput.AddAxisListener([this]() { AppendPlot(); });
   RequestRebuild();
}

void NudgeSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   m_player->m_pininput.ClearAxisListeners();
}

void NudgeSettingsPage::AppendPlot()
{
   const float t = static_cast<float>((double)msec() / 1000.);
   Vertex2D nudge = m_player->m_pininput.m_nudgeHandler->GetCabinetAcceleration();
   m_nudgeXPlot.AddPoint(t, nudge.x);
   m_nudgeYPlot.AddPoint(t, nudge.y);
   Vertex2D pos = m_player->m_pininput.m_nudgeHandler->GetCabinetOffset();
   m_cabXPlot.AddPoint(t, pos.x * 1000.f);
   m_cabYPlot.AddPoint(t, pos.y * 1000.f);
}

void NudgeSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);

   AppendPlot();

   const ImGuiStyle& style = ImGui::GetStyle();

   const ImVec2 winSize = ImVec2(GetWindowSize().x, 400.f);
   const float plotWidth = (winSize.x - style.WindowPadding.x * 2.f);
   const float plotHeight = (winSize.y - style.WindowPadding.y * 2.f - style.ItemSpacing.y) / 2.f;
   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowPos(ImVec2(GetWindowPos().x, GetWindowPos().y - winSize.y - style.ItemSpacing.y));
   ImGui::SetNextWindowBgAlpha(0.666f);
   ImGui::SetNextWindowSize(winSize);
   ImGui::Begin("NudgeOverlay", nullptr, window_flags);
   ImPlot::PushStyleColor(ImPlotCol_LegendBg, ImVec4(0.11f, 0.11f, 0.14f, 0.03f));
   ImGui::PushFont(nullptr, style.FontSizeBase * 0.5f); // Smaller font to keep graphics readable

   if (ImPlot::BeginPlot("##NudgeX", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_Opposite);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_nudgeXPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -5.f, 5.f, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y2, -5.f, 5.f, ImGuiCond_Always);
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
      ImPlot::PlotLine("X Position (mm)", &m_cabXPlot.m_data[0].x, &m_cabXPlot.m_data[0].y, m_cabXPlot.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0.25f, 0), ImPlotProp_Offset, m_cabXPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      ImPlot::PlotLine("X Acceleration (m/s^2)", &m_nudgeXPlot.m_data[0].x, &m_nudgeXPlot.m_data[0].y, m_nudgeXPlot.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_nudgeXPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::EndPlot();
   }

   if (ImPlot::BeginPlot("##NudgeY", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_Opposite);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_nudgeYPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -5.f, 5.f, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y2, -5.f, 5.f, ImGuiCond_Always);
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
      ImPlot::PlotLine("Y Position (mm)", &m_cabYPlot.m_data[0].x, &m_cabYPlot.m_data[0].y, m_cabYPlot.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0.25f, 0), ImPlotProp_Offset, m_cabYPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      ImPlot::PlotLine("Y Acceleration (m/s^2)", &m_nudgeYPlot.m_data[0].x, &m_nudgeYPlot.m_data[0].y, m_nudgeYPlot.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_nudgeYPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::EndPlot();
   }

   ImGui::PopFont();
   ImPlot::PopStyleColor();
   ImGui::End();
}

}
