// license:GPLv3+

#pragma once

#include "InGameUIPage.h"
#include "ui/live/PlotData.h"

namespace VPX::InGameUI
{

class PlungerSettingsPage final : public InGameUIPage
{
public:
   PlungerSettingsPage();

   void Open(bool isBackwardAnimation) override;
   void Close(bool isBackwardAnimation) override;
   void Update(float elapsedS) override;
   void CollectPanes(vector<InGameUIPane>& panes) override;

   bool IsPlayerPauseAllowed() const override { return false; }

private:
   void BuildPage() override;

   InputManager& GetInput() const { return m_player->m_pininput; }
   void AppendPlot();
   void RenderPlungerPlots(const ImVec4& rect);

   PlotData m_positionPlot;
   PlotData m_velocityPlot;
};

}
