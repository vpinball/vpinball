// license:GPLv3+

#include "core/stdafx.h"
#include "implot/implot.h"

#include "PlungerSettingsPage.h"

namespace VPX::InGameUI
{

PlungerSettingsPage::PlungerSettingsPage()
   : InGameUIPage("Plunger Settings"s, ""s, SaveMode::Global)
{
   const InputManager& input = GetInput();

   auto keyboardPlunger = std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Keyboard emulated plunger"s);
   AddItem(keyboardPlunger);

   auto oneSecRetract = std::make_unique<InGameUIItem>(
      "One Second Retract"s, "Enable retracting the plunger after a 1 second press when using the digital plunger emulation through keyboard or joystick button"s, false,
      [this]() { return GetInput().m_plunger_retract; }, [this](bool v) { GetInput().m_plunger_retract = v; }, InGameUIItem::ResetSetting(Settings::Player, "PlungerRetract"s),
      InGameUIItem::SaveSettingBool(Settings::Player, "PlungerRetract"s));
   AddItem(oneSecRetract);

   auto hardwarePlunger = std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Hardware sensor plunger"s);
   AddItem(hardwarePlunger);

   const auto& plungerPosSensor = input.GetPlungerPositionSensor();
   auto plungerPosItem = std::make_unique<InGameUIItem>(plungerPosSensor->GetLabel(), "Select to define which analog input to use for plunger position."s, plungerPosSensor.get(), 0x1);
   AddItem(plungerPosItem);

   const auto& plungerVelSensor = input.GetPlungerVelocitySensor();
   auto plungerVelItem = std::make_unique<InGameUIItem>(plungerVelSensor->GetLabel(), "Select to define which analog input to use for plunger velocity."s, plungerVelSensor.get(), 0x2);
   AddItem(plungerVelItem);

   auto linearAcquisition = std::make_unique<InGameUIItem>(
      "Linear Sensor"s, "Select between symmetric (linear) and assymetric sensor"s, false, [this]() { return GetInput().m_linearPlunger; }, [this](bool v)
      { GetInput().m_linearPlunger = v; }, InGameUIItem::ResetSetting(Settings::Player, "PlungerLinearSensor"s), InGameUIItem::SaveSettingBool(Settings::Player, "PlungerLinearSensor"s));
   AddItem(linearAcquisition);

   m_positionPlot.m_rolling = true;
   m_positionPlot.m_timeSpan = 5.f;
   m_velocityPlot.m_rolling = true;
   m_velocityPlot.m_timeSpan = 5.f;
}

void PlungerSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_player->m_pininput.AddAxisListener([this]() { AppendPlot(); });
}

void PlungerSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   m_player->m_pininput.ClearAxisListeners();
}

void PlungerSettingsPage::AppendPlot()
{
   const float t = static_cast<float>(msec() / 1000.);
   m_positionPlot.AddPoint(t, m_player->m_pininput.GetPlungerPos());
   m_velocityPlot.AddPoint(t, m_player->m_pininput.GetPlungerSpeed());
}

void PlungerSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);

   const ImGuiStyle& style = ImGui::GetStyle();

   const ImVec2 winSize = ImVec2(GetWindowSize().x, 200.f);
   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowPos(ImVec2(GetWindowPos().x, GetWindowPos().y - winSize.y - style.ItemSpacing.y));
   ImGui::SetNextWindowBgAlpha(0.5f);
   ImGui::SetNextWindowSize(winSize);
   ImGui::Begin("PlungerOverlay", nullptr, window_flags);
   ImPlot::PushStyleColor(ImPlotCol_LegendBg, ImVec4(0.11f, 0.11f, 0.14f, 0.03f));
   ImGui::PushFont(nullptr, style.FontSizeBase * 0.5f); // Smaller font to keep graphics readable

   AppendPlot();

   if (ImPlot::BeginPlot("##Script", ImVec2(-1, -1), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_positionPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y2, -0.3f, 0.3f, ImGuiCond_Always);
      if (m_player->m_pininput.HasMechPlungerSpeed())
      {
         ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
         ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
         ImPlot::PlotLine("Speed", &m_velocityPlot.m_data[0].x, &m_velocityPlot.m_data[0].y, m_velocityPlot.m_data.size(), ImPlotLineFlags_None, m_velocityPlot.m_offset, 2 * sizeof(float));
         ImPlot::PopStyleColor();
      }
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      ImPlot::PlotLine("Position", &m_positionPlot.m_data[0].x, &m_positionPlot.m_data[0].y, m_positionPlot.m_data.size(), ImPlotLineFlags_None, m_positionPlot.m_offset, 2 * sizeof(float));
      ImPlot::EndPlot();
   }

   ImGui::PopFont();
   ImPlot::PopStyleColor();
   ImGui::End();
}

}
