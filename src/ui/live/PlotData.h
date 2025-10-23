// license:GPLv3+

#pragma once


class PlotData final
{
public:
   PlotData();

   void SetRolling(bool rolling);
   void AddPoint(const float x, const float y);
   bool HasData() const { return !m_data.empty(); }
   ImVec2 GetLast() const;
   float GetMovingMax() const { return m_movingMax; }
   float GetMovingMin() const { return m_movingMin; }

public:
   int m_offset = 0;
   float m_timeSpan = 2.5f;
   ImVector<ImVec2> m_data;
   bool m_rolling = true;
   float m_movingMax = 0.f;
   float m_movingMin = 0.f;

private:
   const int m_maxSize;
};
