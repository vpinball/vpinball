// license:GPLv3+

#pragma once

#include "../PlotData.h"

namespace VPX::InGameUI
{

class NudgeSettingsPage final : public InGameUIPage
{
public:
   NudgeSettingsPage();

   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Render(float elapsed) override;

   bool IsPlayerPauseAllowed() const override { return false; }

private:
   InputManager& GetInput() const { return m_player->m_pininput; }
   void AppendPlot();

   PlotData m_nudgeXPlot;
   PlotData m_nudgeYPlot;
   PlotData m_nudgeXRawPlot[2];
   PlotData m_nudgeYRawPlot[2];

   uint32_t m_resetTimestampMs = 0;
   int m_sensorAcqPeriod[4] {};
};

}
