// license:GPLv3+

#include "core/stdafx.h"
#include "PlungerSettingsPage.h"

#include "implot/implot.h"
#include "input/PlungerHandler.h"
#include "ui/live/LiveUI.h"
#include "ui/live/ingameui/PlungerSensorSettingsPage.h"


namespace VPX::InGameUI
{

PlungerSettingsPage::PlungerSettingsPage()
   : InGameUIPage("Plunger Settings"s, ""s, SaveMode::Global)
{
   m_positionPlot.m_rolling = true;
   m_positionPlot.m_timeSpan = 5.f;
   m_velocityPlot.m_rolling = true;
   m_velocityPlot.m_timeSpan = 5.f;
}

void PlungerSettingsPage::BuildPage()
{
   const InputManager& input = GetInput();

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Hardware sensor plunger"s));

   const int sensorCount = m_player->m_pininput.m_plungerHandler->GetSensorCount();
   for (int i = 0; i < sensorCount; i++)
   {
      const string url = std::format("settings/plunger_sensor_{:d}", i);
      m_player->m_liveUI->m_inGameUI.AddPage(url, [i]() { return std::make_unique<PlungerSensorSettingsPage>(i); });
      const auto& sensor = m_player->m_pininput.m_plungerHandler->GetSensor(i);
      string label = std::format("Sensor #{}", i);
      if (sensor->GetPositionSensor()->IsMapped())
         label = std::format("{}:  {}", label, m_player->m_pininput.GetDeviceName(sensor->GetPositionSensor()->GetMapping().GetDeviceId()));
      else
         label = std::format("{}:  Unmapped", label);
      AddItem(std::make_unique<InGameUIItem>(label, "", url));
   }
   AddItem(std::make_unique<InGameUIItem>("Add a new plunger sensor"s, "Add and setup a new plunger sensor."s,
      [this]()
      {
         std::unique_ptr<PlungerSensor> sensor = std::make_unique<PlungerSensor>(&m_player->m_pininput);
         m_player->m_pininput.m_plungerHandler->AddSensor(sensor);
         const int i = m_player->m_pininput.m_plungerHandler->GetSensorCount() - 1;
         const string url = std::format("settings/plunger_sensor_{:d}", i);
         m_player->m_liveUI->m_inGameUI.AddPage(url, [i]() { return std::make_unique<PlungerSensorSettingsPage>(i); });
         m_player->m_liveUI->m_inGameUI.Navigate(url);
         RequestRebuild();
      }));

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Keyboard emulated plunger"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_PlungerRetract, //
      [this]() { return m_player->m_pininput.m_plungerHandler->IsPullBackandRetract(); }, //
      [this](bool v) { m_player->m_pininput.m_plungerHandler->SetPullBackandRetract(v); }));
}

void PlungerSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_player->m_pininput.AddAxisListener([this]() { AppendPlot(); });
   RequestRebuild();
}

void PlungerSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   m_player->m_pininput.ClearAxisListeners();
}

void PlungerSettingsPage::AppendPlot()
{
   const float t = static_cast<float>((double)msec() / 1000.);
   m_positionPlot.AddPoint(t, m_player->m_pininput.m_plungerHandler->GetPosition());
   m_velocityPlot.AddPoint(t, m_player->m_pininput.m_plungerHandler->GetVelocity());
}

void PlungerSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);

   const ImGuiStyle& style = ImGui::GetStyle();

   const ImVec2 winSize = ImVec2(GetWindowSize().x, 200.f);
   const float plotWidth = (winSize.x - style.WindowPadding.x * 2.f);
   const float plotHeight = (winSize.y - style.WindowPadding.y * 2.f);
   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowPos(ImVec2(GetWindowPos().x, GetWindowPos().y - winSize.y - style.ItemSpacing.y));
   ImGui::SetNextWindowBgAlpha(0.666f);
   ImGui::SetNextWindowSize(winSize);
   ImGui::Begin("PlungerOverlay", nullptr, window_flags);
   ImPlot::PushStyleColor(ImPlotCol_LegendBg, ImVec4(0.11f, 0.11f, 0.14f, 0.03f));
   ImGui::PushFont(nullptr, style.FontSizeBase * 0.5f); // Smaller font to keep graphics readable

   AppendPlot();

   if (ImPlot::BeginPlot("##Plunger", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_positionPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      if (m_velocityPlot.HasData() && m_player->m_pininput.m_plungerHandler->HasVelocity())
      {
         ImPlot::PlotLine("Speed", &m_velocityPlot.m_data[0].x, &m_velocityPlot.m_data[0].y, m_velocityPlot.m_data.size(),
            { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_velocityPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      if (m_positionPlot.HasData())
      {
         ImPlot::PlotLine("Position", &m_positionPlot.m_data[0].x, &m_positionPlot.m_data[0].y, m_positionPlot.m_data.size(),
            { ImPlotProp_Offset, m_positionPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      ImPlot::EndPlot();
   }

   ImGui::PopFont();
   ImPlot::PopStyleColor();
   ImGui::End();
}

}
