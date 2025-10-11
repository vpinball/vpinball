// license:GPLv3+

#pragma once


namespace VPX::InGameUI
{

class PlungerSettingsPage final : public InGameUIPage
{
public:
   PlungerSettingsPage();

   void Open() override;
   void Close() override;
   void Render(float elapsed) override;

   bool IsPlayerPauseAllowed() const override { return false; }

private:
   InputManager& GetInput() const { return m_player->m_pininput; }
   void AppendPlot();

   PlotData m_positionPlot;
   PlotData m_velocityPlot;
};

}
