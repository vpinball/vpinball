// license:GPLv3+

#include "core/stdafx.h"
#include "implot/implot.h"

#include "NudgeSettingsPage.h"

namespace VPX::InGameUI
{

NudgeSettingsPage::NudgeSettingsPage()
   : InGameUIPage("Nudge Settings"s, ""s, SaveMode::Global)
{
   InputManager& input = GetInput();

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Hardware sensor based emulated nudge"s));

   for (int i = 0; i < 2; i++)
   {
      AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Info, "Hardware sensor #" + std::to_string(i + 1)));

      const auto& nudgeXSensor = input.GetNudgeXSensor(i);
      AddItem(std::make_unique<InGameUIItem>(nudgeXSensor->GetLabel(), "Select to define which analog input to use for side nudge."s, nudgeXSensor.get(), 0x7));

      const auto& nudgeYSensor = input.GetNudgeYSensor(i);
      AddItem(std::make_unique<InGameUIItem>(nudgeYSensor->GetLabel(), "Select to define which analog input to use for front nudge."s, nudgeYSensor.get(), 0x7));

      AddItem(std::make_unique<InGameUIItem>(
         i == 0 ? Settings::m_propPlayer_NudgeOrientation0 : Settings::m_propPlayer_NudgeOrientation1, 1.f, "%4.1f deg"s, //
         [this, i]() { return RADTOANG(GetInput().GetNudgeOrientation(i)); }, //
         [this, i](float, float v) { GetInput().SetNudgeOrientation(i, ANGTORAD(v)); }));

      AddItem(std::make_unique<InGameUIItem>(
         i == 0 ? Settings::m_propPlayer_NudgeFilter0 : Settings::m_propPlayer_NudgeFilter1, //
         [this, i]() { return GetInput().IsNudgeFiltered(i); }, //
         [this, i](bool v) { GetInput().SetNudgeFiltered(i, v); }));
   }

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Emulated tilt plumb"s));

   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_SimulatedPlumb, //
      [this]() { return m_player->m_physics->IsPlumbSimulated(); }, //
      [this](bool v) { m_player->m_physics->EnablePlumbSimulation(v); }));

   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_PlumbInertia, 100.f, "%4.1f %%"s, //
      [this]() { return m_player->m_physics->GetPlumbInertia(); }, //
      [this](float, float v) { m_player->m_physics->SetPlumbInertia(v); }));

   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_PlumbThresholdAngle, 1.f, "%4.2f deg"s, //
      [this]() { return RADTOANG(m_player->m_physics->GetPlumbTiltThreshold()); }, //
      [this](float, float v) { m_player->m_physics->SetPlumbTiltThreshold(ANGTORAD(v)); }));

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Keyboard emulated nudge"s));

   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_EnableLegacyNudge, //
      [this]() { return m_player->m_physics->IsLegacyKeyboardNudge(); }, //
      [this](bool v) { m_player->m_physics->SetLegacyKeyboardNudge(v); }));

   AddItem(std::make_unique<InGameUIItem>(
      Settings::m_propPlayer_LegacyNudgeStrength, 1.f, "%4.1f"s, //
      [this]() { return m_player->m_physics->GetLegacyKeyboardNudgeStrength(); }, //
      [this](float, float v) { m_player->m_physics->SetLegacyKeyboardNudgeStrength(v); }));

   ////////////////////////////////////////////////////////////////////////////////////////////////

   AddItem(std::make_unique<InGameUIItem>(InGameUIItem::LabelType::Header, "Visual feedback"s));

   // TODO this property is directly persisted. It does not follow the overall UI design: App/Table/Live state => Implement live state (will also enable table override)
   AddItem(std::make_unique<InGameUIItem>( //
      Settings::m_propPlayer_NudgeStrength, 400.f, "%4.1f %%"s, //
      [this]() { return m_player->m_ptable->m_settings.GetPlayer_NudgeStrength(); }, //
      [this](float, float v)
      {
         m_player->m_ptable->m_settings.SetPlayer_NudgeStrength(v, false);
         m_player->m_liveUI->PushNotification("This change will be applied after restarting the player."s, 3000);
      }));


   m_nudgeXPlot.m_rolling = true;
   m_nudgeXPlot.m_timeSpan = 5.f;
   m_nudgeYPlot.m_rolling = true;
   m_nudgeYPlot.m_timeSpan = 5.f;
   for (int i = 0; i < 2; i++)
   {
      m_nudgeXRawPlot[i].m_rolling = true;
      m_nudgeXRawPlot[i].m_timeSpan = 5.f;
      m_nudgeYRawPlot[i].m_rolling = true;
      m_nudgeYRawPlot[i].m_timeSpan = 5.f;
   }
}

void NudgeSettingsPage::Open(bool isBackwardAnimation)
{
   InGameUIPage::Open(isBackwardAnimation);
   m_player->m_pininput.AddAxisListener([this]() { AppendPlot(); });
}

void NudgeSettingsPage::Close(bool isBackwardAnimation)
{
   InGameUIPage::Close(isBackwardAnimation);
   m_player->m_pininput.ClearAxisListeners();
}

void NudgeSettingsPage::AppendPlot()
{
   const float t = static_cast<float>((double)msec() / 1000.);
   Vertex2D nudge = m_player->m_pininput.GetNudge();
   m_nudgeXPlot.AddPoint(t, nudge.x);
   m_nudgeYPlot.AddPoint(t, nudge.y);
   for (int i = 0; i < 2; i++)
   {
      if (m_player->m_pininput.GetNudgeXSensor(i)->IsMapped())
         m_nudgeXRawPlot[i].AddPoint(t, m_player->m_pininput.GetNudgeXSensor(i)->GetMapping().GetValue());
      if (m_player->m_pininput.GetNudgeYSensor(i)->IsMapped())
         m_nudgeYRawPlot[i].AddPoint(t, m_player->m_pininput.GetNudgeYSensor(i)->GetMapping().GetValue());
   }
}

void NudgeSettingsPage::Render(float elapsed)
{
   InGameUIPage::Render(elapsed);
   const ImGuiStyle& style = ImGui::GetStyle();
   const ImVec2 winSize = ImVec2(GetWindowSize().x, 400.f);
   const float plotHeight = winSize.y / 2 - style.ItemSpacing.x;
   const float plumbSize = plotHeight; //m_player->m_physics->IsPlumbSimulated() ? plotHeight : 0.f;
   const float plotWidth = winSize.x - style.WindowPadding.x * 2.f - style.ItemSpacing.x - plumbSize;

   const uint32_t ms = msec();
   if (ms - m_resetTimestampMs > 2000)
   {
      m_resetTimestampMs = ms;
      for (int sensor = 0; sensor < 2; sensor++)
      {
         m_sensorAcqPeriod[sensor].x = 0;
         m_sensorAcqPeriod[sensor].y = 0;
         if (m_player->m_pininput.GetNudgeXSensor(sensor)->IsMapped())
         {
            m_sensorAcqPeriod[sensor].x = m_player->m_pininput.GetNudgeXSensor(sensor)->GetMapping().GetShortestUpdateMs();
            m_player->m_pininput.GetNudgeXSensor(sensor)->GetMapping().ResetShortestUpdateMs();
         }
         if (m_player->m_pininput.GetNudgeYSensor(sensor)->IsMapped())
         {
            m_sensorAcqPeriod[sensor].y = m_player->m_pininput.GetNudgeYSensor(sensor)->GetMapping().GetShortestUpdateMs();
            m_player->m_pininput.GetNudgeYSensor(sensor)->GetMapping().ResetShortestUpdateMs();
         }
      }
   }

   AppendPlot();

   auto renderSensorInfo = [this, plumbSize](int sensor)
   {
      ImGui::BeginChild(("SensorInfo" + std::to_string(sensor)).c_str(), ImVec2(plumbSize, plumbSize));
      ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(2.f, 2.f));
      ImGui::BeginGroup();
      ImGui::SameLine(0.5f * (plumbSize - ImGui::CalcTextSize("Sensor #1").x));
      ImGui::Text("Sensor #%d", sensor + 1);
      ImVec2 min = ImGui::GetItemRectMin();
      ImVec2 max = ImGui::GetItemRectMax();
      min.y = max.y;
      ImGui::GetWindowDrawList()->AddLine(min, max, IM_COL32_WHITE, 1.0f);
      ImGui::Separator();
      ImGui::Text("Refresh Rate");
      if (m_sensorAcqPeriod[sensor].x != 0)
         ImGui::Text("  X: %4dms", m_sensorAcqPeriod[sensor].x);
      else
         ImGui::Text("  X:  - ms");
      if (m_sensorAcqPeriod[sensor].y != 0)
         ImGui::Text("  Y: %4dms", m_sensorAcqPeriod[sensor].y);
      else
         ImGui::Text("  Y:  - ms");
      ImGui::Separator();
      ImGui::Text("Nudge Frequency");
      float freq = 0.f;
      if (m_player->m_pininput.GetNudgeYSensor(sensor)->IsMapped())
         freq = m_player->m_pininput.GetNudgeYSensor(sensor)->GetMapping().GetMainFrequency();
      if (freq != 0)
         ImGui::Text("  Freq: %4.2fHz", freq);
      else
         ImGui::Text("  Freq:  - Hz");
      ImGui::EndGroup();
      ImGui::EndChild();
   };

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
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_nudgeXPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y2, -0.3f, 0.3f, ImGuiCond_Always);
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      for (int i = 0; i < 2; i++)
         if (m_player->m_pininput.GetNudgeXSensor(i)->IsMapped() && m_nudgeXRawPlot[i].HasData())
            ImPlot::PlotLine(("Sensor X" + std::to_string(i + 1) + " - " + m_player->m_pininput.GetNudgeXSensor(i)->GetMappingLabel()).c_str(), 
               &m_nudgeXRawPlot[i].m_data[0].x, //
               &m_nudgeXRawPlot[i].m_data[0].y, //
               m_nudgeXRawPlot[i].m_data.size(),  //
               ImPlotLineFlags_None, //
               m_nudgeXRawPlot[i].m_offset, 2 * sizeof(float));
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
      ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
      ImPlot::PlotLine("Nudge X", &m_nudgeXPlot.m_data[0].x, &m_nudgeXPlot.m_data[0].y, m_nudgeXPlot.m_data.size(), 0, m_nudgeXPlot.m_offset, 2 * sizeof(float));
      ImPlot::PopStyleColor();
      ImPlot::EndPlot();
   }
   ImGui::PopFont();

   ImGui::SameLine();
   
   renderSensorInfo(0);

   ImGui::PushFont(nullptr, style.FontSizeBase * 0.5f); // Smaller font to keep graphics readable
   if (ImPlot::BeginPlot("##NudgeY", ImVec2(plotWidth, plotHeight), ImPlotFlags_None))
   {
      ImPlot::SetupAxis(ImAxis_X1, nullptr, ImPlotAxisFlags_NoTickLabels);
      ImPlot::SetupAxis(ImAxis_Y1, nullptr, ImPlotAxisFlags_None);
      ImPlot::SetupAxis(ImAxis_Y2, nullptr, ImPlotAxisFlags_AuxDefault);
      ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_nudgeYPlot.m_timeSpan, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -1.2f, 1.2f, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y2, -0.3f, 0.3f, ImGuiCond_Always);
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      for (int i = 0; i < 2; i++)
         if (m_player->m_pininput.GetNudgeYSensor(i)->IsMapped() && m_nudgeYRawPlot[i].HasData())
            ImPlot::PlotLine(("Sensor Y" + std::to_string(i + 1) + " - " + m_player->m_pininput.GetNudgeYSensor(i)->GetMappingLabel()).c_str(), 
               &m_nudgeYRawPlot[i].m_data[0].x, //
               &m_nudgeYRawPlot[i].m_data[0].y, //
               m_nudgeYRawPlot[i].m_data.size(), //
               ImPlotLineFlags_None, //
               m_nudgeYRawPlot[i].m_offset, 2 * sizeof(float));
      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y2);
      ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1, 0, 0, 0.25f));
      ImPlot::PlotLine("Nudge Y", &m_nudgeYPlot.m_data[0].x, &m_nudgeYPlot.m_data[0].y, m_nudgeYPlot.m_data.size(), 0, m_nudgeYPlot.m_offset, 2 * sizeof(float));
      ImPlot::PopStyleColor();
      ImPlot::EndPlot();
   }
   ImGui::PopFont();

   ImGui::SameLine();

   renderSensorInfo(1);

   ImPlot::PopStyleColor();
   ImGui::End();
}

}
