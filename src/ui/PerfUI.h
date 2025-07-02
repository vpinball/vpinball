// license:GPLv3+

#pragma once

#include "core/player.h"

class PerfUI final
{
public:
   PerfUI(Player* const player);
   ~PerfUI();

   void SetDPI(float dpi) { m_dpi = dpi; }

   enum PerfMode
   {
      PM_DISABLED,
      PM_FPS,
      PM_STATS
   };
   void NextPerfMode();
   PerfMode GetPerfMode() const { return m_showPerf; }

   void Update();

private:
   Player* const m_player;
   float m_dpi = 1.0f;

   PerfMode m_showPerf = PerfMode::PM_DISABLED;
   bool m_showAvgFPS = true;
   bool m_showRollingFPS = true;
   
   class PlotData
   {
   public:
      PlotData() : m_maxSize(500)
      {
         m_data.reserve(m_maxSize);
         SetRolling(!m_rolling);
      }
      void SetRolling(bool rolling)
      {
         if (rolling == m_rolling)
            return;
         if (!m_data.empty())
            m_data.shrink(0);
         m_offset = 0;
         m_rolling = rolling;
      }
      void AddPoint(const float x, const float y)
      {
         if (!m_data.empty() && m_data.back().x == x)
            return;
         if (std::isinf(y))
            return;
         if (y > m_movingMax)
            m_movingMax = y;
         else
            m_movingMax = lerp(m_movingMax, y, 0.01f);
         if (m_rolling)
         {
            const float xmod = fmodf(x, m_timeSpan);
            if (!m_data.empty() && xmod < m_data.back().x)
               m_data.shrink(0);
            m_data.push_back(ImVec2(xmod, y));
         }
         else
         {
            if (m_data.size() < m_maxSize)
               m_data.push_back(ImVec2(x, y));
            else
            {
               m_data[m_offset] = ImVec2(x, y);
               m_offset++;
               if (m_offset == m_maxSize)
                  m_offset = 0;
            }
         }
      }
      bool HasData() const { return !m_data.empty(); }
      ImVec2 GetLast() const
      {
         if (m_data.empty())
            return ImVec2 { 0.f, 0.f };
         else if (m_data.size() < m_maxSize || m_offset == 0)
            return m_data.back();
         else
            return m_data[m_offset - 1];
      }
      float GetMovingMax() const { return m_movingMax; }

   public:
      int m_offset = 0;
      float m_timeSpan = 2.5f;
      ImVector<ImVec2> m_data;
      bool m_rolling = true;
      float m_movingMax = 0.f;

   private:
      const int m_maxSize;
   };
   PlotData m_plotFPS, m_plotFPSSmoothed, m_plotPhysx, m_plotPhysxSmoothed, m_plotScript, m_plotScriptSmoothed;
};
