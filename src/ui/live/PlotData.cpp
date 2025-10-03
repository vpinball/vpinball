// license:GPLv3+

#include "core/stdafx.h"

#include "PlotData.h"


PlotData::PlotData()
   : m_maxSize(500)
{
   m_data.reserve(m_maxSize);
   SetRolling(!m_rolling);
}

void PlotData::SetRolling(bool rolling)
{
   if (rolling == m_rolling)
      return;
   if (!m_data.empty())
      m_data.shrink(0);
   m_offset = 0;
   m_rolling = rolling;
}

void PlotData::AddPoint(const float x, const float y)
{
   if (std::isinf(y))
      return;

   if (m_rolling)
   {
      const float xmod = fmodf(x, m_timeSpan);
      if (!m_data.empty() && m_data.back().x == xmod)
         return;

      if (!m_data.empty() && xmod < m_data.back().x)
         m_data.shrink(0);

      m_data.push_back(ImVec2(xmod, y));
   }
   else
   {
      if (!m_data.empty() && m_data.back().x == x)
         return;

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

   if (y > m_movingMax)
      m_movingMax = y;
   else
      m_movingMax = lerp(m_movingMax, y, 0.01f);

   if (y < m_movingMin)
      m_movingMin = y;
   else
      m_movingMin = lerp(m_movingMin, y, 0.01f);
}

bool PlotData::HasData() const
{
   return !m_data.empty();
}

ImVec2 PlotData::GetLast() const
{
   if (m_data.empty())
      return ImVec2 { 0.f, 0.f };
   else if (m_data.size() < m_maxSize || m_offset == 0)
      return m_data.back();
   else
      return m_data[m_offset - 1];
}

float PlotData::GetMovingMin() const
{
   return m_movingMin;
}

float PlotData::GetMovingMax() const
{
   return m_movingMax;
}
