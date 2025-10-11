// license:GPLv3+

#pragma once

#include "core/player.h"
#include "PlotData.h"

class PerfUI final
{
public:
   PerfUI(Player* const player);
   ~PerfUI();

   void SetUIScale(float scale) { m_uiScale = scale; }

   enum PerfMode
   {
      PM_DISABLED,
      PM_FPS,
      PM_STATS
   };
   void NextPerfMode();
   PerfMode GetPerfMode() const { return m_showPerf; }
   void SetPerfMode(PerfMode mode) { m_showPerf = mode; }

   void Update();

private:
   void RenderFPS();
   void RenderStats();

   Player* const m_player;
   float m_uiScale = 1.0f;

   PerfMode m_showPerf = PerfMode::PM_DISABLED;
   bool m_showAvgFPS = true;
   bool m_showRollingFPS = true;
   
   PlotData m_plotFPS;
   PlotData m_plotFPSSmoothed;
   PlotData m_plotPhysx;
   PlotData m_plotPhysxSmoothed;
   PlotData m_plotScript;
   PlotData m_plotScriptSmoothed;
};
