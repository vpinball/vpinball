// license:GPLv3+

#include "core/stdafx.h"
#include "PlungerSensorSettingsPage.h"

#include "input/PlungerHandler.h"
#include "parts/pintable.h"
#include "ui/live/LiveUI.h"

#include "implot/implot.h"

namespace VPX::InGameUI
{

PlungerSensorSettingsPage::PlungerSensorSettingsPage(int sensorIndex)
   : InGameUIPage("Plunger Sensor Settings"s, ""s, SaveMode::Global)
   , m_sensorIndex(sensorIndex)
{
   m_positionPlot.m_rolling = true;
   m_positionPlot.m_timeSpan = 5.f;
   m_velocityPlot.m_rolling = true;
   m_velocityPlot.m_timeSpan = 5.f;
   m_positionRawPlot.m_rolling = true;
   m_positionRawPlot.m_timeSpan = 5.f;
   m_velocityRawPlot.m_rolling = true;
   m_velocityRawPlot.m_timeSpan = 5.f;
}

void PlungerSensorSettingsPage::BuildPage()
{
   if (m_removed)
      return;

   auto linearPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Plunger{}.Linear", m_sensorIndex));
   auto posFilterPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Plunger{}.PosFilter", m_sensorIndex));
   if (!linearPropId.has_value() || !posFilterPropId.has_value())
      return;

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Position sensor"s));
   m_sideAxisSection.AppendSection(this, GetSensor()->GetPositionSensor().get(), std::format("Plunger{}.Position", m_sensorIndex), 0x01, [this]() { RequestRebuild(); });
   AddItem(std::make_unique<InGameUIItem>(posFilterPropId.value(), [this]() { return GetSensor()->IsPositionFilterEnabled(); }, [this](bool v) { GetSensor()->EnablePositionFilter(v); }));
   AddItem(std::make_unique<InGameUIItem>(linearPropId.value(), [this]() { return GetSensor()->IsLinear(); }, [this](bool v) { GetSensor()->SetLinear(v); }));

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Velocity sensor"s));
   m_frontAxisSection.AppendSection(this, GetSensor()->GetVelocitySensor().get(), std::format("Plunger{}.Velocity", m_sensorIndex), 0x02, [this]() { RequestRebuild(); });

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Other settings"s));
   AddItem(std::make_unique<InGameUIItem>("Remove this sensor"s, "Delete this sensor from the plunger sensor list."s,
      [this]()
      {
         m_removed = true;
         m_player->m_pininput.m_plungerHandler->RemoveSensor(m_sensorIndex);
         m_player->m_liveUI->m_inGameUI.NavigateBack();
         RequestRebuild();
      }));
}

void PlungerSensorSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_player->m_pininput.AddAxisListener([this]() { AppendPlot(); });
   RequestRebuild();
}

void PlungerSensorSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   m_player->m_pininput.ClearAxisListeners();
}

void PlungerSensorSettingsPage::AppendPlot()
{
   const float t = static_cast<float>((double)msec() / 1000.);
   m_positionPlot.AddPoint(t, m_player->m_pininput.m_plungerHandler->GetPosition());
   m_velocityPlot.AddPoint(t, m_player->m_pininput.m_plungerHandler->GetVelocity());
   if (const auto& pos = GetSensor()->GetPositionSensor(); pos->IsMapped())
      m_positionRawPlot.AddPoint(t, pos->GetValue());
   if (const auto& vel = GetSensor()->GetVelocitySensor(); vel->IsMapped())
      m_velocityRawPlot.AddPoint(t, vel->GetValue());
}

void PlungerSensorSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);

   if (m_removed)
      return;

   const ImGuiStyle& style = ImGui::GetStyle();

   const ImVec2 winSize = ImVec2(GetWindowSize().x, 400.f);
   const float plotWidth = (winSize.x - style.WindowPadding.x * 2.f);
   const float plotHeight = (winSize.y - style.WindowPadding.y * 2.f - style.ItemSpacing.y) / 2.f;
   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowPos(ImVec2(GetWindowPos().x, GetWindowPos().y - winSize.y - style.ItemSpacing.y));
   ImGui::SetNextWindowBgAlpha(0.666f);
   ImGui::SetNextWindowSize(winSize);
   ImGui::Begin("PlungerSensorOverlay", nullptr, window_flags);
   ImPlot::PushStyleColor(ImPlotCol_LegendBg, ImVec4(0.11f, 0.11f, 0.14f, 0.03f));
   ImGui::PushFont(nullptr, style.FontSizeBase * 0.5f); // Smaller font to keep graphics readable

   AppendPlot();

   // Plot Position
   if (ImPlot::BeginPlot("##PlungerPos", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_positionPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      if (GetSensor()->GetPositionSensor()->IsMapped() && m_positionRawPlot.HasData())
      {
         ImPlot::PlotLine(std::format("Position Sensor - {}", GetSensor()->GetPositionSensor()->GetMappingLabel()).c_str(), &m_positionRawPlot.m_data[0].x, &m_positionRawPlot.m_data[0].y,
            m_positionRawPlot.m_data.size(), { ImPlotProp_Offset, m_positionRawPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      if (m_positionPlot.HasData())
      {
         ImPlot::PlotLine("Position", &m_positionPlot.m_data[0].x, &m_positionPlot.m_data[0].y, m_positionPlot.m_data.size(),
            { ImPlotProp_Offset, m_positionPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      ImPlot::EndPlot();
   }

   // Plot Velocity
   if (ImPlot::BeginPlot("##PlungerVel", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_velocityPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      if (GetSensor()->GetVelocitySensor()->IsMapped() && m_velocityRawPlot.HasData())
      {
         ImPlot::PlotLine(std::format("Velocity Sensor - {}", GetSensor()->GetVelocitySensor()->GetMappingLabel()).c_str(), &m_velocityRawPlot.m_data[0].x, &m_velocityRawPlot.m_data[0].y,
            m_velocityRawPlot.m_data.size(), { ImPlotProp_Offset, m_velocityRawPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      if (m_velocityPlot.HasData())
      {
         ImPlot::PlotLine("Velocity", &m_velocityPlot.m_data[0].x, &m_velocityPlot.m_data[0].y, m_velocityPlot.m_data.size(),
            { ImPlotProp_Offset, m_velocityPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      ImPlot::EndPlot();
   }

   ImGui::PopFont();
   ImPlot::PopStyleColor();
   ImGui::End();
}

}
