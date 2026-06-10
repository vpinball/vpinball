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
   void Render(float elapsed) override;

   bool IsPlayerPauseAllowed() const override { return false; }

private:
   void BuildPage() override;

   InputManager& GetInput() const { return m_player->m_pininput; }
   void AppendPlot();

   PlotData m_positionPlot;
   PlotData m_velocityPlot;
};

}
