// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "ui/live/PlotData.h"

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
   void BuildPage() override;
   InputManager& GetInput() const { return m_player->m_pininput; }
   void AppendPlot();

   unsigned int m_notificationId = 0;

   PlotData m_nudgeXPlot;
   PlotData m_nudgeYPlot;
   PlotData m_cabXPlot;
   PlotData m_cabYPlot;

   uint32_t m_resetTimestampMs = 0;
};

}
