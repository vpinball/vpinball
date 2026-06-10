// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "input/PlungerHandler.h"
#include "ui/live/PlotData.h"
#include "ui/live/ingameui/SensorSetupPage.h"

namespace VPX::InGameUI
{

class PlungerSensorSettingsPage final : public InGameUIPage
{
public:
   PlungerSensorSettingsPage(int sensorIndex);

   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Render(float elapsed) override;

   bool IsPlayerPauseAllowed() const override { return false; }

private:
   void BuildPage() override;
   void AppendPlot();

   const std::unique_ptr<PlungerSensor>& GetSensor() const { return m_player->m_pininput.m_plungerHandler->GetSensor(m_sensorIndex); }

   const int m_sensorIndex;

   SensorSetupPageSection m_sideAxisSection;
   SensorSetupPageSection m_frontAxisSection; // reuse sections for position/velocity

   PlotData m_positionPlot;
   PlotData m_velocityPlot;
   PlotData m_positionRawPlot;
   PlotData m_velocityRawPlot;

   uint32_t m_resetTimestampMs = 0;
   int2 m_sensorAcqPeriod;

   bool m_removed = false;
};

}
