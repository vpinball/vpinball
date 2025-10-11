// license:GPLv3+

#pragma once

class PlumbOverlay final
{
public:
   PlumbOverlay() = default;
   ~PlumbOverlay() = default;

   void SetUIScale(float scale) { m_uiScale = scale; }

   void Update();

private:
   float m_uiScale = 1.0f;
   int m_lastTiltIndex = 0;
   float m_tiltFade = 0.f;
   int m_plumbFadeCounter = 1000;
};
