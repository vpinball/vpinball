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
   void Update(float elapsedS) override;
   void CollectPanes(vector<InGameUIPane>& panes) override;

   bool IsPlayerPauseAllowed() const override { return false; }

private:
   void BuildPage() override;
   InputManager& GetInput() const { return m_player->m_pininput; }
   void AppendPlot();
   void RenderNudgePlots(const ImVec4& rect);

   PlotData m_nudgeXPlot;
   PlotData m_nudgeYPlot;
   PlotData m_cabXPlot;
   PlotData m_cabYPlot;

   uint32_t m_resetTimestampMs = 0;
};

}
