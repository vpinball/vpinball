// license:GPLv3+

#include "core/stdafx.h"
#include "NudgeSensorSettingsPage.h"

#include "parts/pintable.h"
#include "physics/cabinet/CabinetNudgeSensor.h"
#include "physics/cabinet/GamepadNudge.h"
#include "physics/cabinet/NudgeHandler.h"
#include "ui/live/LiveUI.h"

#include "implot/implot.h"

namespace VPX::InGameUI
{

NudgeSensorSettingsPage::NudgeSensorSettingsPage(int sensorIndex)
   : InGameUIPage("Nudge Sensor Settings"s, ""s, SaveMode::Global)
   , m_sensorIndex(sensorIndex)
{
   m_nudgeXPlot.m_rolling = true;
   m_nudgeXPlot.m_timeSpan = 5.f;
   m_nudgeYPlot.m_rolling = true;
   m_nudgeYPlot.m_timeSpan = 5.f;
   m_nudgeXRawPlot1.m_rolling = true;
   m_nudgeXRawPlot1.m_timeSpan = 5.f;
   m_nudgeYRawPlot1.m_rolling = true;
   m_nudgeYRawPlot1.m_timeSpan = 5.f;
   m_nudgeXRawPlot2.m_rolling = true;
   m_nudgeXRawPlot2.m_timeSpan = 5.f;
   m_nudgeYRawPlot2.m_rolling = true;
   m_nudgeYRawPlot2.m_timeSpan = 5.f;
}

void NudgeSensorSettingsPage::BuildPage()
{
   if (m_removed)
      return;

   auto typePropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Type", m_sensorIndex));
   if (!typePropId.has_value())
      return;
   AddItem(std::make_unique<InGameUIItem>(
      typePropId.value(),
      [this]()
      {
         if (dynamic_cast<VPX::Physics::GamepadNudge*>(GetSensor().get()))
            return 0;
         if (const auto cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
            return cabSensor->IsIntentSensor() ? 1 : 2;
         assert(false); // Invalid
         return 0;
      },
      [this](int, int v)
      {
         switch (v)
         {
         case 0:
         {
            std::unique_ptr<VPX::Physics::GamepadNudge> sensor = std::make_unique<VPX::Physics::GamepadNudge>(&m_player->m_pininput);
            sensor->SetStrengthScale(GetSensor()->GetStrengthScale());
            m_player->m_pininput.m_nudgeHandler->ReplaceSensor(m_sensorIndex, std::move(sensor));
            break;
         }
         case 1:
         {
            std::unique_ptr<VPX::Physics::CabinetNudgeSensor> sensor = std::make_unique<VPX::Physics::CabinetNudgeSensor>(&m_player->m_pininput);
            sensor->SetIntentSensor(true);
            if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
            {
               if (cabSensor->GetXVelSensor().IsMapped())
                  sensor->GetXVelSensor().SetMapping(cabSensor->GetXVelSensor().GetMapping());
               if (cabSensor->GetYVelSensor().IsMapped())
                  sensor->GetYVelSensor().SetMapping(cabSensor->GetYVelSensor().GetMapping());
               if (cabSensor->GetXAccSensor().IsMapped())
                  sensor->GetXAccSensor().SetMapping(cabSensor->GetXAccSensor().GetMapping());
               if (cabSensor->GetYAccSensor().IsMapped())
                  sensor->GetYAccSensor().SetMapping(cabSensor->GetYAccSensor().GetMapping());
            }
            m_player->m_pininput.m_nudgeHandler->ReplaceSensor(m_sensorIndex, std::move(sensor));
            break;
         }
         case 2:
         {
            std::unique_ptr<VPX::Physics::CabinetNudgeSensor> sensor = std::make_unique<VPX::Physics::CabinetNudgeSensor>(&m_player->m_pininput);
            sensor->SetIntentSensor(false);
            if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
            {
               if (cabSensor->GetXVelSensor().IsMapped())
                  sensor->GetXVelSensor().SetMapping(cabSensor->GetXVelSensor().GetMapping());
               if (cabSensor->GetYVelSensor().IsMapped())
                  sensor->GetYVelSensor().SetMapping(cabSensor->GetYVelSensor().GetMapping());
               if (cabSensor->GetXAccSensor().IsMapped())
                  sensor->GetXAccSensor().SetMapping(cabSensor->GetXAccSensor().GetMapping());
               if (cabSensor->GetYAccSensor().IsMapped())
                  sensor->GetYAccSensor().SetMapping(cabSensor->GetYAccSensor().GetMapping());
            }
            m_player->m_pininput.m_nudgeHandler->ReplaceSensor(m_sensorIndex, std::move(sensor));
            break;
         }
         default: assert(false); break;
         }
         RequestRebuild();
      }));

   auto strengthPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.Strength", m_sensorIndex));
   if (!strengthPropId.has_value())
      return;
   AddItem(
      std::make_unique<InGameUIItem>(strengthPropId.value(), 100.f, "%4.1f %%", [this]() { return GetSensor()->GetStrengthScale(); }, [this](float, float v) { GetSensor()->SetStrengthScale(v); }));

   if (VPX::Physics::GamepadNudge* gamepadSensor = dynamic_cast<VPX::Physics::GamepadNudge*>(GetSensor().get()); gamepadSensor)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Side nudge sensor"s));
      m_sideAxisSection.AppendSection(this, &gamepadSensor->GetXSensor(), std::format("Nudge{}.X", m_sensorIndex), 1, [this]() { RequestRebuild(); });

      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Front nudge sensor"s));
      m_frontAxisSection.AppendSection(this, &gamepadSensor->GetYSensor(), std::format("Nudge{}.Y", m_sensorIndex), 1, [this]() { RequestRebuild(); });
   }

   if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
   {
      if (!cabSensor->IsIntentSensor())
      {
         auto cabWeightPropId = Settings::GetRegistry().GetPropertyId("Input"s, std::format("Mapping.Nudge{}.CabWeight", m_sensorIndex));
         if (!cabWeightPropId.has_value())
            return;
         AddItem(std::make_unique<InGameUIItem>(
            cabWeightPropId.value(), 1.f, "%4.1f kg", [cabSensor]() { return cabSensor->GetCabinetMass(); }, [cabSensor](float, float v) { cabSensor->SetCabinetMass(v); }));
      }
      
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Side acceleration sensor"s));
      m_sideAxisSection.AppendSection(this, &cabSensor->GetXAccSensor(), std::format("Nudge{}.AccX", m_sensorIndex), 4, [this]() { RequestRebuild(); });

      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Front acceleration sensor"s));
      m_frontAxisSection.AppendSection(this, &cabSensor->GetYAccSensor(), std::format("Nudge{}.AccY", m_sensorIndex), 4, [this]() { RequestRebuild(); });

      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Side velocity sensor"s));
      m_sideAxisSection2.AppendSection(this, &cabSensor->GetXVelSensor(), std::format("Nudge{}.VelX", m_sensorIndex), 2, [this]() { RequestRebuild(); });

      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Front velocity sensor"s));
      m_frontAxisSection2.AppendSection(this, &cabSensor->GetYVelSensor(), std::format("Nudge{}.VelY", m_sensorIndex), 2, [this]() { RequestRebuild(); });
   }

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Other settings"s));
   AddItem(std::make_unique<InGameUIItem>("Remove this sensor"s, "Delete this sensor from the nudge sensor list."s,
      [this]()
      {
         m_removed = true;
         m_player->m_pininput.m_nudgeHandler->RemoveSensor(m_sensorIndex);
         m_player->m_liveUI->m_inGameUI.NavigateBack();
         RequestRebuild();
      }));
}

void NudgeSensorSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_player->m_pininput.AddAxisListener([this]() { AppendPlot(); });
   RequestRebuild();
}

void NudgeSensorSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   m_player->m_pininput.ClearAxisListeners();
}

void NudgeSensorSettingsPage::AppendPlot()
{
   const float t = static_cast<float>((double)msec() / 1000.);

   Vertex2D nudge = m_player->m_pininput.m_nudgeHandler->GetCabinetAcceleration();
   m_nudgeXPlot.AddPoint(t, nudge.x);
   m_nudgeYPlot.AddPoint(t, nudge.y);

   if (VPX::Physics::GamepadNudge* gamepadSensor = dynamic_cast<VPX::Physics::GamepadNudge*>(GetSensor().get()); gamepadSensor)
   {
      m_nudgeXRawPlot1.AddPoint(t, gamepadSensor->GetXSensor().GetValue());
      m_nudgeYRawPlot1.AddPoint(t, gamepadSensor->GetYSensor().GetValue());
   }
   else if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
   {
      m_nudgeXRawPlot1.AddPoint(t, cabSensor->GetXAccSensor().GetValue());
      m_nudgeYRawPlot1.AddPoint(t, cabSensor->GetYAccSensor().GetValue());
      m_nudgeXRawPlot2.AddPoint(t, cabSensor->GetXVelSensor().GetValue());
      m_nudgeYRawPlot2.AddPoint(t, cabSensor->GetYVelSensor().GetValue());
   }
}

void NudgeSensorSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);

   if (m_removed)
      return;

   const ImGuiStyle& style = ImGui::GetStyle();
   const ImVec2 winSize = ImVec2(GetWindowSize().x, 400.f);
   const float plotHeight = (winSize.y - style.WindowPadding.y * 2.f - style.ItemSpacing.y) / 2.f;
   const float plumbSize = plotHeight; //m_player->m_physics->IsPlumbSimulated() ? plotHeight : 0.f;
   const float plotWidth = (winSize.x - style.WindowPadding.x * 2.f) - style.ItemSpacing.x - plumbSize;

   if (const uint32_t ms = msec(); ms - m_resetTimestampMs > 2000)
   {
      m_resetTimestampMs = ms;
      m_sensorAcqPeriod.x = 0;
      m_sensorAcqPeriod.y = 0;
      if (VPX::Physics::GamepadNudge* gamepadSensor = dynamic_cast<VPX::Physics::GamepadNudge*>(GetSensor().get()); gamepadSensor)
      {
         if (gamepadSensor->GetXSensor().IsMapped())
         {
            m_sensorAcqPeriod.x = gamepadSensor->GetXSensor().GetMapping().GetShortestUpdateMs();
            gamepadSensor->GetXSensor().GetMapping().ResetShortestUpdateMs();
         }
         if (gamepadSensor->GetYSensor().IsMapped())
         {
            m_sensorAcqPeriod.y = gamepadSensor->GetYSensor().GetMapping().GetShortestUpdateMs();
            gamepadSensor->GetYSensor().GetMapping().ResetShortestUpdateMs();
         }
      }
      else if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
      {
         if (cabSensor->GetXAccSensor().IsMapped())
         {
            m_sensorAcqPeriod.x = cabSensor->GetXAccSensor().GetMapping().GetShortestUpdateMs();
            cabSensor->GetXAccSensor().GetMapping().ResetShortestUpdateMs();
         }
         else if (cabSensor->GetXVelSensor().IsMapped())
         {
            m_sensorAcqPeriod.x = cabSensor->GetXVelSensor().GetMapping().GetShortestUpdateMs();
            cabSensor->GetXVelSensor().GetMapping().ResetShortestUpdateMs();
         }
         if (cabSensor->GetYAccSensor().IsMapped())
         {
            m_sensorAcqPeriod.y = cabSensor->GetYAccSensor().GetMapping().GetShortestUpdateMs();
            cabSensor->GetYAccSensor().GetMapping().ResetShortestUpdateMs();
         }
         else if (cabSensor->GetYVelSensor().IsMapped())
         {
            m_sensorAcqPeriod.y = cabSensor->GetYVelSensor().GetMapping().GetShortestUpdateMs();
            cabSensor->GetYVelSensor().GetMapping().ResetShortestUpdateMs();
         }
      }
   }

   AppendPlot();

   constexpr ImGuiWindowFlags window_flags
      = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
   ImGui::SetNextWindowPos(ImVec2(GetWindowPos().x, GetWindowPos().y - winSize.y - style.ItemSpacing.y));
   ImGui::SetNextWindowBgAlpha(0.666f);
   ImGui::SetNextWindowSize(winSize);
   ImGui::Begin("NudgeOverlay", nullptr, window_flags);
   ImPlot::PushStyleColor(ImPlotCol_LegendBg, ImVec4(0.11f, 0.11f, 0.14f, 0.03f));

   const auto& sensor = GetSensor();

   ImGui::PushFont(nullptr, style.FontSizeBase * 0.5f); // Smaller font to keep graphics readable
   if (ImPlot::BeginPlot("##NudgeX", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_Opposite);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_nudgeXPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y2, -5.0f, 5.0f, ImGuiCond_Always);

      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      if (VPX::Physics::GamepadNudge* gamepadSensor = dynamic_cast<VPX::Physics::GamepadNudge*>(GetSensor().get()); gamepadSensor)
      {
         if (gamepadSensor->GetXSensor().IsMapped() && m_nudgeXRawPlot1.HasData())
            ImPlot::PlotLine(std::format("Sensor X - {}", gamepadSensor->GetXSensor().GetMappingLabel()).c_str(),
            &m_nudgeXRawPlot1.m_data[0].x, //
            &m_nudgeXRawPlot1.m_data[0].y, //
            m_nudgeXRawPlot1.m_data.size(), { ImPlotProp_Offset, m_nudgeXRawPlot1.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      else if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
      {
         if (cabSensor->GetXAccSensor().IsMapped() && m_nudgeXRawPlot1.HasData())
            ImPlot::PlotLine(std::format("Acc. Sensor X - {} (m/s^2)", cabSensor->GetXAccSensor().GetMappingLabel()).c_str(),
               &m_nudgeXRawPlot1.m_data[0].x, //
               &m_nudgeXRawPlot1.m_data[0].y, //
               m_nudgeXRawPlot1.m_data.size(), { ImPlotProp_Offset, m_nudgeXRawPlot1.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
         if (cabSensor->GetXVelSensor().IsMapped() && m_nudgeXRawPlot2.HasData())
            ImPlot::PlotLine(std::format("Vel. Sensor X - {} (m/s)", cabSensor->GetXVelSensor().GetMappingLabel()).c_str(),
               &m_nudgeXRawPlot2.m_data[0].x, //
               &m_nudgeXRawPlot2.m_data[0].y, //
               m_nudgeXRawPlot2.m_data.size(), { ImPlotProp_Offset, m_nudgeXRawPlot2.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }

      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
      ImPlot::PlotLine("Cabinet Acceleration X (m/s^2)", &m_nudgeXPlot.m_data[0].x, &m_nudgeXPlot.m_data[0].y, m_nudgeXPlot.m_data.size(),
            { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_nudgeXPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::EndPlot();
   }
   ImGui::PopFont();

   ImGui::SameLine();

   {
      ImGui::PushStyleVarY(ImGuiStyleVar_ItemSpacing, 1.f);
      const float downscale = min(1.f, 0.5f * plumbSize / ImGui::CalcTextSize("Refresh Rate").x);
      ImGui::PushFont(nullptr, ImGui::GetStyle().FontSizeBase * downscale); // Eventually smaller font to fit (otherwise aligned to plot font size)
      ImGui::BeginChild(std::format("SensorInfo").c_str(), ImVec2(plumbSize, plumbSize), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
      ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(2.f, 2.f));
      ImGui::SameLine(0.5f * (plumbSize - ImGui::CalcTextSize("Sensor Info").x));
      ImGui::Text("Sensor Info");
      ImVec2 min = ImGui::GetItemRectMin();
      ImVec2 max = ImGui::GetItemRectMax();
      min.y = max.y;
      ImGui::GetWindowDrawList()->AddLine(min, max, IM_COL32_WHITE, 1.0f);
      ImGui::Separator();
      ImGui::Text("Refresh Rate");
      if (m_sensorAcqPeriod.x != 0)
         ImGui::Text("  X: %4dms", m_sensorAcqPeriod.x);
      else
         ImGui::Text("  X:  - ms");
      if (m_sensorAcqPeriod.y != 0)
         ImGui::Text("  Y: %4dms", m_sensorAcqPeriod.y);
      else
         ImGui::Text("  Y:  - ms");
      if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
      {
         float freq = 0.f;
         if (cabSensor->GetYAccSensor().IsMapped())
            freq = cabSensor->GetYAccSensor().GetMapping().GetMainFrequency();
         else if (cabSensor->GetYVelSensor().IsMapped())
            freq = cabSensor->GetYVelSensor().GetMapping().GetMainFrequency();
         if (freq != 0)
         {
            ImGui::Separator();
            ImGui::Text("Front Freq: %4.2fHz", freq);
         }
         freq = 0.f;
         if (cabSensor->GetXAccSensor().IsMapped())
            freq = cabSensor->GetXAccSensor().GetMapping().GetMainFrequency();
         else if (cabSensor->GetXVelSensor().IsMapped())
            freq = cabSensor->GetXVelSensor().GetMapping().GetMainFrequency();
         if (freq != 0)
         {
            ImGui::Separator();
            ImGui::Text("Side Freq: %4.2fHz", freq);
         }
      }
      ImGui::EndChild();
      ImGui::PopFont();
      ImGui::PopStyleVar();
   }

   ImGui::PushFont(nullptr, style.FontSizeBase * 0.5f); // Smaller font to keep graphics readable
   if (ImPlot::BeginPlot("##NudgeY", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_Opposite);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_nudgeYPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y2, -5.0f, 5.0f, ImGuiCond_Always);

      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      if (VPX::Physics::GamepadNudge* gamepadSensor = dynamic_cast<VPX::Physics::GamepadNudge*>(GetSensor().get()); gamepadSensor)
      {
         if (gamepadSensor->GetYSensor().IsMapped() && m_nudgeYRawPlot1.HasData())
            ImPlot::PlotLine(std::format("Sensor Y - {}", gamepadSensor->GetYSensor().GetMappingLabel()).c_str(),
               &m_nudgeYRawPlot1.m_data[0].x, //
               &m_nudgeYRawPlot1.m_data[0].y, //
               m_nudgeYRawPlot1.m_data.size(), { ImPlotProp_Offset, m_nudgeYRawPlot1.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }
      else if (VPX::Physics::CabinetNudgeSensor* cabSensor = dynamic_cast<VPX::Physics::CabinetNudgeSensor*>(GetSensor().get()); cabSensor)
      {
         if (cabSensor->GetYAccSensor().IsMapped() && m_nudgeYRawPlot1.HasData())
            ImPlot::PlotLine(std::format("Acc. Sensor Y - {} (m/s^2)", cabSensor->GetYAccSensor().GetMappingLabel()).c_str(),
               &m_nudgeYRawPlot1.m_data[0].x, //
               &m_nudgeYRawPlot1.m_data[0].y, //
               m_nudgeYRawPlot1.m_data.size(), { ImPlotProp_Offset, m_nudgeYRawPlot1.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
         if (cabSensor->GetYVelSensor().IsMapped() && m_nudgeYRawPlot2.HasData())
            ImPlot::PlotLine(std::format("Vel. Sensor Y - {} (m/s)", cabSensor->GetYVelSensor().GetMappingLabel()).c_str(),
               &m_nudgeYRawPlot2.m_data[0].x, //
               &m_nudgeYRawPlot2.m_data[0].y, //
               m_nudgeYRawPlot2.m_data.size(), { ImPlotProp_Offset, m_nudgeYRawPlot2.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      }

      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
      ImPlot::PlotLine("Cabinet Acceleration Y (m/s^2)", &m_nudgeYPlot.m_data[0].x, &m_nudgeYPlot.m_data[0].y, m_nudgeYPlot.m_data.size(),
         { ImPlotProp_FillColor, ImVec4(1, 0, 0, 0.25f), ImPlotProp_Offset, m_nudgeYPlot.m_offset, ImPlotProp_Stride, 2 * (int)sizeof(float) });
      ImPlot::EndPlot();
   }
   ImGui::PopFont();

   ImPlot::PopStyleColor();
   ImGui::End();
}

}
