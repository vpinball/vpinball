// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "physics/cabinet/NudgeHandler.h"
#include "ui/live/PlotData.h"
#include "ui/live/ingameui/SensorSetupPage.h"

namespace VPX::InGameUI
{

class NudgeSensorSettingsPage final : public InGameUIPage
{
public:
   NudgeSensorSettingsPage(int sensorIndex);

   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Render(float elapsed) override;

   bool IsPlayerPauseAllowed() const override { return false; }

   void BuildPage() override;

private:
   void AppendPlot();

   InputManager& GetInput() const { return m_player->m_pininput; }
   const std::unique_ptr<VPX::Physics::NudgeSensor>& GetSensor() const { return m_player->m_pininput.m_nudgeHandler->GetSensor(m_sensorIndex); }

   const int m_sensorIndex;

   SensorSetupPageSection m_sideAxisSection;
   SensorSetupPageSection m_frontAxisSection;
   SensorSetupPageSection m_sideAxisSection2;
   SensorSetupPageSection m_frontAxisSection2;

   PlotData m_nudgeXPlot;
   PlotData m_nudgeYPlot;
   PlotData m_nudgeXRawPlot1;
   PlotData m_nudgeYRawPlot1;
   PlotData m_nudgeXRawPlot2;
   PlotData m_nudgeYRawPlot2;

   uint32_t m_resetTimestampMs = 0;
   int2 m_sensorAcqPeriod;

   bool m_removed = false;
};

}
