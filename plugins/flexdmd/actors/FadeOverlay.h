#pragma once

#include "Actor.h"

class FadeOverlay final : public Actor
{
public:
   FadeOverlay(FlexDMD* pFlexDMD, const string& name)
      : Actor(pFlexDMD, name) { }
   ~FadeOverlay() = default;

   float GetAlpha() const { return m_alpha; }
   void SetAlpha(float alpha) { m_alpha = alpha; }
   void SetColor(uint32_t color) { m_color = color; }
   void Draw(VP::SurfaceGraphics* pGraphics) override;

private:
   float m_alpha = 1.f;
   uint32_t m_color = 0x00000000;
};
